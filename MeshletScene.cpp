#include "MeshletScene.h"


MeshletScene::~MeshletScene()
{
    DestroyGlobalSSBOs();
}

void MeshletScene::ShutDown(void)
{
    DestroyGlobalSSBOs();
}


void MeshletScene::AddModel(StaticMeshletModel* model)
{
    if (!model)
        return;

    models.push_back(model);
}


void MeshletScene::MergeModel(const StaticMeshletModel& model, uint32_t modelIndex)
{
    const uint32_t vertexBase =
        static_cast<uint32_t>(
            vertices.size());

    const uint32_t meshletVertexBase =
        static_cast<uint32_t>(
            meshletVertices.size());

    const uint32_t triangleBase =
        static_cast<uint32_t>(
            meshletTriangles.size());


    // ------------------------------------------------------------
    // Vertices
    // ------------------------------------------------------------

    const auto& srcVertices =
        model.GetVertices();

    vertices.insert(
        vertices.end(),
        srcVertices.begin(),
        srcVertices.end());


    // ------------------------------------------------------------
    // Meshlet vertex references
    // ------------------------------------------------------------

    const auto& srcMeshletVertices =
        model.GetMeshletVertices();

    for (uint32_t index :
    srcMeshletVertices)
    {
        meshletVertices.push_back(
            index + vertexBase);
    }


    // ------------------------------------------------------------
    // Meshlet triangles
    // ------------------------------------------------------------

    const auto& srcMeshletTriangles =
        model.GetMeshletTriangles();

    meshletTriangles.insert(
        meshletTriangles.end(),
        srcMeshletTriangles.begin(),
        srcMeshletTriangles.end());


    // ------------------------------------------------------------
    // Meshlet metadata
    // ------------------------------------------------------------

    const auto& srcMeshlets =
        model.GetMeshlets();

    for (const MeshletData& src :
        srcMeshlets)
    {
        MeshletData dst = src;

        dst.vertexOffset += meshletVertexBase;
        dst.triangleOffset += triangleBase;
        dst.modelIndex = modelIndex;

        meshlets.push_back(dst);
    }
}



VkResult MeshletScene::Build()
{
    vertices.clear();
    meshlets.clear();
    meshletVertices.clear();
    meshletTriangles.clear();
    modelMatrices.clear();

    uint32_t modelIndex = 0;

    for (const StaticMeshletModel* model : models)
    {
        if (!model)
            continue;

        // ----------------------------------------------------
        // One matrix per model
        // ----------------------------------------------------
        modelMatrices.push_back(model->GetModelMatrix());

        // ----------------------------------------------------
        // Every meshlet from this model gets this index
        // ----------------------------------------------------
        MergeModel(
            *model,
            modelIndex);

        ++modelIndex;
    }

    return CreateGlobalSSBOs();
}





VkResult MeshletScene::CreateDeviceLocalSSBO(
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
        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

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


    VkMemoryAllocateFlagsInfo allocFlagsInfo{};
    allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;


    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = &allocFlagsInfo;
    allocInfo.allocationSize = memReq.size;

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
    // *. Get Device Address
    // ============================================================


    VkBufferDeviceAddressInfo addressInfo{};

    addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    addressInfo.buffer = outSSBO.vkBuffer;
    outSSBO.deviceAddress = vkGetBufferDeviceAddress(gVulkanContext.vkDevice, &addressInfo);


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

VkResult MeshletScene::CreateGlobalSSBOs(void)
{
    VkResult vkResult = VK_SUCCESS;


    // ============================================================
    // Vertex data
    // ============================================================

    vkResult =
        CreateDeviceLocalSSBO(
            vertices.data(),
            vertices.size() *
            sizeof(VertexData_Meshlet),
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


    // ============================================================
    // Global Model Matrices
    // ============================================================

    vkResult =
        CreateDeviceLocalSSBO(
            modelMatrices.data(),
            modelMatrices.size() * sizeof(glm::mat4),
            meshletGPUData.modelMatricesBuffer);

    if (vkResult != VK_SUCCESS)
        return vkResult;


    fprintf(
        gpFILE,
        "StaticMeshletModel::CreateMeshletSSBOs() : "
        "Created device-local meshlet buffers. "
        "Vertices=%u Meshlets=%u.\n",
        meshletGPUData.vertexCount,
        meshletGPUData.meshletCount);


    return VK_SUCCESS;
}

void MeshletScene::DestroyGlobalSSBOs()
{
    auto DestroySSBO =
        [](VulkanSSBO& ssbo)
        {
            if (ssbo.vkBuffer != VK_NULL_HANDLE)
            {
                vkDestroyBuffer(
                    gVulkanContext.vkDevice,
                    ssbo.vkBuffer,
                    nullptr);

                ssbo.vkBuffer =
                    VK_NULL_HANDLE;
            }

            if (ssbo.vkDeviceMemory != VK_NULL_HANDLE)
            {
                vkFreeMemory(
                    gVulkanContext.vkDevice,
                    ssbo.vkDeviceMemory,
                    nullptr);

                ssbo.vkDeviceMemory =
                    VK_NULL_HANDLE;
            }

            ssbo.deviceAddress = 0;
            ssbo.size = 0;
        };


    DestroySSBO(meshletGPUData.vertexBuffer);
    DestroySSBO(meshletGPUData.meshletBuffer);
    DestroySSBO(meshletGPUData.meshletTriangleBuffer);
    DestroySSBO(meshletGPUData.meshletVertexBuffer);
    DestroySSBO(meshletGPUData.modelMatricesBuffer);
}

