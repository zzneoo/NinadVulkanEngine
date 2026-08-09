#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

#include <VK.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>

#include "Material_BasicPBR.h"
#include "VulkanContext.h"

extern FILE* gpFILE;



struct StaticMaterialInfo
{
    std::string materialName;
    std::string path;

    Material_BasicPBR* data = nullptr;
};


class StaticModel
{
public:

    StaticModel(
        const char* modelPath,
        bool index32,
        VkDescriptorSetLayout vkDescriptorSetLayout);

    ~StaticModel();


    // ------------------------------------------------------------
    // Rendering
    // ------------------------------------------------------------

    const VulkanComboData* GetVulkanComboData() const
    {
        return &vulkanComboData;
    }


    // ------------------------------------------------------------
    // Materials
    // ------------------------------------------------------------

    VkDescriptorSet GetMaterialDescriptorSet(uint32_t materialIndex) const
    {
        if (materialIndex >= PBR_Materials.size())
        {
            fprintf(
                gpFILE,
                "StaticModel::GetMaterialDescriptorSet: "
                "Invalid material index %u\n",
                materialIndex);

            return VK_NULL_HANDLE;
        }

        return PBR_Materials[materialIndex].data->getDescriptorSet();
    }


    glm::uvec4 GetPBR_MaterialGlobalIDs() const
    {
        if (PBR_Materials.empty())
            return glm::uvec4(0);

        return PBR_Materials[0].data->GetPBR_MaterialGlobalIDs();
    }


private:

    // ------------------------------------------------------------
    // Model loading
    // ------------------------------------------------------------

    VkResult LoadModel_Static_PBR(
        const char* modelPath,
        bool index32,
        VkDescriptorSetLayout vkDescriptorSetLayout);


    // ------------------------------------------------------------
    // Vulkan buffers
    // ------------------------------------------------------------

    VkResult ZzCreateVertexBuffer(
        const float* vertices,
        VkDeviceSize vertexBufferSize,
        VulkanData* vulkanData);


    VkResult ZzCreateIndex16Buffer(
        const uint16_t* indices,
        VkDeviceSize indexBufferSize,
        VulkanData* vulkanData);


    VkResult ZzCreateIndex32Buffer(
        const uint32_t* indices,
        VkDeviceSize indexBufferSize,
        VulkanData* vulkanData);


    VkResult ZzCreateVertexAndIndex16Buffer(
        const float* vertices,
        VkDeviceSize vertexBufferSize,
        const uint16_t* indices,
        VkDeviceSize indexBufferSize,
        VulkanComboData* vulkanComboData);


    VkResult ZzCreateVertexAndIndex32Buffer(
        const float* vertices,
        VkDeviceSize vertexBufferSize,
        const uint32_t* indices,
        VkDeviceSize indexBufferSize,
        VulkanComboData* vulkanComboData);


    void ZzDestroyVertexBuffer(
        VulkanData* vulkanData);


    void ZzDestroyIndexBuffer(
        VulkanData* vulkanData);


    void ZzDestroyVertexAndIndexBuffer(
        VulkanComboData* vulkanComboData);


private:

    // ------------------------------------------------------------
    // Assimp
    // ------------------------------------------------------------

    Assimp::Importer importer;

    const aiScene* scene = nullptr;


    // ------------------------------------------------------------
    // Vulkan
    // ------------------------------------------------------------

    VulkanComboData vulkanComboData;


    // ------------------------------------------------------------
    // Materials
    // ------------------------------------------------------------

    std::vector<StaticMaterialInfo> PBR_Materials;
};