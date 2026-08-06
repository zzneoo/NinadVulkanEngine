#pragma once
//#include"vmath.h"
#include <vulkan/vulkan.h>        // you must define platform before including this file (Windows / Linux / macOS / iOS / Android / <other>)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define MYICON 101
#define MAX_FRAMES 2 // for double buffering

// VulkanContext
// Owns Vulkan instance/device lifetime.

// Win32Window
// Owns Win32 window and Vulkan surface.

// SwapchainContext
// Owns swapchain-dependent resources.

// FrameContext
// Owns per-frame resources (MAX_FRAMES).

//Uniform Buffer Objects------------------------------------------------


struct UniformBufferObject_FrameData
{
	glm::mat4 view;
	glm::mat4 proj;
    float fTime;
    uint32_t frameID;
	float pad0[2];

	glm::vec3 cameraPos; // Camera position for rendering
    // pad to 16 bytes (std140 rules)
	float pad1[1];
};

//----------------------------------------------------------------------

struct VertexData_PositionColor
{
	glm::vec3 pos;
	glm::vec3 color;
};

struct VertexData_PositionTexCoord
{
	glm::vec3 pos;
	glm::vec2 texCoord;
};

struct VertexData_PositionTexCoordNormalColor
{
	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec3 color;
};

struct VertexData_PositionTexCoordNormalTangent
{
	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec3 tangent;
};

struct VertexData_Skinned
{
	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec4 tangent;
	glm::ivec4 boneIDs;
	glm::vec4 boneWeights;
};

struct VertexData_Position
{
	glm::vec3 pos;
};

struct PushConstants
{
	glm::mat4 model;
	glm::vec3 v3Color;
	float fFactor;
	glm::uvec4 materialIDs;
};

//win32
struct CamStruct
{
	bool bCameraMoving_Forward;
	bool bCameraMoving_Backward;
	bool bCameraMoving_Right;
	bool bCameraMoving_Left;
	uint16_t mouseX;
	uint16_t mouseY;
	float CameraTurboSpeed = 1.0f;
};

struct ClientSize
{
	uint16_t ClientWidth;
	uint16_t ClientHeight;
};

//vertex buffer related variables
typedef struct
{
	VkBuffer vkBuffer;
	VkDeviceMemory vkDeviceMemory;
	void* pData; // pointer to the mapped memory
}UniformData;

typedef struct
{
	VkBuffer vkBuffer;
	VkDeviceMemory vkDeviceMemory;
} VulkanData;

struct VulkanSSBO 
{
	VkBuffer vkBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vkDeviceMemory = VK_NULL_HANDLE;
	VkDescriptorBufferInfo descriptor{};
	void* mapped = nullptr;           // persistently mapped pointer (or null)
	VkDeviceSize size = 0;
};

typedef struct
{
	VulkanData vertexData;
	VulkanData indexData;
	uint32_t indicesCount;
} VulkanComboData;

typedef struct
{
	VkImage        vkImage;
	VkDeviceMemory vkDeviceMemory;
	VkImageView    vkImageView;
	uint32_t       globalTextureArrayIndex; // index in the global texture array, used for descriptor sets
	VkSampler	   vkSampler; // sampler for this image (if needed)
}ImageData;

typedef struct
{
	VkImage* swapchainImage_Array;
	VkImageView* swapchainImageView_Array;
	ImageData* imageData_depthBuffer;
}SwapChainResourceData;

typedef struct PipelineData
{
	VkPipeline vkPipeline = VK_NULL_HANDLE;
	VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;
	VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;
	VkShaderModule vkVertexShaderModule = VK_NULL_HANDLE;
	VkShaderModule vkFragmentShaderModule = VK_NULL_HANDLE;
}PipelineData;


