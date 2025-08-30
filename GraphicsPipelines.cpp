#include "GraphicsPipelines.h"

GraphicsPipelines::GraphicsPipelines()
{
	vkResult = VK_SUCCESS;

	vkResult = createPipelineLayouts();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "GraphicsPipelines() : createPipelineLayouts() failed (%d).\n", vkResult);
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
	destroyPipelineLayouts();
	destroyPipelines();
}

//----------------------------Pipeline Layouts------------------------------------------------

VkResult GraphicsPipelines::createPipelineLayout_previewImage(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;


    // Declare and initialize VkPipelineLayoutCreateInfo structure which will have information about the pipeline layout.
    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
    memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));

    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.pNext = NULL;
    vkPipelineLayoutCreateInfo.flags = 0;
    vkPipelineLayoutCreateInfo.setLayoutCount = 1;
    vkPipelineLayoutCreateInfo.pSetLayouts = &gpDescriptorSetLayouts->vkDescriptorSetLayout_SingleImage; // pointer to the descriptor set layout for preview image
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0; // no push constant ranges for preview image pipeline layout
    vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL; // no push constant ranges for preview image pipeline layout

    vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &PreviewImage.vkPipelineLayout);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createPipelineLayout() -> vkCreatePipelineLayout() :  failed: %d.\n", vkResult);
        return(vkResult);
    }

    return(vkResult);
}

VkResult GraphicsPipelines::createPipelineLayout_Impostor(void)
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
    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
    memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));

    VkDescriptorSetLayout vkDescriptorSetLayouts[] = { gpDescriptorSetLayouts->vkDescriptorSetLayout_frameData, gpDescriptorSetLayouts->vkDescriptorSetLayout_AlbedoNormal };

    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.pNext = NULL;
    vkPipelineLayoutCreateInfo.flags = 0;
    vkPipelineLayoutCreateInfo.setLayoutCount = 2;// two descriptor set layout for imposter pipeline layout
    vkPipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts; // pointer to the descriptor set layout
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 1; // one push constant range for imposter pipeline layout
    vkPipelineLayoutCreateInfo.pPushConstantRanges = &vkPushConstantRange; // pointer to the push constant range

    vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &Impostor.vkPipelineLayout);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createPipelinnLayout_Imposter() -> vkCreatePipelineLayout() :  failed: %d.\n", vkResult);
        return(vkResult);
    }

    return(vkResult);
}

VkResult GraphicsPipelines::createPipelineLayout_Phong(void)
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
    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
    memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));

    VkDescriptorSetLayout vkDescriptorSetLayouts[] = { gpDescriptorSetLayouts->vkDescriptorSetLayout_frameData };

    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.pNext = NULL;
    vkPipelineLayoutCreateInfo.flags = 0;
    vkPipelineLayoutCreateInfo.setLayoutCount = 1;// two descriptor set layout for imposter pipeline layout
    vkPipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts; // pointer to the descriptor set layout
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 1; // one push constant range for imposter pipeline layout
    vkPipelineLayoutCreateInfo.pPushConstantRanges = &vkPushConstantRange; // pointer to the push constant range

    vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &Phong.vkPipelineLayout);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createPipelineLayout_Phong() -> vkCreatePipelineLayout() :  failed: %d.\n", vkResult);
        return(vkResult);
    }

    return(vkResult);
}

VkResult GraphicsPipelines::createPipelineLayout_PBR(void)
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
    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
    memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));

    VkDescriptorSetLayout vkDescriptorSetLayouts[] = { gpDescriptorSetLayouts->vkDescriptorSetLayout_frameData,gpDescriptorSetLayouts->vkDescriptorSetLayout_BasicPBR };

    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.pNext = NULL;
    vkPipelineLayoutCreateInfo.flags = 0;
    vkPipelineLayoutCreateInfo.setLayoutCount = 2;// two descriptor set layout for imposter pipeline layout
    vkPipelineLayoutCreateInfo.pSetLayouts = vkDescriptorSetLayouts; // pointer to the descriptor set layout
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 1; // one push constant range for imposter pipeline layout
    vkPipelineLayoutCreateInfo.pPushConstantRanges = &vkPushConstantRange; // pointer to the push constant range

    vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &PBR.vkPipelineLayout);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createPipelinnLayout_PBR() -> vkCreatePipelineLayout() :  failed: %d.\n", vkResult);
        return(vkResult);
    }

    return(vkResult);
}

//--------------------------------------------------------------------------------------------
VkResult GraphicsPipelines::createPipelineLayouts(void)
{
	VkResult vkResult = VK_SUCCESS;
    //pipeline layout for preview image
    vkResult = createPipelineLayout_previewImage();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createPipelineLayouts() : createPipelineLayout_previewImage() failed (%d).\n", vkResult);
        return(vkResult);
    }
    //pipeline layout for impostor
    vkResult = createPipelineLayout_Impostor();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createPipelineLayouts() : createPipelineLayout_Impostor() failed (%d).\n", vkResult);
        return(vkResult);
    }
    //pipeline layout for phong
    vkResult = createPipelineLayout_Phong();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createPipelineLayouts() : createPipelineLayout_Phong() failed (%d).\n", vkResult);
        return(vkResult);
    }
    //pipeline layout for PBR
    vkResult = createPipelineLayout_PBR();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createPipelineLayouts() : createPipelineLayout_PBR() failed (%d).\n", vkResult);
        return(vkResult);
	}
	return(vkResult);
}

void GraphicsPipelines::destroyPipelineLayouts(void)
{
    ////pipeline layout
    //if (vkPipelineLayout)
    //{
    //    vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
    //    vkPipelineLayout = VK_NULL_HANDLE;
    //}

    //pipeline layout for preview image
    if (PreviewImage.vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, PreviewImage.vkPipelineLayout, NULL);
        PreviewImage.vkPipelineLayout = VK_NULL_HANDLE;
    }

    //pipeline layout for imposter
    if (Impostor.vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, Impostor.vkPipelineLayout, NULL);
        Impostor.vkPipelineLayout = VK_NULL_HANDLE;
    }

    //pipeline layout for phong
    if (Phong.vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, Phong.vkPipelineLayout, NULL);
        Phong.vkPipelineLayout = VK_NULL_HANDLE;
    }

    //pipeline layout for PBR
    if (PBR.vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, PBR.vkPipelineLayout, NULL);
        PBR.vkPipelineLayout = VK_NULL_HANDLE;
    }
}

//----------------------------Pipelines-------------------------------------------------------

VkResult GraphicsPipelines::createGraphicsPipeline_Impostor(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;


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
    vkViewport.width = (float)vkExtent2D_Swapchain.width;
    vkViewport.height = (float)vkExtent2D_Swapchain.height;
    vkViewport.minDepth = 0.0f;
    vkViewport.maxDepth = 1.0f;

    memset((void*)&vkRect2D_Scissor, 0, sizeof(VkRect2D));
    vkRect2D_Scissor.offset.x = 0;
    vkRect2D_Scissor.offset.y = 0;
    vkRect2D_Scissor.extent.width = vkExtent2D_Swapchain.width;
    vkRect2D_Scissor.extent.height = vkExtent2D_Swapchain.height;

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
    vkPipelineShaderStageCreateInfo[0].module = vkShaderModule_impostor_vs; // vertex shader module
    vkPipelineShaderStageCreateInfo[0].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[0].pSpecializationInfo = NULL; // no specialization info
    //fragment shader stage
    vkPipelineShaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo[1].flags = 0;
    vkPipelineShaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; // fragment shader
    vkPipelineShaderStageCreateInfo[1].module = vkShaderModule_impostor_fs; // fragment shader module
    vkPipelineShaderStageCreateInfo[1].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[1].pSpecializationInfo = NULL; // no specialization info

    //tesselation state
    //no tessellation state right now


    //pipeline cache 
    VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
    memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
    vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkPipelineCacheCreateInfo.pNext = NULL;
    vkPipelineCacheCreateInfo.flags = 0;
    vkPipelineCacheCreateInfo.initialDataSize = 0;
    vkPipelineCacheCreateInfo.pInitialData = NULL;

    VkPipelineCache vkPipelineCache;
    vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipeline() : vkCreatePipelineCache() failed: %d .\n", vkResult);
        return(vkResult);
    }

    //  Declare and initialize VkGraphicsPipelineCreateInfo structure.
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkGraphicsPipelineCreateInfo.pNext = NULL;
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
    vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass; // render pass
    vkGraphicsPipelineCreateInfo.subpass = 0; // subpass index
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no base pipeline
    vkGraphicsPipelineCreateInfo.basePipelineIndex = -1; // no base pipeline index
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo); // number of shader stages
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo; // shader stages


    //  Call vkCreateGraphicsPipelines() API to create the graphics pipeline.
    vkResult = vkCreateGraphicsPipelines(vkDevice, vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &Impostor.vkPipeline);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipeline() : vkCreateGraphicsPipelines() failed: %d .\n", vkResult);

        //destroy pipeline cache
        vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
        vkPipelineCache = VK_NULL_HANDLE;

        return(vkResult);
    }

    //destroy pipeline cache
    vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
    vkPipelineCache = VK_NULL_HANDLE;

    return(vkResult);
}

VkResult GraphicsPipelines::createGraphicsPipeline_Phong(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;


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
    vkViewport.width = (float)vkExtent2D_Swapchain.width;
    vkViewport.height = (float)vkExtent2D_Swapchain.height;
    vkViewport.minDepth = 0.0f;
    vkViewport.maxDepth = 1.0f;

    memset((void*)&vkRect2D_Scissor, 0, sizeof(VkRect2D));
    vkRect2D_Scissor.offset.x = 0;
    vkRect2D_Scissor.offset.y = 0;
    vkRect2D_Scissor.extent.width = vkExtent2D_Swapchain.width;
    vkRect2D_Scissor.extent.height = vkExtent2D_Swapchain.height;

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
    vkPipelineShaderStageCreateInfo[0].module = vkShaderModule_phong_vs; // vertex shader module
    vkPipelineShaderStageCreateInfo[0].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[0].pSpecializationInfo = NULL; // no specialization info
    //fragment shader stage
    vkPipelineShaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo[1].flags = 0;
    vkPipelineShaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; // fragment shader
    vkPipelineShaderStageCreateInfo[1].module = vkShaderModule_phong_fs; // fragment shader module
    vkPipelineShaderStageCreateInfo[1].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[1].pSpecializationInfo = NULL; // no specialization info

    //tesselation state
    //no tessellation state right now


    //pipeline cache 
    VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
    memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
    vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkPipelineCacheCreateInfo.pNext = NULL;
    vkPipelineCacheCreateInfo.flags = 0;
    vkPipelineCacheCreateInfo.initialDataSize = 0;
    vkPipelineCacheCreateInfo.pInitialData = NULL;

    VkPipelineCache vkPipelineCache;
    vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipeline() : vkCreatePipelineCache() failed: %d .\n", vkResult);
        return(vkResult);
    }

    //  Declare and initialize VkGraphicsPipelineCreateInfo structure.
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkGraphicsPipelineCreateInfo.pNext = NULL;
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
    vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass; // render pass
    vkGraphicsPipelineCreateInfo.subpass = 0; // subpass index
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no base pipeline
    vkGraphicsPipelineCreateInfo.basePipelineIndex = -1; // no base pipeline index
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo); // number of shader stages
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo; // shader stages


    //  Call vkCreateGraphicsPipelines() API to create the graphics pipeline.
    vkResult = vkCreateGraphicsPipelines(vkDevice, vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &Phong.vkPipeline);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipeline() : vkCreateGraphicsPipelines() failed: %d .\n", vkResult);

        //destroy pipeline cache
        vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
        vkPipelineCache = VK_NULL_HANDLE;

        return(vkResult);
    }

    //destroy pipeline cache
    vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
    vkPipelineCache = VK_NULL_HANDLE;

    return(vkResult);
}

VkResult GraphicsPipelines::createGraphicsPipeline_PBR(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;


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
    vkViewport.width = (float)vkExtent2D_Swapchain.width;
    vkViewport.height = (float)vkExtent2D_Swapchain.height;
    vkViewport.minDepth = 0.0f;
    vkViewport.maxDepth = 1.0f;

    memset((void*)&vkRect2D_Scissor, 0, sizeof(VkRect2D));
    vkRect2D_Scissor.offset.x = 0;
    vkRect2D_Scissor.offset.y = 0;
    vkRect2D_Scissor.extent.width = vkExtent2D_Swapchain.width;
    vkRect2D_Scissor.extent.height = vkExtent2D_Swapchain.height;

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
    vkPipelineShaderStageCreateInfo[0].module = vkShaderModule_PBR_vs; // vertex shader module
    vkPipelineShaderStageCreateInfo[0].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[0].pSpecializationInfo = NULL; // no specialization info
    //fragment shader stage
    vkPipelineShaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo[1].flags = 0;
    vkPipelineShaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; // fragment shader
    vkPipelineShaderStageCreateInfo[1].module = vkShaderModule_PBR_fs; // fragment shader module
    vkPipelineShaderStageCreateInfo[1].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[1].pSpecializationInfo = NULL; // no specialization info

    //tesselation state
    //no tessellation state right now


    //pipeline cache 
    VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
    memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
    vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkPipelineCacheCreateInfo.pNext = NULL;
    vkPipelineCacheCreateInfo.flags = 0;
    vkPipelineCacheCreateInfo.initialDataSize = 0;
    vkPipelineCacheCreateInfo.pInitialData = NULL;

    VkPipelineCache vkPipelineCache;
    vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipeline() : vkCreatePipelineCache() failed: %d .\n", vkResult);
        return(vkResult);
    }

    //  Declare and initialize VkGraphicsPipelineCreateInfo structure.
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkGraphicsPipelineCreateInfo.pNext = NULL;
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
    vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass; // render pass
    vkGraphicsPipelineCreateInfo.subpass = 0; // subpass index
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no base pipeline
    vkGraphicsPipelineCreateInfo.basePipelineIndex = -1; // no base pipeline index
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo); // number of shader stages
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo; // shader stages


    //  Call vkCreateGraphicsPipelines() API to create the graphics pipeline.
    vkResult = vkCreateGraphicsPipelines(vkDevice, vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &PBR.vkPipeline);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipeline() : vkCreateGraphicsPipelines() failed: %d .\n", vkResult);

        //destroy pipeline cache
        vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
        vkPipelineCache = VK_NULL_HANDLE;

        return(vkResult);
    }

    //destroy pipeline cache
    vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
    vkPipelineCache = VK_NULL_HANDLE;

    return(vkResult);
}

VkResult GraphicsPipelines::createGraphicsPipeline_PreviewImage(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;



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
    vkViewport.width = (float)vkExtent2D_Swapchain.width;
    vkViewport.height = (float)vkExtent2D_Swapchain.height;
    vkViewport.minDepth = 0.0f;
    vkViewport.maxDepth = 1.0f;

    memset((void*)&vkRect2D_Scissor, 0, sizeof(VkRect2D));
    vkRect2D_Scissor.offset.x = 0;
    vkRect2D_Scissor.offset.y = 0;
    vkRect2D_Scissor.extent.width = vkExtent2D_Swapchain.width;
    vkRect2D_Scissor.extent.height = vkExtent2D_Swapchain.height;

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
    vkPipelineShaderStageCreateInfo[0].module = vkShaderModule_previewImage_vs; // vertex shader module
    vkPipelineShaderStageCreateInfo[0].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[0].pSpecializationInfo = NULL; // no specialization info
    //fragment shader stage
    vkPipelineShaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo[1].flags = 0;
    vkPipelineShaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; // fragment shader
    vkPipelineShaderStageCreateInfo[1].module = vkShaderModule_previewImage_fs; // fragment shader module
    vkPipelineShaderStageCreateInfo[1].pName = "main"; // entry point name
    vkPipelineShaderStageCreateInfo[1].pSpecializationInfo = NULL; // no specialization info

    //tesselation state
    //no tessellation state right now


    //pipeline cache 
    VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
    memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
    vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkPipelineCacheCreateInfo.pNext = NULL;
    vkPipelineCacheCreateInfo.flags = 0;
    vkPipelineCacheCreateInfo.initialDataSize = 0;
    vkPipelineCacheCreateInfo.pInitialData = NULL;

    VkPipelineCache vkPipelineCache;
    vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipeline() : vkCreatePipelineCache() failed: %d .\n", vkResult);
        return(vkResult);
    }

    //  Declare and initialize VkGraphicsPipelineCreateInfo structure.
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkGraphicsPipelineCreateInfo.pNext = NULL;
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
    vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass; // render pass
    vkGraphicsPipelineCreateInfo.subpass = 0; // subpass index
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // no base pipeline
    vkGraphicsPipelineCreateInfo.basePipelineIndex = -1; // no base pipeline index
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo); // number of shader stages
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo; // shader stages


    //  Call vkCreateGraphicsPipelines() API to create the graphics pipeline.
    vkResult = vkCreateGraphicsPipelines(vkDevice, vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &PreviewImage.vkPipeline);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipeline() : vkCreateGraphicsPipelines() failed: %d .\n", vkResult);

        //destroy pipeline cache
        vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
        vkPipelineCache = VK_NULL_HANDLE;

        return(vkResult);
    }

    //destroy pipeline cache
    vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
    vkPipelineCache = VK_NULL_HANDLE;

    return(vkResult);
}

VkResult GraphicsPipelines::createPipelines(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;
    // code

    //vkResult = createGraphicsPipeline_PerVertexColor();
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "createGraphicsPipelines() : createGraphicsPipeline_PerVertexColor() failed: %d.\n", vkResult);
    //    return vkResult;
    //}

    //vkResult = createGraphicsPipeline_WhiteVertex();
    //if (vkResult != VK_SUCCESS)
    //{
    //    fprintf(gpFILE, "createGraphicsPipelines() : createGraphicsPipeline_WhiteVertex() failed: %d.\n", vkResult);
    //    return vkResult;
    //}

    vkResult = createGraphicsPipeline_PreviewImage();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipelines() : createGraphicsPipeline_PreviewImage() failed: %d.\n", vkResult);
        return vkResult;
    }

    vkResult = createGraphicsPipeline_Impostor();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipelines() : createGraphicsPipeline_Impostor() failed: %d.\n", vkResult);
        return vkResult;
    }

    //phong
    vkResult = createGraphicsPipeline_Phong();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipelines() : createGraphicsPipeline_Phong() failed: %d.\n", vkResult);
        return vkResult;
    }

    //PBR
    vkResult = createGraphicsPipeline_PBR();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createGraphicsPipelines() : createGraphicsPipeline_PBR() failed: %d.\n", vkResult);
        return vkResult;
    }


    return vkResult;
}

void GraphicsPipelines::destroyPipelines(void)
{

    if (PreviewImage.vkPipeline)
    {
        vkDestroyPipeline(vkDevice, PreviewImage.vkPipeline, NULL);
        PreviewImage.vkPipeline = VK_NULL_HANDLE;
    }
    if (Impostor.vkPipeline)
    {
        vkDestroyPipeline(vkDevice, Impostor.vkPipeline, NULL);
        Impostor.vkPipeline = VK_NULL_HANDLE;
    }
    if (Phong.vkPipeline)
    {
        vkDestroyPipeline(vkDevice, Phong.vkPipeline, NULL);
        Phong.vkPipeline = VK_NULL_HANDLE;
    }
    if (PBR.vkPipeline)
    {
        vkDestroyPipeline(vkDevice, PBR.vkPipeline, NULL);
        PBR.vkPipeline = VK_NULL_HANDLE;
    }
}



