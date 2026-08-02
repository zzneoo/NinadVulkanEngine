#pragma once

#include <vulkan/vulkan.h>

class VulkanContext
{
public:
    // Instance
    VkInstance vkInstance = VK_NULL_HANDLE;

    // Physical device
    VkPhysicalDevice* vkPhysicalDevice_Array = NULL;
    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties{};

    // Queue
    uint32_t vkGraphicsQueueFamilyIndex = UINT32_MAX;
    VkQueue vkGraphicsQueue = VK_NULL_HANDLE;

    // Logical device
    VkDevice vkDevice = VK_NULL_HANDLE;
    // Command infrastructure
    VkCommandPool vkCommandPool = VK_NULL_HANDLE;

    // Validation
    bool vkValidationEnabled = true;
    uint32_t enabledValidationLayerCount = 0;
    const char* enabledValidationLayerNames_array[1];//for VK_LAYER_KHRONOS_validation
    VkDebugReportCallbackEXT vkDebugReportCallbackEXT = VK_NULL_HANDLE;
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_fnptr = nullptr;
};

extern VulkanContext gVulkanContext;

