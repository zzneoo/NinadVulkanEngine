#pragma once
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <string.h>
#include "VK.h"


struct DescriptorSetLayouts
{
public:
	DescriptorSetLayouts();
	~DescriptorSetLayouts();

	static VkResult createDescriptorSetLayout(const VkDescriptorSetLayoutBinding* DescriptorSetLayoutBinding, uint32_t  bindingCount, VkDescriptorSetLayout* DescriptorSetLayout);

	VkDescriptorSetLayout vkDescriptorSetLayout_frameData = VK_NULL_HANDLE;
	VkDescriptorSetLayout vkDescriptorSetLayout_frameDataBoneData = VK_NULL_HANDLE;
	VkDescriptorSetLayout vkDescriptorSetLayout_SingleImage = VK_NULL_HANDLE;
	VkDescriptorSetLayout vkDescriptorSetLayout_AlbedoNormal = VK_NULL_HANDLE;
	VkDescriptorSetLayout vkDescriptorSetLayout_BasicPBR = VK_NULL_HANDLE;
	VkDescriptorSetLayout vkDescriptorSetLayout_GlobalTextureArray = VK_NULL_HANDLE;

	VkResult vkResult;

private:
	VkResult createDescriptorSetLayouts(void);
	void destroyDescriptorSetLayouts(void);

	VkResult createDescriptorSetLayout_FrameData(void);
	VkResult createDescriptorSetLayout_FrameDataBoneData(void);
	VkResult createDescriptorSetLayout_SingleImage(void);
	VkResult createDescriptorSetLayout_AlbedoNormal(void);
	VkResult createDescriptorSetLayout_BasicPBR(void);
	VkResult createDescriptorSetLayout_GlobalTextureArray(void);

};

