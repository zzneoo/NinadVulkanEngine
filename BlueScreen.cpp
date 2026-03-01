// header files
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYDDSLOADER_IMPLEMENTATION
#include "tinyddsloader.h"
using namespace tinyddsloader;

//#define PRINT_EXTENIONS

#define MAX_FRAMES 2 // for double buffering

// Vulkan related header files
#define VK_USE_PLATFORM_WIN32_KHR // define the current Vulkan platform
#include <vulkan/vulkan.h>        // you must define platform before including this file (Windows / Linux / macOS / iOS / Android / <other>)

//GLM related header files
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Assimp related header files
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#define IMGUI_ENABLE

#ifdef IMGUI_ENABLE
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_win32.h"
#endif // IMGUI_ENABLE

#include <cstdint>
#include <windowsx.h>
#include <iostream>

// Vulkan related libraries
#pragma comment(lib, "vulkan-1.lib")
#pragma comment(lib, "assimp-vc143-mt.lib")

#include "VK.h"
#include "Camera.h"

//Material BasicPBR
#include "Material_BasicPBR.h"

#include "DescriptorSetLayouts.h"
#include "GraphicsPipelines.h"
#include "AnimatedModel.h"

// macros
#define WIN_WIDTH  1920
#define WIN_HEIGHT 1080
#define WIN_TITLE  TEXT("NDT:Vulkan AstroMediComp")
#define LINE_END     "-------------------------------------------------------------------------------------\n"

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#ifdef IMGUI_ENABLE
//IMGUI related global 
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
VkDescriptorPool gImguiDescriptorPool;
ImGuiIO* g_io = nullptr;

static float fFactor = 0.0f;
static glm::vec3 v3Color = glm::vec3(0.0f);
#endif // IMGUI_ENABLE

// global variable declarations
const char* gpszAppName = "ARTR";
HWND            ghwnd = NULL;
BOOL            gbFullscreen = FALSE;
BOOL            gbActive = FALSE;
WINDOWPLACEMENT wpPrev;
DWORD           dwStyle;

// for file IO
FILE* gpFILE = NULL;

Camera camera; // camera object

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

// VK_KHR_SWAPCHAIN_EXTENSION_NAME
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
uint32_t     gSwapchainImageCount = UINT32_MAX;

// Command pool
VkCommandPool vkCommandPool = VK_NULL_HANDLE;

// Command buffers
VkCommandBuffer* vkCommandBuffer_Array = NULL;

// RenderPass
//VkRenderPass vkRenderPass = VK_NULL_HANDLE;

// Framebuffers
//VkFramebuffer* vkFramebuffer_Array = NULL;

// Semaphores
VkSemaphore vkSemaphore_Timeline = VK_NULL_HANDLE;
VkSemaphore* vkSemaphore_BackBuffer = VK_NULL_HANDLE;
VkSemaphore* vkSemaphore_RenderComplete = VK_NULL_HANDLE;

uint64_t gTimelineValue = 0;
uint64_t gFrameTimelineValues[MAX_FRAMES] = {};

// Fences
//VkFence* vkFence_Array = VK_NULL_HANDLE;

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

// Swapchain resources
SwapChainResourceData gSwapChainResourceData;

// position
VulkanData vertexData_coloredTriangle;
VulkanData vertexData_Axis;
//VulkanData vertexData_Impostor;
//VulkanData indexData_Impostor;

VulkanComboData  ImpostorBufferData;
VulkanComboData  CubeBufferData;
VulkanComboData  SuzanneBufferData;
VulkanComboData  SphereBufferData;
//VulkanComboData  RatBufferData;

AnimatedModel* pModel_Rat = NULL;


const uint16_t triangle_indices[] =
{
    0, 1, 2, // first triangle
};

const uint16_t quad_indices[] =
{
    0, 2, 3, // first triangle
    0, 1, 2, // second triangle
};



//UniformData uniformBufferData_camera[MAX_FRAMES];
UniformData uniformBufferData_frameData[MAX_FRAMES];

//GrassImage
ImageData grassTextureData;
ImageData borderTextureData;
ImageData imposterTextureData;
ImageData imposterTextureData_blackAlder_Field_02_Albedo;
ImageData imposterTextureData_blackAlder_Field_02_Normal;

//Samplers
VkSampler vkSampler_LinearClampAniso = VK_NULL_HANDLE;//need mipmaps to be generated for aniso 
VkSampler vkSampler_LinearClamp = VK_NULL_HANDLE;
VkSampler vkSampler_LinearMipmapClamp = VK_NULL_HANDLE;

VkDescriptorPool vkDescriptorPool = VK_NULL_HANDLE;
VkDescriptorSet  vkDescriptorSets_frameData[MAX_FRAMES];
VkDescriptorSet  vkDescriptorSets_frameDataBoneData[MAX_FRAMES];
VkDescriptorSet  vkDescriptorSet_SingleImage;
VkDescriptorSet  vkDescriptorSet_AlbedoNormal;

Material_BasicPBR* pMaterial_BasicPBR_RockyGround = NULL;
Material_BasicPBR* pMaterial_BasicPBR_GrassyGround = NULL;

std::vector<ImageData*> global_textureArray;
VkDescriptorPool global_textureArray_vkDescriptorPool;
VkDescriptorSet global_textureArray_vkDescriptorSet;

//desccriptor set layouts 
DescriptorSetLayouts* gpDescriptorSetLayouts = NULL;

//for pipeline
VkViewport vkViewport;
VkRect2D vkRect2D_Scissor;

//All pipelines

GraphicsPipelines* gpGraphicsPipelines = NULL;

// entry-point function
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpszCmdLine, _In_ int iCmdShow)
{
    // function prototypes
    VkResult initialize(void);
    VkResult display(void);
    void update(void);
    void uninitialize();

    // For Windows 10 and later
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // local variable declarations
    MSG        msg;
    WNDCLASSEX wndclass;
    TCHAR      szAppName[255];
    HWND       hwnd = NULL;
    BOOL       bDone = FALSE;
    VkResult   vkResult = VK_SUCCESS;

    // code
    // open the log file
    fopen_s(&gpFILE, "ZzLog.txt", "w");
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
        fprintf(gpFILE, "\n\n\n\nWinMain() : initialize() succeeded.\n");
        fprintf(gpFILE, LINE_END);
        fprintf(gpFILE, LINE_END);
    }

    // show the window
    if (hwnd)
    {
        ShowWindow(hwnd, iCmdShow);
        SetForegroundWindow(hwnd);
        SetFocus(hwnd);
    }

    // globals (or static in your function)
    LARGE_INTEGER prevTime;
    LARGE_INTEGER freq;
    double    secsPerCount;

    // before your main loop, once:
    QueryPerformanceFrequency(&freq);
    secsPerCount = 1.0 / double(freq.QuadPart);
    QueryPerformanceCounter(&prevTime);


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
            //delta time calculation
                    // get current time
            LARGE_INTEGER curTime;
            QueryPerformanceCounter(&curTime);

            // compute delta
            double deltaTime = double(curTime.QuadPart - prevTime.QuadPart) * secsPerCount;

            // store it for your game/camera update
            MyWin32::fDeltaTime = float(deltaTime);

            // update prevTime for the next iteration!
            prevTime = curTime;

            // Render
            vkResult = display();

            if (vkResult != VK_FALSE && vkResult != VK_SUCCESS)
            {
                fprintf(gpFILE, "WinMain() : display() failed (%d).\n", vkResult);
                bDone = TRUE;
            }

            // Update
            update();

            MyWin32::iFrameID++;
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
    VkResult resize(int width, int height);
    void ToggleFullscreen(void);

#ifdef IMGUI_ENABLE
    if (ImGui_ImplWin32_WndProcHandler(hwnd, iMsg, wParam, lParam))
        return true; // ImGui handled it
#endif // IMGUI_ENABLE

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
        if (wParam == SIZE_MINIMIZED)
        {
            gbActive = FALSE;
            return(0);
        }

        MyWin32::myClientSize.ClientWidth = LOWORD(lParam);
        MyWin32::myClientSize.ClientHeight = HIWORD(lParam);

        resize(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_MOUSEMOVE:
        MyWin32::myCamStruct.mouseX = GET_X_LPARAM(lParam);
        MyWin32::myCamStruct.mouseY = GET_Y_LPARAM(lParam);
        break;

    case WM_KEYDOWN:

        switch (LOWORD(wParam))
        {
        case 0x57: //W          
            MyWin32::myCamStruct.bCameraMoving_Forward = true;
            break;

        case 0x41: //A
            MyWin32::myCamStruct.bCameraMoving_Left = true;
            break;

        case 0x53: //S
            MyWin32::myCamStruct.bCameraMoving_Backward = true;
            break;

        case 0x44: //D
            MyWin32::myCamStruct.bCameraMoving_Right = true;
            break;

        case VK_SHIFT:
            MyWin32::myCamStruct.CameraTurboSpeed = 7.0f;
            break;


        case 0x47: //G
            if (FALSE == MyWin32::isGUI)
            {
                MyWin32::isGUI = TRUE;
                MyWin32::bFirstFrame = TRUE;
                ShowCursor(TRUE);
            }
            else
            {
                MyWin32::isGUI = FALSE;
                ShowCursor(FALSE);
            }
            break;

            //0 to 9
            {
        case 0x30:
            camera.CameraTurboSpeed(0, false);
            break;
        case 0x31:
            camera.CameraTurboSpeed(1, false);
            break;
        case 0x32:
            camera.CameraTurboSpeed(2, false);
            break;
        case 0x33:
            camera.CameraTurboSpeed(3, false);
            break;
        case 0x34:
            camera.CameraTurboSpeed(4, false);
            break;
        case 0x35:
            camera.CameraTurboSpeed(5, false);
            break;
        case 0x36:
            camera.CameraTurboSpeed(6, false);
            break;
        case 0x37:
            camera.CameraTurboSpeed(7, false);
            break;
        case 0x38:
            camera.CameraTurboSpeed(8, false);
            break;
        case 0x39:
            camera.CameraTurboSpeed(9, false);
            break;
            }

        case VK_ESCAPE:
            DestroyWindow(hwnd);
            break;
        default:
            break;
        }
        break;

    case WM_KEYUP:

        switch (wParam)
        {

        case 0x57: //W          
            MyWin32::myCamStruct.bCameraMoving_Forward = false;
            break;

        case 0x41: //A
            MyWin32::myCamStruct.bCameraMoving_Left = false;
            break;

        case 0x53: //S
            MyWin32::myCamStruct.bCameraMoving_Backward = false;
            break;

        case 0x44: //D
            MyWin32::myCamStruct.bCameraMoving_Right = false;
            break;

        case VK_SHIFT:
            MyWin32::myCamStruct.CameraTurboSpeed = 1.0f;
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

                //ShowCursor(FALSE);

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

        //ShowCursor(TRUE);

        gbFullscreen = FALSE;
    }
}

#ifdef IMGUI_ENABLE
static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;

    fprintf(gpFILE, "[vulkan] Error: VkResult = %d\n", err);

    // Optionally you can translate the error code to something more readable:
    switch (err)
    {
    case VK_ERROR_OUT_OF_HOST_MEMORY:       fprintf(gpFILE, "VK_ERROR_OUT_OF_HOST_MEMORY\n"); break;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:     fprintf(gpFILE, "VK_ERROR_OUT_OF_DEVICE_MEMORY\n"); break;
    case VK_ERROR_INITIALIZATION_FAILED:    fprintf(gpFILE, "VK_ERROR_INITIALIZATION_FAILED\n"); break;
    case VK_ERROR_DEVICE_LOST:              fprintf(gpFILE, "VK_ERROR_DEVICE_LOST\n"); break;
    case VK_ERROR_MEMORY_MAP_FAILED:        fprintf(gpFILE, "VK_ERROR_MEMORY_MAP_FAILED\n"); break;
    case VK_ERROR_LAYER_NOT_PRESENT:        fprintf(gpFILE, "VK_ERROR_LAYER_NOT_PRESENT\n"); break;
    case VK_ERROR_EXTENSION_NOT_PRESENT:    fprintf(gpFILE, "VK_ERROR_EXTENSION_NOT_PRESENT\n"); break;
    case VK_ERROR_FEATURE_NOT_PRESENT:      fprintf(gpFILE, "VK_ERROR_FEATURE_NOT_PRESENT\n"); break;
    case VK_ERROR_INCOMPATIBLE_DRIVER:      fprintf(gpFILE, "VK_ERROR_INCOMPATIBLE_DRIVER\n"); break;
    case VK_ERROR_TOO_MANY_OBJECTS:         fprintf(gpFILE, "VK_ERROR_TOO_MANY_OBJECTS\n"); break;
    case VK_ERROR_FORMAT_NOT_SUPPORTED:     fprintf(gpFILE, "VK_ERROR_FORMAT_NOT_SUPPORTED\n"); break;
    case VK_ERROR_FRAGMENTED_POOL:          fprintf(gpFILE, "VK_ERROR_FRAGMENTED_POOL\n"); break;
    default:                                fprintf(gpFILE, "Unknown Vulkan error\n"); break;
    }

#ifdef _DEBUG
    __debugbreak(); // Only on Windows in debug mode
#endif

    exit(-1);
}
#endif // IMGUI_ENABLE

int file_exists(const char* path) {
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
}

// compile shader using build.bat
void compileShaderVS_FS(const char* shaderName)
{
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    char command[256];

    // Build the command line: cmd.exe /c "build.bat Impostor"
    snprintf(command, sizeof(command), "cmd.exe /c \"build.bat %s\"", shaderName);

    BOOL success = CreateProcessA(
        NULL,           // App name (null if included in command line)
        (LPSTR)command, // Command line (must be mutable)
        NULL, NULL,     // Process/thread security
        FALSE,          // Inherit handles
        0,              // Creation flags
        NULL,           // Environment
        NULL,           // Current directory
        &si, &pi
    );

    if (!success)
    {
        std::cerr << "Failed to run process: " << GetLastError() << std::endl;
        exit(EXIT_FAILURE);
    }

    // Wait until the process exits
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);


    //------------verify if shader files exist----------------
    char filename[256];
    snprintf(filename, sizeof(filename), "%s.vert.spv", shaderName);
    if (!file_exists(filename))
    {
        STARTUPINFOA sInfo = { sizeof(sInfo) };
        PROCESS_INFORMATION pInfo;
        snprintf(command, sizeof(command), "notepad.exe \"%s\"", "vsCompileLog.txt");
        CreateProcessA(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &sInfo, &pInfo);
        // Wait until the process exits
        WaitForSingleObject(pInfo.hProcess, INFINITE);
        CloseHandle(pInfo.hProcess);
        CloseHandle(pInfo.hThread);
        exit(EXIT_FAILURE);
    }
    snprintf(filename, sizeof(filename), "%s.frag.spv", shaderName);
    if (!file_exists(filename))
    {
        STARTUPINFOA sInfo = { sizeof(sInfo) };
        PROCESS_INFORMATION pInfo;
        snprintf(command, sizeof(command), "notepad.exe \"%s\"", "fsCompileLog.txt");
        CreateProcessA(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &sInfo, &pInfo);
        // Wait until the process exits
        WaitForSingleObject(pInfo.hProcess, INFINITE);
        CloseHandle(pInfo.hProcess);
        CloseHandle(pInfo.hThread);
        exit(EXIT_FAILURE);
    }
}

VkResult LoadModel_Phong(const char* modelPath, VulkanComboData* vulkanComboData, bool index32)
{
    VkResult ZzCreateVertexAndIndex16Buffer(
        const float* vertices, VkDeviceSize vertexBufferSize,
        const uint16_t * indices, VkDeviceSize indexBufferSize,
        VulkanComboData * vulkanComboData
    );
    VkResult ZzCreateVertexAndIndex32Buffer(
        const float* vertices, VkDeviceSize vertexBufferSize,
        const uint32_t * indices, VkDeviceSize indexBufferSize,
        VulkanComboData * vulkanComboData
    );

    VkResult vkResult = VK_SUCCESS;//"Resources/Models/Test/suzanne.obj"

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(modelPath,
        aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipWindingOrder);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
        fprintf(gpFILE, "LoadModel_Phong() : Assimp Importer::ReadFile() failed.\n");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    //std::cout << "Meshes: " << scene->mNumMeshes << std::endl;

    std::vector<VertexData_PositionTexCoordNormalColor> vkVertices;

    if (index32)
    {
        std::vector<uint32_t> vkIndices;

        if (scene && scene->mRootNode) {
            for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
                aiMesh* mesh = scene->mMeshes[meshIndex];

                // Vertices
                for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                    VertexData_PositionTexCoordNormalColor v{};

                    // Position
                    if (mesh->HasPositions()) {
                        v.pos = glm::vec3(
                            mesh->mVertices[i].x,
                            mesh->mVertices[i].y,
                            mesh->mVertices[i].z
                        );
                    }

                    // Normal
                    if (mesh->HasNormals()) {
                        v.normal = glm::vec3(
                            mesh->mNormals[i].x,
                            mesh->mNormals[i].y,
                            mesh->mNormals[i].z
                        );
                    }

                    // TexCoords (Assimp supports 8 UV channels; we use channel 0)
                    if (mesh->HasTextureCoords(0)) {
                        v.texCoord = glm::vec2(
                            mesh->mTextureCoords[0][i].x,
                            mesh->mTextureCoords[0][i].y
                        );
                    }
                    else {
                        v.texCoord = glm::vec2(0.0f);
                    }

                    // Vertex Colors (channel 0, RGBA)
                    if (mesh->HasVertexColors(0)) {
                        v.color = glm::vec3(
                            mesh->mColors[0][i].r,
                            mesh->mColors[0][i].g,
                            mesh->mColors[0][i].b
                        );
                    }
                    else {
                        v.color = glm::vec3(1.0f); // default white
                    }

                    vkVertices.push_back(v);
                }

                // Indices (faces are always triangles if aiProcess_Triangulate is used)
                for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
                    const aiFace& face = mesh->mFaces[f];
                    for (unsigned int j = 0; j < face.mNumIndices; j++) {
                        vkIndices.push_back(face.mIndices[j]);
                    }
                }
            }
        }

        vulkanComboData->indicesCount = static_cast<uint32_t>(vkIndices.size());

        //Suzanne
        vkResult = ZzCreateVertexAndIndex32Buffer(
            (float*)vkVertices.data(), vkVertices.size() * sizeof(VertexData_PositionTexCoordNormalColor),
            vkIndices.data(), vkIndices.size() * sizeof(uint32_t),
            vulkanComboData
        );
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "initialize() : ZzCreateVetexAndIndexBuffer() for phong failed (%d).\n", vkResult);
            return(vkResult);
        }
    }
    else
    {
        std::vector<uint16_t> vkIndices;

        if (scene && scene->mRootNode) {
            for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
                aiMesh* mesh = scene->mMeshes[meshIndex];

                // Vertices
                for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                    VertexData_PositionTexCoordNormalColor v{};

                    // Position
                    if (mesh->HasPositions()) {
                        v.pos = glm::vec3(
                            mesh->mVertices[i].x,
                            mesh->mVertices[i].y,
                            mesh->mVertices[i].z
                        );
                    }

                    // Normal
                    if (mesh->HasNormals()) {
                        v.normal = glm::vec3(
                            mesh->mNormals[i].x,
                            mesh->mNormals[i].y,
                            mesh->mNormals[i].z
                        );
                    }

                    // TexCoords (Assimp supports 8 UV channels; we use channel 0)
                    if (mesh->HasTextureCoords(0)) {
                        v.texCoord = glm::vec2(
                            mesh->mTextureCoords[0][i].x,
                            mesh->mTextureCoords[0][i].y
                        );
                    }
                    else {
                        v.texCoord = glm::vec2(0.0f);
                    }

                    // Vertex Colors (channel 0, RGBA)
                    if (mesh->HasVertexColors(0)) {
                        v.color = glm::vec3(
                            mesh->mColors[0][i].r,
                            mesh->mColors[0][i].g,
                            mesh->mColors[0][i].b
                        );
                    }
                    else {
                        v.color = glm::vec3(1.0f); // default white
                    }

                    vkVertices.push_back(v);
                }

                // Indices (faces are always triangles if aiProcess_Triangulate is used)
                for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
                    const aiFace& face = mesh->mFaces[f];
                    for (unsigned int j = 0; j < face.mNumIndices; j++) {
                        vkIndices.push_back(face.mIndices[j]);
                    }
                }
            }
        }

        vulkanComboData->indicesCount = static_cast<uint32_t>(vkIndices.size());

        //Suzanne
        vkResult = ZzCreateVertexAndIndex16Buffer(
            (float*)vkVertices.data(), vkVertices.size() * sizeof(VertexData_PositionTexCoordNormalColor),
            vkIndices.data(), vkIndices.size() * sizeof(uint16_t),
            vulkanComboData
        );
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "initialize() : ZzCreateVetexAndIndexBuffer() for phong failed (%d).\n", vkResult);
            return(vkResult);
        }
    }

    return vkResult;
}

VkResult LoadModel_PBR(const char* modelPath, VulkanComboData* vulkanComboData, bool index32)
{
    VkResult vkResult = VK_SUCCESS;
    // Similar implementation as LoadModel_Phong but for PBR-specific vertex structure
    // This function is a placeholder and should be implemented similarly to LoadModel_Phong
    VkResult ZzCreateVertexAndIndex16Buffer(
        const float* vertices, VkDeviceSize vertexBufferSize,
        const uint16_t * indices, VkDeviceSize indexBufferSize,
        VulkanComboData * vulkanComboData
    );
    VkResult ZzCreateVertexAndIndex32Buffer(
        const float* vertices, VkDeviceSize vertexBufferSize,
        const uint32_t * indices, VkDeviceSize indexBufferSize,
        VulkanComboData * vulkanComboData
    );

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(modelPath,
        aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipWindingOrder | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
        fprintf(gpFILE, "LoadModel_PBR() : Assimp Importer::ReadFile() failed.\n");
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    //std::cout << "Meshes: " << scene->mNumMeshes << std::endl;

    std::vector<VertexData_PositionTexCoordNormalTangent> vkVertices;

    if (index32)
    {
        std::vector<uint32_t> vkIndices;

        if (scene && scene->mRootNode) {
            for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
                aiMesh* mesh = scene->mMeshes[meshIndex];

                // Vertices
                for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                    VertexData_PositionTexCoordNormalTangent v{};

                    // Position
                    if (mesh->HasPositions()) {
                        v.pos = glm::vec3(
                            mesh->mVertices[i].x,
                            mesh->mVertices[i].y,
                            mesh->mVertices[i].z
                        );
                    }

                    // Normal
                    if (mesh->HasNormals())
                    {
                        v.normal = glm::vec3(
                            mesh->mNormals[i].x,
                            mesh->mNormals[i].y,
                            mesh->mNormals[i].z
                        );
                    }
                    else {
                        v.normal = glm::vec3(0.0f, 0.0f, 1.0f); // default fallback
                    }

                    // TexCoords (Assimp supports 8 UV channels; we use channel 0)
                    if (mesh->HasTextureCoords(0))
                    {
                        v.texCoord = glm::vec2(
                            mesh->mTextureCoords[0][i].x,
                            mesh->mTextureCoords[0][i].y
                        );
                    }
                    else {
                        v.texCoord = glm::vec2(0.0f);
                    }

                    // Vertex Tangents 
                    if (mesh->HasTangentsAndBitangents()) {
                        v.tangent = glm::vec3(
                            mesh->mTangents[i].x,
                            mesh->mTangents[i].y,
                            mesh->mTangents[i].z
                        );
                    }
                    else {
                        v.tangent = glm::vec3(1.0f, 0.0f, 0.0f); // default fallback
                    }

                    vkVertices.push_back(v);
                }

                // Indices (faces are always triangles if aiProcess_Triangulate is used)
                for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
                    const aiFace& face = mesh->mFaces[f];
                    for (unsigned int j = 0; j < face.mNumIndices; j++) {
                        vkIndices.push_back(face.mIndices[j]);
                    }
                }
            }
        }

        vulkanComboData->indicesCount = static_cast<uint32_t>(vkIndices.size());

        //PBR model
        vkResult = ZzCreateVertexAndIndex32Buffer(
            (float*)vkVertices.data(), vkVertices.size() * sizeof(VertexData_PositionTexCoordNormalTangent),
            vkIndices.data(), vkIndices.size() * sizeof(uint32_t),
            vulkanComboData
        );
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "initialize() : ZzCreateVetexAndIndexBuffer() for PBR model failed (%d).\n", vkResult);
            return(vkResult);
        }
    }
    else
    {
        std::vector<uint16_t> vkIndices;

        if (scene && scene->mRootNode) {
            for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
                aiMesh* mesh = scene->mMeshes[meshIndex];

                // Vertices
                for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                    VertexData_PositionTexCoordNormalTangent v{};

                    // Position
                    if (mesh->HasPositions()) {
                        v.pos = glm::vec3(
                            mesh->mVertices[i].x,
                            mesh->mVertices[i].y,
                            mesh->mVertices[i].z
                        );
                    }

                    // Normal
                    if (mesh->HasNormals())
                    {
                        v.normal = glm::vec3(
                            mesh->mNormals[i].x,
                            mesh->mNormals[i].y,
                            mesh->mNormals[i].z
                        );
                    }
                    else {
                        v.normal = glm::vec3(0.0f, 0.0f, 1.0f); // default fallback
                    }

                    // TexCoords (Assimp supports 8 UV channels; we use channel 0)
                    if (mesh->HasTextureCoords(0))
                    {
                        v.texCoord = glm::vec2(
                            mesh->mTextureCoords[0][i].x,
                            mesh->mTextureCoords[0][i].y
                        );
                    }
                    else {
                        v.texCoord = glm::vec2(0.0f);
                    }

                    // Vertex Tangents 
                    if (mesh->HasTangentsAndBitangents()) {
                        v.tangent = glm::vec3(
                            mesh->mTangents[i].x,
                            mesh->mTangents[i].y,
                            mesh->mTangents[i].z
                        );
                    }
                    else {
                        v.tangent = glm::vec3(1.0f, 0.0f, 0.0f); // default fallback
                    }

                    vkVertices.push_back(v);
                }

                // Indices (faces are always triangles if aiProcess_Triangulate is used)
                for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
                    const aiFace& face = mesh->mFaces[f];
                    for (unsigned int j = 0; j < face.mNumIndices; j++) {
                        vkIndices.push_back(face.mIndices[j]);
                    }
                }
            }
        }

        vulkanComboData->indicesCount = static_cast<uint32_t>(vkIndices.size());

        //Suzanne
        vkResult = ZzCreateVertexAndIndex16Buffer(
            (float*)vkVertices.data(), vkVertices.size() * sizeof(VertexData_PositionTexCoordNormalTangent),
            vkIndices.data(), vkIndices.size() * sizeof(uint16_t),
            vulkanComboData
        );
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "initialize() : ZzCreateVetexAndIndexBuffer() for PBR model failed (%d).\n", vkResult);
            return(vkResult);
        }
    }

    return VK_SUCCESS;
}

//Skeleton animation utilities

//// Convert aiMatrix4x4 to glm::mat4 (Assimp row-major)
//static glm::mat4 aiMat4ToGlm(const aiMatrix4x4& m) {
//    // Assimp stores row-major; glm::mat4 expects column-major when accessed with ptr.
//    glm::mat4 out;
//    out[0][0] = m.a1; out[1][0] = m.a2; out[2][0] = m.a3; out[3][0] = m.a4;
//    out[0][1] = m.b1; out[1][1] = m.b2; out[2][1] = m.b3; out[3][1] = m.b4;
//    out[0][2] = m.c1; out[1][2] = m.c2; out[2][2] = m.c3; out[3][2] = m.c4;
//    out[0][3] = m.d1; out[1][3] = m.d2; out[2][3] = m.d3; out[3][3] = m.d4;
//    return out;
//}
//
//// Utility: add bone influence to a vertex (keep up to 4 influences; replace smallest if needed)
//static void AddBoneDataToVertex(VertexData_Skinned& v, int boneIndex, float weight) {
//    // Find first zero-weight slot
//    for (int i = 0; i < 4; ++i) {
//        if (v.boneWeights[i] == 0.0f) {
//            v.boneIDs[i] = boneIndex;
//            v.boneWeights[i] = weight;
//            return;
//        }
//    }
//    // All slots taken: find smallest weight slot and replace if this weight is larger
//    int minIdx = 0;
//    float minVal = v.boneWeights[0];
//    for (int i = 1; i < 4; ++i) {
//        if (v.boneWeights[i] < minVal) {
//            minVal = v.boneWeights[i];
//            minIdx = i;
//        }
//    }
//    if (weight > minVal) {
//        v.boneIDs[minIdx] = boneIndex;
//        v.boneWeights[minIdx] = weight;
//    }
//}
//
//double TicksPerSecond(const aiAnimation* anim)
//{
//    return anim->mTicksPerSecond != 0.0
//        ? anim->mTicksPerSecond
//        : 25.0f;
//}
//
//double AnimationTime(const aiAnimation* anim, float timeInSeconds)
//{
//    float timeInTicks = timeInSeconds * (float)TicksPerSecond(anim);
//    return fmod(timeInTicks, anim->mDuration);
//}
//
//uint32_t FindPositionKey(float animationTime, const aiNodeAnim* channel)
//{
//    for (uint32_t i = 0; i < channel->mNumPositionKeys - 1; i++)
//    {
//        if (animationTime < channel->mPositionKeys[i + 1].mTime)
//            return i;
//    }
//    return channel->mNumPositionKeys - 2;
//}
//
//uint32_t FindRotationKey(float animationTime, const aiNodeAnim* channel)
//{
//    for (uint32_t i = 0; i < channel->mNumRotationKeys - 1; i++)
//    {
//        if (animationTime < channel->mRotationKeys[i + 1].mTime)
//            return i;
//    }
//    return channel->mNumRotationKeys - 2;
//}
//
//uint32_t FindScalingKey(float animationTime, const aiNodeAnim* channel)
//{
//    for (uint32_t i = 0; i < channel->mNumScalingKeys - 1; i++)
//    {
//        if (animationTime < channel->mScalingKeys[i + 1].mTime)
//            return i;
//    }
//    return channel->mNumScalingKeys - 2;
//}
//
//glm::vec3 InterpolatePosition(float time, const aiNodeAnim* channel)
//{
//    if (channel->mNumPositionKeys == 1)
//        return glm::vec3(channel->mPositionKeys[0].mValue.x,
//            channel->mPositionKeys[0].mValue.y,
//            channel->mPositionKeys[0].mValue.z);
//
//    int index = FindPositionKey(time, channel);
//    int next = index + 1;
//
//    float delta =
//        (float)channel->mPositionKeys[next].mTime -
//        (float)channel->mPositionKeys[index].mTime;
//
//    float factor =
//        (float)(time - channel->mPositionKeys[index].mTime) / delta;
//
//    auto& a = channel->mPositionKeys[index].mValue;
//    auto& b = channel->mPositionKeys[next].mValue;
//
//    return glm::mix(
//        glm::vec3(a.x, a.y, a.z),
//        glm::vec3(b.x, b.y, b.z),
//        factor
//    );
//}
//
//glm::quat InterpolateRotation(float time, const aiNodeAnim* channel)
//{
//    if (channel->mNumRotationKeys == 1)
//    {
//        auto& q = channel->mRotationKeys[0].mValue;
//        return glm::quat(q.w, q.x, q.y, q.z);
//    }
//
//    int index = FindRotationKey(time, channel);
//    int next = index + 1;
//
//    float delta =
//        (float)channel->mRotationKeys[next].mTime -
//        (float)channel->mRotationKeys[index].mTime;
//
//    float factor =
//        (float)(time - channel->mRotationKeys[index].mTime) / delta;
//
//    auto& a = channel->mRotationKeys[index].mValue;
//    auto& b = channel->mRotationKeys[next].mValue;
//
//    return glm::normalize(glm::slerp(
//        glm::quat(a.w, a.x, a.y, a.z),
//        glm::quat(b.w, b.x, b.y, b.z),
//        factor
//    ));
//}
//
//glm::vec3 InterpolateScale(float animationTime, const aiNodeAnim* channel)
//{
//    // Only one key no interpolation needed
//    if (channel->mNumScalingKeys == 1)
//    {
//        const aiVector3D& v = channel->mScalingKeys[0].mValue;
//        return glm::vec3(v.x, v.y, v.z);
//    }
//
//    uint32_t index = FindScalingKey(animationTime, channel);
//    uint32_t nextIndex = index + 1;
//
//    float t1 = (float)channel->mScalingKeys[index].mTime;
//    float t2 = (float)channel->mScalingKeys[nextIndex].mTime;
//
//    float factor = (animationTime - t1) / (t2 - t1);
//    factor = glm::clamp(factor, 0.0f, 1.0f);
//
//    const aiVector3D& start = channel->mScalingKeys[index].mValue;
//    const aiVector3D& end = channel->mScalingKeys[nextIndex].mValue;
//
//    return glm::mix(
//        glm::vec3(start.x, start.y, start.z),
//        glm::vec3(end.x, end.y, end.z),
//        factor
//    );
//}
//
//const aiNodeAnim* FindNodeAnim(const aiAnimation* anim,
//    const std::string& name)
//{
//    for (uint32_t i = 0; i < anim->mNumChannels; i++)
//    {
//        const aiNodeAnim* channel = anim->mChannels[i];
//        if (name == channel->mNodeName.C_Str())
//            return channel;
//    }
//    return nullptr;
//}


//// for skinned mesh
//std::unordered_map<std::string, uint32_t> boneMapping; // bone name -> index
//std::vector<glm::mat4> boneOffsetMatrices;// indexed by bone index
//
//
//
//void ReadNodeHierarchy(float animationTime,
//    const aiAnimation* animation,
//    aiNode* node,
//    const glm::mat4& parentTransform)
//{
//    std::string nodeName(node->mName.C_Str());
//
//    // Node's default transform (bind pose)
//    glm::mat4 nodeTransform =
//        aiMat4ToGlm(node->mTransformation);
//
//    const aiNodeAnim* channel = FindNodeAnim(animation, nodeName);
//
//    // If animated, override bind transform
//    if (channel)
//    {
//        glm::vec3 T = InterpolatePosition(animationTime, channel);
//        glm::quat R = InterpolateRotation(animationTime, channel);
//        glm::vec3 S = InterpolateScale(animationTime, channel);
//
//        nodeTransform =
//            glm::translate(glm::mat4(1.0f), T) *
//            glm::mat4_cast(R) *
//            glm::scale(glm::mat4(1.0f), S);
//    }
//
//    glm::mat4 globalTransform = parentTransform * nodeTransform;
//
//    // If this node corresponds to a bone, update final matrix
//    auto it = boneMapping.find(nodeName);
//    if (it != boneMapping.end())
//    {
//        uint32_t boneIndex = it->second;
//
//       // glm::mat4 finalTransformation = globalInverseTransform * globalTransform * boneOffsetMatrices[boneIndex];
//
//    }
//
//    // Recurse
//    for (uint32_t i = 0; i < node->mNumChildren; i++)
//    {
//        ReadNodeHierarchy(animationTime,
//            animation,
//            node->mChildren[i],
//            globalTransform);
//    }
//}
//
//VkResult LoadModel_Animated_PBR(const char* modelPath, VulkanComboData* vulkanComboData, bool index32)
//{
//    VkResult vkResult = VK_SUCCESS;
//    // Similar implementation as LoadModel_Phong but for PBR-specific vertex structure
//    // This function is a placeholder and should be implemented similarly to LoadModel_Phong
//    VkResult ZzCreateVertexAndIndex16Buffer(
//        const float* vertices, VkDeviceSize vertexBufferSize,
//        const uint16_t * indices, VkDeviceSize indexBufferSize,
//        VulkanComboData * vulkanComboData
//    );
//    VkResult ZzCreateVertexAndIndex32Buffer(
//        const float* vertices, VkDeviceSize vertexBufferSize,
//        const uint32_t * indices, VkDeviceSize indexBufferSize,
//        VulkanComboData * vulkanComboData
//    );
//
//    Assimp::Importer importer;
//    const aiScene* scene = importer.ReadFile(modelPath,
//          aiProcess_Triangulate             // ensure all faces are triangles
//        | aiProcess_JoinIdenticalVertices   // remove duplicates (safe, keeps indices consistent)
//        | aiProcess_GenSmoothNormals        // generate normals if missing
//        | aiProcess_CalcTangentSpace        // tangents for normal mapping
//        | aiProcess_LimitBoneWeights        // clamp to 4 bone weights per vertex (required for GPU skinning)
//        | aiProcess_ImproveCacheLocality    // optimize vertex cache locality
//        | aiProcess_SortByPType             // separate points/lines/triangles (we only want triangles)
//		| aiProcess_FlipWindingOrder        // convert to Vulkan's right-handed coordinate system
//    );
//
//    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
//    {
//        std::cerr << "Assimp Error: " << importer.GetErrorString() << std::endl;
//        fprintf(gpFILE, "LoadModel_PBR_Skinned() : Assimp Importer::ReadFile() failed.\n");
//        return VK_ERROR_INITIALIZATION_FAILED;
//    }
//    //std::cout << "Meshes: " << scene->mNumMeshes << std::endl;
//
//    std::vector<VertexData_Skinned> vkVertices;
//
//	// for skinned mesh
//    uint32_t numBones = 0;
//
//    if (index32)
//    {
//        std::vector<uint32_t> vkIndices;
//        // before looping meshes:
//        boneMapping.clear();
//        boneOffsetMatrices.clear();
//        numBones = 0;
//
//        if (scene && scene->mRootNode) {
//            for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
//                aiMesh* mesh = scene->mMeshes[meshIndex];
//
//                // Vertices
//                for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
//                    VertexData_Skinned v{};
//                    v.boneIDs = glm::ivec4(0);
//                    v.boneWeights = glm::vec4(0.0f);
//
//                    // Position
//                    if (mesh->HasPositions()) {
//                        v.pos = glm::vec3(
//                            mesh->mVertices[i].x,
//                            mesh->mVertices[i].y,
//                            mesh->mVertices[i].z
//                        );
//                    }
//
//                    // Normal
//                    if (mesh->HasNormals())
//                    {
//                        v.normal = glm::vec3(
//                            mesh->mNormals[i].x,
//                            mesh->mNormals[i].y,
//                            mesh->mNormals[i].z
//                        );
//                    }
//                    else {
//                        v.normal = glm::vec3(0.0f, 0.0f, 1.0f); // default fallback
//                    }
//
//                    // TexCoords (Assimp supports 8 UV channels; we use channel 0)
//                    if (mesh->HasTextureCoords(0))
//                    {
//                        v.texCoord = glm::vec2(
//                            mesh->mTextureCoords[0][i].x,
//                            mesh->mTextureCoords[0][i].y
//                        );
//                    }
//                    else {
//                        v.texCoord = glm::vec2(0.0f);
//                    }
//
//                    // Vertex Tangents 
//                    if (mesh->HasTangentsAndBitangents())
//                    {
//                        glm::vec3 tangent(
//                            mesh->mTangents[i].x,
//                            mesh->mTangents[i].y,
//                            mesh->mTangents[i].z
//                        );
//
//                        glm::vec3 bitangent(
//                            mesh->mBitangents[i].x,
//                            mesh->mBitangents[i].y,
//                            mesh->mBitangents[i].z
//                        );
//
//                        glm::vec3 normal = glm::normalize(v.normal);
//                        tangent = glm::normalize(tangent);
//
//                        float handedness = (glm::dot(glm::cross(tangent, normal), bitangent) < 0.0f)? -1.0f: 1.0f;
//                        v.tangent = glm::vec4(tangent, handedness);
//                    }
//                    else {
//                        // Fallback: X-axis tangent, right-handed
//                        v.tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
//                    }
//
//                    vkVertices.push_back(v);
//                }
//
//                // Indices (faces are always triangles if aiProcess_Triangulate is used)
//                for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
//                    const aiFace& face = mesh->mFaces[f];
//                    for (unsigned int j = 0; j < face.mNumIndices; j++) {
//                        vkIndices.push_back(face.mIndices[j]);
//                    }
//                }
//
//                //bones
//
//                // inside mesh loop, after creating a temporary vertices[] array for this mesh:
//                if (mesh->mNumBones > 0) {
//                    for (unsigned int i = 0; i < mesh->mNumBones; ++i) {
//                        std::string boneName(mesh->mBones[i]->mName.C_Str());
//                        uint32_t boneIndex = 0;
//                        if (boneMapping.find(boneName) == boneMapping.end()) {
//                            boneIndex = numBones++;
//                            boneMapping[boneName] = boneIndex;
//
//                            glm::mat4 offsetMatrix = aiMat4ToGlm(mesh->mBones[i]->mOffsetMatrix);
//                            boneOffsetMatrices.push_back(offsetMatrix);
//                        }
//                        else {
//                            boneIndex = boneMapping[boneName];
//                        }
//
//                        // assign weights to the affected vertices (vertex indices are mesh-local)
//                        for (unsigned int w = 0; w < mesh->mBones[i]->mNumWeights; ++w) {
//                            unsigned int vertexID = mesh->mBones[i]->mWeights[w].mVertexId;
//                            float weight = mesh->mBones[i]->mWeights[w].mWeight;
//                            // make sure the vertex array exists and has the vertexID
//                            //tmpVertices[vertexID].AddBoneData(boneIndex, weight);
//
//                            AddBoneDataToVertex(vkVertices[vertexID], boneIndex, weight);
//
//
//                        }
//                    }
//                }
//
//                assert(boneOffsetMatrices.size() == numBones);
//            }
//
//
//        }
//
//        vulkanComboData->indicesCount = static_cast<uint32_t>(vkIndices.size());
//
//        //PBR model
//        vkResult = ZzCreateVertexAndIndex32Buffer(
//            (float*)vkVertices.data(), vkVertices.size() * sizeof(VertexData_Skinned),
//            vkIndices.data(), vkIndices.size() * sizeof(uint32_t),
//            vulkanComboData
//        );
//        if (vkResult != VK_SUCCESS)
//        {
//            fprintf(gpFILE, "initialize() : ZzCreateVetexAndIndexBuffer() for Animated PBR model failed (%d).\n", vkResult);
//            return(vkResult);
//        }
//    }
//    else
//    {
//        std::vector<uint16_t> vkIndices;
//
//        if (scene && scene->mRootNode) {
//            for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
//                aiMesh* mesh = scene->mMeshes[meshIndex];
//
//                // Vertices
//                for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
//                    VertexData_Skinned v{};
//
//                    // Position
//                    if (mesh->HasPositions()) {
//                        v.pos = glm::vec3(
//                            mesh->mVertices[i].x,
//                            mesh->mVertices[i].y,
//                            mesh->mVertices[i].z
//                        );
//                    }
//
//                    // Normal
//                    if (mesh->HasNormals())
//                    {
//                        v.normal = glm::vec3(
//                            mesh->mNormals[i].x,
//                            mesh->mNormals[i].y,
//                            mesh->mNormals[i].z
//                        );
//                    }
//                    else {
//                        v.normal = glm::vec3(0.0f, 0.0f, 1.0f); // default fallback
//                    }
//
//                    // TexCoords (Assimp supports 8 UV channels; we use channel 0)
//                    if (mesh->HasTextureCoords(0))
//                    {
//                        v.texCoord = glm::vec2(
//                            mesh->mTextureCoords[0][i].x,
//                            mesh->mTextureCoords[0][i].y
//                        );
//                    }
//                    else {
//                        v.texCoord = glm::vec2(0.0f);
//                    }
//
//                    // Vertex Tangents 
//                    if (mesh->HasTangentsAndBitangents())
//                    {
//                        glm::vec3 tangent(
//                            mesh->mTangents[i].x,
//                            mesh->mTangents[i].y,
//                            mesh->mTangents[i].z
//                        );
//
//                        glm::vec3 bitangent(
//                            mesh->mBitangents[i].x,
//                            mesh->mBitangents[i].y,
//                            mesh->mBitangents[i].z
//                        );
//
//                        glm::vec3 normal = glm::normalize(v.normal);
//                        tangent = glm::normalize(tangent);
//
//                        float handedness = (glm::dot(glm::cross(tangent, normal), bitangent) < 0.0f) ? -1.0f : 1.0f;
//                        v.tangent = glm::vec4(tangent, handedness);
//                    }
//                    else{
//                        // Fallback: X-axis tangent, right-handed
//                        v.tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
//                    }
//
//                    vkVertices.push_back(v);
//                }
//
//                // Indices (faces are always triangles if aiProcess_Triangulate is used)
//                for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
//                    const aiFace& face = mesh->mFaces[f];
//                    for (unsigned int j = 0; j < face.mNumIndices; j++) {
//                        vkIndices.push_back(face.mIndices[j]);
//                    }
//                }
//            }
//        }
//
//        vulkanComboData->indicesCount = static_cast<uint32_t>(vkIndices.size());
//
//        //Suzanne
//        vkResult = ZzCreateVertexAndIndex16Buffer(
//            (float*)vkVertices.data(), vkVertices.size() * sizeof(VertexData_Skinned),
//            vkIndices.data(), vkIndices.size() * sizeof(uint16_t),
//            vulkanComboData
//        );
//        if (vkResult != VK_SUCCESS)
//        {
//            fprintf(gpFILE, "initialize() : ZzCreateVetexAndIndexBuffer() for Animated PBR model failed (%d).\n", vkResult);
//            return(vkResult);
//        }
//    }
//
//    return VK_SUCCESS;
//}

static VkResult initialLayoutTransitions(void)
{
    void transitionDepthLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

    VkResult vkResult = VK_SUCCESS;

    //-----------------------------------------------------------------------------------------------------------------------

// Transition the depth image layout to be optimal for depth attachment
    // Transition the image layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo commandBufferAllocateInfo;
    memset(&commandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = vkCommandPool; // Command pool for allocation
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Primary command buffer
    commandBufferAllocateInfo.commandBufferCount = 1; // Allocate one command buffer
    vkResult = vkAllocateCommandBuffers(vkDevice, &commandBufferAllocateInfo, &commandBuffer);
    if (vkResult != VK_SUCCESS)
    {
        return vkResult;
    }
    // Begin command buffer recording
    VkCommandBufferBeginInfo commandBufferBeginInfo;
    memset(&commandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // One-time use command buffer
    vkResult = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
    if (vkResult != VK_SUCCESS)
    {
        return vkResult;
    }

    transitionDepthLayout(
        commandBuffer,
        gSwapChainResourceData.imageData_depthBuffer[0].vkImage,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
    );
    transitionDepthLayout(
        commandBuffer,
        gSwapChainResourceData.imageData_depthBuffer[1].vkImage,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
    );

    // End command buffer recording
    vkResult = vkEndCommandBuffer(commandBuffer);
    if (vkResult != VK_SUCCESS)
    {
        return vkResult;
    }
    // Submit the command buffer and wait for it to finish
    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(VkSubmitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkResult = vkQueueSubmit(vkQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (vkResult != VK_SUCCESS)
    {
        return vkResult;
    }
    vkQueueWaitIdle(vkQueue);
    // Free the command buffer
    vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &commandBuffer);

    return vkResult;
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
    VkResult createSwapchainResources(void);
    VkResult createCommandPool(void);
    VkResult createCommandBuffers(void);
    VkResult ZzCreateVertexBuffer(const float* vertices, VkDeviceSize vertexBufferSize, VulkanData * vulkanData);
    VkResult ZzCreateVertexAndIndex16Buffer(
        const float* vertices, VkDeviceSize vertexBufferSize,
        const uint16_t * indices, VkDeviceSize indexBufferSize,
        VulkanComboData * vulkanComboData
    );

    VkResult LoadModel_Phong(const char* modelPath, VulkanComboData * vulkanComboData, bool index32);
    VkResult LoadModel_PBR(const char* modelPath, VulkanComboData * vulkanComboData, bool index32);
    VkResult LoadModel_Animated_PBR(const char* modelPath, VulkanComboData * vulkanComboData, bool index32);
	VkResult createGlobalTextureDescriptorArray(void);

    //VkResult createVertexBuffer_uvQuad(void);
    VkResult createUniformBuffer(void);
    VkResult createShaders(void);

    VkResult createDescriptorSetLayouts(void);

    VkResult createDescriptorPool(void);
    VkResult createDescriptorSet_FrameData(void);
    VkResult createDescriptorSet_FrameDataBoneData(void);
    VkResult createDescriptorSet_SingleImage(void);
    VkResult createDescriptorSet_AlbedoNormal(void);

    VkResult createSamplers(void);

    //VkResult createRenderPass(void);
    VkResult createGraphicsPipelines(void);
    VkResult createDepthResources(void);
    //VkResult createFramebuffers(void);
    VkResult createSemaphores(void);
    //VkResult createFences(void);

    VkResult loadTextureData(ImageData * imageData, const char* filePath, VkFormat format);
    VkResult loadTextureData_dds_c_bc7(ImageData * imageData, const char* filename, VkFormat format);

    //allocate memory for MAX_FRAME arrays
    {
        //gSwapChainResourceData.imageData_depthBuffer = (ImageData*)malloc(sizeof(ImageData) * MAX_FRAMES);
    }

    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    compileShaderVS_FS("Impostor");
    compileShaderVS_FS("Phong");
    compileShaderVS_FS("PBR");
    compileShaderVS_FS("PBR_Skinned");

    // code
    // STEP 3 : Create Vulkan instance
    vkResult = createVulkanInstance();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createVulkanInstance() failed (%d).\n", vkResult);
        return(vkResult);
    }

    // STEP 4 : Create Vulkan Presentation Surface
    vkResult = getSupportedSurface();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : getSupportedSurface() failed (%d).\n", vkResult);
        return(vkResult);
    }

    // STEP 5 : Enumerate and select required physical device and its queue family index
    vkResult = getPhysicalDevice();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : getPhysicalDevice() failed (%d).\n", vkResult);
        return(vkResult);
    }

    // STEP 6 : Print Vulkan information
    vkResult = printVkInfo();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : printVkInfo() failed (%d).\n", vkResult);
        return(vkResult);
    }

    // STEP 8 : Create Vulkan (Logical) Device
    vkResult = createVulkanDevice();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createVulkanDevice() failed (%d).\n", vkResult);
        return(vkResult);
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

    vkResult = createSwapchainResources();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createSwapchainResources() failed (%d).\n", vkResult);
        return(vkResult);
    }

    // STEP 14 : Create command pool
    vkResult = createCommandPool();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createCommandPool() failed (%d).\n", vkResult);
        return(vkResult);
    }
    //-----------------------------------------------------------------------------------------------
    //samplers
    vkResult = createSamplers();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createSamplers() failed (%d).\n", vkResult);
        return(vkResult);
    }

    vkResult = initialLayoutTransitions();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : initialLayoutTransitions() failed (%d).\n", vkResult);
        return(vkResult);
	}

    //VK_FORMAT_R8G8B8A8_UNORM, // RGBA format
    //VK_FORMAT_R8G8B8A8_SRGB,  // SRGB format
    // 
    //load all textures
    vkResult = loadTextureData(&grassTextureData, "Resources/StockTextures/lion.png", VK_FORMAT_R8G8B8A8_SRGB);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : loadTextureData() failed (%d).\n", vkResult);
        return(vkResult);
    }

    vkResult = loadTextureData(&borderTextureData, "Resources/StockTextures/border.png", VK_FORMAT_R8G8B8A8_SRGB);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : loadTextureData() failed (%d).\n", vkResult);
        return(vkResult);
    }

    //vkResult = loadTextureData(&imposterTextureData, "Resources/StockTextures/test_impo.png", VK_FORMAT_R8G8B8A8_SRGB);
    ////vkResult = loadTextureData(&imposterTextureData, "Resources/StockTextures/count_big_2048.png");
    //if (vkResult != VK_SUCCESS)
    //{
    //	fprintf(gpFILE, "initialize() : loadTextureData() imposterTextureData failed (%d).\n", vkResult);
    //	return(vkResult);
    //}

    //vkResult = loadTextureData(&imposterTextureData_blackAlder_Field_02_Albedo, "Resources/Impostors/BlackAlder_Field_02/T_Impostor_BaseColor_Field_02_Summer.tga", VK_FORMAT_R8G8B8A8_UNORM);
    //if (vkResult != VK_SUCCESS)
    //{
    //	fprintf(gpFILE, "initialize() : loadTextureData() imposterTextureData_blackAlder_Field_02_Albedo failed (%d).\n", vkResult);
    //	return(vkResult);
    //}

    //vkResult = loadTextureData(&imposterTextureData_blackAlder_Field_02_Normal, "Resources/Impostors/BlackAlder_Field_02/T_Impostor_Normal_Field_02_Summer.tga" , VK_FORMAT_R8G8B8A8_UNORM);
 //   if (vkResult != VK_SUCCESS)
 //   {
 //       fprintf(gpFILE, "initialize() : loadTextureData() imposterTextureData_blackAlder_Field_02_Normal failed (%d).\n", vkResult);
 //       return(vkResult);
 //   }

    //dds
    //albedo
    vkResult = loadTextureData_dds_c_bc7(&imposterTextureData_blackAlder_Field_02_Albedo, "Resources/Impostors/BlackAlder_Field_02/T_Impostor_BaseColor_Field_02_Summer.dds", VK_FORMAT_BC7_SRGB_BLOCK);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : loadTextureData_dds() imposterTextureData_blackAlder_Field_02_Normal failed (%d).\n", vkResult);
        return(vkResult);
    }

    //normal
    vkResult = loadTextureData_dds_c_bc7(&imposterTextureData_blackAlder_Field_02_Normal, "Resources/Impostors/BlackAlder_Field_02/T_Impostor_Normal_Field_02_Summer.dds", VK_FORMAT_BC7_UNORM_BLOCK);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : loadTextureData_dds() imposterTextureData_blackAlder_Field_02_Normal failed (%d).\n", vkResult);
        return(vkResult);
    }

    ////lion
    //vkResult = loadTextureData_dds_c_bc7(&imposterTextureData, "Resources/StockTextures/lion.dds", VK_FORMAT_BC7_SRGB_BLOCK);
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "initialize() : loadTextureData_dds() imposterTextureData_ failed (%d).\n", vkResult);
    //    return(vkResult);
    //}




    // STEP 15 : Create command buffers
    vkResult = createCommandBuffers();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createCommandBuffers() failed (%d).\n", vkResult);
        return(vkResult);
    }

    //create vertex buffer for colored triangle
    // 3 vertices, each with 3D position and RGB color
    const VertexData_PositionColor vertices[] = {
        {{ 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},  // bottom (z=0), red
        {{ -1.0f,  0.0f,-1.0f}, {0.0f, 1.0f, 0.0f}},  // right, green
        {{ 1.0f,  0.0f,-1.0f}, {0.0f, 0.0f, 1.0f}},  // left, blue
    };

    vkResult = ZzCreateVertexBuffer((float*)vertices, sizeof(vertices), &vertexData_coloredTriangle);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : ZzCreateVertexBuffer() failed (%d).\n", vkResult);
        return(vkResult);
    }

    //--------------------------------------------------------------------------------------------------

    //create vertex buffer for axis
    float lineWidth = 0.1f;

    const VertexData_PositionColor AxisVerticesColor[] = {
        {{ 0.0f, lineWidth, lineWidth}, {1.0f, 0.0f, 0.0f}},
        {{ 0.0f,  -lineWidth, -lineWidth}, {1.0f, 0.0f, 0.0f}},  //x-axis
        {{ 10000.0f,  0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},

        {{ 0.0f, 10000.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ -lineWidth,  0.0f, lineWidth}, {0.0f, 1.0f, 0.0f}},  //y-axis
        {{ lineWidth,  0.0f, -lineWidth}, {0.0f, 1.0f, 0.0f}},

        {{ lineWidth, lineWidth, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ -lineWidth,  -lineWidth, 0.0f}, {0.0f, 0.0f, 1.0f}},  //z-axis
        {{ 0.0f,  0.0f, 10000.0f}, {0.0f, 0.0f, 1.0f}},
    };

    vkResult = ZzCreateVertexBuffer((float*)AxisVerticesColor, sizeof(AxisVerticesColor), &vertexData_Axis);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : ZzCreateVertexBuffer() for Axis failed (%d).\n", vkResult);
        return(vkResult);
    }

    //--------------------------------------------------------------------------------------------------

    //create vertex buffer for Impostor
    const VertexData_PositionTexCoord ImpostorPosUV[] = {
        { {  0.244862f, -0.592991f, 8.643809f }, { 0.002604f, 0.000521f } },
        { { -0.264767f, -0.083362f, 8.643809f }, { 0.000521f, 0.002604f } },
        { {  0.627084f,  0.298860f, 8.643809f }, { 0.004166f, 0.004166f } },
        { {  1.009306f, -0.592991f, 8.643809f }, { 0.005726f, 0.000521f } },
        { { -0.264767f,  0.935896f, 8.643809f }, { 0.000521f, 0.006767f } },
        { {  1.518936f, -0.083362f, 8.643809f }, { 0.007812f, 0.002604f } },
        { {  0.117455f,  1.318118f, 8.643809f }, { 0.002083f, 0.008331f } },
        { {  1.518936f,  0.935896f, 8.643809f }, { 0.007812f, 0.006767f } },
        { {  1.136714f,  1.318118f, 8.643809f }, { 0.006248f, 0.008331f } } };

    //create index buffer for Impostor
    const uint16_t impostor_indices[] = {
    0, 1, 2,
    3, 0, 2,
    4, 2, 1,
    5, 3, 2,
    4, 6, 2,
    7, 5, 2,
    6, 8, 2,
    8, 7, 2
    };

    //vulkanComboData for Impostor
    vkResult = ZzCreateVertexAndIndex16Buffer(
        (float*)ImpostorPosUV, sizeof(ImpostorPosUV),
        impostor_indices, sizeof(impostor_indices),
        &ImpostorBufferData
    );
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : ZzCreateVetexAndIndexBuffer() failed (%d).\n", vkResult);
        return(vkResult);
    }

    //vulkanComboData for cube

// 24 unique vertices (each cube face has 4 verts with its own normal/uv)
    static const VertexData_PositionTexCoordNormalColor cubeVertices[] = {
        // Front face (Z+)
        {{-1, -1,  1}, {0, 0}, {0, 0, 1}, {1, 0, 0}}, // bottom-left
        {{ 1, -1,  1}, {1, 0}, {0, 0, 1}, {1, 0, 0}}, // bottom-right
        {{ 1,  1,  1}, {1, 1}, {0, 0, 1}, {1, 0, 0}}, // top-right
        {{-1,  1,  1}, {0, 1}, {0, 0, 1}, {1, 0, 0}}, // top-left

        // Back face (Z-)
        {{ 1, -1, -1}, {0, 0}, {0, 0,-1}, {0, 1, 0}},
        {{-1, -1, -1}, {1, 0}, {0, 0,-1}, {0, 1, 0}},
        {{-1,  1, -1}, {1, 1}, {0, 0,-1}, {0, 1, 0}},
        {{ 1,  1, -1}, {0, 1}, {0, 0,-1}, {0, 1, 0}},

        // Left face (X-)
        {{-1, -1, -1}, {0, 0}, {-1, 0, 0}, {0, 0, 1}},
        {{-1, -1,  1}, {1, 0}, {-1, 0, 0}, {0, 0, 1}},
        {{-1,  1,  1}, {1, 1}, {-1, 0, 0}, {0, 0, 1}},
        {{-1,  1, -1}, {0, 1}, {-1, 0, 0}, {0, 0, 1}},

        // Right face (X+)
        {{ 1, -1,  1}, {0, 0}, {1, 0, 0}, {1, 1, 0}},
        {{ 1, -1, -1}, {1, 0}, {1, 0, 0}, {1, 1, 0}},
        {{ 1,  1, -1}, {1, 1}, {1, 0, 0}, {1, 1, 0}},
        {{ 1,  1,  1}, {0, 1}, {1, 0, 0}, {1, 1, 0}},

        // Top face (Y+)
        {{-1,  1,  1}, {0, 0}, {0, 1, 0}, {0, 1, 1}},
        {{ 1,  1,  1}, {1, 0}, {0, 1, 0}, {0, 1, 1}},
        {{ 1,  1, -1}, {1, 1}, {0, 1, 0}, {0, 1, 1}},
        {{-1,  1, -1}, {0, 1}, {0, 1, 0}, {0, 1, 1}},

        // Bottom face (Y-)
        {{-1, -1, -1}, {0, 0}, {0,-1, 0}, {1, 0, 1}},
        {{ 1, -1, -1}, {1, 0}, {0,-1, 0}, {1, 0, 1}},
        {{ 1, -1,  1}, {1, 1}, {0,-1, 0}, {1, 0, 1}},
        {{-1, -1,  1}, {0, 1}, {0,-1, 0}, {1, 0, 1}},
    };

    // Indices (36 = 6 faces × 2 triangles × 3 verts)
    static const uint16_t cubeIndices[] = {
        0,  2,  1,   2,  0,  3,  // Front
        4,  6,  5,   6,  4,  7,  // Back
        8, 10,  9,  10,  8, 11,  // Left
       12, 14, 13,  14, 12, 15,  // Right
       16, 18, 17,  18, 16, 19,  // Top
       20, 22, 21,  22, 20, 23   // Bottom
    };

    //vulkanComboData for cube
    vkResult = ZzCreateVertexAndIndex16Buffer(
        (float*)cubeVertices, sizeof(cubeVertices),
        cubeIndices, sizeof(cubeIndices),
        &CubeBufferData
    );
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : ZzCreateVetexAndIndexBuffer() for cube failed (%d).\n", vkResult);
        return(vkResult);
    }

    //------------------------------------------------------------------------------------------------------------------

        //create uniform buffer
    vkResult = createUniformBuffer();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createUniformBuffer() failed (%d).\n", vkResult);
        return(vkResult);
    }

    // STEP  : Create shaders
    vkResult = createShaders();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createShaders() failed (%d).\n", vkResult);
        return(vkResult);
    }


    //DescriptorSetLayouts
    gpDescriptorSetLayouts = new DescriptorSetLayouts();
    vkResult = gpDescriptorSetLayouts->vkResult;
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : pDescriptorSetLayouts->init() failed (%d).\n", vkResult);
        return(vkResult);
    }

    //Create Descriptor Pool
    vkResult = createDescriptorPool();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createDiscriptorPool() failed (%d).\n", vkResult);
        return(vkResult);
    }

    //Create Descriptor Set
    vkResult = createDescriptorSet_FrameData();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createDescriptorSet_FrameData() failed (%d).\n", vkResult);
        return(vkResult);
    }

    //Create Descriptor Set for Image
    vkResult = createDescriptorSet_SingleImage();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createDescriptorSet_SingleImage() failed (%d).\n", vkResult);
        return(vkResult);
    }
    //Create Descriptor Set for Albedo Normal
    vkResult = createDescriptorSet_AlbedoNormal();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createDescriptorSet_AlbedoNormal() failed (%d).\n", vkResult);
        return(vkResult);
    }

    //-----------------------------------------------------------------------------------------------

    //// STEP 16 : Create RenderPass
    //vkResult = createRenderPass();
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "initialize() : createRenderPass() failed (%d).\n", vkResult);
    //    return(vkResult);
    //}

    //pipeline
    vkResult = createGraphicsPipelines();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createGraphicsPipelines() failed (%d).\n", vkResult);
        return(vkResult);
    }

    ////  Create Framebuffers
    //vkResult = createFramebuffers();
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "initialize() : createFramebuffers() failed (%d).\n", vkResult);
    //    return(vkResult);
    //}

    // STEP 18 : Create semaphores and fences
    vkResult = createSemaphores();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createSemaphores() failed (%d).\n", vkResult);
        return(vkResult);
    }

    //vkResult = createFences();
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "initialize() : createFences() failed (%d).\n", vkResult);
    //    return(vkResult);
    //}

    // initialize clear color values
    memset((void*)&vkClearColorValue, 0, sizeof(VkClearColorValue));

    // this is analogous to glClearColor() or DirectX's clearColor[] array
    vkClearColorValue.float32[0] = 0.05f;
    vkClearColorValue.float32[1] = 0.05f;
    vkClearColorValue.float32[2] = 0.05f;
    vkClearColorValue.float32[3] = 1.0f;

#ifdef IMGUI_ENABLE
    //IMGUI Init

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    //ImGuiIO& io = ImGui::GetIO();
    g_io = &ImGui::GetIO();
    g_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // optional
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // optional
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // optional

    float dpiScale = 3.0f; // Adjust based on DPI (e.g. 2.0 for 4K on 1080p UI size)

    // Scale all UI elements globally
    g_io->FontGlobalScale = dpiScale;

    ImGui::StyleColorsDark();  // or Light, Classic, etc.

    //---------------------
    ImGui_ImplWin32_Init(ghwnd);  // HWND from CreateWindow or similar
    //---------------------
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER,                1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000 }
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes); // sufficient descriptor sets
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    vkCreateDescriptorPool(vkDevice, &pool_info, nullptr, &gImguiDescriptorPool);
    //--------------------
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vkInstance;
    init_info.PhysicalDevice = vkPhysicalDevice_Selected;
    init_info.Device = vkDevice;
    init_info.QueueFamily = graphicsQueueFamilyIndex_Selected;
    init_info.Queue = vkQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = gImguiDescriptorPool;
	init_info.RenderPass = VK_NULL_HANDLE;
    init_info.Subpass = 0;
    init_info.MinImageCount = 2;    // e.g. 2 or 3
    init_info.ImageCount = gSwapchainImageCount; // Match your swapchain image count
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = check_vk_result;

    VkPipelineRenderingCreateInfo pipelineRenderingInfo{};
    pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipelineRenderingInfo.colorAttachmentCount = 1;

	VkFormat swapchainImageFormat = VK_FORMAT_B8G8R8A8_SRGB; // Match your swapchain image format
	pipelineRenderingInfo.pColorAttachmentFormats = &swapchainImageFormat; // Match your swapchain image format
    pipelineRenderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
    pipelineRenderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

	init_info.UseDynamicRendering = VK_TRUE; // Enable dynamic rendering support
	init_info.PipelineRenderingCreateInfo = pipelineRenderingInfo;
  

    ImGui_ImplVulkan_Init(&init_info);

    //-----------------------------
    // Immediately after ImGui_ImplVulkan_Init() and after creating framebuffers:
    ImGui_ImplVulkan_CreateFontsTexture(); // new no-arg call (uploads fonts intern

#endif // IMGUI_ENABLE

    //-------------------------------------Resources---------------------------------------

    //--------------------------------------Assimp Load Model----------------------------------------------------
    vkResult = LoadModel_Phong("Resources/Models/Test/suzanne.obj", &SuzanneBufferData, false);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : LoadModel_Phong() failed (%d).\n", vkResult);
        return(vkResult);
    }

    vkResult = LoadModel_PBR("Resources/Models/Test/sphere.obj", &SphereBufferData, false);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : sphere LoadModel_PBR()  failed (%d).\n", vkResult);
        return(vkResult);
    }

    //Rat
    //vkResult = LoadModel_Animated_PBR("Resources/Models/Test/Anim_Rat_Walk.FBX", &RatBufferData, true);
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "initialize() : Rat LoadModel_Animated_PBR()  failed (%d).\n", vkResult);
    //    return(vkResult);
    //}

    pModel_Rat = new AnimatedModel("Resources/Models/Test/RatWithMat.FBX", true, gpDescriptorSetLayouts->vkDescriptorSetLayout_BasicPBR);

    //createDescriptorSet_FrameDataBoneData
	vkResult = createDescriptorSet_FrameDataBoneData();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "initialize() : createDescriptorSet_FrameDataBoneData() failed (%d).\n", vkResult);
		return(vkResult);
	}


    //-------------------------------------PBR model textures----------------------------------------------------------

    const char* pathRockyGround = "Resources/PBR_Materials/T_omfr20_4K/";
    pMaterial_BasicPBR_RockyGround = new Material_BasicPBR(gpDescriptorSetLayouts->vkDescriptorSetLayout_BasicPBR, pathRockyGround);
    vkResult = pMaterial_BasicPBR_RockyGround->getVkResult();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : pMaterial_BasicPBR_RockyGround failed to load (%d).\n", vkResult);
        return(vkResult);
    }

    const char* pathGrassyGround = "Resources/PBR_Materials/T_sbykqdp0_4K/";
    pMaterial_BasicPBR_GrassyGround = new Material_BasicPBR(gpDescriptorSetLayouts->vkDescriptorSetLayout_BasicPBR, pathGrassyGround);
    vkResult = pMaterial_BasicPBR_GrassyGround->getVkResult();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : pMaterial_BasicPBR_GrassyGround failed to load (%d).\n", vkResult);
        return(vkResult);
    }


    //--------------------------------------------------------------------------------------------------


    MyWin32::gProjectionMatrix = glm::perspectiveLH_ZO(
        glm::radians(MyWin32::fovY),
        (float)WIN_WIDTH / (float)WIN_HEIGHT,
        MyWin32::gNearFarFrustum.x,
        MyWin32::gNearFarFrustum.y
    );
    MyWin32::gProjectionMatrix[1][1] *= -1.0f; // flip the Y axis for Vulkan

    //---------------------------------GlobalTextureDescriptorArray-----------------------------------

	vkResult = createGlobalTextureDescriptorArray();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createGlobalTextureDescriptorArray() failed (%d).\n", vkResult);
        return(vkResult);
	}

    //-------------------------------------------------------------------------------------


    

    bInitialized = TRUE;
    fprintf(gpFILE, "initialize() : initialize complete.\n");

    //---------------------------------Test-----------------------------------


    //-----------------------------------------------------------------------------


    return(vkResult);
}


// Helper: find a memory type index with required properties
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_Selected, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    fprintf(gpFILE, "Failed to find suitable memory type.\n");
    exit(EXIT_FAILURE);
}

VkResult resize(int width, int height)
{
    // function declarations
    VkResult createSwapchain(VkBool32);
    VkResult createCommandBuffers(void);
    VkResult createPipelineLayout(void);
    VkResult createGraphicsPipelines(void);
    //VkResult createRenderPass(void);
    VkResult createSwapchainResources(void);
    //VkResult createFramebuffers(void);

    void destroySwapchainResources(void);
    void destroyGraphicsPipelines(void);



    // variable declarations
    VkResult vkResult = VK_SUCCESS;
    // code
    // reset the height to 1 to avoid a division by 0
    if (height <= 0)
    {
        height = 1;
    }

    // if control comes here before initialization is completed, return false
    if (bInitialized == FALSE)
    {
        fprintf(gpFILE, "resize() : initialization yet not completed.\n");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    bInitialized = FALSE; // set to false to avoid multiple calls to resize() before initialization is completed

    // set the global winWidth and winHeight variables
    winWidth = width;
    winHeight = height;

    //--------------------------------------------------------------------------------------
    if (vkDevice)
        vkDeviceWaitIdle(vkDevice); // wait for the device to finish all operations before resizing

    //destroy old swapchain
    if (vkSwapchainKHR == VK_NULL_HANDLE)
    {
        fprintf(gpFILE, "resize() : vkSwapchainKHR is already NULL canot proceed.\n");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    ////destroy framebuffers
    //if (vkFramebuffer_Array)
    //{
    //    for (uint32_t i = 0; i < gSwapchainImageCount; i++)
    //    {
    //        vkDestroyFramebuffer(vkDevice, vkFramebuffer_Array[i], NULL);
    //        vkFramebuffer_Array[i] = VK_NULL_HANDLE;

    //        //fprintf(gpFILE, "resize() : vkDestroyFramebuffer() succeeded for iteration %d.\n", i);
    //    }

    //    free(vkFramebuffer_Array);
    //    vkFramebuffer_Array = NULL;

    //    //fprintf(gpFILE, "resize() : successfully freed the memory allocated to vkFramebuffer_Array.\n");
    //}

    // vkCommandBuffer
    for (uint32_t i = 0; i < gSwapchainImageCount; i++)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_Array[i]);
        // fprintf(gpFILE, "resize() : vkFreeCommandBuffers() succeeded for iteration %d.\n", i);
    }
    if (vkCommandBuffer_Array)
    {
        free(vkCommandBuffer_Array);
        vkCommandBuffer_Array = NULL;

        // fprintf(gpFILE, "resize() : successfully freed memory for the command buffer array.\n");
    }

    // pipeline
    destroyGraphicsPipelines();

    //// renderpass
    //if (vkRenderPass)
    //{
    //    vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
    //    vkRenderPass = VK_NULL_HANDLE;

    //    //fprintf(gpFILE, "resize() : vkDestroyRenderPass() succeeded.\n");
    //}

    //destroy swapchain resources
    destroySwapchainResources();


    // destroy swapchain
    if (vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
        vkSwapchainKHR = VK_NULL_HANDLE;

        //fprintf(gpFILE, "resize() : vkDestroySwapchainKHR() succeeded.\n");
    }

    //-----------------------------------------------------------------------------
    // re-create swapchain

    vkResult = createSwapchain(VK_TRUE);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "resize() : createSwapchain() failed (%d).\n", vkResult);
        return(vkResult);
    }

    vkResult = createSwapchainResources();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createSwapchainResources() failed (%d).\n", vkResult);
        return(vkResult);
    }

    ////create render pass
    //vkResult = createRenderPass();
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "resize() : createRenderPass() failed (%d).\n", vkResult);
    //    return(vkResult);
    //}

    ////create pipeline layout
    //vkResult = createPipelineLayout();
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "resize() : createPipelineLayout() failed (%d).\n", vkResult);
    //    return(vkResult);
    //}

    //create graphics pipeline
    vkResult = createGraphicsPipelines();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "resize() : createGraphicsPipeline() failed (%d).\n", vkResult);
        return(vkResult);
    }

    ////create framebuffers
    //vkResult = createFramebuffers();
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "resize() : createFramebuffers() failed (%d).\n", vkResult);
    //    return(vkResult);
    //}

    //create command buffers
    vkResult = createCommandBuffers();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "resize() : createCommandBuffers() failed (%d).\n", vkResult);
        return(vkResult);
    }

    //initial layout transitions (Depth for now)
    vkResult = initialLayoutTransitions();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : initialLayoutTransitions() failed (%d).\n", vkResult);
        return(vkResult);
    }

    ////build command buffers
    //vkResult = buildCommandBuffers(0); // passing 0 as curIndex since we are not using fences here
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "resize() : buildCommandBuffers() failed (%d).\n", vkResult);
    //    return(vkResult);
    //}
    //vkResult = buildCommandBuffers(1);// passing 1 as curIndex since we are not using fences here
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "resize() : buildCommandBuffers() failed (%d).\n", vkResult);
    //    return(vkResult);
    //}

    MyWin32::gProjectionMatrix = glm::perspectiveLH_ZO(
        glm::radians(MyWin32::fovY),
        (float)width / (float)height,
        MyWin32::gNearFarFrustum.x,
        MyWin32::gNearFarFrustum.y
    );
    MyWin32::gProjectionMatrix[1][1] *= -1.0f; // flip the Y axis for Vulkan


    bInitialized = TRUE; // set to true to allow display() to work

    return vkResult;
}

/*
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
*/

VkResult display(void)
{
    VkResult buildCommandBuffers(uint32_t curIndex, uint32_t currentImageIndex);
    //VkResult updateUniformBuffer_camera(uint32_t curIndex);
    VkResult updateUniformBuffer_frameData(uint32_t curIndex);

    VkResult vkResult = VK_SUCCESS;

    if (bInitialized == FALSE)
    {
        fprintf(gpFILE, "display() : initialization yet not completed.\n");
        return (VkResult)VK_FALSE;
    }

    // Use per-frame index
    static uint32_t currentFrame = 0;
    uint32_t curIndex = currentFrame % MAX_FRAMES;

    // Wait for GPU to finish work on the previous frame
    //vkResult = vkWaitForFences(vkDevice, 1, &vkFence_Array[curIndex], VK_TRUE, UINT64_MAX);
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "display() : vkWaitForFences() failed (%d).\n", vkResult);
    //    return vkResult;
    //}



    uint64_t waitValue = gFrameTimelineValues[curIndex];

    if (waitValue != 0)
    {
        VkSemaphoreWaitInfo waitInfo{};
        waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        waitInfo.semaphoreCount = 1;
        waitInfo.pSemaphores = &vkSemaphore_Timeline;
        waitInfo.pValues = &waitValue;

        vkWaitSemaphores(vkDevice, &waitInfo, UINT64_MAX);
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


    //// Reset the fence for use in the current frame
    //vkResult = vkResetFences(vkDevice, 1, &vkFence_Array[curIndex]);
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "display() : vkResetFences() failed (%d).\n", vkResult);
    //    return vkResult;
    //}



    //--------------------------------------------------------------------------------------
        //IMGUI dynamic
    {
        vkResult = buildCommandBuffers(curIndex,currentImageIndex);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "display() : buildCommandBuffers() failed (%d).\n", vkResult);
            return(vkResult);
        }
    }

    uint64_t signalValue = ++gTimelineValue;
    gFrameTimelineValues[curIndex] = signalValue;


    uint64_t signalValues[] =
    {
        0,              // ignored for binary semaphore
        signalValue     // timeline semaphore value
    };

    VkSemaphore waitSemaphores[] =
    {
        vkSemaphore_BackBuffer[curIndex]
    };

    VkPipelineStageFlags waitStages[] =
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkTimelineSemaphoreSubmitInfo timelineInfo{};
    timelineInfo.sType =
        VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
    timelineInfo.signalSemaphoreValueCount = 2;
    timelineInfo.pSignalSemaphoreValues = signalValues;

    VkSemaphore signalSemaphores[] =
    {
        vkSemaphore_RenderComplete[curIndex],
        vkSemaphore_Timeline
    };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = &timelineInfo;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vkCommandBuffer_Array[curIndex];
    submitInfo.signalSemaphoreCount = 2;
    submitInfo.pSignalSemaphores = signalSemaphores;


	vkResult = vkQueueSubmit(vkQueue, 1, &submitInfo, VK_NULL_HANDLE);
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

    //vkResult = updateUniformBuffer_camera(curIndex);
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "display() : updateUniformBuffer_camera() failed (%d).\n", vkResult);
    //    return vkResult;
    //}

    vkResult = updateUniformBuffer_frameData(curIndex);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "display() : updateUniformBuffer_frameData() failed (%d).\n", vkResult);
        return vkResult;
    }

    // Advance to next frame
    currentFrame++;

    return vkResult;
}

void update(void)
{
    // code
	MyWin32::dTotalElapsedTime += MyWin32::fDeltaTime * 100.0f;


    if (FALSE == MyWin32::isGUI)
    {
        camera.UpdateViewMatrix(ghwnd);
    }

}

void uninitialize(void)
{
    // function declarations
    void ToggleFullscreen(void);
    //void destroyGraphicsPipelines(void);
    void destroySwapchainResources(void);
    void destroySamplers(void);
    void destroyShaders(void);

    void destroyTextureData(ImageData * imageData);
    void ZzDestroyVertexBuffer(VulkanData * vulkanData);
    void ZzDestroyIndexBuffer(VulkanData * vulkanData);
    void ZzDestroyVertexAndIndexBuffer(VulkanComboData * vulkanComboData);

    VkResult destroyGlobalTextureDescriptorArray(void);

    // code
    // if application is exitting in fullscreen
    if (gbFullscreen == TRUE)
    {
        ToggleFullscreen();
        gbFullscreen = FALSE;
    }

    // Before destroying the device (and any other vulkan related destruction), ensure that all operations on that device are finished. Till then, wait on that device.
    if (vkDevice)
    {
        vkDeviceWaitIdle(vkDevice);
    }

#ifdef IMGUI_ENABLE
    //IMGUI uninitialization
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        vkDestroyDescriptorPool(vkDevice, gImguiDescriptorPool, nullptr);
        if (gpFILE)
            fprintf(gpFILE, "uninitialize() : ImGui uninitialization is done.\n");
    }
#endif

    //// Fences
    //if (vkFence_Array)
    //{
    //    for (uint32_t i = 0; i < gSwapchainImageCount; i++)
    //    {
    //        vkDestroyFence(vkDevice, vkFence_Array[i], NULL);
    //        vkFence_Array[i] = VK_NULL_HANDLE;

    //    }

    //    free(vkFence_Array);
    //    vkFence_Array = NULL;

    //}

	//Semaphores

    for (size_t i = 0; i < gSwapchainImageCount; i++)
    {
        if (vkSemaphore_RenderComplete[i])
        {
            vkDestroySemaphore(vkDevice, vkSemaphore_RenderComplete[i], NULL);
            vkSemaphore_RenderComplete[i] = VK_NULL_HANDLE;

        }

        if (vkSemaphore_BackBuffer[i])
        {
            vkDestroySemaphore(vkDevice, vkSemaphore_BackBuffer[i], NULL);
            vkSemaphore_BackBuffer[i] = VK_NULL_HANDLE;

        }
    }

    free(vkSemaphore_BackBuffer);
    vkSemaphore_BackBuffer = NULL;

    free(vkSemaphore_RenderComplete);
    vkSemaphore_RenderComplete = NULL;


	//timeline semaphore
    if (vkSemaphore_Timeline)
    {
        vkDestroySemaphore(vkDevice, vkSemaphore_Timeline, NULL);
        vkSemaphore_Timeline = VK_NULL_HANDLE;
	}



    //// sub-step 5 for Step (17)
    //if (vkFramebuffer_Array)
    //{
    //    for (uint32_t i = 0; i < gSwapchainImageCount; i++)
    //    {
    //        vkDestroyFramebuffer(vkDevice, vkFramebuffer_Array[i], NULL);
    //        vkFramebuffer_Array[i] = VK_NULL_HANDLE;

    //    }

    //    free(vkFramebuffer_Array);
    //    vkFramebuffer_Array = NULL;

    //}

    // pipeline
    //destroyGraphicsPipelines();
    if (gpGraphicsPipelines)
    {
        delete gpGraphicsPipelines;
        gpGraphicsPipelines = NULL;
    }

    //// renderpass
    //if (vkRenderPass)
    //{
    //    vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
    //    vkRenderPass = VK_NULL_HANDLE;

    //}

    //discriptor pool // no need to destroy discriptor set if we destroy discriptor pool
    if (vkDescriptorPool)
    {
        vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, NULL);
        vkDescriptorPool = VK_NULL_HANDLE;
        vkDescriptorSets_frameData[0] = VK_NULL_HANDLE; // set to NULL to avoid dangling pointer
        vkDescriptorSets_frameData[1] = VK_NULL_HANDLE; // set to NULL to avoid dangling pointer

        vkDescriptorSets_frameDataBoneData[0] = VK_NULL_HANDLE; // set to NULL to avoid dangling pointer
        vkDescriptorSets_frameDataBoneData[1] = VK_NULL_HANDLE; // set to NULL to avoid dangling pointer


    }

    //Materials
    if (pMaterial_BasicPBR_RockyGround)
    {
        delete pMaterial_BasicPBR_RockyGround;
        pMaterial_BasicPBR_RockyGround = NULL;
    }

    if (pMaterial_BasicPBR_GrassyGround)
    {
        delete pMaterial_BasicPBR_GrassyGround;
        pMaterial_BasicPBR_GrassyGround = NULL;
    }

    //pipeline Layouts

    //descriptor set layouts
    if (gpDescriptorSetLayouts)
    {
        delete gpDescriptorSetLayouts;
        gpDescriptorSetLayouts = NULL;
    }

    //shaderModules
    //destroyShaders();


    //Destroy vertex data for colored triangle
    ZzDestroyVertexBuffer(&vertexData_coloredTriangle);
    //vertexData_Axis 
    ZzDestroyVertexBuffer(&vertexData_Axis);

    //Impostor CombinedData
    ZzDestroyVertexAndIndexBuffer(&ImpostorBufferData);
    ZzDestroyVertexAndIndexBuffer(&CubeBufferData);
    ZzDestroyVertexAndIndexBuffer(&SuzanneBufferData);
    ZzDestroyVertexAndIndexBuffer(&SphereBufferData);
    //ZzDestroyVertexAndIndexBuffer(&RatBufferData);

	delete pModel_Rat;

    //uniform buffer

    ////Camera uniform buffer
 //   for (uint32_t i = 0; i < MAX_FRAMES; i++)
 //   {
 //       // Unmap the uniformData memory
 //       vkUnmapMemory(vkDevice, uniformBufferData_camera[i].vkDeviceMemory);

 //       //uniformData
 //       if (uniformBufferData_camera[i].vkDeviceMemory)
 //       {
 //           vkFreeMemory(vkDevice, uniformBufferData_camera[i].vkDeviceMemory, NULL);
 //           uniformBufferData_camera[i].vkDeviceMemory = VK_NULL_HANDLE;
 //       }
 //       if (uniformBufferData_camera[i].vkBuffer)
 //       {
 //           vkDestroyBuffer(vkDevice, uniformBufferData_camera[i].vkBuffer, NULL);
 //           uniformBufferData_camera[i].vkBuffer = VK_NULL_HANDLE;
 //       }
 //   }

    //FrameData uniform buffer
    for (uint32_t i = 0; i < MAX_FRAMES; i++)
    {
        // Unmap the uniformData memory
        vkUnmapMemory(vkDevice, uniformBufferData_frameData[i].vkDeviceMemory);

        //uniformData
        if (uniformBufferData_frameData[i].vkDeviceMemory)
        {
            vkFreeMemory(vkDevice, uniformBufferData_frameData[i].vkDeviceMemory, NULL);
            uniformBufferData_frameData[i].vkDeviceMemory = VK_NULL_HANDLE;
        }
        if (uniformBufferData_frameData[i].vkBuffer)
        {
            vkDestroyBuffer(vkDevice, uniformBufferData_frameData[i].vkBuffer, NULL);
            uniformBufferData_frameData[i].vkBuffer = VK_NULL_HANDLE;
        }
    }

    for (uint32_t i = 0; i < gSwapchainImageCount; i++)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_Array[i]);
    }

    if (vkCommandBuffer_Array)
    {
        free(vkCommandBuffer_Array);
        vkCommandBuffer_Array = NULL;

    }

    if (vkCommandPool)
    {
        vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
        vkCommandPool = VK_NULL_HANDLE;

    }

    //-----------------------------------------------------------

    //destroy image data
    destroyTextureData(&grassTextureData);
    destroyTextureData(&borderTextureData);
    destroyTextureData(&imposterTextureData);
    destroyTextureData(&imposterTextureData_blackAlder_Field_02_Albedo);
    destroyTextureData(&imposterTextureData_blackAlder_Field_02_Normal);

    // Destroy swapchain resources
    destroySwapchainResources();

	//global texture descriptor array
	destroyGlobalTextureDescriptorArray();


    // Destroy Vulkan swapchain
    if (vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(
            vkDevice,       // [in] Vulkan device handle,
            vkSwapchainKHR, // [in] Vulkan swapchain handle
            NULL            // [in, optional] custom memory allocator
        );

        vkSwapchainKHR = VK_NULL_HANDLE;

    }
    destroySamplers();

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
    if (vkSurfaceKHR)
    {
        vkDestroySurfaceKHR(
            vkInstance,   // [in] Vulkan instance handle
            vkSurfaceKHR, // [in] Vulkan presentation surface handle
            NULL          // [in, optional] pointer to a custom memory allocator (NULL means use a default memory allocator)
        );

        vkSurfaceKHR = VK_NULL_HANDLE;

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

    // Destroy window
    if (ghwnd)
    {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    if (gpFILE)
    {
        fprintf(gpFILE, LINE_END);
        fprintf(gpFILE, LINE_END);
    }

    // close the log file
    if (gpFILE)
    {
        fprintf(gpFILE, "uninitialize() : Program ended successfully.\n");
        fclose(gpFILE);
        gpFILE = NULL;
    }
}

VkResult loadTextureData(ImageData* imageData, const char* filename, VkFormat format)
{
    VkResult vkResult = VK_SUCCESS;

    //2)  Variables to receive image metadata
    int width, height, channels;

    //flip the image vertically
    stbi_set_flip_vertically_on_load(true); // flip the image vertically

    // 3) stbi_load returns a pointer to the pixel data
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data)
    {
        fprintf(gpFILE, "\nFailed: Error loading '%s': %s\n\n", filename, stbi_failure_reason());
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }

    fprintf(gpFILE, "\nSuccess: Loaded image '%s' ( %d x %d ), %d channels\n\n", filename, width, height, channels);

    // 4) Create a Vulkan image and upload the pixel data
    VkBuffer vkBuffer_stagingBuffer;
    VkDeviceMemory vkDeviceMemory_stagingBuffer;
    VkDeviceSize imageSize = width * height * channels;//channels

    // Buffer create info
    VkBufferCreateInfo bufferCreateInfo;
    memset(&bufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = imageSize; // Size of the buffer in bytes
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // Buffer will be used for transfer operations
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // No sharing between queues
    vkResult = vkCreateBuffer(vkDevice, &bufferCreateInfo, NULL, &vkBuffer_stagingBuffer);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "loadTextureData() : vkCreateBuffer() failed (%d).\n", vkResult);
        stbi_image_free(data); // Free the pixel data before returning
        return vkResult;
    }


    // Allocate memory for the buffer
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(vkDevice, vkBuffer_stagingBuffer, &memoryRequirements);
    VkMemoryAllocateInfo memoryAllocateInfo;
    memset(&memoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size; // Size of the memory to allocate
    memoryAllocateInfo.memoryTypeIndex = findMemoryType(
        memoryRequirements.memoryTypeBits, // Memory type bits from the buffer
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT // Memory properties
    );
    if (memoryAllocateInfo.memoryTypeIndex == UINT32_MAX)
    {
        fprintf(gpFILE, "loadTextureData() : findMemoryType() failed.\n");
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL); // Clean up the buffer
        stbi_image_free(data); // Free the pixel data before returning
        return VK_ERROR_MEMORY_MAP_FAILED;
    }
    vkResult = vkAllocateMemory(vkDevice, &memoryAllocateInfo, NULL, &vkDeviceMemory_stagingBuffer);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "loadTextureData() : vkAllocateMemory() failed (%d).\n", vkResult);
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL); // Clean up the buffer
        stbi_image_free(data); // Free the pixel data before returning
        return vkResult;
    }
    // Bind the buffer memory
    vkResult = vkBindBufferMemory(vkDevice, vkBuffer_stagingBuffer, vkDeviceMemory_stagingBuffer, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "loadTextureData() : vkBindBufferMemory() failed (%d).\n", vkResult);
        vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL); // Clean up the memory
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL); // Clean up the buffer
        stbi_image_free(data); // Free the pixel data before returning
        return vkResult;
    }
    // Map the buffer memory and copy the pixel data
    void* mappedMemory;
    vkResult = vkMapMemory(vkDevice, vkDeviceMemory_stagingBuffer, 0, imageSize, 0, &mappedMemory);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "loadTextureData() : vkMapMemory() failed (%d).\n", vkResult);
        vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL); // Clean up the memory
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL); // Clean up the buffer
        stbi_image_free(data); // Free the pixel data before returning
        return vkResult;
    }
    // Copy the pixel data to the mapped memory
    memcpy(mappedMemory, data, imageSize);
    // Unmap the memory after copying
    vkUnmapMemory(vkDevice, vkDeviceMemory_stagingBuffer);



    //VK_FORMAT_R8G8B8A8_UNORM, // RGBA format
    //VK_FORMAT_R8G8B8A8_SRGB,  // SRGB format

    VkFormat linearFormats[4] = {
        VK_FORMAT_R8_SRGB,        // grayscale
        VK_FORMAT_R8G8_SRGB,      // two channel (RG)
        VK_FORMAT_R8G8B8_SRGB,    // three channel RGB
        VK_FORMAT_R8G8B8A8_SRGB,  // four channel RGBA
    };

    // Create a Vulkan image to hold the texture data
    VkImageCreateInfo imageCreateInfo;
    memset(&imageCreateInfo, 0, sizeof(VkImageCreateInfo));
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D; // 2D image
    imageCreateInfo.format = format; // Assuming RGBA format
    imageCreateInfo.extent.width = width; // Width of the image
    imageCreateInfo.extent.height = height; // Height of the image
    imageCreateInfo.extent.depth = 1; // Depth of the image (1 for 2D)
    imageCreateInfo.mipLevels = 1; // No mipmaps
    imageCreateInfo.arrayLayers = 1; // Single layer
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT; // No multisampling
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL; // Optimal tiling for performance
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; // Transfer destination and sampled usage
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // No sharing between queues
    vkResult = vkCreateImage(vkDevice, &imageCreateInfo, NULL, &imageData->vkImage);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "loadTextureData() : vkCreateImage() failed (%d).\n", vkResult);
        vkUnmapMemory(vkDevice, vkDeviceMemory_stagingBuffer); // Unmap the memory
        vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL); // Clean up the memory
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL); // Clean up the buffer
        stbi_image_free(data); // Free the pixel data before returning
        return vkResult;
    }
    // Allocate memory for the image
    VkMemoryRequirements imageMemoryRequirements;
    vkGetImageMemoryRequirements(vkDevice, imageData->vkImage, &imageMemoryRequirements);
    VkMemoryAllocateInfo imageMemoryAllocateInfo;
    memset(&imageMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
    imageMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    imageMemoryAllocateInfo.allocationSize = imageMemoryRequirements.size; // Size of the memory to allocate
    imageMemoryAllocateInfo.memoryTypeIndex = findMemoryType(
        imageMemoryRequirements.memoryTypeBits, // Memory type bits from the image
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT // Memory properties (device local for optimal performance)
    );
    if (imageMemoryAllocateInfo.memoryTypeIndex == UINT32_MAX)
    {
        fprintf(gpFILE, "loadTextureData() : findMemoryType() failed.\n");
        vkDestroyImage(vkDevice, imageData->vkImage, NULL); // Clean up the image
        vkUnmapMemory(vkDevice, vkDeviceMemory_stagingBuffer); // Unmap the memory
        vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL); // Clean up the memory
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL); // Clean up the buffer
        stbi_image_free(data); // Free the pixel data before returning
        return VK_ERROR_MEMORY_MAP_FAILED;
    }

    vkResult = vkAllocateMemory(vkDevice, &imageMemoryAllocateInfo, NULL, &imageData->vkDeviceMemory);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "loadTextureData() : vkAllocateMemory() failed (%d).\n", vkResult);
        vkDestroyImage(vkDevice, imageData->vkImage, NULL); // Clean up the image
        vkUnmapMemory(vkDevice, vkDeviceMemory_stagingBuffer); // Unmap the memory
        vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL); // Clean up the memory
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL); // Clean up the buffer
        stbi_image_free(data); // Free the pixel data before returning
        return vkResult;
    }
    // Bind the image memory
    vkResult = vkBindImageMemory(vkDevice, imageData->vkImage, imageData->vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "loadTextureData() : vkBindImageMemory() failed (%d).\n", vkResult);
        vkDestroyImage(vkDevice, imageData->vkImage, NULL); // Clean up the image
        vkUnmapMemory(vkDevice, vkDeviceMemory_stagingBuffer); // Unmap the memory
        vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL); // Clean up the memory
        vkFreeMemory(vkDevice, imageData->vkDeviceMemory, NULL); // Clean up the image memory
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL); // Clean up the buffer
        stbi_image_free(data); // Free the pixel data before returning
        return vkResult;
    }


    // Transition the image layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo commandBufferAllocateInfo;
    memset(&commandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = vkCommandPool; // Command pool for allocation
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // Primary command buffer
    commandBufferAllocateInfo.commandBufferCount = 1; // Allocate one command buffer
    vkResult = vkAllocateCommandBuffers(vkDevice, &commandBufferAllocateInfo, &commandBuffer);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "loadTextureData() : vkAllocateCommandBuffers() failed (%d).\n", vkResult);
        vkDestroyImage(vkDevice, imageData->vkImage, NULL); // Clean up the image
        vkUnmapMemory(vkDevice, vkDeviceMemory_stagingBuffer); // Unmap the memory
        vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL); // Clean up the memory
        vkFreeMemory(vkDevice, imageData->vkDeviceMemory, NULL); // Clean up the image memory
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL); // Clean up the buffer
        stbi_image_free(data); // Free the pixel data before returning
        return vkResult;
    }
    // Begin command buffer recording
    VkCommandBufferBeginInfo commandBufferBeginInfo;
    memset(&commandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // One-time use command buffer
    vkResult = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "loadTextureData() : vkBeginCommandBuffer() failed (%d).\n", vkResult);
        vkDestroyImage(vkDevice, imageData->vkImage, NULL); // Clean up the image
        vkUnmapMemory(vkDevice, vkDeviceMemory_stagingBuffer); // Unmap the memory
        vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL); // Clean up the memory
        vkFreeMemory(vkDevice, imageData->vkDeviceMemory, NULL); // Clean up the image memory
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL); // Clean up the buffer
        stbi_image_free(data); // Free the pixel data before returning
        return vkResult;
    }
    // Transition the image layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    VkImageMemoryBarrier imageMemoryBarrier;
    memset(&imageMemoryBarrier, 0, sizeof(VkImageMemoryBarrier));
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcAccessMask = 0; // No source access mask
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Destination access mask for transfer write
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Old layout is undefined
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; // New layout is transfer destination optimal
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL; // Source queue family index
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_EXTERNAL; // Destination queue family index
    imageMemoryBarrier.image = imageData->vkImage; // Image to transition
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Color aspect
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0; // Base mip level
    imageMemoryBarrier.subresourceRange.levelCount = 1; // One mip level
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0; // Base array layer
    imageMemoryBarrier.subresourceRange.layerCount = 1; // One layer
    vkCmdPipelineBarrier(
        commandBuffer, // Command buffer to record the barrier
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // Source stage mask
        VK_PIPELINE_STAGE_TRANSFER_BIT, // Destination stage mask
        0, // Dependency flags
        0, NULL, // No memory barriers
        0, NULL, // No buffer barriers
        1, &imageMemoryBarrier // Image memory barrier
    );
    // Copy the pixel data from the staging buffer to the image
    VkBufferImageCopy bufferImageCopy;
    memset(&bufferImageCopy, 0, sizeof(VkBufferImageCopy));
    bufferImageCopy.bufferOffset = 0; // Offset in the buffer
    bufferImageCopy.bufferRowLength = 0; // No row length (image is tightly packed)
    bufferImageCopy.bufferImageHeight = 0; // No image height (image is tightly packed)
    bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Color aspect
    bufferImageCopy.imageSubresource.mipLevel = 0; // Mip level
    bufferImageCopy.imageSubresource.baseArrayLayer = 0; // Base array layer
    bufferImageCopy.imageSubresource.layerCount = 1; // One layer
    bufferImageCopy.imageOffset = { 0, 0, 0 }; // Offset in the image
    bufferImageCopy.imageExtent = { (uint32_t)width, (uint32_t)height, 1 }; // Extent of the image
    vkCmdCopyBufferToImage(
        commandBuffer, // Command buffer to record the copy
        vkBuffer_stagingBuffer, // Source buffer (staging buffer)
        imageData->vkImage, // Destination image
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // Destination image layout
        1, // Number of regions to copy
        &bufferImageCopy // Pointer to the buffer image copy region
    );

    // Transition the image layout to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Source access mask for transfer write
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // Destination access mask for shader read
    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; // Old layout is transfer destination optimal
    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // New layout is shader read only optimal
    vkCmdPipelineBarrier(
        commandBuffer, // Command buffer to record the barrier
        VK_PIPELINE_STAGE_TRANSFER_BIT, // Source stage mask
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // Destination stage mask
        0, // Dependency flags
        0, NULL, // No memory barriers
        0, NULL, // No buffer barriers
        1, &imageMemoryBarrier // Image memory barrier
    );

    // End command buffer recording
    vkResult = vkEndCommandBuffer(commandBuffer);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "loadTextureData() : vkEndCommandBuffer() failed (%d).\n", vkResult);
        vkDestroyImage(vkDevice, imageData->vkImage, NULL); // Clean up the image
        vkUnmapMemory(vkDevice, vkDeviceMemory_stagingBuffer); // Unmap the memory
        vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL); // Clean up the memory
        vkFreeMemory(vkDevice, imageData->vkDeviceMemory, NULL); // Clean up the image memory
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL); // Clean up the buffer
        stbi_image_free(data); // Free the pixel data before returning
        return vkResult;
    }
    // Submit the command buffer to the queue
    VkSubmitInfo submitInfo;
    memset(&submitInfo, 0, sizeof(VkSubmitInfo));
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1; // One command buffer to submit
    submitInfo.pCommandBuffers = &commandBuffer; // Pointer to the command buffer
    vkResult = vkQueueSubmit(vkQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "loadTextureData() : vkQueueSubmit() failed (%d).\n", vkResult);
        vkDestroyImage(vkDevice, imageData->vkImage, NULL); // Clean up the image
        vkUnmapMemory(vkDevice, vkDeviceMemory_stagingBuffer); // Unmap the memory
        vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL); // Clean up the memory
        vkFreeMemory(vkDevice, imageData->vkDeviceMemory, NULL); // Clean up the image memory
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL); // Clean up the buffer
        stbi_image_free(data); // Free the pixel data before returning
        return vkResult;
    }
    // Wait for the queue to finish processing
    vkResult = vkQueueWaitIdle(vkQueue);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "loadTextureData() : vkQueueWaitIdle() failed (%d).\n", vkResult);
        vkDestroyImage(vkDevice, imageData->vkImage, NULL); // Clean up the image
        vkUnmapMemory(vkDevice, vkDeviceMemory_stagingBuffer); // Unmap the memory
        vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL); // Clean up the memory
        vkFreeMemory(vkDevice, imageData->vkDeviceMemory, NULL); // Clean up the image memory
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL); // Clean up the buffer
        stbi_image_free(data); // Free the pixel data before returning
        return vkResult;
    }
    // Clean up the staging buffer and memory
    vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL); // Clean up the staging buffer
    vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL); // Clean up the staging buffer memory
    // At this point, the texture image is ready to be used in rendering
    // 5) Create an image view for the texture image
    VkImageViewCreateInfo imageViewCreateInfo;
    memset(&imageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = imageData->vkImage; // The image to create the view for
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // 2D image view
    imageViewCreateInfo.format = format; // Format of the image
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Color aspect
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0; // Base mip level
    imageViewCreateInfo.subresourceRange.levelCount = 1; // One mip level
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0; // Base array layer
    imageViewCreateInfo.subresourceRange.layerCount = 1; // One layer
    vkResult = vkCreateImageView(vkDevice, &imageViewCreateInfo, NULL, &imageData->vkImageView);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "loadTextureData() : vkCreateImageView() failed (%d).\n", vkResult);
        vkDestroyImage(vkDevice, imageData->vkImage, NULL); // Clean up the image
        vkUnmapMemory(vkDevice, vkDeviceMemory_stagingBuffer); // Unmap the memory
        vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL); // Clean up the memory
        vkFreeMemory(vkDevice, imageData->vkDeviceMemory, NULL); // Clean up the image memory
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL); // Clean up the buffer
        stbi_image_free(data); // Free the pixel data before returning
        return vkResult;
    }
    // At this point, the texture image is ready to be used in rendering
    fprintf(gpFILE, "loadTextureData() : Texture image loaded successfully.\n");

    // 13) Free the pixel data when done
    stbi_image_free(data);

    // 14) Return success
    return vkResult;
}

VkResult loadTextureData_dds(ImageData* imageData, const char* filename)
{

    DDSFile dds;
    const auto res = dds.Load(filename); // tinyddsloader accepts const char*
    if (res != Result::Success)
    {
        fprintf(gpFILE, "loadTextureData_dds() : Failed to load DDS file '%s'\n", filename);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    const uint32_t arrayCount = dds.GetArraySize();
    const uint32_t mipCount = dds.GetMipCount();

    const uint32_t width = dds.GetWidth();
    const uint32_t height = dds.GetHeight();
    const uint32_t depth = dds.GetDepth();

    for (uint32_t arrayIdx = 0; arrayIdx < arrayCount; ++arrayIdx) {
        for (uint32_t mipIdx = 0; mipIdx < mipCount; ++mipIdx) {
            const DDSFile::ImageData* img = dds.GetImageData(mipIdx, arrayIdx);
            if (!img) {
                std::cerr << "  ImageData null for array " << arrayIdx << " mip " << mipIdx << "\n";
                continue;
            }

            //whats next?

        }
    }

    return VK_SUCCESS;
}

static inline uint64_t bc7_subresource_size(uint32_t width, uint32_t height)
{
    uint32_t blocksX = (width + 3) / 4; // ceil(width/4)
    uint32_t blocksY = (height + 3) / 4; // ceil(height/4)
    return (uint64_t)blocksX * (uint64_t)blocksY * 16ULL;
}

VkResult loadTextureData_dds_c_bc7(ImageData* imageData, const char* filename, VkFormat format)
{
    if (!imageData || !filename) return VK_ERROR_INITIALIZATION_FAILED;

    // Validate that caller passed a BC7 format
    if (!(format == VK_FORMAT_BC7_UNORM_BLOCK || format == VK_FORMAT_BC7_SRGB_BLOCK)) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): format is not BC7 (format=%d)\n", (int)format);
        return VK_ERROR_FORMAT_NOT_SUPPORTED;
    }

    DDSFile dds;
    const auto res = dds.Load(filename);
    if (res != Result::Success) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): tinyddsloader failed to load '%s' (res=%d)\n", filename, static_cast<int>(res));
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    const uint32_t arrayCount = dds.GetArraySize();
    const uint32_t mipCount = dds.GetMipCount();
    const uint32_t baseWidth = dds.GetWidth();
    const uint32_t baseHeight = dds.GetHeight();
    const uint32_t depth = dds.GetDepth();

    if (arrayCount == 0 || mipCount == 0) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): empty DDS '%s'\n", filename);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    const size_t subCount = (size_t)arrayCount * (size_t)mipCount;

    // Subresource bookkeeping (C-style)
    typedef struct { uint32_t mip; uint32_t layer; uint64_t rawSize; uint64_t paddedSize; } SubInfo;
    SubInfo* subs = (SubInfo*)malloc(sizeof(SubInfo) * subCount);
    if (!subs) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): malloc subs failed\n");
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    // Compute raw and padded sizes using bc7_subresource_size()
    uint64_t totalSize = 0;
    size_t idx = 0;
    for (uint32_t layer = 0; layer < arrayCount; ++layer) {
        for (uint32_t mip = 0; mip < mipCount; ++mip) {
            const DDSFile::ImageData* img = dds.GetImageData(mip, layer);
            if (!img) {
                fprintf(gpFILE, "loadTextureData_dds_c_bc7(): missing ImageData layer=%u mip=%u\n", layer, mip);
                free(subs);
                return VK_ERROR_INITIALIZATION_FAILED;
            }

            // Use loader's reported mip dims if present; fallback to base >> mip with min 1
            uint32_t mipW = img->m_width ? img->m_width : (baseWidth >> mip ? (baseWidth >> mip) : 1u);
            uint32_t mipH = img->m_height ? img->m_height : (baseHeight >> mip ? (baseHeight >> mip) : 1u);

            uint64_t raw = bc7_subresource_size(mipW, mipH);
            uint64_t padded = (raw + 3ull) & ~3ull; // pad to 4 byte boundary for bufferOffset

            subs[idx].mip = mip;
            subs[idx].layer = layer;
            subs[idx].rawSize = raw;
            subs[idx].paddedSize = padded;
            totalSize += padded;
            ++idx;
        }
    }

    if (totalSize == 0) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): totalSize == 0 for '%s'\n", filename);
        free(subs);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Create staging buffer
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

    VkBufferCreateInfo bufInfo = {};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = totalSize;
    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult vkRes = vkCreateBuffer(vkDevice, &bufInfo, nullptr, &stagingBuffer);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkCreateBuffer failed (%d)\n", vkRes);
        free(subs);
        return vkRes;
    }

    VkMemoryRequirements memReq = {};
    vkGetBufferMemoryRequirements(vkDevice, stagingBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (allocInfo.memoryTypeIndex == UINT32_MAX) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): findMemoryType failed for staging\n");
        vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
        free(subs);
        return VK_ERROR_MEMORY_MAP_FAILED;
    }

    vkRes = vkAllocateMemory(vkDevice, &allocInfo, nullptr, &stagingMemory);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkAllocateMemory failed (%d)\n", vkRes);
        vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    vkRes = vkBindBufferMemory(vkDevice, stagingBuffer, stagingMemory, 0);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkBindBufferMemory failed (%d)\n", vkRes);
        vkFreeMemory(vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    // Map and copy BC7 subresources into staging (use bc7_subresource_size)
    void* mapped = NULL;
    vkRes = vkMapMemory(vkDevice, stagingMemory, 0, totalSize, 0, &mapped);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkMapMemory failed (%d)\n", vkRes);
        vkFreeMemory(vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    uint64_t offset = 0;
    idx = 0;
    for (uint32_t layer = 0; layer < arrayCount; ++layer) {
        for (uint32_t mip = 0; mip < mipCount; ++mip) {
            const DDSFile::ImageData* img = dds.GetImageData(mip, layer);
            // compute size (should match earlier)
            uint32_t mipW = img->m_width ? img->m_width : (baseWidth >> mip ? (baseWidth >> mip) : 1u);
            uint32_t mipH = img->m_height ? img->m_height : (baseHeight >> mip ? (baseHeight >> mip) : 1u);
            uint64_t size = bc7_subresource_size(mipW, mipH);

            if (!img->m_mem) {
                fprintf(gpFILE, "loadTextureData_dds_c_bc7(): null m_mem for layer=%u mip=%u\n", layer, mip);
                vkUnmapMemory(vkDevice, stagingMemory);
                vkFreeMemory(vkDevice, stagingMemory, nullptr);
                vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
                free(subs);
                return VK_ERROR_INITIALIZATION_FAILED;
            }

            // copy computed number of bytes from loader pointer
            memcpy((uint8_t*)mapped + offset, img->m_mem, (size_t)size);

            // pad to paddedSize if required
            if (subs[idx].paddedSize > size) {
                memset((uint8_t*)mapped + offset + size, 0, (size_t)(subs[idx].paddedSize - size));
            }

            offset += subs[idx].paddedSize;
            ++idx;
        }
    }

    vkUnmapMemory(vkDevice, stagingMemory);

    // Create VkImage with the DDS mipCount / arrayCount
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format; // BC7_UNORM or BC7_SRGB as validated earlier
    imageInfo.extent.width = baseWidth;
    imageInfo.extent.height = baseHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipCount;
    imageInfo.arrayLayers = arrayCount;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.flags = (arrayCount == 6) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;

    VkImage image = VK_NULL_HANDLE;
    vkRes = vkCreateImage(vkDevice, &imageInfo, nullptr, &image);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkCreateImage failed (%d)\n", vkRes);
        vkFreeMemory(vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    VkMemoryRequirements imageMemReq = {};
    vkGetImageMemoryRequirements(vkDevice, image, &imageMemReq);

    VkMemoryAllocateInfo imageAlloc = {};
    imageAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    imageAlloc.allocationSize = imageMemReq.size;
    imageAlloc.memoryTypeIndex = findMemoryType(imageMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (imageAlloc.memoryTypeIndex == UINT32_MAX) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): findMemoryType failed for image\n");
        vkDestroyImage(vkDevice, image, nullptr);
        vkFreeMemory(vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
        free(subs);
        return VK_ERROR_MEMORY_MAP_FAILED;
    }

    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    vkRes = vkAllocateMemory(vkDevice, &imageAlloc, nullptr, &imageMemory);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkAllocateMemory(image) failed (%d)\n", vkRes);
        vkDestroyImage(vkDevice, image, nullptr);
        vkFreeMemory(vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    vkRes = vkBindImageMemory(vkDevice, image, imageMemory, 0);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkBindImageMemory failed (%d)\n", vkRes);
        vkFreeMemory(vkDevice, imageMemory, nullptr);
        vkDestroyImage(vkDevice, image, nullptr);
        vkFreeMemory(vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    // Command buffer: transition, copy, transition
    VkCommandBufferAllocateInfo cmdAlloc = {};
    cmdAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAlloc.commandPool = vkCommandPool;
    cmdAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAlloc.commandBufferCount = 1;

    VkCommandBuffer cmd = VK_NULL_HANDLE;
    vkRes = vkAllocateCommandBuffers(vkDevice, &cmdAlloc, &cmd);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkAllocateCommandBuffers failed (%d)\n", vkRes);
        vkFreeMemory(vkDevice, imageMemory, nullptr);
        vkDestroyImage(vkDevice, image, nullptr);
        vkFreeMemory(vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkRes = vkBeginCommandBuffer(cmd, &beginInfo);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkBeginCommandBuffer failed (%d)\n", vkRes);
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &cmd);
        vkFreeMemory(vkDevice, imageMemory, nullptr);
        vkDestroyImage(vkDevice, image, nullptr);
        vkFreeMemory(vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    VkImageMemoryBarrier barrierToTransfer = {};
    barrierToTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrierToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrierToTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierToTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierToTransfer.image = image;
    barrierToTransfer.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrierToTransfer.subresourceRange.baseMipLevel = 0;
    barrierToTransfer.subresourceRange.levelCount = mipCount;
    barrierToTransfer.subresourceRange.baseArrayLayer = 0;
    barrierToTransfer.subresourceRange.layerCount = arrayCount;
    barrierToTransfer.srcAccessMask = 0;
    barrierToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrierToTransfer);

    // Build copy regions (C array)
    VkBufferImageCopy* copies = (VkBufferImageCopy*)malloc(sizeof(VkBufferImageCopy) * subCount);
    if (!copies) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): malloc copies failed\n");
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &cmd);
        vkFreeMemory(vkDevice, imageMemory, nullptr);
        vkDestroyImage(vkDevice, image, nullptr);
        vkFreeMemory(vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
        free(subs);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    offset = 0;
    idx = 0;
    for (uint32_t layer = 0; layer < arrayCount; ++layer) {
        for (uint32_t mip = 0; mip < mipCount; ++mip) {
            const DDSFile::ImageData* img = dds.GetImageData(mip, layer);
            VkBufferImageCopy region = {};
            region.bufferOffset = offset;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = mip;
            region.imageSubresource.baseArrayLayer = layer;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = { 0,0,0 };
            region.imageExtent = { img->m_width ? img->m_width : (baseWidth >> mip ? (baseWidth >> mip) : 1u),
                                   img->m_height ? img->m_height : (baseHeight >> mip ? (baseHeight >> mip) : 1u),
                                   img->m_depth ? img->m_depth : 1u };
            copies[idx] = region;
            offset += subs[idx].paddedSize;
            ++idx;
        }
    }

    vkCmdCopyBufferToImage(cmd, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        (uint32_t)subCount, copies);

    VkImageMemoryBarrier barrierToReadable = barrierToTransfer;
    barrierToReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrierToReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrierToReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrierToReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrierToReadable);

    // Finish & submit
    vkRes = vkEndCommandBuffer(cmd);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkEndCommandBuffer failed (%d)\n", vkRes);
        free(copies);
        free(subs);
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &cmd);
        vkFreeMemory(vkDevice, imageMemory, nullptr);
        vkDestroyImage(vkDevice, image, nullptr);
        vkFreeMemory(vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
        return vkRes;
    }

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;

    vkRes = vkQueueSubmit(vkQueue, 1, &submit, VK_NULL_HANDLE);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkQueueSubmit failed (%d)\n", vkRes);
        free(copies);
        free(subs);
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &cmd);
        vkFreeMemory(vkDevice, imageMemory, nullptr);
        vkDestroyImage(vkDevice, image, nullptr);
        vkFreeMemory(vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
        return vkRes;
    }

    vkRes = vkQueueWaitIdle(vkQueue);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkQueueWaitIdle failed (%d)\n", vkRes);
        free(copies);
        free(subs);
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &cmd);
        vkFreeMemory(vkDevice, imageMemory, nullptr);
        vkDestroyImage(vkDevice, image, nullptr);
        vkFreeMemory(vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
        return vkRes;
    }

    // free command buffer
    vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &cmd);

    // Create image view
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    if (arrayCount == 1) viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    else if (arrayCount == 6 && (imageInfo.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)) viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    else viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipCount;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = arrayCount;

    VkImageView imageView = VK_NULL_HANDLE;
    vkRes = vkCreateImageView(vkDevice, &viewInfo, nullptr, &imageView);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkCreateImageView failed (%d)\n", vkRes);
        free(copies);
        free(subs);
        vkFreeMemory(vkDevice, imageMemory, nullptr);
        vkDestroyImage(vkDevice, image, nullptr);
        vkFreeMemory(vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
        return vkRes;
    }

    // cleanup staging
    vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(vkDevice, stagingMemory, nullptr);

    // fill output
    imageData->vkImage = image;
    imageData->vkDeviceMemory = imageMemory;
    imageData->vkImageView = imageView;
    // imageData->width = baseWidth;
    // imageData->height = baseHeight;
    // imageData->mipLevels = mipCount;
    // imageData->arrayLayers = arrayCount;

    fprintf(gpFILE, "loadTextureData_dds_c_bc7(): Loaded '%s' w=%u h=%u mips=%u layers=%u (BC7)\n",
        filename, baseWidth, baseHeight, mipCount, arrayCount);

    // free temporaries
    free(copies);
    free(subs);

    return VK_SUCCESS;
}

void destroyTextureData(ImageData* imageData)
{
    if (imageData->vkImageView)
    {
        vkDestroyImageView(vkDevice, imageData->vkImageView, NULL);
        imageData->vkImageView = VK_NULL_HANDLE;
    }
    if (imageData->vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, imageData->vkDeviceMemory, NULL);
        imageData->vkDeviceMemory = VK_NULL_HANDLE;
    }

    if (imageData->vkImage)
    {
        vkDestroyImage(vkDevice, imageData->vkImage, NULL);
        imageData->vkImage = VK_NULL_HANDLE;
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

    // fill instance extension names
    vkResult = fillInstanceExtensionNames();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVulkanInstance() : fillInstanceExtensionNames() failed.\n");
        return(vkResult);
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
    }
    /*
     *  initialize struct VkApplicationInfo
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
     *  initialize struct VkInstanceCreateInfo by using
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


    //ZzNeO features
    //VkPhysicalDeviceFeatures enabledFeatures;
    //memset((void*)&enabledFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
    //enabledFeatures.samplerAnisotropy = VK_TRUE; // enable anisotropic filtering
    //enabledFeatures.tessellationShader = VK_TRUE; // enable tessellation shader

    //enabledFeatures.geometryShader = VK_TRUE; // enable geometry shader

        //------------------------------//
    VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering{};
    dynamicRendering.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    dynamicRendering.dynamicRendering = VK_TRUE;

    VkPhysicalDeviceTimelineSemaphoreFeatures timeline{};
    timeline.sType =VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
    timeline.timelineSemaphore = VK_TRUE;
    timeline.pNext = &dynamicRendering;

    VkPhysicalDeviceDescriptorIndexingFeatures indexing{};
    indexing.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    indexing.runtimeDescriptorArray = VK_TRUE;
    indexing.descriptorBindingPartiallyBound = VK_TRUE;
    indexing.descriptorBindingVariableDescriptorCount = VK_TRUE;
    indexing.shaderSampledImageArrayNonUniformIndexing = VK_TRUE; 
    indexing.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    indexing.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
	indexing.pNext = &timeline;

	//synchronization2

	VkPhysicalDeviceSynchronization2Features sync2{};
	sync2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
	sync2.synchronization2 = VK_TRUE;
	sync2.pNext = &indexing;

    //descriptorBindingSampledImageUpdateAfterBind
    //descriptorBindingUpdateUnusedWhilePending

    VkPhysicalDeviceFeatures2 features2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	features2.features.samplerAnisotropy = VK_TRUE; // enable anisotropic filtering
	features2.features.tessellationShader = VK_TRUE; // enable tessellation shader
	features2.pNext = &sync2;

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

    vkFormat_Color = VK_FORMAT_B8G8R8A8_SRGB;
    vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    /*
     *  Free the above created array
     */
    if (vkSurfaceFormatKHR_Array)
    {
        free(vkSurfaceFormatKHR_Array);
        vkSurfaceFormatKHR_Array = NULL;

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
    }

    return(vkResult);
}

VkResult createGlobalTextureDescriptorArray(void)
{
    const uint32_t MAX_TEXTURES = 1024;
    
	VkResult vkResult = VK_SUCCESS; 

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = MAX_TEXTURES;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;


    vkResult = vkCreateDescriptorPool(vkDevice, &poolInfo, nullptr, &global_textureArray_vkDescriptorPool);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createTextureIndexing() : vkCreateDescriptorPool() failed (%d).\n", vkResult);
		return vkResult;
	}


    uint32_t variableCount = MAX_TEXTURES;

    VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{};
    countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    countInfo.descriptorSetCount = 1;
    countInfo.pDescriptorCounts = &variableCount;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = &countInfo;
    allocInfo.descriptorPool = global_textureArray_vkDescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &gpDescriptorSetLayouts->vkDescriptorSetLayout_GlobalTextureArray;


    vkResult = vkAllocateDescriptorSets(vkDevice, &allocInfo, &global_textureArray_vkDescriptorSet);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createTextureIndexing() : vkAllocateDescriptorSets() failed (%d).\n", vkResult);
		return vkResult;

    }


	std::vector<VkDescriptorImageInfo> infos(global_textureArray.size());
    for (size_t i = 0; i < global_textureArray.size(); i++)
    {
        infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        infos[i].imageView = global_textureArray[i]->vkImageView;
		infos[i].sampler = global_textureArray[i]->vkSampler;
    }

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = global_textureArray_vkDescriptorSet;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = static_cast<uint32_t>(infos.size());
    write.pImageInfo = infos.data();

	vkUpdateDescriptorSets(vkDevice, 1, &write, 0, nullptr);


	return vkResult;
}

VkResult destroyGlobalTextureDescriptorArray(void)
{
    if (global_textureArray_vkDescriptorPool)
    {
        vkDestroyDescriptorPool(vkDevice, global_textureArray_vkDescriptorPool, nullptr);
        global_textureArray_vkDescriptorPool = VK_NULL_HANDLE;
    }

	return VK_SUCCESS;

}

VkResult createSamplers(void)
{
    VkResult vkResult = VK_SUCCESS;

    //vkSampler_LinearClampAniso
    VkSamplerCreateInfo samplerCreateInfo;
    memset(&samplerCreateInfo, 0, sizeof(VkSamplerCreateInfo));
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR; // Magnification filter
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR; // Minification filter
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // U coordinate addressing mode
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // V coordinate addressing mode
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // W coordinate addressing mode
    samplerCreateInfo.anisotropyEnable = VK_TRUE; // Enable anisotropic filtering
    samplerCreateInfo.maxAnisotropy = 16.0f; // Maximum anisotropy level
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Border color
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE; // Use normalized coordinates
    samplerCreateInfo.compareEnable = VK_FALSE; // No comparison
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS; // Comparison operation (not used here)
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // Mipmap mode
    samplerCreateInfo.mipLodBias = 0.0f; // Mipmap level of detail bias
    samplerCreateInfo.minLod = 0.0f; // Minimum LOD
    samplerCreateInfo.maxLod = 14.0f; // Maximum LOD (0 means no mipmaps)
    vkResult = vkCreateSampler(vkDevice, &samplerCreateInfo, NULL, &vkSampler_LinearClampAniso);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createSamplers() : vkCreateSampler() for vkSampler_LinearClampAniso failed (%d).\n", vkResult);
        return vkResult;
    }

    //vkSampler_LinearClamp
    memset(&samplerCreateInfo, 0, sizeof(VkSamplerCreateInfo));
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR; // Magnification filter
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR; // Minification filter
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // U coordinate addressing mode
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // V coordinate addressing mode
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // W coordinate addressing mode
    samplerCreateInfo.anisotropyEnable = VK_FALSE; // Disable anisotropic filtering
    samplerCreateInfo.maxAnisotropy = 1.0f; // Maximum anisotropy level (1.0 means no anisotropic filtering)
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Border color
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE; // Use normalized coordinates
    samplerCreateInfo.compareEnable = VK_FALSE; // No comparison
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS; // Comparison operation (not used here)
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // Mipmap mode
    samplerCreateInfo.mipLodBias = 0.0f; // Mipmap level of detail bias
    samplerCreateInfo.minLod = 0.0f; // Minimum LOD
    samplerCreateInfo.maxLod = 0.0f; // Maximum LOD (0 means no mipmaps)
    vkResult = vkCreateSampler(vkDevice, &samplerCreateInfo, NULL, &vkSampler_LinearClamp);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createSamplers() : vkCreateSampler() for vkSampler_LinearClamp failed (%d).\n", vkResult);
        return vkResult;
    }

    //vkSampler_LinearMipmapClamp
    memset(&samplerCreateInfo, 0, sizeof(VkSamplerCreateInfo));
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR; // Magnification filter
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR; // Minification filter
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // U coordinate addressing mode
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // V coordinate addressing mode
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // W coordinate addressing mode
    samplerCreateInfo.anisotropyEnable = VK_FALSE; // Disable anisotropic filtering
    samplerCreateInfo.maxAnisotropy = 1.0f; // Maximum anisotropy level (1.0 means no anisotropic filtering)
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // Border color
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE; // Use normalized coordinates
    samplerCreateInfo.compareEnable = VK_FALSE; // No comparison
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS; // Comparison operation (not used here)
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // Mipmap mode
    samplerCreateInfo.mipLodBias = 0.0f; // Mipmap level of detail bias
    samplerCreateInfo.minLod = 0.0f; // Minimum LOD
    samplerCreateInfo.maxLod = 14.0f; // Maximum LOD (0 means no mipmaps)
    vkResult = vkCreateSampler(vkDevice, &samplerCreateInfo, NULL, &vkSampler_LinearMipmapClamp);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createSamplers() : vkCreateSampler() for vkSampler_LinearMipmapClamp failed (%d).\n", vkResult);
        return vkResult;
    }


    fprintf(gpFILE, "createSamplers() : Samplers created successfully.\n");

    return VK_SUCCESS;
}

void destroySamplers(void)
{
    // code
    if (vkSampler_LinearClampAniso)
    {
        vkDestroySampler(vkDevice, vkSampler_LinearClampAniso, NULL);
        vkSampler_LinearClampAniso = VK_NULL_HANDLE;
    }
    if (vkSampler_LinearClamp)
    {
        vkDestroySampler(vkDevice, vkSampler_LinearClamp, NULL);
        vkSampler_LinearClamp = VK_NULL_HANDLE;
    }
    if (vkSampler_LinearMipmapClamp)
    {
        vkDestroySampler(vkDevice, vkSampler_LinearMipmapClamp, NULL);
        vkSampler_LinearMipmapClamp = VK_NULL_HANDLE;
    }

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

        vkExtent2D_Swapchain.width = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width)); // clamp the width between minImageExtent.width and maxImageExtent.width
        vkExtent2D_Swapchain.height = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.height, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height)); // clamp the height between minImageExtent.height and maxImageExtent.height

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
        &gSwapchainImageCount, // [out] Swapchain Image Count
        NULL                  // [out, optional] Swapchain Image array
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createImagesAndImageViews() : vkGetSwapchainImagesKHR()'s 1st call failed.\n");
        return(vkResult);
    }
    else if (gSwapchainImageCount == 0)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFILE, "createImagesAndImageViews() : Swapchain image count is 0.\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFILE, "createImagesAndImageViews() : gives swapchain image count = %d\n", gSwapchainImageCount);
    }

    // Declare a global VkImage array and allocate it to the swapchain image count using malloc().
    gSwapChainResourceData.swapchainImage_Array = (VkImage*)malloc(sizeof(VkImage) * gSwapchainImageCount);

    // Now call the same function again, which we called in step 1 and fill this array.
    vkResult = vkGetSwapchainImagesKHR(
        vkDevice,
        vkSwapchainKHR,
        &gSwapchainImageCount,
        gSwapChainResourceData.swapchainImage_Array
    );

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createImagesAndImageViews() : vkGetSwapchainImagesKHR()'s 2nd call failed.\n");
        return(vkResult);
    }

    //  Declare another global array of type VkImageView and allocate it to the size of swapchain image count.
    gSwapChainResourceData.swapchainImageView_Array = (VkImageView*)malloc(sizeof(VkImageView) * gSwapchainImageCount);

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
    for (uint32_t i = 0; i < gSwapchainImageCount; i++)
    {
        vkImageViewCreateInfo.image = gSwapChainResourceData.swapchainImage_Array[i];

        vkResult = vkCreateImageView(
            vkDevice,                    // [in] VkDevice
            &vkImageViewCreateInfo,      // [in] VkImageViewCreateInfo *
            NULL,                        // [in] custom memory allocator
            &gSwapChainResourceData.swapchainImageView_Array[i] // [out] VkImageView * 
        );

        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createImagesAndImageViews() : vkCreateImageView() failed for iteration %d. (%d)\n", i, vkResult);
            return(vkResult);
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
    vkCommandBuffer_Array = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * gSwapchainImageCount);

    // sub-step 3 : In a loop which is equal to swapchain image count, allocate each command buffer 
    //              in the above array by using vkAllocateCommandBuffers() API. 
    //              Remember, at the time of allocation, all buffers are going to be empty. 
    //              Later we will record graphics / compute commands into them.
    for (uint32_t i = 0; i < gSwapchainImageCount; i++)
    {
        vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_Array[i]);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createCommandBuffers() : vkAllocateCommandBuffers failed for iteration %d.\n", i);
            return(vkResult);
        }
    }

    return(vkResult);
}

/*
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

    memset((void*)&vertexData_position, 0, sizeof(VulkanData));

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


    //---------------
    vkResult = vkBindBufferMemory(vkDevice, vertexData_position.vkBuffer, vertexData_position.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
        return(vkResult);
    }


    //----------------
    void* data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_position.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkMapMemory() :  failed.\n");
        return(vkResult);
    }


    //-------actual memory mapped io
    memcpy(data, triangle_position, sizeof(triangle_position));

    //-------unmap memory
    vkUnmapMemory(vkDevice, vertexData_position.vkDeviceMemory);

    return vkResult;
}
*/

VkResult ZzCreateVertexBuffer(const float* vertices, VkDeviceSize vertexBufferSize, VulkanData* vulkanData)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    //// 3 vertices, each with 3D position and RGB color
    //const VertexData_PositionColor vertices[] = {
    //    {{ 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // bottom (z=0), red
    //    {{ -1.0f,  -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},  // right, green
    //    {{ 1.0f,  -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // left, blue
    //};


    //staging buffer
    VulkanData vertexData_stagingBffer_position;
    memset((void*)&vertexData_stagingBffer_position, 0, sizeof(VulkanData));

    VkBufferCreateInfo vkBufferCreateInfo_stagingBuffer;
    memset((void*)&vkBufferCreateInfo_stagingBuffer, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo_stagingBuffer.pNext = NULL;
    vkBufferCreateInfo_stagingBuffer.flags = 0;
    vkBufferCreateInfo_stagingBuffer.size = vertexBufferSize;
    vkBufferCreateInfo_stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // staging buffer is used for transfering data to device local buffer
    vkBufferCreateInfo_stagingBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // not sharing with other queues

    // Call vkCreateBuffer() to create the staging buffer
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo_stagingBuffer, NULL, &vertexData_stagingBffer_position.vkBuffer);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer():  failed.\n");
        return(vkResult);
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

    //---------------
    // Bind the staging buffer memory to the staging buffer
    vkResult = vkBindBufferMemory(vkDevice, vertexData_stagingBffer_position.vkBuffer, vertexData_stagingBffer_position.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
        return(vkResult);
    }

    //----------------
    void* data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_stagingBffer_position.vkDeviceMemory, 0, vkMemoryAllocateInfo_stagingBuffer.allocationSize, 0, &data);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkMapMemory() :  failed.\n");
        return(vkResult);
    }


    //-------actual memory mapped io
    memcpy(data, vertices, vertexBufferSize);
    //-------unmap memory
    vkUnmapMemory(vkDevice, vertexData_stagingBffer_position.vkDeviceMemory);

    //-----------------------------------------------------------------------------------

    //device buffer
    memset((void*)vulkanData, 0, sizeof(VulkanData));
    VkBufferCreateInfo vkBufferCreateInfo_deviceBuffer;
    memset((void*)&vkBufferCreateInfo_deviceBuffer, 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo_deviceBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo_deviceBuffer.pNext = NULL;
    vkBufferCreateInfo_deviceBuffer.flags = 0;
    vkBufferCreateInfo_deviceBuffer.size = vertexBufferSize;
    vkBufferCreateInfo_deviceBuffer.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; // device buffer is used for vertex buffer and transfer destination
    vkBufferCreateInfo_deviceBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // not sharing with other queues

    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo_deviceBuffer, NULL, &vulkanData->vkBuffer);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer():  failed.\n");
        return(vkResult);
    }


    //------------
    // Get memory requirements for the device local buffer
    VkMemoryRequirements vkMemoryRequirements_deviceBuffer;
    memset((void*)&vkMemoryRequirements_deviceBuffer, 0, sizeof(vkMemoryRequirements_deviceBuffer));
    vkGetBufferMemoryRequirements(vkDevice, vulkanData->vkBuffer, &vkMemoryRequirements_deviceBuffer);
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
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo_deviceBuffer, NULL, &vulkanData->vkDeviceMemory);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  failed.\n");
        return(vkResult);
    }

    //---------------
    // Bind the device local buffer memory to the device local buffer
    vkResult = vkBindBufferMemory(vkDevice, vulkanData->vkBuffer, vulkanData->vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
        return(vkResult);
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


    //----------------
    // Record the command to copy data from staging buffer to device local buffer
    VkBufferCopy vkBufferCopy;
    memset((void*)&vkBufferCopy, 0, sizeof(VkBufferCopy));
    vkBufferCopy.srcOffset = 0; // offset in the source buffer
    vkBufferCopy.dstOffset = 0; // offset in the destination buffer
    vkBufferCopy.size = vertexBufferSize; // size of the data to copy
    vkCmdCopyBuffer(vkCommandBuffer_Copy, vertexData_stagingBffer_position.vkBuffer, vulkanData->vkBuffer, 1, &vkBufferCopy);

    // End command buffer recording
    vkResult = vkEndCommandBuffer(vkCommandBuffer_Copy);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkEndCommandBuffer() :  failed.\n");
        return(vkResult);
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

    // Wait for the queue to finish processing
    vkResult = vkQueueWaitIdle(vkQueue);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkQueueWaitIdle() :  failed.\n");
        return(vkResult);
    }


    //----------------
    // Free the staging buffer
    if (vertexData_stagingBffer_position.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_stagingBffer_position.vkBuffer, NULL);
        vertexData_stagingBffer_position.vkBuffer = VK_NULL_HANDLE;
    }


    // Free the staging buffer memory
    if (vertexData_stagingBffer_position.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_stagingBffer_position.vkDeviceMemory, NULL);
        vertexData_stagingBffer_position.vkDeviceMemory = VK_NULL_HANDLE;
    }

    //-----------------------------------------------------------------------------------
    // Now, vertexData_position.vkBuffer contains the device local buffer with the triangle position data
    // and vertexData_position.vkDeviceMemory contains the device local buffer memory.

    if (vkCommandBuffer_Copy)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_Copy);
        vkCommandBuffer_Copy = VK_NULL_HANDLE;
    }

    return vkResult;
}

VkResult ZzCreateIndex16Buffer(const uint16_t* indices, VkDeviceSize indexBufferSize, VulkanData* vulkanData)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    //position index buffer
//----------------------------------------------------------------------------------------------------
    memset((void*)vulkanData, 0, sizeof(VulkanData));

    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = indexBufferSize;
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vulkanData->vkBuffer);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer() (index):  failed.\n");
        return(vkResult);
    }

    //------------
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(vkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, vulkanData->vkBuffer, &vkMemoryRequirements);

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
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vulkanData->vkDeviceMemory);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  failed.\n");
        return(vkResult);
    }


    //---------------
    vkResult = vkBindBufferMemory(vkDevice, vulkanData->vkBuffer, vulkanData->vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
        return(vkResult);
    }


    //----------------
    void* dataIndex = NULL;
    vkResult = vkMapMemory(vkDevice, vulkanData->vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &dataIndex);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkMapMemory() :  failed.\n");
        return(vkResult);
    }


    //-------actual memory mapped io
    memcpy(dataIndex, indices, indexBufferSize);

    //-------unmap memory
    vkUnmapMemory(vkDevice, vulkanData->vkDeviceMemory);

    return vkResult;
}

VkResult ZzCreateIndex32Buffer(const uint32_t* indices, VkDeviceSize indexBufferSize, VulkanData* vulkanData)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    //position index buffer
//----------------------------------------------------------------------------------------------------
    memset((void*)vulkanData, 0, sizeof(VulkanData));

    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = indexBufferSize;
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vulkanData->vkBuffer);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer() (index):  failed.\n");
        return(vkResult);
    }

    //------------
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(vkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, vulkanData->vkBuffer, &vkMemoryRequirements);

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
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vulkanData->vkDeviceMemory);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  failed.\n");
        return(vkResult);
    }


    //---------------
    vkResult = vkBindBufferMemory(vkDevice, vulkanData->vkBuffer, vulkanData->vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
        return(vkResult);
    }


    //----------------
    void* dataIndex = NULL;
    vkResult = vkMapMemory(vkDevice, vulkanData->vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &dataIndex);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkMapMemory() :  failed.\n");
        return(vkResult);
    }


    //-------actual memory mapped io
    memcpy(dataIndex, indices, indexBufferSize);

    //-------unmap memory
    vkUnmapMemory(vkDevice, vulkanData->vkDeviceMemory);

    return vkResult;
}

VkResult ZzCreateVertexAndIndex16Buffer(
    const float* vertices,
    VkDeviceSize vertexBufferSize,
    const uint16_t* indices,
    VkDeviceSize indexBufferSize,
    VulkanComboData* vulkanComboData)
{
    // Create vertex buffer
    VkResult vkResult = ZzCreateVertexBuffer(vertices, vertexBufferSize, &vulkanComboData->vertexData);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "ZzCreateVertexAndIndexBuffer() -> ZzCreateVertexBuffer() failed.\n");
        return vkResult;
    }
    // Create index buffer
    vkResult = ZzCreateIndex16Buffer(indices, indexBufferSize, &vulkanComboData->indexData);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "ZzCreateVertexAndIndexBuffer() -> ZzCreateIndex16Buffer() failed.\n");
        return vkResult;
    }
    return vkResult;
}

VkResult ZzCreateVertexAndIndex32Buffer(
    const float* vertices,
    VkDeviceSize vertexBufferSize,
    const uint32_t* indices,
    VkDeviceSize indexBufferSize,
    VulkanComboData* vulkanComboData)
{
    // Create vertex buffer
    VkResult vkResult = ZzCreateVertexBuffer(vertices, vertexBufferSize, &vulkanComboData->vertexData);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "ZzCreateVertexAndIndexBuffer() -> ZzCreateVertexBuffer() failed.\n");
        return vkResult;
    }
    // Create index buffer
    vkResult = ZzCreateIndex32Buffer(indices, indexBufferSize, &vulkanComboData->indexData);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "ZzCreateVertexAndIndexBuffer() -> ZzCreateIndex16Buffer() failed.\n");
        return vkResult;
    }
    return vkResult;
}

void ZzDestroyVertexBuffer(VulkanData* vulkanData)
{

    // Destroy the vertex buffer
    if (vulkanData->vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vulkanData->vkBuffer, NULL);
        vulkanData->vkBuffer = VK_NULL_HANDLE;
    }
    // Free the vertex buffer memory
    if (vulkanData->vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vulkanData->vkDeviceMemory, NULL);
        vulkanData->vkDeviceMemory = VK_NULL_HANDLE;
    }
}

void ZzDestroyIndexBuffer(VulkanData* vulkanData)
{
    // Destroy the index buffer
    if (vulkanData->vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vulkanData->vkBuffer, NULL);
        vulkanData->vkBuffer = VK_NULL_HANDLE;
    }
    // Free the index buffer memory
    if (vulkanData->vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vulkanData->vkDeviceMemory, NULL);
        vulkanData->vkDeviceMemory = VK_NULL_HANDLE;
    }
}

void ZzDestroyVertexAndIndexBuffer(VulkanComboData* vulkanComboData)
{
    // Destroy vertex buffer
    ZzDestroyVertexBuffer(&vulkanComboData->vertexData);
    // Destroy index buffer
    ZzDestroyIndexBuffer(&vulkanComboData->indexData);
}

VkResult createVertexBuffer_coloredTriangle(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // 3 vertices, each with 3D position and RGB color
    const VertexData_PositionColor vertices[] = {
        {{ 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // bottom (z=0), red
        {{ -1.0f,  -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},  // right, green
        {{ 1.0f,  -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // left, blue
    };

    //    // 3 vertices, each with 3D position and RGB color
    //const VertexData_PositionTexCoord vertices[] = {
    //    {{ 1.0f, 1.0f, 0.0f}, { 0.0f, 0.0f}},  // bottom (z=0), red
    //    {{ -1.0f,  -1.0f, 0.0f}, {0.0f,  0.0f}},  // right, green
    //    {{ 1.0f,  -1.0f, 0.0f}, {0.0f, 0.0f}},  // left, blue
    //};


    //    // 3 vertices, each with 3D position and RGB color
    //const VertexData_Position vertices[] = {
    //    {{ 0.0f, 1.0f, 0.0f}},  // bottom (z=0), red
    //    {{ -1.0f,  -1.0f, 0.0f}},  // right, green
    //    {{ 1.0f,  -1.0f, 0.0f}},  // left, blue
    //};

    //staging buffer
    VulkanData vertexData_stagingBffer_position;
    memset((void*)&vertexData_stagingBffer_position, 0, sizeof(VulkanData));

    VkBufferCreateInfo vkBufferCreateInfo_stagingBuffer;
    memset((void*)&vkBufferCreateInfo_stagingBuffer, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo_stagingBuffer.pNext = NULL;
    vkBufferCreateInfo_stagingBuffer.flags = 0;
    vkBufferCreateInfo_stagingBuffer.size = sizeof(vertices);
    vkBufferCreateInfo_stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // staging buffer is used for transfering data to device local buffer
    vkBufferCreateInfo_stagingBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // not sharing with other queues

    // Call vkCreateBuffer() to create the staging buffer
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo_stagingBuffer, NULL, &vertexData_stagingBffer_position.vkBuffer);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer():  failed.\n");
        return(vkResult);
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

    //---------------
    // Bind the staging buffer memory to the staging buffer
    vkResult = vkBindBufferMemory(vkDevice, vertexData_stagingBffer_position.vkBuffer, vertexData_stagingBffer_position.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
        return(vkResult);
    }

    //----------------
    void* data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_stagingBffer_position.vkDeviceMemory, 0, vkMemoryAllocateInfo_stagingBuffer.allocationSize, 0, &data);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkMapMemory() :  failed.\n");
        return(vkResult);
    }


    //-------actual memory mapped io
    memcpy(data, vertices, sizeof(vertices));
    //-------unmap memory
    vkUnmapMemory(vkDevice, vertexData_stagingBffer_position.vkDeviceMemory);

    //-----------------------------------------------------------------------------------

    //device buffer
    memset((void*)&vertexData_coloredTriangle, 0, sizeof(VulkanData));
    VkBufferCreateInfo vkBufferCreateInfo_deviceBuffer;
    memset((void*)&vkBufferCreateInfo_deviceBuffer, 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo_deviceBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo_deviceBuffer.pNext = NULL;
    vkBufferCreateInfo_deviceBuffer.flags = 0;
    vkBufferCreateInfo_deviceBuffer.size = sizeof(vertices);
    vkBufferCreateInfo_deviceBuffer.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; // device buffer is used for vertex buffer and transfer destination
    vkBufferCreateInfo_deviceBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // not sharing with other queues

    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo_deviceBuffer, NULL, &vertexData_coloredTriangle.vkBuffer);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer():  failed.\n");
        return(vkResult);
    }


    //------------
    // Get memory requirements for the device local buffer
    VkMemoryRequirements vkMemoryRequirements_deviceBuffer;
    memset((void*)&vkMemoryRequirements_deviceBuffer, 0, sizeof(vkMemoryRequirements_deviceBuffer));
    vkGetBufferMemoryRequirements(vkDevice, vertexData_coloredTriangle.vkBuffer, &vkMemoryRequirements_deviceBuffer);
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
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo_deviceBuffer, NULL, &vertexData_coloredTriangle.vkDeviceMemory);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  failed.\n");
        return(vkResult);
    }

    //---------------
    // Bind the device local buffer memory to the device local buffer
    vkResult = vkBindBufferMemory(vkDevice, vertexData_coloredTriangle.vkBuffer, vertexData_coloredTriangle.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
        return(vkResult);
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


    //----------------
    // Record the command to copy data from staging buffer to device local buffer
    VkBufferCopy vkBufferCopy;
    memset((void*)&vkBufferCopy, 0, sizeof(VkBufferCopy));
    vkBufferCopy.srcOffset = 0; // offset in the source buffer
    vkBufferCopy.dstOffset = 0; // offset in the destination buffer
    vkBufferCopy.size = sizeof(vertices); // size of the data to copy
    vkCmdCopyBuffer(vkCommandBuffer_Copy, vertexData_stagingBffer_position.vkBuffer, vertexData_coloredTriangle.vkBuffer, 1, &vkBufferCopy);

    // End command buffer recording
    vkResult = vkEndCommandBuffer(vkCommandBuffer_Copy);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkEndCommandBuffer() :  failed.\n");
        return(vkResult);
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

    // Wait for the queue to finish processing
    vkResult = vkQueueWaitIdle(vkQueue);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkQueueWaitIdle() :  failed.\n");
        return(vkResult);
    }


    //----------------
    // Free the staging buffer
    if (vertexData_stagingBffer_position.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_stagingBffer_position.vkBuffer, NULL);
        vertexData_stagingBffer_position.vkBuffer = VK_NULL_HANDLE;
    }


    // Free the staging buffer memory
    if (vertexData_stagingBffer_position.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_stagingBffer_position.vkDeviceMemory, NULL);
        vertexData_stagingBffer_position.vkDeviceMemory = VK_NULL_HANDLE;
    }

    //-----------------------------------------------------------------------------------
    // Now, vertexData_position.vkBuffer contains the device local buffer with the triangle position data
    // and vertexData_position.vkDeviceMemory contains the device local buffer memory.

    if (vkCommandBuffer_Copy)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_Copy);
        vkCommandBuffer_Copy = VK_NULL_HANDLE;
    }

    return vkResult;
}
/*
VkResult createVertexBuffer_uvQuad(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

  //  // 3 vertices, each with 3D position and RGB color
  //  const VertexData_PositionTexCoord verticesUv[] = {
  //      {{ 1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},  // TR
        //{{ -1.0f,  1.0f, 0.0f}, {0.0f,1.0f}},  // TL
  //      {{ -1.0f,  -1.0f, 0.0f}, {0.0f,0.0f}}, // BL
        //{{ 1.0f,  -1.0f, 0.0f}, {1.0f,0.0f}},  // BR
  //  };

    //const VertexData_PositionTexCoord Vertices_Uv[] = {
    //{{  0.244862f,  8.643809f, -0.592991f }, { 0.002604f, 0.999479f }},
    //{{ -0.264767f,  8.643809f, -0.083362f }, { 0.000521f, 0.997396f }},
    //{{  0.627084f,  8.643809f,  0.298860f }, { 0.004166f, 0.995834f }},
    //{{  1.009306f,  8.643809f, -0.592991f }, { 0.005726f, 0.999479f }},
    //{{ -0.264767f,  8.643809f,  0.935896f }, { 0.000521f, 0.993233f }},
    //{{  1.518936f,  8.643809f, -0.083362f }, { 0.007812f, 0.997396f }},
    //{{  0.117455f,  8.643809f,  1.318118f }, { 0.002083f, 0.991669f }},
    //{{  1.518936f,  8.643809f,  0.935896f }, { 0.007812f, 0.993233f }},
    //{{  1.136714f,  8.643809f,  1.318118f }, { 0.006248f, 0.991669f }},
    //};

        const VertexData_PositionTexCoord verticesUv[] = {
    {{  0.244862f,  8.643809f, -0.592991f }, { 0.002604f, 0.000521f }},
    {{ -0.264767f,  8.643809f, -0.083362f }, { 0.000521f, 0.002604f }},
    {{  0.627084f,  8.643809f,  0.298860f }, { 0.004166f, 0.004166f }},
    {{  1.009306f,  8.643809f, -0.592991f }, { 0.005726f, 0.000521f }},
    {{ -0.264767f,  8.643809f,  0.935896f }, { 0.000521f, 0.006767f }},
    {{  1.518936f,  8.643809f, -0.083362f }, { 0.007812f, 0.002604f }},
    {{  0.117455f,  8.643809f,  1.318118f }, { 0.002083f, 0.008331f }},
    {{  1.518936f,  8.643809f,  0.935896f }, { 0.007812f, 0.006767f }},
    {{  1.136714f,  8.643809f,  1.318118f }, { 0.006248f, 0.008331f }},
    };


    //staging buffer
    VulkanData vertexData_stagingBffer;
    memset((void*)&vertexData_stagingBffer, 0, sizeof(VulkanData));

    VkBufferCreateInfo vkBufferCreateInfo_stagingBuffer;
    memset((void*)&vkBufferCreateInfo_stagingBuffer, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo_stagingBuffer.pNext = NULL;
    vkBufferCreateInfo_stagingBuffer.flags = 0;
    vkBufferCreateInfo_stagingBuffer.size = sizeof(verticesUv);
    vkBufferCreateInfo_stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // staging buffer is used for transfering data to device local buffer
    vkBufferCreateInfo_stagingBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // not sharing with other queues

    // Call vkCreateBuffer() to create the staging buffer
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo_stagingBuffer, NULL, &vertexData_stagingBffer.vkBuffer);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer():  failed.\n");
        return(vkResult);
    }

    //------------
    // Get memory requirements for the staging buffer
    VkMemoryRequirements vkMemoryRequirements_stagingBuffer;
    memset((void*)&vkMemoryRequirements_stagingBuffer, 0, sizeof(vkMemoryRequirements_stagingBuffer));

    vkGetBufferMemoryRequirements(vkDevice, vertexData_stagingBffer.vkBuffer, &vkMemoryRequirements_stagingBuffer);
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
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo_stagingBuffer, NULL, &vertexData_stagingBffer.vkDeviceMemory);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  failed.\n");
        return(vkResult);
    }

    //---------------
    // Bind the staging buffer memory to the staging buffer
    vkResult = vkBindBufferMemory(vkDevice, vertexData_stagingBffer.vkBuffer, vertexData_stagingBffer.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
        return(vkResult);
    }

    //----------------
    void* data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_stagingBffer.vkDeviceMemory, 0, vkMemoryAllocateInfo_stagingBuffer.allocationSize, 0, &data);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkMapMemory() :  failed.\n");
        return(vkResult);
    }


    //-------actual memory mapped io
    memcpy(data, verticesUv, sizeof(verticesUv));
    //-------unmap memory
    vkUnmapMemory(vkDevice, vertexData_stagingBffer.vkDeviceMemory);

    //-----------------------------------------------------------------------------------

    //device buffer
    memset((void*)&vertexData_Impostor, 0, sizeof(VulkanData));
    VkBufferCreateInfo vkBufferCreateInfo_deviceBuffer;
    memset((void*)&vkBufferCreateInfo_deviceBuffer, 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo_deviceBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo_deviceBuffer.pNext = NULL;
    vkBufferCreateInfo_deviceBuffer.flags = 0;
    vkBufferCreateInfo_deviceBuffer.size = sizeof(verticesUv);
    vkBufferCreateInfo_deviceBuffer.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT; // device buffer is used for vertex buffer and transfer destination
    vkBufferCreateInfo_deviceBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // not sharing with other queues

    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo_deviceBuffer, NULL, &vertexData_Impostor.vkBuffer);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer():  failed.\n");
        return(vkResult);
    }


    //------------
    // Get memory requirements for the device local buffer
    VkMemoryRequirements vkMemoryRequirements_deviceBuffer;
    memset((void*)&vkMemoryRequirements_deviceBuffer, 0, sizeof(vkMemoryRequirements_deviceBuffer));
    vkGetBufferMemoryRequirements(vkDevice, vertexData_Impostor.vkBuffer, &vkMemoryRequirements_deviceBuffer);
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
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo_deviceBuffer, NULL, &vertexData_Impostor.vkDeviceMemory);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  failed.\n");
        return(vkResult);
    }

    //---------------
    // Bind the device local buffer memory to the device local buffer
    vkResult = vkBindBufferMemory(vkDevice, vertexData_Impostor.vkBuffer, vertexData_Impostor.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
        return(vkResult);
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


    //----------------
    // Record the command to copy data from staging buffer to device local buffer
    VkBufferCopy vkBufferCopy;
    memset((void*)&vkBufferCopy, 0, sizeof(VkBufferCopy));
    vkBufferCopy.srcOffset = 0; // offset in the source buffer
    vkBufferCopy.dstOffset = 0; // offset in the destination buffer
    vkBufferCopy.size = sizeof(verticesUv); // size of the data to copy
    vkCmdCopyBuffer(vkCommandBuffer_Copy, vertexData_stagingBffer.vkBuffer, vertexData_Impostor.vkBuffer, 1, &vkBufferCopy);

    // End command buffer recording
    vkResult = vkEndCommandBuffer(vkCommandBuffer_Copy);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkEndCommandBuffer() :  failed.\n");
        return(vkResult);
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

    // Wait for the queue to finish processing
    vkResult = vkQueueWaitIdle(vkQueue);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkQueueWaitIdle() :  failed.\n");
        return(vkResult);
    }


    //----------------
    // Free the staging buffer
    if (vertexData_stagingBffer.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_stagingBffer.vkBuffer, NULL);
        vertexData_stagingBffer.vkBuffer = VK_NULL_HANDLE;
    }


    // Free the staging buffer memory
    if (vertexData_stagingBffer.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_stagingBffer.vkDeviceMemory, NULL);
        vertexData_stagingBffer.vkDeviceMemory = VK_NULL_HANDLE;
    }

    //-----------------------------------------------------------------------------------
    // Now, vertexData_position.vkBuffer contains the device local buffer with the triangle position data
    // and vertexData_position.vkDeviceMemory contains the device local buffer memory.

    if (vkCommandBuffer_Copy)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_Copy);
        vkCommandBuffer_Copy = VK_NULL_HANDLE;
    }




    //position index buffer
    //----------------------------------------------------------------------------------------------------
    memset((void*)&vertexData_Impostor_index, 0, sizeof(VulkanData));

    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(impostor_indices);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_Impostor_index.vkBuffer);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkCreateBuffer() (index):  failed.\n");
        return(vkResult);
    }

    //------------
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(vkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, vertexData_Impostor_index.vkBuffer, &vkMemoryRequirements);

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
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_Impostor_index.vkDeviceMemory);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkAllocateMemory() :  failed.\n");
        return(vkResult);
    }


    //---------------
    vkResult = vkBindBufferMemory(vkDevice, vertexData_Impostor_index.vkBuffer, vertexData_Impostor_index.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkBindBufferMemory() :  failed.\n");
        return(vkResult);
    }


    //----------------
    void* dataIndex = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_Impostor_index.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &dataIndex);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createVertexBuffer() -> vkMapMemory() :  failed.\n");
        return(vkResult);
    }


    //-------actual memory mapped io
    memcpy(dataIndex, impostor_indices, sizeof(impostor_indices));

    //-------unmap memory
    vkUnmapMemory(vkDevice, vertexData_Impostor_index.vkDeviceMemory);

    return vkResult;
}
*/
//VkResult createDiscriptorSet(void)
//{
//    // local variables
//    VkResult vkResult = VK_SUCCESS;
//
//    // code
//    // Declare and initialize VkDescriptorSetAllocateInfo structure.
//    VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
//    memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
//
//    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//    vkDescriptorSetAllocateInfo.pNext = NULL;
//    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool; // global descriptor pool
//    vkDescriptorSetAllocateInfo.descriptorSetCount = 1; // we are allocating only one descriptor set
//    vkDescriptorSetAllocateInfo.pSetLayouts = &vkDescriptorSetLayout; // pointer to the descriptor set layout
//
//    // Call vkAllocateDescriptorSets() to allocate the descriptor set
//    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet);
//
//    if (vkResult != VK_SUCCESS)
//    {
//        fprintf(gpFILE, "createDiscriptorSet() : vkAllocateDescriptorSets() failed.\n");
//        return(vkResult);
//    }
//    else
//    {
//        fprintf(gpFILE, "createDiscriptorSet() : vkAllocateDescriptorSets() succeeded.\n");
//    }
//
//    return(vkResult);
//}

VkResult createUniformBuffer(void)
{
    //VkResult updateUniformBuffer_camera(uint32_t frameIndex);   
    VkResult updateUniformBuffer_frameData(uint32_t frameIndex);

    VkResult vkResult = VK_SUCCESS;

    ////UniformBufferObject_camera 
 //   for (uint32_t k = 0; k < MAX_FRAMES; k++)
 //   {
 //       //code
 //       VkBufferCreateInfo vkBufferCreateInfo;
 //       memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

 //       vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
 //       vkBufferCreateInfo.pNext = NULL;
 //       vkBufferCreateInfo.flags = 0;
 //       vkBufferCreateInfo.size = sizeof(UniformBufferObject_camera);
 //       vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

 //       memset((void*)&uniformBufferData_camera[k], 0, sizeof(UniformData));

 //       vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &uniformBufferData_camera[k].vkBuffer);

 //       if (vkResult != VK_SUCCESS)
 //       {
 //           fprintf(gpFILE, "createUniformBuffer() -> vkCreateBuffer():  failed.\n");
 //           return(vkResult);
 //       }

 //       //------------
 //       VkMemoryRequirements vkMemoryRequirements;
 //       memset((void*)&vkMemoryRequirements, 0, sizeof(vkMemoryRequirements));

 //       vkGetBufferMemoryRequirements(vkDevice, uniformBufferData_camera[k].vkBuffer, &vkMemoryRequirements);

 //       //------------
 //       VkMemoryAllocateInfo vkMemoryAllocateInfo;
 //       memset((void*)&vkMemoryAllocateInfo, 0, sizeof(vkMemoryAllocateInfo));

 //       vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
 //       vkMemoryAllocateInfo.pNext = NULL;
 //       vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
 //       vkMemoryAllocateInfo.memoryTypeIndex = 0;

 //       //-------------
 //       for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
 //       {
 //           if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
 //           {
    //			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) // host visible and coherent memory (no need to manage vulkan cache for flushing or mapping)
    //			{
    //				// If the memory type is suitable, set the memoryTypeIndex and break the loop
 //                   vkMemoryAllocateInfo.memoryTypeIndex = i;
 //                   break;
 //               }
 //           }

 //           vkMemoryRequirements.memoryTypeBits >>= 1;
 //       }

 //       //--------------
 //       vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformBufferData_camera[k].vkDeviceMemory);
 //       if (vkResult != VK_SUCCESS)
 //       {
 //           fprintf(gpFILE, "createUniformBuffer() -> vkAllocateMemory() :  failed.\n");
 //           return(vkResult);
 //       }


 //       //---------------
 //       vkResult = vkBindBufferMemory(vkDevice, uniformBufferData_camera[k].vkBuffer, uniformBufferData_camera[k].vkDeviceMemory, 0);
 //       if (vkResult != VK_SUCCESS)
 //       {
 //           fprintf(gpFILE, "createUniformBuffer() -> vkBindBufferMemory() :  failed.\n");
 //           return(vkResult);
 //       }

 //       // Map the uniform buffer memory

 //       vkResult = vkMapMemory(vkDevice, uniformBufferData_camera[k].vkDeviceMemory, 0, sizeof(UniformBufferObject_camera), 0, &uniformBufferData_camera[k].pData);
 //       if (vkResult != VK_SUCCESS)
 //       {
 //           fprintf(gpFILE, "createUniformBuffer() -> vkMapMemory() :  failed.\n");
 //           return(vkResult);
 //       }

 //       //call updateUniformBuffer() to update the uniform buffer with initial data
 //       vkResult = updateUniformBuffer_camera(k);
 //       if (vkResult != VK_SUCCESS)
 //       {
 //           fprintf(gpFILE, "createUniformBuffer() -> updateUniformBuffer() :  failed.\n");
 //           return(vkResult);
 //       }
 //   }

    //UniformBufferObject_frameData
    for (uint32_t k = 0; k < MAX_FRAMES; k++)
    {
        //code
        VkBufferCreateInfo vkBufferCreateInfo;
        memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

        vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        vkBufferCreateInfo.pNext = NULL;
        vkBufferCreateInfo.flags = 0;
        vkBufferCreateInfo.size = sizeof(UniformBufferObject_FrameData);
        vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        memset((void*)&uniformBufferData_frameData[k], 0, sizeof(UniformData));

        vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &uniformBufferData_frameData[k].vkBuffer);

        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createUniformBuffer() -> vkCreateBuffer():  failed.\n");
            return(vkResult);
        }

        //------------
        VkMemoryRequirements vkMemoryRequirements;
        memset((void*)&vkMemoryRequirements, 0, sizeof(vkMemoryRequirements));

        vkGetBufferMemoryRequirements(vkDevice, uniformBufferData_frameData[k].vkBuffer, &vkMemoryRequirements);

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
                if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) // host visible and coherent memory (no need to manage vulkan cache for flushing or mapping)
                {
                    // If the memory type is suitable, set the memoryTypeIndex and break the loop
                    vkMemoryAllocateInfo.memoryTypeIndex = i;
                    break;
                }
            }

            vkMemoryRequirements.memoryTypeBits >>= 1;
        }

        //--------------
        vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformBufferData_frameData[k].vkDeviceMemory);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createUniformBuffer() -> vkAllocateMemory() :  failed.\n");
            return(vkResult);
        }


        //---------------
        vkResult = vkBindBufferMemory(vkDevice, uniformBufferData_frameData[k].vkBuffer, uniformBufferData_frameData[k].vkDeviceMemory, 0);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createUniformBuffer() -> vkBindBufferMemory() :  failed.\n");
            return(vkResult);
        }

        // Map the uniform buffer memory

        vkResult = vkMapMemory(vkDevice, uniformBufferData_frameData[k].vkDeviceMemory, 0, sizeof(UniformBufferObject_FrameData), 0, &uniformBufferData_frameData[k].pData);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createUniformBuffer() -> vkMapMemory() :  failed.\n");
            return(vkResult);
        }

        //call updateUniformBuffer() to update the uniform buffer with initial data
        vkResult = updateUniformBuffer_frameData(k);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createUniformBuffer() -> updateUniformBuffer() :  failed.\n");
            return(vkResult);
        }
    }


    return vkResult;
}

//VkResult updateUniformBuffer_camera(uint32_t curIndex)
//{
//    // local variables
//    VkResult vkResult = VK_SUCCESS;
//
//    // code
//    UniformBufferObject_camera uniformTransformBufferObject_camera;
//    memset((void*)&uniformTransformBufferObject_camera, 0, sizeof(UniformBufferObject_camera));
//    // Initialize the uniform buffer object with some data
//
//    //uniformTransformBufferObject.model = glm::mat4(1.0f); // identity matrix
//
//    uniformTransformBufferObject_camera.view = camera.GetViewMatrix(); // identity matrix
//
//
//    //glm::mat4 ortho = glm::mat4(1.0f);
//
//    //if (winWidth <=  winHeight )
//    //{
//    //	ortho = glm::ortho(-100.0f, 100.0f, 100.0f * (float)winHeight / (float)winWidth, -100.0f * (float)winHeight / (float)winWidth, -100.0f, 100.0f);
//    //}
//    //else
//    //{
//    //	ortho = glm::ortho(-100.0f * (float)winWidth / (float)winHeight, 100.0f * (float)winWidth / (float)winHeight, 100.0f, -100.0f, -100.0f, 100.0f);
//    //}
//
//    uniformTransformBufferObject_camera.proj = MyWin32::gProjectionMatrix; //  projection matrix
//
//    // Copy the data to the uniform buffer
//    memcpy(uniformBufferData_camera[curIndex].pData, &uniformTransformBufferObject_camera, sizeof(UniformBufferObject_camera));
//
//    return vkResult;
//}

VkResult updateUniformBuffer_frameData(uint32_t curIndex)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    //----------------------------------------------------------------------------------------------------------

    //uniformTransformBufferObject_frameData
    UniformBufferObject_FrameData uniformTransformBufferObject_frameData;
    memset((void*)&uniformTransformBufferObject_frameData, 0, sizeof(UniformBufferObject_FrameData));
    // Initialize the uniform buffer object with some data
    uniformTransformBufferObject_frameData.fTime = 0.0f; // time in seconds
    uniformTransformBufferObject_frameData.frameID = 1; // current frame index
    uniformTransformBufferObject_frameData.view = camera.GetViewMatrix(); // view matrix
    uniformTransformBufferObject_frameData.proj = MyWin32::gProjectionMatrix; // projection matrix
    uniformTransformBufferObject_frameData.cameraPos = camera.GetCameraPos(); // camera position

    // Copy the data to the uniform buffer
    memcpy(uniformBufferData_frameData[curIndex].pData, &uniformTransformBufferObject_frameData, sizeof(UniformBufferObject_FrameData));

    return vkResult;
}

VkResult createShaderModule(VkShaderModule* shaderModule, const char* fileName)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    FILE* fp = NULL;
    size_t size = 0;

    errno_t err = fopen_s(&fp, fileName, "rb");

    if (err != 0)
    {
        fprintf(gpFILE, "createShaders() -> fopen_s() :  failed.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
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


    fclose(fp);

    VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0;
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, shaderModule);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createShaders() -> vkCreateShaderModule() :  failed.\n");
        return(vkResult);
    }

    if (shaderData)
    {
        free(shaderData);
        shaderData = NULL;
    }


    return vkResult;

}

VkResult createShaders(void)
{
    VkResult createShaderModule(VkShaderModule * shaderModule, const char* fileName);
    // local variables
    VkResult vkResult = VK_SUCCESS;

    //vkResult = createShaderModule(&vkShaderModule_basic_vs, "shader.vert.spv");
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "createShaders() -> createShaderModule() for vertex shader failed.\n");
    //    return vkResult;
    //}

    //vkResult = createShaderModule(&vkShaderModule_basic_fs, "shader.frag.spv");
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "createShaders() -> createShaderModule() for fragment shader failed.\n");
    //    return vkResult;
    //}

 //   //vkShaderModule_whiteVertex_vs
 //   vkResult = createShaderModule(&vkShaderModule_whiteVertex_vs, "whiteVertex.vert.spv");
 //   if (vkResult != VK_SUCCESS)
 //   {
 //       fprintf(gpFILE, "createShaders() -> createShaderModule() for white vertex shader failed.\n");
 //       return vkResult;
 //   }

 //   //vkShaderModule_whiteFragment_fs
 //   vkResult = createShaderModule(&vkShaderModule_whiteVertex_fs, "whiteVertex.frag.spv");
 //   if (vkResult != VK_SUCCESS)
 //   {
 //       fprintf(gpFILE, "createShaders() -> createShaderModule() for white fragment shader failed.\n");
 //       return vkResult;
 //   }

 //   //----------

 //   //vkShaderModule_previewImage_vs
 //   vkResult = createShaderModule(&vkShaderModule_previewImage_vs, "PreviewImage.vert.spv");
 //   if (vkResult != VK_SUCCESS)
 //   {
 //       fprintf(gpFILE, "createShaders() -> createShaderModule() for preview image vertex shader failed.\n");
 //       return vkResult;
 //   }

 //   //vkShaderModule_previewImage_fs
 //   vkResult = createShaderModule(&vkShaderModule_previewImage_fs, "PreviewImage.frag.spv");
 //   if (vkResult != VK_SUCCESS)
 //   {
 //       fprintf(gpFILE, "createShaders() -> createShaderModule() for preview image fragment shader failed.\n");
 //       return vkResult;
 //   }

 //   //----------

 //   //vkShaderModule_impostor_vs
 //   vkResult = createShaderModule(&vkShaderModule_impostor_vs, "Impostor.vert.spv");
 //   if (vkResult != VK_SUCCESS)
 //   {
 //       fprintf(gpFILE, "createShaders() -> createShaderModule() for impostor vertex shader failed.\n");
 //       return vkResult;
 //   }

 //   //vkShaderModule_impostor_fs
 //   vkResult = createShaderModule(&vkShaderModule_impostor_fs, "Impostor.frag.spv");
 //   if (vkResult != VK_SUCCESS)
 //   {
 //       fprintf(gpFILE, "createShaders() -> createShaderModule() for impostor fragment shader failed.\n");
 //       return vkResult;
 //   }

 //   //----------phongshader--------------
 //   vkResult = createShaderModule(&vkShaderModule_phong_vs, "Phong.vert.spv");
 //   if (vkResult != VK_SUCCESS)
 //   {
 //       fprintf(gpFILE, "createShaders() -> createShaderModule() for phong vertex shader failed.\n");
 //       return vkResult;
 //   }

 //   vkResult = createShaderModule(&vkShaderModule_phong_fs, "Phong.frag.spv");
 //   if (vkResult != VK_SUCCESS)
 //   {
 //       fprintf(gpFILE, "createShaders() -> createShaderModule() for phong fragment shader failed.\n");
 //       return vkResult;
 //   }

 //   //-----------PBR shader----------------
 //   vkResult = createShaderModule(&vkShaderModule_PBR_vs, "PBR.vert.spv");
 //   if (vkResult != VK_SUCCESS)
 //   {
 //       fprintf(gpFILE, "createShaders() -> createShaderModule() for pbr vertex shader failed.\n");
 //       return vkResult;
 //   }
 //   vkResult = createShaderModule(&vkShaderModule_PBR_fs, "PBR.frag.spv");
 //   if (vkResult != VK_SUCCESS)
 //   {
 //       fprintf(gpFILE, "createShaders() -> createShaderModule() for pbr fragment shader failed.\n");
 //       return vkResult;
 //   }

	////PBR_Skinned
	//vkResult = createShaderModule(&vkShaderModule_PBR_Skinned_vs, "PBR_Skinned.vert.spv");
 //   if (vkResult != VK_SUCCESS)
 //   {
 //       fprintf(gpFILE, "createShaders() -> createShaderModule() for pbr skinned vertex shader failed.\n");
	//	return vkResult;
 //   }
	//vkResult = createShaderModule(&vkShaderModule_PBR_Skinned_fs, "PBR_Skinned.frag.spv");
 //   if (vkResult != VK_SUCCESS)
 //   {
 //       fprintf(gpFILE, "createShaders() -> createShaderModule() for pbr skinned fragment shader failed.\n");
 //       return vkResult;
	//}


    return vkResult;
}

void destroyShaders(void)
{
    //if (vkShaderModule_basic_fs)
    //{
    //    vkDestroyShaderModule(vkDevice, vkShaderModule_basic_fs, NULL);
    //    vkShaderModule_basic_fs = VK_NULL_HANDLE;
    //}
    //if (vkShaderModule_basic_vs)
    //{
    //    vkDestroyShaderModule(vkDevice, vkShaderModule_basic_vs, NULL);
    //    vkShaderModule_basic_vs = VK_NULL_HANDLE;
    //}

 //   //-- shader modules for white vertex shaders
 //   if (vkShaderModule_whiteVertex_vs)
 //   {
 //       vkDestroyShaderModule(vkDevice, vkShaderModule_whiteVertex_vs, NULL);
 //       vkShaderModule_whiteVertex_vs = VK_NULL_HANDLE;
 //   }

 //   if (vkShaderModule_whiteVertex_fs)
 //   {
 //       vkDestroyShaderModule(vkDevice, vkShaderModule_whiteVertex_fs, NULL);
 //       vkShaderModule_whiteVertex_fs = VK_NULL_HANDLE;
 //   }

 //   //-- shader modules for previewImage shaders

 //   if (vkShaderModule_previewImage_vs)
 //   {
 //       vkDestroyShaderModule(vkDevice, vkShaderModule_previewImage_vs, NULL);
 //       vkShaderModule_previewImage_vs = VK_NULL_HANDLE;
 //   }

 //   if (vkShaderModule_previewImage_fs)
 //   {
 //       vkDestroyShaderModule(vkDevice, vkShaderModule_previewImage_fs, NULL);
 //       vkShaderModule_previewImage_fs = VK_NULL_HANDLE;
 //   }

 //   //-- shader modules for impostor shaders
 //   if (vkShaderModule_impostor_vs)
 //   {
 //       vkDestroyShaderModule(vkDevice, vkShaderModule_impostor_vs, NULL);
 //       vkShaderModule_impostor_vs = VK_NULL_HANDLE;
 //   }

 //   if (vkShaderModule_impostor_fs)
 //   {
 //       vkDestroyShaderModule(vkDevice, vkShaderModule_impostor_fs, NULL);
 //       vkShaderModule_impostor_fs = VK_NULL_HANDLE;
 //   }

 //   //-- shader modules for phong shaders
 //   if (vkShaderModule_phong_vs)
 //   {
 //       vkDestroyShaderModule(vkDevice, vkShaderModule_phong_vs, NULL);
 //       vkShaderModule_phong_vs = VK_NULL_HANDLE;
 //   }
 //   if (vkShaderModule_phong_fs)
 //   {
 //       vkDestroyShaderModule(vkDevice, vkShaderModule_phong_fs, NULL);
 //       vkShaderModule_phong_fs = VK_NULL_HANDLE;
 //   }

 //   //-- shader modules for PBR shaders
 //   if (vkShaderModule_PBR_vs)
 //   {
 //       vkDestroyShaderModule(vkDevice, vkShaderModule_PBR_vs, NULL);
 //       vkShaderModule_PBR_vs = VK_NULL_HANDLE;
 //   }
 //   if (vkShaderModule_PBR_fs)
 //   {
 //       vkDestroyShaderModule(vkDevice, vkShaderModule_PBR_fs, NULL);
 //       vkShaderModule_PBR_fs = VK_NULL_HANDLE;
 //   }

	////PBR_Skinned
 //   if (vkShaderModule_PBR_Skinned_vs)
 //   {
 //       vkDestroyShaderModule(vkDevice, vkShaderModule_PBR_Skinned_vs, NULL);
 //       vkShaderModule_PBR_Skinned_vs = VK_NULL_HANDLE;
	//}
 //   if (vkShaderModule_PBR_Skinned_fs)
 //   {
 //       vkDestroyShaderModule(vkDevice, vkShaderModule_PBR_Skinned_fs, NULL);
 //       vkShaderModule_PBR_Skinned_fs = VK_NULL_HANDLE;
	//}
}

/*
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

    fclose(fp);


    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0;
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_basic_fs);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createShaders() -> vkCreateShaderModule() :  failed.\n");
        return(vkResult);
    }

    if (shaderData)
    {
        free(shaderData);
        shaderData = NULL;
    }

    return vkResult;
}
*/


VkResult createDescriptorPool(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    // Declare and initialize VkDescriptorPoolSize structure which will have information about the descriptor pool size.
    VkDescriptorPoolSize vkDescriptorPoolSizes[] =
    {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 * MAX_FRAMES}, // descriptor type and descriptor count
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6 + 3 +3}, // descriptor type and descriptor count for combined image sampler
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 * MAX_FRAMES}, // descriptor type and descriptor count for storage buffer//bones
    };

    // Declare and initialize VkDescriptorPoolCreateInfo structure and refer above VkDescriptorPoolSize into it.
    VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;
    memset((void*)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));

    vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vkDescriptorPoolCreateInfo.pNext = NULL;
    vkDescriptorPoolCreateInfo.flags = 0; // no flags
    vkDescriptorPoolCreateInfo.maxSets = (2 * MAX_FRAMES) + 2 + 2 + 1; // maximum number of descriptor sets that can be allocated from this pool
    vkDescriptorPoolCreateInfo.poolSizeCount = _ARRAYSIZE(vkDescriptorPoolSizes); // number of descriptor pool sizes
    vkDescriptorPoolCreateInfo.pPoolSizes = vkDescriptorPoolSizes;

    // Call vkCreateDescriptorPool() to create the actual descriptor pool.
    vkResult = vkCreateDescriptorPool(vkDevice, &vkDescriptorPoolCreateInfo, NULL, &vkDescriptorPool);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createDescriptorPool() -> vkCreateDescriptorPool() :  failed: %d.\n", vkResult);
        return(vkResult);
    }

    return(vkResult);

}

VkResult createDescriptorSet_FrameData(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    // Declare and initialize VkDescriptorSetAllocateInfo structure.
    VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
    memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));

    VkDescriptorSetLayout layouts[MAX_FRAMES]{};
    for (uint32_t i = 0; i < MAX_FRAMES; i++)
        layouts[i] = gpDescriptorSetLayouts->vkDescriptorSetLayout_frameData;

    //------------------------------------------------------------------------------------------------
    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool; // global descriptor pool
    vkDescriptorSetAllocateInfo.descriptorSetCount = MAX_FRAMES;//
    vkDescriptorSetAllocateInfo.pSetLayouts = layouts; // pointer to the descriptor set layout


    // Call vkAllocateDescriptorSets() to allocate the descriptor set
    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, vkDescriptorSets_frameData);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createDiscriptorSet() : vkAllocateDescriptorSets() failed.\n");
        return(vkResult);
    }

    //UniformBufferObject_camera
    for (size_t i = 0; i < MAX_FRAMES; i++)
    {
        // Declare and initialize VkDescriptorBufferInfo structure which will have information about the uniform buffer.
        //VkDescriptorBufferInfo vkDescriptorBufferInfo_camera;
        //memset((void*)&vkDescriptorBufferInfo_camera, 0, sizeof(VkDescriptorBufferInfo));
        //vkDescriptorBufferInfo_camera.buffer = uniformBufferData_camera[i].vkBuffer; // uniform buffer
        //vkDescriptorBufferInfo_camera.offset = 0; // offset in the buffer
        //vkDescriptorBufferInfo_camera.range = sizeof(UniformBufferObject_camera); // size of the buffer

        VkDescriptorBufferInfo vkDescriptorBufferInfo_frameData;
        memset((void*)&vkDescriptorBufferInfo_frameData, 0, sizeof(VkDescriptorBufferInfo));
        vkDescriptorBufferInfo_frameData.buffer = uniformBufferData_frameData[i].vkBuffer; // uniform buffer
        vkDescriptorBufferInfo_frameData.offset = 0; // offset in the buffer
        vkDescriptorBufferInfo_frameData.range = sizeof(UniformBufferObject_FrameData); // size of the buffer

        //write or copy the descriptor set with the uniform buffer information
        // Declare and initialize VkWriteDescriptorSet structure which will have information about the descriptor set.
        VkWriteDescriptorSet vkWriteDescriptorSet_array[1];
        memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

        vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkWriteDescriptorSet_array[0].pNext = NULL;
        vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSets_frameData[i]; // descriptor set
        vkWriteDescriptorSet_array[0].dstBinding = 0; // 0 means the index number of the binding
        vkWriteDescriptorSet_array[0].dstArrayElement = 0; // 0 means the index number of the array element
        vkWriteDescriptorSet_array[0].descriptorCount = 1; // we are using only one descriptor, more incase of array
        vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // type of the descriptor
        vkWriteDescriptorSet_array[0].pImageInfo = NULL; // no image info
        vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo_frameData; // pointer to the buffer info
        vkWriteDescriptorSet_array[0].pTexelBufferView = NULL; // no texel buffer view

        //vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //vkWriteDescriptorSet_array[1].pNext = NULL;
        //vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSets[i]; // descriptor set
        //vkWriteDescriptorSet_array[1].dstBinding = 1; // 1 means the index number of the binding
        //vkWriteDescriptorSet_array[1].dstArrayElement = 0; // 0 means the index number of the array element
        //vkWriteDescriptorSet_array[1].descriptorCount = 1; // we are using only one descriptor
        //vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // type of the descriptor
        //vkWriteDescriptorSet_array[1].pImageInfo = NULL; // no image info
        //vkWriteDescriptorSet_array[1].pBufferInfo = &vkDescriptorBufferInfo_frameData; // pointer to the buffer info
        //vkWriteDescriptorSet_array[1].pTexelBufferView = NULL; // no texel buffer view


        // Call vkUpdateDescriptorSets() to update the descriptor set with the uniform buffer information.
        vkUpdateDescriptorSets(vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);

    }


    return(vkResult);
}

VkResult createDescriptorSet_FrameDataBoneData(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    // Declare and initialize VkDescriptorSetAllocateInfo structure.
    VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
    memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));

    VkDescriptorSetLayout layouts[MAX_FRAMES]{};
    for (uint32_t i = 0; i < MAX_FRAMES; i++)
        layouts[i] = gpDescriptorSetLayouts->vkDescriptorSetLayout_frameDataBoneData;

    //------------------------------------------------------------------------------------------------
    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool; // global descriptor pool
    vkDescriptorSetAllocateInfo.descriptorSetCount = MAX_FRAMES;//
    vkDescriptorSetAllocateInfo.pSetLayouts = layouts; // pointer to the descriptor set layout


    // Call vkAllocateDescriptorSets() to allocate the descriptor set
    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, vkDescriptorSets_frameDataBoneData);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createDiscriptorSet() : vkAllocateDescriptorSets() failed.\n");
        return(vkResult);
    }

    //UniformBufferObject_camera
    for (size_t i = 0; i < MAX_FRAMES; i++)
    {
        // Declare and initialize VkDescriptorBufferInfo structure which will have information about the uniform buffer.
        //VkDescriptorBufferInfo vkDescriptorBufferInfo_camera;
        //memset((void*)&vkDescriptorBufferInfo_camera, 0, sizeof(VkDescriptorBufferInfo));
        //vkDescriptorBufferInfo_camera.buffer = uniformBufferData_camera[i].vkBuffer; // uniform buffer
        //vkDescriptorBufferInfo_camera.offset = 0; // offset in the buffer
        //vkDescriptorBufferInfo_camera.range = sizeof(UniformBufferObject_camera); // size of the buffer

        VkDescriptorBufferInfo vkDescriptorBufferInfo_frameData;
        memset((void*)&vkDescriptorBufferInfo_frameData, 0, sizeof(VkDescriptorBufferInfo));
        vkDescriptorBufferInfo_frameData.buffer = uniformBufferData_frameData[i].vkBuffer; // uniform buffer
        vkDescriptorBufferInfo_frameData.offset = 0; // offset in the buffer
        vkDescriptorBufferInfo_frameData.range = sizeof(UniformBufferObject_FrameData); // size of the buffer

		VkDescriptorBufferInfo vkDescriptorBufferInfo_boneData;
		memset((void*)&vkDescriptorBufferInfo_boneData, 0, sizeof(VkDescriptorBufferInfo));
		vkDescriptorBufferInfo_boneData.buffer = pModel_Rat->GetBoneSSBOs()[i].vkBuffer; // uniform buffer
		vkDescriptorBufferInfo_boneData.offset = 0; // offset in the buffer
		vkDescriptorBufferInfo_boneData.range = pModel_Rat->GetBoneSSBOs()[i].size; // size of the buffer

        //write or copy the descriptor set with the uniform buffer information
        // Declare and initialize VkWriteDescriptorSet structure which will have information about the descriptor set.
        VkWriteDescriptorSet vkWriteDescriptorSet_array[2];
        memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

        vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkWriteDescriptorSet_array[0].pNext = NULL;
        vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSets_frameDataBoneData[i]; // descriptor set
        vkWriteDescriptorSet_array[0].dstBinding = 0; // 0 means the index number of the binding
        vkWriteDescriptorSet_array[0].dstArrayElement = 0; // 0 means the index number of the array element
        vkWriteDescriptorSet_array[0].descriptorCount = 1; // we are using only one descriptor, more incase of array
        vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // type of the descriptor
        vkWriteDescriptorSet_array[0].pImageInfo = NULL; // no image info
        vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo_frameData; // pointer to the buffer info
        vkWriteDescriptorSet_array[0].pTexelBufferView = NULL; // no texel buffer view

		vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		vkWriteDescriptorSet_array[1].pNext = NULL;
		vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSets_frameDataBoneData[i]; // descriptor set
		vkWriteDescriptorSet_array[1].dstBinding = 1; // 1 means the index number of the binding
		vkWriteDescriptorSet_array[1].dstArrayElement = 0; // 0 means the index number of the array element
		vkWriteDescriptorSet_array[1].descriptorCount = 1; // we are using only one descriptor
		vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // type of the descriptor
		vkWriteDescriptorSet_array[1].pImageInfo = NULL; // no image info
		vkWriteDescriptorSet_array[1].pBufferInfo = &vkDescriptorBufferInfo_boneData; // pointer to the buffer info
		vkWriteDescriptorSet_array[1].pTexelBufferView = NULL; // no texel buffer view


        // Call vkUpdateDescriptorSets() to update the descriptor set with the uniform buffer information.
        vkUpdateDescriptorSets(vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);

    }


    return(vkResult);
}

VkResult createDescriptorSet_SingleImage(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    // Declare and initialize VkDescriptorSetAllocateInfo structure.
    VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
    memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));

    //VkDescriptorSetLayout layouts[MAX_FRAMES]{};
    //for (uint32_t i = 0; i < MAX_FRAMES; i++)
    //    layouts[i] = vkDescriptorSetLayout_CamImage;

    //------------------------------------------------------------------------------------------------
    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool; // global descriptor pool
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1;//
    vkDescriptorSetAllocateInfo.pSetLayouts = &gpDescriptorSetLayouts->vkDescriptorSetLayout_SingleImage; // pointer to the descriptor set layout


    // Call vkAllocateDescriptorSets() to allocate the descriptor set
    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_SingleImage);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createDiscriptorSet() : vkAllocateDescriptorSets() failed.\n");
        return(vkResult);
    }

    //UniformBufferObject_frameData
    //for (size_t i = 0; i < 1; i++)//MAX_FRAMES
    {
        // Declare and initialize VkDescriptorBufferInfo structure which will have information about the uniform buffer.
        //VkDescriptorBufferInfo vkDescriptorBufferInfo_FrameData;
        //memset((void*)&vkDescriptorBufferInfo_FrameData, 0, sizeof(VkDescriptorBufferInfo));
        //vkDescriptorBufferInfo_FrameData.buffer = uniformBufferData_frameData[i].vkBuffer; // uniform buffer
        //vkDescriptorBufferInfo_FrameData.offset = 0; // offset in the buffer
        //vkDescriptorBufferInfo_FrameData.range = sizeof(UniformBufferObject_FrameData); // size of the buffer

        //Declare and initialize VkDescriptorImageInfo structure which will have information about the image.
        VkDescriptorImageInfo vkDescriptorImageInfo;
        memset((void*)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));
        vkDescriptorImageInfo.sampler = vkSampler_LinearClamp; // sampler for the image
        vkDescriptorImageInfo.imageView = grassTextureData.vkImageView; // image view for the image
        vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // image layout for the image

        //write or copy the descriptor set with the uniform buffer information
        // Declare and initialize VkWriteDescriptorSet structure which will have information about the descriptor set.
        VkWriteDescriptorSet vkWriteDescriptorSet_array[1];
        memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

        vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vkWriteDescriptorSet_array[0].pNext = NULL;
        vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet_SingleImage; // descriptor set
        vkWriteDescriptorSet_array[0].dstBinding = 0; // 0 means the index number of the binding
        vkWriteDescriptorSet_array[0].dstArrayElement = 0; // 0 means the index number of the array element
        vkWriteDescriptorSet_array[0].descriptorCount = 1; // we are using only one descriptor
        vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // type of the descriptor
        vkWriteDescriptorSet_array[0].pImageInfo = &vkDescriptorImageInfo;
        vkWriteDescriptorSet_array[0].pBufferInfo = NULL; // pointer to the buffer info
        vkWriteDescriptorSet_array[0].pTexelBufferView = NULL; // no texel buffer view

        //vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        //vkWriteDescriptorSet_array[1].pNext = NULL;
        //vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSets_camImage[i]; // descriptor set
        //vkWriteDescriptorSet_array[1].dstBinding = 1; // 1 means the index number of the binding
        //vkWriteDescriptorSet_array[1].dstArrayElement = 0; // 0 means the index number of the array element
        //vkWriteDescriptorSet_array[1].descriptorCount = 1; // we are using only one descriptor
        //vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // type of the descriptor
        //vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo; // pointer to the image info
        //vkWriteDescriptorSet_array[1].pBufferInfo = NULL; // no buffer info
        //vkWriteDescriptorSet_array[1].pTexelBufferView = NULL; // no texel buffer view


        // Call vkUpdateDescriptorSets() to update the descriptor set with the uniform buffer information.
        vkUpdateDescriptorSets(vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);

    }


    return(vkResult);
}

VkResult createDescriptorSet_AlbedoNormal(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    // Declare and initialize VkDescriptorSetAllocateInfo structure.
    VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
    memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));

    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool; // global descriptor pool
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1;//
    vkDescriptorSetAllocateInfo.pSetLayouts = &gpDescriptorSetLayouts->vkDescriptorSetLayout_AlbedoNormal; // pointer to the descriptor set layout

    // Call vkAllocateDescriptorSets() to allocate the descriptor set
    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_AlbedoNormal);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createDiscriptorSet() : vkAllocateDescriptorSets() failed.\n");
        return(vkResult);
    }
    // Declare and initialize VkDescriptorImageInfo structure which will have information about the albedo image.
    VkDescriptorImageInfo vkDescriptorImageInfo_Albedo;
    memset((void*)&vkDescriptorImageInfo_Albedo, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo_Albedo.sampler = vkSampler_LinearClamp; // sampler for the albedo image
    vkDescriptorImageInfo_Albedo.imageView = imposterTextureData_blackAlder_Field_02_Albedo.vkImageView; // image view for the albedo image
    vkDescriptorImageInfo_Albedo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // image layout for the albedo image
    // Declare and initialize VkDescriptorImageInfo structure which will have information about the normal image.
    VkDescriptorImageInfo vkDescriptorImageInfo_Normal;
    memset((void*)&vkDescriptorImageInfo_Normal, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo_Normal.sampler = vkSampler_LinearMipmapClamp; // sampler for the normal image
    vkDescriptorImageInfo_Normal.imageView = imposterTextureData_blackAlder_Field_02_Normal.vkImageView; // image view for the normal image
    vkDescriptorImageInfo_Normal.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // image layout for the normal image

    //write or copy the descriptor set with the albedo and normal image information
    // Declare and initialize VkWriteDescriptorSet structure which will have information about the descriptor set.
    VkWriteDescriptorSet vkWriteDescriptorSet_array[2];
    memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));
    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].pNext = NULL;
    vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet_AlbedoNormal; // descriptor set
    vkWriteDescriptorSet_array[0].dstBinding = 0; // 0 means the index number of the binding
    vkWriteDescriptorSet_array[0].dstArrayElement = 0; // 0 means the index number of the array element
    vkWriteDescriptorSet_array[0].descriptorCount = 1; // we are using only one descriptor
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // type of the descriptor
    vkWriteDescriptorSet_array[0].pImageInfo = &vkDescriptorImageInfo_Albedo; // pointer to the albedo image info
    vkWriteDescriptorSet_array[0].pBufferInfo = NULL; // no buffer info
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL; // no texel buffer view

    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].pNext = NULL;
    vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSet_AlbedoNormal; // descriptor set
    vkWriteDescriptorSet_array[1].dstBinding = 1; // 1 means the index number of the binding
    vkWriteDescriptorSet_array[1].dstArrayElement = 0; // 0 means the index number of the array element
    vkWriteDescriptorSet_array[1].descriptorCount = 1; // we are using only one descriptor
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // type of the descriptor
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo_Normal; // pointer to the normal image info
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL; // no buffer info
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL; // no tex


    // Call vkUpdateDescriptorSets() to update the descriptor set with the albedo and normal image information.
    vkUpdateDescriptorSets(vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);

    return vkResult;
}

VkResult getSupportedDepthFormat(VkFormat* pVkFormat)
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
        vkGetPhysicalDeviceFormatProperties(vkPhysicalDevice_Selected, formats[i], &formatProperties);
        if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            *pVkFormat = formats[i];
            return VK_SUCCESS;
        }
    }


    return vkResult;
}

VkResult createDepthResources(void)
{
    VkResult vkResult = VK_SUCCESS;

    // Find the depth format supported by the device
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    vkResult = getSupportedDepthFormat(&depthFormat);
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
    imageInfo.extent.width = vkExtent2D_Swapchain.width;
    imageInfo.extent.height = vkExtent2D_Swapchain.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = depthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    gSwapChainResourceData.imageData_depthBuffer = (ImageData*)malloc(sizeof(ImageData) * MAX_FRAMES);

    for (uint32_t i = 0; i < MAX_FRAMES; i++)
    {
        vkResult = vkCreateImage(vkDevice, &imageInfo, NULL, &gSwapChainResourceData.imageData_depthBuffer[i].vkImage);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "Failed to create depth image.\n");
            return vkResult;
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(vkDevice, gSwapChainResourceData.imageData_depthBuffer[i].vkImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo;
        memset(&allocInfo, 0, sizeof(VkMemoryAllocateInfo));

        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(
            memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        vkResult = vkAllocateMemory(vkDevice, &allocInfo, NULL, &gSwapChainResourceData.imageData_depthBuffer[i].vkDeviceMemory);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "Failed to allocate depth image memory.\n");
            return vkResult;
        }


        vkBindImageMemory(vkDevice, gSwapChainResourceData.imageData_depthBuffer[i].vkImage, gSwapChainResourceData.imageData_depthBuffer[i].vkDeviceMemory, 0);


        //Image View 
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = gSwapChainResourceData.imageData_depthBuffer[i].vkImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_D32_SFLOAT;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        vkResult = vkCreateImageView(vkDevice, &viewInfo, nullptr, &gSwapChainResourceData.imageData_depthBuffer[i].vkImageView);
        if (vkResult != VK_SUCCESS) {
            fprintf(gpFILE, "Failed to create depth image view.\n");
            return vkResult;
        }
    }


    return vkResult;
}

VkResult createSwapchainResources(void)
{
    VkResult vkResult = VK_SUCCESS;
    //image and image view
    vkResult = createImagesAndImageViews();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createSwapchainResources() -> createImagesAndImageViews() failed.\n");
        return vkResult;
    }

    //depth resources
    vkResult = createDepthResources();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createSwapchainResources() -> createDepthResources() failed.\n");
        return vkResult;
    }

    return vkResult;
}

void destroySwapchainResources(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;
    // code
    //Swapchain images and image views-------------------------------------

        // destroy swapchain images and image views
    for (uint32_t i = 0; i < gSwapchainImageCount; i++)
    {
        vkDestroyImageView(vkDevice, gSwapChainResourceData.swapchainImageView_Array[i], NULL);
        gSwapChainResourceData.swapchainImageView_Array[i] = VK_NULL_HANDLE;

        //vkDestroyImage(vkDevice, swapchainImage_Array[i], NULL);// Not needed, as images are managed by the swapchain  
        gSwapChainResourceData.swapchainImage_Array[i] = VK_NULL_HANDLE;

    }

    if (gSwapChainResourceData.swapchainImageView_Array)
    {
        free(gSwapChainResourceData.swapchainImageView_Array);
        gSwapChainResourceData.swapchainImageView_Array = NULL;
    }
    if (gSwapChainResourceData.swapchainImage_Array)
    {
        free(gSwapChainResourceData.swapchainImage_Array);
        gSwapChainResourceData.swapchainImage_Array = NULL;
    }

    //destroy depth resources
    for (uint32_t i = 0; i < MAX_FRAMES; i++)
    {
        //image view
        if (gSwapChainResourceData.imageData_depthBuffer[i].vkImageView)
        {
            vkDestroyImageView(vkDevice, gSwapChainResourceData.imageData_depthBuffer[i].vkImageView, NULL);
            gSwapChainResourceData.imageData_depthBuffer[i].vkImageView = VK_NULL_HANDLE;
        }
        //image
        if (gSwapChainResourceData.imageData_depthBuffer[i].vkImage)
        {
            vkDestroyImage(vkDevice, gSwapChainResourceData.imageData_depthBuffer[i].vkImage, NULL);
            gSwapChainResourceData.imageData_depthBuffer[i].vkImage = VK_NULL_HANDLE;
        }
        // free depth buffer memory
        if (gSwapChainResourceData.imageData_depthBuffer[i].vkDeviceMemory)
        {
            vkFreeMemory(vkDevice, gSwapChainResourceData.imageData_depthBuffer[i].vkDeviceMemory, NULL);
            gSwapChainResourceData.imageData_depthBuffer[i].vkDeviceMemory = VK_NULL_HANDLE;
        }
    }

    if (gSwapChainResourceData.imageData_depthBuffer)
    {
        free(gSwapChainResourceData.imageData_depthBuffer);
        gSwapChainResourceData.imageData_depthBuffer = NULL;
    }

    //---------------------------------------------------------------------
}

//VkResult createRenderPass(void)
//{
//    // local variables
//    VkResult vkResult = VK_SUCCESS;
//
//    // code
//    VkAttachmentDescription colorAttachment[1];
//    memset((void*)&colorAttachment, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(colorAttachment));
//    colorAttachment[0].format = vkFormat_Color;
//    colorAttachment[0].samples = VK_SAMPLE_COUNT_1_BIT;
//    colorAttachment[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//    colorAttachment[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//    colorAttachment[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//    colorAttachment[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//    colorAttachment[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//    colorAttachment[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//
//    VkAttachmentDescription depthAttachment;
//    memset((void*)&depthAttachment, 0, sizeof(VkAttachmentDescription));
//    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
//    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//
//    VkAttachmentReference colorRef;
//    memset((void*)&colorRef, 0, sizeof(VkAttachmentReference));
//    colorRef.attachment = 0;
//    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//    VkAttachmentReference depthRef;
//    memset((void*)&depthRef, 0, sizeof(VkAttachmentReference));
//    depthRef.attachment = 1;
//    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//    // sub-step (3) : Declare and initialize VkSubPassDescription structure and keep reference about VkAttachmentReference structure.
//    VkSubpassDescription vkSubpassDescription;
//    memset((void*)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));
//
//    vkSubpassDescription.flags = 0;
//    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//    vkSubpassDescription.inputAttachmentCount = 0;
//    vkSubpassDescription.pInputAttachments = NULL;
//    vkSubpassDescription.colorAttachmentCount = _ARRAYSIZE(colorAttachment);
//    vkSubpassDescription.pColorAttachments = &colorRef; // pointer to the color attachment reference
//    vkSubpassDescription.pResolveAttachments = NULL;
//    vkSubpassDescription.pDepthStencilAttachment = &depthRef; // pointer to the depth attachment reference
//    vkSubpassDescription.preserveAttachmentCount = 0;
//    vkSubpassDescription.pPreserveAttachments = NULL;
//
//    ////----------dependancy-----------------------
//
//    //// after setting up vkSubpassDescription …
//    //VkSubpassDependency dependency = {};
//    //dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//    //dependency.dstSubpass = 0;
//    //dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//    //dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//    //dependency.srcAccessMask = 0;
//    //dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//
//    ////-------------------------------------------
//
//
//    // sub-step (4) : Declare and initialize VkRenderPassCreateInfo structure and refer above VkAttachmentDescription and VkSubPassDescription into it. Remember: here also we need attachment information in the form of image views which will be used by framebuffer later. We also need to specify inter-dependency between subpasses if needed.
//
//    VkAttachmentDescription vkAttachmentDescription_Array[2] = { colorAttachment[0], depthAttachment };
//
//    VkRenderPassCreateInfo vkRenderPassCreateInfo;
//    memset((void*)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));
//
//    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//    vkRenderPassCreateInfo.pNext = NULL;
//    vkRenderPassCreateInfo.attachmentCount = 2; // we are using two attachments: color and depth
//    vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_Array; // pointer to the attachment description array
//    vkRenderPassCreateInfo.subpassCount = 1;
//    vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
//    vkRenderPassCreateInfo.dependencyCount = 0;
//    vkRenderPassCreateInfo.pDependencies = NULL;
//    //vkRenderPassCreateInfo.dependencyCount = 1;
//    //vkRenderPassCreateInfo.pDependencies = &dependency;
//
//    // sub-step (5) : Now call vkCreateRenderPass() API to create the actual RenderPass.
//    vkResult = vkCreateRenderPass(vkDevice, &vkRenderPassCreateInfo, NULL, &vkRenderPass);
//
//    if (vkResult != VK_SUCCESS)
//    {
//        fprintf(gpFILE, "createRenderPass() : vkCreateRenderPass() failed.\n");
//        return(vkResult);
//    }
//
//    return(vkResult);
//}

//VkResult createFramebuffers(void)
//{
//    // local variables
//    VkResult vkResult = VK_SUCCESS;
//
//    VkImageView attachments[2];
//    memset((void*)attachments, 0, sizeof(VkImageView) * _ARRAYSIZE(attachments));
//
//    //attachments[1] = vkImageView_Depth; // depth attachment
//
//    //  Declare and initialize VkFramebufferCreateInfo structure.
//    VkFramebufferCreateInfo vkFramebufferCreateInfo;
//    memset((void*)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));
//
//    //  Allocate the framebuffer array by malloc() equal to the size of swapchain image count.
//    vkFramebuffer_Array = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * gSwapchainImageCount);
//
//    //  Start a loop for swapchain image count and call vkCreateFramebuffer() API to create framebuffers.
//    for (uint32_t i = 0; i < gSwapchainImageCount; i++)
//    {
//
//        attachments[0] = gSwapChainResourceData.swapchainImageView_Array[i]; // color attachment
//        attachments[1] = gSwapChainResourceData.imageData_depthBuffer[i].vkImageView; // depth attachment
//
//        vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//        vkFramebufferCreateInfo.pNext = NULL;
//        vkFramebufferCreateInfo.flags = 0;
//        vkFramebufferCreateInfo.renderPass = vkRenderPass;
//        vkFramebufferCreateInfo.attachmentCount = 2; // color and depth attachments
//        vkFramebufferCreateInfo.pAttachments = attachments; // array of attachments 
//        vkFramebufferCreateInfo.width = vkExtent2D_Swapchain.width;
//        vkFramebufferCreateInfo.height = vkExtent2D_Swapchain.height;
//        vkFramebufferCreateInfo.layers = 1;
//
//        vkResult = vkCreateFramebuffer(vkDevice, &vkFramebufferCreateInfo, NULL, &vkFramebuffer_Array[i]);
//        if (vkResult != VK_SUCCESS)
//        {
//            fprintf(gpFILE, "createFramebuffers() : vkCreateFramebuffer() failed for iteration %d.\n", i);
//            return(vkResult);
//        }
//
//    }
//
//    return(vkResult);
//}

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

    vkSemaphore_BackBuffer = (VkSemaphore*)malloc(sizeof(VkSemaphore) * gSwapchainImageCount);
    vkSemaphore_RenderComplete = (VkSemaphore*)malloc(sizeof(VkSemaphore) * gSwapchainImageCount);


    for (uint32_t i = 0; i < gSwapchainImageCount; i++)
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

    }


	//timeline semophores

	VkSemaphoreTypeCreateInfo timelineSemaphoreCreateInfo;
	memset(&timelineSemaphoreCreateInfo, 0, sizeof(VkSemaphoreTypeCreateInfo));

	timelineSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
	timelineSemaphoreCreateInfo.pNext = NULL;
	timelineSemaphoreCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
	timelineSemaphoreCreateInfo.initialValue = 0;


    //vkSemaphore_timeline
	VkSemaphoreCreateInfo vkSemaphoreCreateInfo_Timeline;
	memset(&vkSemaphoreCreateInfo_Timeline, 0, sizeof(VkSemaphoreCreateInfo));
	vkSemaphoreCreateInfo_Timeline.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkSemaphoreCreateInfo_Timeline.pNext = &timelineSemaphoreCreateInfo;
	vkSemaphoreCreateInfo_Timeline.flags = 0;
	vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo_Timeline, NULL, &vkSemaphore_Timeline);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createSemaphores() : vkCreateSemaphore() failed for timeline semaphore.\n");
		return(vkResult);
	}


    return(vkResult);
}

//VkResult createFences(void)
//{
//    // local variables
//    VkResult vkResult = VK_SUCCESS;
//
//    // code
//    //  In CreateFences() user-defined function, declare, memset() and initialize VkFenceCreateInfo structure.
//    VkFenceCreateInfo vkFenceCreateInfo;
//    memset((void*)&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));
//
//    vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//    vkFenceCreateInfo.pNext = NULL;
//    vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
//
//    //  In this function, allocate our global fence array to the size of the swapchain image count using malloc().
//    vkFence_Array = (VkFence*)malloc(sizeof(VkFence) * gSwapchainImageCount); // for the sake of brevity, we are avoiding error checking for malloc()
//
//    //  Now in a loop, call vkCreateFence() to initialize our global fences array.
//    for (uint32_t i = 0; i < gSwapchainImageCount; i++)
//    {
//        vkResult = vkCreateFence(vkDevice, &vkFenceCreateInfo, NULL, &vkFence_Array[i]);
//        if (vkResult != VK_SUCCESS)
//        {
//            fprintf(gpFILE, "createFences() : vkCreateFence() failed for iteration %d.\n", i);
//            return(vkResult);
//        }
//    }
//
//    return(vkResult);
//}

#ifdef IMGUI_ENABLE

VkResult RenderImGui(uint32_t curIndex)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // ImGui per-frame setup
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // UI
    //  Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        //static int counter = 0;
        //static bool enableFirst = false;
        //static bool enableSecond = false;

        ImGui::Begin("Basic Settings");                          // Create a window called "Hello, world!" and append into it.

        //ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        //ImGui::Checkbox("Demo Window", &enableFirst);      // Edit bools storing our window open/close state
        //ImGui::Checkbox("Another Window", &enableSecond);

        ImGui::SliderFloat("fFactor", &fFactor, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("v3Color", (float*)&v3Color); // Edit 3 floats representing a color

        //if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        //    counter++;
        //ImGui::SameLine();
        //ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / g_io->Framerate, g_io->Framerate);
        ImGui::End();
    }

    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();

    // Render ImGui
    ImGui_ImplVulkan_RenderDrawData(draw_data, vkCommandBuffer_Array[curIndex]);

    return vkResult;
}

#endif //IMGUI_ENABLE
void RenderFullscreenQuad(uint32_t curIndex)
{
    // Bind the pipeline and descriptor sets
    vkCmdBindPipeline(vkCommandBuffer_Array[curIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gpGraphicsPipelines->PreviewImage.vkPipeline);
    vkCmdBindDescriptorSets(vkCommandBuffer_Array[curIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gpGraphicsPipelines->PreviewImage.vkPipelineLayout, 0, 1, &vkDescriptorSet_SingleImage, 0, NULL);
    vkCmdDraw(vkCommandBuffer_Array[curIndex], 3, 1, 0, 0);
}

void RenderColoredTriangle(uint32_t curIndex)
{
    static float fAngle = 0.0f;
    fAngle += MyWin32::fDeltaTime * 300.0f; // Increment the angle for rotation
    if (fAngle >= 360.0f)
        fAngle = fAngle - 360.0f; // Reset the angle if it exceeds 360 degrees

    PushConstants pushConstants;
    memset(&pushConstants, 0, sizeof(PushConstants));

    pushConstants.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(fAngle), glm::vec3(0.0f, 0.0f, 1.0f));

#ifdef IMGUI_ENABLE
    pushConstants.v3Color = v3Color;
    pushConstants.fFactor = fFactor;
#else
    pushConstants.v3Color = glm::vec3(1.0);
    pushConstants.fFactor = 1.0f;
#endif //IMGUI_ENABLE


    // Bind the pipeline and descriptor sets
    vkCmdBindPipeline(vkCommandBuffer_Array[curIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gpGraphicsPipelines->ColoredVertex.vkPipeline);
    vkCmdBindDescriptorSets(vkCommandBuffer_Array[curIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gpGraphicsPipelines->ColoredVertex.vkPipelineLayout, 0, 1, &vkDescriptorSets_frameData[curIndex], 0, NULL);

    // Push the model matrix
    vkCmdPushConstants(
        vkCommandBuffer_Array[curIndex],
        gpGraphicsPipelines->ColoredVertex.vkPipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, // stages where the push constant will be used
        0,                                      // offset
        sizeof(PushConstants),                  // size
        &pushConstants                          // pointer to our data
    );
    // Bind vertex buffer
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(vkCommandBuffer_Array[curIndex], 0, 1, &vertexData_coloredTriangle.vkBuffer, &offset);

    // Draw the triangle
    vkCmdDraw(vkCommandBuffer_Array[curIndex], 3, 1, 0, 0);
}

void RenderAxes(uint32_t curIndex)
{
    PushConstants pushConstants;
    memset(&pushConstants, 0, sizeof(PushConstants));

    pushConstants.model = glm::mat4(1.0f); // Identity matrix for no transformation

    pushConstants.v3Color = glm::vec3(1.0);
    pushConstants.fFactor = 1.0f;

    // Bind the pipeline and descriptor sets
    vkCmdBindPipeline(vkCommandBuffer_Array[curIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gpGraphicsPipelines->ColoredVertex.vkPipeline);
    vkCmdBindDescriptorSets(vkCommandBuffer_Array[curIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gpGraphicsPipelines->ColoredVertex.vkPipelineLayout, 0, 1, &vkDescriptorSets_frameData[curIndex], 0, NULL);

    // Push the model matrix
    vkCmdPushConstants(
        vkCommandBuffer_Array[curIndex],
        gpGraphicsPipelines->ColoredVertex.vkPipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, // stages where the push constant will be used
        0,                                      // offset
        sizeof(PushConstants),                  // size
        &pushConstants                          // pointer to our data
    );
    // Bind vertex buffer
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(vkCommandBuffer_Array[curIndex], 0, 1, &vertexData_Axis.vkBuffer, &offset);

    // Draw the triangle
    vkCmdDraw(vkCommandBuffer_Array[curIndex], 9, 1, 0, 0);


}

void RenderImpostor(uint32_t curIndex)
{
    PushConstants pushConstants;
    pushConstants.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 3.0f, 0.0f));//glm::vec3(0.0f, 3.0f, 0.0f)
    //pushConstants.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    //ushConstants.model = glm::mat4(1.0f); // Identity matrix for no transformation

#ifdef IMGUI_ENABLE
    pushConstants.v3Color = v3Color;
    pushConstants.fFactor = fFactor;
#else
    pushConstants.v3Color = glm::vec3(1.0);
    pushConstants.fFactor = 1.0f;
#endif //IMGUI_ENABLE


    // Bind the pipeline and descriptor sets
    vkCmdBindPipeline(vkCommandBuffer_Array[curIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gpGraphicsPipelines->Impostor.vkPipeline);

    VkDescriptorSet vkLocalDescriptorSets[] = { vkDescriptorSets_frameData[curIndex], vkDescriptorSet_AlbedoNormal };
    vkCmdBindDescriptorSets(vkCommandBuffer_Array[curIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gpGraphicsPipelines->Impostor.vkPipelineLayout, 0, 2, vkLocalDescriptorSets, 0, NULL);

    // Push the model matrix
    vkCmdPushConstants(
        vkCommandBuffer_Array[curIndex],
        gpGraphicsPipelines->Impostor.vkPipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, // stages where the push constant will be used
        0,                                      // offset
        sizeof(PushConstants),                  // size
        &pushConstants                          // pointer to our data
    );
    // Bind vertex buffer
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(vkCommandBuffer_Array[curIndex], 0, 1, &ImpostorBufferData.vertexData.vkBuffer, &offset);
    vkCmdBindIndexBuffer(vkCommandBuffer_Array[curIndex], ImpostorBufferData.indexData.vkBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(vkCommandBuffer_Array[curIndex], (uint32_t)24, 1, 0, 0, 0);

}

void RenderCube(uint32_t curIndex)
{
    PushConstants pushConstants;
    pushConstants.model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 0.0f, 2.0f));//glm::vec3(0.0f, 3.0f, 0.0f)
    //pushConstants.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    //ushConstants.model = glm::mat4(1.0f); // Identity matrix for no transformation

#ifdef IMGUI_ENABLE
    pushConstants.v3Color = v3Color;
    pushConstants.fFactor = fFactor;
#else
    pushConstants.v3Color = glm::vec3(1.0);
    pushConstants.fFactor = 1.0f;
#endif //IMGUI_ENABLE


    // Bind the pipeline and descriptor sets
    vkCmdBindPipeline(vkCommandBuffer_Array[curIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gpGraphicsPipelines->Phong.vkPipeline);

    VkDescriptorSet vkLocalDescriptorSets[] = { vkDescriptorSets_frameData[curIndex] };
    vkCmdBindDescriptorSets(vkCommandBuffer_Array[curIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gpGraphicsPipelines->Phong.vkPipelineLayout, 0, 1, vkLocalDescriptorSets, 0, NULL);

    // Push the model matrix
    vkCmdPushConstants(
        vkCommandBuffer_Array[curIndex],
        gpGraphicsPipelines->Phong.vkPipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, // stages where the push constant will be used
        0,                                      // offset
        sizeof(PushConstants),                  // size
        &pushConstants                          // pointer to our data
    );
    // Bind vertex buffer
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(vkCommandBuffer_Array[curIndex], 0, 1, &CubeBufferData.vertexData.vkBuffer, &offset);
    vkCmdBindIndexBuffer(vkCommandBuffer_Array[curIndex], CubeBufferData.indexData.vkBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(vkCommandBuffer_Array[curIndex], (uint32_t)36, 1, 0, 0, 0);

}

void RenderSuzanne(uint32_t curIndex)
{
    static float fAngle = 0.0f;
    fAngle += MyWin32::fDeltaTime * 100.0f; // Increment the angle for rotation
    if (fAngle >= 360.0f)
        fAngle = fAngle - 360.0f; // Reset the angle if it exceeds 360 degrees

    PushConstants pushConstants;
    pushConstants.model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, -10.0f, 2.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(fAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    //pushConstants.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    //ushConstants.model = glm::mat4(1.0f); // Identity matrix for no transformation

#ifdef IMGUI_ENABLE
    pushConstants.v3Color = v3Color;
    pushConstants.fFactor = fFactor;
#else
    pushConstants.v3Color = glm::vec3(1.0);
    pushConstants.fFactor = 1.0f;
#endif //IMGUI_ENABLE


    // Bind the pipeline and descriptor sets
    vkCmdBindPipeline(vkCommandBuffer_Array[curIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gpGraphicsPipelines->Phong.vkPipeline);

    VkDescriptorSet vkLocalDescriptorSets[] = { vkDescriptorSets_frameData[curIndex] };
    vkCmdBindDescriptorSets(vkCommandBuffer_Array[curIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gpGraphicsPipelines->Phong.vkPipelineLayout, 0, 1, vkLocalDescriptorSets, 0, NULL);

    // Push the model matrix
    vkCmdPushConstants(
        vkCommandBuffer_Array[curIndex],
        gpGraphicsPipelines->Phong.vkPipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, // stages where the push constant will be used
        0,                                      // offset
        sizeof(PushConstants),                  // size
        &pushConstants                          // pointer to our data
    );
    // Bind vertex buffer
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(vkCommandBuffer_Array[curIndex], 0, 1, &SuzanneBufferData.vertexData.vkBuffer, &offset);
    vkCmdBindIndexBuffer(vkCommandBuffer_Array[curIndex], SuzanneBufferData.indexData.vkBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(vkCommandBuffer_Array[curIndex], SuzanneBufferData.indicesCount, 1, 0, 0, 0);

}

void RenderPBR_Basic(uint32_t curIndex)
{
    static float fAngle = 0.0f;
    fAngle += MyWin32::fDeltaTime * 20.0f; // Increment the angle for rotation
    if (fAngle >= 360.0f)
        fAngle = fAngle - 360.0f; // Reset the angle if it exceeds 360 degrees

    PushConstants pushConstants;
    pushConstants.model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, 10.0f, 2.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(fAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    //pushConstants.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    //ushConstants.model = glm::mat4(1.0f); // Identity matrix for no transformation

#ifdef IMGUI_ENABLE
    pushConstants.v3Color = v3Color;
    pushConstants.fFactor = fFactor;
#else
    pushConstants.v3Color = glm::vec3(1.0);
    pushConstants.fFactor = 1.0f;
#endif //IMGUI_ENABLE


    // Bind the pipeline and descriptor sets
    vkCmdBindPipeline(vkCommandBuffer_Array[curIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gpGraphicsPipelines->PBR.vkPipeline);

    VkDescriptorSet vkLocalDescriptorSets[] = { vkDescriptorSets_frameData[curIndex],pMaterial_BasicPBR_GrassyGround->getDescriptorSet() };
    vkCmdBindDescriptorSets(vkCommandBuffer_Array[curIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gpGraphicsPipelines->PBR.vkPipelineLayout, 0, _ARRAYSIZE(vkLocalDescriptorSets), vkLocalDescriptorSets, 0, NULL);

    // Push the model matrix
    vkCmdPushConstants(
        vkCommandBuffer_Array[curIndex],
        gpGraphicsPipelines->PBR.vkPipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, // stages where the push constant will be used
        0,                                      // offset
        sizeof(PushConstants),                  // size
        &pushConstants                          // pointer to our data
    );
    // Bind vertex buffer
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(vkCommandBuffer_Array[curIndex], 0, 1, &SphereBufferData.vertexData.vkBuffer, &offset);
    vkCmdBindIndexBuffer(vkCommandBuffer_Array[curIndex], SphereBufferData.indexData.vkBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(vkCommandBuffer_Array[curIndex], SphereBufferData.indicesCount, 1, 0, 0, 0);

}

void RenderPBR_Skinned(uint32_t curIndex)
{
    static float fAngle = 0.0f;
    fAngle += MyWin32::fDeltaTime * 20.0f; // Increment the angle for rotation
    if (fAngle >= 360.0f)
        fAngle = fAngle - 360.0f; // Reset the angle if it exceeds 360 degrees

    pModel_Rat->UpdateAnimation(MyWin32::fDeltaTime * 1.0f, curIndex);

    PushConstants pushConstants;
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 0.0f));
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(fAngle), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));

	pushConstants.model = translation * rotation * scale;
    //pushConstants.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    //ushConstants.model = glm::mat4(1.0f); // Identity matrix for no transformation

	pushConstants.materialIDs = glm::uvec4(0, 1, 2, 0);
    
#ifdef IMGUI_ENABLE
    pushConstants.v3Color = v3Color;
    pushConstants.fFactor = fFactor;
#else
    pushConstants.v3Color = glm::vec3(1.0);
    pushConstants.fFactor = 1.0f;
#endif //IMGUI_ENABLE


    // Bind the pipeline and descriptor sets
    vkCmdBindPipeline(vkCommandBuffer_Array[curIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gpGraphicsPipelines->PBR_Skinned.vkPipeline);

    VkDescriptorSet vkLocalDescriptorSets[] = { vkDescriptorSets_frameDataBoneData[curIndex],pModel_Rat->GetMaterialDescriptorSet(0),global_textureArray_vkDescriptorSet };
  //  VkDescriptorSet vkLocalDescriptorSets[] = { vkDescriptorSets_frameDataBoneData[curIndex],pMaterial_BasicPBR_GrassyGround->getDescriptorSet() };
    vkCmdBindDescriptorSets(vkCommandBuffer_Array[curIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gpGraphicsPipelines->PBR_Skinned.vkPipelineLayout, 0, _ARRAYSIZE(vkLocalDescriptorSets), vkLocalDescriptorSets, 0, NULL);

    // Push the model matrix
    vkCmdPushConstants(
        vkCommandBuffer_Array[curIndex],
        gpGraphicsPipelines->PBR_Skinned.vkPipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, // stages where the push constant will be used
        0,                                      // offset
        sizeof(PushConstants),                  // size
        &pushConstants                          // pointer to our data
    );
    // Bind vertex buffer
    VkDeviceSize offset = 0;

	const VulkanComboData* vulkanComboData_Rat = pModel_Rat->GetVulkanComboData();

    vkCmdBindVertexBuffers(vkCommandBuffer_Array[curIndex], 0, 1, &vulkanComboData_Rat->vertexData.vkBuffer, &offset);
    vkCmdBindIndexBuffer(vkCommandBuffer_Array[curIndex], vulkanComboData_Rat->indexData.vkBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(vkCommandBuffer_Array[curIndex], vulkanComboData_Rat->indicesCount, 1, 0, 0, 0);

}

void transitionImageLayout(
    VkCommandBuffer cmd,
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout)
{
    VkImageMemoryBarrier2 barrier{
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2
    };

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.image = image;

    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    // ---- Transition logic ----

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        barrier.srcAccessMask = VK_ACCESS_2_NONE;

        barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        barrier.dstAccessMask =
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    {
        barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;

        barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
        barrier.dstAccessMask = VK_ACCESS_2_NONE;
    }
    else
    {
        throw std::runtime_error("Unsupported layout transition");
    }

    VkDependencyInfo dep{
        VK_STRUCTURE_TYPE_DEPENDENCY_INFO
    };
    dep.imageMemoryBarrierCount = 1;
    dep.pImageMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(cmd, &dep);
}

void transitionDepthLayout(
    VkCommandBuffer cmd,
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout)
{
    VkImageMemoryBarrier2 barrier{
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2
    };

    barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
    barrier.srcAccessMask = VK_ACCESS_2_NONE;

    barrier.dstStageMask =
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
        VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;

    barrier.dstAccessMask =
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.image = image;

    barrier.subresourceRange.aspectMask =
        VK_IMAGE_ASPECT_DEPTH_BIT;
    barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    dep.imageMemoryBarrierCount = 1;
    dep.pImageMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(cmd, &dep);
}


VkResult buildCommandBuffers(uint32_t curIndex, uint32_t currentImageIndex)
{
    VkResult vkResult = VK_SUCCESS;

    // === Only operate on curIndex ===

    // Reset the command buffer
    vkResult = vkResetCommandBuffer(vkCommandBuffer_Array[curIndex], 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "buildCommandBuffers() : vkResetCommandBuffer() failed for command buffer %d.\n", curIndex);
        return vkResult;
    }

    // Begin recording
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkResult = vkBeginCommandBuffer(vkCommandBuffer_Array[curIndex], &beginInfo);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "buildCommandBuffers() : vkBeginCommandBuffer() failed for command buffer %d.\n", curIndex);
        return vkResult;
    }

    //   VkClearValue clearValue = {};
    //   clearValue.color = vkClearColorValue;
       //clearValue.depthStencil = vkClearDepthStencilValue;

    //VkClearValue clearValues[2];
    //clearValues[0].color.float32[0] = 0.01f;
    //clearValues[0].color.float32[1] = 0.01f;
    //clearValues[0].color.float32[2] = 0.01f;
    //clearValues[0].color.float32[3] = 1.0f;
    //clearValues[1].depthStencil.depth = 1.0f;
    //clearValues[1].depthStencil.stencil = 0;

    //VkRenderPassBeginInfo renderPassInfo = {};
    //renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    //renderPassInfo.renderPass = vkRenderPass;
    //renderPassInfo.framebuffer = vkFramebuffer_Array[curIndex];
    //renderPassInfo.renderArea.extent = vkExtent2D_Swapchain;
    //renderPassInfo.clearValueCount = 2;
    //renderPassInfo.pClearValues = clearValues;

    //vkCmdBeginRenderPass(vkCommandBuffer_Array[curIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    //Dynamic Rendering

    transitionImageLayout(
        vkCommandBuffer_Array[curIndex],
        gSwapChainResourceData.swapchainImage_Array[currentImageIndex],
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	);


	// We can also use dynamic rendering instead of render pass and framebuffer. In that case, we need to declare and initialize VkRenderingInfo structure and call vkCmdBeginRendering() API to begin the dynamic rendering. Remember, if we are using dynamic rendering, then we don't need to create render pass and framebuffer objects at all.
    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView = gSwapChainResourceData.swapchainImageView_Array[currentImageIndex];
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkClearValue clearColor = { {{0.01f, 0.01f, 0.02f, 1.0f}} };
    colorAttachment.clearValue = clearColor;

    VkRenderingAttachmentInfo depthAttachment{};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depthAttachment.imageView = gSwapChainResourceData.imageData_depthBuffer[currentImageIndex].vkImageView;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.clearValue.depthStencil = { 1.0f, 0 };

	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea.offset = { 0, 0 };
	renderingInfo.renderArea.extent = vkExtent2D_Swapchain;
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;
	renderingInfo.pDepthAttachment = &depthAttachment;
	vkCmdBeginRendering(vkCommandBuffer_Array[curIndex], &renderingInfo);


    // RenderFullscreenQuad(curIndex); // Render the fullscreen quad

    RenderAxes(curIndex); // Render the axes

    RenderColoredTriangle(curIndex); // Render the colored triangle
    // Draw the indexed triangle
    //vkCmdBindIndexBuffer(vkCommandBuffer_Array[curIndex], vertexData_position_index.vkBuffer, 0, VK_INDEX_TYPE_UINT32);
    //vkCmdDrawIndexed(vkCommandBuffer_Array[curIndex], (uint32_t)(sizeof(triangle_indices) / sizeof(triangle_indices[0])), 1, 0, 0, 0);

    RenderImpostor(curIndex); // Render the UV quad

    RenderCube(curIndex); // Render the cube
    RenderSuzanne(curIndex); // Render the suzanne

    RenderPBR_Basic(curIndex); // Render the PBR basic model

	RenderPBR_Skinned(curIndex); // Render the PBR skinned model

#ifdef IMGUI_ENABLE
    RenderImGui(curIndex); // Render ImGui UI
#endif //IMGUI_ENABLE

   // vkCmdEndRenderPass(vkCommandBuffer_Array[curIndex]);

	// If we are using dynamic rendering, then we need to call vkCmdEndRendering() API to end the dynamic rendering.
	vkCmdEndRendering(vkCommandBuffer_Array[curIndex]);


    transitionImageLayout(
        vkCommandBuffer_Array[curIndex],
        gSwapChainResourceData.swapchainImage_Array[currentImageIndex],
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	);





    vkResult = vkEndCommandBuffer(vkCommandBuffer_Array[curIndex]);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "buildCommandBuffers() : vkEndCommandBuffer() failed for command buffer %d.\n", curIndex);
        return vkResult;
    }

    return vkResult;
}

VkResult createGraphicsPipelines(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;
    // code

    if (gpGraphicsPipelines)
    {
        vkResult = gpGraphicsPipelines->createPipelines();
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createGraphicsPipelines() : gpGraphicsPipelines->createPipelines() failed: %d.\n", vkResult);
            return vkResult;
        }
    }
    else
    {
        gpGraphicsPipelines = new GraphicsPipelines();
        vkResult = gpGraphicsPipelines->vkResult;
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createGraphicsPipelines() : gpGraphicsPipelines->createPipelines() failed: %d.\n", vkResult);
            return vkResult;
        }
    }



    return vkResult;
}

void destroyGraphicsPipelines(void)
{

    if (gpGraphicsPipelines)
    {
        gpGraphicsPipelines->destroyPipelines();
    }


}

VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT vkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    //code
    fprintf(gpFILE, "NDT_Validation: %s %d = %s \n", pLayerPrefix, messageCode, pMessage);

    return VK_FALSE;
}

