#pragma once

#include <vulkan/vulkan.h>
#include "VK.h"

struct FrameContext
{
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    VkSemaphore vkSemaphore_Acquire = VK_NULL_HANDLE;
    VkSemaphore vkSemaphore_RenderComplete = VK_NULL_HANDLE;

    uint64_t timelineValue = 0;

    UniformData uniformData{};

    VkDescriptorSet vkDescriptor_FrameData = VK_NULL_HANDLE;
    VkDescriptorSet vkDescriptor_FrameDataBoneData = VK_NULL_HANDLE;
};

extern FrameContext gFrames[MAX_FRAMES];