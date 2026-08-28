#pragma once
#include "VK.h"
#include "DescriptorSetLayouts.h"
#include <windows.h>
#include <vector>
#include "VulkanContext.h"
#include "SwapchainContext.h"

//extern VkDevice vkDevice;
extern DescriptorSetLayouts* gpDescriptorSetLayouts;
extern FILE* gpFILE;
extern VkViewport vkViewport;
extern VkRect2D vkScissor;
//extern VkExtent2D vkExtent2D_Swapchain;
extern VkRect2D vkRect2D_Scissor;
//extern  VkRenderPass vkRenderPass;


class ComputePipelines
{
public:
	ComputePipelines();
	~ComputePipelines();


	PipelineData TextureGradient;
	PipelineData VolumetricClouds;

	VkResult vkResult;

	VkResult createPipelines(void);
	void destroyPipelines(void);

private:

	std::vector<VkPipelineLayout> vkPipelineLayoutList;
	std::vector<VkShaderModule> vkShaderModuleList;
	std::vector<VkPipeline> vkPipelineList;
	std::vector<VkPipelineCache> vkPipelineCacheList;

	VkResult createShaderModule(VkShaderModule* shaderModule, const char* fileName);

	VkResult createPipelineLayout(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo, VkPipelineLayout* vkPipelineLayout, VkPushConstantRange vkPushConstantRange);
	void destroyPipelineLayouts(void);

	VkResult createShaderModules(void);
	void destroyShaderModules(void);

	void destroyPipelineCaches(void);

	//----------------------------Pipelines-------------------------------------------------------

	VkResult createComputePipeline_TextureGradient(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createComputePipeline_VolumetricClouds(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);


};

