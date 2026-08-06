#pragma once
#define NOMINMAX

#include <vulkan/vulkan.h>
#include "VK.h"    // For SwapChainResourceData
#include "VulkanContext.h"


extern FILE* gpFILE;

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
    SwapChainResourceData resourceData;

    // Rendering
    uint32_t currentImageIndex = UINT32_MAX;

	VkResult Initialize();
	void Shutdown();

    private:

        VkResult CreateSwapchain(VkBool32 vsync);
		void DestroySwapchain();

        VkResult CreateSwapchainResources();
		void DestroySwapchainResources();

		VkResult CreateImagesAndImageViews(void);
        VkResult CreateDepthResources(void);

		VkResult GetSupportedDepthFormat(VkFormat* pVkFormat);
};

extern SwapchainContext gSwapchain;