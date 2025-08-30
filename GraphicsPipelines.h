#pragma once
#include "VK.h"
#include "DescriptorSetLayouts.h"
#include <windows.h>

extern VkDevice vkDevice;
extern DescriptorSetLayouts* gpDescriptorSetLayouts;
extern FILE* gpFILE;
extern VkViewport vkViewport;
extern VkRect2D vkScissor;
extern VkExtent2D vkExtent2D_Swapchain;
extern VkRect2D vkRect2D_Scissor;
extern  VkRenderPass vkRenderPass;

extern VkShaderModule vkShaderModule_impostor_vs;
extern VkShaderModule vkShaderModule_impostor_fs;
extern VkShaderModule vkShaderModule_previewImage_vs;
extern VkShaderModule vkShaderModule_previewImage_fs;
extern VkShaderModule vkShaderModule_phong_vs;
extern VkShaderModule vkShaderModule_phong_fs;
extern VkShaderModule vkShaderModule_PBR_vs;
extern VkShaderModule vkShaderModule_PBR_fs;

struct GraphicsPipelines
{
public:
	GraphicsPipelines();
	~GraphicsPipelines();

	PipelineData Impostor;
	PipelineData PreviewImage;
	PipelineData Phong;
	PipelineData PBR;

	VkResult createPipelines(void);
	void destroyPipelines(void);

	VkResult vkResult;

private:
	VkResult createPipelineLayout_previewImage(void);
	VkResult createPipelineLayout_Impostor(void);
	VkResult createPipelineLayout_Phong(void);
	VkResult createPipelineLayout_PBR(void);

	VkResult createPipelineLayouts(void);
	void destroyPipelineLayouts(void);

	//----------------------------Pipelines-------------------------------------------------------

	VkResult createGraphicsPipeline_PreviewImage(void);
	VkResult createGraphicsPipeline_Impostor(void);
	VkResult createGraphicsPipeline_Phong(void);
	VkResult createGraphicsPipeline_PBR(void);


};

