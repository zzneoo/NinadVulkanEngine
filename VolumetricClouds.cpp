#include "VolumetricClouds.h"


VolumetricClouds::VolumetricClouds() 
{
    VkResult res = VK_SUCCESS;

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
    storageImageInfo.imageView = imageData_Clouds.vkImageView;
    storageImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    // Binding 1: Combined Image Sampler
    VkDescriptorImageInfo sampledImageInfo{};
    //sampledImageInfo.sampler = imageData_Noise.vkSampler; // Provide your sampler handle here
    //sampledImageInfo.imageView = imageData_Noise.vkImageView;
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
        1,              // Count of writes updated
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

    vkCmdBindDescriptorSets(
        gFrames[curIndex].commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        vkPipelineLayout,
        0,
        1,
        &vkDescriptorSet_VolumetricClouds,
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

