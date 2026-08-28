#pragma once
#include "VK.h"
#include "VulkanContext.h"
#include "FrameContext.h"
#include "DescriptorSetLayouts.h"
#include <iostream>

extern VkDescriptorPool vkDescriptorPool;

class VolumetricClouds
{
public:
	VolumetricClouds();
	~VolumetricClouds();

	ImageData GetImageData(void)
	{
		return imageData_Clouds;
	}

	void Compute_VolumetricClouds(
		uint32_t curIndex,
		VkPipeline vkPipeline,
		VkPipelineLayout vkPipelineLayout);


private:
	VkResult InitialLayoutTransitions(void);

	void TransitionImageLayout(
		VkCommandBuffer cmd,
		VkImage image,
		VkImageLayout oldLayout,
		VkImageLayout newLayout);

	VkResult CreateCloudTexture(
		uint32_t width,
		uint32_t height,
		VkFormat format,
		ImageData& imageData);

	VkResult CreateDescriptorSet_VolumetricClouds();

	VkResult vkResult;
	ImageData imageData_Clouds{};
	VkDescriptorSet vkDescriptorSet_VolumetricClouds;
	uint32_t Width;
	uint32_t Height;


};

