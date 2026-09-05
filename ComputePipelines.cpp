#include "ComputePipelines.h"

ComputePipelines* gpComputePipelines = NULL;

ComputePipelines::ComputePipelines()
{
	vkResult = VK_SUCCESS;

	vkResult = createShaderModules();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "ComputePipelines() : createShaderModules() failed (%d).\n", vkResult);
		return;
	}

	vkResult = createPipelines();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "ComputePipelines() : createPipelines() failed (%d).\n", vkResult);
		return;
	}
}


ComputePipelines::~ComputePipelines()
{
	destroyPipelines();
	destroyPipelineLayouts();
	destroyPipelineCaches();
	destroyShaderModules();
}


//------------------------------Shader Modules------------------------------------------------

VkResult ComputePipelines::createShaderModule(VkShaderModule* shaderModule, const char* fileName)
{
	// local variables
	VkResult vkResult = VK_SUCCESS;

	FILE* fp = NULL;
	size_t size = 0;

	errno_t err = fopen_s(&fp, fileName, "rb");

	if (err != 0)
	{
		fprintf(gpFILE, "createShaders() -> fopen_s() : failed.\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}


	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);

	char* shaderData = (char*)malloc(sizeof(char) * size);

	if (!shaderData)
	{
		fprintf(gpFILE, "createShaders() -> shaderData size failed.\n");
		fclose(fp);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}

	size_t retVal = fread(shaderData, size, 1, fp);

	if (retVal != 1)
	{
		fprintf(gpFILE, "createShaders() -> fread() : failed.\n");
		fclose(fp);
		free(shaderData);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}


	fclose(fp);

	VkShaderModuleCreateInfo vkShaderModuleCreateInfo{};
	vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vkShaderModuleCreateInfo.pNext = NULL;
	vkShaderModuleCreateInfo.flags = 0;
	vkShaderModuleCreateInfo.codeSize = size;
	vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;


	vkResult = vkCreateShaderModule(
		gVulkanContext.vkDevice,
		&vkShaderModuleCreateInfo,
		NULL,
		shaderModule
	);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createShaders() -> vkCreateShaderModule() : failed.\n");

		free(shaderData);
		shaderData = NULL;

		return(vkResult);
	}


	if (shaderData)
	{
		free(shaderData);
		shaderData = NULL;
	}


	vkShaderModuleList.push_back(*shaderModule);

	return vkResult;
}


//------------------------------Shader Modules------------------------------------------------

VkResult ComputePipelines::createShaderModules()
{
	VkResult vkResult = VK_SUCCESS;


	// Texture Gradient Compute Shader
	vkResult = createShaderModule(
		&TextureGradient.vkComputeShaderModule,
		"TextureGradient.comp.spv"
	);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(
			gpFILE,
			"createShaders() -> createShaderModule() for TextureGradient compute shader failed.\n"
		);

		return vkResult;
	}

	// Volumetric Clouds Compute Shader
	vkResult = createShaderModule(
		&VolumetricClouds.vkComputeShaderModule,
		"VolumetricClouds.comp.spv"
	);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(
			gpFILE,
			"createShaders() -> createShaderModule() for VolumetricClouds compute shader failed.\n"
		);

		return vkResult;
	}


	return(vkResult);
}


void ComputePipelines::destroyShaderModules(void)
{
	// destroy shader modules from the list in reverse order
	for (int32_t i = (int32_t)vkShaderModuleList.size() - 1; i >= 0; i--)
	{
		if (vkShaderModuleList[i])
		{
			vkDestroyShaderModule(
				gVulkanContext.vkDevice,
				vkShaderModuleList[i],
				NULL
			);

			vkShaderModuleList[i] = VK_NULL_HANDLE;
		}
	}

	vkShaderModuleList.clear();
}


//----------------------------Pipeline Caches------------------------------------------------

void ComputePipelines::destroyPipelineCaches(void)
{
	// destroy pipeline caches from the list in reverse order
	for (int32_t i = (int32_t)vkPipelineCacheList.size() - 1; i >= 0; i--)
	{
		if (vkPipelineCacheList[i])
		{
			vkDestroyPipelineCache(
				gVulkanContext.vkDevice,
				vkPipelineCacheList[i],
				NULL
			);

			vkPipelineCacheList[i] = VK_NULL_HANDLE;
		}
	}

	vkPipelineCacheList.clear();
}


//----------------------------Pipeline Layouts------------------------------------------------

VkResult ComputePipelines::createPipelineLayout(
	VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo,
	VkPipelineLayout* vkPipelineLayout,
	VkPushConstantRange vkPushConstantRange
)
{
	// local variables
	VkResult vkResult = VK_SUCCESS;


	vkPipelineLayoutCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	vkPipelineLayoutCreateInfo.pNext = NULL;
	vkPipelineLayoutCreateInfo.flags = 0;

	vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	vkPipelineLayoutCreateInfo.pPushConstantRanges =&vkPushConstantRange;


	vkResult = vkCreatePipelineLayout(
		gVulkanContext.vkDevice,
		&vkPipelineLayoutCreateInfo,
		NULL,
		vkPipelineLayout
	);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(
			gpFILE,
			"createPipelineLayout() -> vkCreatePipelineLayout() : failed: %d.\n",
			vkResult
		);

		return(vkResult);
	}


	vkPipelineLayoutList.push_back(*vkPipelineLayout);

	return(vkResult);
}


void ComputePipelines::destroyPipelineLayouts(void)
{
	// destroy vkPipelineLayoutList
	for (int32_t i = (int32_t)vkPipelineLayoutList.size() - 1; i >= 0; i--)
	{
		if (vkPipelineLayoutList[i])
		{
			vkDestroyPipelineLayout(
				gVulkanContext.vkDevice,
				vkPipelineLayoutList[i],
				NULL
			);

			vkPipelineLayoutList[i] = VK_NULL_HANDLE;
		}
	}

	vkPipelineLayoutList.clear();
}


//----------------------------Pipelines-------------------------------------------------------

//TextureGradient
VkResult ComputePipelines::createComputePipeline_TextureGradient(
	VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo
)
{
	// local variables
	VkResult vkResult = VK_SUCCESS;


	// Push constants
	VkPushConstantRange vkPushConstantRange{};
	vkPushConstantRange.offset = 0;
	vkPushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	vkPushConstantRange.size = 0;


	vkResult = createPipelineLayout(
		vkPipelineLayoutCreateInfo,
		&TextureGradient.vkPipelineLayout,
		vkPushConstantRange
	);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(
			gpFILE,
			"createGraphicsPipeline_TextureGradient() : createPipelineLayout() failed (%d).\n",
			vkResult
		);

		return(vkResult);
	}


	//------------------------------------------------------------------------
	// Compute shader stage
	//------------------------------------------------------------------------

	VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo{};

	vkPipelineShaderStageCreateInfo.sType =VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo.pNext = NULL;
	vkPipelineShaderStageCreateInfo.flags = 0;
	vkPipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	vkPipelineShaderStageCreateInfo.module =TextureGradient.vkComputeShaderModule;
	vkPipelineShaderStageCreateInfo.pName = "main";
	vkPipelineShaderStageCreateInfo.pSpecializationInfo = NULL;


	//------------------------------------------------------------------------
	// Pipeline cache
	//------------------------------------------------------------------------

	if (TextureGradient.vkPipelineCache == VK_NULL_HANDLE)
	{
		VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo{};


		vkPipelineCacheCreateInfo.sType =VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		vkPipelineCacheCreateInfo.pNext = NULL;
		vkPipelineCacheCreateInfo.flags = 0;
		vkPipelineCacheCreateInfo.initialDataSize = 0;
		vkPipelineCacheCreateInfo.pInitialData = NULL;


		vkResult = vkCreatePipelineCache(
			gVulkanContext.vkDevice,
			&vkPipelineCacheCreateInfo,
			NULL,
			&TextureGradient.vkPipelineCache
		);

		if (vkResult != VK_SUCCESS)
		{
			fprintf(
				gpFILE,
				"createGraphicsPipeline_TextureGradient() : vkCreatePipelineCache() failed (%d).\n",
				vkResult
			);

			return(vkResult);
		}


		vkPipelineCacheList.push_back(
			TextureGradient.vkPipelineCache
		);
	}


	//------------------------------------------------------------------------
	// Compute pipeline
	//------------------------------------------------------------------------

	VkComputePipelineCreateInfo vkComputePipelineCreateInfo{};

	vkComputePipelineCreateInfo.sType =VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	vkComputePipelineCreateInfo.pNext = NULL;
	vkComputePipelineCreateInfo.flags = 0;
	vkComputePipelineCreateInfo.stage = vkPipelineShaderStageCreateInfo;
	vkComputePipelineCreateInfo.layout = TextureGradient.vkPipelineLayout;
	vkComputePipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	vkComputePipelineCreateInfo.basePipelineIndex = -1;


	//------------------------------------------------------------------------
	// Create compute pipeline
	//------------------------------------------------------------------------

	vkResult = vkCreateComputePipelines(
		gVulkanContext.vkDevice,
		TextureGradient.vkPipelineCache,
		1,
		&vkComputePipelineCreateInfo,
		NULL,
		&TextureGradient.vkPipeline
	);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(
			gpFILE,
			"createGraphicsPipeline_TextureGradient() : vkCreateComputePipelines() failed: %d.\n",
			vkResult
		);

		return(vkResult);
	}


	return(vkResult);
}



//Volumetric Clouds
VkResult ComputePipelines::createComputePipeline_VolumetricClouds(
	VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo
)
{
	// local variables
	VkResult vkResult = VK_SUCCESS;


	// Push constants
	VkPushConstantRange vkPushConstantRange{};
	vkPushConstantRange.offset = 0;
	vkPushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	vkPushConstantRange.size = sizeof(CloudPushConstants);


	vkResult = createPipelineLayout(
		vkPipelineLayoutCreateInfo,
		&VolumetricClouds.vkPipelineLayout,
		vkPushConstantRange
	);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(
			gpFILE,
			"createGraphicsPipeline_VolumetricClouds() : createPipelineLayout() failed (%d).\n",
			vkResult
		);

		return(vkResult);
	}


	//------------------------------------------------------------------------
	// Compute shader stage
	//------------------------------------------------------------------------

	VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo{};

	vkPipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo.pNext = NULL;
	vkPipelineShaderStageCreateInfo.flags = 0;
	vkPipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	vkPipelineShaderStageCreateInfo.module = VolumetricClouds.vkComputeShaderModule;
	vkPipelineShaderStageCreateInfo.pName = "main";
	vkPipelineShaderStageCreateInfo.pSpecializationInfo = NULL;


	//------------------------------------------------------------------------
	// Pipeline cache
	//------------------------------------------------------------------------

	if (VolumetricClouds.vkPipelineCache == VK_NULL_HANDLE)
	{
		VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo{};


		vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		vkPipelineCacheCreateInfo.pNext = NULL;
		vkPipelineCacheCreateInfo.flags = 0;
		vkPipelineCacheCreateInfo.initialDataSize = 0;
		vkPipelineCacheCreateInfo.pInitialData = NULL;


		vkResult = vkCreatePipelineCache(
			gVulkanContext.vkDevice,
			&vkPipelineCacheCreateInfo,
			NULL,
			&VolumetricClouds.vkPipelineCache
		);

		if (vkResult != VK_SUCCESS)
		{
			fprintf(
				gpFILE,
				"createGraphicsPipeline_VolumetricClouds() : vkCreatePipelineCache() failed (%d).\n",
				vkResult
			);

			return(vkResult);
		}


		vkPipelineCacheList.push_back(
			VolumetricClouds.vkPipelineCache
		);
	}


	//------------------------------------------------------------------------
	// Compute pipeline
	//------------------------------------------------------------------------

	VkComputePipelineCreateInfo vkComputePipelineCreateInfo{};

	vkComputePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	vkComputePipelineCreateInfo.pNext = NULL;
	vkComputePipelineCreateInfo.flags = 0;
	vkComputePipelineCreateInfo.stage = vkPipelineShaderStageCreateInfo;
	vkComputePipelineCreateInfo.layout = VolumetricClouds.vkPipelineLayout;
	vkComputePipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	vkComputePipelineCreateInfo.basePipelineIndex = -1;


	//------------------------------------------------------------------------
	// Create compute pipeline
	//------------------------------------------------------------------------

	vkResult = vkCreateComputePipelines(
		gVulkanContext.vkDevice,
		VolumetricClouds.vkPipelineCache,
		1,
		&vkComputePipelineCreateInfo,
		NULL,
		&VolumetricClouds.vkPipeline
	);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(
			gpFILE,
			"createGraphicsPipeline_VolumetricClouds() : vkCreateComputePipelines() failed: %d.\n",
			vkResult
		);

		return(vkResult);
	}


	return(vkResult);
}


//--------------------------------------------------------------------------------------------

VkResult ComputePipelines::createPipelines(void)
{
	// local variables
	VkResult vkResult = VK_SUCCESS;


	std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;

	VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo{};


	//--------------------------------------------------------------------------------------------
	// Texture Gradient
	//--------------------------------------------------------------------------------------------

	/*
	 * Put the descriptor set layout used by TextureGradient here.
	 *
	 * Example:
	 *
	 * vkDescriptorSetLayouts =
	 * {
	 *     gpDescriptorSetLayouts->vkDescriptorSetLayout_TextureGradient
	 * };
	 *
	 * The actual descriptor-set layout name must match
	 * DescriptorSetLayouts.h.
	 */

	//TextureGradient
	//-------------------------------------------------------------------------------------------
	vkDescriptorSetLayouts ={ gpDescriptorSetLayouts->vkDescriptorSetLayout_ComputeStorageImage };
	vkPipelineLayoutCreateInfo = {};

	vkPipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
	vkPipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts.data();


	vkResult = createComputePipeline_TextureGradient(vkPipelineLayoutCreateInfo);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(
			gpFILE,
			"createPipelines() : createComputePipeline_TextureGradient() failed: %d.\n",
			vkResult
		);

		return vkResult;
	}


	vkPipelineList.push_back(TextureGradient.vkPipeline);


	//Volumetric Clouds
	//--------------------------------------------------------------------------------------------

	vkDescriptorSetLayouts = { gpDescriptorSetLayouts->vkDescriptorSetLayout_frameData, gpDescriptorSetLayouts->vkDescriptorSetLayout_VolumetricClouds };
	vkPipelineLayoutCreateInfo = {};

	vkPipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
	vkPipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts.data();


	vkResult = createComputePipeline_VolumetricClouds(vkPipelineLayoutCreateInfo);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(
			gpFILE,
			"createPipelines() : createComputePipeline_VolumetricClouds() failed: %d.\n",
			vkResult
		);

		return vkResult;
	}


	vkPipelineList.push_back(VolumetricClouds.vkPipeline);


	//--------------------------------------------------------------------------------------------

	return vkResult;
}


void ComputePipelines::destroyPipelines(void)
{
	// destroy vkPipelineList
	for (int32_t i = (int32_t)vkPipelineList.size() - 1; i >= 0; i--)
	{
		if (vkPipelineList[i])
		{
			vkDestroyPipeline(
				gVulkanContext.vkDevice,
				vkPipelineList[i],
				NULL
			);

			vkPipelineList[i] = VK_NULL_HANDLE;
		}
	}

	vkPipelineList.clear();
}