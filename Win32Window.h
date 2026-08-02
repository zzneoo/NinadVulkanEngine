#pragma once

#include <Windows.h>
#include <vulkan/vulkan.h>

class Win32Window
{
public:

    HWND hwnd = nullptr;

    bool fullscreen = false;
    bool active = false;

    WINDOWPLACEMENT previousPlacement{};
    DWORD windowStyle = 0;

    VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;
};

extern Win32Window gWindow;