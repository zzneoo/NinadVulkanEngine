#include "VolumetricClouds.h"
#include "stb_image.h"

VolumetricClouds::VolumetricClouds() 
{
    VkResult res = VK_SUCCESS;


    //Noise3D
    uint32_t mipLevels = 0;

    bool success = Load3DTextureWithMipmaps(
        gVulkanContext.vkDevice,         // Logical device handle
        gVulkanContext.vkCommandPool,    // Command pool for single-time commands
        gVulkanContext.vkGraphicsQueue,  // Graphics queue handle
        "Resources/Noise/Nubis3D",      // Directory path containing slices
        "NubisVoxelCloudNoise",        // Base file prefix
        ".tga",                        // File extension
        128,                           // Number of Z-slices (depth)
        3,                             // Zero-padding width (e.g., 3 for .001, .002...)
        imageData_Noise3D,                  // Output ImageData struct (populated by ref)
        mipLevels                      // Output total calculated mip levels
    );
    imageData_Noise3D.vkSampler = vkSampler_LinearMipmapRepeat;

    if (!success) {
        fprintf(gpFILE, "VolumetricClouds() : Load3DTextureWithMipmaps() failed (%d).\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
    }

    //cloud Texture

    Width = WIN_WIDTH;
    Height = WIN_HEIGHT;
    res = CreateCloudTexture(Width, Height, VK_FORMAT_R8G8B8A8_UNORM,imageData_Clouds);
    if (res != VK_SUCCESS)
    {
        fprintf(gpFILE, "VolumetricClouds() : CreateCloudTexture() failed (%d).\n", vkResult);
        vkResult = res;
    }

    res = CreateDescriptorSet_VolumetricClouds();
    if (res != VK_SUCCESS)
    {
        fprintf(gpFILE, "VolumetricClouds() : CreateDescriptorSet_VolumetricClouds() failed (%d).\n", vkResult);
        vkResult = res;
    }


    //Initial Layout transitions
    res = InitialLayoutTransitions();
    if (res != VK_SUCCESS)
    {
        fprintf(gpFILE, "VolumetricClouds() : InitialLayoutTransitions() failed (%d).\n", vkResult);
        vkResult = res;
    }


}

VolumetricClouds::~VolumetricClouds()
{
    //cloud texture
    if (imageData_Clouds.vkImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(gVulkanContext.vkDevice, imageData_Clouds.vkImageView, nullptr);
        imageData_Clouds.vkImageView = VK_NULL_HANDLE;
    }
    if (imageData_Clouds.vkImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(gVulkanContext.vkDevice, imageData_Clouds.vkImage, nullptr);
        imageData_Clouds.vkImage = VK_NULL_HANDLE;
    }
    if (imageData_Clouds.vkDeviceMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gVulkanContext.vkDevice, imageData_Clouds.vkDeviceMemory, nullptr);
        imageData_Clouds.vkDeviceMemory = VK_NULL_HANDLE;
    }

    //noise3D texture
        //cloud texture
    if (imageData_Noise3D.vkImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(gVulkanContext.vkDevice, imageData_Noise3D.vkImageView, nullptr);
        imageData_Noise3D.vkImageView = VK_NULL_HANDLE;
    }
    if (imageData_Noise3D.vkImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(gVulkanContext.vkDevice, imageData_Noise3D.vkImage, nullptr);
        imageData_Noise3D.vkImage = VK_NULL_HANDLE;
    }
    if (imageData_Noise3D.vkDeviceMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gVulkanContext.vkDevice, imageData_Noise3D.vkDeviceMemory, nullptr);
        imageData_Noise3D.vkDeviceMemory = VK_NULL_HANDLE;
    }
}

VkResult VolumetricClouds::CreateCloudTexture(
    uint32_t width,
    uint32_t height,
    VkFormat format,
    ImageData& imageData)
{
    imageData = {};

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent = { width, height, 1 };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage =
        VK_IMAGE_USAGE_STORAGE_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkResult result = vkCreateImage(
        gVulkanContext.vkDevice,
        &imageInfo,
        nullptr,
        &imageData.vkImage);

    if (result != VK_SUCCESS)
        return result;

    VkMemoryRequirements memReq{};
    vkGetImageMemoryRequirements(
        gVulkanContext.vkDevice,
        imageData.vkImage,
        &memReq);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex =
        gVulkanContext.FindMemoryType(
            memReq.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    result = vkAllocateMemory(
        gVulkanContext.vkDevice,
        &allocInfo,
        nullptr,
        &imageData.vkDeviceMemory);

    if (result != VK_SUCCESS)
        return result;

    result = vkBindImageMemory(
        gVulkanContext.vkDevice,
        imageData.vkImage,
        imageData.vkDeviceMemory,
        0);

    if (result != VK_SUCCESS)
        return result;

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = imageData.vkImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    result = vkCreateImageView(
        gVulkanContext.vkDevice,
        &viewInfo,
        nullptr,
        &imageData.vkImageView);

    if (result != VK_SUCCESS)
        return result;

    imageData.vkSampler = vkSampler_LinearClamp;


    return VK_SUCCESS;
}

void VolumetricClouds::TransitionImageLayout(
    VkCommandBuffer cmd,
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout)
{
    VkImageMemoryBarrier2 barrier{
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2
    };

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.image = image;

    barrier.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_COLOR_BIT;

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount =
        VK_REMAINING_MIP_LEVELS;

    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount =
        VK_REMAINING_ARRAY_LAYERS;


    // --------------------------------------------------------
    // UNDEFINED -> COLOR_ATTACHMENT_OPTIMAL
    // --------------------------------------------------------
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcStageMask =
            VK_PIPELINE_STAGE_2_NONE;

        barrier.srcAccessMask =
            VK_ACCESS_2_NONE;

        barrier.dstStageMask =
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

        barrier.dstAccessMask =
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    }

    // --------------------------------------------------------
    // UNDEFINED -> SHADER_READ_ONLY_OPTIMAL
    // --------------------------------------------------------
    else if (oldLayout ==
        VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout ==
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcStageMask =
            VK_PIPELINE_STAGE_2_NONE;

        barrier.srcAccessMask =
            VK_ACCESS_2_NONE;

        barrier.dstStageMask =
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

        barrier.dstAccessMask =
            VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    }

    // --------------------------------------------------------
    // UNDEFINED -> PRESENT_SRC_KHR
    // --------------------------------------------------------
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        barrier.srcStageMask =
            VK_PIPELINE_STAGE_2_NONE;

        barrier.srcAccessMask =
            VK_ACCESS_2_NONE;

        barrier.dstStageMask =
            VK_PIPELINE_STAGE_2_NONE;

        barrier.dstAccessMask =
            VK_ACCESS_2_NONE;
    }

    // --------------------------------------------------------
    // PRESENT_SRC_KHR -> COLOR_ATTACHMENT_OPTIMAL
    // --------------------------------------------------------
    else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR &&
        newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcStageMask =
            VK_PIPELINE_STAGE_2_NONE;

        barrier.srcAccessMask =
            VK_ACCESS_2_NONE;

        barrier.dstStageMask =
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

        barrier.dstAccessMask =
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    }
    // --------------------------------------------------------
    // SHADER_READ_ONLY_OPTIMAL -> COLOR_ATTACHMENT_OPTIMAL
    // --------------------------------------------------------
    else if (oldLayout ==
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
        newLayout ==
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcStageMask =
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

        barrier.srcAccessMask =
            VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;

        barrier.dstStageMask =
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

        barrier.dstAccessMask =
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    }

    // --------------------------------------------------------
    // COLOR_ATTACHMENT_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL
    // --------------------------------------------------------
    else if (oldLayout ==
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
        newLayout ==
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcStageMask =
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

        barrier.srcAccessMask =
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;

        barrier.dstStageMask =
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;

        barrier.dstAccessMask =
            VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    }

    // --------------------------------------------------------
    // COLOR_ATTACHMENT_OPTIMAL -> PRESENT_SRC_KHR
    // --------------------------------------------------------
    else if (oldLayout ==
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
        newLayout ==
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        barrier.srcStageMask =
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

        barrier.srcAccessMask =
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;

        barrier.dstStageMask =
            VK_PIPELINE_STAGE_2_NONE;

        barrier.dstAccessMask =
            VK_ACCESS_2_NONE;
    }

    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_GENERAL)
    {
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        barrier.srcAccessMask = VK_ACCESS_2_NONE;

        barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_GENERAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;

        barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_GENERAL)
    {
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;

        barrier.dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
    }
    else
    {
        throw std::runtime_error(
            "Unsupported layout transition");
    }

    VkDependencyInfo dep{
        VK_STRUCTURE_TYPE_DEPENDENCY_INFO
    };

    dep.imageMemoryBarrierCount = 1;
    dep.pImageMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(cmd, &dep);
}

VkResult VolumetricClouds::InitialLayoutTransitions(void)
{


    VkResult vkResult = VK_SUCCESS;

    //-----------------------------------------------------------------------------------------------------------------------

// Transition the depth image layout to be optimal for depth attachment
    // Transition the image layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = gVulkanContext.vkCommandPool; // Command pool for allocation
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Primary command buffer
    commandBufferAllocateInfo.commandBufferCount = 1; // Allocate one command buffer
    vkResult = vkAllocateCommandBuffers(gVulkanContext.vkDevice, &commandBufferAllocateInfo, &commandBuffer);
    if (vkResult != VK_SUCCESS)
    {
        return vkResult;
    }
    // Begin command buffer recording
    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // One-time use command buffer
    vkResult = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
    if (vkResult != VK_SUCCESS)
    {
        return vkResult;
    }

    
    TransitionImageLayout(
        commandBuffer,
        imageData_Clouds.vkImage,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    // End command buffer recording
    vkResult = vkEndCommandBuffer(commandBuffer);
    if (vkResult != VK_SUCCESS)
    {
        return vkResult;
    }
    // Submit the command buffer and wait for it to finish
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkResult = vkQueueSubmit(gVulkanContext.vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (vkResult != VK_SUCCESS)
    {
        return vkResult;
    }
    vkQueueWaitIdle(gVulkanContext.vkGraphicsQueue);
    // Free the command buffer
    vkFreeCommandBuffers(gVulkanContext.vkDevice, gVulkanContext.vkCommandPool, 1, &commandBuffer);

    return vkResult;
}

VkResult VolumetricClouds::CreateDescriptorSet_VolumetricClouds()
{
    VkDescriptorSetAllocateInfo allocInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO
    };
    allocInfo.descriptorPool = vkDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &gpDescriptorSetLayouts->vkDescriptorSetLayout_VolumetricClouds;

    VkResult result = vkAllocateDescriptorSets(
        gVulkanContext.vkDevice,
        &allocInfo,
        &vkDescriptorSet_VolumetricClouds);

    if (result != VK_SUCCESS)
        return result;

    // Binding 0: Storage Image
    VkDescriptorImageInfo storageImageInfo{};
    storageImageInfo.sampler = vkSampler_LinearClamp;
    storageImageInfo.imageView = imageData_Clouds.vkImageView;
    storageImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    // Binding 1: Combined Image Sampler
    VkDescriptorImageInfo sampledImageInfo{};
    sampledImageInfo.sampler = imageData_Noise3D.vkSampler; // Provide your sampler handle here
    sampledImageInfo.imageView = imageData_Noise3D.vkImageView;
    sampledImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writes[2]{};

    // Write for Binding 0
    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = vkDescriptorSet_VolumetricClouds;
    writes[0].dstBinding = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    writes[0].pImageInfo = &storageImageInfo;

    // Write for Binding 1
    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = vkDescriptorSet_VolumetricClouds;
    writes[1].dstBinding = 1;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].pImageInfo = &sampledImageInfo;

    vkUpdateDescriptorSets(
        gVulkanContext.vkDevice,
        2,              // Count of writes updated
        writes,
        0,
        nullptr);

    return VK_SUCCESS;
}

void VolumetricClouds::Compute_VolumetricClouds(
    uint32_t curIndex,
    VkPipeline vkPipeline,
    VkPipelineLayout vkPipelineLayout)
{

    //-------------------------------------------------------------------------
// Compute Texture
//-------------------------------------------------------------------------
    VkImage vkImage = imageData_Clouds.vkImage;

    TransitionImageLayout(
        gFrames[curIndex].commandBuffer,
        vkImage,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL
    );

    vkCmdBindPipeline(
        gFrames[curIndex].commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        vkPipeline
    );

    VkDescriptorSet vkLocalDescriptorSets[] = { gFrames[curIndex].vkDescriptor_FrameData, vkDescriptorSet_VolumetricClouds };

    vkCmdBindDescriptorSets(
        gFrames[curIndex].commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        vkPipelineLayout,
        0,
        2,
        vkLocalDescriptorSets,
        0,
        nullptr
    );

    vkCmdDispatch(
        gFrames[curIndex].commandBuffer,
        (WIN_WIDTH + 7) / 8,
        (WIN_HEIGHT + 7) / 8,
        1
    );

    TransitionImageLayout(
        gFrames[curIndex].commandBuffer,
        vkImage,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
}

bool VolumetricClouds::Load3DTextureWithMipmaps(
    VkDevice device,
    VkCommandPool commandPool,
    VkQueue graphicsQueue,
    const std::string& baseFolder,
    const std::string& filePrefix,
    const std::string& fileExtension,
    uint32_t sliceCount,
    uint32_t zeroPadding,
    ImageData& outImageData,
    uint32_t& outMipLevels)
{
    if (sliceCount == 0) return false;

    int width = 0, height = 0, channels = 0;
    std::vector<stbi_uc*> loadedSlices(sliceCount, nullptr);
    VkDeviceSize sliceSize = 0;

    // 1. Load Slices from disk into CPU staging memory
    for (uint32_t z = 0; z < sliceCount; ++z) {
        std::stringstream ss;
        // Produces: baseFolder/NubisVoxelCloudNoise.001.tga
        ss << baseFolder << "/" << filePrefix << "."
            << std::setfill('0') << std::setw(zeroPadding) << (z + 1) // 001, 002, ..., 128
            << fileExtension;

        std::string filename = ss.str();

        int sWidth, sHeight, sChannels;
        stbi_uc* pixels = stbi_load(filename.c_str(), &sWidth, &sHeight, &sChannels, STBI_rgb_alpha);
        if (!pixels) {
            std::cerr << "Failed to load slice texture: " << filename << std::endl;
            for (uint32_t i = 0; i < z; ++i) stbi_image_free(loadedSlices[i]);
            return false;
        }

        if (z == 0) {
            width = sWidth;
            height = sHeight;
            channels = 4; // Forced to RGBA by STBI_rgb_alpha
            sliceSize = static_cast<VkDeviceSize>(width * height * channels);
        }
        else if (sWidth != width || sHeight != height) {
            std::cerr << "Slice dimension mismatch at file: " << filename << std::endl;
            for (uint32_t i = 0; i <= z; ++i) stbi_image_free(loadedSlices[i]);
            return false;
        }

        loadedSlices[z] = pixels;
    }

    VkDeviceSize depth = sliceCount;
    VkDeviceSize volumeSize = sliceSize * depth;
    outMipLevels = static_cast<uint32_t>(std::floor(std::log2((std::max)({ (uint32_t)width, (uint32_t)height, (uint32_t)depth })))) + 1;

    // 2. Allocate and map Host-Visible Staging Buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = volumeSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer);

    VkMemoryRequirements bufferMemReqs;
    vkGetBufferMemoryRequirements(device, stagingBuffer, &bufferMemReqs);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = bufferMemReqs.size;
    allocInfo.memoryTypeIndex = gVulkanContext.FindMemoryType(bufferMemReqs.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    vkAllocateMemory(device, &allocInfo, nullptr, &stagingBufferMemory);
    vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

    // 3. Copy slice memory into staging buffer contiguously and free STB pointers
    void* mappedPtr = nullptr;
    vkMapMemory(device, stagingBufferMemory, 0, volumeSize, 0, &mappedPtr);
    for (uint32_t z = 0; z < depth; ++z) {
        uint8_t* dst = static_cast<uint8_t*>(mappedPtr) + (z * sliceSize);
        memcpy(dst, loadedSlices[z], sliceSize);
        stbi_image_free(loadedSlices[z]);
    }
    vkUnmapMemory(device, stagingBufferMemory);

    // 4. Create Optimal 3D VkImage
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_3D;
    imageInfo.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(depth) };
    imageInfo.mipLevels = outMipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if (vkCreateImage(device, &imageInfo, nullptr, &outImageData.vkImage) != VK_SUCCESS) {
        return false;
    }

    VkMemoryRequirements imgMemReqs;
    vkGetImageMemoryRequirements(device, outImageData.vkImage, &imgMemReqs);

    VkMemoryAllocateInfo imgAllocInfo{};
    imgAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    imgAllocInfo.allocationSize = imgMemReqs.size;
    imgAllocInfo.memoryTypeIndex = gVulkanContext.FindMemoryType(imgMemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &imgAllocInfo, nullptr, &outImageData.vkDeviceMemory) != VK_SUCCESS) {
        return false;
    }
    vkBindImageMemory(device, outImageData.vkImage, outImageData.vkDeviceMemory, 0);

    // 5. Begin Command Buffer for Transfer and Mip Generation
    VkCommandBuffer cmd = gVulkanContext.BeginSingleTimeCommands(device, commandPool);

    // Transition all mip levels to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = outImageData.vkImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = outMipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier);

    // Copy staging buffer to Mip Level 0
    VkBufferImageCopy copyRegion{};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;
    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageOffset = { 0, 0, 0 };
    copyRegion.imageExtent = imageInfo.extent;

    vkCmdCopyBufferToImage(cmd, stagingBuffer, outImageData.vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    // 6. Blit downsampling loop for 3D Volume Mip Generation
    int32_t mipWidth = width;
    int32_t mipHeight = height;
    int32_t mipDepth = static_cast<int32_t>(depth);

    for (uint32_t i = 1; i < outMipLevels; i++) {
        // Transition mip (i - 1) to TRANSFER_SRC
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.subresourceRange.levelCount = 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &barrier);

        // Blit calculation across X, Y, and Z
        VkImageBlit blit{};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, mipDepth };

        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = {
            mipWidth > 1 ? mipWidth / 2 : 1,
            mipHeight > 1 ? mipHeight / 2 : 1,
            mipDepth > 1 ? mipDepth / 2 : 1
        };

        vkCmdBlitImage(cmd,
            outImageData.vkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            outImageData.vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit, VK_FILTER_LINEAR);

        // Transition mip (i - 1) to SHADER_READ_ONLY
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
        if (mipDepth > 1) mipDepth /= 2;
    }

    // Transition the final mip level to SHADER_READ_ONLY
    barrier.subresourceRange.baseMipLevel = outMipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier);

    gVulkanContext.EndSingleTimeCommands(device, commandPool, graphicsQueue, cmd);

    // Clean up temporary staging buffer
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    // 7. Create 3D Image View
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = outImageData.vkImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = outMipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    return vkCreateImageView(device, &viewInfo, nullptr, &outImageData.vkImageView) == VK_SUCCESS;
}

