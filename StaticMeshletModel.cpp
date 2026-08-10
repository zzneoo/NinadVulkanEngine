#include "StaticMeshletModel.h"

#include <iostream>


StaticMeshletModel::StaticMeshletModel(const char* modelPath, VkDescriptorSetLayout vkDescriptorSetLayout)
{


    VkResult vkResult = LoadModel_Static_PBR(modelPath, vkDescriptorSetLayout);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE,"StaticMeshletModel::StaticMeshletModel() : ""LoadModel_Static_PBR() failed (%d).\n", vkResult);
    }
}

StaticMeshletModel::~StaticMeshletModel()
{


    DestroyMeshletBuffers();

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

VkResult StaticMeshletModel::LoadModel_Static_PBR(
    const char* modelPath,
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
            "StaticMeshletModel::LoadModel_Static_PBR() : "
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
            "StaticMeshletModel::LoadModel_Static_PBR() : "
            "Expected exactly 1 mesh, found %u.\n",
            scene->mNumMeshes);

        return VK_ERROR_INITIALIZATION_FAILED;
    }

    aiMesh* mesh = scene->mMeshes[0];

    if (!mesh)
    {
        fprintf(
            gpFILE,
            "StaticMeshletModel::LoadModel_Static_PBR() : "
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
            MaterialInfo info;

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

        
        vkResult = CreateMeshlets(vkIndices, vkVertices);

        if (vkResult != VK_SUCCESS)
        {
            return vkResult;
        }

    }



    // ============================================================
    // Done
    // ============================================================



    return VK_SUCCESS;
}



VkResult StaticMeshletModel::CreateDeviceLocalSSBO(
    const void* data,
    VkDeviceSize size,
    VulkanSSBO& outSSBO)
{
    VkResult vkResult = VK_SUCCESS;

    outSSBO = {};
    outSSBO.size = size;

    // ============================================================
    // 1. Create staging buffer
    // ============================================================

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

    VkBufferCreateInfo stagingCreateInfo{};

    stagingCreateInfo.sType =
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    stagingCreateInfo.size =
        size;

    stagingCreateInfo.usage =
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    stagingCreateInfo.sharingMode =
        VK_SHARING_MODE_EXCLUSIVE;


    vkResult =
        vkCreateBuffer(
            gVulkanContext.vkDevice,
            &stagingCreateInfo,
            nullptr,
            &stagingBuffer);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    VkMemoryRequirements stagingMemReq{};

    vkGetBufferMemoryRequirements(
        gVulkanContext.vkDevice,
        stagingBuffer,
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
            &stagingMemory);

    if (vkResult != VK_SUCCESS)
    {
        vkDestroyBuffer(
            gVulkanContext.vkDevice,
            stagingBuffer,
            nullptr);

        return vkResult;
    }


    vkResult =
        vkBindBufferMemory(
            gVulkanContext.vkDevice,
            stagingBuffer,
            stagingMemory,
            0);

    if (vkResult != VK_SUCCESS)
    {
        vkFreeMemory(
            gVulkanContext.vkDevice,
            stagingMemory,
            nullptr);

        vkDestroyBuffer(
            gVulkanContext.vkDevice,
            stagingBuffer,
            nullptr);

        return vkResult;
    }


    // ============================================================
    // 2. Copy CPU data to staging buffer
    // ============================================================

    void* mapped = nullptr;

    vkResult =
        vkMapMemory(
            gVulkanContext.vkDevice,
            stagingMemory,
            0,
            size,
            0,
            &mapped);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    memcpy(
        mapped,
        data,
        static_cast<size_t>(size));

    vkUnmapMemory(
        gVulkanContext.vkDevice,
        stagingMemory);


    // ============================================================
    // 3. Create DEVICE LOCAL SSBO
    // ============================================================

    VkBufferCreateInfo bufferCreateInfo{};

    bufferCreateInfo.sType =
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

    bufferCreateInfo.size =
        size;

    bufferCreateInfo.usage =
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
        VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    bufferCreateInfo.sharingMode =
        VK_SHARING_MODE_EXCLUSIVE;


    vkResult =
        vkCreateBuffer(
            gVulkanContext.vkDevice,
            &bufferCreateInfo,
            nullptr,
            &outSSBO.vkBuffer);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    VkMemoryRequirements memReq{};

    vkGetBufferMemoryRequirements(
        gVulkanContext.vkDevice,
        outSSBO.vkBuffer,
        &memReq);


    VkMemoryAllocateInfo allocInfo{};

    allocInfo.sType =
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

    allocInfo.allocationSize =
        memReq.size;

    allocInfo.memoryTypeIndex =
        gVulkanContext.FindMemoryType(
            memReq.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);


    vkResult =
        vkAllocateMemory(
            gVulkanContext.vkDevice,
            &allocInfo,
            nullptr,
            &outSSBO.vkDeviceMemory);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    vkResult =
        vkBindBufferMemory(
            gVulkanContext.vkDevice,
            outSSBO.vkBuffer,
            outSSBO.vkDeviceMemory,
            0);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    // ============================================================
    // 4. Copy staging to DEVICE LOCAL
    // ============================================================

    VkCommandBufferAllocateInfo cmdAllocInfo{};

    cmdAllocInfo.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    cmdAllocInfo.commandPool =
        gVulkanContext.vkCommandPool;

    cmdAllocInfo.level =
        VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    cmdAllocInfo.commandBufferCount =
        1;


    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

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

    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;


    vkCmdCopyBuffer(
        commandBuffer,
        stagingBuffer,
        outSSBO.vkBuffer,
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

    submitInfo.commandBufferCount =
        1;

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
    // 5. Descriptor information
    // ============================================================

    outSSBO.descriptor.buffer =
        outSSBO.vkBuffer;

    outSSBO.descriptor.offset =
        0;

    outSSBO.descriptor.range =
        outSSBO.size;


    // ============================================================
    // 6. Destroy staging resources
    // ============================================================

    vkFreeCommandBuffers(
        gVulkanContext.vkDevice,
        gVulkanContext.vkCommandPool,
        1,
        &commandBuffer);


    vkDestroyBuffer(
        gVulkanContext.vkDevice,
        stagingBuffer,
        nullptr);


    vkFreeMemory(
        gVulkanContext.vkDevice,
        stagingMemory,
        nullptr);


    return VK_SUCCESS;
}


VkResult StaticMeshletModel::CreateMeshletSSBOs(
    const std::vector<VertexData_PositionTexCoordNormalTangent>& vertices)
{
    VkResult vkResult = VK_SUCCESS;


    // ============================================================
    // Vertex data
    // ============================================================

    vkResult =
        CreateDeviceLocalSSBO(
            vertices.data(),
            vertices.size() *
            sizeof(VertexData_PositionTexCoordNormalTangent),
            meshletGPUData.vertexBuffer);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    meshletGPUData.vertexCount =
        static_cast<uint32_t>(
            vertices.size());


    // ============================================================
    // Meshlet metadata
    // ============================================================

    vkResult =
        CreateDeviceLocalSSBO(
            meshlets.data(),
            meshlets.size() *
            sizeof(MeshletData),
            meshletGPUData.meshletBuffer);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    // ============================================================
    // Meshlet to original vertex references
    // ============================================================

    vkResult =
        CreateDeviceLocalSSBO(
            meshletVertices.data(),
            meshletVertices.size() *
            sizeof(uint32_t),
            meshletGPUData.meshletVertexBuffer);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    // ============================================================
    // Local triangle indices
    // ============================================================

    vkResult =
        CreateDeviceLocalSSBO(
            meshletTriangles.data(),
            meshletTriangles.size() *
            sizeof(uint8_t),
            meshletGPUData.meshletTriangleBuffer);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    meshletGPUData.meshletCount =
        static_cast<uint32_t>(
            meshlets.size());


    fprintf(
        gpFILE,
        "StaticMeshletModel::CreateMeshletSSBOs() : "
        "Created device-local meshlet buffers. "
        "Vertices=%u Meshlets=%u.\n",
        meshletGPUData.vertexCount,
        meshletGPUData.meshletCount);


    return VK_SUCCESS;
}

VkResult StaticMeshletModel::CreateMeshlets(
    const std::vector<uint32_t>& indices,
    const std::vector<VertexData_PositionTexCoordNormalTangent>& vertices)
{
    // ============================================================
    // Meshlet limits
    // ============================================================

    constexpr size_t MAX_MESHLET_VERTICES = 64;
    constexpr size_t MAX_MESHLET_TRIANGLES = 126;


    if (indices.empty())
    {
        fprintf(
            gpFILE,
            "StaticMeshletModel::CreateMeshlets() : "
            "Index array is empty.\n");

        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (vertices.empty())
    {
        fprintf(
            gpFILE,
            "StaticMeshletModel::CreateMeshlets() : "
            "Vertex array is empty.\n");

        return VK_ERROR_INITIALIZATION_FAILED;
    }


    // ============================================================
    // Maximum number of meshlets required
    // ============================================================

    size_t maxMeshlets =
        meshopt_buildMeshletsBound(
            indices.size(),
            MAX_MESHLET_VERTICES,
            MAX_MESHLET_TRIANGLES);


    if (maxMeshlets == 0)
    {
        fprintf(
            gpFILE,
            "StaticMeshletModel::CreateMeshlets() : "
            "meshopt_buildMeshletsBound() returned 0.\n");

        return VK_ERROR_INITIALIZATION_FAILED;
    }


    // ============================================================
    // Temporary meshlet storage
    // ============================================================

    std::vector<meshopt_Meshlet> tempMeshlets(
        maxMeshlets);

    std::vector<uint32_t> tempMeshletVertices(
        maxMeshlets *
        MAX_MESHLET_VERTICES);

    std::vector<unsigned char> tempMeshletTriangles(
        maxMeshlets *
        MAX_MESHLET_TRIANGLES *
        3);


    // ============================================================
    // Build meshlets
    // ============================================================

    size_t meshletCount =
        meshopt_buildMeshlets(
            tempMeshlets.data(),

            tempMeshletVertices.data(),

            tempMeshletTriangles.data(),

            indices.data(),
            indices.size(),

            &vertices[0].pos.x,
            vertices.size(),

            sizeof(
                VertexData_PositionTexCoordNormalTangent),

            MAX_MESHLET_VERTICES,
            MAX_MESHLET_TRIANGLES,

            0.0f);


    if (meshletCount == 0)
    {
        fprintf(
            gpFILE,
            "StaticMeshletModel::CreateMeshlets() : "
            "meshopt_buildMeshlets() returned 0.\n");

        return VK_ERROR_INITIALIZATION_FAILED;
    }


    fprintf(
        gpFILE,
        "StaticMeshletModel::CreateMeshlets() : "
        "Generated %zu meshlets.\n",
        meshletCount);


    tempMeshlets.resize(meshletCount);


    // ============================================================
    // Optimize each meshlet
    // ============================================================

    for (const meshopt_Meshlet& meshlet :
        tempMeshlets)
    {
        meshopt_optimizeMeshlet(
            tempMeshletVertices.data() +
            meshlet.vertex_offset,

            tempMeshletTriangles.data() +
            meshlet.triangle_offset,

            meshlet.triangle_count,
            meshlet.vertex_count);
    }


    // ============================================================
    // Determine actual used ranges
    // ============================================================

    size_t totalMeshletVertices = 0;
    size_t totalMeshletTriangles = 0;

    for (const meshopt_Meshlet& meshlet :
        tempMeshlets)
    {
        const size_t vertexEnd =
            static_cast<size_t>(
                meshlet.vertex_offset +
                meshlet.vertex_count);

        const size_t triangleEnd =
            static_cast<size_t>(
                meshlet.triangle_offset +
                meshlet.triangle_count * 3);

        totalMeshletVertices =
            std::max(
                totalMeshletVertices,
                vertexEnd);

        totalMeshletTriangles =
            std::max(
                totalMeshletTriangles,
                triangleEnd);
    }


    // ============================================================
    // Copy final meshlet data into class-owned arrays
    // ============================================================

    meshlets.clear();
    meshletVertices.clear();
    meshletTriangles.clear();


    meshlets.reserve(meshletCount);

    meshletVertices.reserve(
        totalMeshletVertices);

    meshletTriangles.reserve(
        totalMeshletTriangles);


    // ------------------------------------------------------------
    // Meshlet metadata
    // ------------------------------------------------------------

    for (const meshopt_Meshlet& src :
        tempMeshlets)
    {
        MeshletData dst{};

        dst.vertexOffset =
            static_cast<uint32_t>(
                src.vertex_offset);

        dst.triangleOffset =
            static_cast<uint32_t>(
                src.triangle_offset);

        dst.vertexCount =
            static_cast<uint32_t>(
                src.vertex_count);

        dst.triangleCount =
            static_cast<uint32_t>(
                src.triangle_count);

        meshlets.push_back(dst);
    }


    // ------------------------------------------------------------
    // Meshlet vertex references
    // ------------------------------------------------------------

    meshletVertices.assign(
        tempMeshletVertices.begin(),
        tempMeshletVertices.begin() +
        totalMeshletVertices);


    // ------------------------------------------------------------
    // Meshlet triangle indices
    // ------------------------------------------------------------

    meshletTriangles.assign(
        tempMeshletTriangles.begin(),
        tempMeshletTriangles.begin() +
        totalMeshletTriangles);


    // ============================================================
    // Log generated data
    // ============================================================

    fprintf(
        gpFILE,
        "StaticMeshletModel::CreateMeshlets() : "
        "Vertices=%zu, Triangles=%zu.\n",
        meshletVertices.size(),
        meshletTriangles.size());


    // ============================================================
    // Create device-local SSBOs
    // ============================================================

    VkResult vkResult =
        CreateMeshletSSBOs(vertices);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(
            gpFILE,
            "StaticMeshletModel::CreateMeshlets() : "
            "CreateMeshletSSBOs() failed (%d).\n",
            vkResult);

        return vkResult;
    }


    fprintf(
        gpFILE,
        "StaticMeshletModel::CreateMeshlets() : "
        "Meshlet generation and GPU upload completed.\n");



    //update descriptor
    vkResult = createDescriptorSet_Meshlet();

    if (vkResult != VK_SUCCESS)
    {
        fprintf(
            gpFILE,
            "StaticMeshletModel::LoadModel_Static_PBR() : "
            "createDescriptorSet_Meshlet() failed.\n");

        return vkResult;
    }


    return VK_SUCCESS;
}

VkResult StaticMeshletModel::createDescriptorSet_Meshlet(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;


    // ============================================================
    // Allocate descriptor set
    // ============================================================

    VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo{};
    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool =vkDescriptorPool;
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1;
vkDescriptorSetAllocateInfo.pSetLayouts = &gpDescriptorSetLayouts->vkDescriptorSetLayout_Meshlet;


    // Allocate descriptor set
    vkResult =
        vkAllocateDescriptorSets(
            gVulkanContext.vkDevice,
            &vkDescriptorSetAllocateInfo,
            &vkDescriptorSet_Meshlet);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(
            gpFILE,
            "createDescriptorSet_Meshlet() : "
            "vkAllocateDescriptorSets() failed.\n");

        return vkResult;
    }


    // ============================================================
    // Descriptor buffer information
    // ============================================================

    VkDescriptorBufferInfo vkDescriptorBufferInfo_Vertex;

    memset(
        (void*)&vkDescriptorBufferInfo_Vertex,
        0,
        sizeof(VkDescriptorBufferInfo));

    vkDescriptorBufferInfo_Vertex =
        meshletGPUData.vertexBuffer.descriptor;


    VkDescriptorBufferInfo vkDescriptorBufferInfo_Meshlet;

    memset(
        (void*)&vkDescriptorBufferInfo_Meshlet,
        0,
        sizeof(VkDescriptorBufferInfo));

    vkDescriptorBufferInfo_Meshlet =
        meshletGPUData.meshletBuffer.descriptor;


    VkDescriptorBufferInfo vkDescriptorBufferInfo_MeshletVertex;

    memset(
        (void*)&vkDescriptorBufferInfo_MeshletVertex,
        0,
        sizeof(VkDescriptorBufferInfo));

    vkDescriptorBufferInfo_MeshletVertex =
        meshletGPUData.meshletVertexBuffer.descriptor;


    VkDescriptorBufferInfo vkDescriptorBufferInfo_MeshletTriangle;

    memset(
        (void*)&vkDescriptorBufferInfo_MeshletTriangle,
        0,
        sizeof(VkDescriptorBufferInfo));

    vkDescriptorBufferInfo_MeshletTriangle =
        meshletGPUData.meshletTriangleBuffer.descriptor;


    // ============================================================
    // Write descriptor set
    // ============================================================

    VkWriteDescriptorSet vkWriteDescriptorSet_array[4];

    memset(
        (void*)vkWriteDescriptorSet_array,
        0,
        sizeof(VkWriteDescriptorSet) * 4);


    // ------------------------------------------------------------
    // Binding 0 : MeshletData[]
    // ------------------------------------------------------------

    vkWriteDescriptorSet_array[0].sType =
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

    vkWriteDescriptorSet_array[0].pNext =
        NULL;

    vkWriteDescriptorSet_array[0].dstSet =
        vkDescriptorSet_Meshlet;

    vkWriteDescriptorSet_array[0].dstBinding =
        0;

    vkWriteDescriptorSet_array[0].dstArrayElement =
        0;

    vkWriteDescriptorSet_array[0].descriptorCount =
        1;

    vkWriteDescriptorSet_array[0].descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    vkWriteDescriptorSet_array[0].pBufferInfo =
        &vkDescriptorBufferInfo_Meshlet;


    // ------------------------------------------------------------
    // Binding 1 : meshletVertices[]
    // ------------------------------------------------------------

    vkWriteDescriptorSet_array[1].sType =
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

    vkWriteDescriptorSet_array[1].pNext =
        NULL;

    vkWriteDescriptorSet_array[1].dstSet =
        vkDescriptorSet_Meshlet;

    vkWriteDescriptorSet_array[1].dstBinding =
        1;

    vkWriteDescriptorSet_array[1].dstArrayElement =
        0;

    vkWriteDescriptorSet_array[1].descriptorCount =
        1;

    vkWriteDescriptorSet_array[1].descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    vkWriteDescriptorSet_array[1].pBufferInfo =
        &vkDescriptorBufferInfo_MeshletVertex;


    // ------------------------------------------------------------
    // Binding 2 : meshletTriangles[]
    // ------------------------------------------------------------

    vkWriteDescriptorSet_array[2].sType =
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

    vkWriteDescriptorSet_array[2].pNext =
        NULL;

    vkWriteDescriptorSet_array[2].dstSet =
        vkDescriptorSet_Meshlet;

    vkWriteDescriptorSet_array[2].dstBinding =
        2;

    vkWriteDescriptorSet_array[2].dstArrayElement =
        0;

    vkWriteDescriptorSet_array[2].descriptorCount =
        1;

    vkWriteDescriptorSet_array[2].descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    vkWriteDescriptorSet_array[2].pBufferInfo =
        &vkDescriptorBufferInfo_MeshletTriangle;


    // ------------------------------------------------------------
    // Binding 3 : VertexData[]
    // ------------------------------------------------------------

    vkWriteDescriptorSet_array[3].sType =
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

    vkWriteDescriptorSet_array[3].pNext =
        NULL;

    vkWriteDescriptorSet_array[3].dstSet =
        vkDescriptorSet_Meshlet;

    vkWriteDescriptorSet_array[3].dstBinding =
        3;

    vkWriteDescriptorSet_array[3].dstArrayElement =
        0;

    vkWriteDescriptorSet_array[3].descriptorCount =
        1;

    vkWriteDescriptorSet_array[3].descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    vkWriteDescriptorSet_array[3].pBufferInfo =
        &vkDescriptorBufferInfo_Vertex;


    // ============================================================
    // Update descriptor set
    // ============================================================

    vkUpdateDescriptorSets(
        gVulkanContext.vkDevice,
        4,
        vkWriteDescriptorSet_array,
        0,
        NULL);


    return vkResult;
}


void StaticMeshletModel::DestroyMeshletBuffers()
{
    if (meshletGPUData.vertexBuffer.vkBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(
            gVulkanContext.vkDevice,
            meshletGPUData.vertexBuffer.vkBuffer,
            nullptr);

        meshletGPUData.vertexBuffer.vkBuffer =
            VK_NULL_HANDLE;
    }

    if (meshletGPUData.vertexBuffer.vkDeviceMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(
            gVulkanContext.vkDevice,
            meshletGPUData.vertexBuffer.vkDeviceMemory,
            nullptr);

        meshletGPUData.vertexBuffer.vkDeviceMemory =
            VK_NULL_HANDLE;
    }


    if (meshletGPUData.meshletBuffer.vkBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(
            gVulkanContext.vkDevice,
            meshletGPUData.meshletBuffer.vkBuffer,
            nullptr);

        meshletGPUData.meshletBuffer.vkBuffer =
            VK_NULL_HANDLE;
    }

    if (meshletGPUData.meshletBuffer.vkDeviceMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(
            gVulkanContext.vkDevice,
            meshletGPUData.meshletBuffer.vkDeviceMemory,
            nullptr);

        meshletGPUData.meshletBuffer.vkDeviceMemory =
            VK_NULL_HANDLE;
    }


    if (meshletGPUData.meshletVertexBuffer.vkBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(
            gVulkanContext.vkDevice,
            meshletGPUData.meshletVertexBuffer.vkBuffer,
            nullptr);

        meshletGPUData.meshletVertexBuffer.vkBuffer =
            VK_NULL_HANDLE;
    }

    if (meshletGPUData.meshletVertexBuffer.vkDeviceMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(
            gVulkanContext.vkDevice,
            meshletGPUData.meshletVertexBuffer.vkDeviceMemory,
            nullptr);

        meshletGPUData.meshletVertexBuffer.vkDeviceMemory =
            VK_NULL_HANDLE;
    }


    if (meshletGPUData.meshletTriangleBuffer.vkBuffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(
            gVulkanContext.vkDevice,
            meshletGPUData.meshletTriangleBuffer.vkBuffer,
            nullptr);

        meshletGPUData.meshletTriangleBuffer.vkBuffer =
            VK_NULL_HANDLE;
    }

    if (meshletGPUData.meshletTriangleBuffer.vkDeviceMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(
            gVulkanContext.vkDevice,
            meshletGPUData.meshletTriangleBuffer.vkDeviceMemory,
            nullptr);

        meshletGPUData.meshletTriangleBuffer.vkDeviceMemory =
            VK_NULL_HANDLE;
    }


    meshletGPUData.vertexCount = 0;
    meshletGPUData.meshletCount = 0;
}
