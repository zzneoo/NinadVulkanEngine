#pragma once
//#include"vmath.h"
#include <vulkan/vulkan.h>        // you must define platform before including this file (Windows / Linux / macOS / iOS / Android / <other>)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define MYICON 101

//Uniform Buffer Objects------------------------------------------------
//struct UniformBufferObject_camera
//{
//    ///glm::mat4 model;
//    glm::mat4 view;
//    glm::mat4 proj;
//};

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
}ImageData;

typedef struct
{
	VkImage* swapchainImage_Array;
	VkImageView* swapchainImageView_Array;
	ImageData* imageData_depthBuffer;
}SwapChainResourceData;

typedef struct
{
	VkPipeline vkPipeline;
	VkPipelineLayout vkPipelineLayout;
	VkShaderModule vkVertexShaderModule;
	VkShaderModule vkFragmentShaderModule;
}PipelineData;


