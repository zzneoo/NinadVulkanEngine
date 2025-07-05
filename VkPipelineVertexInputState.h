#pragma once
#include "framework.h"
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#include"VK.h"
#include "VertexInputState.h"

struct MyVkPipelineVertexInputStateList
{
	MyVkPipelineVertexInputStateList(void);

	//VkPipelineVertexInputStateCreateInfo Array_Position;
	MyVertexInputState Index_PositionColor;

private:

	void Initialize(void);

	//Index_PositionColor
	void Initialize_Index_PositionColor(void);

};

