#pragma once
#include "VK.h"
#include <tinyddsloader.h>
#include <Windows.h>

extern VkPhysicalDevice vkPhysicalDevice_Selected;
extern VkDevice vkDevice;
extern VkQueue vkQueue;
extern VkCommandPool vkCommandPool;
extern VkDescriptorPool vkDescriptorPool;
extern VkSampler vkSampler_LinearClamp;


extern FILE* gpFILE;

struct Material_BasicPBR
{
public:
	Material_BasicPBR(VkDescriptorSetLayout layout, const char* pathAlbedo, const char* pathNormal, const char* PathORX);
	~Material_BasicPBR();

	VkDescriptorSet getDescriptorSet(void) const { return(vkDescriptorSet); }


private:

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkResult loadTextureData_dds_c_bc7(ImageData* imageData, const char* filename, VkFormat format);

	VkResult createDescriptorSet(void);

	VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSet vkDescriptorSet = VK_NULL_HANDLE;
	ImageData Albedo;
	ImageData Normal;
	ImageData ORX;

};

