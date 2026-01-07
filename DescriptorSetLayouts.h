#pragma once
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <string.h>


struct DescriptorSetLayouts
{
public:
	DescriptorSetLayouts();
	~DescriptorSetLayouts();


	VkDescriptorSetLayout vkDescriptorSetLayout_frameData = VK_NULL_HANDLE;
	VkDescriptorSetLayout vkDescriptorSetLayout_frameDataBoneData = VK_NULL_HANDLE;
	VkDescriptorSetLayout vkDescriptorSetLayout_SingleImage = VK_NULL_HANDLE;
	VkDescriptorSetLayout vkDescriptorSetLayout_AlbedoNormal = VK_NULL_HANDLE;
	VkDescriptorSetLayout vkDescriptorSetLayout_BasicPBR = VK_NULL_HANDLE;

	VkResult vkResult;

private:
	VkResult createDescriptorSetLayouts(void);
	void destroyDescriptorSetLayouts(void);

	VkResult createDescriptorSetLayout_FrameData(void);
	VkResult createDescriptorSetLayout_FrameDataBoneData(void);
	VkResult createDescriptorSetLayout_SingleImage(void);
	VkResult createDescriptorSetLayout_AlbedoNormal(void);
	VkResult createDescriptorSetLayout_BasicPBR(void);

};

