#include "StaticModel.h"

#include <iostream>


StaticModel::StaticModel(const char* modelPath, bool index32, VkDescriptorSetLayout vkDescriptorSetLayout)
{
    vulkanComboData = {};

    VkResult vkResult = LoadModel_Static_PBR(modelPath, index32, vkDescriptorSetLayout);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE,"StaticModel::StaticModel() : ""LoadModel_Static_PBR() failed (%d).\n", vkResult);
    }
}

StaticModel::~StaticModel()
{
    ZzDestroyVertexAndIndexBuffer(&vulkanComboData);

    // PBR materials
    for (auto& matInfo : PBR_Materials)
    {
        if (matInfo.data)
        {
            delete matInfo.data;
            matInfo.data = nullptr;
        }
    }

    importer.FreeScene();

    scene = nullptr;
}

VkResult StaticModel::LoadModel_Static_PBR(
    const char* modelPath,
    bool index32,
    VkDescriptorSetLayout vkDescriptorSetLayout)
{
    VkResult vkResult = VK_SUCCESS;

    // ============================================================
    // Load model
    // ============================================================

    scene = importer.ReadFile(
        modelPath,

        aiProcess_Triangulate
        | aiProcess_JoinIdenticalVertices
        | aiProcess_GenSmoothNormals
        | aiProcess_CalcTangentSpace
        | aiProcess_ImproveCacheLocality
        | aiProcess_SortByPType
        | aiProcess_FlipWindingOrder
    );

    if (!scene ||
        (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
    {
        std::cerr
            << "Assimp Error: "
            << importer.GetErrorString()
            << std::endl;

        fprintf(
            gpFILE,
            "StaticModel::LoadModel_Static_PBR() : "
            "Assimp Importer::ReadFile() failed.\n");

        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // ============================================================
    // STATIC MODEL REQUIREMENT:
    // Currently this loader intentionally supports ONE mesh.
    // ============================================================

    if (scene->mNumMeshes != 1)
    {
        fprintf(
            gpFILE,
            "StaticModel::LoadModel_Static_PBR() : "
            "Expected exactly 1 mesh, found %u.\n",
            scene->mNumMeshes);

        return VK_ERROR_INITIALIZATION_FAILED;
    }

    aiMesh* mesh = scene->mMeshes[0];

    if (!mesh)
    {
        fprintf(
            gpFILE,
            "StaticModel::LoadModel_Static_PBR() : "
            "scene->mMeshes[0] is NULL.\n");

        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // ============================================================
    // Materials
    // ============================================================

    uint16_t numMaterials =
        static_cast<uint16_t>(
            scene->mNumMaterials);

    for (unsigned int i = 0;
        i < numMaterials;
        ++i)
    {
        const aiMaterial* mat =
            scene->mMaterials[i];

        aiString matName;

        std::string materialName =
            "(unnamed)";

        if (AI_SUCCESS ==
            mat->Get(
                AI_MATKEY_NAME,
                matName))
        {
            materialName =
                matName.C_Str();
        }

        aiString tex;

        if (AI_SUCCESS ==
            mat->GetTexture(
                aiTextureType_DIFFUSE,
                0,
                &tex))
        {
            StaticMaterialInfo info;

            std::string directory =
                std::filesystem::path(
                    tex.C_Str())
                .parent_path()
                .string();

            if (!directory.empty())
            {
                directory +=
                    std::filesystem::path::
                    preferred_separator;
            }

            info.materialName =
                materialName;

            info.path =
                directory;

            info.data =
                new Material_BasicPBR(
                    vkDescriptorSetLayout,
                    directory.c_str());

            PBR_Materials.push_back(
                info);
        }
    }

    // ============================================================
    // Vertex data
    // ============================================================

    std::vector<VertexData_PositionTexCoordNormalTangent> vkVertices;

    vkVertices.reserve(
        mesh->mNumVertices);

    // ============================================================
    // 32-bit indices
    // ============================================================

    if (index32)
    {
        std::vector<uint32_t> vkIndices;

        vkIndices.reserve(
            mesh->mNumFaces * 3);

        // --------------------------------------------------------
        // Vertices
        // --------------------------------------------------------

        for (unsigned int i = 0;
            i < mesh->mNumVertices;
            ++i)
        {
            VertexData_PositionTexCoordNormalTangent v{};

            // ----------------------------------------------------
            // Position
            // ----------------------------------------------------

            if (mesh->HasPositions())
            {
                v.pos =
                    glm::vec3(
                        mesh->mVertices[i].x,
                        mesh->mVertices[i].y,
                        mesh->mVertices[i].z);
            }

            // ----------------------------------------------------
            // Normal
            // ----------------------------------------------------

            if (mesh->HasNormals())
            {
                v.normal =
                    glm::vec3(
                        mesh->mNormals[i].x,
                        mesh->mNormals[i].y,
                        mesh->mNormals[i].z);
            }
            else
            {
                v.normal =
                    glm::vec3(
                        0.0f,
                        0.0f,
                        1.0f);
            }

            // ----------------------------------------------------
            // UV
            // ----------------------------------------------------

            if (mesh->HasTextureCoords(0))
            {
                v.texCoord =
                    glm::vec2(
                        mesh->mTextureCoords[0][i].x,
                        mesh->mTextureCoords[0][i].y);
            }
            else
            {
                v.texCoord =
                    glm::vec2(0.0f);
            }

            // ----------------------------------------------------
            // Tangent
            // ----------------------------------------------------

            if (mesh->HasTangentsAndBitangents())
            {
                glm::vec3 tangent(
                    mesh->mTangents[i].x,
                    mesh->mTangents[i].y,
                    mesh->mTangents[i].z);

                glm::vec3 bitangent(
                    mesh->mBitangents[i].x,
                    mesh->mBitangents[i].y,
                    mesh->mBitangents[i].z);

                glm::vec3 normal =
                    glm::normalize(v.normal);

                tangent =
                    glm::normalize(tangent);

                float handedness =
                    (glm::dot(
                        glm::cross(
                            tangent,
                            normal),
                        bitangent) < 0.0f)
                    ? -1.0f
                    : 1.0f;

                v.tangent =
                    glm::vec4(
                        tangent,
                        handedness);
            }
            else
            {
                v.tangent =
                    glm::vec4(
                        1.0f,
                        0.0f,
                        0.0f,
                        1.0f);
            }

            vkVertices.push_back(v);
        }

        // --------------------------------------------------------
        // Indices
        //
        // IMPORTANT:
        // Single mesh => Assimp's indices are already correct.
        // No vertex offset is required.
        // --------------------------------------------------------

        for (unsigned int f = 0;
            f < mesh->mNumFaces;
            ++f)
        {
            const aiFace& face =
                mesh->mFaces[f];

            for (unsigned int j = 0;
                j < face.mNumIndices;
                ++j)
            {
                vkIndices.push_back(
                    static_cast<uint32_t>(
                        face.mIndices[j]));
            }
        }

        // --------------------------------------------------------
        // Index count
        // --------------------------------------------------------

        vulkanComboData.indicesCount =
            static_cast<uint32_t>(
                vkIndices.size());

        // --------------------------------------------------------
        // Create Vulkan buffers
        // --------------------------------------------------------

        vkResult =
            ZzCreateVertexAndIndex32Buffer(
                reinterpret_cast<const float*>(
                    vkVertices.data()),

                vkVertices.size() *
                sizeof(VertexData_PositionTexCoordNormalTangent),

                vkIndices.data(),

                vkIndices.size() *
                sizeof(uint32_t),

                &vulkanComboData);

        if (vkResult != VK_SUCCESS)
        {
            fprintf(
                gpFILE,
                "StaticModel::LoadModel_Static_PBR() : "
                "ZzCreateVertexAndIndex32Buffer() failed (%d).\n",
                vkResult);

            return vkResult;
        }
    }

    // ============================================================
    // 16-bit indices
    // ============================================================

    else
    {
        std::vector<uint16_t> vkIndices;

        vkIndices.reserve(
            mesh->mNumFaces * 3);

        // --------------------------------------------------------
        // Make sure the mesh can fit in uint16 indices
        // --------------------------------------------------------

        if (mesh->mNumVertices >
            (std::numeric_limits<uint16_t>::max)() + 1u)
        {
            fprintf(
                gpFILE,
                "StaticModel::LoadModel_Static_PBR() : "
                "16-bit index overflow. "
                "Mesh has %u vertices. "
                "Use index32=true.\n",
                mesh->mNumVertices);

            return VK_ERROR_FEATURE_NOT_PRESENT;
        }

        // --------------------------------------------------------
        // Vertices
        // --------------------------------------------------------

        for (unsigned int i = 0;
            i < mesh->mNumVertices;
            ++i)
        {
            VertexData_PositionTexCoordNormalTangent v{};

            // ----------------------------------------------------
            // Position
            // ----------------------------------------------------

            if (mesh->HasPositions())
            {
                v.pos =
                    glm::vec3(
                        mesh->mVertices[i].x,
                        mesh->mVertices[i].y,
                        mesh->mVertices[i].z);
            }

            // ----------------------------------------------------
            // Normal
            // ----------------------------------------------------

            if (mesh->HasNormals())
            {
                v.normal =
                    glm::vec3(
                        mesh->mNormals[i].x,
                        mesh->mNormals[i].y,
                        mesh->mNormals[i].z);
            }
            else
            {
                v.normal =
                    glm::vec3(
                        0.0f,
                        0.0f,
                        1.0f);
            }

            // ----------------------------------------------------
            // UV
            // ----------------------------------------------------

            if (mesh->HasTextureCoords(0))
            {
                v.texCoord =
                    glm::vec2(
                        mesh->mTextureCoords[0][i].x,
                        mesh->mTextureCoords[0][i].y);
            }
            else
            {
                v.texCoord =
                    glm::vec2(0.0f);
            }

            // ----------------------------------------------------
            // Tangent
            // ----------------------------------------------------

            if (mesh->HasTangentsAndBitangents())
            {
                glm::vec3 tangent(
                    mesh->mTangents[i].x,
                    mesh->mTangents[i].y,
                    mesh->mTangents[i].z);

                glm::vec3 bitangent(
                    mesh->mBitangents[i].x,
                    mesh->mBitangents[i].y,
                    mesh->mBitangents[i].z);

                glm::vec3 normal =
                    glm::normalize(v.normal);

                tangent =
                    glm::normalize(tangent);

                float handedness =
                    (glm::dot(
                        glm::cross(
                            tangent,
                            normal),
                        bitangent) < 0.0f)
                    ? -1.0f
                    : 1.0f;

                v.tangent =
                    glm::vec4(
                        tangent,
                        handedness);
            }
            else
            {
                v.tangent =
                    glm::vec4(
                        1.0f,
                        0.0f,
                        0.0f,
                        1.0f);
            }

            vkVertices.push_back(v);
        }

        // --------------------------------------------------------
        // Indices
        //
        // IMPORTANT:
        // Single mesh => Assimp's indices are already correct.
        // No vertex offset is required.
        // --------------------------------------------------------

        for (unsigned int f = 0;
            f < mesh->mNumFaces;
            ++f)
        {
            const aiFace& face =
                mesh->mFaces[f];

            for (unsigned int j = 0;
                j < face.mNumIndices;
                ++j)
            {
                uint32_t index =
                    face.mIndices[j];

                if (index >
                    (std::numeric_limits<uint16_t>::max)())
                {
                    fprintf(
                        gpFILE,
                        "StaticModel::LoadModel_Static_PBR() : "
                        "16-bit index overflow. "
                        "Use index32=true.\n");

                    return VK_ERROR_FEATURE_NOT_PRESENT;
                }

                vkIndices.push_back(
                    static_cast<uint16_t>(
                        index));
            }
        }

        // --------------------------------------------------------
        // Index count
        // --------------------------------------------------------

        vulkanComboData.indicesCount =
            static_cast<uint32_t>(
                vkIndices.size());

        // --------------------------------------------------------
        // Create Vulkan buffers
        // --------------------------------------------------------

        vkResult =
            ZzCreateVertexAndIndex16Buffer(
                reinterpret_cast<const float*>(
                    vkVertices.data()),

                vkVertices.size() *
                sizeof(VertexData_PositionTexCoordNormalTangent),

                vkIndices.data(),

                vkIndices.size() *
                sizeof(uint16_t),

                &vulkanComboData);

        if (vkResult != VK_SUCCESS)
        {
            fprintf(
                gpFILE,
                "StaticModel::LoadModel_Static_PBR() : "
                "ZzCreateVertexAndIndex16Buffer() failed (%d).\n",
                vkResult);

            return vkResult;
        }
    }

    // ============================================================
    // Done
    // ============================================================

    fprintf(
        gpFILE,
        "StaticModel::LoadModel_Static_PBR() : "
        "Loaded static model successfully. "
        "Meshes=%u Vertices=%u Faces=%u Indices=%u.\n",
        scene->mNumMeshes,
        mesh->mNumVertices,
        mesh->mNumFaces,
        vulkanComboData.indicesCount);

    return VK_SUCCESS;
}

VkResult StaticModel::ZzCreateVertexBuffer(
    const float* vertices,
    VkDeviceSize vertexBufferSize,
    VulkanData* vulkanData)
{
    VkResult vkResult = VK_SUCCESS;


    // ============================================================
    // Staging buffer
    // ============================================================

    VulkanData stagingBuffer{};


    VkBufferCreateInfo stagingCreateInfo{};
    stagingCreateInfo.sType =
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    stagingCreateInfo.size =
        vertexBufferSize;

    stagingCreateInfo.usage =
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    stagingCreateInfo.sharingMode =
        VK_SHARING_MODE_EXCLUSIVE;


    vkResult =
        vkCreateBuffer(
            gVulkanContext.vkDevice,
            &stagingCreateInfo,
            nullptr,
            &stagingBuffer.vkBuffer);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    // ============================================================
    // Staging memory
    // ============================================================

    VkMemoryRequirements stagingMemReq{};

    vkGetBufferMemoryRequirements(
        gVulkanContext.vkDevice,
        stagingBuffer.vkBuffer,
        &stagingMemReq);


    VkMemoryAllocateInfo stagingAllocInfo{};

    stagingAllocInfo.sType =
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    stagingAllocInfo.allocationSize =
        stagingMemReq.size;


    stagingAllocInfo.memoryTypeIndex =
        gVulkanContext.FindMemoryType(
            stagingMemReq.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


    vkResult =
        vkAllocateMemory(
            gVulkanContext.vkDevice,
            &stagingAllocInfo,
            nullptr,
            &stagingBuffer.vkDeviceMemory);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    vkResult =
        vkBindBufferMemory(
            gVulkanContext.vkDevice,
            stagingBuffer.vkBuffer,
            stagingBuffer.vkDeviceMemory,
            0);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    // ============================================================
    // Upload CPU data to staging buffer
    // ============================================================

    void* data = nullptr;

    vkResult =
        vkMapMemory(
            gVulkanContext.vkDevice,
            stagingBuffer.vkDeviceMemory,
            0,
            vertexBufferSize,
            0,
            &data);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    memcpy(
        data,
        vertices,
        static_cast<size_t>(vertexBufferSize));


    vkUnmapMemory(
        gVulkanContext.vkDevice,
        stagingBuffer.vkDeviceMemory);


    // ============================================================
    // Device-local vertex buffer
    // ============================================================

    VkBufferCreateInfo deviceCreateInfo{};

    deviceCreateInfo.sType =
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    deviceCreateInfo.size =
        vertexBufferSize;

    deviceCreateInfo.usage =
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    deviceCreateInfo.sharingMode =
        VK_SHARING_MODE_EXCLUSIVE;


    vkResult =
        vkCreateBuffer(
            gVulkanContext.vkDevice,
            &deviceCreateInfo,
            nullptr,
            &vulkanData->vkBuffer);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    VkMemoryRequirements deviceMemReq{};

    vkGetBufferMemoryRequirements(
        gVulkanContext.vkDevice,
        vulkanData->vkBuffer,
        &deviceMemReq);


    VkMemoryAllocateInfo deviceAllocInfo{};

    deviceAllocInfo.sType =
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    deviceAllocInfo.allocationSize =
        deviceMemReq.size;

    deviceAllocInfo.memoryTypeIndex =
        gVulkanContext.FindMemoryType(
            deviceMemReq.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


    vkResult =
        vkAllocateMemory(
            gVulkanContext.vkDevice,
            &deviceAllocInfo,
            nullptr,
            &vulkanData->vkDeviceMemory);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    vkResult =
        vkBindBufferMemory(
            gVulkanContext.vkDevice,
            vulkanData->vkBuffer,
            vulkanData->vkDeviceMemory,
            0);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    // ============================================================
    // Copy staging -> device local
    // ============================================================

    VkCommandBufferAllocateInfo cmdAllocInfo{};

    cmdAllocInfo.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    cmdAllocInfo.commandPool =
        gVulkanContext.vkCommandPool;

    cmdAllocInfo.level =
        VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    cmdAllocInfo.commandBufferCount = 1;


    VkCommandBuffer commandBuffer =
        VK_NULL_HANDLE;


    vkResult =
        vkAllocateCommandBuffers(
            gVulkanContext.vkDevice,
            &cmdAllocInfo,
            &commandBuffer);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    VkCommandBufferBeginInfo beginInfo{};

    beginInfo.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    beginInfo.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;


    vkResult =
        vkBeginCommandBuffer(
            commandBuffer,
            &beginInfo);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    VkBufferCopy copyRegion{};

    copyRegion.size =
        vertexBufferSize;


    vkCmdCopyBuffer(
        commandBuffer,
        stagingBuffer.vkBuffer,
        vulkanData->vkBuffer,
        1,
        &copyRegion);


    vkResult =
        vkEndCommandBuffer(
            commandBuffer);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    VkSubmitInfo submitInfo{};

    submitInfo.sType =
        VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.commandBufferCount = 1;

    submitInfo.pCommandBuffers =
        &commandBuffer;


    vkResult =
        vkQueueSubmit(
            gVulkanContext.vkGraphicsQueue,
            1,
            &submitInfo,
            VK_NULL_HANDLE);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    vkResult =
        vkQueueWaitIdle(
            gVulkanContext.vkGraphicsQueue);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    // ============================================================
    // Cleanup temporary resources
    // ============================================================

    vkFreeCommandBuffers(
        gVulkanContext.vkDevice,
        gVulkanContext.vkCommandPool,
        1,
        &commandBuffer);


    vkDestroyBuffer(
        gVulkanContext.vkDevice,
        stagingBuffer.vkBuffer,
        nullptr);


    vkFreeMemory(
        gVulkanContext.vkDevice,
        stagingBuffer.vkDeviceMemory,
        nullptr);


    return VK_SUCCESS;
}

VkResult StaticModel::ZzCreateIndex16Buffer(
    const uint16_t* indices,
    VkDeviceSize indexBufferSize,
    VulkanData* vulkanData)
{
    VkResult vkResult = VK_SUCCESS;


    VkBufferCreateInfo bufferInfo{};

    bufferInfo.sType =
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    bufferInfo.size =
        indexBufferSize;

    bufferInfo.usage =
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    bufferInfo.sharingMode =
        VK_SHARING_MODE_EXCLUSIVE;


    vkResult =
        vkCreateBuffer(
            gVulkanContext.vkDevice,
            &bufferInfo,
            nullptr,
            &vulkanData->vkBuffer);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    VkMemoryRequirements memReq{};

    vkGetBufferMemoryRequirements(
        gVulkanContext.vkDevice,
        vulkanData->vkBuffer,
        &memReq);


    VkMemoryAllocateInfo allocInfo{};

    allocInfo.sType =
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    allocInfo.allocationSize =
        memReq.size;

    allocInfo.memoryTypeIndex =
        gVulkanContext.FindMemoryType(
            memReq.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


    vkResult =
        vkAllocateMemory(
            gVulkanContext.vkDevice,
            &allocInfo,
            nullptr,
            &vulkanData->vkDeviceMemory);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    vkResult =
        vkBindBufferMemory(
            gVulkanContext.vkDevice,
            vulkanData->vkBuffer,
            vulkanData->vkDeviceMemory,
            0);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    void* mapped = nullptr;

    vkResult =
        vkMapMemory(
            gVulkanContext.vkDevice,
            vulkanData->vkDeviceMemory,
            0,
            indexBufferSize,
            0,
            &mapped);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    memcpy(
        mapped,
        indices,
        static_cast<size_t>(indexBufferSize));


    vkUnmapMemory(
        gVulkanContext.vkDevice,
        vulkanData->vkDeviceMemory);


    return VK_SUCCESS;
}

VkResult StaticModel::ZzCreateIndex32Buffer(
    const uint32_t* indices,
    VkDeviceSize indexBufferSize,
    VulkanData* vulkanData)
{
    VkResult vkResult = VK_SUCCESS;



    VkBufferCreateInfo bufferInfo{};

    bufferInfo.sType =
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    bufferInfo.size =
        indexBufferSize;

    bufferInfo.usage =
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    bufferInfo.sharingMode =
        VK_SHARING_MODE_EXCLUSIVE;


    vkResult =
        vkCreateBuffer(
            gVulkanContext.vkDevice,
            &bufferInfo,
            nullptr,
            &vulkanData->vkBuffer);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    VkMemoryRequirements memReq{};

    vkGetBufferMemoryRequirements(
        gVulkanContext.vkDevice,
        vulkanData->vkBuffer,
        &memReq);


    VkMemoryAllocateInfo allocInfo{};

    allocInfo.sType =
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    allocInfo.allocationSize =
        memReq.size;

    allocInfo.memoryTypeIndex =
        gVulkanContext.FindMemoryType(
            memReq.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


    vkResult =
        vkAllocateMemory(
            gVulkanContext.vkDevice,
            &allocInfo,
            nullptr,
            &vulkanData->vkDeviceMemory);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    vkResult =
        vkBindBufferMemory(
            gVulkanContext.vkDevice,
            vulkanData->vkBuffer,
            vulkanData->vkDeviceMemory,
            0);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    void* mapped = nullptr;

    vkResult =
        vkMapMemory(
            gVulkanContext.vkDevice,
            vulkanData->vkDeviceMemory,
            0,
            indexBufferSize,
            0,
            &mapped);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    memcpy(
        mapped,
        indices,
        static_cast<size_t>(indexBufferSize));


    vkUnmapMemory(
        gVulkanContext.vkDevice,
        vulkanData->vkDeviceMemory);


    return VK_SUCCESS;
}

VkResult StaticModel::ZzCreateVertexAndIndex16Buffer(
    const float* vertices,
    VkDeviceSize vertexBufferSize,
    const uint16_t* indices,
    VkDeviceSize indexBufferSize,
    VulkanComboData* vulkanComboData)
{
    VkResult vkResult =
        ZzCreateVertexBuffer(
            vertices,
            vertexBufferSize,
            &vulkanComboData->vertexData);


    if (vkResult != VK_SUCCESS)
    {
        fprintf(
            gpFILE,
            "StaticModel::ZzCreateVertexAndIndex16Buffer() : "
            "ZzCreateVertexBuffer() failed.\n");

        return vkResult;
    }


    vkResult =
        ZzCreateIndex16Buffer(
            indices,
            indexBufferSize,
            &vulkanComboData->indexData);


    if (vkResult != VK_SUCCESS)
    {
        fprintf(
            gpFILE,
            "StaticModel::ZzCreateVertexAndIndex16Buffer() : "
            "ZzCreateIndex16Buffer() failed.\n");

        return vkResult;
    }


    return VK_SUCCESS;
}

VkResult StaticModel::ZzCreateVertexAndIndex32Buffer(
    const float* vertices,
    VkDeviceSize vertexBufferSize,
    const uint32_t* indices,
    VkDeviceSize indexBufferSize,
    VulkanComboData* vulkanComboData)
{
    VkResult vkResult =
        ZzCreateVertexBuffer(
            vertices,
            vertexBufferSize,
            &vulkanComboData->vertexData);


    if (vkResult != VK_SUCCESS)
    {
        fprintf(
            gpFILE,
            "StaticModel::ZzCreateVertexAndIndex32Buffer() : "
            "ZzCreateVertexBuffer() failed.\n");

        return vkResult;
    }


    vkResult =
        ZzCreateIndex32Buffer(
            indices,
            indexBufferSize,
            &vulkanComboData->indexData);


    if (vkResult != VK_SUCCESS)
    {
        fprintf(
            gpFILE,
            "StaticModel::ZzCreateVertexAndIndex32Buffer() : "
            "ZzCreateIndex32Buffer() failed.\n");

        return vkResult;
    }


    return VK_SUCCESS;
}

void StaticModel::ZzDestroyVertexBuffer(
    VulkanData* vulkanData)
{
    if (vulkanData->vkBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(
            gVulkanContext.vkDevice,
            vulkanData->vkBuffer,
            nullptr);

        vulkanData->vkBuffer =
            VK_NULL_HANDLE;
    }


    if (vulkanData->vkDeviceMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(
            gVulkanContext.vkDevice,
            vulkanData->vkDeviceMemory,
            nullptr);

        vulkanData->vkDeviceMemory =
            VK_NULL_HANDLE;
    }
}

void StaticModel::ZzDestroyIndexBuffer(
    VulkanData* vulkanData)
{
    if (vulkanData->vkBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(
            gVulkanContext.vkDevice,
            vulkanData->vkBuffer,
            nullptr);

        vulkanData->vkBuffer =
            VK_NULL_HANDLE;
    }


    if (vulkanData->vkDeviceMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(
            gVulkanContext.vkDevice,
            vulkanData->vkDeviceMemory,
            nullptr);

        vulkanData->vkDeviceMemory =
            VK_NULL_HANDLE;
    }
}

void StaticModel::ZzDestroyVertexAndIndexBuffer(
    VulkanComboData* vulkanComboData)
{
    ZzDestroyVertexBuffer(
        &vulkanComboData->vertexData);

    ZzDestroyIndexBuffer(
        &vulkanComboData->indexData);
}