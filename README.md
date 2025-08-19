# VulkanPerspectiveTriangle

glslangValidator.exe -V -H -o shader.vert.spv "C:\Users\Ninad\source\repos\BlueScreen\shader.vert"

.\texconv.exe -f BC7_UNORM -o .\out\ -m 1 "C:\Users\Ninad\source\repos\BlueScreen\Resources\Impostors\BlackAlder_Field_02\T_Impostor_BaseColor_Field_02_Summer.DDS"

//-----------------------------------------------------------------------------------------------//
vkDescriptorPool -->VkDescriptorSet   
VkDescriptorSetLayout -->VkDescriptorSet  

VkWriteDescriptorSet(VkDescriptorSet)

VkDescriptorSetLayout(VkDescriptorSetLayoutBinding)
VkPipelineLayout(VkDescriptorSetLayout)


//-----------------------------------------------------------------------------------------------//

Next to do...
0) convert coloredQuad to uvQuad(pipeline + uv attribute)
1) group together pipeline and pipelineLayout
2) Billboard(start with uv quad)

