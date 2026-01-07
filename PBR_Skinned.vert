#version 460

layout(location = 0) in vec3  vPosition;
layout(location = 1) in vec2  vTexCoord;
layout(location = 2) in vec3  vNormal;
layout(location = 3) in vec3  vTangent;
layout(location = 4) in ivec4 vBoneIDs;
layout(location = 5) in vec4  vBoneWeights;

layout(std140, set = 0, binding = 0) uniform FrameData
{
    mat4 view;
    mat4 projection;

    float fTime;
    uint  frameID;
    vec2  _pad0;

    vec3  cameraPos;
    float _pad;
} global;

layout(set = 0, binding = 1, std430) readonly buffer BoneData
{
    mat4 bones[];
} boneData;

layout(push_constant) uniform PushConstants
{
    mat4 model;
    vec3 v3Color;
    float fFactor;
} pc;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outColor;
layout(location = 3) out vec3 outTangent;
layout(location = 4) out vec3 vWorldPos;

void main()
{
    outTexCoord = vTexCoord;

    // -------------------------------
    // Skinning
    // -------------------------------
    mat4 skinMatrix =
          vBoneWeights.x * boneData.bones[vBoneIDs.x]
        + vBoneWeights.y * boneData.bones[vBoneIDs.y]
        + vBoneWeights.z * boneData.bones[vBoneIDs.z]
        + vBoneWeights.w * boneData.bones[vBoneIDs.w];

    vec4 skinnedPos     = skinMatrix * vec4(vPosition, 1.0);
    vec3 skinnedNormal  = mat3(skinMatrix) * vNormal;
    vec3 skinnedTangent = mat3(skinMatrix) * vTangent;

    // -------------------------------
    // Model transform
    // -------------------------------
    vec4 worldPos = pc.model * skinnedPos;
    vWorldPos = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(pc.model)));

    outNormal  = normalize(normalMatrix * skinnedNormal);
    outTangent = normalize(normalMatrix * skinnedTangent);

    outColor = vec3(outNormal);

    gl_Position = global.projection * global.view * worldPos;
}
