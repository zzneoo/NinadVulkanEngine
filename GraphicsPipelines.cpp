#include "GraphicsPipelines.h"

GraphicsPipelines::GraphicsPipelines()
{
	vkResult = VK_SUCCESS;

    vkResult = createShaderModules();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "GraphicsPipelines() : createShaderModules() failed (%d).\n", vkResult);
        return;
	}

	vkResult = createPipelines();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "GraphicsPipelines() : createPipelines() failed (%d).\n", vkResult);
        return;
	}

}

GraphicsPipelines::~GraphicsPipelines()
{

	destroyPipelines();
    destroyPipelineLayouts();
	destroyPipelineCaches();
    destroyShaderModules();

}

VkResult GraphicsPipelines::createShaderModule(VkShaderModule* shaderModule, const char* fileName)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    FILE* fp = NULL;
    size_t size = 0;

    errno_t err = fopen_s(&fp, fileName, "rb");

    if (err != 0)
    {
        fprintf(gpFILE, "createShaders() -> fopen_s() :  failed.\n");
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
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }

    size_t retVal = fread(shaderData, size, 1, fp);

    if (retVal != 1)
    {
        fprintf(gpFILE, "createShaders() -> fread() :  failed.\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }


    fclose(fp);

    VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0;
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

    vkResult = vkCreateShaderModule(gVulkanContext.vkDevice, &vkShaderModuleCreateInfo, NULL, shaderModule);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createShaders() -> vkCreateShaderModule() :  failed.\n");
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
VkResult GraphicsPipelines::createShaderModules()
{
        VkResult vkResult = VK_SUCCESS;

        vkResult = createShaderModule(&ColoredVertex.vkVertexShaderModule, "shader.vert.spv");
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createShaders() -> createShaderModule() for vertex shader failed.\n");
            return vkResult;
        }

        vkResult = createShaderModule(&ColoredVertex.vkFragmentShaderModule, "shader.frag.spv");
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createShaders() -> createShaderModule() for fragment shader failed.\n");
            return vkResult;
        }

        //vkShaderModule_whiteVertex_vs
        vkResult = createShaderModule(&WhiteVertex.vkVertexShaderModule, "whiteVertex.vert.spv");
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createShaders() -> createShaderModule() for white vertex shader failed.\n");
            return vkResult;
        }

        //vkShaderModule_whiteFragment_fs
        vkResult = createShaderModule(&WhiteVertex.vkFragmentShaderModule, "whiteVertex.frag.spv");
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createShaders() -> createShaderModule() for white fragment shader failed.\n");
            return vkResult;
        }

        //----------

        //vkShaderModule_previewImage_vs
        vkResult = createShaderModule(&PreviewImage.vkVertexShaderModule, "PreviewImage.vert.spv");
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createShaders() -> createShaderModule() for preview image vertex shader failed.\n");
            return vkResult;
        }

        //vkShaderModule_previewImage_fs
        vkResult = createShaderModule(&PreviewImage.vkFragmentShaderModule, "PreviewImage.frag.spv");
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createShaders() -> createShaderModule() for preview image fragment shader failed.\n");
            return vkResult;
        }

        //----------

        //vkShaderModule_impostor_vs
        vkResult = createShaderModule(&Impostor.vkVertexShaderModule, "Impostor.vert.spv");
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createShaders() -> createShaderModule() for impostor vertex shader failed.\n");
            return vkResult;
        }

        //vkShaderModule_impostor_fs
        vkResult = createShaderModule(&Impostor.vkFragmentShaderModule, "Impostor.frag.spv");
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createShaders() -> createShaderModule() for impostor fragment shader failed.\n");
            return vkResult;
        }

        //----------phongshader--------------
        vkResult = createShaderModule(&Phong.vkVertexShaderModule, "Phong.vert.spv");
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createShaders() -> createShaderModule() for phong vertex shader failed.\n");
            return vkResult;
        }

        vkResult = createShaderModule(&Phong.vkFragmentShaderModule, "Phong.frag.spv");
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createShaders() -> createShaderModule() for phong fragment shader failed.\n");
            return vkResult;
        }

        //-----------PBR shader----------------
        vkResult = createShaderModule(&PBR.vkVertexShaderModule, "PBR.vert.spv");
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createShaders() -> createShaderModule() for pbr vertex shader failed.\n");
            return vkResult;
        }
        vkResult = createShaderModule(&PBR.vkFragmentShaderModule, "PBR.frag.spv");
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createShaders() -> createShaderModule() for pbr fragment shader failed.\n");
            return vkResult;
        }

        //PBR_Skinned
        vkResult = createShaderModule(&PBR_Skinned.vkVertexShaderModule, "PBR_Skinned.vert.spv");
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createShaders() -> createShaderModule() for pbr skinned vertex shader failed.\n");
            return vkResult;
        }
        vkResult = createShaderModule(&PBR_Skinned.vkFragmentShaderModule, "PBR_Skinned.frag.spv");
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createShaders() -> createShaderModule() for pbr skinned fragment shader failed.\n");
            return vkResult;
        }

		return(vkResult);   
}

void GraphicsPipelines::destroyShaderModules(void)
{

    // destroy shader modules from the list in reverse order
    for (int32_t i = (int32_t)vkShaderModuleList.size() - 1; i >= 0; i--)
    {
        if (vkShaderModuleList[i])
        {
            vkDestroyShaderModule(gVulkanContext.vkDevice, vkShaderModuleList[i], NULL);
            vkShaderModuleList[i] = VK_NULL_HANDLE;
        }
    }

	vkShaderModuleList.clear();

}

void GraphicsPipelines::destroyPipelineCaches(void)
{
    // destroy pipeline caches from the list in reverse order
    for (int32_t i = (int32_t)vkPipelineCacheList.size() - 1; i >= 0; i--)
    {
        if (vkPipelineCacheList[i])
        {
            vkDestroyPipelineCache(gVulkanContext.vkDevice, vkPipelineCacheList[i], NULL);
            vkPipelineCacheList[i] = VK_NULL_HANDLE;
        }
    }
	vkPipelineCacheList.clear();
}

//----------------------------Pipeline Layouts------------------------------------------------

VkResult GraphicsPipelines::createPipelineLayout(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo,VkPipelineLayout *vkPipelineLayout)

{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    //push constant
// Declare and initialize VkPushConstantRange structure which will have information about the push constant range.
    VkPushConstantRange vkPushConstantRange;
    memset((void*)&vkPushConstantRange, 0, sizeof(VkPushConstantRange));
    vkPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // stage flags for the push constant range
    vkPushConstantRange.offset = 0; // offset in the push constant range
    vkPushConstantRange.size = sizeof(PushConstants); // size of the push constant range

    // Declare and initialize VkPipelineLayoutCreateInfo structure which will have information about the pipeline layout.
    //VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
    //memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));


    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.pNext = NULL;
    vkPipelineLayoutCreateInfo.flags = 0;
    //vkPipelineLayoutCreateInfo.setLayoutCount = setLayoutCount;// two descriptor set layout for imposter pipeline layout
    //vkPipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts; // pointer to the descriptor set layout
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 1; // one push constant range for imposter pipeline layout
    vkPipelineLayoutCreateInfo.pPushConstantRanges = &vkPushConstantRange; // pointer to the push constant range

    vkResult = vkCreatePipelineLayout(gVulkanContext.vkDevice, &vkPipelineLayoutCreateInfo, NULL, vkPipelineLayout);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createPipelinnLayout_PBR_Skinned() -> vkCreatePipelineLayout() :  failed: %d.\n", vkResult);
        return(vkResult);
    }

	vkPipelineLayoutList.push_back(*vkPipelineLayout);

    return(vkResult);
}

void GraphicsPipelines::destroyPipelineLayouts(void)
{

	//destroy vkPipelineLayoutList
	for (int32_t i = (int32_t)vkPipelineLayoutList.size() - 1; i >= 0; i--)
    {
        if (vkPipelineLayoutList[i])
        {
            vkDestroyPipelineLayout(gVulkanContext.vkDevice, vkPipelineLayoutList[i], NULL);
            vkPipelineLayoutList[i] = VK_NULL_HANDLE;
        }
	}

	vkPipelineLayoutList.clear();

}

//----------------------------Pipelines-------------------------------------------------------

VkResult GraphicsPipelines::createGraphicsPipeline_Impostor(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

	vkResult = createPipelineLayout(vkPipelineLayoutCreateInfo, &Impostor.vkPipelineLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createGraphicsPipeline_Impostor() : createPipelineLayout() failed (%d).\n", vkResult);
		return(vkResult);
	}


    //vertex input state
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1];
    memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

    //position
    vkVertexInputBindingDescription_array[0].binding = 0; // VDG's location = 0
    vkVertexInputBindingDescription_array[0].stride = sizeof(VertexData_PositionTexCoord); // size of each vertex
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex data

    //vertex input attribute state
    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[2];
    memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));

    //position
    vkVertexInputAttributeDescription_array[0].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[0].location = 0; // location index
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[0].offset = offsetof(VertexData_PositionTexCoord, pos); // offset of each vertex

    //uv
    vkVertexInputAttributeDescription_array[1].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[1].location = 1; // location index
    vkVertexInputAttributeDescription_array[1].format = VK_FORMAT_R32G32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[1].offset = offsetof(VertexData_PositionTexCoord, texCoord); // offset of each vertex

    //  Declare and initialize VkPipelineVertexInputStateCreateInfo structure.
    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
    memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    vkPipelineVertexInputStateCreateInfo.flags = 0;
    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;

    //  Declare and initialize VkPipelineInputAssemblyStateCreateInfo structure.
    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
    memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
    vkPipelineInputAssemblyStateCreateInfo.flags = 0;
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // triangle list
    vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; // no primitive restart


    //  Declare and initialize VkPipelineRasterizationStateCreateInfo structure.
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE; // no depth clamp
    vkPipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE; // no depth bias
    vkPipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f; // no depth bias
    vkPipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f; // no depth bias clamp
    vkPipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f; // no depth bias slope factor
    vkPipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; // no rasterizer discard
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // fill mode
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT; // back face culling
    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //  anti clockwise front face
    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f; // line width

    // Color blend state
    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
    memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));
    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE; // enable blending
    vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // all color components
    vkPipelineColorBlendAttachmentState_array[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // source color blend factor
    vkPipelineColorBlendAttachmentState_array[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // destination color blend factor
    vkPipelineColorBlendAttachmentState_array[0].colorBlendOp = VK_BLEND_OP_ADD; // color blend operation
    vkPipelineColorBlendAttachmentState_array[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // source alpha blend factor
    vkPipelineColorBlendAttachmentState_array[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // destination alpha blend factor
    vkPipelineColorBlendAttachmentState_array[0].alphaBlendOp = VK_BLEND_OP_ADD; // alpha blend operation




    //ColorBlendStateCreateInfo
    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
    memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStateCreateInfo.pNext = NULL;
    vkPipelineColorBlendStateCreateInfo.flags = 0;
    vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
    vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;
    vkPipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE; // no logic op

    //viewport sciccor state
    memset((void*)&vkViewport, 0, sizeof(VkViewport));
    vkViewport.x = 0.0f;
    vkViewport.y = 0.0f;
    vkViewport.width = (float)gSwapchain.vkExtent2D.width;
    vkViewport.height = (float)gSwapchain.vkExtent2D.height;
    vkViewport.minDepth = 0.0f;
    vkViewport.maxDepth = 1.0f;

    memset((void*)&vkRect2D_Scissor, 0, sizeof(VkRect2D));
    vkRect2D_Scissor.offset.x = 0;
    vkRect2D_Scissor.offset.y = 0;
    vkRect2D_Scissor.extent.width = gSwapchain.vkExtent2D.width;
    vkRect2D_Scissor.extent.height = gSwapchain.vkExtent2D.height;

    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
    memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStateCreateInfo.pNext = NULL;
    vkPipelineViewportStateCreateInfo.flags = 0;
    vkPipelineViewportStateCreateInfo.viewportCount = 1; // 1 viewport
    vkPipelineViewportStateCreateInfo.pViewports = &vkViewport; // viewport
    vkPipelineViewportStateCreateInfo.scissorCount = 1; // 1 scissor
    vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_Scissor; // scissor

    //// depth stencil state
    //VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;   
    //memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    //vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    //vkPipelineDepthStencilStateCreateInfo.pNext = NULL;

    //depth stencil state
    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;
    memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    vkPipelineDepthStencilStateCreateInfo.pNext = NULL;
    vkPipelineDepthStencilStateCreateInfo.flags = 0;
    vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE; // enable depth test
    vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE; // enable depth write
    vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS; // depth compare operation
    vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE; // no depth bounds test
    vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE; // no stencil test
    //vkPipelineDepthStencilStateCreateInfo.front = {}; // no front stencil state
    //vkPipelineDepthStencilStateCreateInfo.back = {}; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS; // no back stencil state

    vkPipelineDepthStencilStateCreateInfo.front = vkPipelineDepthStencilStateCreateInfo.back; // use the same state for front and back

    vkPipelineDepthStencilStateCreateInfo.minDepthBounds = 0.0f; // min depth bounds
    vkPipelineDepthStencilStateCreateInfo.maxDepthBounds = 1.0f; // max depth bounds



    //dynamic state (viewport, scissor ,depth bias ,blend constants, stensil mask,line width, etc)
    //no dynamic state right now

    //multisample state
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStateCreateInfo.pNext = NULL;
    vkPipelineMultisampleStateCreateInfo.flags = 0;
    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // 1 sample
    //vkPipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE; // no sample shading
    //vkPipelineMultisampleStateCreateInfo.minSampleShading = 0.0f; // no min sample shading
    //vkPipelineMultisampleStateCreateInfo.pSampleMask = NULL; // no sample mask
    //vkPipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_TRUE; // no alpha to coverage
    //vkPipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // no alpha to one


    //shader stage state
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo[2];
    memset((void*)&vkPipelineShaderStageCreateInfo, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo));
    //vertex shader stage
    vkPipelineShaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[0].pNext = NULL;
    vkPipelineShaderStageCreateInfo[0].flags = 0;
    vkPipelineShaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT; // vertex shader
    vkPipelineShaderStageCreateInfo[0].module = Impostor.vkVertexShaderModule; // vertex shader module
    vkPipelineShaderStageCreateInfo[0].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[0].pSpecializationInfo = NULL; // no specialization info
    //fragment shader stage
    vkPipelineShaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo[1].flags = 0;
    vkPipelineShaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; // fragment shader
    vkPipelineShaderStageCreateInfo[1].module = Impostor.vkFragmentShaderModule; // fragment shader module
    vkPipelineShaderStageCreateInfo[1].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[1].pSpecializationInfo = NULL; // no specialization info

    //tesselation state
    //no tessellation state right now

    if(Impostor.vkPipelineCache == VK_NULL_HANDLE)
    {
        //create pipeline cache
        VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
        memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
        vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        vkPipelineCacheCreateInfo.pNext = NULL;
        vkPipelineCacheCreateInfo.flags = 0;
        vkPipelineCacheCreateInfo.initialDataSize = 0;
        vkPipelineCacheCreateInfo.pInitialData = NULL;
        vkResult = vkCreatePipelineCache(gVulkanContext.vkDevice, &vkPipelineCacheCreateInfo, NULL, &Impostor.vkPipelineCache);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createGraphicsPipeline_Impostor() : vkCreatePipelineCache() failed (%d).\n", vkResult);
            return(vkResult);
        }
		vkPipelineCacheList.push_back(Impostor.vkPipelineCache);
	}

    VkPipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

    VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &colorFormat;

    renderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
    renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;



    //  Declare and initialize VkGraphicsPipelineCreateInfo structure.
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vkGraphicsPipelineCreateInfo.pNext = &renderingInfo; // pointer to the rendering info structure for dynamic rendering
    vkGraphicsPipelineCreateInfo.flags = 0;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pTessellationState = NULL; // no tessellation state
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDynamicState = NULL; // no dynamic state
    vkGraphicsPipelineCreateInfo.layout = Impostor.vkPipelineLayout; // pipeline layout
	vkGraphicsPipelineCreateInfo.renderPass = VK_NULL_HANDLE; // no render pass
    vkGraphicsPipelineCreateInfo.subpass = 0; // subpass index
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no base pipeline
    vkGraphicsPipelineCreateInfo.basePipelineIndex = -1; // no base pipeline index
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo); // number of shader stages
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo; // shader stages


    //  Call vkCreateGraphicsPipelines() API to create the graphics pipeline.
    vkResult = vkCreateGraphicsPipelines(gVulkanContext.vkDevice, Impostor.vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &Impostor.vkPipeline);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipeline() : vkCreateGraphicsPipelines() failed: %d .\n", vkResult);
        return(vkResult);
    }

    return(vkResult);
}

VkResult GraphicsPipelines::createGraphicsPipeline_Phong(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

	vkResult = createPipelineLayout(vkPipelineLayoutCreateInfo, &Phong.vkPipelineLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createGraphicsPipeline_Phong() : createPipelineLayout() failed (%d).\n", vkResult);
		return(vkResult);
	}


    //vertex input state
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1];
    memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

    //position
    vkVertexInputBindingDescription_array[0].binding = 0; // VDG's location = 0
    vkVertexInputBindingDescription_array[0].stride = sizeof(VertexData_PositionTexCoordNormalColor); // size of each vertex
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex data

    //vertex input attribute state
    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[4];
    memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));

    //position
    vkVertexInputAttributeDescription_array[0].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[0].location = 0; // location index
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[0].offset = offsetof(VertexData_PositionTexCoordNormalColor, pos); // offset of each vertex

    //uv
    vkVertexInputAttributeDescription_array[1].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[1].location = 1; // location index
    vkVertexInputAttributeDescription_array[1].format = VK_FORMAT_R32G32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[1].offset = offsetof(VertexData_PositionTexCoordNormalColor, texCoord); // offset of each vertex

    //normal
    vkVertexInputAttributeDescription_array[2].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[2].location = 2; // location index
    vkVertexInputAttributeDescription_array[2].format = VK_FORMAT_R32G32B32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[2].offset = offsetof(VertexData_PositionTexCoordNormalColor, normal); // offset of each vertex

    //color
    vkVertexInputAttributeDescription_array[3].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[3].location = 3; // location index
    vkVertexInputAttributeDescription_array[3].format = VK_FORMAT_R32G32B32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[3].offset = offsetof(VertexData_PositionTexCoordNormalColor, color); // offset of each vertex

    //  Declare and initialize VkPipelineVertexInputStateCreateInfo structure.
    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
    memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    vkPipelineVertexInputStateCreateInfo.flags = 0;
    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;

    //  Declare and initialize VkPipelineInputAssemblyStateCreateInfo structure.
    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
    memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
    vkPipelineInputAssemblyStateCreateInfo.flags = 0;
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // triangle list
    vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; // no primitive restart


    //  Declare and initialize VkPipelineRasterizationStateCreateInfo structure.
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE; // no depth clamp
    vkPipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE; // no depth bias
    vkPipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f; // no depth bias
    vkPipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f; // no depth bias clamp
    vkPipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f; // no depth bias slope factor
    vkPipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; // no rasterizer discard
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // fill mode
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT; // back face culling
    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //  anti clockwise front face
    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f; // line width

    // Color blend state
    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
    memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));
    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE; // enable blending
    vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // all color components
    vkPipelineColorBlendAttachmentState_array[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // source color blend factor
    vkPipelineColorBlendAttachmentState_array[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // destination color blend factor
    vkPipelineColorBlendAttachmentState_array[0].colorBlendOp = VK_BLEND_OP_ADD; // color blend operation
    vkPipelineColorBlendAttachmentState_array[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // source alpha blend factor
    vkPipelineColorBlendAttachmentState_array[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // destination alpha blend factor
    vkPipelineColorBlendAttachmentState_array[0].alphaBlendOp = VK_BLEND_OP_ADD; // alpha blend operation




    //ColorBlendStateCreateInfo
    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
    memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStateCreateInfo.pNext = NULL;
    vkPipelineColorBlendStateCreateInfo.flags = 0;
    vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
    vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;
    vkPipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE; // no logic op

    //viewport sciccor state
    memset((void*)&vkViewport, 0, sizeof(VkViewport));
    vkViewport.x = 0.0f;
    vkViewport.y = 0.0f;
    vkViewport.width = (float)gSwapchain.vkExtent2D.width;
    vkViewport.height = (float)gSwapchain.vkExtent2D.height;
    vkViewport.minDepth = 0.0f;
    vkViewport.maxDepth = 1.0f;

    memset((void*)&vkRect2D_Scissor, 0, sizeof(VkRect2D));
    vkRect2D_Scissor.offset.x = 0;
    vkRect2D_Scissor.offset.y = 0;
    vkRect2D_Scissor.extent.width = gSwapchain.vkExtent2D.width;
    vkRect2D_Scissor.extent.height = gSwapchain.vkExtent2D.height;

    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
    memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStateCreateInfo.pNext = NULL;
    vkPipelineViewportStateCreateInfo.flags = 0;
    vkPipelineViewportStateCreateInfo.viewportCount = 1; // 1 viewport
    vkPipelineViewportStateCreateInfo.pViewports = &vkViewport; // viewport
    vkPipelineViewportStateCreateInfo.scissorCount = 1; // 1 scissor
    vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_Scissor; // scissor

    //// depth stencil state
    //VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;   
    //memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    //vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    //vkPipelineDepthStencilStateCreateInfo.pNext = NULL;

    //depth stencil state
    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;
    memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    vkPipelineDepthStencilStateCreateInfo.pNext = NULL;
    vkPipelineDepthStencilStateCreateInfo.flags = 0;
    vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE; // enable depth test
    vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE; // enable depth write
    vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS; // depth compare operation
    vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE; // no depth bounds test
    vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE; // no stencil test
    //vkPipelineDepthStencilStateCreateInfo.front = {}; // no front stencil state
    //vkPipelineDepthStencilStateCreateInfo.back = {}; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS; // no back stencil state

    vkPipelineDepthStencilStateCreateInfo.front = vkPipelineDepthStencilStateCreateInfo.back; // use the same state for front and back

    vkPipelineDepthStencilStateCreateInfo.minDepthBounds = 0.0f; // min depth bounds
    vkPipelineDepthStencilStateCreateInfo.maxDepthBounds = 1.0f; // max depth bounds



    //dynamic state (viewport, scissor ,depth bias ,blend constants, stensil mask,line width, etc)
    //no dynamic state right now

    //multisample state
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStateCreateInfo.pNext = NULL;
    vkPipelineMultisampleStateCreateInfo.flags = 0;
    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // 1 sample
    //vkPipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE; // no sample shading
    //vkPipelineMultisampleStateCreateInfo.minSampleShading = 0.0f; // no min sample shading
    //vkPipelineMultisampleStateCreateInfo.pSampleMask = NULL; // no sample mask
    //vkPipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_TRUE; // no alpha to coverage
    //vkPipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // no alpha to one


    //shader stage state
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo[2];
    memset((void*)&vkPipelineShaderStageCreateInfo, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo));
    //vertex shader stage
    vkPipelineShaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[0].pNext = NULL;
    vkPipelineShaderStageCreateInfo[0].flags = 0;
    vkPipelineShaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT; // vertex shader
    vkPipelineShaderStageCreateInfo[0].module = Phong.vkVertexShaderModule; // vertex shader module
    vkPipelineShaderStageCreateInfo[0].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[0].pSpecializationInfo = NULL; // no specialization info
    //fragment shader stage
    vkPipelineShaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo[1].flags = 0;
    vkPipelineShaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; // fragment shader
    vkPipelineShaderStageCreateInfo[1].module = Phong.vkFragmentShaderModule; // fragment shader module
    vkPipelineShaderStageCreateInfo[1].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[1].pSpecializationInfo = NULL; // no specialization info

    //tesselation state
    //no tessellation state right now


    //pipeline cache 
    if(Phong.vkPipelineCache == VK_NULL_HANDLE)
    {
        //create pipeline cache
        VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
        memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
        vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        vkPipelineCacheCreateInfo.pNext = NULL;
        vkPipelineCacheCreateInfo.flags = 0;
        vkPipelineCacheCreateInfo.initialDataSize = 0;
        vkPipelineCacheCreateInfo.pInitialData = NULL;
        vkResult = vkCreatePipelineCache(gVulkanContext.vkDevice, &vkPipelineCacheCreateInfo, NULL, &Phong.vkPipelineCache);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createGraphicsPipeline_Phong() : vkCreatePipelineCache() failed (%d).\n", vkResult);
            return(vkResult);
		}
		vkPipelineCacheList.push_back(Phong.vkPipelineCache);
	}

    VkPipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

    VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &colorFormat;

    renderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
    renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;


    //  Declare and initialize VkGraphicsPipelineCreateInfo structure.
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vkGraphicsPipelineCreateInfo.pNext = &renderingInfo; // pointer to the rendering info structure for dynamic rendering
    vkGraphicsPipelineCreateInfo.flags = 0;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pTessellationState = NULL; // no tessellation state
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDynamicState = NULL; // no dynamic state
    vkGraphicsPipelineCreateInfo.layout = Phong.vkPipelineLayout; // pipeline layout
	vkGraphicsPipelineCreateInfo.renderPass = VK_NULL_HANDLE; // no render pass
    vkGraphicsPipelineCreateInfo.subpass = 0; // subpass index
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no base pipeline
    vkGraphicsPipelineCreateInfo.basePipelineIndex = -1; // no base pipeline index
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo); // number of shader stages
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo; // shader stages


    //  Call vkCreateGraphicsPipelines() API to create the graphics pipeline.
    vkResult = vkCreateGraphicsPipelines(gVulkanContext.vkDevice, Phong.vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &Phong.vkPipeline);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipeline() : vkCreateGraphicsPipelines() failed: %d .\n", vkResult);
        return(vkResult);
    }

    return(vkResult);
}

VkResult GraphicsPipelines::createGraphicsPipeline_PBR(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

	vkResult = createPipelineLayout(vkPipelineLayoutCreateInfo,&PBR.vkPipelineLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createGraphicsPipeline_PBR() : createPipelineLayout() failed: %d .\n", vkResult);
		return(vkResult);
	}

    //vertex input state
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1];
    memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

    //VertexData_PositionTexCoordNormalTangent
    vkVertexInputBindingDescription_array[0].binding = 0; // VDG's location = 0
    vkVertexInputBindingDescription_array[0].stride = sizeof(VertexData_PositionTexCoordNormalTangent); // size of each vertex
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex data

    //vertex input attribute state
    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[4];
    memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));

    //position
    vkVertexInputAttributeDescription_array[0].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[0].location = 0; // location index
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[0].offset = offsetof(VertexData_PositionTexCoordNormalTangent, pos); // offset of each vertex

    //uv
    vkVertexInputAttributeDescription_array[1].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[1].location = 1; // location index
    vkVertexInputAttributeDescription_array[1].format = VK_FORMAT_R32G32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[1].offset = offsetof(VertexData_PositionTexCoordNormalTangent, texCoord); // offset of each vertex

    //normal
    vkVertexInputAttributeDescription_array[2].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[2].location = 2; // location index
    vkVertexInputAttributeDescription_array[2].format = VK_FORMAT_R32G32B32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[2].offset = offsetof(VertexData_PositionTexCoordNormalTangent, normal); // offset of each vertex

    //tangent
    vkVertexInputAttributeDescription_array[3].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[3].location = 3; // location index
    vkVertexInputAttributeDescription_array[3].format = VK_FORMAT_R32G32B32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[3].offset = offsetof(VertexData_PositionTexCoordNormalTangent, tangent); // offset of each vertex

    //  Declare and initialize VkPipelineVertexInputStateCreateInfo structure.
    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
    memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    vkPipelineVertexInputStateCreateInfo.flags = 0;
    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;

    //  Declare and initialize VkPipelineInputAssemblyStateCreateInfo structure.
    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
    memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
    vkPipelineInputAssemblyStateCreateInfo.flags = 0;
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // triangle list
    vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; // no primitive restart


    //  Declare and initialize VkPipelineRasterizationStateCreateInfo structure.
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE; // no depth clamp
    vkPipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE; // no depth bias
    vkPipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f; // no depth bias
    vkPipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f; // no depth bias clamp
    vkPipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f; // no depth bias slope factor
    vkPipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; // no rasterizer discard
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // fill mode
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT; // back face culling
    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //  anti clockwise front face
    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f; // line width

    // Color blend state
    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
    memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));
    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE; // enable blending
    vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // all color components
    vkPipelineColorBlendAttachmentState_array[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // source color blend factor
    vkPipelineColorBlendAttachmentState_array[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // destination color blend factor
    vkPipelineColorBlendAttachmentState_array[0].colorBlendOp = VK_BLEND_OP_ADD; // color blend operation
    vkPipelineColorBlendAttachmentState_array[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // source alpha blend factor
    vkPipelineColorBlendAttachmentState_array[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // destination alpha blend factor
    vkPipelineColorBlendAttachmentState_array[0].alphaBlendOp = VK_BLEND_OP_ADD; // alpha blend operation




    //ColorBlendStateCreateInfo
    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
    memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStateCreateInfo.pNext = NULL;
    vkPipelineColorBlendStateCreateInfo.flags = 0;
    vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
    vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;
    vkPipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE; // no logic op

    //viewport sciccor state
    memset((void*)&vkViewport, 0, sizeof(VkViewport));
    vkViewport.x = 0.0f;
    vkViewport.y = 0.0f;
    vkViewport.width = (float)gSwapchain.vkExtent2D.width;
    vkViewport.height = (float)gSwapchain.vkExtent2D.height;
    vkViewport.minDepth = 0.0f;
    vkViewport.maxDepth = 1.0f;

    memset((void*)&vkRect2D_Scissor, 0, sizeof(VkRect2D));
    vkRect2D_Scissor.offset.x = 0;
    vkRect2D_Scissor.offset.y = 0;
    vkRect2D_Scissor.extent.width = gSwapchain.vkExtent2D.width;
    vkRect2D_Scissor.extent.height = gSwapchain.vkExtent2D.height;

    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
    memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStateCreateInfo.pNext = NULL;
    vkPipelineViewportStateCreateInfo.flags = 0;
    vkPipelineViewportStateCreateInfo.viewportCount = 1; // 1 viewport
    vkPipelineViewportStateCreateInfo.pViewports = &vkViewport; // viewport
    vkPipelineViewportStateCreateInfo.scissorCount = 1; // 1 scissor
    vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_Scissor; // scissor

    //// depth stencil state
    //VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;   
    //memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    //vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    //vkPipelineDepthStencilStateCreateInfo.pNext = NULL;

    //depth stencil state
    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;
    memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    vkPipelineDepthStencilStateCreateInfo.pNext = NULL;
    vkPipelineDepthStencilStateCreateInfo.flags = 0;
    vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE; // enable depth test
    vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE; // enable depth write
    vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS; // depth compare operation
    vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE; // no depth bounds test
    vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE; // no stencil test
    //vkPipelineDepthStencilStateCreateInfo.front = {}; // no front stencil state
    //vkPipelineDepthStencilStateCreateInfo.back = {}; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS; // no back stencil state

    vkPipelineDepthStencilStateCreateInfo.front = vkPipelineDepthStencilStateCreateInfo.back; // use the same state for front and back

    vkPipelineDepthStencilStateCreateInfo.minDepthBounds = 0.0f; // min depth bounds
    vkPipelineDepthStencilStateCreateInfo.maxDepthBounds = 1.0f; // max depth bounds



    //dynamic state (viewport, scissor ,depth bias ,blend constants, stensil mask,line width, etc)
    //no dynamic state right now

    //multisample state
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStateCreateInfo.pNext = NULL;
    vkPipelineMultisampleStateCreateInfo.flags = 0;
    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // 1 sample
    //vkPipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE; // no sample shading
    //vkPipelineMultisampleStateCreateInfo.minSampleShading = 0.0f; // no min sample shading
    //vkPipelineMultisampleStateCreateInfo.pSampleMask = NULL; // no sample mask
    //vkPipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_TRUE; // no alpha to coverage
    //vkPipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // no alpha to one


    //shader stage state
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo[2];
    memset((void*)&vkPipelineShaderStageCreateInfo, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo));
    //vertex shader stage
    vkPipelineShaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[0].pNext = NULL;
    vkPipelineShaderStageCreateInfo[0].flags = 0;
    vkPipelineShaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT; // vertex shader
    vkPipelineShaderStageCreateInfo[0].module = PBR.vkVertexShaderModule; // vertex shader module
    vkPipelineShaderStageCreateInfo[0].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[0].pSpecializationInfo = NULL; // no specialization info
    //fragment shader stage
    vkPipelineShaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo[1].flags = 0;
    vkPipelineShaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; // fragment shader
    vkPipelineShaderStageCreateInfo[1].module = PBR.vkFragmentShaderModule; // fragment shader module
    vkPipelineShaderStageCreateInfo[1].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[1].pSpecializationInfo = NULL; // no specialization info

    //tesselation state
    //no tessellation state right now


    //pipeline cache 

    if(PBR.vkPipelineCache == VK_NULL_HANDLE)
    {
        //create pipeline cache
        VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
        memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
        vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        vkPipelineCacheCreateInfo.pNext = NULL;
        vkPipelineCacheCreateInfo.flags = 0;
        vkPipelineCacheCreateInfo.initialDataSize = 0;
        vkPipelineCacheCreateInfo.pInitialData = NULL;
        vkResult = vkCreatePipelineCache(gVulkanContext.vkDevice, &vkPipelineCacheCreateInfo, NULL, &PBR.vkPipelineCache);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createGraphicsPipeline_PBR() : vkCreatePipelineCache() failed (%d).\n", vkResult);
            return(vkResult);
        }
		vkPipelineCacheList.push_back(PBR.vkPipelineCache);
	}

    VkPipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

    VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &colorFormat;

    renderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
    renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    //  Declare and initialize VkGraphicsPipelineCreateInfo structure.
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vkGraphicsPipelineCreateInfo.pNext = &renderingInfo; // pointer to the rendering info structure for dynamic rendering
    vkGraphicsPipelineCreateInfo.flags = 0;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pTessellationState = NULL; // no tessellation state
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDynamicState = NULL; // no dynamic state
    vkGraphicsPipelineCreateInfo.layout = PBR.vkPipelineLayout; // pipeline layout
	vkGraphicsPipelineCreateInfo.renderPass = VK_NULL_HANDLE; // no render pass
    vkGraphicsPipelineCreateInfo.subpass = 0; // subpass index
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no base pipeline
    vkGraphicsPipelineCreateInfo.basePipelineIndex = -1; // no base pipeline index
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo); // number of shader stages
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo; // shader stages


    //  Call vkCreateGraphicsPipelines() API to create the graphics pipeline.
    vkResult = vkCreateGraphicsPipelines(gVulkanContext.vkDevice, PBR.vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &PBR.vkPipeline);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipeline() : vkCreateGraphicsPipelines() failed: %d .\n", vkResult);
        return(vkResult);
    }

    return(vkResult);
}

VkResult GraphicsPipelines::createGraphicsPipeline_PBR_Skinned(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;


	vkResult = createPipelineLayout(vkPipelineLayoutCreateInfo, &PBR_Skinned.vkPipelineLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createGraphicsPipeline_PBR_Skinned() : createPipelineLayout() failed: %d .\n", vkResult);
		return(vkResult);
	}


    //vertex input state
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1];
    memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

    //VertexData_Skinned
    vkVertexInputBindingDescription_array[0].binding = 0; // VDG's location = 0
    vkVertexInputBindingDescription_array[0].stride = sizeof(VertexData_Skinned); // size of each vertex
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex data

    //vertex input attribute state
    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[6];
    memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));

    //position
    vkVertexInputAttributeDescription_array[0].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[0].location = 0; // location index
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[0].offset = offsetof(VertexData_Skinned, pos); // offset of each vertex

    //uv
    vkVertexInputAttributeDescription_array[1].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[1].location = 1; // location index
    vkVertexInputAttributeDescription_array[1].format = VK_FORMAT_R32G32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[1].offset = offsetof(VertexData_Skinned, texCoord); // offset of each vertex

    //normal
    vkVertexInputAttributeDescription_array[2].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[2].location = 2; // location index
    vkVertexInputAttributeDescription_array[2].format = VK_FORMAT_R32G32B32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[2].offset = offsetof(VertexData_Skinned, normal); // offset of each vertex

    //tangent
    vkVertexInputAttributeDescription_array[3].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[3].location = 3; // location index
    vkVertexInputAttributeDescription_array[3].format = VK_FORMAT_R32G32B32A32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[3].offset = offsetof(VertexData_Skinned, tangent); // offset of each vertex

	//boneIDs
	vkVertexInputAttributeDescription_array[4].binding = 0; // binding index
	vkVertexInputAttributeDescription_array[4].location = 4; // location index
	vkVertexInputAttributeDescription_array[4].format = VK_FORMAT_R32G32B32A32_SINT; // format of each vertex
	vkVertexInputAttributeDescription_array[4].offset = offsetof(VertexData_Skinned, boneIDs); // offset of each vertex

	//boneWeights
	vkVertexInputAttributeDescription_array[5].binding = 0; // binding index
	vkVertexInputAttributeDescription_array[5].location = 5; // location index
	vkVertexInputAttributeDescription_array[5].format = VK_FORMAT_R32G32B32A32_SFLOAT; // format of each vertex
	vkVertexInputAttributeDescription_array[5].offset = offsetof(VertexData_Skinned, boneWeights); // offset of each vertex


    //  Declare and initialize VkPipelineVertexInputStateCreateInfo structure.
    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
    memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    vkPipelineVertexInputStateCreateInfo.flags = 0;
    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;

    //  Declare and initialize VkPipelineInputAssemblyStateCreateInfo structure.
    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
    memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
    vkPipelineInputAssemblyStateCreateInfo.flags = 0;
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // triangle list
    vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; // no primitive restart


    //  Declare and initialize VkPipelineRasterizationStateCreateInfo structure.
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE; // no depth clamp
    vkPipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE; // no depth bias
    vkPipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f; // no depth bias
    vkPipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f; // no depth bias clamp
    vkPipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f; // no depth bias slope factor
    vkPipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; // no rasterizer discard
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // fill mode
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT; // back face culling
    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //  anti clockwise front face
    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f; // line width

    // Color blend state
    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
    memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));
    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE; // enable blending
    vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // all color components
    vkPipelineColorBlendAttachmentState_array[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // source color blend factor
    vkPipelineColorBlendAttachmentState_array[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // destination color blend factor
    vkPipelineColorBlendAttachmentState_array[0].colorBlendOp = VK_BLEND_OP_ADD; // color blend operation
    vkPipelineColorBlendAttachmentState_array[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // source alpha blend factor
    vkPipelineColorBlendAttachmentState_array[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // destination alpha blend factor
    vkPipelineColorBlendAttachmentState_array[0].alphaBlendOp = VK_BLEND_OP_ADD; // alpha blend operation




    //ColorBlendStateCreateInfo
    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
    memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStateCreateInfo.pNext = NULL;
    vkPipelineColorBlendStateCreateInfo.flags = 0;
    vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
    vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;
    vkPipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE; // no logic op

    //viewport sciccor state
    memset((void*)&vkViewport, 0, sizeof(VkViewport));
    vkViewport.x = 0.0f;
    vkViewport.y = 0.0f;
    vkViewport.width = (float)gSwapchain.vkExtent2D.width;
    vkViewport.height = (float)gSwapchain.vkExtent2D.height;
    vkViewport.minDepth = 0.0f;
    vkViewport.maxDepth = 1.0f;

    memset((void*)&vkRect2D_Scissor, 0, sizeof(VkRect2D));
    vkRect2D_Scissor.offset.x = 0;
    vkRect2D_Scissor.offset.y = 0;
    vkRect2D_Scissor.extent.width = gSwapchain.vkExtent2D.width;
    vkRect2D_Scissor.extent.height = gSwapchain.vkExtent2D.height;

    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
    memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStateCreateInfo.pNext = NULL;
    vkPipelineViewportStateCreateInfo.flags = 0;
    vkPipelineViewportStateCreateInfo.viewportCount = 1; // 1 viewport
    vkPipelineViewportStateCreateInfo.pViewports = &vkViewport; // viewport
    vkPipelineViewportStateCreateInfo.scissorCount = 1; // 1 scissor
    vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_Scissor; // scissor

    //// depth stencil state
    //VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;   
    //memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    //vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    //vkPipelineDepthStencilStateCreateInfo.pNext = NULL;

    //depth stencil state
    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;
    memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    vkPipelineDepthStencilStateCreateInfo.pNext = NULL;
    vkPipelineDepthStencilStateCreateInfo.flags = 0;
    vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE; // enable depth test
    vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE; // enable depth write
    vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS; // depth compare operation
    vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE; // no depth bounds test
    vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE; // no stencil test
    //vkPipelineDepthStencilStateCreateInfo.front = {}; // no front stencil state
    //vkPipelineDepthStencilStateCreateInfo.back = {}; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS; // no back stencil state

    vkPipelineDepthStencilStateCreateInfo.front = vkPipelineDepthStencilStateCreateInfo.back; // use the same state for front and back

    vkPipelineDepthStencilStateCreateInfo.minDepthBounds = 0.0f; // min depth bounds
    vkPipelineDepthStencilStateCreateInfo.maxDepthBounds = 1.0f; // max depth bounds



    //dynamic state (viewport, scissor ,depth bias ,blend constants, stensil mask,line width, etc)
    //no dynamic state right now

    //multisample state
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStateCreateInfo.pNext = NULL;
    vkPipelineMultisampleStateCreateInfo.flags = 0;
    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // 1 sample
    //vkPipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE; // no sample shading
    //vkPipelineMultisampleStateCreateInfo.minSampleShading = 0.0f; // no min sample shading
    //vkPipelineMultisampleStateCreateInfo.pSampleMask = NULL; // no sample mask
    //vkPipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_TRUE; // no alpha to coverage
    //vkPipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // no alpha to one


    //shader stage state
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo[2];
    memset((void*)&vkPipelineShaderStageCreateInfo, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo));
    //vertex shader stage
    vkPipelineShaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[0].pNext = NULL;
    vkPipelineShaderStageCreateInfo[0].flags = 0;
    vkPipelineShaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT; // vertex shader
    vkPipelineShaderStageCreateInfo[0].module = PBR_Skinned.vkVertexShaderModule; // vertex shader module
    vkPipelineShaderStageCreateInfo[0].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[0].pSpecializationInfo = NULL; // no specialization info
    //fragment shader stage
    vkPipelineShaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo[1].flags = 0;
    vkPipelineShaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; // fragment shader
    vkPipelineShaderStageCreateInfo[1].module = PBR_Skinned.vkFragmentShaderModule; // fragment shader module
    vkPipelineShaderStageCreateInfo[1].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[1].pSpecializationInfo = NULL; // no specialization info

    //tesselation state
    //no tessellation state right now


    //pipeline cache 
    if(PBR_Skinned.vkPipelineCache == VK_NULL_HANDLE)
    {
        //create pipeline cache
        VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
        memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
        vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        vkPipelineCacheCreateInfo.pNext = NULL;
        vkPipelineCacheCreateInfo.flags = 0;
        vkPipelineCacheCreateInfo.initialDataSize = 0;
        vkPipelineCacheCreateInfo.pInitialData = NULL;
        vkResult = vkCreatePipelineCache(gVulkanContext.vkDevice, &vkPipelineCacheCreateInfo, NULL, &PBR_Skinned.vkPipelineCache);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFILE, "createGraphicsPipeline_PBR_Skinned() : vkCreatePipelineCache() failed (%d).\n", vkResult);
            return(vkResult);
		}
		vkPipelineCacheList.push_back(PBR_Skinned.vkPipelineCache);
	}

    VkPipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

    VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &colorFormat;

    renderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
    renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    //  Declare and initialize VkGraphicsPipelineCreateInfo structure.
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vkGraphicsPipelineCreateInfo.pNext = &renderingInfo; // pointer to the rendering info structure for dynamic rendering
    vkGraphicsPipelineCreateInfo.flags = 0;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pTessellationState = NULL; // no tessellation state
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDynamicState = NULL; // no dynamic state
    vkGraphicsPipelineCreateInfo.layout = PBR_Skinned.vkPipelineLayout; // pipeline layout
	vkGraphicsPipelineCreateInfo.renderPass = VK_NULL_HANDLE; // no render pass
    vkGraphicsPipelineCreateInfo.subpass = 0; // subpass index
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no base pipeline
    vkGraphicsPipelineCreateInfo.basePipelineIndex = -1; // no base pipeline index
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo); // number of shader stages
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo; // shader stages


    //  Call vkCreateGraphicsPipelines() API to create the graphics pipeline.
    vkResult = vkCreateGraphicsPipelines(gVulkanContext.vkDevice, PBR_Skinned.vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &PBR_Skinned.vkPipeline);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipeline() : vkCreateGraphicsPipelines() failed: %d .\n", vkResult);
        return(vkResult);
    }

    return(vkResult);
}

VkResult GraphicsPipelines::createGraphicsPipeline_PreviewImage(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

	vkResult = createPipelineLayout(vkPipelineLayoutCreateInfo, &PreviewImage.vkPipelineLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createGraphicsPipeline_PreviewImage() : createPipelineLayout() failed: %d .\n", vkResult);
		return(vkResult);
	}

    //  Declare and initialize VkPipelineVertexInputStateCreateInfo structure.
    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
    memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    vkPipelineVertexInputStateCreateInfo.flags = 0;
    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 0; // no vertex binding descriptions
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = NULL; // no vertex binding descriptions
    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0; // no vertex attribute descriptions
    vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = NULL; // no vertex attribute descriptions

    //  Declare and initialize VkPipelineInputAssemblyStateCreateInfo structure.
    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
    memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
    vkPipelineInputAssemblyStateCreateInfo.flags = 0;
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // triangle list
    vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; // no primitive restart


    //  Declare and initialize VkPipelineRasterizationStateCreateInfo structure.
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE; // no depth clamp
    vkPipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE; // no depth bias
    vkPipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f; // no depth bias
    vkPipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f; // no depth bias clamp
    vkPipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f; // no depth bias slope factor
    vkPipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; // no rasterizer discard
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // fill mode
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE; // disabled
    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //  anti clockwise front face
    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f; // line width

    // Color blend state
    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
    memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));
    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE; // no blending
    vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // all color components
    //vkPipelineColorBlendAttachmentState_array[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // source color blend factor
    //vkPipelineColorBlendAttachmentState_array[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // destination color blend factor
    //vkPipelineColorBlendAttachmentState_array[0].colorBlendOp = VK_BLEND_OP_ADD; // color blend operation
    //vkPipelineColorBlendAttachmentState_array[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // source alpha blend factor
    //vkPipelineColorBlendAttachmentState_array[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // destination alpha blend factor
    //vkPipelineColorBlendAttachmentState_array[0].alphaBlendOp = VK_BLEND_OP_ADD; // alpha blend operation



    //ColorBlendStateCreateInfo
    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
    memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStateCreateInfo.pNext = NULL;
    vkPipelineColorBlendStateCreateInfo.flags = 0;
    vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
    vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;
    vkPipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE; // no logic op

    //viewport sciccor state
    memset((void*)&vkViewport, 0, sizeof(VkViewport));
    vkViewport.x = 0.0f;
    vkViewport.y = 0.0f;
    vkViewport.width = (float)gSwapchain.vkExtent2D.width;
    vkViewport.height = (float)gSwapchain.vkExtent2D.height;
    vkViewport.minDepth = 0.0f;
    vkViewport.maxDepth = 1.0f;

    memset((void*)&vkRect2D_Scissor, 0, sizeof(VkRect2D));
    vkRect2D_Scissor.offset.x = 0;
    vkRect2D_Scissor.offset.y = 0;
    vkRect2D_Scissor.extent.width = gSwapchain.vkExtent2D.width;
    vkRect2D_Scissor.extent.height = gSwapchain.vkExtent2D.height;

    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
    memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStateCreateInfo.pNext = NULL;
    vkPipelineViewportStateCreateInfo.flags = 0;
    vkPipelineViewportStateCreateInfo.viewportCount = 1; // 1 viewport
    vkPipelineViewportStateCreateInfo.pViewports = &vkViewport; // viewport
    vkPipelineViewportStateCreateInfo.scissorCount = 1; // 1 scissor
    vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_Scissor; // scissor

    //depth stencil state
    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;
    memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    vkPipelineDepthStencilStateCreateInfo.pNext = NULL;
    vkPipelineDepthStencilStateCreateInfo.flags = 0;
    vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_FALSE; // 
    vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_FALSE; // 
    vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS; // depth compare operation
    vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE; // no depth bounds test
    vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE; // no stencil test
    vkPipelineDepthStencilStateCreateInfo.front = {}; // no front stencil state
    vkPipelineDepthStencilStateCreateInfo.back = {}; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.minDepthBounds = 0.0f; // min depth bounds
    vkPipelineDepthStencilStateCreateInfo.maxDepthBounds = 1.0f; // max depth bounds

    //dynamic state (viewport, scissor ,depth bias ,blend constants, stensil mask,line width, etc)
    //no dynamic state right now

    //multisample state
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStateCreateInfo.pNext = NULL;
    vkPipelineMultisampleStateCreateInfo.flags = 0;
    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // 1 sample
    //vkPipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE; // no sample shading
    //vkPipelineMultisampleStateCreateInfo.minSampleShading = 0.0f; // no min sample shading
    //vkPipelineMultisampleStateCreateInfo.pSampleMask = NULL; // no sample mask
    //vkPipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE; // no alpha to coverage
    //vkPipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // no alpha to one


    //shader stage state
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo[2];
    memset((void*)&vkPipelineShaderStageCreateInfo, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo));
    //vertex shader stage
    vkPipelineShaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[0].pNext = NULL;
    vkPipelineShaderStageCreateInfo[0].flags = 0;
    vkPipelineShaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT; // vertex shader
    vkPipelineShaderStageCreateInfo[0].module = PreviewImage.vkVertexShaderModule; // vertex shader module
    vkPipelineShaderStageCreateInfo[0].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[0].pSpecializationInfo = NULL; // no specialization info
    //fragment shader stage
    vkPipelineShaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo[1].flags = 0;
    vkPipelineShaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; // fragment shader
    vkPipelineShaderStageCreateInfo[1].module = PreviewImage.vkFragmentShaderModule; // fragment shader module
    vkPipelineShaderStageCreateInfo[1].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[1].pSpecializationInfo = NULL; // no specialization info

    //tesselation state
    //no tessellation state right now


    //pipeline cache 
    if(PreviewImage.vkPipelineCache == VK_NULL_HANDLE)
    {
        //create pipeline cache
        VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
        memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
        vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        vkPipelineCacheCreateInfo.pNext = NULL;
        vkPipelineCacheCreateInfo.flags = 0;
        vkPipelineCacheCreateInfo.initialDataSize = 0;
        vkPipelineCacheCreateInfo.pInitialData = NULL;
        vkResult = vkCreatePipelineCache(gVulkanContext.vkDevice, &vkPipelineCacheCreateInfo, NULL, &PreviewImage.vkPipelineCache);
        if (vkResult != VK_SUCCESS)
        {
			fprintf(gpFILE, "createGraphicsPipeline_PreviewImage() : vkCreatePipelineCache() failed (%d).\n", vkResult);
			return(vkResult);
		}
		vkPipelineCacheList.push_back(PreviewImage.vkPipelineCache);
	}

    VkPipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

    VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &colorFormat;

    renderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
    renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    //  Declare and initialize VkGraphicsPipelineCreateInfo structure.
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vkGraphicsPipelineCreateInfo.pNext = &renderingInfo; // pointer to the rendering info structure for dynamic rendering
    vkGraphicsPipelineCreateInfo.flags = 0;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pTessellationState = NULL; // no tessellation state
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDynamicState = NULL; // no dynamic state
    vkGraphicsPipelineCreateInfo.layout = PreviewImage.vkPipelineLayout; // pipeline layout
	vkGraphicsPipelineCreateInfo.renderPass = VK_NULL_HANDLE; // no render pass
    vkGraphicsPipelineCreateInfo.subpass = 0; // subpass index
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no base pipeline
    vkGraphicsPipelineCreateInfo.basePipelineIndex = -1; // no base pipeline index
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo); // number of shader stages
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo; // shader stages


    //  Call vkCreateGraphicsPipelines() API to create the graphics pipeline.
    vkResult = vkCreateGraphicsPipelines(gVulkanContext.vkDevice, PreviewImage.vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &PreviewImage.vkPipeline);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipeline() : vkCreateGraphicsPipelines() failed: %d .\n", vkResult);
        return(vkResult);
    }

    return(vkResult);
}

VkResult GraphicsPipelines::createGraphicsPipeline_WhiteVertex(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

	vkResult = createPipelineLayout(vkPipelineLayoutCreateInfo, &WhiteVertex.vkPipelineLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createGraphicsPipeline_WhiteVertex() : createPipelineLayout() failed: %d .\n", vkResult);
		return(vkResult);
	}

    // code
    // 
    /*
    //vertex input state
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[2];
    memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

    //position
    vkVertexInputBindingDescription_array[0].binding = 0; // VDG's location = 0
    vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3; // size of each vertex
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex data
    //color
    vkVertexInputBindingDescription_array[1].binding = 1; // VDG's location = 1
    vkVertexInputBindingDescription_array[1].stride = sizeof(float) * 3; // size of each vertex
    vkVertexInputBindingDescription_array[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex data
    */

    //vertex input state
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1];
    memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

    //position
    vkVertexInputBindingDescription_array[0].binding = 0; // VDG's location = 0
    vkVertexInputBindingDescription_array[0].stride = sizeof(VertexData_Position); // size of each vertex
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex data

    //vertex input attribute state
    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[1];
    memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));

    //position
    vkVertexInputAttributeDescription_array[0].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[0].location = 0; // location index
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[0].offset = offsetof(VertexData_Position, pos); // offset of each vertex


    //  Declare and initialize VkPipelineVertexInputStateCreateInfo structure.
    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
    memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    vkPipelineVertexInputStateCreateInfo.flags = 0;
    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;

    //  Declare and initialize VkPipelineInputAssemblyStateCreateInfo structure.
    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
    memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
    vkPipelineInputAssemblyStateCreateInfo.flags = 0;
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // triangle list
    vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; // no primitive restart


    //  Declare and initialize VkPipelineRasterizationStateCreateInfo structure.
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE; // no depth clamp
    vkPipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE; // no depth bias
    vkPipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f; // no depth bias
    vkPipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f; // no depth bias clamp
    vkPipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f; // no depth bias slope factor
    vkPipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; // no rasterizer discard
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // fill mode
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT; // back face culling
    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //  anti clockwise front face
    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f; // line width

    // Color blend state
    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
    memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));
    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE; // no blending
    vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // all color components
    //vkPipelineColorBlendAttachmentState_array[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // source color blend factor
    //vkPipelineColorBlendAttachmentState_array[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // destination color blend factor
    //vkPipelineColorBlendAttachmentState_array[0].colorBlendOp = VK_BLEND_OP_ADD; // color blend operation
    //vkPipelineColorBlendAttachmentState_array[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // source alpha blend factor
    //vkPipelineColorBlendAttachmentState_array[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // destination alpha blend factor
    //vkPipelineColorBlendAttachmentState_array[0].alphaBlendOp = VK_BLEND_OP_ADD; // alpha blend operation



    //ColorBlendStateCreateInfo
    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
    memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStateCreateInfo.pNext = NULL;
    vkPipelineColorBlendStateCreateInfo.flags = 0;
    vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
    vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;
    vkPipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE; // no logic op

    //viewport sciccor state
    memset((void*)&vkViewport, 0, sizeof(VkViewport));
    vkViewport.x = 0.0f;
    vkViewport.y = 0.0f;
    vkViewport.width = (float)gSwapchain.vkExtent2D.width;
    vkViewport.height = (float)gSwapchain.vkExtent2D.height;
    vkViewport.minDepth = 0.0f;
    vkViewport.maxDepth = 1.0f;

    memset((void*)&vkRect2D_Scissor, 0, sizeof(VkRect2D));
    vkRect2D_Scissor.offset.x = 0;
    vkRect2D_Scissor.offset.y = 0;
    vkRect2D_Scissor.extent.width = gSwapchain.vkExtent2D.width;
    vkRect2D_Scissor.extent.height = gSwapchain.vkExtent2D.height;

    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
    memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStateCreateInfo.pNext = NULL;
    vkPipelineViewportStateCreateInfo.flags = 0;
    vkPipelineViewportStateCreateInfo.viewportCount = 1; // 1 viewport
    vkPipelineViewportStateCreateInfo.pViewports = &vkViewport; // viewport
    vkPipelineViewportStateCreateInfo.scissorCount = 1; // 1 scissor
    vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_Scissor; // scissor

    //// depth stencil state
    //VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;   
    //memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    //vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    //vkPipelineDepthStencilStateCreateInfo.pNext = NULL;

    //depth stencil state
    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;
    memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    vkPipelineDepthStencilStateCreateInfo.pNext = NULL;
    vkPipelineDepthStencilStateCreateInfo.flags = 0;
    vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE; // enable depth test
    vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE; // enable depth write
    vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS; // depth compare operation
    vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE; // no depth bounds test
    vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE; // no stencil test
    //vkPipelineDepthStencilStateCreateInfo.front = {}; // no front stencil state
    //vkPipelineDepthStencilStateCreateInfo.back = {}; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS; // no back stencil state

    vkPipelineDepthStencilStateCreateInfo.front = vkPipelineDepthStencilStateCreateInfo.back; // use the same state for front and back

    vkPipelineDepthStencilStateCreateInfo.minDepthBounds = 0.0f; // min depth bounds
    vkPipelineDepthStencilStateCreateInfo.maxDepthBounds = 1.0f; // max depth bounds

    //dynamic state (viewport, scissor ,depth bias ,blend constants, stensil mask,line width, etc)
    //no dynamic state right now

    //multisample state
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStateCreateInfo.pNext = NULL;
    vkPipelineMultisampleStateCreateInfo.flags = 0;
    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // 1 sample
    //vkPipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE; // no sample shading
    //vkPipelineMultisampleStateCreateInfo.minSampleShading = 0.0f; // no min sample shading
    //vkPipelineMultisampleStateCreateInfo.pSampleMask = NULL; // no sample mask
    //vkPipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE; // no alpha to coverage
    //vkPipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // no alpha to one


    //shader stage state
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo[2];
    memset((void*)&vkPipelineShaderStageCreateInfo, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo));
    //vertex shader stage
    vkPipelineShaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[0].pNext = NULL;
    vkPipelineShaderStageCreateInfo[0].flags = 0;
    vkPipelineShaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT; // vertex shader
    vkPipelineShaderStageCreateInfo[0].module = WhiteVertex.vkVertexShaderModule; // vertex shader module
    vkPipelineShaderStageCreateInfo[0].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[0].pSpecializationInfo = NULL; // no specialization info
    //fragment shader stage
    vkPipelineShaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo[1].flags = 0;
    vkPipelineShaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; // fragment shader
    vkPipelineShaderStageCreateInfo[1].module = WhiteVertex.vkFragmentShaderModule; // fragment shader module
    vkPipelineShaderStageCreateInfo[1].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[1].pSpecializationInfo = NULL; // no specialization info

    //tesselation state
    //no tessellation state right now


    //pipeline cache 
    if(WhiteVertex.vkPipelineCache == VK_NULL_HANDLE)
    {
        //create pipeline cache
        VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
        memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
        vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        vkPipelineCacheCreateInfo.pNext = NULL;
        vkPipelineCacheCreateInfo.flags = 0;
        vkPipelineCacheCreateInfo.initialDataSize = 0;
        vkPipelineCacheCreateInfo.pInitialData = NULL;
        vkResult = vkCreatePipelineCache(gVulkanContext.vkDevice, &vkPipelineCacheCreateInfo, NULL, &WhiteVertex.vkPipelineCache);
        if (vkResult != VK_SUCCESS)
        {
			fprintf(gpFILE, "createGraphicsPipeline_WhiteVertex() : vkCreatePipelineCache() failed (%d).\n", vkResult);
			return(vkResult);
		}
		vkPipelineCacheList.push_back(WhiteVertex.vkPipelineCache);
	}

    VkPipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

    VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &colorFormat;

    renderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
    renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    //  Declare and initialize VkGraphicsPipelineCreateInfo structure.
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vkGraphicsPipelineCreateInfo.pNext = &renderingInfo; // pointer to the rendering info structure for dynamic rendering
    vkGraphicsPipelineCreateInfo.flags = 0;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pTessellationState = NULL; // no tessellation state
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo; // depth stencil state
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDynamicState = NULL; // no dynamic state
    vkGraphicsPipelineCreateInfo.layout = WhiteVertex.vkPipelineLayout; // pipeline layout
	vkGraphicsPipelineCreateInfo.renderPass = VK_NULL_HANDLE; // no render pass
    vkGraphicsPipelineCreateInfo.subpass = 0; // subpass index
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no base pipeline
    vkGraphicsPipelineCreateInfo.basePipelineIndex = -1; // no base pipeline index
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo); // number of shader stages
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo; // shader stages


    //  Call vkCreateGraphicsPipelines() API to create the graphics pipeline.
    vkResult = vkCreateGraphicsPipelines(gVulkanContext.vkDevice, WhiteVertex.vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &WhiteVertex.vkPipeline);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipeline() : vkCreateGraphicsPipelines() failed: %d .\n", vkResult);
        return(vkResult);
    }

    return(vkResult);
}

VkResult GraphicsPipelines::createGraphicsPipeline_ColoredVertex(VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

	vkResult = createPipelineLayout(vkPipelineLayoutCreateInfo, &ColoredVertex.vkPipelineLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "createGraphicsPipeline_ColoredVertex() : createPipelineLayout() failed: %d .\n", vkResult);
		return(vkResult);
	}


    // code
    // 
    /*
    //vertex input state
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[2];
    memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

    //position
    vkVertexInputBindingDescription_array[0].binding = 0; // VDG's location = 0
    vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3; // size of each vertex
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex data
    //color
    vkVertexInputBindingDescription_array[1].binding = 1; // VDG's location = 1
    vkVertexInputBindingDescription_array[1].stride = sizeof(float) * 3; // size of each vertex
    vkVertexInputBindingDescription_array[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex data
    */

    //vertex input state
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1];
    memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

    //position
    vkVertexInputBindingDescription_array[0].binding = 0; // VDG's location = 0
    vkVertexInputBindingDescription_array[0].stride = sizeof(VertexData_PositionColor); // size of each vertex
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex data

    //vertex input attribute state
    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[2];
    memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));

    //position
    vkVertexInputAttributeDescription_array[0].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[0].location = 0; // location index
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[0].offset = offsetof(VertexData_PositionColor, pos); // offset of each vertex

    //color
    vkVertexInputAttributeDescription_array[1].binding = 0; // binding index
    vkVertexInputAttributeDescription_array[1].location = 1; // location index
    vkVertexInputAttributeDescription_array[1].format = VK_FORMAT_R32G32B32_SFLOAT; // format of each vertex
    vkVertexInputAttributeDescription_array[1].offset = offsetof(VertexData_PositionColor, color); // offset of each vertex

    //  Declare and initialize VkPipelineVertexInputStateCreateInfo structure.
    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
    memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    vkPipelineVertexInputStateCreateInfo.flags = 0;
    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;

    //  Declare and initialize VkPipelineInputAssemblyStateCreateInfo structure.
    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
    memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
    vkPipelineInputAssemblyStateCreateInfo.flags = 0;
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // triangle list
    vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; // no primitive restart


    //  Declare and initialize VkPipelineRasterizationStateCreateInfo structure.
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE; // no depth clamp
    vkPipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE; // no depth bias
    vkPipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f; // no depth bias
    vkPipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f; // no depth bias clamp
    vkPipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f; // no depth bias slope factor
    vkPipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; // no rasterizer discard
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // fill mode
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE; // no culling
    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //  anti clockwise front face
    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f; // line width

    // Color blend state
    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
    memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));
    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE; // no blending
    vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // all color components
    //vkPipelineColorBlendAttachmentState_array[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // source color blend factor
    //vkPipelineColorBlendAttachmentState_array[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // destination color blend factor
    //vkPipelineColorBlendAttachmentState_array[0].colorBlendOp = VK_BLEND_OP_ADD; // color blend operation
    //vkPipelineColorBlendAttachmentState_array[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // source alpha blend factor
    //vkPipelineColorBlendAttachmentState_array[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // destination alpha blend factor
    //vkPipelineColorBlendAttachmentState_array[0].alphaBlendOp = VK_BLEND_OP_ADD; // alpha blend operation



    //ColorBlendStateCreateInfo
    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
    memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStateCreateInfo.pNext = NULL;
    vkPipelineColorBlendStateCreateInfo.flags = 0;
    vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
    vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;
    vkPipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE; // no logic op

    //viewport sciccor state
    memset((void*)&vkViewport, 0, sizeof(VkViewport));
    vkViewport.x = 0.0f;
    vkViewport.y = 0.0f;
    vkViewport.width = (float)gSwapchain.vkExtent2D.width;
    vkViewport.height = (float)gSwapchain.vkExtent2D.height;
    vkViewport.minDepth = 0.0f;
    vkViewport.maxDepth = 1.0f;

    memset((void*)&vkRect2D_Scissor, 0, sizeof(VkRect2D));
    vkRect2D_Scissor.offset.x = 0;
    vkRect2D_Scissor.offset.y = 0;
    vkRect2D_Scissor.extent.width = gSwapchain.vkExtent2D.width;
    vkRect2D_Scissor.extent.height = gSwapchain.vkExtent2D.height;

    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
    memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStateCreateInfo.pNext = NULL;
    vkPipelineViewportStateCreateInfo.flags = 0;
    vkPipelineViewportStateCreateInfo.viewportCount = 1; // 1 viewport
    vkPipelineViewportStateCreateInfo.pViewports = &vkViewport; // viewport
    vkPipelineViewportStateCreateInfo.scissorCount = 1; // 1 scissor
    vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_Scissor; // scissor

    //// depth stencil state
    //VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;   
    //memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    //vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    //vkPipelineDepthStencilStateCreateInfo.pNext = NULL;

    //depth stencil state
    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;
    memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    vkPipelineDepthStencilStateCreateInfo.pNext = NULL;
    vkPipelineDepthStencilStateCreateInfo.flags = 0;
    vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE; // enable depth test
    vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE; // enable depth write
    vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS; // depth compare operation
    vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE; // no depth bounds test
    vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE; // no stencil test
    //vkPipelineDepthStencilStateCreateInfo.front = {}; // no front stencil state
    //vkPipelineDepthStencilStateCreateInfo.back = {}; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP; // no back stencil state
    vkPipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS; // no back stencil state

    vkPipelineDepthStencilStateCreateInfo.front = vkPipelineDepthStencilStateCreateInfo.back; // use the same state for front and back

    vkPipelineDepthStencilStateCreateInfo.minDepthBounds = 0.0f; // min depth bounds
    vkPipelineDepthStencilStateCreateInfo.maxDepthBounds = 1.0f; // max depth bounds


    //dynamic state (viewport, scissor ,depth bias ,blend constants, stensil mask,line width, etc)
    //no dynamic state right now

    //multisample state
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStateCreateInfo.pNext = NULL;
    vkPipelineMultisampleStateCreateInfo.flags = 0;
    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // 1 sample
    //vkPipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE; // no sample shading
    //vkPipelineMultisampleStateCreateInfo.minSampleShading = 0.0f; // no min sample shading
    //vkPipelineMultisampleStateCreateInfo.pSampleMask = NULL; // no sample mask
    //vkPipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE; // no alpha to coverage
    //vkPipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // no alpha to one


    //shader stage state
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo[2];
    memset((void*)&vkPipelineShaderStageCreateInfo, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo));
    //vertex shader stage
    vkPipelineShaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[0].pNext = NULL;
    vkPipelineShaderStageCreateInfo[0].flags = 0;
    vkPipelineShaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT; // vertex shader
    vkPipelineShaderStageCreateInfo[0].module = ColoredVertex.vkVertexShaderModule; // vertex shader module
    vkPipelineShaderStageCreateInfo[0].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[0].pSpecializationInfo = NULL; // no specialization info
    //fragment shader stage
    vkPipelineShaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo[1].flags = 0;
    vkPipelineShaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; // fragment shader
    vkPipelineShaderStageCreateInfo[1].module = ColoredVertex.vkFragmentShaderModule; // fragment shader module
    vkPipelineShaderStageCreateInfo[1].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[1].pSpecializationInfo = NULL; // no specialization info

    //tesselation state
    //no tessellation state right now


    //pipeline cache 
    if(ColoredVertex.vkPipelineCache == VK_NULL_HANDLE)
    {
        //create pipeline cache
        VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
        memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
        vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        vkPipelineCacheCreateInfo.pNext = NULL;
        vkPipelineCacheCreateInfo.flags = 0;
        vkPipelineCacheCreateInfo.initialDataSize = 0;
        vkPipelineCacheCreateInfo.pInitialData = NULL;
        vkResult = vkCreatePipelineCache(gVulkanContext.vkDevice, &vkPipelineCacheCreateInfo, NULL, &ColoredVertex.vkPipelineCache);
        if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFILE, "createGraphicsPipeline_ColoredVertex() : vkCreatePipelineCache() failed (%d).\n", vkResult);
			return(vkResult);
		}
		vkPipelineCacheList.push_back(ColoredVertex.vkPipelineCache);
	}

    VkPipelineRenderingCreateInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

    VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &colorFormat;

    renderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
    renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    //  Declare and initialize VkGraphicsPipelineCreateInfo structure.
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vkGraphicsPipelineCreateInfo.pNext = &renderingInfo;
    vkGraphicsPipelineCreateInfo.flags = 0;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pTessellationState = NULL; // no tessellation state
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo; // no depth stencil state
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDynamicState = NULL; // no dynamic state
    vkGraphicsPipelineCreateInfo.layout = ColoredVertex.vkPipelineLayout; // pipeline layout
	vkGraphicsPipelineCreateInfo.renderPass = VK_NULL_HANDLE; // no render pass
    vkGraphicsPipelineCreateInfo.subpass = 0; // subpass index
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no base pipeline
    vkGraphicsPipelineCreateInfo.basePipelineIndex = -1; // no base pipeline index
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo); // number of shader stages
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo; // shader stages


    //  Call vkCreateGraphicsPipelines() API to create the graphics pipeline.
    vkResult = vkCreateGraphicsPipelines(gVulkanContext.vkDevice, ColoredVertex.vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &ColoredVertex.vkPipeline);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipeline() : vkCreateGraphicsPipelines() failed: %d .\n", vkResult);
        return(vkResult);
    }

    return(vkResult);
}

//--------------------------------------------------------------------------------------------

VkResult GraphicsPipelines::createPipelines(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;
    // code

    std::vector<VkDescriptorSetLayout> vkDescriptorSetLayouts;
    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
    memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));

	//--------------------------------------------------------------------------------------------

	//preview image
	vkDescriptorSetLayouts = { gpDescriptorSetLayouts->vkDescriptorSetLayout_SingleImage };
	memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
	vkPipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
	vkPipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts.data();

    vkResult = createGraphicsPipeline_PreviewImage(vkPipelineLayoutCreateInfo);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipelines() : createGraphicsPipeline_PreviewImage() failed: %d.\n", vkResult);
        return vkResult;
    }

	vkPipelineList.push_back(PreviewImage.vkPipeline);

	//--------------------------------------------------------------------------------------------

    //impostor
	vkDescriptorSetLayouts = { gpDescriptorSetLayouts->vkDescriptorSetLayout_frameData, gpDescriptorSetLayouts->vkDescriptorSetLayout_AlbedoNormal };

	memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
	vkPipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
	vkPipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts.data();

	vkResult = createGraphicsPipeline_Impostor(vkPipelineLayoutCreateInfo);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipelines() : createGraphicsPipeline_Impostor() failed: %d.\n", vkResult);
        return vkResult;
    }

	vkPipelineList.push_back(Impostor.vkPipeline);

	//--------------------------------------------------------------------------------------------

    //phong
	vkDescriptorSetLayouts = { gpDescriptorSetLayouts->vkDescriptorSetLayout_frameData };
	memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
	vkPipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
	vkPipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts.data();


    vkResult = createGraphicsPipeline_Phong(vkPipelineLayoutCreateInfo);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipelines() : createGraphicsPipeline_Phong() failed: %d.\n", vkResult);
        return vkResult;
    }

	vkPipelineList.push_back(Phong.vkPipeline);

	//--------------------------------------------------------------------------------------------

    //PBR
	vkDescriptorSetLayouts = { gpDescriptorSetLayouts->vkDescriptorSetLayout_frameData, gpDescriptorSetLayouts->vkDescriptorSetLayout_GlobalTextureArray };
	memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
	vkPipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
	vkPipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts.data();

    vkResult = createGraphicsPipeline_PBR(vkPipelineLayoutCreateInfo);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipelines() : createGraphicsPipeline_PBR() failed: %d.\n", vkResult);
        return vkResult;
    }
	vkPipelineList.push_back(PBR.vkPipeline);

	//--------------------------------------------------------------------------------------------

	//PBR_skinned
    vkDescriptorSetLayouts = { gpDescriptorSetLayouts->vkDescriptorSetLayout_frameDataBoneData,gpDescriptorSetLayouts->vkDescriptorSetLayout_BasicPBR,gpDescriptorSetLayouts->vkDescriptorSetLayout_GlobalTextureArray };
	memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
	vkPipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
	vkPipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts.data();

	vkResult = createGraphicsPipeline_PBR_Skinned(vkPipelineLayoutCreateInfo);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipelines() : createGraphicsPipeline_PBR_skinned() failed: %d.\n", vkResult);
        return vkResult;
	}
	vkPipelineList.push_back(PBR_Skinned.vkPipeline);

	//--------------------------------------------------------------------------------------------

	//White Vertex
	vkDescriptorSetLayouts = { gpDescriptorSetLayouts->vkDescriptorSetLayout_frameData };
	memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
	vkPipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
	vkPipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts.data();

	vkResult = createGraphicsPipeline_WhiteVertex(vkPipelineLayoutCreateInfo);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipelines() : createGraphicsPipeline_WhiteVertex() failed: %d.\n", vkResult);
		return vkResult;
    }
	vkPipelineList.push_back(WhiteVertex.vkPipeline);

	//--------------------------------------------------------------------------------------------

	//Colored Vertex
	vkDescriptorSetLayouts = { gpDescriptorSetLayouts->vkDescriptorSetLayout_frameData };
	memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
	vkPipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(vkDescriptorSetLayouts.size());
	vkPipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts.data();

	vkResult = createGraphicsPipeline_ColoredVertex(vkPipelineLayoutCreateInfo);
    if (vkResult != VK_SUCCESS)
    {
		fprintf(gpFILE, "createGraphicsPipelines() : createGraphicsPipeline_ColoredVertex() failed: %d.\n", vkResult);
		return vkResult;
    }
	vkPipelineList.push_back(ColoredVertex.vkPipeline);

	//--------------------------------------------------------------------------------------------

    return vkResult;
}

void GraphicsPipelines::destroyPipelines(void)
{

    //destroy vkPipelineList
    for (int32_t i = (int32_t)vkPipelineList.size() - 1; i >= 0; i--)
    {
        if (vkPipelineList[i])
        {
            vkDestroyPipeline(gVulkanContext.vkDevice, vkPipelineList[i], NULL);
            vkPipelineList[i] = VK_NULL_HANDLE;
        }
    }

    vkPipelineList.clear();


}



