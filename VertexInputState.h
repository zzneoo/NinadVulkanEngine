#pragma once
#include "framework.h"
#include <vulkan/vulkan_core.h>


struct MyVertexInputState
{
	MyVertexInputState(void);
	~MyVertexInputState(void);

	VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
	VkVertexInputBindingDescription* vkVertexInputBindingDescription_Array;
	VkVertexInputAttributeDescription* vkVertexInputAttributeDescription_Array;
};