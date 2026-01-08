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

	VkResult createPipelines(void);
	void destroyPipelines(void);

	VkResult vkResult;

private:

	std::vector<VkPipelineLayout> vkPipelineLayoutList;

	VkResult createShaderModule(VkShaderModule* shaderModule, const char* fileName);

	VkResult createPipelineLayout(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo, VkPipelineLayout* vkPipelineLayout);

	VkResult createPipelineLayout_previewImage(void);
	VkResult createPipelineLayout_Phong(void);
	VkResult createPipelineLayout_PBR(void);
	VkResult createPipelineLayout_WhiteVertex(void);
	VkResult createPipelineLayout_ColoredVertex(void);

	VkResult createPipelineLayouts(void);
	void destroyPipelineLayouts(void);

	VkResult createShaderModules(void);
	void destroyShaderModules(void);

	//----------------------------Pipelines-------------------------------------------------------

	VkResult createGraphicsPipeline_PreviewImage(void);
	VkResult createGraphicsPipeline_Impostor(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createGraphicsPipeline_Phong(void);
	VkResult createGraphicsPipeline_PBR(void);
	VkResult createGraphicsPipeline_PBR_Skinned(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo);
	VkResult createGraphicsPipeline_WhiteVertex(void);
	VkResult createGraphicsPipeline_ColoredVertex(void);


};

