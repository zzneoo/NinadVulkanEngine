#include "SwapchainContext.h"

SwapchainContext gSwapchain;

VkResult SwapchainContext::Initialize()
{
	// Initialization code for the swapchain context
	// This would typically involve creating the swapchain, setting up image views, etc.

	VkResult vkResult = VK_SUCCESS;
        // Create Swapchain
    vkResult = CreateSwapchain(VK_FALSE);
    if (vkResult != VK_SUCCESS)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // why are we using hard-coded value here? Sir will tell us at 23rd / 24th step (resize())

        fprintf(gpFILE, "initialize() : CreateSwapchain() failed (%d).\n", vkResult);
        return(vkResult);
    }

    vkResult = CreateSwapchainResources();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : CreateSwapchainResources() failed (%d).\n", vkResult);
        return(vkResult);
    }

	return VK_SUCCESS;
}

void SwapchainContext::Shutdown()
{
	// Cleanup code for the swapchain context
	// This would typically involve destroying the swapchain, image views, etc.
    DestroySwapchainResources();
	DestroySwapchain();

}



VkResult SwapchainContext::CreateSwapchain(VkBool32 vsync) // vsync == vertical synchronisation
{
    // function declarations
    VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void);
    VkResult getPhysicalDevicePresentMode(void);

    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    /*
     * sub-step 1 : Get physical device surface supported color format and physical device surface supported color space using Step (10).
     */
    vkResult = getPhysicalDeviceSurfaceFormatAndColorSpace();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createSwapchain() : getPhysicalDeviceSurfaceFormatAndColorSpace() failed (%d).\n", vkResult);
        return(vkResult);
    }

    /*
     * sub-step 2 : Get physical device surface capabilities by using Vulkan API vkGetPhysicalDeviceSurfaceCapabilitiesKHR()
     *              and accordingly initialize VkSurfaceCapabilitiesKHR structure.
     */
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
    memset((void*)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));

    vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        gVulkanContext.vkPhysicalDevice, // [in] vulkan physical device
        gWindow.vkSurfaceKHR,              // [in] vulkan surface
        &vkSurfaceCapabilitiesKHR  // [out] pointer to a VkSurfaceCapabilitiesKHR structure 
    );
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createSwapchain() : vkGetPhysicalDeviceSurfaceCapabilitiesKHR() failed (%d).\n", vkResult);
        return(vkResult);
    }

    /*
     * sub-step 3 : By using minImageCount and maxImageCount members of above structure,
     *              decide desired image count of swapchain. (Remember : swapchain is a set of images)
     */
    uint32_t testingNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;
    uint32_t desiredNumberOfSwapchainImages = 0;

    if (vkSurfaceCapabilitiesKHR.maxImageCount > 0 && vkSurfaceCapabilitiesKHR.maxImageCount < testingNumberOfSwapchainImages)
    {
        desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
    }
    else
    {
        desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount;
    }

    fprintf(gpFILE, "createSwapchain() : minImageCount = %u, maxImageCount = %u\n", vkSurfaceCapabilitiesKHR.minImageCount, vkSurfaceCapabilitiesKHR.maxImageCount);

    /*
     * sub-step 4 : By using currentExtent.width and currentExtent.height members of above structure
     *              and comparing them with current width and height of window, decide image width and
     *              image height of the swapchain.
     */
    memset((void*)&vkExtent2D, 0, sizeof(VkExtent2D));

    if (vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
    {
        vkExtent2D.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
        vkExtent2D.height = vkSurfaceCapabilitiesKHR.currentExtent.height;

        fprintf(gpFILE, "createSwapchain() : (1) Swapchain Image width x height = %u x %u\n",
            vkExtent2D.width,
                vkExtent2D.height); // using %u because width and height are unsigned integers
    }
    else
    {
        // if surface size is already defined, then swapchain image size MUST match with it
        VkExtent2D vkExtent2D;
        memset((void*)&vkExtent2D, 0, sizeof(VkExtent2D));

        vkExtent2D.width = (uint32_t)width;
        vkExtent2D.height = (uint32_t)height;

        vkExtent2D.width = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width)); // clamp the width between minImageExtent.width and maxImageExtent.width
        vkExtent2D.height = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.height, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height)); // clamp the height between minImageExtent.height and maxImageExtent.height

        /*
         * Example of clamping between minimum and maximum values:
         *
         *          max(2, min(4, 3))
         *          max(2, 3)
         *          3
         */

        fprintf(gpFILE, "createSwapchain() : (2) Swapchain Image width x height = %u x %u\n",
            vkExtent2D.width,
            vkExtent2D.height); // using %u because width and height are unsigned integers
    }

    /*
     * sub-step 5 : Decide how we are going to use the swapchain images.
     *              Means, whether we are going to store image data
     *              and (1) use it later (Deferred Rendering)
     *              or  (2) we are going to use it immediately as color attachment.
     *
     *              [So we are setting the swapchain image usage flags]
     */
    VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT; // VkImageUsageFlags is an enum

    /*
     * sub-step 6 : Swapchain is capable of storing the transformed image before presentation which is called “pre-transform”.
     *              While creating swapchain, we can decide whether to pre-transform
     *              or not the swapchain images (pre-transform also includes flipping of image).
     */
    VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR; // VkSurfaceTransformFlagBitsKHR is an enum

    if (vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;
    }


    // Get presentation mode for swapchain images using Step (11).

    vkResult = getPhysicalDevicePresentMode();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createSwapchain() : getPhysicalDevicePresentMode() failed (%d).\n", vkResult);
        return(vkResult);
    }

    //sub-step 8 : According to the above data, declare, memset() and initialize VkSwapchainCreateInfoKHR structure.

    VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
    memset((void*)&vkSwapchainCreateInfoKHR, 0, sizeof(VkSwapchainCreateInfoKHR));

    vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vkSwapchainCreateInfoKHR.pNext = NULL;
    vkSwapchainCreateInfoKHR.flags = 0;
    vkSwapchainCreateInfoKHR.surface = gWindow.vkSurfaceKHR;
    vkSwapchainCreateInfoKHR.minImageCount = desiredNumberOfSwapchainImages;
    vkSwapchainCreateInfoKHR.imageFormat = vkFormat;
    vkSwapchainCreateInfoKHR.imageColorSpace = vkColorSpace;
    vkSwapchainCreateInfoKHR.imageExtent.width = vkExtent2D.width;
    vkSwapchainCreateInfoKHR.imageExtent.height = vkExtent2D.height;
    vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
    vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
    vkSwapchainCreateInfoKHR.imageArrayLayers = 1;                                 // used for layered rendering (eg. in mobiles) 
    vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;         // for sharing swapchain across queues (exclusive means don't share)
    vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // same giving glClearColor(0.0f, 0.0f, 0.0f, 1.0f) -> the last 1.0f
    vkSwapchainCreateInfoKHR.presentMode = vkPresentModeKHR;
    vkSwapchainCreateInfoKHR.clipped = VK_TRUE;

    // oldSwapchain member will be used in resize() later


    // At the end, call vkCreateSwapchainKHR() Vulkan API to create the swapchain.

    vkResult = vkCreateSwapchainKHR(
        gVulkanContext.vkDevice,                  // [in] vulkan device handle
        &vkSwapchainCreateInfoKHR, // [in] pointer to a VkSwapchainCreateInfoKHR structure
        NULL,                      // [in, optional] custom memory allocator
        &vkSwapchainKHR            // [out] pointer to VkSwapchainKHR 
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createSwapchain() : vkCreateSwapchainKHR() failed (%d).\n", vkResult);
        return(vkResult);
    }

    // When done, destroy it in uninitialize() by using vkDestroySwapchain() Vulkan API.


   // remaining code in uninitialize()

    return(vkResult);
}

void SwapchainContext::DestroySwapchain(void)
{

    if (vkSwapchainKHR != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(
            gVulkanContext.vkDevice, // [in] vulkan device handle
            vkSwapchainKHR, // [in] swapchain to destroy
            NULL                       // [in, optional] custom memory allocator
        );
        vkSwapchainKHR = VK_NULL_HANDLE;
    }
}

VkResult SwapchainContext::CreateSwapchainResources(void)
{
    VkResult vkResult = VK_SUCCESS;
    //image and image view
    vkResult = CreateImagesAndImageViews();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createSwapchainResources() -> CreateImagesAndImageViews() failed.\n");
        return vkResult;
    }

    ////depth resources
    //vkResult = CreateDepthResources();
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "createSwapchainResources() -> CreateDepthResources() failed.\n");
    //    return vkResult;
    //}

    return vkResult;
}

void SwapchainContext::DestroySwapchainResources(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;
    // code
    //Swapchain images and image views-------------------------------------

        // destroy swapchain images and image views
    for (uint32_t i = 0; i < imageCount; i++)
    {
        vkDestroyImageView(gVulkanContext.vkDevice, resourceData.swapchainImageView_Array[i], NULL);
        resourceData.swapchainImageView_Array[i] = VK_NULL_HANDLE;

        //vkDestroyImage(gVulkanContext.vkDevice, swapchainImage_Array[i], NULL);// Not needed, as images are managed by the swapchain  
        resourceData.swapchainImage_Array[i] = VK_NULL_HANDLE;
    }

    if (resourceData.swapchainImageView_Array)
    {
        free(resourceData.swapchainImageView_Array);
        resourceData.swapchainImageView_Array = NULL;
    }
    if (resourceData.swapchainImage_Array)
    {
        free(resourceData.swapchainImage_Array);
        resourceData.swapchainImage_Array = NULL;
    }

    /*
    //destroy depth resources
    for (uint32_t i = 0; i < MAX_FRAMES; i++)
    {
        //image view
        if (resourceData.imageData_depthBuffer[i].vkImageView)
        {
            vkDestroyImageView(gVulkanContext.vkDevice, resourceData.imageData_depthBuffer[i].vkImageView, NULL);
            resourceData.imageData_depthBuffer[i].vkImageView = VK_NULL_HANDLE;
        }
        //image
        if (resourceData.imageData_depthBuffer[i].vkImage)
        {
            vkDestroyImage(gVulkanContext.vkDevice, resourceData.imageData_depthBuffer[i].vkImage, NULL);
            resourceData.imageData_depthBuffer[i].vkImage = VK_NULL_HANDLE;
        }
        // free depth buffer memory
        if (resourceData.imageData_depthBuffer[i].vkDeviceMemory)
        {
            vkFreeMemory(gVulkanContext.vkDevice, resourceData.imageData_depthBuffer[i].vkDeviceMemory, NULL);
            resourceData.imageData_depthBuffer[i].vkDeviceMemory = VK_NULL_HANDLE;
        }
    }

    if (resourceData.imageData_depthBuffer)
    {
        free(resourceData.imageData_depthBuffer);
        resourceData.imageData_depthBuffer = NULL;
    }
    */

    //---------------------------------------------------------------------
}

VkResult SwapchainContext::CreateImagesAndImageViews(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    // Get swapchain image count in a global variable using vkGetSwapchainImagesKHR().
    vkResult = vkGetSwapchainImagesKHR(
        gVulkanContext.vkDevice,             // [in] VkDevice (logical device)
        vkSwapchainKHR,       // [in] VkSwapchainKHR
        &imageCount, // [out] Swapchain Image Count
        NULL                  // [out, optional] Swapchain Image array
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createImagesAndImageViews() : vkGetSwapchainImagesKHR()'s 1st call failed.\n");
        return(vkResult);
    }
    else if (imageCount == 0)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFILE, "createImagesAndImageViews() : Swapchain image count is 0.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createImagesAndImageViews() : gives swapchain image count = %d\n", imageCount);
    }

    // Declare a global VkImage array and allocate it to the swapchain image count using malloc().
    resourceData.swapchainImage_Array = (VkImage*)malloc(sizeof(VkImage) * imageCount);
    // Now call the same function again, which we called in step 1 and fill this array.
    vkResult = vkGetSwapchainImagesKHR(
        gVulkanContext.vkDevice,
        vkSwapchainKHR,
        &imageCount,
        resourceData.swapchainImage_Array
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createImagesAndImageViews() : vkGetSwapchainImagesKHR()'s 2nd call failed.\n");
        return(vkResult);
    }

    //  Declare another global array of type VkImageView and allocate it to the size of swapchain image count.
    resourceData.swapchainImageView_Array = (VkImageView*)malloc(sizeof(VkImageView) * imageCount);

    // Declare and initialize VkImageViewCreateInfo struct except its “.image” member.
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = vkFormat;
    vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R; // part of VkComponentMapping
    vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // aspectMask => which part of the image or whole of the image is going to be affected by image barrier
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.image = VK_NULL_HANDLE;

    //  Now, start a loop for swapchain image count and inside this loop initialize the above “.image” member 
    //              to the swapchain image array index we obtained above and then call vkCreateImageView() API 
    //              to fill the above image view array.
    for (uint32_t i = 0; i < imageCount; i++)
    {
        vkImageViewCreateInfo.image = resourceData.swapchainImage_Array[i];

        vkResult = vkCreateImageView(
            gVulkanContext.vkDevice,                    // [in] VkDevice
            &vkImageViewCreateInfo,      // [in] VkImageViewCreateInfo *
            NULL,                        // [in] custom memory allocator
            &resourceData.swapchainImageView_Array[i] // [out] VkImageView * 
        );

        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createImagesAndImageViews() : vkCreateImageView() failed for iteration %d. (%d)\n", i, vkResult);
            return(vkResult);
        }
    }

    return(vkResult);
}

VkResult SwapchainContext::GetSupportedDepthFormat(VkFormat* pVkFormat)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    VkFormat formats[] = {
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D16_UNORM_S8_UINT,
    };

    for (uint32_t i = 0; i < sizeof(formats) / sizeof(formats[0]); i++)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(gVulkanContext.vkPhysicalDevice, formats[i], &formatProperties);
        if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            *pVkFormat = formats[i];
            return VK_SUCCESS;
        }
    }


    return vkResult;
}

VkResult SwapchainContext::CreateDepthResources(void)
{
    VkResult vkResult = VK_SUCCESS;

    // Find the depth format supported by the device
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    vkResult = GetSupportedDepthFormat(&depthFormat);
    if (vkResult != VK_SUCCESS || (depthFormat == VK_FORMAT_UNDEFINED))
    {
        fprintf(gpFILE, "Failed to get supported depth format.\n");
        return vkResult;
    }

    VkImageCreateInfo imageInfo;
    memset(&imageInfo, 0, sizeof(VkImageCreateInfo));

    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = NULL;
    imageInfo.flags = 0;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = WIN_WIDTH;
    imageInfo.extent.height = WIN_HEIGHT;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = depthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    resourceData.imageData_depthBuffer = (ImageData*)malloc(sizeof(ImageData) * MAX_FRAMES);

    for (uint32_t i = 0; i < MAX_FRAMES; i++)
    {
        vkResult = vkCreateImage(gVulkanContext.vkDevice, &imageInfo, NULL, &resourceData.imageData_depthBuffer[i].vkImage);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "Failed to create depth image.\n");
            return vkResult;
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(gVulkanContext.vkDevice, resourceData.imageData_depthBuffer[i].vkImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo;
        memset(&allocInfo, 0, sizeof(VkMemoryAllocateInfo));

        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = gVulkanContext.FindMemoryType(
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        vkResult = vkAllocateMemory(gVulkanContext.vkDevice, &allocInfo, NULL, &resourceData.imageData_depthBuffer[i].vkDeviceMemory);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "Failed to allocate depth image memory.\n");
            return vkResult;
        }


        vkBindImageMemory(gVulkanContext.vkDevice, resourceData.imageData_depthBuffer[i].vkImage, resourceData.imageData_depthBuffer[i].vkDeviceMemory, 0);


        //Image View 
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = resourceData.imageData_depthBuffer[i].vkImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_D32_SFLOAT;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        vkResult = vkCreateImageView(gVulkanContext.vkDevice, &viewInfo, nullptr, &resourceData.imageData_depthBuffer[i].vkImageView);
        if (vkResult != VK_SUCCESS) {
            fprintf(gpFILE, "Failed to create depth image view.\n");
            return vkResult;
        }
    }


    return vkResult;
}