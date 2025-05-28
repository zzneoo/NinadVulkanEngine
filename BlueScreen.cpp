// header files
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

// Vulkan related header files
#define VK_USE_PLATFORM_WIN32_KHR // define the current Vulkan platform
#include <vulkan/vulkan.h>        // you must define platform before including this file (Windows / Linux / macOS / iOS / Android / <other>)

#include "VK.h"
#include <cstdint>

// Vulkan related libraries
#pragma comment(lib, "vulkan-1.lib")

// macros
#define WIN_WIDTH  800
#define WIN_HEIGHT 600
#define WIN_TITLE  TEXT(" Vulkan ")
#define LINE_END     "-------------------------------------------------------------------------------------\n"

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variable declarations
const char* gpszAppName = "ARTR";
HWND            ghwnd = NULL;
BOOL            gbFullscreen = FALSE;
BOOL            gbActive = FALSE;
WINDOWPLACEMENT wpPrev;
DWORD           dwStyle;

// for file IO
FILE* gpFILE = NULL;

// Vulkan related global variables
// Instance extension related variables
uint32_t enabledInstanceExtensionCount = 0;

// 1. VK_KHR_SURFACE_EXTENSION_NAME
// 2. VK_KHR_WIN32_SURFACE_EXTENSION_NAME
// 3. VK_EXT_DEBUG_REPORT_EXTENSION_NAME
const char* enabledInstanceExtensionNames_Array[3];

// Vulkan Instance
VkInstance vkInstance = VK_NULL_HANDLE;

// Vulkan Presentation Surface
VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

// Vulkan Physical Device related global variables
VkPhysicalDevice                 vkPhysicalDevice_Selected = VK_NULL_HANDLE;
uint32_t                         graphicsQueueFamilyIndex_Selected = UINT32_MAX;
uint32_t                         physicalDeviceCount = 0;
VkPhysicalDevice* vkPhysicalDevice_Array = NULL;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;

// Vulkan Device Extension related variables
uint32_t enabledDeviceExtensionCount = 0;

// 1. VK_KHR_SWAPCHAIN_EXTENSION_NAME
const char* enabledDeviceExtensionNames_Array[1]; 

// Logical Device
VkDevice vkDevice = VK_NULL_HANDLE;

// Device Queue
VkQueue vkQueue = VK_NULL_HANDLE;

// Color format and color space
VkFormat        vkFormat_Color = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

// Presentation mode
VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;

// Swapchain
int winWidth = WIN_WIDTH;
int winHeight = WIN_HEIGHT;

VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE;
VkExtent2D     vkExtent2D_Swapchain;

// Swapchain images and swapchain image views
uint32_t     swapchainImageCount = UINT32_MAX;
VkImage* swapchainImage_Array = NULL;
VkImageView* swapchainImageView_Array = NULL;

// Command pool
VkCommandPool vkCommandPool = VK_NULL_HANDLE;

// Command buffers
VkCommandBuffer* vkCommandBuffer_Array = NULL;

// RenderPass
VkRenderPass vkRenderPass = VK_NULL_HANDLE;

// Framebuffers
VkFramebuffer* vkFramebuffer_Array = NULL;

// Semaphores
VkSemaphore* vkSemaphore_BackBuffer = VK_NULL_HANDLE;
VkSemaphore* vkSemaphore_RenderComplete = VK_NULL_HANDLE;

// Fences
VkFence* vkFence_Array = VK_NULL_HANDLE;

// Clear Color
VkClearColorValue vkClearColorValue;

// For Rendering
BOOL     bInitialized = FALSE;
uint32_t currentImageIndex = UINT32_MAX;

//validation
BOOL bValidation = TRUE;
uint32_t enabledValidationLayerCount = 0;
const char* enabledValidationLayerNames_array[1];//for VK_LAYER_KHRONOS_validation
VkDebugReportCallbackEXT vkDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_fnptr = NULL;

//vertex buffer related variables
typedef struct
{
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
}VertexData;

// position
VertexData vertexData_position;

//shader related variables
VkShaderModule vkShaderModule_vertex_shader = VK_NULL_HANDLE;
VkShaderModule vkShaderModule_fragment_shader = VK_NULL_HANDLE;

//desccriptor set layout 
VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;

//vkPipeline Layout
VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;

//pipeline
VkViewport vkViewport;
VkRect2D vkRect2D_Scissor;
VkPipeline vkPipeline = VK_NULL_HANDLE;

// entry-point function
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpszCmdLine,_In_ int iCmdShow)
{
    // function prototypes
    VkResult initialize(void);
    VkResult display(void);
    void update(void);
    void uninitialize();

    // local variable declarations
    MSG        msg;
    WNDCLASSEX wndclass;
    TCHAR      szAppName[255];
    HWND       hwnd = NULL;
    BOOL       bDone = FALSE;
    VkResult   vkResult = VK_SUCCESS;

    // code
    // open the log file
     fopen_s(&gpFILE,"Log.txt", "w");
    if (gpFILE == NULL)
    {
        MessageBox(NULL, TEXT("WinMain() : fopen() failed to open the log file."), TEXT("Error"), MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(gpFILE, "WinMain() : Program started successfully.\n");
        fprintf(gpFILE, LINE_END);
    }

    // copy the global app name into the local app name
	wsprintf(szAppName, TEXT("%s"), gpszAppName);

    // zero-out the window class
    ZeroMemory(&wndclass, sizeof(WNDCLASSEX));

    // fill the window class
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpfnWndProc = WndProc;
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

    // register the window class
    RegisterClassEx(&wndclass);

    // create the main window in memory
    hwnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        szAppName,
        WIN_TITLE,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
        (GetSystemMetrics(SM_CXSCREEN) - WIN_WIDTH) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - WIN_HEIGHT) / 2,
        WIN_WIDTH,
        WIN_HEIGHT,
        HWND_DESKTOP,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd)
    {
        MessageBox(NULL, TEXT("WinMain() : CreateWindowEx() failed."), TEXT("Error"), MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }

    // copy the local window handle into the global window handle
    ghwnd = hwnd;

    // initialization
    vkResult = initialize();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "WinMain() : initialize() failed.\n");
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        fprintf(gpFILE, LINE_END);
        fprintf(gpFILE, "WinMain() : initialize() succeeded.\n");
    }

    // show the window
    if (hwnd)
    {
        ShowWindow(hwnd, iCmdShow);
        SetForegroundWindow(hwnd);
        SetFocus(hwnd);
    }


    // gameloop
    while (bDone == FALSE)
    {
        ZeroMemory(&msg, sizeof(MSG));
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bDone = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else if (gbActive)
        {
            // Render
            display();

            // Update
            update();
        }
    }

    // uninitialization
    uninitialize();

    // un-register the window class
    UnregisterClass(szAppName, hInstance);

    return((int)msg.wParam);
}

// callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    // function declarations
    void resize(int width, int height);
    void ToggleFullscreen(void);

    // code
    switch (iMsg)
    {
    case WM_CREATE:
        memset(&wpPrev, 0, sizeof(WINDOWPLACEMENT));
        wpPrev.length = sizeof(WINDOWPLACEMENT);
        break;
    case WM_SETFOCUS:
        gbActive = TRUE;
        break;
    case WM_KILLFOCUS:
        gbActive = FALSE;
        break;
    case WM_ERASEBKGND:
        return(0);
    case WM_SIZE:
        resize(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_KEYDOWN:
        switch (LOWORD(wParam))
        {
        case VK_ESCAPE:
            DestroyWindow(hwnd);
            break;
        default:
            break;
        }
        break;
    case WM_CHAR:
        switch (LOWORD(wParam))
        {
        case 'F':
        case 'f':
            ToggleFullscreen();
            break;
        default:
            break;
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        break;
    }

    return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

// toggle fullscreen
void ToggleFullscreen(void)
{
    // local variables
    MONITORINFO monitorInfo = { sizeof(MONITORINFO) };

    // code
    // if window isn't in fullscreen mode
    if (gbFullscreen == FALSE)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
        if (dwStyle & WS_OVERLAPPEDWINDOW)
        {
            if (GetWindowPlacement(ghwnd, &wpPrev) &&
                GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &monitorInfo))
            {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(
                    ghwnd,
                    HWND_TOP,
                    monitorInfo.rcMonitor.left,
                    monitorInfo.rcMonitor.top,
                    monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                    monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                    SWP_NOZORDER | SWP_FRAMECHANGED
                );

                ShowCursor(FALSE);

                gbFullscreen = TRUE;
            }
        }
    }
    else // window is already in fullscreen mode
    {
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPos(
            ghwnd,
            HWND_TOP,
            0,
            0,
            0,
            0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED
        );

        ShowCursor(TRUE);

        gbFullscreen = FALSE;
    }
}



VkResult initialize(void)
{
    // function declarations
    VkResult createVulkanInstance(void);
    VkResult getSupportedSurface(void);
    VkResult getPhysicalDevice(void);
    VkResult printVkInfo(void);
    VkResult createVulkanDevice(void);
    void getDeviceQueue(void);
    VkResult createSwapchain(VkBool32);
    VkResult createImagesAndImageViews(void);
    VkResult createCommandPool(void);
    VkResult createCommandBuffers(void);
    VkResult createVertexBuffer(void);
    VkResult createShaders(void);
	VkResult createDiscriptorSetLayout(void);
	VkResult createPipelineLayout(void);

    VkResult createRenderPass(void);
	VkResult createGraphicsPipeline(void);
    VkResult createFramebuffers(void);
    VkResult createSemaphores(void);
    VkResult createFences(void);
    VkResult buildCommandBuffers(void);

    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    // STEP 3 : Create Vulkan instance
    vkResult = createVulkanInstance();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createVulkanInstance() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : createVulkanInstance() succeeded.\n");
    }

    // STEP 4 : Create Vulkan Presentation Surface
    vkResult = getSupportedSurface();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : getSupportedSurface() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : getSupportedSurface() succeeded.\n");
    }

    // STEP 5 : Enumerate and select required physical device and its queue family index
    vkResult = getPhysicalDevice();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : getPhysicalDevice() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : getPhysicalDevice() succeeded.\n");
    }

    // STEP 6 : Print Vulkan information
    vkResult = printVkInfo();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : printVkInfo() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : printVkInfo() succeeded.\n");
    }

    // STEP 8 : Create Vulkan (Logical) Device
    vkResult = createVulkanDevice();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createVulkanDevice() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : createVulkanDevice() succeeded.\n");
    }

    // STEP 9 : Create Vulkan Device Queue
    getDeviceQueue();

    // STEP 12 : Create Swapchain
    vkResult = createSwapchain(VK_FALSE);
    if (vkResult != VK_SUCCESS)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // why are we using hard-coded value here? Sir will tell us at 23rd / 24th step (resize())

        fprintf(gpFILE, "initialize() : createSwapchain() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : createSwapchain() succeeded.\n");
    }

    // STEP 13 : Create Vulkan images and image views
    vkResult = createImagesAndImageViews();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createImagesAndImageViews() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : createImagesAndImageViews() succeeded.\n");
    }

    // STEP 14 : Create command pool
    vkResult = createCommandPool();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createCommandPool() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : createCommandPool() succeeded.\n");
    }

    // STEP 15 : Create command buffers
    vkResult = createCommandBuffers();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createCommandBuffers() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : createCommandBuffers() succeeded.\n");
    }

    //create vertex buffer

    vkResult = createVertexBuffer();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createVertexBuffer() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : createVertexBuffer() succeeded.\n");
    }

     // STEP  : Create shaders
    vkResult = createShaders();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createShaders() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : createShaders() succeeded.\n");
    }

	vkResult = createDiscriptorSetLayout();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createDescriptorSetLayout() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : createDescriptorSetLayout() succeeded.\n");
    }

	//pipeline layout
	vkResult = createPipelineLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "initialize() : createPipelineLayout() failed (%d).\n", vkResult);
		return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "initialize() : createPipelineLayout() succeeded.\n");
	}


    // STEP 16 : Create RenderPass
    vkResult = createRenderPass();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createRenderPass() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : createRenderPass() succeeded.\n");
    }

    //pipeline
	vkResult = createGraphicsPipeline();

    if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "initialize() : createGraphicsPipeline() failed (%d).\n", vkResult);
		return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "initialize() : createGraphicsPipeline() succeeded.\n");
	}

    //  Create Framebuffers
    vkResult = createFramebuffers();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createFramebuffers() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : createFramebuffers() succeeded.\n");
    }

    // STEP 18 : Create semaphores and fences
    vkResult = createSemaphores();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createSemaphores() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : createSemaphores() succeeded.\n");
    }

    vkResult = createFences();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createFences() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : createFences() succeeded.\n");
    }

    // initialize clear color values
    memset((void*)&vkClearColorValue, 0, sizeof(VkClearColorValue));

    // this is analogous to glClearColor() or DirectX's clearColor[] array
    vkClearColorValue.float32[0] = 0.0f;
    vkClearColorValue.float32[1] = 0.0f;
    vkClearColorValue.float32[2] = 1.0f;
    vkClearColorValue.float32[3] = 1.0f;

    // STEP 19 : Building the command buffers
    vkResult = buildCommandBuffers();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : buildCommandBuffers() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "initialize() : buildCommandBuffers() succeeded.\n");
    }

    // Initialization is completed.
    bInitialized = TRUE;

    return(vkResult);
}

void resize(int width, int height)
{
    // code
    // reset the height to 1 to avoid a division by 0
    if (height <= 0)
    {
        height = 1;
    }

    // set the global winWidth and winHeight variables
    winWidth = width;
    winHeight = height;
}

VkResult display_(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    // if control comes here before initialization is completed, return false
    if (bInitialized == FALSE)
    {
        fprintf(gpFILE, "display() : initialization yet not completed.\n");
        return((VkResult)VK_FALSE);
    }

   static uint32_t curIndex = 0;

    // -----------------------------------------------------------------------------------------------------------------------------------------
// use fence to allow host to wait for completion of execution of previous command buffer
// -----------------------------------------------------------------------------------------------------------------------------------------
    vkResult = vkWaitForFences(
        vkDevice,                          // [in] Vulkan logical device
        1,                                 // [in] number of fences to wait for
        &vkFence_Array[curIndex], // [in] array of fences
        VK_TRUE,                           // [in] waitAll (type : VkBool32, description : wait for all fences in the array?)
        UINT64_MAX                         // [in] timeout in nanoseconds (UINT64_MAX is the amount in nanoseconds)
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "display() : vkWaitForFences() failed (%d).\n", vkResult);
        return(vkResult);
    }

    // -----------------------------------------------------------------------------------------------------------------------------------------
    // now ready the fences for execution of next command buffer
    // -----------------------------------------------------------------------------------------------------------------------------------------
    vkResult = vkResetFences(vkDevice, 1, &vkFence_Array[curIndex]);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "display() : vkResetFences() failed (%d).\n", vkResult);
        return(vkResult);
    }

	curIndex = (curIndex + 1) % swapchainImageCount;


    // -----------------------------------------------------------------------------------------------------------------------------------------
    // acquire index of next swapchain image
    // -----------------------------------------------------------------------------------------------------------------------------------------
    vkResult = vkAcquireNextImageKHR(
        vkDevice,               // [in] Vulkan logical device
        vkSwapchainKHR,         // [in] Vulkan Swapchain (from which swapchain to acquire the next image)
        UINT64_MAX,             // [in] timeout in nanoseconds (here we are waiting for swapchain to give us the image. Swapchain may not necessarily give the image when you ask for it, so how much to wait before trying again?)
        vkSemaphore_BackBuffer[0], // [in] Vulkan semaphore (here we are waiting for another queue to release the image held by another queue demanded by the swapchain)
        VK_NULL_HANDLE,         // [in] Vulkan fence
        &currentImageIndex      // [out] next image index
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "display() : vkAcquireNextImageKHR() failed (%d).\n", vkResult);
        return(vkResult);
    }



    // -----------------------------------------------------------------------------------------------------------------------------------------
    // One of the members of the VkSubmitInfo structure requires an array of pipeline stages. 
    // We have only 1 of completion of color attachment output. Still we need a 1 member array.
    // -----------------------------------------------------------------------------------------------------------------------------------------
    const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    // -----------------------------------------------------------------------------------------------------------------------------------------
    // declare, memset and initialize VkSubmitInfo structure
    // -----------------------------------------------------------------------------------------------------------------------------------------
    VkSubmitInfo vkSubmitInfo;
    memset((void*)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));

    vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo.pNext = NULL;
    vkSubmitInfo.pWaitDstStageMask = &waitDstStageMask; // single member array
    vkSubmitInfo.waitSemaphoreCount = 1;
    vkSubmitInfo.pWaitSemaphores = &vkSemaphore_BackBuffer[0]; // array of semaphores to wait for
    vkSubmitInfo.commandBufferCount = 1;
    vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_Array[currentImageIndex];
    vkSubmitInfo.signalSemaphoreCount = 1;
    vkSubmitInfo.pSignalSemaphores = &vkSemaphore_RenderComplete[0];

    // -----------------------------------------------------------------------------------------------------------------------------------------
    // now submit above work to the queue
    // -----------------------------------------------------------------------------------------------------------------------------------------
    vkResult = vkQueueSubmit(
        vkQueue,                         // [in] queue 
        1,                               // [in] number of submit info structures
        &vkSubmitInfo,                   // [in] array of submit info structures
        vkFence_Array[currentImageIndex] // [in] fence 
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "display() : vkQueueSubmit() failed (%d).\n", vkResult);
        return(vkResult);
    }

    // -----------------------------------------------------------------------------------------------------------------------------------------
    // we are going to present the rendered image after declaring and initializing VkPresentInfoKHR structure 
    // -----------------------------------------------------------------------------------------------------------------------------------------
    VkPresentInfoKHR vkPresentInfoKHR;
    memset((void*)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));

    vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    vkPresentInfoKHR.pNext = NULL;
    vkPresentInfoKHR.swapchainCount = 1;
    vkPresentInfoKHR.pSwapchains = &vkSwapchainKHR;
    vkPresentInfoKHR.pImageIndices = &currentImageIndex;
    vkPresentInfoKHR.waitSemaphoreCount = 1;
    vkPresentInfoKHR.pWaitSemaphores = &vkSemaphore_RenderComplete[0];

    // -----------------------------------------------------------------------------------------------------------------------------------------
    // now, present the queue
    // -----------------------------------------------------------------------------------------------------------------------------------------
    vkResult = vkQueuePresentKHR(vkQueue, &vkPresentInfoKHR);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "display() : vkQueuePresentKHR() failed (%d).\n", vkResult);
        return(vkResult);
    }

	vkDeviceWaitIdle(vkDevice);// or queue wait idle?

    return(vkResult);
}


VkResult display(void)
{
    VkResult vkResult = VK_SUCCESS;

    if (bInitialized == FALSE)
    {
        fprintf(gpFILE, "display() : initialization yet not completed.\n");
        return (VkResult)VK_FALSE;
    }

    // Use per-frame index
    static uint32_t currentFrame = 0;
	uint32_t curIndex = currentFrame % swapchainImageCount;

    // Wait for GPU to finish work on the previous frame
    vkResult = vkWaitForFences(vkDevice, 1, &vkFence_Array[curIndex], VK_TRUE, UINT64_MAX);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "display() : vkWaitForFences() failed (%d).\n", vkResult);
        return vkResult;
    }

    // Reset the fence for use in the current frame
    vkResult = vkResetFences(vkDevice, 1, &vkFence_Array[curIndex]);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "display() : vkResetFences() failed (%d).\n", vkResult);
        return vkResult;
    }

    // Acquire next image from the swapchain
    vkResult = vkAcquireNextImageKHR(
        vkDevice,
        vkSwapchainKHR,
        UINT64_MAX,
        vkSemaphore_BackBuffer[curIndex],  // now using per-frame semaphore
        VK_NULL_HANDLE,
        &currentImageIndex
    );
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "display() : vkAcquireNextImageKHR() failed (%d).\n", vkResult);
        return vkResult;
    }

    // Submit the command buffer for rendering
    const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo vkSubmitInfo;
    memset(&vkSubmitInfo, 0, sizeof(VkSubmitInfo));
    vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo.waitSemaphoreCount = 1;
    vkSubmitInfo.pWaitSemaphores = &vkSemaphore_BackBuffer[curIndex];
    vkSubmitInfo.pWaitDstStageMask = &waitDstStageMask;
    vkSubmitInfo.commandBufferCount = 1;
    vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_Array[currentImageIndex];
    vkSubmitInfo.signalSemaphoreCount = 1;
    vkSubmitInfo.pSignalSemaphores = &vkSemaphore_RenderComplete[curIndex];

    vkResult = vkQueueSubmit(
        vkQueue,
        1,
        &vkSubmitInfo,
        vkFence_Array[curIndex]  // associate fence with this submission
    );
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "display() : vkQueueSubmit() failed (%d).\n", vkResult);
        return vkResult;
    }

    // Present the rendered image
    VkPresentInfoKHR vkPresentInfoKHR;
    memset(&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));
    vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    vkPresentInfoKHR.waitSemaphoreCount = 1;
    vkPresentInfoKHR.pWaitSemaphores = &vkSemaphore_RenderComplete[curIndex];
    vkPresentInfoKHR.swapchainCount = 1;
    vkPresentInfoKHR.pSwapchains = &vkSwapchainKHR;
    vkPresentInfoKHR.pImageIndices = &currentImageIndex;

    vkResult = vkQueuePresentKHR(vkQueue, &vkPresentInfoKHR);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "display() : vkQueuePresentKHR() failed (%d).\n", vkResult);
        return vkResult;
    }

    // Advance to next frame
    currentFrame++;

    return vkResult;
}


void update(void)
{
    // code
}

void uninitialize(void)
{
    // function declarations
    void ToggleFullscreen(void);

    // code
    // if application is exitting in fullscreen
    if (gbFullscreen == TRUE)
    {
        ToggleFullscreen();
        gbFullscreen = FALSE;
    }

    if (gpFILE)
    {
        fprintf(gpFILE, LINE_END);
    }

    // Before destroying the device (and any other vulkan related destruction), ensure that all operations on that device are finished. Till then, wait on that device.
    if (vkDevice)
    {
        vkDeviceWaitIdle(vkDevice);
        fprintf(gpFILE, "uninitialize() : vkDeviceWaitIdle() is done.\n");
    }

    // sub-step 7 for step (18)
    if (vkFence_Array)
    {
        for (uint32_t i = 0; i < swapchainImageCount; i++)
        {
            vkDestroyFence(vkDevice, vkFence_Array[i], NULL);
            vkFence_Array[i] = VK_NULL_HANDLE;

            fprintf(gpFILE, "uninitialize() : vkDestroyFence() succeeded for iteration %d.\n", i);
        }

        free(vkFence_Array);
        vkFence_Array = NULL;

        fprintf(gpFILE, "uninitialize() : successfully freed the memory allocated to vkFence_Array.\n");
    }

    // sub-step 8 for step (18)

    for (size_t i = 0; i < swapchainImageCount; i++)
    {
        if (vkSemaphore_RenderComplete[i])
        {
            vkDestroySemaphore(vkDevice, vkSemaphore_RenderComplete[i], NULL);
            vkSemaphore_RenderComplete[i] = VK_NULL_HANDLE;

            fprintf(gpFILE, "uninitialize() : vkDestroySemaphore() succeeded for the render complete semaphore.\n");
        }

        if (vkSemaphore_BackBuffer[i])
        {
            vkDestroySemaphore(vkDevice, vkSemaphore_BackBuffer[i], NULL);
            vkSemaphore_BackBuffer[i] = VK_NULL_HANDLE;

            fprintf(gpFILE, "uninitialize() : vkDestroySemaphore() succeeded for the back buffer semaphore.\n");
        }
    }

    free(vkSemaphore_BackBuffer);
    vkSemaphore_BackBuffer = NULL;

	free(vkSemaphore_RenderComplete);
	vkSemaphore_RenderComplete = NULL;



    // sub-step 5 for Step (17)
    if (vkFramebuffer_Array)
    {
        for (uint32_t i = 0; i < swapchainImageCount; i++)
        {
            vkDestroyFramebuffer(vkDevice, vkFramebuffer_Array[i], NULL);
			vkFramebuffer_Array[i] = VK_NULL_HANDLE;

            fprintf(gpFILE, "uninitialize() : vkDestroyFramebuffer() succeeded for iteration %d.\n", i);
        }

        free(vkFramebuffer_Array);
        vkFramebuffer_Array = NULL;

        fprintf(gpFILE, "uninitialize() : successfully freed the memory allocated to vkFramebuffer_Array.\n");
    }

    // pipeline
    if (vkPipeline)
    {
        vkDestroyPipeline(vkDevice, vkPipeline, NULL);
        vkPipeline = VK_NULL_HANDLE;

        fprintf(gpFILE, "uninitialize() : vkDestroyPipeline() succeeded.\n");
    }

    // renderpass
    if (vkRenderPass)
    {
        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
        vkRenderPass = VK_NULL_HANDLE;

        fprintf(gpFILE, "uninitialize() : vkDestroyRenderPass() succeeded.\n");
    }

    //pipeline layout
    if (vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
        vkPipelineLayout = VK_NULL_HANDLE;
        fprintf(gpFILE, "uninitialize() : vkDestroyPipelineLayout() succeeded.\n");
    }

	//descriptor set layout
    if (vkDescriptorSetLayout)
	{
		vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, NULL);
		vkDescriptorSetLayout = VK_NULL_HANDLE;

		fprintf(gpFILE, "uninitialize() : vkDestroyDescriptorSetLayout() succeeded.\n");
	}


    //shaderModule 
    if (vkShaderModule_fragment_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_fragment_shader, NULL);
        vkShaderModule_fragment_shader = VK_NULL_HANDLE;
    }
    if (vkShaderModule_vertex_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_vertex_shader, NULL);
        vkShaderModule_vertex_shader = VK_NULL_HANDLE;
    }

    //vertexData
    if (vertexData_position.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_position.vkDeviceMemory, NULL);
        vertexData_position.vkDeviceMemory = VK_NULL_HANDLE;

    }

    if (vertexData_position.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_position.vkBuffer, NULL);
        vertexData_position.vkBuffer = VK_NULL_HANDLE;
    }

    // sub-step 4 for step (15)
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_Array[i]);
        fprintf(gpFILE, "uninitialize() : vkFreeCommandBuffers() succeeded for iteration %d.\n", i);
    }

    // sub-step 5 for step (15)
    if (vkCommandBuffer_Array)
    {
        free(vkCommandBuffer_Array);
        vkCommandBuffer_Array = NULL;

        fprintf(gpFILE, "uninitialize() : successfully freed memory for the command buffer array.\n");
    }

    // sub-step 3 for step (14)
    if (vkCommandPool)
    {
        vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
        vkCommandPool = VK_NULL_HANDLE;

        fprintf(gpFILE, "uninitialize() : vkDestroyCommandPool() succeeded.\n");
    }

    // sub-step 9 for step (13)
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkDestroyImageView(vkDevice, swapchainImageView_Array[i], NULL);
        fprintf(gpFILE, "uninitialize() : vkDestroyImageView() succeeded for iteration %d.\n", i);
    }

    // sub-step 10 for step (13)
    if (swapchainImageView_Array)
    {
        free(swapchainImageView_Array);
        swapchainImageView_Array = NULL;

        fprintf(gpFILE, "uninitialize() : successfully freed the swapchain image views array.\n");
    }

    // sub-step 8 for step (13)
    if (swapchainImage_Array)
    {
        free(swapchainImage_Array);
        swapchainImage_Array = NULL;

        fprintf(gpFILE, "uninitialize() : successfully freed the swapchain images array.\n");
    }

    // Destroy Vulkan swapchain
    if (vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(
            vkDevice,       // [in] Vulkan device handle,
            vkSwapchainKHR, // [in] Vulkan swapchain handle
            NULL            // [in, optional] custom memory allocator
        );

        vkSwapchainKHR = VK_NULL_HANDLE;

        fprintf(gpFILE, "uninitialize() : vkDestroySwapchainKHR() succeeded.\n");
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

        fprintf(gpFILE, "uninitialize() : vkDestroyDevice() succeeded.\n");

        vkDevice = VK_NULL_HANDLE;
    }

    /*
     * no need to destroy selected physical device
     */

     // Destroy the VkSurfaceKHR object
    if (vkSurfaceKHR)
    {
        vkDestroySurfaceKHR(
            vkInstance,   // [in] Vulkan instance handle
            vkSurfaceKHR, // [in] Vulkan presentation surface handle
            NULL          // [in, optional] pointer to a custom memory allocator (NULL means use a default memory allocator)
        );

        vkSurfaceKHR = VK_NULL_HANDLE;

        fprintf(gpFILE, "uninitialize() : vkDestroySurfaceKHR() succeeded.\n");
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

        fprintf(gpFILE, "uninitialize() : vkDestroyInstance() succeeded.\n");
    }

    // Destroy window
    if (ghwnd)
    {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    // close the log file
    if (gpFILE)
    {
        fprintf(gpFILE, "uninitialize() : Program ended successfully.\n");
        fclose(gpFILE);
        gpFILE = NULL;
    }
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// -                                                    Vulkan related function definitions                                                    -
// ---------------------------------------------------------------------------------------------------------------------------------------------
VkResult createVulkanInstance(void)
{
    // function declarations
    VkResult fillInstanceExtensionNames(void);
    VkResult fillValidaionLayerNames(void);
    VkResult createValidationCallbackFunction(void);

    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    /*
     * sub-step 1 : as explained before, fill and initialize required
     *              extension names and count global variables
     */
    vkResult = fillInstanceExtensionNames();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVulkanInstance() : fillInstanceExtensionNames() failed.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createVulkanInstance() : fillInstanceExtensionNames() succeeded.\n");
    }

    //fill  validation layer
    if (TRUE == bValidation)
    {

        vkResult = fillValidaionLayerNames();
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createVulkanInstance() : fillValidaionLayerNames() failed.\n");
            return(vkResult);
        }
        else
        {
            fprintf(gpFILE, "createVulkanInstance() : fillValidaionLayerNames() succeeded.\n");
        }
    }
    /*
     * sub-step 2 : initialize struct VkApplicationInfo
     */
    VkApplicationInfo vkApplicationInfo;
    memset(&vkApplicationInfo, 0, sizeof(VkApplicationInfo));

    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // structure type (VkStructureType)
    vkApplicationInfo.pNext = NULL;                               // pointer to a structure extending this structure (linked list)
    vkApplicationInfo.pApplicationName = gpszAppName;                        // application name (can be anything, but to be meaningful, we will use the global app name)
    vkApplicationInfo.applicationVersion = 1;                                  // can be anything, we will just use 1 (developer-supplied version)
    vkApplicationInfo.pEngineName = gpszAppName;                        // engine name (again, can be anything, but to be meaningful, we will use the global app name)
    vkApplicationInfo.engineVersion = 1;                                  // can be anything, we will just use 1 (developer-supplied version)
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;                 // must be the highest Vulkan API Version

    /*
     * sub-step 3 : initialize struct VkInstanceCreateInfo by using
     *              information from sub-step 1 and sub-step 2
     */
    VkInstanceCreateInfo vkInstanceCreateInfo;
    memset(&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));

    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // structure type (VkStructureType)
    vkInstanceCreateInfo.pNext = NULL;                                   // pointer to a structure extending this structure (linked list)
    vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;                     // pointer to a VkApplicationInfo structure 
    vkInstanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;          // number of enabled Vulkan instance extensions
    vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_Array;    // array of enabled Vulkan instance extension names

    if (TRUE == bValidation)
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
     * sub-step 4 : call vkCreateInstance() to get VkInstance in a
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
    else
    {
        fprintf(gpFILE, "createVulkanInstance() : vkCreateInstance() succeeded.\n");
    }

    /*
     * sub-step 5 : destroy VkInstance in uninitialize()
     */

     // code in uninitialize()

        //do for validation callbacks
    if (TRUE == bValidation)
    {
        vkResult = createValidationCallbackFunction();
    }

    return(vkResult);
}

VkResult fillValidaionLayerNames(void)
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
    else
    {
        fprintf(gpFILE, "fillValidaionLayerNames() : first call to vkEnumerateInstanceLayerProperties() succeeded.\n");
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
    else
    {
        fprintf(gpFILE, "fillValidaionLayerNames() : second call to vkEnumerateInstanceLayerProperties() succeeded.\n");
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

        fprintf(gpFILE, "fillValidaionLayerNames() : Vulkan instance extension name = %s\n", validationLayerNames_array[i]);
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
VkResult createValidationCallbackFunction(void)
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
    else
    {
        fprintf(gpFILE, "createValidationCallbackFunction() : vkGetInstanceProcAddr succeded to get vkCreateDebugReportCallbackEXT_fnptr .\n");
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
    else
    {
        fprintf(gpFILE, "createValidationCallbackFunction() : vkGetInstanceProcAddr succeded to get vkDestroyDebugReportCallbackEXT_fnptr .\n");
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
    else
    {
        fprintf(gpFILE, "createValidationCallbackFunction() : vkCreateDebugReportCallbackEXT_fnptr succeeded.\n");
    }

    return vkResult;
}

VkResult fillInstanceExtensionNames(void)
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
    else
    {
        fprintf(gpFILE, "fillInstanceExtensionNames() : first call to vkEnumerateInstanceExtensionProperties() succeeded.\n");
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
    else
    {
        fprintf(gpFILE, "fillInstanceExtensionNames() : second call to vkEnumerateInstanceExtensionProperties() succeeded.\n");
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

        fprintf(gpFILE, "fillInstanceExtensionNames() : Vulkan instance extension name = %s\n", instanceExtensionNames_Array[i]);
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
            if (TRUE == bValidation)
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
        if (TRUE == bValidation)
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
        if (TRUE == bValidation)
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

VkResult getSupportedSurface(void)
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
    vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(ghwnd, GWLP_HINSTANCE); // this member can also be initialized by using "(HINSTANCE)GetModuleHandle(NULL);"
    vkWin32SurfaceCreateInfoKHR.hwnd = ghwnd;

    /*
     * sub-step 3 : Now call vkCreateWin32SurfaceKHR() to create the presentation surface object.
     */
    vkResult = vkCreateWin32SurfaceKHR(
        vkInstance,                   // [in] Vulkan instance object (until you get device, Vulkan instance will be used)
        &vkWin32SurfaceCreateInfoKHR, // [in] Surface create info's address
        NULL,                         // [in] memory allocator
        &vkSurfaceKHR                 // [out] pointer to a VkSurfaceKHR object
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "getSupportedSurface() : vkCreateWin32SurfaceKHR() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "getSupportedSurface() : vkCreateWin32SurfaceKHR() succeeded.\n");
    }

    return(vkResult);
}

VkResult getPhysicalDevice(void)
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
    else
    {
        fprintf(gpFILE, "getPhysicalDevice() : 1st call to vkEnumeratePhysicalDevices() succeeded.\n");
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
    else
    {
        fprintf(gpFILE, "getPhysicalDevice() : 2nd call to vkEnumeratePhysicalDevices() succeeded.\n");
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
                vkSurfaceKHR,                     // [in] VkSurfaceKHR object
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
                    vkPhysicalDevice_Selected = vkPhysicalDevice_Array[i];
                    graphicsQueueFamilyIndex_Selected = j;
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

            fprintf(gpFILE, "getPhysicalDevice() : succeeded to free isQueueSurfaceSupported_Array.\n");
        }

        if (vkQueueFamilyProperties_Array)
        {
            free(vkQueueFamilyProperties_Array);
            vkQueueFamilyProperties_Array = NULL;

            fprintf(gpFILE, "getPhysicalDevice() : succeeded to free vkQueueFamilyProperties_Array.\n");
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

            fprintf(gpFILE, "getPhysicalDevice() : succeeded to free vkPhysicalDevice_Array.\n");
        }

        fprintf(gpFILE, "getPhysicalDevice() : failed to select graphics supported physical device.\n");

        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }

    /*
     * sub-step 7 : memset() the global physical device memory property structure
     */
    memset((void*)&vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));

    /*
     * sub-step 8 : Initialize above structure by using vkGetPhysicalDeviceMemoryProperties().
     */
    vkGetPhysicalDeviceMemoryProperties(
        vkPhysicalDevice_Selected,        // [in] Vulkan physical device
        &vkPhysicalDeviceMemoryProperties // [out] address to a structure of VkPhysicalDeviceMemoryProperties 
    );

    /*
     * sub-step 9 : Declare a local structure variable VkPhysicalDeviceFeatures, memset() it and
     *              initialize it by calling vkGetPhysicalDeviceFeatures().
     */
    VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
    memset((void*)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));

    vkGetPhysicalDeviceFeatures(
        vkPhysicalDevice_Selected, // [in] Vulkan physical device
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

VkResult printVkInfo(void)
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

    fprintf(gpFILE, LINE_END);

    /*
     *  Free physical device array here, which we removed from the if(bFound == VK_TRUE) block of getPhysicalDevice().
     */
    if (vkPhysicalDevice_Array)
    {
        free(vkPhysicalDevice_Array);
        vkPhysicalDevice_Array = NULL;

        fprintf(gpFILE, "printVkInfo() : succeeded to free vkPhysicalDevice_Array.\n");
    }

    return(vkResult);
}

VkResult fillDeviceExtensionNames(void)
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
        vkPhysicalDevice_Selected, // [in] VkPhysicalDevice 
        NULL,                      // [in, optional] layer name to retrieve extensions from (NULL means you want all extensions)
        &deviceExtensionCount,     // [out] count of supported extensions
        NULL                       // [out, optional] array of VkExtensionProperties to retrieve extension properties
    );
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "fillDeviceExtensionNames() : first call to vkEnumerateDeviceExtensionProperties() failed.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "fillDeviceExtensionNames() : first call to vkEnumerateDeviceExtensionProperties() succeeded.\n");
    }

    /*
     *  allocate and fill struct VkExtensionProperties array
     *              corresponding to above count
     */
    VkExtensionProperties* vkExtensionProperties_Array = NULL;
    vkExtensionProperties_Array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);


    vkResult = vkEnumerateDeviceExtensionProperties(
        vkPhysicalDevice_Selected,
        NULL,
        &deviceExtensionCount,
        vkExtensionProperties_Array
    );
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "fillDeviceExtensionNames() : second call to vkEnumerateDeviceExtensionProperties() failed.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "fillDeviceExtensionNames() : second call to vkEnumerateDeviceExtensionProperties() succeeded.\n");
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

        fprintf(gpFILE, "fillDeviceExtensionNames() : Vulkan device extension name = %s\n", deviceExtensionNames_Array[i]);
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

    for (uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        // using macros is recommended, instead of actual extension names
        if (strcmp(deviceExtensionNames_Array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            vulkanSwapchainExtensionFound = VK_TRUE;
            enabledDeviceExtensionNames_Array[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
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
    else
    {
        fprintf(gpFILE, "fillDeviceExtensionNames() : VK_KHR_SWAPCHAIN_EXTENSION_NAME found.\n");
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

/*
 *  Create a user-defined function “createVulkanDevice()”.
 */
VkResult createVulkanDevice(void)
{
    // function declarations
    VkResult fillDeviceExtensionNames(void);

    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    /*
     *  Call previously created fillDeviceExtensionNames() in it.
     */

     //  fill and initialize required device extension names and count global variables
    vkResult = fillDeviceExtensionNames();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVulkanDevice() : fillDeviceExtensionNames() failed.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createVulkanDevice() : fillDeviceExtensionNames() succeeded.\n");
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
    vkDeviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_Selected;
    vkDeviceQueueCreateInfo.queueCount = 1;
    vkDeviceQueueCreateInfo.pQueuePriorities = queuePriorities; // default queue priority

    VkDeviceCreateInfo vkDeviceCreateInfo;
    memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));

    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pNext = NULL;
    vkDeviceCreateInfo.flags = 0;
    vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
    vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_Array;
    vkDeviceCreateInfo.enabledLayerCount = 0;
    vkDeviceCreateInfo.ppEnabledLayerNames = NULL;
    vkDeviceCreateInfo.pEnabledFeatures = NULL;

    // newly added code (after vkGetDeviceQueue() was returning VK_NULL_HANDLE)
    vkDeviceCreateInfo.queueCreateInfoCount = 1;
    vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;

    /*
     *  Now call vkCreateDevice() Vulkan API to actually
     *              create the Vulkan device and do error-checking.
     */
    vkResult = vkCreateDevice(
        vkPhysicalDevice_Selected, // [in] Vulkan physical device handle
        &vkDeviceCreateInfo,       // [in] VkDeviceCreateInfo*
        NULL,                      // [in, optional] pointer to a custom memory allocator
        &vkDevice                  // [out] VkDevice*
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVulkanDevice() : vkCreateDevice() failed.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createVulkanDevice() : vkCreateDevice() succeeded.\n");
    }

    return(vkResult);
}

void getDeviceQueue(void)
{
    // code
    /*
     *  Call vkGetDeviceQueue() using newly created VkDevice,
     *              selected family index, 0th queue in that selected queue family.
     */
    vkGetDeviceQueue(
        vkDevice,                          // [in] vulkan logical device handle 
        graphicsQueueFamilyIndex_Selected, // [in] selected queue family index
        0,                                 // [in] queue family index
        &vkQueue                           // [out] VkQueue* 
    );

    if (vkQueue == VK_NULL_HANDLE)
    {
        fprintf(gpFILE, "getDeviceQueue() : vkGetDeviceQueue() returned NULL for VkQueue.\n");
        return;
    }
    else
    {
        fprintf(gpFILE, "getDeviceQueue() : vkGetDeviceQueue() succeeded.\n");
    }
}

VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    /*
     *  Call vkGetPhysicalDeviceSurfaceFormatsKHR() first to retrieve the supported count of supported surface color formats.
     */
    uint32_t formatCount = 0;

    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(
        vkPhysicalDevice_Selected, // [in] handle to vulkan physical device
        vkSurfaceKHR,              // [in] which surface?
        &formatCount,              // [out, optional] count of surface formats
        NULL                       // [in] array of surface formats
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "getPhysicalDeviceSurfaceFormatAndColorSpace() : vkGetPhysicalDeviceSurfaceFormatsKHR()'s 1st call failed.\n");
        return(vkResult);
    }
    else if (formatCount == 0)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFILE, "getPhysicalDeviceSurfaceFormatAndColorSpace() : Format count is 0.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "getPhysicalDeviceSurfaceFormatAndColorSpace() : vkGetPhysicalDeviceSurfaceFormatsKHR()'s 1st call succeeded.\n");
    }

    /*
     *  Declare and allocate an array of VkSurfaceFormatKHR structure corresponding to above count.
     *             VkColorFormat and VkColorSpace are the two members.
     */
    VkSurfaceFormatKHR* vkSurfaceFormatKHR_Array = (VkSurfaceFormatKHR*)malloc(formatCount * sizeof(VkSurfaceFormatKHR));

    /*
     *  Call the same above function again, but now to fill the above declared array.
     */
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(
        vkPhysicalDevice_Selected,
        vkSurfaceKHR,
        &formatCount,
        vkSurfaceFormatKHR_Array
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "getPhysicalDeviceSurfaceFormatAndColorSpace() : vkGetPhysicalDeviceSurfaceFormatsKHR()'s 2nd call failed.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "getPhysicalDeviceSurfaceFormatAndColorSpace() : vkGetPhysicalDeviceSurfaceFormatsKHR()'s 2nd call succeeded.\n");
    }

    /*
     *  According to the contents of the above filled array,
     *              decide the surface color format and color space.
     */

     // decide the surface color format first
    if (formatCount == 1 && vkSurfaceFormatKHR_Array[0].format == VK_FORMAT_UNDEFINED) // here, "undefined" means that Vulkan doesn't have a default format, so you can give one yours and it will try to give you that, if it can't, swapchain creation will fail
    {
        vkFormat_Color = VK_FORMAT_B8G8R8A8_UNORM;
    }
    else
    {
        vkFormat_Color = vkSurfaceFormatKHR_Array[0].format;
    }

    // now decide the surface color space
    vkColorSpaceKHR = vkSurfaceFormatKHR_Array[0].colorSpace;

    /*
     *  Free the above created array
     */
    if (vkSurfaceFormatKHR_Array)
    {
        free(vkSurfaceFormatKHR_Array);
        vkSurfaceFormatKHR_Array = NULL;

        fprintf(gpFILE, "getPhysicalDeviceSurfaceFormatAndColorSpace() : vkSurfaceFormatKHR_Array is freed successfully.\n");
    }

    return(vkResult);
}

VkResult getPhysicalDevicePresentMode(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    /*
     * Call vkGetPhysicalDeviceSurfacePresentModesKHR() first to retrieve the count of supported presentation modes.
     */
    uint32_t presentModeCount = 0;

    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(
        vkPhysicalDevice_Selected, // [in] vulkan physical device handle
        vkSurfaceKHR,              // [in] vulkan surface
        &presentModeCount,         // [out, optional] count of present modes
        NULL                       // [out, optional] present modes array 
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "getPhysicalDevicePresentMode() : vkGetPhysicalDeviceSurfacePresentModesKHR()'s 1st call failed.\n");
        return(vkResult);
    }
    else if (presentModeCount == 0)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFILE, "getPhysicalDevicePresentMode() : Present mode count is 0.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "getPhysicalDevicePresentMode() : vkGetPhysicalDeviceSurfacePresentModesKHR()'s 1st call succeeded.\n");
    }

    /*
     *  Declare and allocate an array of VkPresentModeKHR enum corresponding to above count.
     */
    VkPresentModeKHR* vkPresentModeKHR_Array = (VkPresentModeKHR*)malloc(presentModeCount * sizeof(VkPresentModeKHR));

    /*
     *  Call the above function again to fill the above array.
     */
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(
        vkPhysicalDevice_Selected,
        vkSurfaceKHR,
        &presentModeCount,
        vkPresentModeKHR_Array
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "getPhysicalDevicePresentMode() : vkGetPhysicalDeviceSurfacePresentModesKHR()'s 2nd call failed.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "getPhysicalDevicePresentMode() : vkGetPhysicalDeviceSurfacePresentModesKHR()'s 2nd call succeeded.\n");
    }

    /*
     *  According to the contents of the above filled array, decide the presentation mode.
     */
    for (uint32_t i = 0; i < presentModeCount; i++)
    {
        if (vkPresentModeKHR_Array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    if (vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR)
    {
        vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
    }

    /*
     * Free the above created array.
     */
    if (vkPresentModeKHR_Array)
    {
        free(vkPresentModeKHR_Array);
        vkPresentModeKHR_Array = NULL;

        fprintf(gpFILE, "getPhysicalDevicePresentMode() : vkPresentModeKHR_Array is freed successfully.\n");
    }

    return(vkResult);
}

VkResult createSwapchain(VkBool32 vsync) // vsync == vertical synchronisation
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
    else
    {
        fprintf(gpFILE, "createSwapchain() : getPhysicalDeviceSurfaceFormatAndColorSpace() succeeded.\n");
    }

    /*
     * sub-step 2 : Get physical device surface capabilities by using Vulkan API vkGetPhysicalDeviceSurfaceCapabilitiesKHR()
     *              and accordingly initialize VkSurfaceCapabilitiesKHR structure.
     */
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
    memset((void*)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));

    vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        vkPhysicalDevice_Selected, // [in] vulkan physical device
        vkSurfaceKHR,              // [in] vulkan surface
        &vkSurfaceCapabilitiesKHR  // [out] pointer to a VkSurfaceCapabilitiesKHR structure 
    );
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createSwapchain() : vkGetPhysicalDeviceSurfaceCapabilitiesKHR() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createSwapchain() : vkGetPhysicalDeviceSurfaceCapabilitiesKHR() succeeded.\n");
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
    memset((void*)&vkExtent2D_Swapchain, 0, sizeof(VkExtent2D));

    if (vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
    {
        vkExtent2D_Swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
        vkExtent2D_Swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;

        fprintf(gpFILE, "createSwapchain() : (1) Swapchain Image width x height = %u x %u\n",
            vkExtent2D_Swapchain.width,
            vkExtent2D_Swapchain.height); // using %u because width and height are unsigned integers
    }
    else
    {
        // if surface size is already defined, then swapchain image size MUST match with it
        VkExtent2D vkExtent2D;
        memset((void*)&vkExtent2D, 0, sizeof(VkExtent2D));

        vkExtent2D.width = (uint32_t)winWidth;
        vkExtent2D.height = (uint32_t)winHeight;

        vkExtent2D_Swapchain.width = max(vkSurfaceCapabilitiesKHR.minImageExtent.width, min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width)); // clamp the width between minImageExtent.width and maxImageExtent.width
        vkExtent2D_Swapchain.height = max(vkSurfaceCapabilitiesKHR.minImageExtent.height, min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height)); // clamp the height between minImageExtent.height and maxImageExtent.height

        /*
         * Example of clamping between minimum and maximum values:
         *
         *          max(2, min(4, 3))
         *          max(2, 3)
         *          3
         */

        fprintf(gpFILE, "createSwapchain() : (2) Swapchain Image width x height = %u x %u\n",
            vkExtent2D_Swapchain.width,
            vkExtent2D_Swapchain.height); // using %u because width and height are unsigned integers
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
    else
    {
        fprintf(gpFILE, "createSwapchain() : getPhysicalDevicePresentMode() succeeded.\n");
    }

    
    //sub-step 8 : According to the above data, declare, memset() and initialize VkSwapchainCreateInfoKHR structure.
     
    VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
    memset((void*)&vkSwapchainCreateInfoKHR, 0, sizeof(VkSwapchainCreateInfoKHR));

    vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vkSwapchainCreateInfoKHR.pNext = NULL;
    vkSwapchainCreateInfoKHR.flags = 0;
    vkSwapchainCreateInfoKHR.surface = vkSurfaceKHR;
    vkSwapchainCreateInfoKHR.minImageCount = desiredNumberOfSwapchainImages;
    vkSwapchainCreateInfoKHR.imageFormat = vkFormat_Color;
    vkSwapchainCreateInfoKHR.imageColorSpace = vkColorSpaceKHR;
    vkSwapchainCreateInfoKHR.imageExtent.width = vkExtent2D_Swapchain.width;
    vkSwapchainCreateInfoKHR.imageExtent.height = vkExtent2D_Swapchain.height;
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
        vkDevice,                  // [in] vulkan device handle
        &vkSwapchainCreateInfoKHR, // [in] pointer to a VkSwapchainCreateInfoKHR structure
        NULL,                      // [in, optional] custom memory allocator
        &vkSwapchainKHR            // [out] pointer to VkSwapchainKHR 
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createSwapchain() : vkCreateSwapchainKHR() failed (%d).\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createSwapchain() : vkCreateSwapchainKHR() succeeded.\n");
    }

    
      // When done, destroy it in uninitialize() by using vkDestroySwapchain() Vulkan API.
     

     // remaining code in uninitialize()

    return(vkResult);
}

VkResult createImagesAndImageViews(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    // Get swapchain image count in a global variable using vkGetSwapchainImagesKHR().
    vkResult = vkGetSwapchainImagesKHR(
        vkDevice,             // [in] VkDevice (logical device)
        vkSwapchainKHR,       // [in] VkSwapchainKHR
        &swapchainImageCount, // [out] Swapchain Image Count
        NULL                  // [out, optional] Swapchain Image array
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createImagesAndImageViews() : vkGetSwapchainImagesKHR()'s 1st call failed.\n");
        return(vkResult);
    }
    else if (swapchainImageCount == 0)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFILE, "createImagesAndImageViews() : Swapchain image count is 0.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createImagesAndImageViews() : vkGetSwapchainImagesKHR()'s 1st call succeeded.\n");
        fprintf(gpFILE, "createImagesAndImageViews() : gives swapchain image count = %d\n", swapchainImageCount);
    }

    // Declare a global VkImage array and allocate it to the swapchain image count using malloc().
    swapchainImage_Array = (VkImage*)malloc(sizeof(VkImage) * swapchainImageCount);

    // Now call the same function again, which we called in step 1 and fill this array.
    vkResult = vkGetSwapchainImagesKHR(
        vkDevice,
        vkSwapchainKHR,
        &swapchainImageCount,
        swapchainImage_Array
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createImagesAndImageViews() : vkGetSwapchainImagesKHR()'s 2nd call failed.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createImagesAndImageViews() : vkGetSwapchainImagesKHR()'s 2nd call succeeded.\n");
    }

    //  Declare another global array of type VkImageView and allocate it to the size of swapchain image count.
    swapchainImageView_Array = (VkImageView*)malloc(sizeof(VkImageView) * swapchainImageCount);

    // Declare and initialize VkImageViewCreateInfo struct except its “.image” member.
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = vkFormat_Color;
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
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkImageViewCreateInfo.image = swapchainImage_Array[i];

        vkResult = vkCreateImageView(
            vkDevice,                    // [in] VkDevice
            &vkImageViewCreateInfo,      // [in] VkImageViewCreateInfo *
            NULL,                        // [in] custom memory allocator
            &swapchainImageView_Array[i] // [out] VkImageView * 
        );

        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createImagesAndImageViews() : vkCreateImageView() failed for iteration %d. (%d)\n", i, vkResult);
            return(vkResult);
        }
        else
        {
            fprintf(gpFILE, "createImagesAndImageViews() : vkCreateImageView() succeeded for iteration %d.\n", i);
        }
    }

    return(vkResult);
}

VkResult createCommandPool(void)
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
    vkCommandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_Selected;

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
    else
    {
        fprintf(gpFILE, "createCommandPool() : vkCreateCommandPool() succeeded.\n");
    }

    return(vkResult);
}

VkResult createCommandBuffers(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    // sub-step 1 : Declare and initialize struct VkCommandBufferAllocateInfo
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
    memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));

    vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo.pNext = NULL;
    vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBufferAllocateInfo.commandBufferCount = 1;

    // sub-step 2 : Declare a command buffer array globally and allocate it to the size of swapchain image count.
    vkCommandBuffer_Array = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * swapchainImageCount);

    // sub-step 3 : In a loop which is equal to swapchain image count, allocate each command buffer 
    //              in the above array by using vkAllocateCommandBuffers() API. 
    //              Remember, at the time of allocation, all buffers are going to be empty. 
    //              Later we will record graphics / compute commands into them.
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_Array[i]);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createCommandBuffers() : vkAllocateCommandBuffers failed for iteration %d.\n", i);
            return(vkResult);
        }
        else
        {
            fprintf(gpFILE, "createCommandBuffers() : vkAllocateCommandBuffers succeeded for iteration %d.\n", i);
        }
    }

    return(vkResult);
}

VkResult createVertexBuffer_(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    float triangle_position[] =
    {
        0.0f,1.0f,0.0f,
        -1.0f,-1.0f,0.0f,
        1.0f,-1.0f,0.0f
    };

    memset((void*)&vertexData_position, 0, sizeof(VertexData));

    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(triangle_position);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_position.vkBuffer);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer():  failed.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer():  succeeded.\n");
    }

    //------------
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(vkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, vertexData_position.vkBuffer, &vkMemoryRequirements);

    //------------
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo, 0, sizeof(vkMemoryAllocateInfo));

    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;

    //-------------
    for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }

        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    //--------------
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position.vkDeviceMemory);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  failed.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  succeeded.\n");
    }
    
    //---------------
    vkResult = vkBindBufferMemory(vkDevice, vertexData_position.vkBuffer, vertexData_position.vkDeviceMemory,0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  succeeded.\n");
    }

    //----------------
    void* data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_position.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkMapMemory() :  failed.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkMapMemory() :  succeeded.\n");
    }

    //-------actual memory mapped io
    memcpy(data, triangle_position, sizeof(triangle_position));

    //-------unmap memory
    vkUnmapMemory(vkDevice, vertexData_position.vkDeviceMemory);

    return vkResult;
}

VkResult createVertexBuffer(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    float triangle_position[] =
    {
        0.0f,1.0f,0.0f,
        -1.0f,-1.0f,0.0f,
        1.0f,-1.0f,0.0f
    };

	//staging buffer
	VertexData vertexData_stagingBffer_position;
	memset((void*)&vertexData_stagingBffer_position, 0, sizeof(VertexData));

	VkBufferCreateInfo vkBufferCreateInfo_stagingBuffer;
	memset((void*)&vkBufferCreateInfo_stagingBuffer, 0, sizeof(VkBufferCreateInfo));

	vkBufferCreateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo_stagingBuffer.pNext = NULL;
	vkBufferCreateInfo_stagingBuffer.flags = 0;
	vkBufferCreateInfo_stagingBuffer.size = sizeof(triangle_position);
	vkBufferCreateInfo_stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // staging buffer is used for transfering data to device local buffer
	vkBufferCreateInfo_stagingBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // not sharing with other queues

	// Call vkCreateBuffer() to create the staging buffer
	vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo_stagingBuffer, NULL, &vertexData_stagingBffer_position.vkBuffer);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer():  failed.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer():  succeeded.\n");
    }

	//------------
	// Get memory requirements for the staging buffer
	VkMemoryRequirements vkMemoryRequirements_stagingBuffer;
	memset((void*)&vkMemoryRequirements_stagingBuffer, 0, sizeof(vkMemoryRequirements_stagingBuffer));

	vkGetBufferMemoryRequirements(vkDevice, vertexData_stagingBffer_position.vkBuffer, &vkMemoryRequirements_stagingBuffer);
	//------------
	// Allocate memory for the staging buffer
	VkMemoryAllocateInfo vkMemoryAllocateInfo_stagingBuffer;
	memset((void*)&vkMemoryAllocateInfo_stagingBuffer, 0, sizeof(VkMemoryAllocateInfo));
	vkMemoryAllocateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo_stagingBuffer.pNext = NULL;
	vkMemoryAllocateInfo_stagingBuffer.allocationSize = vkMemoryRequirements_stagingBuffer.size;
	vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = 0;
	//-------------
	// Find a suitable memory type for the staging buffer
	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements_stagingBuffer.memoryTypeBits & 1) == 1)
		{
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) // host visible and coherent memory(no need to manage vulkan cache  for flushing or mapping)
			{
				vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = i;
				break;
			}
		}

		vkMemoryRequirements_stagingBuffer.memoryTypeBits >>= 1;
	}

	//--------------  
	// Allocate memory for the staging buffer
	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo_stagingBuffer, NULL, &vertexData_stagingBffer_position.vkDeviceMemory);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  failed.\n");
		return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  succeeded.\n");
	}
	//---------------
	// Bind the staging buffer memory to the staging buffer
	vkResult = vkBindBufferMemory(vkDevice, vertexData_stagingBffer_position.vkBuffer, vertexData_stagingBffer_position.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
		return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  succeeded.\n");
	}

	//----------------
	void* data = NULL;
	vkResult = vkMapMemory(vkDevice, vertexData_stagingBffer_position.vkDeviceMemory, 0, vkMemoryAllocateInfo_stagingBuffer.allocationSize, 0, &data);
    if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkMapMemory() :  failed.\n");
		return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkMapMemory() :  succeeded.\n");
	}

	//-------actual memory mapped io
	memcpy(data, triangle_position, sizeof(triangle_position));
	//-------unmap memory
	vkUnmapMemory(vkDevice, vertexData_stagingBffer_position.vkDeviceMemory);

	//-----------------------------------------------------------------------------------

	//device buffer
	memset((void*)&vertexData_position, 0, sizeof(VertexData));
	VkBufferCreateInfo vkBufferCreateInfo_deviceBuffer;
	memset((void*)&vkBufferCreateInfo_deviceBuffer, 0, sizeof(VkBufferCreateInfo));
	vkBufferCreateInfo_deviceBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo_deviceBuffer.pNext = NULL;
	vkBufferCreateInfo_deviceBuffer.flags = 0;
	vkBufferCreateInfo_deviceBuffer.size = sizeof(triangle_position);
	vkBufferCreateInfo_deviceBuffer.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; // device buffer is used for vertex buffer and transfer destination
	vkBufferCreateInfo_deviceBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // not sharing with other queues

	vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo_deviceBuffer, NULL, &vertexData_position.vkBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer():  failed.\n");
		return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer():  succeeded.\n");
	}

	//------------
	// Get memory requirements for the device local buffer
	VkMemoryRequirements vkMemoryRequirements_deviceBuffer;
	memset((void*)&vkMemoryRequirements_deviceBuffer, 0, sizeof(vkMemoryRequirements_deviceBuffer));
	vkGetBufferMemoryRequirements(vkDevice, vertexData_position.vkBuffer, &vkMemoryRequirements_deviceBuffer);
	//------------
	// Allocate memory for the device local buffer
	VkMemoryAllocateInfo vkMemoryAllocateInfo_deviceBuffer;
	memset((void*)&vkMemoryAllocateInfo_deviceBuffer, 0, sizeof(VkMemoryAllocateInfo));
	vkMemoryAllocateInfo_deviceBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo_deviceBuffer.pNext = NULL;
	vkMemoryAllocateInfo_deviceBuffer.allocationSize = vkMemoryRequirements_deviceBuffer.size;
	vkMemoryAllocateInfo_deviceBuffer.memoryTypeIndex = 0;
	//-------------
	// Find a suitable memory type for the device local buffer

    for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements_deviceBuffer.memoryTypeBits & 1) == 1)
		{
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) // device local memory
			{
				vkMemoryAllocateInfo_deviceBuffer.memoryTypeIndex = i;
				break;
			}
		}

		vkMemoryRequirements_deviceBuffer.memoryTypeBits >>= 1;
	}

	//--------------
	// Allocate memory for the device local buffer
	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo_deviceBuffer, NULL, &vertexData_position.vkDeviceMemory);
        
    if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  failed.\n");
		return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  succeeded.\n");
	}
	//---------------
	// Bind the device local buffer memory to the device local buffer
	vkResult = vkBindBufferMemory(vkDevice, vertexData_position.vkBuffer, vertexData_position.vkDeviceMemory, 0);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
        return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  succeeded.\n");
	}

	//----------------
	//command buffer for copy
	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
	memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));
	vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo.pNext = NULL;
	vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
	vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkCommandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer vkCommandBuffer_Copy = VK_NULL_HANDLE;
	vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_Copy);
	if (vkResult != VK_SUCCESS)
	{
    	fprintf(gpFILE, "createVertexBuffer() -> vkAllocateCommandBuffers() :  failed.\n");
		return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkAllocateCommandBuffers() :  succeeded.\n");
	}

	//----------------
	
	// Begin command buffer recording
	VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
	memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
	vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkCommandBufferBeginInfo.pNext = NULL;
	vkCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // one time submit means we will submit this command buffer only once
	vkCommandBufferBeginInfo.pInheritanceInfo = NULL; // not using secondary command buffer inheritance
	vkResult = vkBeginCommandBuffer(vkCommandBuffer_Copy, &vkCommandBufferBeginInfo);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkBeginCommandBuffer() :  failed.\n");
		return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkBeginCommandBuffer() :  succeeded.\n");
	}

	//----------------
	// Record the command to copy data from staging buffer to device local buffer
	VkBufferCopy vkBufferCopy;
	memset((void*)&vkBufferCopy, 0, sizeof(VkBufferCopy));
	vkBufferCopy.srcOffset = 0; // offset in the source buffer
	vkBufferCopy.dstOffset = 0; // offset in the destination buffer
	vkBufferCopy.size = sizeof(triangle_position); // size of the data to copy
	vkCmdCopyBuffer(vkCommandBuffer_Copy, vertexData_stagingBffer_position.vkBuffer, vertexData_position.vkBuffer, 1, &vkBufferCopy);

	// End command buffer recording
	vkResult = vkEndCommandBuffer(vkCommandBuffer_Copy);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkEndCommandBuffer() :  failed.\n");
		return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkEndCommandBuffer() :  succeeded.\n");
	}

	//----------------
	// Submit the command buffer to the queue
	VkSubmitInfo vkSubmitInfo;
	memset((void*)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));
	vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo.pNext = NULL;
	vkSubmitInfo.waitSemaphoreCount = 0; // no wait semaphores
	vkSubmitInfo.pWaitSemaphores = NULL; // no wait semaphores
	vkSubmitInfo.pWaitDstStageMask = NULL; // no wait stage mask
	vkSubmitInfo.commandBufferCount = 1; // one command buffer
	vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_Copy; // pointer to the command buffer to submit
	vkSubmitInfo.signalSemaphoreCount = 0; // no signal semaphores
	vkSubmitInfo.pSignalSemaphores = NULL; // no signal semaphores


    vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo, VK_NULL_HANDLE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkQueueSubmit() :  failed.\n");
		return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkQueueSubmit() :  succeeded.\n");
	}
	// Wait for the queue to finish processing
	vkResult = vkQueueWaitIdle(vkQueue);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkQueueWaitIdle() :  failed.\n");
		return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkQueueWaitIdle() :  succeeded.\n");
	}

	//----------------
	// Free the staging buffer
	if (vertexData_stagingBffer_position.vkBuffer)
	{
		vkDestroyBuffer(vkDevice, vertexData_stagingBffer_position.vkBuffer, NULL);
		vertexData_stagingBffer_position.vkBuffer = VK_NULL_HANDLE;
		fprintf(gpFILE, "createVertexBuffer() -> vkDestroyBuffer() :  staging buffer destroyed.\n");
	}
	else
	{
		fprintf(gpFILE, "createVertexBuffer() -> vkDestroyBuffer() :  staging buffer already destroyed.\n");
	}

	// Free the staging buffer memory
	if (vertexData_stagingBffer_position.vkDeviceMemory)
	{
		vkFreeMemory(vkDevice, vertexData_stagingBffer_position.vkDeviceMemory, NULL);
		vertexData_stagingBffer_position.vkDeviceMemory = VK_NULL_HANDLE;
		fprintf(gpFILE, "createVertexBuffer() -> vkFreeMemory() :  staging buffer memory freed.\n");
	}

	//-----------------------------------------------------------------------------------
	// Now, vertexData_position.vkBuffer contains the device local buffer with the triangle position data
	// and vertexData_position.vkDeviceMemory contains the device local buffer memory.

	if (vkCommandBuffer_Copy)
	{
		vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_Copy);
		vkCommandBuffer_Copy = VK_NULL_HANDLE;
		fprintf(gpFILE, "createVertexBuffer() -> vkFreeCommandBuffers() :  command buffer freed.\n");
	}
    else
    {
	    fprintf(gpFILE, "createVertexBuffer() -> vkFreeCommandBuffers() :  command buffer already freed.\n");
	}

    return vkResult;
}

VkResult createShaders(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    //vertex shader
    const char* szFileName = "shader.vert.spv";
    FILE* fp = NULL;
    size_t size = 0; 

    errno_t err = fopen_s(&fp,szFileName, "rb");

    if (err != 0)
    {
        fprintf(gpFILE, "createShaders() -> fopen_s() :  failed.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gpFILE, "createShaders() -> fopen_s() :  succeeded.\n");
    }

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);


    fseek(fp, 0L, SEEK_SET);

    char* shaderData = (char*)malloc(sizeof(char) * size);

    if (!shaderData)
    {
        fprintf(gpFILE, "createShaders() -> shaderData size failed.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }

    size_t retVal = fread(shaderData, size, 1, fp);

    if (retVal != 1)
    {
        fprintf(gpFILE, "createShaders() -> fread() :  failed.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gpFILE, "createShaders() -> fread() :  succeeded.\n");
    }

    fclose(fp);

    VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0;
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_vertex_shader);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createShaders() -> vkCreateShaderModule() :  failed.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createShaders() ->  vertex vkCreateShaderModule() :  succeeded.\n");
    }

    if (shaderData)
    {
        free(shaderData);
        shaderData = NULL;
    }


    //-----------------------------------------------------------------------------------------------
    //fragmentShader

        //vertex shader
    szFileName = "shader.frag.spv";
    fp = NULL;
    size = 0;

    err = fopen_s(&fp, szFileName, "rb");

    if (err != 0)
    {
        fprintf(gpFILE, "createShaders() -> fopen_s() :  failed.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gpFILE, "createShaders() -> fopen_s() :  succeeded.\n");
    }

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);


    fseek(fp, 0L, SEEK_SET);

     shaderData = (char*)malloc(sizeof(char) * size);

    if (!shaderData)
    {
        fprintf(gpFILE, "createShaders() -> shaderData size failed.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }

     retVal = fread(shaderData, size, 1, fp);

    if (retVal != 1)
    {
        fprintf(gpFILE, "createShaders() -> fread() :  failed.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gpFILE, "createShaders() -> fread() :  succeeded.\n");
    }

    fclose(fp);


    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0;
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_fragment_shader);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createShaders() -> vkCreateShaderModule() :  failed.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createShaders() ->  fragment vkCreateShaderModule() :  succeeded.\n");
    }

    if (shaderData)
    {
        free(shaderData);
		shaderData = NULL;
    }

    return vkResult;
}

VkResult createDiscriptorSetLayout(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
    memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));

	vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	vkDescriptorSetLayoutCreateInfo.pNext = NULL;
	vkDescriptorSetLayoutCreateInfo.flags = 0;
	vkDescriptorSetLayoutCreateInfo.bindingCount = 0;
	vkDescriptorSetLayoutCreateInfo.pBindings = NULL;//vkDescriptorSetLayoutBindings _Array;

	//VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding;
 //   memset((void*)&vkDescriptorSetLayoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));

	//vkDescriptorSetLayoutBinding.binding = 0;
	//vkDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	//vkDescriptorSetLayoutBinding.descriptorCount = 1;
	//vkDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	//vkDescriptorSetLayoutBinding.pImmutableSamplers = NULL;

	vkResult = vkCreateDescriptorSetLayout(vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createDiscriptorSetLayout() -> vkCreateDescriptorSetLayout() :  failed: %d.\n", vkResult);
        return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "createDiscriptorSetLayout() -> vkCreateDescriptorSetLayout() :  succeeded.\n");
	}

    return(vkResult);
}

VkResult createPipelineLayout(void)
{
	// local variables
	VkResult vkResult = VK_SUCCESS;


    // code
	VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
	memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));

	vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	vkPipelineLayoutCreateInfo.pNext = NULL;
	vkPipelineLayoutCreateInfo.flags = 0;
	vkPipelineLayoutCreateInfo.setLayoutCount = 1;
	vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout;
	vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;

	vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &vkPipelineLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createPipelineLayout() -> vkCreatePipelineLayout() :  failed: %d.\n", vkResult);
		return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "createPipelineLayout() -> vkCreatePipelineLayout() :  succeeded.\n");
	}

	return(vkResult);
}

VkResult createRenderPass(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    // sub-step (1) : Declare and initialize VkAttachmentDescription structures array (Number of elements in the array depends upon number of attachments). Although we have only 1 attachment for this example, we will consider it as an array.
    VkAttachmentDescription vkAttachmentDescription_Array[1];
    memset((void*)vkAttachmentDescription_Array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_Array));

    vkAttachmentDescription_Array[0].flags = 0;
    vkAttachmentDescription_Array[0].format = vkFormat_Color;
    vkAttachmentDescription_Array[0].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_Array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // what to do when entering the RenderPass?
    vkAttachmentDescription_Array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_Array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // though it is saying "stencil" as a member, it is for both depth and stencil
    vkAttachmentDescription_Array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_Array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_Array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // sub-step (2) : Declare and initialize VkAttachmentReference structure which will have information about the attachment which we described above.
    VkAttachmentReference vkAttachmentReference;
    memset((void*)&vkAttachmentReference, 0, sizeof(VkAttachmentReference));

    vkAttachmentReference.attachment = 0; // 0 means the index number of the attachment 
    vkAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // sub-step (3) : Declare and initialize VkSubPassDescription structure and keep reference about VkAttachmentReference structure.
    VkSubpassDescription vkSubpassDescription;
    memset((void*)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));

    vkSubpassDescription.flags = 0;
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkSubpassDescription.inputAttachmentCount = 0;
    vkSubpassDescription.pInputAttachments = NULL;
    vkSubpassDescription.colorAttachmentCount = _ARRAYSIZE(vkAttachmentDescription_Array);
    vkSubpassDescription.pColorAttachments = &vkAttachmentReference;
    vkSubpassDescription.pResolveAttachments = NULL;
    vkSubpassDescription.pDepthStencilAttachment = NULL;
    vkSubpassDescription.preserveAttachmentCount = 0;
    vkSubpassDescription.pPreserveAttachments = NULL;

    // sub-step (4) : Declare and initialize VkRenderPassCreateInfo structure and refer above VkAttachmentDescription and VkSubPassDescription into it. Remember: here also we need attachment information in the form of image views which will be used by framebuffer later. We also need to specify inter-dependency between subpasses if needed.
    VkRenderPassCreateInfo vkRenderPassCreateInfo;
    memset((void*)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));

    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vkRenderPassCreateInfo.pNext = NULL;
    vkRenderPassCreateInfo.attachmentCount = _ARRAYSIZE(vkAttachmentDescription_Array);
    vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_Array;
    vkRenderPassCreateInfo.subpassCount = 1;
    vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
    vkRenderPassCreateInfo.dependencyCount = 0;
    vkRenderPassCreateInfo.pDependencies = NULL;

    // sub-step (5) : Now call vkCreateRenderPass() API to create the actual RenderPass.
    vkResult = vkCreateRenderPass(vkDevice, &vkRenderPassCreateInfo, NULL, &vkRenderPass);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createRenderPass() : vkCreateRenderPass() failed.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createRenderPass() : vkCreateRenderPass() succeeded.\n");
    }

    return(vkResult);
}

VkResult createGraphicsPipeline(void)
{
	// local variables
	VkResult vkResult = VK_SUCCESS;

    // code
    // 
	//vertex input state
	VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1];
	memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));
	vkVertexInputBindingDescription_array[0].binding = 0; // binding index
	vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3; // size of each vertex
	vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex data

    //vertex input attribute state
	VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[1];
	memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));
	vkVertexInputAttributeDescription_array[0].binding = 0; // binding index
	vkVertexInputAttributeDescription_array[0].location = 0; // location index
	vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT; // format of each vertex
	vkVertexInputAttributeDescription_array[0].offset = 0; // offset of each vertex

	//  Declare and initialize VkPipelineVertexInputStateCreateInfo structure.
	VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
	memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
	vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vkPipelineVertexInputStateCreateInfo.pNext = NULL;
	vkPipelineVertexInputStateCreateInfo.flags = 0;
	vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
	vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
	vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
	vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;

	//  Declare and initialize VkPipelineInputAssemblyStateCreateInfo structure.
	VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
	memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
	vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
	vkPipelineInputAssemblyStateCreateInfo.flags = 0;
	vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // triangle list
	vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; // no primitive restart


	//  Declare and initialize VkPipelineRasterizationStateCreateInfo structure.
	VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
	memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
	vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	vkPipelineRasterizationStateCreateInfo.pNext = NULL;
	vkPipelineRasterizationStateCreateInfo.flags = 0;
	vkPipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE; // no depth clamp
    vkPipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE; // no depth bias
    vkPipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f; // no depth bias
    vkPipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f; // no depth bias clamp
    vkPipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f; // no depth bias slope factor
	vkPipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; // no rasterizer discard
	vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // fill mode
	vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT; // back face culling
	vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; //  clockwise front face
	vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f; // line width

	// Color blend state
	VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
	memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));
	vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE; // no blending
	vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // all color components
	//vkPipelineColorBlendAttachmentState_array[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // source color blend factor
	//vkPipelineColorBlendAttachmentState_array[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // destination color blend factor
	//vkPipelineColorBlendAttachmentState_array[0].colorBlendOp = VK_BLEND_OP_ADD; // color blend operation
	//vkPipelineColorBlendAttachmentState_array[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // source alpha blend factor
	//vkPipelineColorBlendAttachmentState_array[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // destination alpha blend factor
	//vkPipelineColorBlendAttachmentState_array[0].alphaBlendOp = VK_BLEND_OP_ADD; // alpha blend operation



    //ColorBlendStateCreateInfo
	VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
	memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
	vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	vkPipelineColorBlendStateCreateInfo.pNext = NULL;
	vkPipelineColorBlendStateCreateInfo.flags = 0;
	vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
	vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;
	vkPipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE; // no logic op
    
	//viewport sciccor state
    memset((void*)&vkViewport, 0, sizeof(VkViewport));
	vkViewport.x = 0.0f;
	vkViewport.y = 0.0f;
	vkViewport.width = (float)vkExtent2D_Swapchain.width;
	vkViewport.height = (float)vkExtent2D_Swapchain.height;
	vkViewport.minDepth = 0.0f;
	vkViewport.maxDepth = 1.0f;

	memset((void*)&vkRect2D_Scissor, 0, sizeof(VkRect2D));
	vkRect2D_Scissor.offset.x = 0;
	vkRect2D_Scissor.offset.y = 0;
	vkRect2D_Scissor.extent.width = vkExtent2D_Swapchain.width;
	vkRect2D_Scissor.extent.height = vkExtent2D_Swapchain.height;

	VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
	memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
	vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vkPipelineViewportStateCreateInfo.pNext = NULL;
	vkPipelineViewportStateCreateInfo.flags = 0;
	vkPipelineViewportStateCreateInfo.viewportCount = 1; // 1 viewport
	vkPipelineViewportStateCreateInfo.pViewports = &vkViewport; // viewport
	vkPipelineViewportStateCreateInfo.scissorCount = 1; // 1 scissor
	vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_Scissor; // scissor

	//// depth stencil state
	//VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;   
	//memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
	//vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	//vkPipelineDepthStencilStateCreateInfo.pNext = NULL;

	//dynamic state (viewport, scissor ,depth bias ,blend constants, stensil mask,line width, etc)
    //no dynamic state right now

	//multisample state
	VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
	memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
	vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	vkPipelineMultisampleStateCreateInfo.pNext = NULL;
	vkPipelineMultisampleStateCreateInfo.flags = 0;
	vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // 1 sample
	//vkPipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE; // no sample shading
	//vkPipelineMultisampleStateCreateInfo.minSampleShading = 0.0f; // no min sample shading
	//vkPipelineMultisampleStateCreateInfo.pSampleMask = NULL; // no sample mask
	//vkPipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE; // no alpha to coverage
	//vkPipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // no alpha to one


	//shader stage state
	VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo[2];
	memset((void*)&vkPipelineShaderStageCreateInfo, 0, sizeof(VkPipelineShaderStageCreateInfo)* _ARRAYSIZE(vkPipelineShaderStageCreateInfo));
	//vertex shader stage
	vkPipelineShaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo[0].pNext = NULL;
	vkPipelineShaderStageCreateInfo[0].flags = 0;
	vkPipelineShaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT; // vertex shader
	vkPipelineShaderStageCreateInfo[0].module = vkShaderModule_vertex_shader; // vertex shader module
	vkPipelineShaderStageCreateInfo[0].pName = "main"; // entry point name
	vkPipelineShaderStageCreateInfo[0].pSpecializationInfo = NULL; // no specialization info
	//fragment shader stage
	vkPipelineShaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo[1].pNext = NULL;
	vkPipelineShaderStageCreateInfo[1].flags = 0;
	vkPipelineShaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; // fragment shader
	vkPipelineShaderStageCreateInfo[1].module = vkShaderModule_fragment_shader; // fragment shader module
	vkPipelineShaderStageCreateInfo[1].pName = "main"; // entry point name
	vkPipelineShaderStageCreateInfo[1].pSpecializationInfo = NULL; // no specialization info

	//tesselation state
	//no tessellation state right now

    
	//pipeline cache 
	VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
	memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
	vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	vkPipelineCacheCreateInfo.pNext = NULL;
	vkPipelineCacheCreateInfo.flags = 0;
	vkPipelineCacheCreateInfo.initialDataSize = 0;
	vkPipelineCacheCreateInfo.pInitialData = NULL;

	VkPipelineCache vkPipelineCache;
	vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createGraphicsPipeline() : vkCreatePipelineCache() failed: %d .\n", vkResult);
		return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "createGraphicsPipeline() : vkCreatePipelineCache() succeeded.\n");
	}

	//  Declare and initialize VkGraphicsPipelineCreateInfo structure.
	VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
	memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
	vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vkGraphicsPipelineCreateInfo.pNext = NULL;
	vkGraphicsPipelineCreateInfo.flags = 0;
	vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pTessellationState = NULL; // no tessellation state
	vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pDepthStencilState = NULL; // no depth stencil state
	vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pDynamicState = NULL; // no dynamic state
	vkGraphicsPipelineCreateInfo.layout = vkPipelineLayout; // pipeline layout
	vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass; // render pass
	vkGraphicsPipelineCreateInfo.subpass = 0; // subpass index
	vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no base pipeline
	vkGraphicsPipelineCreateInfo.basePipelineIndex = -1; // no base pipeline index
	vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo); // number of shader stages
	vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo; // shader stages

                      
	//  Call vkCreateGraphicsPipelines() API to create the graphics pipeline.
	vkResult = vkCreateGraphicsPipelines(vkDevice, vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &vkPipeline);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createGraphicsPipeline() : vkCreateGraphicsPipelines() failed: %d .\n", vkResult);

        //destroy pipeline cache
        vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
        vkPipelineCache = VK_NULL_HANDLE;

		return(vkResult);
	}
	else
	{
		fprintf(gpFILE, "createGraphicsPipeline() : vkCreateGraphicsPipelines() succeeded.\n");
	}

	//destroy pipeline cache
	vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
	vkPipelineCache = VK_NULL_HANDLE;

	return(vkResult);
}

VkResult createFramebuffers(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    // Declare an array of VkImageView equal to the number of attachments. Means in our example, an array of 1 member.
    VkImageView vkImageView_Attachments_Array[1];
    memset((void*)vkImageView_Attachments_Array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_Attachments_Array));

    //  Declare and initialize VkFramebufferCreateInfo structure.
    VkFramebufferCreateInfo vkFramebufferCreateInfo;
    memset((void*)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));

    vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    vkFramebufferCreateInfo.pNext = NULL;
    vkFramebufferCreateInfo.flags = 0;
    vkFramebufferCreateInfo.renderPass = vkRenderPass;
    vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_Attachments_Array);
    vkFramebufferCreateInfo.pAttachments = vkImageView_Attachments_Array;
    vkFramebufferCreateInfo.width = vkExtent2D_Swapchain.width;
    vkFramebufferCreateInfo.height = vkExtent2D_Swapchain.height;
    vkFramebufferCreateInfo.layers = 1;

    //  Allocate the framebuffer array by malloc() equal to the size of swapchain image count.
    vkFramebuffer_Array = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * swapchainImageCount);

    //  Start a loop for swapchain image count and call vkCreateFramebuffer() API to create framebuffers.
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkImageView_Attachments_Array[0] = swapchainImageView_Array[i];

        vkResult = vkCreateFramebuffer(vkDevice, &vkFramebufferCreateInfo, NULL, &vkFramebuffer_Array[i]);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createFramebuffers() : vkCreateFramebuffer() failed for iteration %d.\n", i);
            return(vkResult);
        }
        else
        {
            fprintf(gpFILE, "createFramebuffers() : vkCreateFramebuffer() succeeded for iteration %d.\n", i);
        }
    }

    return(vkResult);
}

VkResult createSemaphores(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    // In CreateSemaphore() user-defined function, declare, memset() and initialize VkSemaphoreCreateInfo structure.
    VkSemaphoreCreateInfo vkSemaphoreCreateInfo;
    memset((void*)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));

    vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkSemaphoreCreateInfo.pNext = NULL; // by default, the semaphore will be created as a binary semaphore. 
    vkSemaphoreCreateInfo.flags = 0;    // MUST be 0, this member is reserved.

    vkSemaphore_BackBuffer = (VkSemaphore*)malloc(sizeof(VkSemaphore) * swapchainImageCount);
    vkSemaphore_RenderComplete = (VkSemaphore*)malloc(sizeof(VkSemaphore) * swapchainImageCount);


	for (int i = 0; i < swapchainImageCount; i++)
	{
        //Now call vkCreateSemaphore() API 2 times to create our 2 semaphore objects. Remember, both will use the same create info structure.
        vkResult = vkCreateSemaphore(
            vkDevice,               // [in] vulkan logical device
            &vkSemaphoreCreateInfo, // [in] pointer to a semaphore create info structure
            NULL,                   // [in, optional] pointer to a custom memory allocator
            &vkSemaphore_BackBuffer[i] // [out] VkSemaphore object
        );

        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createSemaphores() : vkCreateSemaphore() failed for back buffer semaphore.\n");
            return(vkResult);
        }
        else
        {
            fprintf(gpFILE, "createSemaphores() : vkCreateSemaphore() succeeded for back buffer semaphore.\n");
        }

        vkResult = vkCreateSemaphore(
            vkDevice,
            &vkSemaphoreCreateInfo,
            NULL,
            &vkSemaphore_RenderComplete[i]
        );

        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createSemaphores() : vkCreateSemaphore() failed for render complete semaphore.\n");
            return(vkResult);
        }
        else
        {
            fprintf(gpFILE, "createSemaphores() : vkCreateSemaphore() succeeded for render complete semaphore.\n");
        }
	}

    

    return(vkResult);
}

VkResult createFences(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    //  In CreateFences() user-defined function, declare, memset() and initialize VkFenceCreateInfo structure.
    VkFenceCreateInfo vkFenceCreateInfo;
    memset((void*)&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));

    vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkFenceCreateInfo.pNext = NULL;
    vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    //  In this function, allocate our global fence array to the size of the swapchain image count using malloc().
    vkFence_Array = (VkFence*)malloc(sizeof(VkFence) * swapchainImageCount); // for the sake of brevity, we are avoiding error checking for malloc()

    //  Now in a loop, call vkCreateFence() to initialize our global fences array.
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkResult = vkCreateFence(vkDevice, &vkFenceCreateInfo, NULL, &vkFence_Array[i]);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createFences() : vkCreateFence() failed for iteration %d.\n", i);
            return(vkResult);
        }
        else
        {
            fprintf(gpFILE, "createFences() : vkCreateFence() succeeded for iteration %d.\n", i);
        }
    }

    return(vkResult);
}

VkResult buildCommandBuffers(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    //  Start a loop with swapchain image count as the counter.
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        //  Inside the loop, call vkResetCommandBuffer() to reset the contents of command buffers.
        vkResult = vkResetCommandBuffer(
            vkCommandBuffer_Array[i], // [in] which command buffer?
            0                         // [in] Command buffer reset flags. Here, 0 means don't release the resources created by the command pool for these command buffers.
        );

        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "buildCommandBuffers() : vkResetCommandBuffer() failed for command buffer %d.\n", i);
            return(vkResult);
        }
        else
        {
            fprintf(gpFILE, "buildCommandBuffers() : vkResetCommandBuffer() succeeded for command buffer %d.\n", i);
        }

        // Then declare, memset(), initialize VkCommandBufferBeginInfo structure.
        VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
        memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

        vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkCommandBufferBeginInfo.pNext = NULL;
        vkCommandBufferBeginInfo.flags = 0; // Here, this zero indicates (1) We will use only primary command buffer, (2) We are not going to use this command buffer simultaneously between multiple threads.

        //  Now call vkBeginCommandBuffer() API to record different vulkan drawing related commands. Do error checking.
        vkResult = vkBeginCommandBuffer(vkCommandBuffer_Array[i], &vkCommandBufferBeginInfo);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "buildCommandBuffers() : vkBeginCommandBuffer() failed for command buffer %d.\n", i);
            return(vkResult);
        }
        else
        {
            fprintf(gpFILE, "buildCommandBuffers() : vkBeginCommandBuffer() succeeded for command buffer %d.\n", i);
        }

        //  Declare, memset() and initialize struct array of VkClearValue type. Remember : internally, this is union. Our array will be of 1 element. This number depends upon the number of attachments to the framebuffer. As we have only 1 attachment (Color), hence the array is of 1 element. For color, .color member is meaningful, .depthStencil value is meaningless. When depth attachment will be there, vice versa will be true. To this color member, we need to assign a VkClearColorValue structure. To do this, declare globally the VkClearColorValue structure variable and memset() and initialize it in initialize(). Remember : we are going to clear .color member of VkClearValue structure by VkClearColorValue structure because in Step (16) of RenderPass, we specified .loadOp member of VkAttachmentDescription structure to VK_ATTACHMENT_LOAD_OP_CLEAR.
        VkClearValue vkClearValue_Array[1];
        memset((void*)vkClearValue_Array, 0, sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_Array));

        vkClearValue_Array[0].color = vkClearColorValue;

        //  Then, declare, memset() and initialize the VkRenderPassBeginInfo structure.
        VkRenderPassBeginInfo vkRenderPassBeginInfo;
        memset((void*)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));

        vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        vkRenderPassBeginInfo.pNext = NULL;
        vkRenderPassBeginInfo.renderPass = vkRenderPass;
        vkRenderPassBeginInfo.renderArea.offset.x = 0; // these 4 members are analogous to glViewport() or D3D Viewport
        vkRenderPassBeginInfo.renderArea.offset.y = 0;
        vkRenderPassBeginInfo.renderArea.extent.width = vkExtent2D_Swapchain.width;
        vkRenderPassBeginInfo.renderArea.extent.height = vkExtent2D_Swapchain.height;
        vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_Array);
        vkRenderPassBeginInfo.pClearValues = vkClearValue_Array;
        vkRenderPassBeginInfo.framebuffer = vkFramebuffer_Array[i];

        //  Then, begin the render pass by calling vkCmdBeginRenderPass(). Remember : the code written in “begin render pass” and “end render pass” itself is the code of sub-pass if no sub-pass is explicitly created. In other words, even if no sub-pass is declared explicitly, there is always 1 sub-pass for a render pass.
        vkCmdBeginRenderPass(
            vkCommandBuffer_Array[i],  // [in] command buffer
            &vkRenderPassBeginInfo,    // [in] render pass begin info
            VK_SUBPASS_CONTENTS_INLINE // [in] subpass contents (inline means : contents of this renderpass are contents of the subpass and part of the primary command buffer)
        );

        // here, we should call Vulkan drawing functions

		//  Bind the pipeline by calling vkCmdBindPipeline(). Do error checking.
		vkCmdBindPipeline(
			vkCommandBuffer_Array[i], // [in] command buffer
			VK_PIPELINE_BIND_POINT_GRAPHICS, // [in] pipeline bind point
			vkPipeline                // [in] pipeline object
		);

		//  Bind the vertex buffer by calling vkCmdBindVertexBuffers(). Do error checking.
        VkDeviceSize vkDeviceSize_VertexBuffer_offset_array[1];
		memset((void*)vkDeviceSize_VertexBuffer_offset_array, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_VertexBuffer_offset_array));    
		vkDeviceSize_VertexBuffer_offset_array[0] = 0; // offset of the vertex buffer

		vkCmdBindVertexBuffers(
			vkCommandBuffer_Array[i], // [in] command buffer
			0,                       // [in] binding index
			1,                       // [in] vertex buffer count
			&vertexData_position.vkBuffer,         // [in] vertex buffer object
			vkDeviceSize_VertexBuffer_offset_array // [in] offset array
		);

		vkCmdDraw(
			vkCommandBuffer_Array[i], // [in] command buffer
			3, // [in] number of vertices to draw
			1,                           // [in] instance count
			0,                           // [in] first vertex
			0                            // [in] first instance
		);

        //  End the render pass by calling vkCmdEndRenderPass().
        vkCmdEndRenderPass(vkCommandBuffer_Array[i]);

        // End the recording of the command buffer by calling vkEndCommandBuffer(). Do error checking
        vkResult = vkEndCommandBuffer(vkCommandBuffer_Array[i]);

        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "buildCommandBuffers() : vkEndCommandBuffer() failed for command buffer %d.\n", i);
            return(vkResult);
        }
        else
        {
            fprintf(gpFILE, "buildCommandBuffers() : vkEndCommandBuffer() succeeded for command buffer %d.\n", i);
        }

        //  Close the loop.
    }

    return(vkResult);
}


VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT vkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    //code
    fprintf(gpFILE, "NDT_Validation: %s %d = %s \n", pLayerPrefix , messageCode , pMessage);

    return VK_FALSE;
}

