#include "FrameContext.h"


FrameContext gFrames[MAX_FRAMES];

VkResult FrameContext::Initialize(VulkanContext* context)
{
    VkResult vkResult = VK_SUCCESS;

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = context->vkCommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    vkResult = vkAllocateCommandBuffers(
        context->vkDevice,
        &commandBufferAllocateInfo,
        &commandBuffer);

    if (vkResult != VK_SUCCESS)
    {
        return vkResult;
    }

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vkResult = vkCreateSemaphore(
        context->vkDevice,
        &semaphoreCreateInfo,
        nullptr,
        &vkSemaphore_Acquire);

    if (vkResult != VK_SUCCESS)
    {
        return vkResult;
    }

    vkResult = vkCreateSemaphore(
        context->vkDevice,
        &semaphoreCreateInfo,
        nullptr,
        &vkSemaphore_RenderComplete);

    if (vkResult != VK_SUCCESS)
    {
        return vkResult;
    }

    return VK_SUCCESS;
}

void FrameContext::Destroy(VulkanContext* context)
{
    if (vkSemaphore_RenderComplete != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(
            context->vkDevice,
            vkSemaphore_RenderComplete,
            nullptr);
        vkSemaphore_RenderComplete = VK_NULL_HANDLE;
    }

    if (vkSemaphore_Acquire != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(
            context->vkDevice,
            vkSemaphore_Acquire,
            nullptr);
        vkSemaphore_Acquire = VK_NULL_HANDLE;
    }

    if (commandBuffer != VK_NULL_HANDLE)
    {
        vkFreeCommandBuffers(
            context->vkDevice,
            context->vkCommandPool,
            1,
            &commandBuffer);
        commandBuffer = VK_NULL_HANDLE;
    }
}