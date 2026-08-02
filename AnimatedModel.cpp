#include "AnimatedModel.h"
#include <iostream>

AnimatedModel::AnimatedModel(const char* modelPath, bool index32,VkDescriptorSetLayout vkDescriptorSetLayout)
{
    // Always start clean
    memset(&vulkanComboData, 0, sizeof(VulkanComboData));

    boneMapping.clear();
    boneOffsetMatrices.clear();
    finalBoneMatrices.clear();

	VkResult vkResult = LoadModel_Animated_PBR(modelPath, index32, vkDescriptorSetLayout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "AnimatedModel::AnimatedModel() : LoadModel_Animated_PBR() failed (%d).\n", vkResult);
	}

    for (uint32_t i = 0; i < 2; ++i)
    {
        VkResult res = createSSBO(gVulkanContext.vkDevice, gVulkanContext.vkPhysicalDevice, boneSSBO[i]);
        if (res != VK_SUCCESS)
            throw std::runtime_error("Failed to create bone SSBO");
    }

}
AnimatedModel::~AnimatedModel()
{
	ZzDestroyVertexAndIndexBuffer(&vulkanComboData);
	//bone SSBOs
    for (uint32_t i = 0; i < 2; ++i)
    {
        if (boneSSBO[i].vkBuffer != VK_NULL_HANDLE)
        {
            vkUnmapMemory(gVulkanContext.vkDevice, boneSSBO[i].vkDeviceMemory);
            vkDestroyBuffer(gVulkanContext.vkDevice, boneSSBO[i].vkBuffer, nullptr);
            vkFreeMemory(gVulkanContext.vkDevice, boneSSBO[i].vkDeviceMemory, nullptr);
            boneSSBO[i].vkBuffer = VK_NULL_HANDLE;
            boneSSBO[i].vkDeviceMemory = VK_NULL_HANDLE;
            boneSSBO[i].mapped = nullptr;
        }
	}

	//PBR materials
    for (auto& matInfo : PBR_Materials) {
        if (matInfo.data) {
            delete matInfo.data;
            matInfo.data = nullptr;
        }
	}



    importer.FreeScene();

    scene = nullptr;
    currentAnimation = nullptr;

    boneMapping.clear();
    boneOffsetMatrices.clear();
    finalBoneMatrices.clear();
}

VkResult AnimatedModel::LoadModel_Animated_PBR(const char* modelPath, bool index32, VkDescriptorSetLayout vkDescriptorSetLayout_PBR)
{
    VkResult vkResult = VK_SUCCESS;
    // Similar implementation as LoadModel_Phong but for PBR-specific vertex structure
    // This function is a placeholder and should be implemented similarly to LoadModel_Phong

         scene = importer.ReadFile(modelPath,
        aiProcess_Triangulate             // ensure all faces are triangles
        | aiProcess_JoinIdenticalVertices   // remove duplicates (safe, keeps indices consistent)
        | aiProcess_GenSmoothNormals        // generate normals if missing
        | aiProcess_CalcTangentSpace        // tangents for normal mapping
        | aiProcess_LimitBoneWeights        // clamp to 4 bone weights per vertex (required for GPU skinning)
        | aiProcess_ImproveCacheLocality    // optimize vertex cache locality
        | aiProcess_SortByPType             // separate points/lines/triangles (we only want triangles)
        | aiProcess_FlipWindingOrder        // convert to Vulkan's right-handed coordinate system
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
        fprintf(gpFILE, "LoadModel_PBR_Skinned() : Assimp Importer::ReadFile() failed.\n");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

	//number of materials
	uint16_t numMaterials = scene->mNumMaterials;

    for (unsigned int i = 0; i < numMaterials; ++i) 
    {
        const aiMaterial* mat = scene->mMaterials[i];

        // Try to get material name (optional)
        aiString matName;
        std::string materialName = "(unnamed)";
        if (AI_SUCCESS == mat->Get(AI_MATKEY_NAME, matName)) materialName = matName.C_Str();

        //std::cout << "Meshes: " << scene->mNumMeshes << std::endl;

        aiString tex;
        // We only ask for the path here; other args are optional
        if (AI_SUCCESS == mat->GetTexture(aiTextureType_DIFFUSE, 0, &tex)) {
            MaterialInfo info;

            std::string directory = std::filesystem::path(tex.C_Str()).parent_path().string();
            if (!directory.empty())
                directory += std::filesystem::path::preferred_separator;

            info.materialName = materialName;
            info.path = directory;
            info.data = new Material_BasicPBR(vkDescriptorSetLayout_PBR, directory.c_str());
            PBR_Materials.push_back(info);
        }


        //    for (unsigned i = 0; i < mat->mNumProperties; i++)
        //    {
        //        aiMaterialProperty* prop = mat->mProperties[i];
        //        
        //        fprintf(gpFILE, "Property %d: key=%s, semantic=%u, index=%u, dataLength=%u, type=%u\n",
                   //i, prop->mKey.C_Str(), prop->mSemantic, prop->mIndex, prop->mDataLength, prop->mType);
        //    }

	}

	//Global inverse transform
	globalInverseTransform = glm::inverse(aiMat4ToGlm(scene->mRootNode->mTransformation));

    std::vector<VertexData_Skinned> vkVertices;

    // for skinned mesh
    uint32_t numBones = 0;

    if (index32)
    {
        std::vector<uint32_t> vkIndices;
        // before looping meshes:
        boneMapping.clear();
        boneOffsetMatrices.clear();
        numBones = 0;

        if (scene && scene->mRootNode) {
            for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
                aiMesh* mesh = scene->mMeshes[meshIndex];

                // Vertices
                for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                    VertexData_Skinned v{};
                    v.boneIDs = glm::ivec4(-1);
                    v.boneWeights = glm::vec4(0.0f);

                    // Position
                    if (mesh->HasPositions()) {
                        v.pos = glm::vec3(
                            mesh->mVertices[i].x,
                            mesh->mVertices[i].y,
                            mesh->mVertices[i].z
                        );
                    }

                    // Normal
                    if (mesh->HasNormals())
                    {
                        v.normal = glm::vec3(
                            mesh->mNormals[i].x,
                            mesh->mNormals[i].y,
                            mesh->mNormals[i].z
                        );
                    }
                    else {
                        v.normal = glm::vec3(0.0f, 0.0f, 1.0f); // default fallback
                    }

                    // TexCoords (Assimp supports 8 UV channels; we use channel 0)
                    if (mesh->HasTextureCoords(0))
                    {
                        v.texCoord = glm::vec2(
                            mesh->mTextureCoords[0][i].x,
                            mesh->mTextureCoords[0][i].y
                        );
                    }
                    else {
                        v.texCoord = glm::vec2(0.0f);
                    }

                    // Vertex Tangents 
                    if (mesh->HasTangentsAndBitangents())
                    {
                        glm::vec3 tangent(
                            mesh->mTangents[i].x,
                            mesh->mTangents[i].y,
                            mesh->mTangents[i].z
                        );

                        glm::vec3 bitangent(
                            mesh->mBitangents[i].x,
                            mesh->mBitangents[i].y,
                            mesh->mBitangents[i].z
                        );

                        glm::vec3 normal = glm::normalize(v.normal);
                        tangent = glm::normalize(tangent);

                        float handedness = (glm::dot(glm::cross(tangent, normal), bitangent) < 0.0f) ? -1.0f : 1.0f;
                        v.tangent = glm::vec4(tangent, handedness);
                    }
                    else {
                        // Fallback: X-axis tangent, right-handed
                        v.tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
                    }

                    vkVertices.push_back(v);
                }

                // Indices (faces are always triangles if aiProcess_Triangulate is used)
                for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
                    const aiFace& face = mesh->mFaces[f];
                    for (unsigned int j = 0; j < face.mNumIndices; j++) {
                        vkIndices.push_back(face.mIndices[j]);
                    }
                }

                //bones

                // inside mesh loop, after creating a temporary vertices[] array for this mesh:
                if (mesh->mNumBones > 0) {
                    for (unsigned int i = 0; i < mesh->mNumBones; ++i) {
                        std::string boneName(mesh->mBones[i]->mName.C_Str());
                        uint32_t boneIndex = 0;
                        if (boneMapping.find(boneName) == boneMapping.end()) {
                            boneIndex = numBones++;
                            boneMapping[boneName] = boneIndex;

                            glm::mat4 offsetMatrix = aiMat4ToGlm(mesh->mBones[i]->mOffsetMatrix);
                            boneOffsetMatrices.push_back(offsetMatrix);
                        }
                        else {
                            boneIndex = boneMapping[boneName];
                        }

                        // assign weights to the affected vertices (vertex indices are mesh-local)
                        for (unsigned int w = 0; w < mesh->mBones[i]->mNumWeights; ++w) {
                            unsigned int vertexID = mesh->mBones[i]->mWeights[w].mVertexId;
                            float weight = mesh->mBones[i]->mWeights[w].mWeight;
                            // make sure the vertex array exists and has the vertexID
                            //tmpVertices[vertexID].AddBoneData(boneIndex, weight);

                            AddBoneDataToVertex(vkVertices[vertexID], boneIndex, weight);


                        }
                    }
                }

                assert(boneOffsetMatrices.size() == numBones);
            }


        }

        vulkanComboData.indicesCount = static_cast<uint32_t>(vkIndices.size());

        //PBR model
        vkResult = ZzCreateVertexAndIndex32Buffer(
            (float*)vkVertices.data(), vkVertices.size() * sizeof(VertexData_Skinned),
            vkIndices.data(), vkIndices.size() * sizeof(uint32_t),
            &vulkanComboData
        );
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "initialize() : ZzCreateVetexAndIndexBuffer() for Animated PBR model failed (%d).\n", vkResult);
            return(vkResult);
        }
    }
    else
    {
        std::vector<uint16_t> vkIndices;

        if (scene && scene->mRootNode) {
            for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
                aiMesh* mesh = scene->mMeshes[meshIndex];

                // Vertices
                for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                    VertexData_Skinned v{};

                    // Position
                    if (mesh->HasPositions()) {
                        v.pos = glm::vec3(
                            mesh->mVertices[i].x,
                            mesh->mVertices[i].y,
                            mesh->mVertices[i].z
                        );
                    }

                    // Normal
                    if (mesh->HasNormals())
                    {
                        v.normal = glm::vec3(
                            mesh->mNormals[i].x,
                            mesh->mNormals[i].y,
                            mesh->mNormals[i].z
                        );
                    }
                    else {
                        v.normal = glm::vec3(0.0f, 0.0f, 1.0f); // default fallback
                    }

                    // TexCoords (Assimp supports 8 UV channels; we use channel 0)
                    if (mesh->HasTextureCoords(0))
                    {
                        v.texCoord = glm::vec2(
                            mesh->mTextureCoords[0][i].x,
                            mesh->mTextureCoords[0][i].y
                        );
                    }
                    else {
                        v.texCoord = glm::vec2(0.0f);
                    }

                    // Vertex Tangents 
                    if (mesh->HasTangentsAndBitangents())
                    {
                        glm::vec3 tangent(
                            mesh->mTangents[i].x,
                            mesh->mTangents[i].y,
                            mesh->mTangents[i].z
                        );

                        glm::vec3 bitangent(
                            mesh->mBitangents[i].x,
                            mesh->mBitangents[i].y,
                            mesh->mBitangents[i].z
                        );

                        glm::vec3 normal = glm::normalize(v.normal);
                        tangent = glm::normalize(tangent);

                        float handedness = (glm::dot(glm::cross(tangent, normal), bitangent) < 0.0f) ? -1.0f : 1.0f;
                        v.tangent = glm::vec4(tangent, handedness);
                    }
                    else {
                        // Fallback: X-axis tangent, right-handed
                        v.tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
                    }

                    vkVertices.push_back(v);
                }

                // Indices (faces are always triangles if aiProcess_Triangulate is used)
                for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
                    const aiFace& face = mesh->mFaces[f];
                    for (unsigned int j = 0; j < face.mNumIndices; j++) {
                        vkIndices.push_back(face.mIndices[j]);
                    }
                }
            }
        }

        vulkanComboData.indicesCount = static_cast<uint32_t>(vkIndices.size());

        //Suzanne
        vkResult = ZzCreateVertexAndIndex16Buffer(
            (float*)vkVertices.data(), vkVertices.size() * sizeof(VertexData_Skinned),
            vkIndices.data(), vkIndices.size() * sizeof(uint16_t),
            &vulkanComboData
        );
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "initialize() : ZzCreateVetexAndIndexBuffer() for Animated PBR model failed (%d).\n", vkResult);
            return(vkResult);
        }
    }

    finalBoneMatrices.resize(numBones);
    for (uint32_t i = 0; i < numBones; ++i)
        finalBoneMatrices[i] = glm::mat4(1.0f);

	this->numBones = static_cast<uint16_t>(numBones);

    if (scene->mNumAnimations > 0)
        currentAnimation = scene->mAnimations[0];

    return VK_SUCCESS;
}

const aiNodeAnim* AnimatedModel::FindNodeAnim(const aiAnimation* anim,
    const std::string& name)
{
    for (uint32_t i = 0; i < anim->mNumChannels; i++)
    {
        const aiNodeAnim* channel = anim->mChannels[i];
        if (name == channel->mNodeName.C_Str())
            return channel;
    }
    return nullptr;
}

uint32_t AnimatedModel::FindPositionKey(float animationTime, const aiNodeAnim* channel)
{
    for (uint32_t i = 0; i < channel->mNumPositionKeys - 1; i++)
    {
        if (animationTime < channel->mPositionKeys[i + 1].mTime)
            return i;
    }
    return channel->mNumPositionKeys - 2;
}

uint32_t AnimatedModel::FindRotationKey(float animationTime, const aiNodeAnim* channel)
{
    for (uint32_t i = 0; i < channel->mNumRotationKeys - 1; i++)
    {
        if (animationTime < channel->mRotationKeys[i + 1].mTime)
            return i;
    }
    return channel->mNumRotationKeys - 2;
}

uint32_t AnimatedModel::FindScalingKey(float animationTime, const aiNodeAnim* channel)
{
    for (uint32_t i = 0; i < channel->mNumScalingKeys - 1; i++)
    {
        if (animationTime < channel->mScalingKeys[i + 1].mTime)
            return i;
    }
    return channel->mNumScalingKeys - 2;
}

glm::vec3 AnimatedModel::InterpolatePosition(float time, const aiNodeAnim* channel)
{
    if (channel->mNumPositionKeys == 1)
        return glm::vec3(channel->mPositionKeys[0].mValue.x,
            channel->mPositionKeys[0].mValue.y,
            channel->mPositionKeys[0].mValue.z);

    int index = FindPositionKey(time, channel);
    int next = index + 1;

    float delta =
        (float)channel->mPositionKeys[next].mTime -
        (float)channel->mPositionKeys[index].mTime;

    float factor =
        (float)(time - channel->mPositionKeys[index].mTime) / delta;

    auto& a = channel->mPositionKeys[index].mValue;
    auto& b = channel->mPositionKeys[next].mValue;

    return glm::mix(
        glm::vec3(a.x, a.y, a.z),
        glm::vec3(b.x, b.y, b.z),
        factor
    );
}

glm::quat AnimatedModel::InterpolateRotation(float time, const aiNodeAnim* channel)
{
    if (channel->mNumRotationKeys == 1)
    {
        auto& q = channel->mRotationKeys[0].mValue;
        return glm::quat(q.w, q.x, q.y, q.z);
    }

    int index = FindRotationKey(time, channel);
    int next = index + 1;

    float delta =
        (float)channel->mRotationKeys[next].mTime -
        (float)channel->mRotationKeys[index].mTime;

    float factor =
        (float)(time - channel->mRotationKeys[index].mTime) / delta;

    auto& a = channel->mRotationKeys[index].mValue;
    auto& b = channel->mRotationKeys[next].mValue;

    return glm::normalize(glm::slerp(
        glm::quat(a.w, a.x, a.y, a.z),
        glm::quat(b.w, b.x, b.y, b.z),
        factor
    ));
}

glm::vec3 AnimatedModel::InterpolateScale(float animationTime, const aiNodeAnim* channel)
{
    // Only one key no interpolation needed
    if (channel->mNumScalingKeys == 1)
    {
        const aiVector3D& v = channel->mScalingKeys[0].mValue;
        return glm::vec3(v.x, v.y, v.z);
    }

    uint32_t index = FindScalingKey(animationTime, channel);
    uint32_t nextIndex = index + 1;

    float t1 = (float)channel->mScalingKeys[index].mTime;
    float t2 = (float)channel->mScalingKeys[nextIndex].mTime;

    float factor = (animationTime - t1) / (t2 - t1);
    factor = glm::clamp(factor, 0.0f, 1.0f);

    const aiVector3D& start = channel->mScalingKeys[index].mValue;
    const aiVector3D& end = channel->mScalingKeys[nextIndex].mValue;

    return glm::mix(
        glm::vec3(start.x, start.y, start.z),
        glm::vec3(end.x, end.y, end.z),
        factor
    );
}

void AnimatedModel::ReadNodeHierarchy(float animationTime,
    const aiAnimation* animation,
    aiNode* node,
    const glm::mat4& parentTransform)
{
    std::string nodeName(node->mName.C_Str());

    // Node's default transform (bind pose)
    glm::mat4 nodeTransform =
        aiMat4ToGlm(node->mTransformation);

    const aiNodeAnim* channel = FindNodeAnim(animation, nodeName);

    // If animated, override bind transform
    if (channel)
    {
        glm::vec3 T = InterpolatePosition(animationTime, channel);
        glm::quat R = InterpolateRotation(animationTime, channel);
        glm::vec3 S = InterpolateScale(animationTime, channel);

        nodeTransform =
            glm::translate(glm::mat4(1.0f), T) *
            glm::mat4_cast(R) *
            glm::scale(glm::mat4(1.0f), S);
    }

    glm::mat4 globalTransform = parentTransform * nodeTransform;

    // If this node corresponds to a bone, update final matrix
    auto it = boneMapping.find(nodeName);
    if (it != boneMapping.end())
    {
        uint32_t boneIndex = it->second;

        // glm::mat4 finalTransformation = globalInverseTransform * globalTransform * boneOffsetMatrices[boneIndex];
        finalBoneMatrices[boneIndex] = globalInverseTransform * globalTransform * boneOffsetMatrices[boneIndex];

    }

    // Recurse
    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        ReadNodeHierarchy(animationTime,
            animation,
            node->mChildren[i],
            globalTransform);
    }
}

void AnimatedModel::UpdateAnimation(float deltaTime,uint16_t currFrame)
{
    if (!scene || !currentAnimation)
        return;

    const float ticksPerSecond =
        (currentAnimation->mTicksPerSecond != 0.0f)
        ? (float)currentAnimation->mTicksPerSecond
        : 25.0f;

    animationTime += deltaTime * ticksPerSecond * animationSpeed;
    animationTime = fmod(animationTime, (float)currentAnimation->mDuration);

    ReadNodeHierarchy(
        animationTime,
        currentAnimation,
        scene->mRootNode,
        glm::mat4(1.0f)
    );

	// Update SSBO with final bone matrices
	memcpy(boneSSBO[currFrame].mapped, finalBoneMatrices.data(), sizeof(glm::mat4) * finalBoneMatrices.size());

}

glm::mat4 AnimatedModel::aiMat4ToGlm(const aiMatrix4x4& m) {
    // Assimp stores row-major; glm::mat4 expects column-major when accessed with ptr.
    glm::mat4 out;
    out[0][0] = m.a1; out[1][0] = m.a2; out[2][0] = m.a3; out[3][0] = m.a4;
    out[0][1] = m.b1; out[1][1] = m.b2; out[2][1] = m.b3; out[3][1] = m.b4;
    out[0][2] = m.c1; out[1][2] = m.c2; out[2][2] = m.c3; out[3][2] = m.c4;
    out[0][3] = m.d1; out[1][3] = m.d2; out[2][3] = m.d3; out[3][3] = m.d4;
    return out;
}

// Helper: find memory type
uint32_t AnimatedModel::FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((typeFilter & (1u << i)) && ((memProps.memoryTypes[i].propertyFlags & properties) == properties)) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type!");
}

void AnimatedModel::AddBoneDataToVertex(VertexData_Skinned& v, int boneIndex, float weight) {
    // Find first zero-weight slot
    for (int i = 0; i < 4; ++i) {
        if (v.boneWeights[i] == 0.0f) {
            v.boneIDs[i] = boneIndex;
            v.boneWeights[i] = weight;
            return;
        }
    }
    // All slots taken: find smallest weight slot and replace if this weight is larger
    int minIdx = 0;
    float minVal = v.boneWeights[0];
    for (int i = 1; i < 4; ++i) {
        if (v.boneWeights[i] < minVal) {
            minVal = v.boneWeights[i];
            minIdx = i;
        }
    }
    if (weight > minVal) {
        v.boneIDs[minIdx] = boneIndex;
        v.boneWeights[minIdx] = weight;
    }
}

// createSSBO: creates a storage buffer backed by host-visible coherent memory and maps it persistently
VkResult AnimatedModel::createSSBO(VkDevice device, VkPhysicalDevice physicalDevice, VulkanSSBO& outSSBO)
{
    outSSBO.size = sizeof(glm::mat4) * (size_t)this->numBones;

    VkBufferCreateInfo bufCI{};
    bufCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufCI.size = outSSBO.size;
    bufCI.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; // transfer dst optional
    bufCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult res = vkCreateBuffer(device, &bufCI, nullptr, &outSSBO.vkBuffer);
    if (res != VK_SUCCESS) return res;

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device, outSSBO.vkBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;

    // Request HOST_VISIBLE && HOST_COHERENT for simplicity.
    VkMemoryPropertyFlags desired = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    uint32_t memTypeIndex = FindMemoryType(physicalDevice, memReq.memoryTypeBits, desired);
    allocInfo.memoryTypeIndex = memTypeIndex;

    res = vkAllocateMemory(device, &allocInfo, nullptr, &outSSBO.vkDeviceMemory);
    if (res != VK_SUCCESS) {
        vkDestroyBuffer(device, outSSBO.vkBuffer, nullptr);
        outSSBO.vkBuffer = VK_NULL_HANDLE;
        return res;
    }

    // Bind and map persistently
    res = vkBindBufferMemory(device, outSSBO.vkBuffer, outSSBO.vkDeviceMemory, 0);
    if (res != VK_SUCCESS) {
        vkFreeMemory(device, outSSBO.vkDeviceMemory, nullptr);
        vkDestroyBuffer(device, outSSBO.vkBuffer, nullptr);
        outSSBO.vkDeviceMemory = VK_NULL_HANDLE;
        outSSBO.vkBuffer = VK_NULL_HANDLE;
        return res;
    }

    // Persistently map entire allocation
    void* mappedPtr = nullptr;
    res = vkMapMemory(device, outSSBO.vkDeviceMemory, 0, outSSBO.size, 0, &mappedPtr);
    if (res != VK_SUCCESS) {
        vkFreeMemory(device, outSSBO.vkDeviceMemory, nullptr);
        vkDestroyBuffer(device, outSSBO.vkBuffer, nullptr);
        outSSBO.vkDeviceMemory = VK_NULL_HANDLE;
        outSSBO.vkBuffer = VK_NULL_HANDLE;
        return res;
    }

    outSSBO.mapped = mappedPtr;

    // Zero-initialize (optional)
    std::memset(outSSBO.mapped, 0, (size_t)outSSBO.size);

    // Descriptor info for descriptor updates
    outSSBO.descriptor.buffer = outSSBO.vkBuffer;
    outSSBO.descriptor.offset = 0;
    outSSBO.descriptor.range = outSSBO.size;

    return VK_SUCCESS;
}



VkResult AnimatedModel::ZzCreateVertexBuffer(const float* vertices, VkDeviceSize vertexBufferSize, VulkanData* vulkanData)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    //// 3 vertices, each with 3D position and RGB color
    //const VertexData_PositionColor vertices[] = {
    //    {{ 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // bottom (z=0), red
    //    {{ -1.0f,  -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},  // right, green
    //    {{ 1.0f,  -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // left, blue
    //};


    //staging buffer
    VulkanData vertexData_stagingBffer_position;
    memset((void*)&vertexData_stagingBffer_position, 0, sizeof(VulkanData));

    VkBufferCreateInfo vkBufferCreateInfo_stagingBuffer;
    memset((void*)&vkBufferCreateInfo_stagingBuffer, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo_stagingBuffer.pNext = NULL;
    vkBufferCreateInfo_stagingBuffer.flags = 0;
    vkBufferCreateInfo_stagingBuffer.size = vertexBufferSize;
    vkBufferCreateInfo_stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // staging buffer is used for transfering data to device local buffer
    vkBufferCreateInfo_stagingBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // not sharing with other queues

    // Call vkCreateBuffer() to create the staging buffer
    vkResult = vkCreateBuffer(gVulkanContext.vkDevice, &vkBufferCreateInfo_stagingBuffer, NULL, &vertexData_stagingBffer_position.vkBuffer);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer():  failed.\n");
        return(vkResult);
    }

    //------------
    // Get memory requirements for the staging buffer
    VkMemoryRequirements vkMemoryRequirements_stagingBuffer;
    memset((void*)&vkMemoryRequirements_stagingBuffer, 0, sizeof(vkMemoryRequirements_stagingBuffer));

    vkGetBufferMemoryRequirements(gVulkanContext.vkDevice, vertexData_stagingBffer_position.vkBuffer, &vkMemoryRequirements_stagingBuffer);
    //------------
    // Allocate memory for the staging buffer
    VkMemoryAllocateInfo vkMemoryAllocateInfo_stagingBuffer;
    memset((void*)&vkMemoryAllocateInfo_stagingBuffer, 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo_stagingBuffer.pNext = NULL;
    vkMemoryAllocateInfo_stagingBuffer.allocationSize = vkMemoryRequirements_stagingBuffer.size;
    vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = 0;
    //-------------
    // Find a suitable memory type for the staging buffer
    for (uint32_t i = 0; i < gVulkanContext.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if ((vkMemoryRequirements_stagingBuffer.memoryTypeBits & 1) == 1)
        {
            if (gVulkanContext.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) // host visible and coherent memory(no need to manage vulkan cache  for flushing or mapping)
            {
                vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = i;
                break;
            }
        }

        vkMemoryRequirements_stagingBuffer.memoryTypeBits >>= 1;
    }

    //--------------  
    // Allocate memory for the staging buffer
    vkResult = vkAllocateMemory(gVulkanContext.vkDevice, &vkMemoryAllocateInfo_stagingBuffer, NULL, &vertexData_stagingBffer_position.vkDeviceMemory);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  failed.\n");
        return(vkResult);
    }

    //---------------
    // Bind the staging buffer memory to the staging buffer
    vkResult = vkBindBufferMemory(gVulkanContext.vkDevice, vertexData_stagingBffer_position.vkBuffer, vertexData_stagingBffer_position.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
        return(vkResult);
    }

    //----------------
    void* data = NULL;
    vkResult = vkMapMemory(gVulkanContext.vkDevice, vertexData_stagingBffer_position.vkDeviceMemory, 0, vkMemoryAllocateInfo_stagingBuffer.allocationSize, 0, &data);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkMapMemory() :  failed.\n");
        return(vkResult);
    }


    //-------actual memory mapped io
    memcpy(data, vertices, vertexBufferSize);
    //-------unmap memory
    vkUnmapMemory(gVulkanContext.vkDevice, vertexData_stagingBffer_position.vkDeviceMemory);

    //-----------------------------------------------------------------------------------

    //device buffer
    memset((void*)vulkanData, 0, sizeof(VulkanData));
    VkBufferCreateInfo vkBufferCreateInfo_deviceBuffer;
    memset((void*)&vkBufferCreateInfo_deviceBuffer, 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo_deviceBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo_deviceBuffer.pNext = NULL;
    vkBufferCreateInfo_deviceBuffer.flags = 0;
    vkBufferCreateInfo_deviceBuffer.size = vertexBufferSize;
    vkBufferCreateInfo_deviceBuffer.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; // device buffer is used for vertex buffer and transfer destination
    vkBufferCreateInfo_deviceBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // not sharing with other queues

    vkResult = vkCreateBuffer(gVulkanContext.vkDevice, &vkBufferCreateInfo_deviceBuffer, NULL, &vulkanData->vkBuffer);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer():  failed.\n");
        return(vkResult);
    }


    //------------
    // Get memory requirements for the device local buffer
    VkMemoryRequirements vkMemoryRequirements_deviceBuffer;
    memset((void*)&vkMemoryRequirements_deviceBuffer, 0, sizeof(vkMemoryRequirements_deviceBuffer));
    vkGetBufferMemoryRequirements(gVulkanContext.vkDevice, vulkanData->vkBuffer, &vkMemoryRequirements_deviceBuffer);
    //------------
    // Allocate memory for the device local buffer
    VkMemoryAllocateInfo vkMemoryAllocateInfo_deviceBuffer;
    memset((void*)&vkMemoryAllocateInfo_deviceBuffer, 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo_deviceBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo_deviceBuffer.pNext = NULL;
    vkMemoryAllocateInfo_deviceBuffer.allocationSize = vkMemoryRequirements_deviceBuffer.size;
    vkMemoryAllocateInfo_deviceBuffer.memoryTypeIndex = 0;
    //-------------
    // Find a suitable memory type for the device local buffer

    for (uint32_t i = 0; i < gVulkanContext.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if ((vkMemoryRequirements_deviceBuffer.memoryTypeBits & 1) == 1)
        {
            if (gVulkanContext.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) // device local memory
            {
                vkMemoryAllocateInfo_deviceBuffer.memoryTypeIndex = i;
                break;
            }
        }

        vkMemoryRequirements_deviceBuffer.memoryTypeBits >>= 1;
    }

    //--------------
    // Allocate memory for the device local buffer
    vkResult = vkAllocateMemory(gVulkanContext.vkDevice, &vkMemoryAllocateInfo_deviceBuffer, NULL, &vulkanData->vkDeviceMemory);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  failed.\n");
        return(vkResult);
    }

    //---------------
    // Bind the device local buffer memory to the device local buffer
    vkResult = vkBindBufferMemory(gVulkanContext.vkDevice, vulkanData->vkBuffer, vulkanData->vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
        return(vkResult);
    }


    //----------------
    //command buffer for copy
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
    memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));
    vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo.pNext = NULL;
    vkCommandBufferAllocateInfo.commandPool = gVulkanContext.vkCommandPool;
    vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBufferAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer vkCommandBuffer_Copy = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(gVulkanContext.vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_Copy);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkAllocateCommandBuffers() :  failed.\n");
        return(vkResult);
    }


    //----------------

    // Begin command buffer recording
    VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
    memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
    vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo.pNext = NULL;
    vkCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // one time submit means we will submit this command buffer only once
    vkCommandBufferBeginInfo.pInheritanceInfo = NULL; // not using secondary command buffer inheritance
    vkResult = vkBeginCommandBuffer(vkCommandBuffer_Copy, &vkCommandBufferBeginInfo);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBeginCommandBuffer() :  failed.\n");
        return(vkResult);
    }


    //----------------
    // Record the command to copy data from staging buffer to device local buffer
    VkBufferCopy vkBufferCopy;
    memset((void*)&vkBufferCopy, 0, sizeof(VkBufferCopy));
    vkBufferCopy.srcOffset = 0; // offset in the source buffer
    vkBufferCopy.dstOffset = 0; // offset in the destination buffer
    vkBufferCopy.size = vertexBufferSize; // size of the data to copy
    vkCmdCopyBuffer(vkCommandBuffer_Copy, vertexData_stagingBffer_position.vkBuffer, vulkanData->vkBuffer, 1, &vkBufferCopy);

    // End command buffer recording
    vkResult = vkEndCommandBuffer(vkCommandBuffer_Copy);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkEndCommandBuffer() :  failed.\n");
        return(vkResult);
    }


    //----------------
    // Submit the command buffer to the queue
    VkSubmitInfo vkSubmitInfo;
    memset((void*)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));
    vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo.pNext = NULL;
    vkSubmitInfo.waitSemaphoreCount = 0; // no wait semaphores
    vkSubmitInfo.pWaitSemaphores = NULL; // no wait semaphores
    vkSubmitInfo.pWaitDstStageMask = NULL; // no wait stage mask
    vkSubmitInfo.commandBufferCount = 1; // one command buffer
    vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_Copy; // pointer to the command buffer to submit
    vkSubmitInfo.signalSemaphoreCount = 0; // no signal semaphores
    vkSubmitInfo.pSignalSemaphores = NULL; // no signal semaphores


    vkResult = vkQueueSubmit(gVulkanContext.vkGraphicsQueue, 1, &vkSubmitInfo, VK_NULL_HANDLE);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkQueueSubmit() :  failed.\n");
        return(vkResult);
    }

    // Wait for the queue to finish processing
    vkResult = vkQueueWaitIdle(gVulkanContext.vkGraphicsQueue);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkQueueWaitIdle() :  failed.\n");
        return(vkResult);
    }


    //----------------
    // Free the staging buffer
    if (vertexData_stagingBffer_position.vkBuffer)
    {
        vkDestroyBuffer(gVulkanContext.vkDevice, vertexData_stagingBffer_position.vkBuffer, NULL);
        vertexData_stagingBffer_position.vkBuffer = VK_NULL_HANDLE;
    }


    // Free the staging buffer memory
    if (vertexData_stagingBffer_position.vkDeviceMemory)
    {
        vkFreeMemory(gVulkanContext.vkDevice, vertexData_stagingBffer_position.vkDeviceMemory, NULL);
        vertexData_stagingBffer_position.vkDeviceMemory = VK_NULL_HANDLE;
    }

    //-----------------------------------------------------------------------------------
    // Now, vertexData_position.vkBuffer contains the device local buffer with the triangle position data
    // and vertexData_position.vkDeviceMemory contains the device local buffer memory.

    if (vkCommandBuffer_Copy)
    {
        vkFreeCommandBuffers(gVulkanContext.vkDevice, gVulkanContext.vkCommandPool, 1, &vkCommandBuffer_Copy);
        vkCommandBuffer_Copy = VK_NULL_HANDLE;
    }

    return vkResult;
}
VkResult AnimatedModel::ZzCreateIndex16Buffer(const uint16_t* indices, VkDeviceSize indexBufferSize, VulkanData* vulkanData)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    //position index buffer
//----------------------------------------------------------------------------------------------------
    memset((void*)vulkanData, 0, sizeof(VulkanData));

    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = indexBufferSize;
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    vkResult = vkCreateBuffer(gVulkanContext.vkDevice, &vkBufferCreateInfo, NULL, &vulkanData->vkBuffer);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer() (index):  failed.\n");
        return(vkResult);
    }

    //------------
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(vkMemoryRequirements));

    vkGetBufferMemoryRequirements(gVulkanContext.vkDevice, vulkanData->vkBuffer, &vkMemoryRequirements);

    //------------
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(vkMemoryAllocateInfo));

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;

    //-------------
    for (uint32_t i = 0; i < gVulkanContext.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if (gVulkanContext.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }

        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    //--------------
    vkResult = vkAllocateMemory(gVulkanContext.vkDevice, &vkMemoryAllocateInfo, NULL, &vulkanData->vkDeviceMemory);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  failed.\n");
        return(vkResult);
    }


    //---------------
    vkResult = vkBindBufferMemory(gVulkanContext.vkDevice, vulkanData->vkBuffer, vulkanData->vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
        return(vkResult);
    }


    //----------------
    void* dataIndex = NULL;
    vkResult = vkMapMemory(gVulkanContext.vkDevice, vulkanData->vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &dataIndex);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkMapMemory() :  failed.\n");
        return(vkResult);
    }


    //-------actual memory mapped io
    memcpy(dataIndex, indices, indexBufferSize);

    //-------unmap memory
    vkUnmapMemory(gVulkanContext.vkDevice, vulkanData->vkDeviceMemory);

    return vkResult;
}
VkResult AnimatedModel::ZzCreateIndex32Buffer(const uint32_t* indices, VkDeviceSize indexBufferSize, VulkanData* vulkanData)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    //position index buffer
//----------------------------------------------------------------------------------------------------
    memset((void*)vulkanData, 0, sizeof(VulkanData));

    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = indexBufferSize;
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    vkResult = vkCreateBuffer(gVulkanContext.vkDevice, &vkBufferCreateInfo, NULL, &vulkanData->vkBuffer);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer() (index):  failed.\n");
        return(vkResult);
    }

    //------------
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(vkMemoryRequirements));

    vkGetBufferMemoryRequirements(gVulkanContext.vkDevice, vulkanData->vkBuffer, &vkMemoryRequirements);

    //------------
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(vkMemoryAllocateInfo));

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;

    //-------------
    for (uint32_t i = 0; i < gVulkanContext.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if (gVulkanContext.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }

        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    //--------------
    vkResult = vkAllocateMemory(gVulkanContext.vkDevice, &vkMemoryAllocateInfo, NULL, &vulkanData->vkDeviceMemory);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  failed.\n");
        return(vkResult);
    }


    //---------------
    vkResult = vkBindBufferMemory(gVulkanContext.vkDevice, vulkanData->vkBuffer, vulkanData->vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
        return(vkResult);
    }


    //----------------
    void* dataIndex = NULL;
    vkResult = vkMapMemory(gVulkanContext.vkDevice, vulkanData->vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &dataIndex);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkMapMemory() :  failed.\n");
        return(vkResult);
    }


    //-------actual memory mapped io
    memcpy(dataIndex, indices, indexBufferSize);

    //-------unmap memory
    vkUnmapMemory(gVulkanContext.vkDevice, vulkanData->vkDeviceMemory);

    return vkResult;
}
VkResult AnimatedModel::ZzCreateVertexAndIndex16Buffer(
    const float* vertices,
    VkDeviceSize vertexBufferSize,
    const uint16_t* indices,
    VkDeviceSize indexBufferSize,
    VulkanComboData* vulkanComboData)
{
    // Create vertex buffer
    VkResult vkResult = ZzCreateVertexBuffer(vertices, vertexBufferSize, &vulkanComboData->vertexData);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "ZzCreateVertexAndIndexBuffer() -> ZzCreateVertexBuffer() failed.\n");
        return vkResult;
    }
    // Create index buffer
    vkResult = ZzCreateIndex16Buffer(indices, indexBufferSize, &vulkanComboData->indexData);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "ZzCreateVertexAndIndexBuffer() -> ZzCreateIndex16Buffer() failed.\n");
        return vkResult;
    }
    return vkResult;
}

VkResult AnimatedModel::ZzCreateVertexAndIndex32Buffer(
    const float* vertices,
    VkDeviceSize vertexBufferSize,
    const uint32_t* indices,
    VkDeviceSize indexBufferSize,
    VulkanComboData* vulkanComboData)
{
    // Create vertex buffer
    VkResult vkResult = ZzCreateVertexBuffer(vertices, vertexBufferSize, &vulkanComboData->vertexData);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "ZzCreateVertexAndIndexBuffer() -> ZzCreateVertexBuffer() failed.\n");
        return vkResult;
    }
    // Create index buffer
    vkResult = ZzCreateIndex32Buffer(indices, indexBufferSize, &vulkanComboData->indexData);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "ZzCreateVertexAndIndexBuffer() -> ZzCreateIndex16Buffer() failed.\n");
        return vkResult;
    }
    return vkResult;
}

void AnimatedModel::ZzDestroyVertexBuffer(VulkanData* vulkanData)
{

    // Destroy the vertex buffer
    if (vulkanData->vkBuffer)
    {
        vkDestroyBuffer(gVulkanContext.vkDevice, vulkanData->vkBuffer, NULL);
        vulkanData->vkBuffer = VK_NULL_HANDLE;
    }
    // Free the vertex buffer memory
    if (vulkanData->vkDeviceMemory)
    {
        vkFreeMemory(gVulkanContext.vkDevice, vulkanData->vkDeviceMemory, NULL);
        vulkanData->vkDeviceMemory = VK_NULL_HANDLE;
    }
}
void AnimatedModel::ZzDestroyIndexBuffer(VulkanData* vulkanData)
{
    // Destroy the index buffer
    if (vulkanData->vkBuffer)
    {
        vkDestroyBuffer(gVulkanContext.vkDevice, vulkanData->vkBuffer, NULL);
        vulkanData->vkBuffer = VK_NULL_HANDLE;
    }
    // Free the index buffer memory
    if (vulkanData->vkDeviceMemory)
    {
        vkFreeMemory(gVulkanContext.vkDevice, vulkanData->vkDeviceMemory, NULL);
        vulkanData->vkDeviceMemory = VK_NULL_HANDLE;
    }
}
void AnimatedModel::ZzDestroyVertexAndIndexBuffer(VulkanComboData* vulkanComboData)
{
    // Destroy vertex buffer
    ZzDestroyVertexBuffer(&vulkanComboData->vertexData);
    // Destroy index buffer
    ZzDestroyIndexBuffer(&vulkanComboData->indexData);
}
