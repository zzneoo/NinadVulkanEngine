#include "VulkanContext.h"

//VulkanContext
VulkanContext gVulkanContext;

VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT vkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    //code
    fprintf(gpFILE, "NDT_Validation: %s %d = %s \n", pLayerPrefix, messageCode, pMessage);

    return VK_FALSE;
}

VkResult VulkanContext::Initialize()
{
	// Initialize Vulkan instance, physical device, logical device, and other resources here.

	VkResult vkResult = VK_SUCCESS;

	vkResult = CreateVulkanInstance();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "VulkanContext::Initialize() : CreateVulkanInstance() failed.\n");
		return(vkResult);
	}

	vkResult = GetSupportedSurface();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "VulkanContext::Initialize() : GetSupportedSurface() failed.\n");
        return(vkResult);
	}

	vkResult = GetPhysicalDevice();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "VulkanContext::Initialize() : GetPhysicalDevice() failed.\n");
        return(vkResult);
	}

	vkResult = PrintVkInfo();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "VulkanContext::Initialize() : PrintVkInfo() failed.\n");
        return(vkResult);
	}

	vkResult = CreateVulkanDevice();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "VulkanContext::Initialize() : CreateVulkanDevice() failed.\n");
        return(vkResult);
	}

	vkResult = GetDeviceQueue();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "VulkanContext::Initialize() : GetDeviceQueue() failed.\n");
        return(vkResult);
	}

	vkResult = CreateCommandPool();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "VulkanContext::Initialize() : CreateCommandPool() failed.\n");
        return(vkResult);
	}

	return VK_SUCCESS;
}

void VulkanContext::Shutdown()
{
	// Clean up Vulkan resources here.

	// Destroy Vulkan command pool
	if (vkCommandPool)
	{
        vkDestroyCommandPool(
            vkDevice,      // [in] Vulkan device handle
            vkCommandPool, // [in] Vulkan command pool handle
            NULL           // [in, optional] pointer to a custom memory allocator
		);

		vkCommandPool = VK_NULL_HANDLE;
	}

        /*
     * no need to destroy / uninitialize device queue
     */

     // Destroy Vulkan device
    if (vkDevice)
    {
        // finally, destroy it
        vkDestroyDevice(
            vkDevice, // [in] Vulkan device handle
            NULL      // [in, optional] pointer to a custom memory allocator
        );
        vkDevice = VK_NULL_HANDLE;
    }

    /*
     * no need to destroy selected physical device
     */

     // Destroy the VkSurfaceKHR object
    if (gWindow.vkSurfaceKHR)
    {
        vkDestroySurfaceKHR(
            vkInstance,   // [in] Vulkan instance handle
            gWindow.vkSurfaceKHR, // [in] Vulkan presentation surface handle
            NULL          // [in, optional] pointer to a custom memory allocator (NULL means use a default memory allocator)
        );

        gWindow.vkSurfaceKHR = VK_NULL_HANDLE;
    }

    //validation
    if (vkDebugReportCallbackEXT && vkDestroyDebugReportCallbackEXT_fnptr)
    {
        vkDestroyDebugReportCallbackEXT_fnptr(vkInstance, vkDebugReportCallbackEXT, NULL);
        vkDebugReportCallbackEXT = VK_NULL_HANDLE;
        vkDestroyDebugReportCallbackEXT_fnptr = NULL;
    }

    // Destroy the VkInstance
    if (vkInstance)
    {
        vkDestroyInstance(
            vkInstance, // [in] Vulkan instance handle
            NULL        // [in, optional] pointer to a custom memory allocator (NULL means use a default memory allocator)
        );

        vkInstance = VK_NULL_HANDLE;
    }
}

// Helper: find a memory type index with required properties
uint32_t VulkanContext::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    fprintf(gpFILE, "Failed to find suitable memory type.\n");
    exit(EXIT_FAILURE);
}

VkResult VulkanContext::FillInstanceExtensionNames(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    /*
     * sub-step 1 : Find how many extensions are supported by the Vulkan
     *              driver of this version and keep it in a local variable
     */
    uint32_t instanceExtensionCount = 0;

    vkResult = vkEnumerateInstanceExtensionProperties(
        NULL,                    // [in, optional] layer name to retrieve extensions from (NULL means you want all extensions)
        &instanceExtensionCount, // [out] count of supported extensions
        NULL                     // [out, optional] array of VkExtensionProperties to retrieve extension properties
    );
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "fillInstanceExtensionNames() : first call to vkEnumerateInstanceExtensionProperties() failed.\n");
        return(vkResult);
    }

    /*
     * sub-step 2 : allocate and fill struct VkExtensionProperties array
     *              corresponding to above count
     */
    VkExtensionProperties* vkExtensionProperties_Array = NULL;
    vkExtensionProperties_Array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);

    /*
     * for the sake of brevity, we are avoiding error checking for malloc()
     * but in real world, you should do this error-checking
     */

    vkResult = vkEnumerateInstanceExtensionProperties(
        NULL,
        &instanceExtensionCount,
        vkExtensionProperties_Array
    );
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "fillInstanceExtensionNames() : second call to vkEnumerateInstanceExtensionProperties() failed.\n");
        return(vkResult);
    }

    /*
     * sub-step 3 : fill & display a local string array of extension names
     *              obtained from VkExtensionProperties struct array
     */
    char** instanceExtensionNames_Array = NULL;
    instanceExtensionNames_Array = (char**)malloc(sizeof(char*) * instanceExtensionCount);

    fprintf(gpFILE, LINE_END);

    for (uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        instanceExtensionNames_Array[i] = (char*)malloc(sizeof(char) * (strlen(vkExtensionProperties_Array[i].extensionName) + 1));
        memcpy(
            instanceExtensionNames_Array[i],                         // destination
            vkExtensionProperties_Array[i].extensionName,            // source
            strlen(vkExtensionProperties_Array[i].extensionName) + 1 // length
        );

#ifdef PRINT_EXTENIONS
        fprintf(gpFILE, "fillInstanceExtensionNames() : Vulkan instance extension name = %s\n", instanceExtensionNames_Array[i]);
#endif //PRINT_EXTENIONS
    }

    fprintf(gpFILE, LINE_END);

    /*
     * sub-step 4 : as not required here onwards, free the VkExtensionProperties array
     */
    free(vkExtensionProperties_Array);
    vkExtensionProperties_Array = NULL;

    /*
     * sub-step 5 : find whether above extension names contain our required 2 extensions ->
     *                  (1) VK_KHR_SURFACE_EXTENSION_NAME
     *                  (2) VK_KHR_WIN32_SURFACE_EXTENSION_NAME
     *
     *              Accordingly, set 2 global variables ->
     *                  (1) Required extension count
     *                  (2) Required extension names array
     */
    VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
    VkBool32 win32SurfaceExtensionFound = VK_FALSE;
    VkBool32 debugReportExtensionFound = VK_FALSE;

    for (uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        // using macros is recommended, instead of actual extension names
        if (strcmp(instanceExtensionNames_Array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
        {
            vulkanSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_Array[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }

        if (strcmp(instanceExtensionNames_Array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
        {
            win32SurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_Array[enabledInstanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
        }

        if (strcmp(instanceExtensionNames_Array[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
        {
            debugReportExtensionFound = VK_TRUE;
            if (TRUE == vkValidationEnabled)
                enabledInstanceExtensionNames_Array[enabledInstanceExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
            else
            {
                //array will not have entry of VK_EXT_DEBUG_REPORT_EXTENSION_NAME
            }
        }
    }

    /*
     * sub-step 6 : as not needed henceforth, free the local strings array
     */
    for (uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        free(instanceExtensionNames_Array[i]);
        instanceExtensionNames_Array[i] = NULL;
    }

    free(instanceExtensionNames_Array);
    instanceExtensionNames_Array = NULL;

    /*
     * sub-step 7 : print whether our Vulkan driver
     *              supports our required extensions or not
     */
    if (vulkanSurfaceExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hard-coded failure
        fprintf(gpFILE, "fillInstanceExtensionNames() : VK_KHR_SURFACE_EXTENSION_NAME not found.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "fillInstanceExtensionNames() : VK_KHR_SURFACE_EXTENSION_NAME found.\n");
    }

    if (win32SurfaceExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hard-coded failure
        fprintf(gpFILE, "fillInstanceExtensionNames() : VK_KHR_WIN32_SURFACE_EXTENSION_NAME not found.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "fillInstanceExtensionNames() : VK_KHR_WIN32_SURFACE_EXTENSION_NAME found.\n");
    }

    fprintf(gpFILE, LINE_END);


    if (debugReportExtensionFound == VK_FALSE)
    {
        if (true == vkValidationEnabled)
        {
            vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hard-coded failure
            fprintf(gpFILE, "fillInstanceExtensionNames() : Validation is on but VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found.\n");
            return(vkResult);
        }
        else
        {
            fprintf(gpFILE, "fillInstanceExtensionNames() : Validation is off & VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found.\n");
        }

    }
    else
    {
        if (true == vkValidationEnabled)
        {
            fprintf(gpFILE, "fillInstanceExtensionNames() : Validation is on & VK_EXT_DEBUG_REPORT_EXTENSION_NAME found.\n");
        }
        else
        {
            fprintf(gpFILE, "fillInstanceExtensionNames() : Validation is off & VK_EXT_DEBUG_REPORT_EXTENSION_NAME found.\n");
        }
    }

    /*
     * sub-step 8 : print only enabled extension names
     */
    for (uint32_t i = 0; i < enabledInstanceExtensionCount; i++)
    {
        fprintf(gpFILE, "fillInstanceExtensionNames() : Enabled Vulkan instance extension name = %s\n", enabledInstanceExtensionNames_Array[i]);
    }

    fprintf(gpFILE, LINE_END);

    return(vkResult);
}
VkResult VulkanContext::FillValidationLayerNames(void)
{
    //code
    VkResult vkResult = VK_SUCCESS;

    uint32_t validationLayerCount = 0;

    vkResult = vkEnumerateInstanceLayerProperties(
        &validationLayerCount,
        NULL
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "fillValidaionLayerNames() : first call to vkEnumerateInstanceLayerProperties() failed.\n");
        return(vkResult);
    }

    VkLayerProperties* vkLayerProperties_array = NULL;
    vkLayerProperties_array = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * validationLayerCount);


    vkResult = vkEnumerateInstanceLayerProperties(
        &validationLayerCount,
        vkLayerProperties_array
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "fillValidaionLayerNames() : second call to vkEnumerateInstanceLayerProperties() failed.\n");
        return(vkResult);
    }

    fprintf(gpFILE, LINE_END);

    char** validationLayerNames_array = NULL;
    validationLayerNames_array = (char**)malloc(sizeof(char*) * validationLayerCount);

    for (uint32_t i = 0; i < validationLayerCount; i++)
    {
        validationLayerNames_array[i] = (char*)malloc(sizeof(char) * (strlen(vkLayerProperties_array[i].layerName) + 1));
        memcpy(
            validationLayerNames_array[i],                         // destination
            vkLayerProperties_array[i].layerName,            // source
            strlen(vkLayerProperties_array[i].layerName) + 1 // length
        );
#ifdef PRINT_EXTENIONS
        fprintf(gpFILE, "fillValidaionLayerNames() : Vulkan instance extension name = %s\n", validationLayerNames_array[i]);
#endif //PRINT_EXTENIONS

    }

    fprintf(gpFILE, LINE_END);

    free(vkLayerProperties_array);
    vkLayerProperties_array = NULL;

    VkBool32 validationLayerFound = VK_FALSE;

    for (uint32_t i = 0; i < validationLayerCount; i++)
    {
        if (strcmp(validationLayerNames_array[i], "VK_LAYER_KHRONOS_validation") == 0)
        {
            validationLayerFound = VK_TRUE;
            enabledValidationLayerNames_array[enabledValidationLayerCount++] = "VK_LAYER_KHRONOS_validation";
        }
    }

    for (uint32_t i = 0; i < validationLayerCount; i++)
    {
        free(validationLayerNames_array[i]);
    }
    free(validationLayerNames_array);
    validationLayerNames_array = NULL;


    if (validationLayerFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hard-coded failure
        fprintf(gpFILE, "fillValidaionLayerNames() : VK_LAYER_KHRONOS_validation not found.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "fillValidaionLayerNames() : VK_LAYER_KHRONOS_validation found.\n");
    }

    for (uint32_t i = 0; i < enabledValidationLayerCount; i++)
    {
        fprintf(gpFILE, "fillValidaionLayerNames() : Enabled Vulkan instance extension name = %s\n", enabledValidationLayerNames_array[i]);
    }

    fprintf(gpFILE, LINE_END);

    return vkResult;
}
VkResult VulkanContext::FillDeviceExtensionNames(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    /*
     *  Find how many extensions are supported by the Vulkan
     *              driver of this version and keep it in a local variable
     */
    uint32_t deviceExtensionCount = 0;

    vkResult = vkEnumerateDeviceExtensionProperties(
        vkPhysicalDevice, // [in] VkPhysicalDevice 
        NULL,                      // [in, optional] layer name to retrieve extensions from (NULL means you want all extensions)
        &deviceExtensionCount,     // [out] count of supported extensions
        NULL                       // [out, optional] array of VkExtensionProperties to retrieve extension properties
    );
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "fillDeviceExtensionNames() : first call to vkEnumerateDeviceExtensionProperties() failed.\n");
        return(vkResult);
    }

    /*
     *  allocate and fill struct VkExtensionProperties array
     *              corresponding to above count
     */
    VkExtensionProperties* vkExtensionProperties_Array = NULL;
    vkExtensionProperties_Array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);


    vkResult = vkEnumerateDeviceExtensionProperties(
        vkPhysicalDevice,
        NULL,
        &deviceExtensionCount,
        vkExtensionProperties_Array
    );
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "fillDeviceExtensionNames() : second call to vkEnumerateDeviceExtensionProperties() failed.\n");
        return(vkResult);
    }

    /*
     *  fill & display a local string array of extension names
     *              obtained from VkExtensionProperties struct array
     */
    char** deviceExtensionNames_Array = NULL;
    deviceExtensionNames_Array = (char**)malloc(sizeof(char*) * deviceExtensionCount);

    fprintf(gpFILE, LINE_END);

    for (uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        deviceExtensionNames_Array[i] = (char*)malloc(sizeof(char) * (strlen(vkExtensionProperties_Array[i].extensionName) + 1));
        memcpy(
            deviceExtensionNames_Array[i],                           // destination
            vkExtensionProperties_Array[i].extensionName,            // source
            strlen(vkExtensionProperties_Array[i].extensionName) + 1 // length
        );

#ifdef PRINT_EXTENIONS
        fprintf(gpFILE, "fillDeviceExtensionNames() : Vulkan device extension name = %s\n", deviceExtensionNames_Array[i]);
#endif // PRINT_EXTENIONS

    }

    fprintf(gpFILE, LINE_END);

    /*
     *  as not required here onwards, free the VkExtensionProperties array
     */
    free(vkExtensionProperties_Array);
    vkExtensionProperties_Array = NULL;

    /*
     * find whether above extension names contain our required 1 extension ->
     *                  (1) VK_KHR_SWAPCHAIN_EXTENSION_NAME
     *
     *              Accordingly, set 2 global variables ->
     *                  (1) Required extension count
     *                  (2) Required extension names array
     */
    VkBool32 vulkanSwapchainExtensionFound = VK_FALSE;
    VkBool32 meshShaderExtensionFound = VK_FALSE;

    for (uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        // using macros is recommended, instead of actual extension names
        if (strcmp(deviceExtensionNames_Array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            vulkanSwapchainExtensionFound = VK_TRUE;
            enabledDeviceExtensionNames_Array[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        }

        if (strcmp(deviceExtensionNames_Array[i], VK_EXT_MESH_SHADER_EXTENSION_NAME) == 0)
        {
            meshShaderExtensionFound = VK_TRUE;
            enabledDeviceExtensionNames_Array[enabledDeviceExtensionCount++] =
                VK_EXT_MESH_SHADER_EXTENSION_NAME;
        }
    }

    /*
     * as not needed henceforth, free the local strings array
     */
    for (uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        free(deviceExtensionNames_Array[i]);
        deviceExtensionNames_Array[i] = NULL;
    }

    free(deviceExtensionNames_Array);
    deviceExtensionNames_Array = NULL;

    /*
     *  print whether our Vulkan driver
     *              supports our required extensions or not
     */
    if (vulkanSwapchainExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hard-coded failure
        fprintf(gpFILE, "fillDeviceExtensionNames() : VK_KHR_SWAPCHAIN_EXTENSION_NAME not found.\n");
        return(vkResult);
    }

    if (meshShaderExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_EXTENSION_NOT_PRESENT;
        fprintf(gpFILE,
            "fillDeviceExtensionNames() : VK_EXT_mesh_shader not found.\n");
        return(vkResult);
    }

    fprintf(gpFILE, LINE_END);

    /*
     *  print only enabled extension names
     */
    for (uint32_t i = 0; i < enabledDeviceExtensionCount; i++)
    {
        fprintf(gpFILE, "fillDeviceExtensionNames() : Enabled Vulkan device extension name = %s\n", enabledDeviceExtensionNames_Array[i]);
    }

    fprintf(gpFILE, LINE_END);

    return(vkResult);
}

VkResult VulkanContext::CreateValidationCallbackFunction(void)
{
    //code
    VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char*, const char*, void*);

    VkResult vkResult = VK_SUCCESS;

    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL;

    vkCreateDebugReportCallbackEXT_fnptr = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");

    if (vkCreateDebugReportCallbackEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFILE, "createValidationCallbackFunction() : vkGetInstanceProcAddr failed to get vkCreateDebugReportCallbackEXT_fnptr .\n");
        return vkResult;
    }

    //vkDebugReportCallbackEXT 
    //vkDestroyDebugReportCallbackEXT_fnptr ;

    vkDestroyDebugReportCallbackEXT_fnptr = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");

    if (vkDestroyDebugReportCallbackEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFILE, "createValidationCallbackFunction() : vkGetInstanceProcAddr failed to get vkDestroyDebugReportCallbackEXT_fnptr .\n");
        return vkResult;
    }

    //get the vulkan debug callback object // 
    VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT;
    memset(&vkDebugReportCallbackCreateInfoEXT, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));

    vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    vkDebugReportCallbackCreateInfoEXT.pNext = NULL;
    vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;// performance,profiling,informative
    vkDebugReportCallbackCreateInfoEXT.pfnCallback = debugReportCallback;
    vkDebugReportCallbackCreateInfoEXT.pUserData = NULL;

    vkResult = vkCreateDebugReportCallbackEXT_fnptr(vkInstance, &vkDebugReportCallbackCreateInfoEXT, NULL, &vkDebugReportCallbackEXT);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createValidationCallbackFunction() : vkCreateDebugReportCallbackEXT_fnptr failed.\n");
        return(vkResult);
    }

    return vkResult;
}
VkResult VulkanContext::PrintVkInfo(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code

    fprintf(gpFILE, "--------------------------Vulkan Info--------------------------\n");
    fprintf(gpFILE, LINE_END);

    /*
     * step (a) : Start a loop using global physical device count
     *            and inside it declare and memset() VkPhysicalDeviceProperties struct variable.
     */
    for (uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));

        /*
         * step (b) : Initialize this struct variable by calling vkGetPhysicalDeviceProperties() Vulkan API
         */
        vkGetPhysicalDeviceProperties(
            vkPhysicalDevice_Array[i],  // [in] VkPhysicalDevice
            &vkPhysicalDeviceProperties // [out] VkPhysicalDeviceProperties
        );

        /*
         * step (c) : Print Vulkan API version using “apiVersion” member of above struct. This requires 3 Vulkan macros.
         */
        uint32_t majorVersion = VK_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);

        fprintf(gpFILE, "printVkInfo() : API Version = %d.%d.%d\n", majorVersion, minorVersion, patchVersion);

        /*
         * step (d) : Print device name by using “deviceName” member of above struct.
         */
        fprintf(gpFILE, "printVkInfo() : Device Name = %s\n", vkPhysicalDeviceProperties.deviceName);

        /*
         * step (e) : Use the “deviceType” member of the above struct in a switch case block
         *            and accordingly print device type.
         */
        switch (vkPhysicalDeviceProperties.deviceType)
        {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            fprintf(gpFILE, "printVkInfo() : Device Type = Integrated GPU (iGPU)\n");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            fprintf(gpFILE, "printVkInfo() : Device Type = Discrete GPU (dGPU)\n");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            fprintf(gpFILE, "printVkInfo() : Device Type = Virtual GPU (vGPU)\n");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            fprintf(gpFILE, "printVkInfo() : Device Type = CPU\n");
            break;
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            fprintf(gpFILE, "printVkInfo() : Device Type = Other\n");
            break;
        default:
            fprintf(gpFILE, "printVkInfo() : Device Type = UNKNOWN\n");
            break;
        }

        /*
         * step (f) : Print hexadecimal vendor ID of the device using the “vendorID” member of the above struct.
         */
        fprintf(gpFILE, "printVkInfo() : Vendor ID   = 0x%04x\n", vkPhysicalDeviceProperties.vendorID);

        /*
         * step (g) : Print hexadecimal device ID using the “deviceID” member of the above struct.
         *
         * [Note : for the sake of completeness,
         * we can repeat step (5) –-> (a) to (h) from getPhysicalDevice()
         * but now instead of assigning selected queue and selected device,
         * print whether this device supports Graphics Bit, Compute Bit, Transfer Bit using if else-if blocks.
         * Similarly, we also can repeat device features from getPhysicalDevice() and can print all,
         * around 50+ device features including support for tessellation shader and geometry shader.]
         */
        fprintf(gpFILE, "printVkInfo() : Device ID   = 0x%04x\n", vkPhysicalDeviceProperties.deviceID);
    }

    //fprintf(gpFILE, LINE_END);

    /*
     *  Free physical device array here, which we removed from the if(bFound == VK_TRUE) block of getPhysicalDevice().
     */
    if (vkPhysicalDevice_Array)
    {
        free(vkPhysicalDevice_Array);
        vkPhysicalDevice_Array = NULL;

    }

    return(vkResult);
}

VkResult VulkanContext::CreateVulkanInstance(void)
{

    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // fill instance extension names
    vkResult = FillInstanceExtensionNames();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVulkanInstance() : FillInstanceExtensionNames() failed.\n");
        return(vkResult);
    }

    //fill  validation layer
    if (true == vkValidationEnabled)
    {

        vkResult = FillValidationLayerNames();
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createVulkanInstance() : FillValidationLayerNames() failed.\n");
            return(vkResult);
        }
    }
    /*
     *  initialize struct VkApplicationInfo
     */
	VkApplicationInfo vkApplicationInfo{};
    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // structure type (VkStructureType)
    vkApplicationInfo.pNext = NULL;                               // pointer to a structure extending this structure (linked list)
    vkApplicationInfo.pApplicationName = gpszAppName;                        // application name (can be anything, but to be meaningful, we will use the global app name)
    vkApplicationInfo.applicationVersion = 1;                                  // can be anything, we will just use 1 (developer-supplied version)
    vkApplicationInfo.pEngineName = gpszAppName;                        // engine name (again, can be anything, but to be meaningful, we will use the global app name)
    vkApplicationInfo.engineVersion = 1;                                  // can be anything, we will just use 1 (developer-supplied version)
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;                 // must be the highest Vulkan API Version

    /*
     *  initialize struct VkInstanceCreateInfo by using
     *              information from sub-step 1 and sub-step 2
     */
	VkInstanceCreateInfo vkInstanceCreateInfo{};
    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // structure type (VkStructureType)
    vkInstanceCreateInfo.pNext = NULL;                                   // pointer to a structure extending this structure (linked list)
    vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;                     // pointer to a VkApplicationInfo structure 
    vkInstanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;          // number of enabled Vulkan instance extensions
    vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_Array;    // array of enabled Vulkan instance extension names

    if (true == vkValidationEnabled)
    {
        vkInstanceCreateInfo.enabledLayerCount = enabledValidationLayerCount;
        vkInstanceCreateInfo.ppEnabledLayerNames = enabledValidationLayerNames_array;
    }
    else
    {
        vkInstanceCreateInfo.enabledLayerCount = 0;
        vkInstanceCreateInfo.ppEnabledLayerNames = NULL;
    }

    /*
     *  call vkCreateInstance() to get VkInstance in a
     *              global variable and do error checking
     */
    vkResult = vkCreateInstance(
        &vkInstanceCreateInfo, // [in] pointer to a VkInstanceCreateInfo structure
        NULL,                  // [in, optional] pointer to a custom memory allocator (NULL means use a default memory allocator)
        &vkInstance            // [out] pointer to a VkInstance handle  
    );

    if (vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
    {
        fprintf(gpFILE, "createVulkanInstance() : vkCreateInstance() failed due to incompatible driver (%d).\n", vkResult);
        return(vkResult);
    }
    else if (vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
    {
        fprintf(gpFILE, "createVulkanInstance() : vkCreateInstance() failed due to required extension not present (%d).\n", vkResult);
        return(vkResult);
    }
    else if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVulkanInstance() : vkCreateInstance() failed due to [unknown reason] (%d).\n", vkResult);
        return(vkResult);
    }
    //else
    //{
    //    fprintf(gpFILE, "createVulkanInstance() : vkCreateInstance() succeeded.\n");
    //}

    /*
     *  destroy VkInstance in uninitialize()
     */

     // code in uninitialize()

        //do for validation callbacks
    if (true == vkValidationEnabled)
    {
        vkResult = CreateValidationCallbackFunction();
    }

    return(vkResult);
}
VkResult VulkanContext::GetSupportedSurface(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    /*
     * sub-step 1 : Declare and memset() a platform-specific
     *              (Windows, Linux, Android, etc.) SurfaceCreateInfo structure.
     */
    VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR;
    memset((void*)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));

    /*
     * sub-step 2 : Initialize it, particularly its hinstance and hwnd members.
     */
    vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vkWin32SurfaceCreateInfoKHR.pNext = NULL;
    vkWin32SurfaceCreateInfoKHR.flags = 0;
    vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(gWindow.hwnd, GWLP_HINSTANCE); // this member can also be initialized by using "(HINSTANCE)GetModuleHandle(NULL);"
    vkWin32SurfaceCreateInfoKHR.hwnd = gWindow.hwnd;

    /*
     * sub-step 3 : Now call vkCreateWin32SurfaceKHR() to create the presentation surface object.
     */
    vkResult = vkCreateWin32SurfaceKHR(
        vkInstance,                   // [in] Vulkan instance object (until you get device, Vulkan instance will be used)
        &vkWin32SurfaceCreateInfoKHR, // [in] Surface create info's address
        NULL,                         // [in] memory allocator
        &gWindow.vkSurfaceKHR                 // [out] pointer to a VkSurfaceKHR object
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "getSupportedSurface() : vkCreateWin32SurfaceKHR() failed (%d).\n", vkResult);
        return(vkResult);
    }

    return(vkResult);
}
VkResult VulkanContext::GetPhysicalDevice(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    /*
     * sub-step 2 : Call vkEnumeratePhysicalDevices() to get physical device count.
     */
    vkResult = vkEnumeratePhysicalDevices(
        vkInstance,           // [in] Vulkan instance handle
        &physicalDeviceCount, // [out] count of available physical devices
        NULL                  // [out, optional] VkPhysicalDevice array  
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "getPhysicalDevice() : 1st call to vkEnumeratePhysicalDevices() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else if (physicalDeviceCount == 0)
    {
        fprintf(gpFILE, "getPhysicalDevice() : 1st call to vkEnumeratePhysicalDevices() resulted in 0 devices.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // hard-coded result
        return(vkResult);
    }


    /*
     * sub-step 3 : Allocate VkPhysicalDevice array according to above count.
     */
    vkPhysicalDevice_Array = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);

    /*
     * sub-step 4 : Call vkEnumeratePhysicalDevices() again to fill the above array.
     */
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_Array);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "getPhysicalDevice() : 2nd call to vkEnumeratePhysicalDevices() failed (%d).\n", vkResult);
        return(vkResult);
    }

    /*
     * sub-step 5 : Start a loop using the above physical device count and physical device array.
     */
    VkBool32 bFound = VK_FALSE;

    for (uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        /*
         * sub-sub-step (1) : Declare a local variable to hold queue count
         */
        uint32_t queueCount = UINT32_MAX;

        /*
         * sub-sub-step (2) : Call vkGetPhysicalDeviceQueueFamilyProperties() to
         *                    initialize the above queue count variable.
         */
        vkGetPhysicalDeviceQueueFamilyProperties(
            vkPhysicalDevice_Array[i], // [in] Vulkan physical device
            &queueCount,               // [out] Queue family count
            NULL                       // [out, optional] VkQueueFamilyProperties array 
        );

        /*
         * sub-sub-step (3) : Allocate VkQueueFamilyProperties array according to above count.
         */
        VkQueueFamilyProperties* vkQueueFamilyProperties_Array = NULL;
        vkQueueFamilyProperties_Array = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueCount);

        /*
         * sub-sub-step (4) : Call vkGetPhysicalDeviceQueueFamilyProperties() again to fill the above array.
         */
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_Array[i], &queueCount, vkQueueFamilyProperties_Array);

        /*
         * sub-sub-step (5) : Declare a VkBool32 type array and allocate it using the same above queue count.
         */
        VkBool32* isQueueSurfaceSupported_Array = NULL;
        isQueueSurfaceSupported_Array = (VkBool32*)malloc(sizeof(VkBool32) * queueCount);

        /*
         * sub-sub-step (6) : Start a nested loop and fill above VkBool32 type array by
         *                    calling vkGetPhysicalDeviceSurfaceSupportKHR().
         */
        for (uint32_t j = 0; j < queueCount; j++)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(
                vkPhysicalDevice_Array[i],        // [in] Vulkan physical device
                j,                                // [in] Queue family index
                gWindow.vkSurfaceKHR,                     // [in] VkSurfaceKHR object
                &isQueueSurfaceSupported_Array[j] // [out] is the queue family supported by the surface?
            );
        }

        /*
         * sub-sub-step (7) : Start another nested loop (not nested in above loop, but nested in main loop) and
         *                    check whether the physical device in its array with its queue family has the graphics bit or not.
         *                    If yes, then this is a selected physical device so assign it to the global variable.
         *                    Similarly, if this index is the selected queue family index, assign it to the global variable too.
         *                    Set bFound = VK_TRUE and break out from the 2nd nested loop.
         */
        for (uint32_t j = 0; j < queueCount; j++)
        {
            if (vkQueueFamilyProperties_Array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) // there are also COMPUTE / TRANSFER bits which can be checked
            {
                if (isQueueSurfaceSupported_Array[j] == VK_TRUE)
                {
                    vkPhysicalDevice = vkPhysicalDevice_Array[i];
                    vkGraphicsQueueFamilyIndex = j;
                    bFound = VK_TRUE;

                    break;
                }
            }
        }

        /*
         * sub-sub-step (8) : Now we are back in the main loop,
         *                    so free the 2 arrays -> Queue family array and the VkBool32 array.
         */
        if (isQueueSurfaceSupported_Array)
        {
            free(isQueueSurfaceSupported_Array);
            isQueueSurfaceSupported_Array = NULL;

        }

        if (vkQueueFamilyProperties_Array)
        {
            free(vkQueueFamilyProperties_Array);
            vkQueueFamilyProperties_Array = NULL;

        }

        /*
         * sub-sub-step (9) : Still being in the main loop, according to the bFound variable, break out from the main loop.
         */
        if (bFound == VK_TRUE)
        {
            break;
        }
    }

    /*
     * sub-step 6 : Do error checking according to the value of bFound.
     */
    if (bFound == VK_TRUE)
    {
        fprintf(gpFILE, "getPhysicalDevice() : succeeded to select required physical device with graphics enabled.\n");
    }
    else
    {
        if (vkPhysicalDevice_Array)
        {
            free(vkPhysicalDevice_Array);
            vkPhysicalDevice_Array = NULL;

        }

        fprintf(gpFILE, "getPhysicalDevice() : failed to select graphics supported physical device.\n");

        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }


    vkGetPhysicalDeviceMemoryProperties(
        vkPhysicalDevice,        // [in] Vulkan physical device
        &vkPhysicalDeviceMemoryProperties // [out] address to a structure of VkPhysicalDeviceMemoryProperties 
    );

    /*
     * sub-step 9 : Declare a local structure variable VkPhysicalDeviceFeatures, memset() it and
     *              initialize it by calling vkGetPhysicalDeviceFeatures().
     */
    VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures{};

    vkGetPhysicalDeviceFeatures(
        vkPhysicalDevice, // [in] Vulkan physical device
        &vkPhysicalDeviceFeatures  // [out] address to a structure of VkPhysicalDeviceFeatures
    );

    /*
     * sub-step 10 : By using the “tessellationShader” member of the above structure,
     *               check the selected device’s tessellation shader support.
     */
    if (vkPhysicalDeviceFeatures.tessellationShader == VK_TRUE)
    {
        fprintf(gpFILE, "getPhysicalDevice() : Selected physical device supports tessellation shader.\n");
    }
    else
    {
        fprintf(gpFILE, "getPhysicalDevice() : Selected physical device doesn't support tessellation shader.\n");
    }

    /*
     * sub-step 11 : By using the “geometryShader” member of the above structure,
     *               check the selected device’s geometry shader support.
     */
    if (vkPhysicalDeviceFeatures.geometryShader == VK_TRUE)
    {
        fprintf(gpFILE, "getPhysicalDevice() : Selected physical device supports geometry shader.\n");
    }
    else
    {
        fprintf(gpFILE, "getPhysicalDevice() : Selected physical device doesn't support geometry shader.\n");
    }

    return(vkResult);
}
VkResult VulkanContext::CreateVulkanDevice(void)
{

    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    /*
     *  Call previously created fillDeviceExtensionNames() in it.
     */

     //  fill and initialize required device extension names and count global variables
    vkResult = FillDeviceExtensionNames();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVulkanDevice() : FillDeviceExtensionNames() failed.\n");
        return(vkResult);
    }

    /*
     *  Declare and initialize VkDeviceCreateInfo structure.
     *              Use previously obtained device extension count and
     *              device extension array to initialize this structure.
     */
     // newly added code (after vkGetDeviceQueue() was returning VK_NULL_HANDLE)
    VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
    memset((void*)&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));

    float queuePriorities[1];
    queuePriorities[0] = 0.0f;

    vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vkDeviceQueueCreateInfo.pNext = NULL;
    vkDeviceQueueCreateInfo.flags = 0;
    vkDeviceQueueCreateInfo.queueFamilyIndex = vkGraphicsQueueFamilyIndex;
    vkDeviceQueueCreateInfo.queueCount = 1;
    vkDeviceQueueCreateInfo.pQueuePriorities = queuePriorities; // default queue priority


    //ZzNeO features
    //VkPhysicalDeviceFeatures enabledFeatures;
    //memset((void*)&enabledFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
    //enabledFeatures.samplerAnisotropy = VK_TRUE; // enable anisotropic filtering
    //enabledFeatures.tessellationShader = VK_TRUE; // enable tessellation shader

    //enabledFeatures.geometryShader = VK_TRUE; // enable geometry shader

        //------------------------------//


    VkPhysicalDeviceVulkan12Features vulkan12Features{};
    vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

    // Required for uint8_t storage-buffer elements
    vulkan12Features.storageBuffer8BitAccess = VK_TRUE;
    vulkan12Features.runtimeDescriptorArray = VK_TRUE;
    vulkan12Features.descriptorBindingPartiallyBound = VK_TRUE;
    vulkan12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;
    vulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    vulkan12Features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    vulkan12Features.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
    vulkan12Features.timelineSemaphore = VK_TRUE;
    vulkan12Features.bufferDeviceAddress = VK_TRUE;
    vulkan12Features.pNext = nullptr;

    //Features vulkan13
    VkPhysicalDeviceVulkan13Features vkPhysicalDeviceVulkan13Features{};
    vkPhysicalDeviceVulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    vkPhysicalDeviceVulkan13Features.dynamicRendering = VK_TRUE;
    vkPhysicalDeviceVulkan13Features.synchronization2 = VK_TRUE;
    vkPhysicalDeviceVulkan13Features.shaderDemoteToHelperInvocation = VK_TRUE;
    vkPhysicalDeviceVulkan13Features.maintenance4 = VK_TRUE;
    vkPhysicalDeviceVulkan13Features.pNext = &vulkan12Features;

    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{};
    meshShaderFeatures.sType =VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
    meshShaderFeatures.taskShader = VK_TRUE;
    meshShaderFeatures.meshShader = VK_TRUE;
    meshShaderFeatures.pNext = &vkPhysicalDeviceVulkan13Features;

    VkPhysicalDeviceFeatures2 features2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.features.samplerAnisotropy = VK_TRUE; // enable anisotropic filtering
    features2.features.tessellationShader = VK_TRUE; // enable tessellation shader
    features2.features.shaderInt64 = VK_TRUE;

    features2.pNext = &meshShaderFeatures;

    VkDeviceCreateInfo vkDeviceCreateInfo;
    memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));

    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pNext = &features2; // for dynamic rendering
    vkDeviceCreateInfo.flags = 0;
    vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
    vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_Array;
    vkDeviceCreateInfo.enabledLayerCount = 0;
    vkDeviceCreateInfo.ppEnabledLayerNames = NULL;
    vkDeviceCreateInfo.pEnabledFeatures = NULL; // for dynamic rendering, this should be NULL and features should be passed by pNext chain as above, but if not using dynamic rendering, then this should point to enabledFeatures variable

    // newly added code (after vkGetDeviceQueue() was returning VK_NULL_HANDLE)
    vkDeviceCreateInfo.queueCreateInfoCount = 1;
    vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;

    /*
     *  Now call vkCreateDevice() Vulkan API to actually
     *              create the Vulkan device and do error-checking.
     */
    vkResult = vkCreateDevice(
        vkPhysicalDevice, // [in] Vulkan physical device handle
        &vkDeviceCreateInfo,       // [in] VkDeviceCreateInfo*
        NULL,                      // [in, optional] pointer to a custom memory allocator
        &vkDevice                  // [out] VkDevice*
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVulkanDevice() : vkCreateDevice() failed.\n");
        return(vkResult);
    }

    return(vkResult);
}

VkResult VulkanContext::GetDeviceQueue(void)
{

	VkResult vkResult = VK_SUCCESS;
    // code
    /*
     *  Call vkGetDeviceQueue() using newly created VkDevice,
     *              selected family index, 0th queue in that selected queue family.
     */
    vkGetDeviceQueue(
        vkDevice,                          // [in] vulkan logical device handle 
        vkGraphicsQueueFamilyIndex, // [in] selected queue family index
        0,                                 // [in] queue family index
        &vkGraphicsQueue                           // [out] VkQueue* 
    );

    if (vkGraphicsQueue == VK_NULL_HANDLE)
    {
        fprintf(gpFILE, "getDeviceQueue() : vkGetDeviceQueue() returned NULL for VkQueue.\n");
		return VK_ERROR_INITIALIZATION_FAILED;
    }

	return(vkResult);
}

VkResult VulkanContext::CreateCommandPool(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    // Declare and initialize VkCommandPoolCreateInfo structure.
    VkCommandPoolCreateInfo vkCommandPoolCreateInfo;
    memset((void*)&vkCommandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));

    vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vkCommandPoolCreateInfo.pNext = NULL;
    vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCommandPoolCreateInfo.queueFamilyIndex = vkGraphicsQueueFamilyIndex;

    // Call vkCreateCommandPool() to create the command pool
    vkResult = vkCreateCommandPool(
        vkDevice,                 // [in] VkDevice
        &vkCommandPoolCreateInfo, // [in] VkCommandPoolCreateInfo *
        NULL,                     // [in] custom memory allocator
        &vkCommandPool            // [out] VkCommandPool *
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createCommandPool() : vkCreateCommandPool() failed.\n");
        return(vkResult);
    }

    return(vkResult);
}

