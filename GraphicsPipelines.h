#pragma once
#include "VK.h"
#include "DescriptorSetLayouts.h"
#include <windows.h>
#include <vector>

extern VkDevice vkDevice;
extern DescriptorSetLayouts* gpDescriptorSetLayouts;
extern FILE* gpFILE;
extern VkViewport vkViewport;
extern VkRect2D vkScissor;
extern VkExtent2D vkExtent2D_Swapchain;
extern VkRect2D vkRect2D_Scissor;
extern  VkRenderPass vkRenderPass;

//extern VkShaderModule vkShaderModule_impostor_vs;
//extern VkShaderModule vkShaderModule_impostor_fs;
//extern VkShaderModule vkShaderModule_previewImage_vs;
//extern VkShaderModule vkShaderModule_previewImage_fs;
//extern VkShaderModule vkShaderModule_phong_vs;
//extern VkShaderModule vkShaderModule_phong_fs;
//extern VkShaderModule vkShaderModule_PBR_vs;
//extern VkShaderModule vkShaderModule_PBR_fs;
//extern VkShaderModule vkShaderModule_PBR_Skinned_vs;
//extern VkShaderModule vkShaderModule_PBR_Skinned_fs;
//extern VkShaderModule vkShaderModule_whiteVertex_vs;
//extern VkShaderModule vkShaderModule_whiteVertex_fs;
//extern VkShaderModule vkShaderModule_basic_vs;
//extern VkShaderModule vkShaderModule_basic_fs;


struct GraphicsPipelines
{
public:
	GraphicsPipelines();
	~GraphicsPipelines();

	PipelineData Impostor;
	PipelineData PreviewImage;
	PipelineData Phong;
	PipelineData PBR;
	PipelineData PBR_Skinned;
	PipelineData WhiteVertex;
	PipelineData ColoredVertex;

	VkResult vkResult;

	VkResult createPipelines(void);
	void destroyPipelines(void);

private:

	std::vector<VkPipelineLayout> vkPipelineLayoutList;
	std::vector<VkShaderModule> vkShaderModuleList;
	std::vector<VkPipeline> vkPipelineList;

	VkResult createShaderModule(VkShaderModule* shaderModule, const char* fileName);

	VkResult createPipelineLayout(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo, VkPipelineLayout* vkPipelineLayout);
	void destroyPipelineLayouts(void);

	VkResult createShaderModules(void);
	void destroyShaderModules(void);

	//----------------------------Pipelines-------------------------------------------------------

	VkResult createGraphicsPipeline_PreviewImage(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createGraphicsPipeline_Impostor(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createGraphicsPipeline_Phong(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createGraphicsPipeline_PBR(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createGraphicsPipeline_PBR_Skinned(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createGraphicsPipeline_WhiteVertex(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createGraphicsPipeline_ColoredVertex(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);


};

