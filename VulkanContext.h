#pragma once

#define LINE_END     "-------------------------------------------------------------------------------------\n"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include "Win32Window.h"

#include <cstdio>

extern FILE* gpFILE;

// global variable declarations
extern const char* gpszAppName;

class VulkanContext
{
public:
    // Instance
    VkInstance vkInstance = VK_NULL_HANDLE;
    // Instance extension related variables
    uint32_t enabledInstanceExtensionCount = 0;

    // 1. VK_KHR_SURFACE_EXTENSION_NAME
    // 2. VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    // 3. VK_EXT_DEBUG_REPORT_EXTENSION_NAME
    const char* enabledInstanceExtensionNames_Array[3];

    // Physical device
    uint32_t physicalDeviceCount = 0;
    VkPhysicalDevice* vkPhysicalDevice_Array = NULL;
    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties{};
    uint32_t enabledDeviceExtensionCount;
    const char* enabledDeviceExtensionNames_Array[2]; //for VK_KHR_SWAPCHAIN_EXTENSION_NAME, MESH_SHADER_EXTENSION

    // Queue
    uint32_t vkGraphicsQueueFamilyIndex = UINT32_MAX;
    VkQueue vkGraphicsQueue = VK_NULL_HANDLE;

    // Logical device
    VkDevice vkDevice = VK_NULL_HANDLE;
    // Command infrastructure
    VkCommandPool vkCommandPool = VK_NULL_HANDLE;

    // Semaphores
    VkSemaphore vkSemaphore_Timeline = VK_NULL_HANDLE;
    uint64_t gTimelineValue = 0;

    // Validation
    bool vkValidationEnabled = true;
    uint32_t enabledValidationLayerCount = 0;
    const char* enabledValidationLayerNames_array[1];//for VK_LAYER_KHRONOS_validation
    VkDebugReportCallbackEXT vkDebugReportCallbackEXT = VK_NULL_HANDLE;
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_fnptr = nullptr;

	//member functions
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkResult Initialize();
    void Shutdown();

    private:
        VkResult FillInstanceExtensionNames(void);
		VkResult FillValidationLayerNames(void);
		VkResult FillDeviceExtensionNames(void);

		VkResult CreateValidationCallbackFunction(void);
        VkResult PrintVkInfo(void);

        VkResult CreateVulkanInstance(void);
		VkResult GetSupportedSurface(void);
		VkResult GetPhysicalDevice(void);
		VkResult CreateVulkanDevice(void);
		VkResult CreateTimelineSemaphore(void);

        VkResult GetDeviceQueue(void);
		VkResult CreateCommandPool(void);

        //debugReportCallback
        //VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT vkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);

};

extern VulkanContext gVulkanContext;
