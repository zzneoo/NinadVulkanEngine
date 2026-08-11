#pragma once
#define NOMINMAX

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
#include "MeshOptimizer/meshoptimizer.h"
#include "DescriptorSetLayouts.h"

extern FILE* gpFILE;

class StaticMeshletModel
{
public:

    StaticMeshletModel(
        const char* modelPath,
        VkDescriptorSetLayout vkDescriptorSetLayout);

    ~StaticMeshletModel();



    const std::vector<VertexData_Meshlet>&
        GetVertices() const
    {
        return vkVertices;
    }

    const std::vector<MeshletData>&
        GetMeshlets() const
    {
        return meshlets;
    }

    const std::vector<uint32_t>&
        GetMeshletVertices() const
    {
        return meshletVertices;
    }

    const std::vector<uint8_t>&
        GetMeshletTriangles() const
    {
        return meshletTriangles;
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
                "StaticMeshletModel::GetMaterialDescriptorSet: "
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
        VkDescriptorSetLayout vkDescriptorSetLayout);


    // ------------------------------------------------------------
    // Vulkan buffers
    // ------------------------------------------------------------



    VkResult CreateMeshlets(
        const std::vector<uint32_t>& indices,
        const std::vector<VertexData_Meshlet>& vertices);


    // ------------------------------------------------------------
    // Assimp
    // ------------------------------------------------------------

    Assimp::Importer importer;

    const aiScene* scene = nullptr;


    // ------------------------------------------------------------
    // Vulkan
    // ------------------------------------------------------------

    // ------------------------------------------------------------
    // Meshlets
    // ------------------------------------------------------------
    std::vector<VertexData_Meshlet> vkVertices;

    std::vector<MeshletData> meshlets;

    std::vector<uint32_t> meshletVertices;

    std::vector<uint8_t> meshletTriangles;


    // ------------------------------------------------------------
    // Materials
    // ------------------------------------------------------------

    std::vector<MaterialInfo> PBR_Materials;


};
