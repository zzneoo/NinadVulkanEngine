#pragma once

//#include <vector>
//#include <cstdint>

#include "StaticMeshletModel.h"

class MeshletScene
{
public:

    MeshletScene() = default;
    ~MeshletScene();

    void ShutDown(void);

    MeshletScene(const MeshletScene&) = delete;
    MeshletScene& operator=(const MeshletScene&) = delete;

    // ------------------------------------------------------------
    // Add models
    // ------------------------------------------------------------

    void AddModel(StaticMeshletModel* model);


    // ------------------------------------------------------------
    // Build global CPU meshlet data
    // ------------------------------------------------------------

    VkResult Build();


    // ------------------------------------------------------------
    // Global counts
    // ------------------------------------------------------------

    uint32_t GetMeshletCount() const
    {
        return static_cast<uint32_t>(
            meshlets.size());
    }


    // ------------------------------------------------------------
    // Global GPU addresses
    // ------------------------------------------------------------

    VkDeviceAddress GetMeshletDataAddress() const
    {
        return meshletGPUData.meshletBuffer.deviceAddress;
    }

    VkDeviceAddress GetMeshletVerticesAddress() const
    {
        return meshletGPUData.meshletVertexBuffer.deviceAddress;
    }

    VkDeviceAddress GetMeshletTrianglesAddress() const
    {
        return meshletGPUData.meshletTriangleBuffer.deviceAddress;
    }

    VkDeviceAddress GetVertexDataAddress() const
    {
        return meshletGPUData.vertexBuffer.deviceAddress;
    }

    VkDeviceAddress GetModelDataAddress() const
    {
        return meshletGPUData.modelDataBuffer.deviceAddress;
    }


private:

    // ------------------------------------------------------------
    // CPU-side merged data
    // ------------------------------------------------------------

    std::vector<VertexData_Meshlet>
        vertices;

    std::vector<MeshletData>
        meshlets;

    std::vector<uint32_t>
        meshletVertices;

    std::vector<uint8_t>
        meshletTriangles;

    //Global Model Matrices
    std::vector<ModelData> gModelData;


    // ------------------------------------------------------------
    // Source models
    // ------------------------------------------------------------

    std::vector<StaticMeshletModel*>
        models;


    // ------------------------------------------------------------
    // GPU buffers
    // ------------------------------------------------------------

    MeshletGPUData meshletGPUData;

    // ------------------------------------------------------------
    // Internal functions
    // ------------------------------------------------------------

    void MergeModel(const StaticMeshletModel& model, uint32_t modelIndex);

    VkResult CreateGlobalSSBOs();
    void DestroyGlobalSSBOs();

    VkResult CreateDeviceLocalSSBO(const void* data, VkDeviceSize size, VulkanSSBO& outSSBO);

};