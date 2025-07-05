#include "VertexInputState.h"

MyVertexInputState::MyVertexInputState(void)//size_t inputBindingCount, size_t inputAttribiuteCount
{
	//this->inputBindingCount = inputBindingCount;
	//this->inputAttributeCount = inputAttribiuteCount;

	//vkVertexInputBindingDescription_Array = new VkVertexInputBindingDescription[inputBindingCount];
	//vkVertexInputAttributeDescriptio_Array = new VkVertexInputAttributeDescription[inputAttribiuteCount];

	memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
	vkVertexInputBindingDescription_Array = nullptr;
	vkVertexInputAttributeDescription_Array = nullptr;

}

MyVertexInputState::~MyVertexInputState(void)
{
	if (vkVertexInputBindingDescription_Array != nullptr)
	{
		delete[] vkVertexInputBindingDescription_Array;
		vkVertexInputBindingDescription_Array = nullptr;
	}

	if (vkVertexInputAttributeDescription_Array != nullptr)
	{
		delete[] vkVertexInputAttributeDescription_Array;
		vkVertexInputAttributeDescription_Array = nullptr;
	}
}
