#pragma once
//#include"vmath.h"
#include <vulkan/vulkan.h>        // you must define platform before including this file (Windows / Linux / macOS / iOS / Android / <other>)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define MYICON 101
#define MAX_FRAMES 2 // for double buffering

// macros
#define WIN_WIDTH  2560
#define WIN_HEIGHT 1440
#define WIN_TITLE  TEXT("NDT:Vulkan AstroMediComp")
#define LINE_END     "-------------------------------------------------------------------------------------\n"

class Material_BasicPBR;
struct UniformBufferObject_FrameData
{
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 prevViewProj;
	glm::mat4 inverseViewProj;

	glm::vec3 sunDir;
    float fTime;

	glm::vec3 cameraPos; // Camera position for rendering
    uint32_t frameID;

	glm::vec4 frustumPlanes[6];
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

struct VertexData_Meshlet
{
	glm::vec4 pos;
	glm::vec4 texCoord;
	glm::vec4 normal;
	glm::vec4 tangent;
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

struct BasicPushConstants
{
	glm::mat4 model;
	glm::vec3 v3Color;
	float fFactor;
	glm::uvec4 materialIDs;
};


struct MeshletPushConstants
{
	//Device Addresses
	VkDeviceAddress meshletData;
	VkDeviceAddress meshletVertices;
	VkDeviceAddress meshletTriangles;
	VkDeviceAddress vertices;
	VkDeviceAddress modelData;

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

	VkDeviceAddress deviceAddress = 0;

	void* mapped = nullptr;

	VkDeviceSize size = 0;
};

struct MeshletData 
{
	uint32_t vertexOffset;
	uint32_t triangleOffset;
	uint32_t vertexCount;
	uint32_t triangleCount;
    glm::vec4 bounds;	  //xyz = center, w = radius
	glm::vec4 coneAxis;   // xyz = axis, w = cutoff
	glm::vec4 coneApex;     // xyz apex

	uint32_t modelIndex;
	uint32_t materialID_PBR;
	uint32_t pad1;
	uint32_t pad2;
};

struct ModelData
{
	glm::mat4 model;
	float maxScale;
	float pad[3];
};

static_assert(sizeof(ModelData) == 80);


struct MeshletGPUData
{
	VulkanSSBO vertexBuffer;
	VulkanSSBO meshletBuffer;
	VulkanSSBO meshletVertexBuffer;
	VulkanSSBO meshletTriangleBuffer;
	VulkanSSBO modelDataBuffer;

	uint32_t vertexCount = 0;
	uint32_t meshletCount = 0;
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

struct GBuffer
{
	std::vector<ImageData> albedo;
	std::vector<ImageData> normal;
	std::vector<ImageData> ORM;
	std::vector<ImageData> depth;
	std::vector<ImageData> velocity;

	uint32_t width = 0;
	uint32_t height = 0;
};

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
	VkShaderModule vkTaskShaderModule = VK_NULL_HANDLE;
	VkShaderModule vkMeshShaderModule = VK_NULL_HANDLE;
	VkShaderModule vkComputeShaderModule = VK_NULL_HANDLE;

}PipelineData;

