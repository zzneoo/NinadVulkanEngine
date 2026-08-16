#include "Material_BasicPBR.h"
using namespace tinyddsloader;

Material_BasicPBR::Material_BasicPBR(VkDescriptorSetLayout layout,const char* path )
{
    vkResult = VK_SUCCESS;
    VkResult res = VK_SUCCESS;

	vkDescriptorSetLayout = layout;

    char albedoPath[512]; // make sure it's big enough
    std::snprintf(albedoPath, sizeof(albedoPath), "%s%s", path, "Albedo.dds");
	res = loadTextureData_dds_c_bc7(&Albedo, albedoPath, vkSampler_LinearRepeatAniso, VK_FORMAT_BC7_SRGB_BLOCK);
    if (res != VK_SUCCESS && vkResult == VK_SUCCESS)
    {
        fprintf(gpFILE, "Failed to load albedo texture\n");
        vkResult = res;
	}
                                            

    char normalPath[512]; // make sure it's big enough
    std::snprintf(normalPath, sizeof(normalPath), "%s%s", path, "Normal.dds");
    res = loadTextureData_dds_c_bc5_normal(&Normal, normalPath, vkSampler_LinearRepeatAniso, VK_FORMAT_BC5_UNORM_BLOCK);
    if (res != VK_SUCCESS && vkResult == VK_SUCCESS)
    {
        fprintf(gpFILE, "Failed to load normal texture\n");
        vkResult = res;
	}


    char PathORX[512]; // make sure it's big enough
    std::snprintf(PathORX, sizeof(PathORX), "%s%s", path, "ORX.dds");
    res = loadTextureData_dds_c_bc7(&ORX, PathORX, vkSampler_LinearRepeatAniso, VK_FORMAT_BC7_UNORM_BLOCK);
    if (res != VK_SUCCESS && vkResult == VK_SUCCESS)
    {
        fprintf(gpFILE, "Failed to load ORM texture\n");
        vkResult = res;
	}


	// create descriptor set
    res = createDescriptorSet();
    if (res != VK_SUCCESS && vkResult == VK_SUCCESS)
    {
        fprintf(gpFILE, "Failed to create descriptor set for material\n");
        vkResult = res;
	}
	
}
Material_BasicPBR::~Material_BasicPBR()
{
    //Albedo
    if (Albedo.vkImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(gVulkanContext.vkDevice, Albedo.vkImageView, nullptr);
        Albedo.vkImageView = VK_NULL_HANDLE;
    }
    if (Albedo.vkImage != VK_NULL_HANDLE)
    {
		vkDestroyImage(gVulkanContext.vkDevice, Albedo.vkImage, nullptr);
        Albedo.vkImage = VK_NULL_HANDLE;
    }
    if (Albedo.vkDeviceMemory != VK_NULL_HANDLE)
	{
        vkFreeMemory(gVulkanContext.vkDevice, Albedo.vkDeviceMemory, nullptr);
        Albedo.vkDeviceMemory = VK_NULL_HANDLE;
    }

    //Normal
    if (Normal.vkImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(gVulkanContext.vkDevice, Normal.vkImageView, nullptr);
        Normal.vkImageView = VK_NULL_HANDLE;
    }
    if (Normal.vkImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(gVulkanContext.vkDevice, Normal.vkImage, nullptr);
        Normal.vkImage = VK_NULL_HANDLE;
    }
    if (Normal.vkDeviceMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gVulkanContext.vkDevice, Normal.vkDeviceMemory, nullptr);
        Normal.vkDeviceMemory = VK_NULL_HANDLE;
    }

	//ORX
    if (ORX.vkImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(gVulkanContext.vkDevice, ORX.vkImageView, nullptr);
        ORX.vkImageView = VK_NULL_HANDLE;
    }
    if (ORX.vkImage != VK_NULL_HANDLE)
    {
        vkDestroyImage(gVulkanContext.vkDevice, ORX.vkImage, nullptr);
        ORX.vkImage = VK_NULL_HANDLE;
    }
    if (ORX.vkDeviceMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(gVulkanContext.vkDevice, ORX.vkDeviceMemory, nullptr);
        ORX.vkDeviceMemory = VK_NULL_HANDLE;
    }

	////descriptor set
 //   if( vkDescriptorSet != VK_NULL_HANDLE)
 //   {
 //       // Note: vkFreeDescriptorSets is not strictly necessary, as all descriptor sets will be freed when the descriptor pool is destroyed.
 //       vkFreeDescriptorSets(vkDevice, vkDescriptorPool, 1, &vkDescriptorSet);
 //       vkDescriptorSet = VK_NULL_HANDLE;
	//}

}

VkResult Material_BasicPBR::createDescriptorSet(void)
{
    // local variables
    VkResult vkResult = VK_SUCCESS;

    // code
    // Declare and initialize VkDescriptorSetAllocateInfo structure.
    VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
    memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));

    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool; // global descriptor pool
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1;//
    vkDescriptorSetAllocateInfo.pSetLayouts = &vkDescriptorSetLayout; // pointer to the descriptor set layout

    // Call vkAllocateDescriptorSets() to allocate the descriptor set
    vkResult = vkAllocateDescriptorSets(gVulkanContext.vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFILE, "createDiscriptorSet() : vkAllocateDescriptorSets() failed.\n");
        return(vkResult);
    }
    // Declare and initialize VkDescriptorImageInfo structure which will have information about the albedo image.
    VkDescriptorImageInfo vkDescriptorImageInfo_Albedo;
    memset((void*)&vkDescriptorImageInfo_Albedo, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo_Albedo.sampler = Albedo.vkSampler; // sampler for the albedo image
    vkDescriptorImageInfo_Albedo.imageView = Albedo.vkImageView; // image view for the albedo image
    vkDescriptorImageInfo_Albedo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // image layout for the albedo image
    // Declare and initialize VkDescriptorImageInfo structure which will have information about the normal image.
    VkDescriptorImageInfo vkDescriptorImageInfo_Normal;
    memset((void*)&vkDescriptorImageInfo_Normal, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo_Normal.sampler = Normal.vkSampler; // sampler for the normal image
    vkDescriptorImageInfo_Normal.imageView = Normal.vkImageView; // image view for the normal image
    vkDescriptorImageInfo_Normal.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // image layout for the normal image

    VkDescriptorImageInfo vkDescriptorImageInfo_ORX;
    memset((void*)&vkDescriptorImageInfo_ORX, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo_ORX.sampler = ORX.vkSampler; // sampler for the normal image
    vkDescriptorImageInfo_ORX.imageView = ORX.vkImageView; // image view for the normal image
    vkDescriptorImageInfo_ORX.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // image layout for the normal image

    //write or copy the descriptor set with the albedo and normal image information
    // Declare and initialize VkWriteDescriptorSet structure which will have information about the descriptor set.
    VkWriteDescriptorSet vkWriteDescriptorSet_array[3];
    memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));
    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].pNext = NULL;
    vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet; // descriptor set
    vkWriteDescriptorSet_array[0].dstBinding = 0; // 0 means the index number of the binding
    vkWriteDescriptorSet_array[0].dstArrayElement = 0; // 0 means the index number of the array element
    vkWriteDescriptorSet_array[0].descriptorCount = 1; // we are using only one descriptor
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // type of the descriptor
    vkWriteDescriptorSet_array[0].pImageInfo = &vkDescriptorImageInfo_Albedo; // pointer to the albedo image info
    vkWriteDescriptorSet_array[0].pBufferInfo = NULL; // no buffer info
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL; // no texel buffer view

    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].pNext = NULL;
    vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSet; // descriptor set
    vkWriteDescriptorSet_array[1].dstBinding = 1; // 1 means the index number of the binding
    vkWriteDescriptorSet_array[1].dstArrayElement = 0; // 0 means the index number of the array element
    vkWriteDescriptorSet_array[1].descriptorCount = 1; // we are using only one descriptor
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // type of the descriptor
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo_Normal; // pointer to the normal image info
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL; // no buffer info
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL; // no tex

    vkWriteDescriptorSet_array[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[2].pNext = NULL;
    vkWriteDescriptorSet_array[2].dstSet = vkDescriptorSet; // descriptor set
    vkWriteDescriptorSet_array[2].dstBinding = 2; // 1 means the index number of the binding
    vkWriteDescriptorSet_array[2].dstArrayElement = 0; // 0 means the index number of the array element
    vkWriteDescriptorSet_array[2].descriptorCount = 1; // we are using only one descriptor
    vkWriteDescriptorSet_array[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // type of the descriptor
    vkWriteDescriptorSet_array[2].pImageInfo = &vkDescriptorImageInfo_ORX; // pointer to the normal image info
    vkWriteDescriptorSet_array[2].pBufferInfo = NULL; // no buffer info
    vkWriteDescriptorSet_array[2].pTexelBufferView = NULL; // no tex


    // Call vkUpdateDescriptorSets() to update the descriptor set with the albedo and normal image information.
    vkUpdateDescriptorSets(gVulkanContext.vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);

    return vkResult;
}

uint32_t Material_BasicPBR::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(gVulkanContext.vkPhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    fprintf(gpFILE, "Failed to find suitable memory type.\n");
    exit(EXIT_FAILURE);
}

static inline uint64_t bc7_subresource_size(uint32_t width, uint32_t height)
{
    uint32_t blocksX = (width + 3) / 4; // ceil(width/4)
    uint32_t blocksY = (height + 3) / 4; // ceil(height/4)
    return (uint64_t)blocksX * (uint64_t)blocksY * 16ULL;
}


static inline uint64_t bc5_subresource_size(uint32_t width, uint32_t height)
{
    uint32_t blocksX = (width + 3) / 4; // ceil(width/4)
    uint32_t blocksY = (height + 3) / 4; // ceil(height/4)
    return (uint64_t)blocksX * (uint64_t)blocksY * 16ULL; // BC5 uses 16 bytes per 4x4 block (two 8-byte channels)
}

VkResult Material_BasicPBR::loadTextureData_dds_c_bc7(ImageData* imageData, const char* filename,VkSampler vkSampler, VkFormat format)
{
    if (!imageData || !filename) return VK_ERROR_INITIALIZATION_FAILED;

    // Validate that caller passed a BC7 format
    if (!(format == VK_FORMAT_BC7_UNORM_BLOCK || format == VK_FORMAT_BC7_SRGB_BLOCK)) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): format is not BC7 (format=%d)\n", (int)format);
        return VK_ERROR_FORMAT_NOT_SUPPORTED;
    }

    DDSFile dds;
    const auto res = dds.Load(filename);
    if (res != Result::Success) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): tinyddsloader failed to load '%s' (res=%d)\n", filename, static_cast<int>(res));
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    const uint32_t arrayCount = dds.GetArraySize();
    const uint32_t mipCount = dds.GetMipCount();
    const uint32_t baseWidth = dds.GetWidth();
    const uint32_t baseHeight = dds.GetHeight();
    const uint32_t depth = dds.GetDepth();

    if (arrayCount == 0 || mipCount == 0) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): empty DDS '%s'\n", filename);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    const size_t subCount = (size_t)arrayCount * (size_t)mipCount;

    // Subresource bookkeeping (C-style)
    typedef struct { uint32_t mip; uint32_t layer; uint64_t rawSize; uint64_t paddedSize; } SubInfo;
    SubInfo* subs = (SubInfo*)malloc(sizeof(SubInfo) * subCount);
    if (!subs) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): malloc subs failed\n");
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    // Compute raw and padded sizes using bc7_subresource_size()
    uint64_t totalSize = 0;
    size_t idx = 0;
    for (uint32_t layer = 0; layer < arrayCount; ++layer) {
        for (uint32_t mip = 0; mip < mipCount; ++mip) {
            const DDSFile::ImageData* img = dds.GetImageData(mip, layer);
            if (!img) {
                fprintf(gpFILE, "loadTextureData_dds_c_bc7(): missing ImageData layer=%u mip=%u\n", layer, mip);
                free(subs);
                return VK_ERROR_INITIALIZATION_FAILED;
            }

            // Use loader's reported mip dims if present; fallback to base >> mip with min 1
            uint32_t mipW = img->m_width ? img->m_width : (baseWidth >> mip ? (baseWidth >> mip) : 1u);
            uint32_t mipH = img->m_height ? img->m_height : (baseHeight >> mip ? (baseHeight >> mip) : 1u);

            uint64_t raw = bc7_subresource_size(mipW, mipH);
            uint64_t padded = (raw + 3ull) & ~3ull; // pad to 4 byte boundary for bufferOffset

            subs[idx].mip = mip;
            subs[idx].layer = layer;
            subs[idx].rawSize = raw;
            subs[idx].paddedSize = padded;
            totalSize += padded;
            ++idx;
        }
    }

    if (totalSize == 0) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): totalSize == 0 for '%s'\n", filename);
        free(subs);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Create staging buffer
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

    VkBufferCreateInfo bufInfo = {};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = totalSize;
    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult vkRes = vkCreateBuffer(gVulkanContext.vkDevice, &bufInfo, nullptr, &stagingBuffer);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkCreateBuffer failed (%d)\n", vkRes);
        free(subs);
        return vkRes;
    }

    VkMemoryRequirements memReq = {};
    vkGetBufferMemoryRequirements(gVulkanContext.vkDevice, stagingBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (allocInfo.memoryTypeIndex == UINT32_MAX) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): findMemoryType failed for staging\n");
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return VK_ERROR_MEMORY_MAP_FAILED;
    }

    vkRes = vkAllocateMemory(gVulkanContext.vkDevice, &allocInfo, nullptr, &stagingMemory);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkAllocateMemory failed (%d)\n", vkRes);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    vkRes = vkBindBufferMemory(gVulkanContext.vkDevice, stagingBuffer, stagingMemory, 0);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkBindBufferMemory failed (%d)\n", vkRes);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    // Map and copy BC7 subresources into staging (use bc7_subresource_size)
    void* mapped = NULL;
    vkRes = vkMapMemory(gVulkanContext.vkDevice, stagingMemory, 0, totalSize, 0, &mapped);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkMapMemory failed (%d)\n", vkRes);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    uint64_t offset = 0;
    idx = 0;
    for (uint32_t layer = 0; layer < arrayCount; ++layer) {
        for (uint32_t mip = 0; mip < mipCount; ++mip) {
            const DDSFile::ImageData* img = dds.GetImageData(mip, layer);
            // compute size (should match earlier)
            uint32_t mipW = img->m_width ? img->m_width : (baseWidth >> mip ? (baseWidth >> mip) : 1u);
            uint32_t mipH = img->m_height ? img->m_height : (baseHeight >> mip ? (baseHeight >> mip) : 1u);
            uint64_t size = bc7_subresource_size(mipW, mipH);

            if (!img->m_mem) {
                fprintf(gpFILE, "loadTextureData_dds_c_bc7(): null m_mem for layer=%u mip=%u\n", layer, mip);
                vkUnmapMemory(gVulkanContext.vkDevice, stagingMemory);
                vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
                vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
                free(subs);
                return VK_ERROR_INITIALIZATION_FAILED;
            }

            // copy computed number of bytes from loader pointer
            memcpy((uint8_t*)mapped + offset, img->m_mem, (size_t)size);

            // pad to paddedSize if required
            if (subs[idx].paddedSize > size) {
                memset((uint8_t*)mapped + offset + size, 0, (size_t)(subs[idx].paddedSize - size));
            }

            offset += subs[idx].paddedSize;
            ++idx;
        }
    }

    vkUnmapMemory(gVulkanContext.vkDevice, stagingMemory);

    // Create VkImage with the DDS mipCount / arrayCount
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format; // BC7_UNORM or BC7_SRGB as validated earlier
    imageInfo.extent.width = baseWidth;
    imageInfo.extent.height = baseHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipCount;
    imageInfo.arrayLayers = arrayCount;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.flags = (arrayCount == 6) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;

    VkImage image = VK_NULL_HANDLE;
    vkRes = vkCreateImage(gVulkanContext.vkDevice, &imageInfo, nullptr, &image);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkCreateImage failed (%d)\n", vkRes);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    VkMemoryRequirements imageMemReq = {};
    vkGetImageMemoryRequirements(gVulkanContext.vkDevice, image, &imageMemReq);

    VkMemoryAllocateInfo imageAlloc = {};
    imageAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    imageAlloc.allocationSize = imageMemReq.size;
    imageAlloc.memoryTypeIndex = findMemoryType(imageMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (imageAlloc.memoryTypeIndex == UINT32_MAX) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): findMemoryType failed for image\n");
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return VK_ERROR_MEMORY_MAP_FAILED;
    }

    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    vkRes = vkAllocateMemory(gVulkanContext.vkDevice, &imageAlloc, nullptr, &imageMemory);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkAllocateMemory(image) failed (%d)\n", vkRes);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    vkRes = vkBindImageMemory(gVulkanContext.vkDevice, image, imageMemory, 0);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkBindImageMemory failed (%d)\n", vkRes);
        vkFreeMemory(gVulkanContext.vkDevice, imageMemory, nullptr);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    // Command buffer: transition, copy, transition
    VkCommandBufferAllocateInfo cmdAlloc = {};
    cmdAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAlloc.commandPool = gVulkanContext.vkCommandPool;
    cmdAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAlloc.commandBufferCount = 1;

    VkCommandBuffer cmd = VK_NULL_HANDLE;
    vkRes = vkAllocateCommandBuffers(gVulkanContext.vkDevice, &cmdAlloc, &cmd);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkAllocateCommandBuffers failed (%d)\n", vkRes);
        vkFreeMemory(gVulkanContext.vkDevice, imageMemory, nullptr);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkRes = vkBeginCommandBuffer(cmd, &beginInfo);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkBeginCommandBuffer failed (%d)\n", vkRes);
        vkFreeCommandBuffers(gVulkanContext.vkDevice, gVulkanContext.vkCommandPool, 1, &cmd);
        vkFreeMemory(gVulkanContext.vkDevice, imageMemory, nullptr);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    VkImageMemoryBarrier barrierToTransfer = {};
    barrierToTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrierToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrierToTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierToTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierToTransfer.image = image;
    barrierToTransfer.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrierToTransfer.subresourceRange.baseMipLevel = 0;
    barrierToTransfer.subresourceRange.levelCount = mipCount;
    barrierToTransfer.subresourceRange.baseArrayLayer = 0;
    barrierToTransfer.subresourceRange.layerCount = arrayCount;
    barrierToTransfer.srcAccessMask = 0;
    barrierToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrierToTransfer);

    // Build copy regions (C array)
    VkBufferImageCopy* copies = (VkBufferImageCopy*)malloc(sizeof(VkBufferImageCopy) * subCount);
    if (!copies) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): malloc copies failed\n");
        vkFreeCommandBuffers(gVulkanContext.vkDevice, gVulkanContext.vkCommandPool, 1, &cmd);
        vkFreeMemory(gVulkanContext.vkDevice, imageMemory, nullptr);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    offset = 0;
    idx = 0;
    for (uint32_t layer = 0; layer < arrayCount; ++layer) {
        for (uint32_t mip = 0; mip < mipCount; ++mip) {
            const DDSFile::ImageData* img = dds.GetImageData(mip, layer);
            VkBufferImageCopy region = {};
            region.bufferOffset = offset;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = mip;
            region.imageSubresource.baseArrayLayer = layer;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = { 0,0,0 };
            region.imageExtent = { img->m_width ? img->m_width : (baseWidth >> mip ? (baseWidth >> mip) : 1u),
                                   img->m_height ? img->m_height : (baseHeight >> mip ? (baseHeight >> mip) : 1u),
                                   img->m_depth ? img->m_depth : 1u };
            copies[idx] = region;
            offset += subs[idx].paddedSize;
            ++idx;
        }
    }

    vkCmdCopyBufferToImage(cmd, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        (uint32_t)subCount, copies);

    VkImageMemoryBarrier barrierToReadable = barrierToTransfer;
    barrierToReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrierToReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrierToReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrierToReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrierToReadable);

    // Finish & submit
    vkRes = vkEndCommandBuffer(cmd);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkEndCommandBuffer failed (%d)\n", vkRes);
        free(copies);
        free(subs);
        vkFreeCommandBuffers(gVulkanContext.vkDevice, gVulkanContext.vkCommandPool, 1, &cmd);
        vkFreeMemory(gVulkanContext.vkDevice, imageMemory, nullptr);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        return vkRes;
    }

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;

    vkRes = vkQueueSubmit(gVulkanContext.vkGraphicsQueue, 1, &submit, VK_NULL_HANDLE);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkQueueSubmit failed (%d)\n", vkRes);
        free(copies);
        free(subs);
        vkFreeCommandBuffers(gVulkanContext.vkDevice, gVulkanContext.vkCommandPool, 1, &cmd);
        vkFreeMemory(gVulkanContext.vkDevice, imageMemory, nullptr);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        return vkRes;
    }

    vkRes = vkQueueWaitIdle(gVulkanContext.vkGraphicsQueue);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkQueueWaitIdle failed (%d)\n", vkRes);
        free(copies);
        free(subs);
        vkFreeCommandBuffers(gVulkanContext.vkDevice, gVulkanContext.vkCommandPool, 1, &cmd);
        vkFreeMemory(gVulkanContext.vkDevice, imageMemory, nullptr);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        return vkRes;
    }

    // free command buffer
    vkFreeCommandBuffers(gVulkanContext.vkDevice, gVulkanContext.vkCommandPool, 1, &cmd);

    // Create image view
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    if (arrayCount == 1) viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    else if (arrayCount == 6 && (imageInfo.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)) viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    else viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipCount;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = arrayCount;

    VkImageView imageView = VK_NULL_HANDLE;
    vkRes = vkCreateImageView(gVulkanContext.vkDevice, &viewInfo, nullptr, &imageView);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc7(): vkCreateImageView failed (%d)\n", vkRes);
        free(copies);
        free(subs);
        vkFreeMemory(gVulkanContext.vkDevice, imageMemory, nullptr);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        return vkRes;
    }

    // cleanup staging
    vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);

    // fill output
    imageData->vkImage = image;
    imageData->vkDeviceMemory = imageMemory;
    imageData->vkImageView = imageView;
	imageData->vkSampler = vkSampler;
    imageData->globalTextureArrayIndex = (uint32_t)global_textureArray.size();
	global_textureArray.push_back(imageData);


    // imageData->width = baseWidth;
    // imageData->height = baseHeight;
    // imageData->mipLevels = mipCount;
    // imageData->arrayLayers = arrayCount;

    fprintf(gpFILE, "loadTextureData_dds_c_bc7(): Loaded '%s' w=%u h=%u mips=%u layers=%u (BC7)\n",
        filename, baseWidth, baseHeight, mipCount, arrayCount);

    // free temporaries
    free(copies);
    free(subs);

    return VK_SUCCESS;
}


VkResult Material_BasicPBR::loadTextureData_dds_c_bc5_normal(ImageData* imageData, const char* filename, VkSampler vkSampler, VkFormat format)
{
    if (!imageData || !filename) return VK_ERROR_INITIALIZATION_FAILED;

    // Validate that caller passed a BC5 format (UNORM or SNORM)
    if (!(format == VK_FORMAT_BC5_UNORM_BLOCK || format == VK_FORMAT_BC5_SNORM_BLOCK)) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): format is not BC5 (format=%d)\n", (int)format);
        return VK_ERROR_FORMAT_NOT_SUPPORTED;
    }

    DDSFile dds;
    const auto res = dds.Load(filename);
    if (res != Result::Success) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): tinyddsloader failed to load '%s' (res=%d)\n", filename, static_cast<int>(res));
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    const uint32_t arrayCount = dds.GetArraySize();
    const uint32_t mipCount = dds.GetMipCount();
    const uint32_t baseWidth = dds.GetWidth();
    const uint32_t baseHeight = dds.GetHeight();
    const uint32_t depth = dds.GetDepth();

    if (arrayCount == 0 || mipCount == 0) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): empty DDS '%s'\n", filename);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    const size_t subCount = (size_t)arrayCount * (size_t)mipCount;

    // Subresource bookkeeping (C-style)
    typedef struct { uint32_t mip; uint32_t layer; uint64_t rawSize; uint64_t paddedSize; } SubInfo;
    SubInfo* subs = (SubInfo*)malloc(sizeof(SubInfo) * subCount);
    if (!subs) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): malloc subs failed\n");
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    // Compute raw and padded sizes using bc5_subresource_size()
    uint64_t totalSize = 0;
    size_t idx = 0;
    for (uint32_t layer = 0; layer < arrayCount; ++layer) {
        for (uint32_t mip = 0; mip < mipCount; ++mip) {
            const DDSFile::ImageData* img = dds.GetImageData(mip, layer);
            if (!img) {
                fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): missing ImageData layer=%u mip=%u\n", layer, mip);
                free(subs);
                return VK_ERROR_INITIALIZATION_FAILED;
            }

            // Use loader's reported mip dims if present; fallback to base >> mip with min 1
            uint32_t mipW = img->m_width ? img->m_width : (baseWidth >> mip ? (baseWidth >> mip) : 1u);
            uint32_t mipH = img->m_height ? img->m_height : (baseHeight >> mip ? (baseHeight >> mip) : 1u);

            uint64_t raw = bc5_subresource_size(mipW, mipH);
            uint64_t padded = (raw + 3ull) & ~3ull; // pad to 4 byte boundary for bufferOffset

            subs[idx].mip = mip;
            subs[idx].layer = layer;
            subs[idx].rawSize = raw;
            subs[idx].paddedSize = padded;
            totalSize += padded;
            ++idx;
        }
    }

    if (totalSize == 0) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): totalSize == 0 for '%s'\n", filename);
        free(subs);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Create staging buffer
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

    VkBufferCreateInfo bufInfo = {};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = totalSize;
    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult vkRes = vkCreateBuffer(gVulkanContext.vkDevice, &bufInfo, nullptr, &stagingBuffer);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): vkCreateBuffer failed (%d)\n", vkRes);
        free(subs);
        return vkRes;
    }

    VkMemoryRequirements memReq = {};
    vkGetBufferMemoryRequirements(gVulkanContext.vkDevice, stagingBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (allocInfo.memoryTypeIndex == UINT32_MAX) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): findMemoryType failed for staging\n");
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return VK_ERROR_MEMORY_MAP_FAILED;
    }

    vkRes = vkAllocateMemory(gVulkanContext.vkDevice, &allocInfo, nullptr, &stagingMemory);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): vkAllocateMemory failed (%d)\n", vkRes);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    vkRes = vkBindBufferMemory(gVulkanContext.vkDevice, stagingBuffer, stagingMemory, 0);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): vkBindBufferMemory failed (%d)\n", vkRes);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    // Map and copy BC5 subresources into staging (use bc5_subresource_size)
    void* mapped = NULL;
    vkRes = vkMapMemory(gVulkanContext.vkDevice, stagingMemory, 0, totalSize, 0, &mapped);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): vkMapMemory failed (%d)\n", vkRes);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    uint64_t offset = 0;
    idx = 0;
    for (uint32_t layer = 0; layer < arrayCount; ++layer) {
        for (uint32_t mip = 0; mip < mipCount; ++mip) {
            const DDSFile::ImageData* img = dds.GetImageData(mip, layer);
            // compute size (should match earlier)
            uint32_t mipW = img->m_width ? img->m_width : (baseWidth >> mip ? (baseWidth >> mip) : 1u);
            uint32_t mipH = img->m_height ? img->m_height : (baseHeight >> mip ? (baseHeight >> mip) : 1u);
            uint64_t size = bc5_subresource_size(mipW, mipH);

            if (!img->m_mem) {
                fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): null m_mem for layer=%u mip=%u\n", layer, mip);
                vkUnmapMemory(gVulkanContext.vkDevice, stagingMemory);
                vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
                vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
                free(subs);
                return VK_ERROR_INITIALIZATION_FAILED;
            }

            // copy computed number of bytes from loader pointer
            memcpy((uint8_t*)mapped + offset, img->m_mem, (size_t)size);

            // pad to paddedSize if required
            if (subs[idx].paddedSize > size) {
                memset((uint8_t*)mapped + offset + size, 0, (size_t)(subs[idx].paddedSize - size));
            }

            offset += subs[idx].paddedSize;
            ++idx;
        }
    }

    vkUnmapMemory(gVulkanContext.vkDevice, stagingMemory);

    // Create VkImage with the DDS mipCount / arrayCount
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format; // BC5_UNORM or BC5_SNORM as validated earlier
    imageInfo.extent.width = baseWidth;
    imageInfo.extent.height = baseHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipCount;
    imageInfo.arrayLayers = arrayCount;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.flags = (arrayCount == 6) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;

    VkImage image = VK_NULL_HANDLE;
    vkRes = vkCreateImage(gVulkanContext.vkDevice, &imageInfo, nullptr, &image);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): vkCreateImage failed (%d)\n", vkRes);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    VkMemoryRequirements imageMemReq = {};
    vkGetImageMemoryRequirements(gVulkanContext.vkDevice, image, &imageMemReq);

    VkMemoryAllocateInfo imageAlloc = {};
    imageAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    imageAlloc.allocationSize = imageMemReq.size;
    imageAlloc.memoryTypeIndex = findMemoryType(imageMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (imageAlloc.memoryTypeIndex == UINT32_MAX) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): findMemoryType failed for image\n");
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return VK_ERROR_MEMORY_MAP_FAILED;
    }

    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    vkRes = vkAllocateMemory(gVulkanContext.vkDevice, &imageAlloc, nullptr, &imageMemory);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): vkAllocateMemory(image) failed (%d)\n", vkRes);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    vkRes = vkBindImageMemory(gVulkanContext.vkDevice, image, imageMemory, 0);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): vkBindImageMemory failed (%d)\n", vkRes);
        vkFreeMemory(gVulkanContext.vkDevice, imageMemory, nullptr);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    // Command buffer: transition, copy, transition
    VkCommandBufferAllocateInfo cmdAlloc = {};
    cmdAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAlloc.commandPool = gVulkanContext.vkCommandPool;
    cmdAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAlloc.commandBufferCount = 1;

    VkCommandBuffer cmd = VK_NULL_HANDLE;
    vkRes = vkAllocateCommandBuffers(gVulkanContext.vkDevice, &cmdAlloc, &cmd);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): vkAllocateCommandBuffers failed (%d)\n", vkRes);
        vkFreeMemory(gVulkanContext.vkDevice, imageMemory, nullptr);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkRes = vkBeginCommandBuffer(cmd, &beginInfo);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): vkBeginCommandBuffer failed (%d)\n", vkRes);
        vkFreeCommandBuffers(gVulkanContext.vkDevice, gVulkanContext.vkCommandPool, 1, &cmd);
        vkFreeMemory(gVulkanContext.vkDevice, imageMemory, nullptr);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return vkRes;
    }

    VkImageMemoryBarrier barrierToTransfer = {};
    barrierToTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierToTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrierToTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrierToTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierToTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrierToTransfer.image = image;
    barrierToTransfer.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrierToTransfer.subresourceRange.baseMipLevel = 0;
    barrierToTransfer.subresourceRange.levelCount = mipCount;
    barrierToTransfer.subresourceRange.baseArrayLayer = 0;
    barrierToTransfer.subresourceRange.layerCount = arrayCount;
    barrierToTransfer.srcAccessMask = 0;
    barrierToTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrierToTransfer);

    // Build copy regions (C array)
    VkBufferImageCopy* copies = (VkBufferImageCopy*)malloc(sizeof(VkBufferImageCopy) * subCount);
    if (!copies) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): malloc copies failed\n");
        vkFreeCommandBuffers(gVulkanContext.vkDevice, gVulkanContext.vkCommandPool, 1, &cmd);
        vkFreeMemory(gVulkanContext.vkDevice, imageMemory, nullptr);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        free(subs);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    offset = 0;
    idx = 0;
    for (uint32_t layer = 0; layer < arrayCount; ++layer) {
        for (uint32_t mip = 0; mip < mipCount; ++mip) {
            const DDSFile::ImageData* img = dds.GetImageData(mip, layer);
            VkBufferImageCopy region = {};
            region.bufferOffset = offset;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = mip;
            region.imageSubresource.baseArrayLayer = layer;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = { 0,0,0 };
            region.imageExtent = { img->m_width ? img->m_width : (baseWidth >> mip ? (baseWidth >> mip) : 1u),
                                   img->m_height ? img->m_height : (baseHeight >> mip ? (baseHeight >> mip) : 1u),
                                   img->m_depth ? img->m_depth : 1u };
            copies[idx] = region;
            offset += subs[idx].paddedSize;
            ++idx;
        }
    }

    vkCmdCopyBufferToImage(cmd, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        (uint32_t)subCount, copies);

    VkImageMemoryBarrier barrierToReadable = barrierToTransfer;
    barrierToReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrierToReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrierToReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrierToReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrierToReadable);

    // Finish & submit
    vkRes = vkEndCommandBuffer(cmd);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): vkEndCommandBuffer failed (%d)\n", vkRes);
        free(copies);
        free(subs);
        vkFreeCommandBuffers(gVulkanContext.vkDevice, gVulkanContext.vkCommandPool, 1, &cmd);
        vkFreeMemory(gVulkanContext.vkDevice, imageMemory, nullptr);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        return vkRes;
    }

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;

    vkRes = vkQueueSubmit(gVulkanContext.vkGraphicsQueue, 1, &submit, VK_NULL_HANDLE);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): vkQueueSubmit failed (%d)\n", vkRes);
        free(copies);
        free(subs);
        vkFreeCommandBuffers(gVulkanContext.vkDevice, gVulkanContext.vkCommandPool, 1, &cmd);
        vkFreeMemory(gVulkanContext.vkDevice, imageMemory, nullptr);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        return vkRes;
    }

    vkRes = vkQueueWaitIdle(gVulkanContext.vkGraphicsQueue);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): vkQueueWaitIdle failed (%d)\n", vkRes);
        free(copies);
        free(subs);
        vkFreeCommandBuffers(gVulkanContext.vkDevice, gVulkanContext.vkCommandPool, 1, &cmd);
        vkFreeMemory(gVulkanContext.vkDevice, imageMemory, nullptr);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        return vkRes;
    }

    // free command buffer
    vkFreeCommandBuffers(gVulkanContext.vkDevice, gVulkanContext.vkCommandPool, 1, &cmd);

    // Create image view
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    if (arrayCount == 1) viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    else if (arrayCount == 6 && (imageInfo.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)) viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    else viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipCount;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = arrayCount;

    VkImageView imageView = VK_NULL_HANDLE;
    vkRes = vkCreateImageView(gVulkanContext.vkDevice, &viewInfo, nullptr, &imageView);
    if (vkRes != VK_SUCCESS) {
        fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): vkCreateImageView failed (%d)\n", vkRes);
        free(copies);
        free(subs);
        vkFreeMemory(gVulkanContext.vkDevice, imageMemory, nullptr);
        vkDestroyImage(gVulkanContext.vkDevice, image, nullptr);
        vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);
        vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
        return vkRes;
    }

    // cleanup staging
    vkDestroyBuffer(gVulkanContext.vkDevice, stagingBuffer, nullptr);
    vkFreeMemory(gVulkanContext.vkDevice, stagingMemory, nullptr);

    // fill output
    imageData->vkImage = image;
    imageData->vkDeviceMemory = imageMemory;
    imageData->vkImageView = imageView;
	imageData->vkSampler = vkSampler;
	imageData->globalTextureArrayIndex = (uint32_t)global_textureArray.size();
	global_textureArray.push_back(imageData);
    // imageData->width = baseWidth;
    // imageData->height = baseHeight;
    // imageData->mipLevels = mipCount;
    // imageData->arrayLayers = arrayCount;

    fprintf(gpFILE, "loadTextureData_dds_c_bc5_normal(): Loaded '%s' w=%u h=%u mips=%u layers=%u (BC5)\n",
        filename, baseWidth, baseHeight, mipCount, arrayCount);

    // free temporaries
    free(copies);
    free(subs);

    return VK_SUCCESS;
}