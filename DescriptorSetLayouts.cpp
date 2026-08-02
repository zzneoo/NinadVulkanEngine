#include "DescriptorSetLayouts.h"

//extern VkDevice vkDevice;
extern FILE* gpFILE;

DescriptorSetLayouts::DescriptorSetLayouts()
{
    vkResult = VK_SUCCESS;

	vkResult = createDescriptorSetLayouts();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "DescriptorSetLayouts() : createDescriptorSetLayouts() failed (%d).\n", vkResult);
        return;
	}

}
DescriptorSetLayouts::~DescriptorSetLayouts()
{
    destroyDescriptorSetLayouts();
}

VkResult DescriptorSetLayouts::createDescriptorSetLayout(const VkDescriptorSetLayoutBinding * DescriptorSetLayoutBinding, uint32_t  bindingCount, VkDescriptorSetLayout *DescriptorSetLayout)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;


	// code
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
	memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
	vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	vkDescriptorSetLayoutCreateInfo.pNext = NULL;
	vkDescriptorSetLayoutCreateInfo.flags = 0;
	vkDescriptorSetLayoutCreateInfo.bindingCount = bindingCount;
	vkDescriptorSetLayoutCreateInfo.pBindings = DescriptorSetLayoutBinding; // pointer to the descriptor set layout binding array

	vkResult = vkCreateDescriptorSetLayout(gVulkanContext.vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, DescriptorSetLayout);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createDiscriptorSetLayout() -> vkCreateDescriptorSetLayout() :  failed: %d.\n", vkResult);
		return(vkResult);

    }

    return(vkResult);
}

VkResult DescriptorSetLayouts::createDescriptorSetLayout_FrameData(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // Declare and initialize VkDescriptorSetLayoutBinding structure which will have information about the descriptor set layout binding.
    VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding_Array[1];
    memset((void*)&vkDescriptorSetLayoutBinding_Array, 0, sizeof(vkDescriptorSetLayoutBinding_Array));

    vkDescriptorSetLayoutBinding_Array[0].binding = 0; // binding index
    vkDescriptorSetLayoutBinding_Array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // descriptor type
    vkDescriptorSetLayoutBinding_Array[0].descriptorCount = 1; // number of descriptors
    vkDescriptorSetLayoutBinding_Array[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // stage flags for the descriptor set layout binding
    vkDescriptorSetLayoutBinding_Array[0].pImmutableSamplers = NULL; // no immutable samplers

    // code
    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
    memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));

    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.pNext = NULL;
    vkDescriptorSetLayoutCreateInfo.flags = 0;
    vkDescriptorSetLayoutCreateInfo.bindingCount = 1;
    vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBinding_Array; // pointer to the descriptor set layout binding array


    vkResult = vkCreateDescriptorSetLayout(gVulkanContext.vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout_frameData);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createDiscriptorSetLayout() -> vkCreateDescriptorSetLayout() :  failed: %d.\n", vkResult);
        return(vkResult);
    }

    return(vkResult);
}

VkResult DescriptorSetLayouts::createDescriptorSetLayout_FrameDataBoneData(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // Declare and initialize VkDescriptorSetLayoutBinding structure which will have information about the descriptor set layout binding.
    VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding_Array[2];
    memset((void*)&vkDescriptorSetLayoutBinding_Array, 0, sizeof(vkDescriptorSetLayoutBinding_Array));

    vkDescriptorSetLayoutBinding_Array[0].binding = 0; // binding index
    vkDescriptorSetLayoutBinding_Array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // descriptor type
    vkDescriptorSetLayoutBinding_Array[0].descriptorCount = 1; // number of descriptors
    vkDescriptorSetLayoutBinding_Array[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; // stage flags for the descriptor set layout binding
    vkDescriptorSetLayoutBinding_Array[0].pImmutableSamplers = NULL; // no immutable samplers

	vkDescriptorSetLayoutBinding_Array[1].binding = 1; // binding index
	vkDescriptorSetLayoutBinding_Array[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // descriptor type
	vkDescriptorSetLayoutBinding_Array[1].descriptorCount = 1; // number of descriptors
	vkDescriptorSetLayoutBinding_Array[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // stage flags for the descriptor set layout binding
	vkDescriptorSetLayoutBinding_Array[1].pImmutableSamplers = NULL; // no immutable samplers

    // code
    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
    memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));

    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.pNext = NULL;
    vkDescriptorSetLayoutCreateInfo.flags = 0;
    vkDescriptorSetLayoutCreateInfo.bindingCount = 2;
    vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBinding_Array; // pointer to the descriptor set layout binding array


    vkResult = vkCreateDescriptorSetLayout(gVulkanContext.vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout_frameDataBoneData);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createDiscriptorSetLayout() -> vkCreateDescriptorSetLayout() :  failed: %d.\n", vkResult);
        return(vkResult);
    }

    return(vkResult);
}

VkResult DescriptorSetLayouts::createDescriptorSetLayout_SingleImage(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding_Array[1];
    memset((void*)&vkDescriptorSetLayoutBinding_Array, 0, sizeof(vkDescriptorSetLayoutBinding_Array));

    vkDescriptorSetLayoutBinding_Array[0].binding = 0; // binding index
    vkDescriptorSetLayoutBinding_Array[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // descriptor type
    vkDescriptorSetLayoutBinding_Array[0].descriptorCount = 1; // number of descriptors
    vkDescriptorSetLayoutBinding_Array[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // stage flags for the descriptor set layout binding
    vkDescriptorSetLayoutBinding_Array[0].pImmutableSamplers = NULL; // no immutable samplers
    // code
    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
    memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.pNext = NULL;
    vkDescriptorSetLayoutCreateInfo.flags = 0;
    vkDescriptorSetLayoutCreateInfo.bindingCount = 1;
    vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBinding_Array; // pointer to the descriptor set layout binding array
    vkResult = vkCreateDescriptorSetLayout(gVulkanContext.vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout_SingleImage);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createDiscriptorSetLayout_Impostor() -> vkCreateDescriptorSetLayout() :  failed: %d.\n", vkResult);
        return(vkResult);
    }


    return vkResult;
}

VkResult DescriptorSetLayouts::createDescriptorSetLayout_AlbedoNormal(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;
    VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding_Array[2];
    memset((void*)&vkDescriptorSetLayoutBinding_Array, 0, sizeof(vkDescriptorSetLayoutBinding_Array));
    vkDescriptorSetLayoutBinding_Array[0].binding = 0; // binding index
    vkDescriptorSetLayoutBinding_Array[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // descriptor type
    vkDescriptorSetLayoutBinding_Array[0].descriptorCount = 1; // number of descriptors
    vkDescriptorSetLayoutBinding_Array[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // stage flags for the descriptor set layout binding
    vkDescriptorSetLayoutBinding_Array[0].pImmutableSamplers = NULL; // no immutable samplers

    vkDescriptorSetLayoutBinding_Array[1].binding = 1; // binding index
    vkDescriptorSetLayoutBinding_Array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // descriptor type
    vkDescriptorSetLayoutBinding_Array[1].descriptorCount = 1; // number of descriptors
    vkDescriptorSetLayoutBinding_Array[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // stage flags for the descriptor set layout binding
    vkDescriptorSetLayoutBinding_Array[1].pImmutableSamplers = NULL; // no immutable samplers
    // code
    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
    memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.pNext = NULL;
    vkDescriptorSetLayoutCreateInfo.flags = 0;
    vkDescriptorSetLayoutCreateInfo.bindingCount = 2;
    vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBinding_Array; // pointer to the descriptor set layout binding array
    vkResult = vkCreateDescriptorSetLayout(gVulkanContext.vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout_AlbedoNormal);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createDiscriptorSetLayout_AlbedoNormal() -> vkCreateDescriptorSetLayout() :  failed: %d.\n", vkResult);
        return(vkResult);
    }
    return vkResult;
}

VkResult DescriptorSetLayouts::createDescriptorSetLayout_BasicPBR(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding_Array[3];
    memset((void*)&vkDescriptorSetLayoutBinding_Array, 0, sizeof(vkDescriptorSetLayoutBinding_Array));
    vkDescriptorSetLayoutBinding_Array[0].binding = 0; // binding index
    vkDescriptorSetLayoutBinding_Array[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // descriptor type
    vkDescriptorSetLayoutBinding_Array[0].descriptorCount = 1; // number of descriptors
    vkDescriptorSetLayoutBinding_Array[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // stage flags for the descriptor set layout binding
    vkDescriptorSetLayoutBinding_Array[0].pImmutableSamplers = NULL; // no immutable samplers

    vkDescriptorSetLayoutBinding_Array[1].binding = 1; // binding index
    vkDescriptorSetLayoutBinding_Array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // descriptor type
    vkDescriptorSetLayoutBinding_Array[1].descriptorCount = 1; // number of descriptors
    vkDescriptorSetLayoutBinding_Array[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // stage flags for the descriptor set layout binding
    vkDescriptorSetLayoutBinding_Array[1].pImmutableSamplers = NULL; // no immutable samplers

    vkDescriptorSetLayoutBinding_Array[2].binding = 2; // binding index
    vkDescriptorSetLayoutBinding_Array[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // descriptor type
    vkDescriptorSetLayoutBinding_Array[2].descriptorCount = 1; // number of descriptors
    vkDescriptorSetLayoutBinding_Array[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // stage flags for the descriptor set layout binding
    vkDescriptorSetLayoutBinding_Array[2].pImmutableSamplers = NULL; // no immutable samplers
    // code
    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
    memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.pNext = NULL;
    vkDescriptorSetLayoutCreateInfo.flags = 0;
    vkDescriptorSetLayoutCreateInfo.bindingCount = 3;
    vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBinding_Array; // pointer to the descriptor set layout binding array

    vkResult = vkCreateDescriptorSetLayout(gVulkanContext.vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout_BasicPBR);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createDescriptorSetLayout_BasicPBR() -> vkCreateDescriptorSetLayout() :  failed: %d.\n", vkResult);
        return(vkResult);
    }
    return vkResult;

}

VkResult DescriptorSetLayouts::createDescriptorSetLayout_GlobalTextureArray(void)
{
    const uint32_t MAX_TEXTURES = 1024;
    VkResult vkResult = VK_SUCCESS;

    VkDescriptorSetLayoutBinding textureBinding{};
    textureBinding.binding = 0;
    textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBinding.descriptorCount = MAX_TEXTURES;
    textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{};
    extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    extendedInfo.bindingCount = 1;
    extendedInfo.pBindingFlags = &bindingFlags;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = &extendedInfo;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &textureBinding;


    vkResult = vkCreateDescriptorSetLayout(gVulkanContext.vkDevice, &layoutInfo, nullptr, &vkDescriptorSetLayout_GlobalTextureArray);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createTextureIndexing() : vkCreateDescriptorSetLayout() failed (%d).\n", vkResult);
        return vkResult;
    }

	return vkResult;
}

//--------------------------------------------------------------------------------------------

VkResult DescriptorSetLayouts::createDescriptorSetLayouts(void)
{

    VkResult vkResult = VK_SUCCESS;

    vkResult = createDescriptorSetLayout_FrameData();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createDescriptorSetLayout_FrameData() failed (%d).\n", vkResult);
        return(vkResult);
    }

	vkResult = createDescriptorSetLayout_FrameDataBoneData();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFILE, "initialize() : createDescriptorSetLayout_FrameDataBoneData() failed (%d).\n", vkResult);
		return(vkResult);
	}

    //descriptor set layout for impostor
    vkResult = createDescriptorSetLayout_SingleImage();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createDescriptorSetLayout_CamImage() failed (%d).\n", vkResult);
        return(vkResult);
    }

    //descriptor set layout for albedo normal
    vkResult = createDescriptorSetLayout_AlbedoNormal();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createDescriptorSetLayout_AlbedoNormal() failed (%d).\n", vkResult);
        return(vkResult);
    }

    //descriptor set layout for PBR basic
    vkResult = createDescriptorSetLayout_BasicPBR();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createDescriptorSetLayout_BasicPBR() failed (%d).\n", vkResult);
        return(vkResult);
    }

	//descriptor set layout for global texture array
	vkResult = createDescriptorSetLayout_GlobalTextureArray();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "initialize() : createDescriptorSetLayout_GlobalTextureArray() failed (%d).\n", vkResult);
        return vkResult;
    }

    return vkResult;
}

void DescriptorSetLayouts::destroyDescriptorSetLayouts(void)
{
    //descriptorSetLayout for frameData
    if (vkDescriptorSetLayout_frameData)
    {
        vkDestroyDescriptorSetLayout(gVulkanContext.vkDevice, vkDescriptorSetLayout_frameData, NULL);
        vkDescriptorSetLayout_frameData = VK_NULL_HANDLE;
    }

    if (vkDescriptorSetLayout_frameDataBoneData)
    {
        vkDestroyDescriptorSetLayout(gVulkanContext.vkDevice, vkDescriptorSetLayout_frameDataBoneData, NULL);
        vkDescriptorSetLayout_frameDataBoneData = VK_NULL_HANDLE;
	}

    //descriptor set layout for impostor
    if (vkDescriptorSetLayout_SingleImage)
    {
        vkDestroyDescriptorSetLayout(gVulkanContext.vkDevice, vkDescriptorSetLayout_SingleImage, NULL);
        vkDescriptorSetLayout_SingleImage = VK_NULL_HANDLE;
    }

    //descriptor set layout for Albedo Normal
    if (vkDescriptorSetLayout_AlbedoNormal)
    {
        vkDestroyDescriptorSetLayout(gVulkanContext.vkDevice, vkDescriptorSetLayout_AlbedoNormal, NULL);
        vkDescriptorSetLayout_AlbedoNormal = VK_NULL_HANDLE;
    }

    //descriptor set layout for PBR
    if (vkDescriptorSetLayout_BasicPBR)
    {
        vkDestroyDescriptorSetLayout(gVulkanContext.vkDevice, vkDescriptorSetLayout_BasicPBR, NULL);
        vkDescriptorSetLayout_BasicPBR = VK_NULL_HANDLE;
    }

	//descriptor set layout for global texture array
    if (vkDescriptorSetLayout_GlobalTextureArray)
    {
        vkDestroyDescriptorSetLayout(gVulkanContext.vkDevice, vkDescriptorSetLayout_GlobalTextureArray, NULL);
        vkDescriptorSetLayout_GlobalTextureArray = VK_NULL_HANDLE;
	}
}
