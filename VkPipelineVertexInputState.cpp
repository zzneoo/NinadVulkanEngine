#pragma once
#include "VkPipelineVertexInputState.h"

MyVkPipelineVertexInputStateList::MyVkPipelineVertexInputStateList(void)
{
    Initialize();
}

void MyVkPipelineVertexInputStateList::Initialize(void)
{
    // Initialize the vertex input state structures.
    Initialize_Index_PositionColor();
}

void MyVkPipelineVertexInputStateList::Initialize_Index_PositionColor(void)
{

    //----------------------------Index_PositionColor--------------------
    // 
	// Vertex input binding state

    const size_t inputBindingCount = 1;
	const size_t inputAttributeCount = 2;

    Index_PositionColor.vkVertexInputBindingDescription_Array =(VkVertexInputBindingDescription*)malloc(sizeof(VkVertexInputBindingDescription) * inputBindingCount);
    memset((void*)Index_PositionColor.vkVertexInputBindingDescription_Array, 0, sizeof(VkVertexInputBindingDescription) * inputBindingCount);
   //buffer
    Index_PositionColor.vkVertexInputBindingDescription_Array[0].binding = 0; // VDG's location = 0
    Index_PositionColor.vkVertexInputBindingDescription_Array[0].stride = sizeof(VertexData_PositionColor); // size of each vertex
    Index_PositionColor.vkVertexInputBindingDescription_Array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // per vertex data

    //vertex input attribute state
    Index_PositionColor.vkVertexInputAttributeDescription_Array = new VkVertexInputAttributeDescription[inputAttributeCount];
	memset((void*)Index_PositionColor.vkVertexInputAttributeDescription_Array, 0, sizeof(VkVertexInputAttributeDescription) * inputAttributeCount);

    //position
    Index_PositionColor.vkVertexInputAttributeDescription_Array[0].binding = 0; // binding index
    Index_PositionColor.vkVertexInputAttributeDescription_Array[0].location = 0; // location index
    Index_PositionColor.vkVertexInputAttributeDescription_Array[0].format = VK_FORMAT_R32G32B32_SFLOAT; // format of each vertex
    Index_PositionColor.vkVertexInputAttributeDescription_Array[0].offset = offsetof(VertexData_PositionColor, pos); // offset of each vertex

    //color
    Index_PositionColor.vkVertexInputAttributeDescription_Array[1].binding = 0; // binding index
    Index_PositionColor.vkVertexInputAttributeDescription_Array[1].location = 1; // location index
    Index_PositionColor.vkVertexInputAttributeDescription_Array[1].format = VK_FORMAT_R32G32B32_SFLOAT; // format of each vertex
    Index_PositionColor.vkVertexInputAttributeDescription_Array[1].offset = offsetof(VertexData_PositionColor, color); // offset of each vertex

    //  Declare and initialize VkPipelineVertexInputStateCreateInfo structure.
    //VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;

   
    memset((void*)&Index_PositionColor.vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));

    Index_PositionColor.vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    Index_PositionColor.vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    Index_PositionColor.vkPipelineVertexInputStateCreateInfo.flags = 0;
	Index_PositionColor.vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = inputBindingCount;
	Index_PositionColor.vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = Index_PositionColor.vkVertexInputBindingDescription_Array;
	Index_PositionColor.vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = inputAttributeCount;
	Index_PositionColor.vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = Index_PositionColor.vkVertexInputAttributeDescription_Array;

    //------------------------------------------------------------------------------------

}
