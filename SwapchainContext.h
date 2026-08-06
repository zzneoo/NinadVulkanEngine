#pragma once

#include <vulkan/vulkan.h>
#include "VK.h"    // For SwapChainResourceData

class SwapchainContext
{
public:

    // Window size
    int width = 1920;
    int height = 1080;

    // Surface capabilities
    VkFormat vkFormat = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR vkColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;

    // Swapchain
    VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE;

    VkExtent2D vkExtent2D{};

    uint32_t imageCount = 0;

    // Images / image views / depth / framebuffers
    SwapChainResourceData resources;

    // Rendering
    uint32_t currentImageIndex = UINT32_MAX;
};

extern SwapchainContext gSwapchain;