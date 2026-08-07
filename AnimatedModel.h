#pragma once
#include <glm/glm.hpp>
#include<glm/gtc/quaternion.hpp>
#include <unordered_map>
#include <string>
#include <VK.h>
// Assimp related header files
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
//filesystem
#include <filesystem>
#include "Material_BasicPBR.h"
#include "VulkanContext.h"

extern FILE* gpFILE;
//extern VkDevice vkDevice;
//extern VkPhysicalDevice vkPhysicalDevice;
//extern VkCommandPool vkCommandPool;
//extern VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;
//extern VkQueue vkQueue;
//extern VkPhysicalDevice vkPhysicalDevice_Selected;

struct MaterialInfo {
    std::string materialName;
    std::string path;
    Material_BasicPBR* data;
};


class AnimatedModel
{
public:
    AnimatedModel(const char* modelPath, bool index32, VkDescriptorSetLayout vkDescriptorSetLayout);
    ~AnimatedModel();

	const uint16_t GetNumBones() const { return numBones; }
    void UpdateAnimation(float deltaTime, uint16_t currFrame);
	const VulkanComboData* GetVulkanComboData() const { return &vulkanComboData; }
	const VulkanSSBO* GetBoneSSBOs() const { return boneSSBO; }
    VkDescriptorSet GetMaterialDescriptorSet(uint32_t materialIndex) const 
    {
        if (materialIndex >= PBR_Materials.size()) 
        {
            fprintf(gpFILE, "GetMaterialDescriptorSet: Invalid material index %u\n", materialIndex);
            return VK_NULL_HANDLE;
        }
        return PBR_Materials[materialIndex].data->getDescriptorSet();
	}

    const glm::uvec4 GetPBR_MaterialGlobalIDs() const
    {
        // Implementation needed
		return PBR_Materials[0].data->GetPBR_MaterialGlobalIDs();
    }

private:
    static glm::mat4 aiMat4ToGlm(const aiMatrix4x4& m);
    // Helper: find memory type
    //uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void AddBoneDataToVertex(VertexData_Skinned& v, int boneIndex, float weight);
    VkResult LoadModel_Animated_PBR(const char* modelPath, bool index32, VkDescriptorSetLayout vkDescriptorSetLayout);

    const aiNodeAnim* FindNodeAnim(const aiAnimation* anim, const std::string& name);

    uint32_t FindPositionKey(float animationTime, const aiNodeAnim* channel);
    uint32_t FindRotationKey(float animationTime, const aiNodeAnim* channel);
    uint32_t FindScalingKey(float animationTime, const aiNodeAnim* channel);

    glm::vec3 InterpolatePosition(float time, const aiNodeAnim* channel);
    glm::quat InterpolateRotation(float time, const aiNodeAnim* channel);
    glm::vec3 InterpolateScale(float animationTime, const aiNodeAnim* channel);

    void ReadNodeHierarchy(float animationTime,
        const aiAnimation* animation,
        aiNode* node,
        const glm::mat4& parentTransform);

    // createSSBO: creates a storage buffer backed by host-visible coherent memory and maps it persistently
    VkResult createSSBO(VkDevice device, VkPhysicalDevice physicalDevice, VulkanSSBO& outSSBO);

    VkResult ZzCreateVertexBuffer(const float* vertices, VkDeviceSize vertexBufferSize, VulkanData* vulkanData);
    VkResult ZzCreateIndex16Buffer(const uint16_t* indices, VkDeviceSize indexBufferSize, VulkanData* vulkanData);
    VkResult ZzCreateIndex32Buffer(const uint32_t* indices, VkDeviceSize indexBufferSize, VulkanData* vulkanData);
    VkResult ZzCreateVertexAndIndex16Buffer(
        const float* vertices, VkDeviceSize vertexBufferSize,
        const uint16_t* indices, VkDeviceSize indexBufferSize,
        VulkanComboData* vulkanComboData
    );
    VkResult ZzCreateVertexAndIndex32Buffer(
        const float* vertices, VkDeviceSize vertexBufferSize,
        const uint32_t* indices, VkDeviceSize indexBufferSize,
        VulkanComboData* vulkanComboData
    );

    void ZzDestroyVertexBuffer(VulkanData* vulkanData);
    void ZzDestroyIndexBuffer(VulkanData* vulkanData);
    void ZzDestroyVertexAndIndexBuffer(VulkanComboData* vulkanComboData);

    Assimp::Importer importer;   // MUST live as long as the model
    const aiScene* scene = nullptr;
    const aiAnimation* currentAnimation = nullptr;

	const float animationSpeed = 1.0f;
	float animationTime = 0.0f;

    glm::mat4 globalInverseTransform;
    std::unordered_map<std::string, uint32_t> boneMapping;
    std::vector<glm::mat4> boneOffsetMatrices;
    std::vector<glm::mat4> finalBoneMatrices;
	uint16_t numBones = 0;

	VulkanComboData vulkanComboData;

    std::vector<MaterialInfo> PBR_Materials;

    VulkanSSBO boneSSBO[2];

};
