#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "VK.h"
#include "VulkanContext.h"
#include "FrameContext.h"
#include "DescriptorSetLayouts.h"
#include <iostream>
#include <sstream>  // Fixes: incomplete type "std::stringstream"
#include <iomanip>  // Needed for std::setfill and std::setw
#include <tinyddsloader.h>
#include "ComputePipelines.h"

extern VkDescriptorPool vkDescriptorPool;
extern VkSampler vkSampler_LinearMipmapRepeat;
extern VkSampler vkSampler_LinearClamp;

class VolumetricClouds
{
public:
	VolumetricClouds();
	~VolumetricClouds();

	ImageData GetImageData_Clouds(void)
	{
		return imageData_Clouds;
	}

	ImageData GetImageData_Noise3D(void)
	{
		return imageData_Noise3D;
	}

	ImageData GetImageData_ModelingData3D(void)
	{
		return imageData_ModelingData3D;
	}

	void Compute_VolumetricClouds(
		uint32_t curIndex,
		VkPipeline vkPipeline,
		VkPipelineLayout vkPipelineLayout);

	glm::vec3 fLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	float fCloudFactor = 1.0f;
	float fLightIntensity = 10.0f;


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

	bool Load3DTextureWithMipmaps(
		VkDevice device,
		VkCommandPool commandPool,
		VkQueue graphicsQueue,
		const std::string& baseFolder,
		const std::string& filePrefix,
		const std::string& fileExtension,
		uint32_t sliceCount,
		uint32_t zeroPadding,
		ImageData& outImageData,
		uint32_t& outMipLevels);

	bool Load3DTexture(
		VkDevice device,
		VkCommandPool commandPool,
		VkQueue graphicsQueue,
		const std::string& baseFolder,
		const std::string& filePrefix,
		const std::string& fileExtension,
		uint32_t sliceCount,
		uint32_t zeroPadding,
		ImageData& outImageData);

	VkResult loadTextureData_dds_bc6_3d(
		const char* filename,
		ImageData* imageData);

	VkResult vkResult;
	ImageData imageData_Clouds{};
	ImageData imageData_Noise3D{};
	ImageData imageData_ModelingData3D{};

	VkDescriptorSet vkDescriptorSet_VolumetricClouds;
	uint32_t Width;
	uint32_t Height;


};

