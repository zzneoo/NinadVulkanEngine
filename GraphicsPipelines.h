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


class GraphicsPipelines
{
public:
	GraphicsPipelines();
	~GraphicsPipelines();

	PipelineData Impostor;
	PipelineData PreviewImage;
	PipelineData DeferredPBR;
	PipelineData Phong;
	PipelineData PBR;
	PipelineData PBR_Skinned;
	PipelineData WhiteVertex;
	PipelineData ColoredVertex;
	PipelineData Meshlet;

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

	VkResult createGraphicsPipeline_PreviewImage(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createGraphicsPipeline_DeferredPBR(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createGraphicsPipeline_Fullscreen(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo, PipelineData& pipeline);
	VkResult createGraphicsPipeline_Impostor(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createGraphicsPipeline_Phong(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createGraphicsPipeline_PBR(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createGraphicsPipeline_PBR_Skinned(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createGraphicsPipeline_WhiteVertex(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createGraphicsPipeline_ColoredVertex(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createGraphicsPipeline_Meshlet(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);


};

