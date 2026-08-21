#pragma once

#include <vulkan/vulkan.h>
#include "VK.h"
#include "VulkanContext.h"

class FrameContext
{
public:
    VkResult Initialize(VulkanContext* context);
    void Destroy(VulkanContext* context);

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    VkSemaphore vkSemaphore_Acquire = VK_NULL_HANDLE;
    VkSemaphore vkSemaphore_RenderComplete = VK_NULL_HANDLE;

    // Semaphores
    //VkSemaphore vkSemaphore_Timeline = VK_NULL_HANDLE;
    uint64_t timelineValue = 0;

    UniformData uniformData{};

    VkDescriptorSet vkDescriptor_FrameData = VK_NULL_HANDLE;
    VkDescriptorSet vkDescriptor_FrameDataBoneData = VK_NULL_HANDLE;
};

extern FrameContext gFrames[MAX_FRAMES];