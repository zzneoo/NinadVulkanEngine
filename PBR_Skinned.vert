#version 460

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec3 vTangent;
layout(location = 4) in ivec4 vBoneIDs;
layout(location = 5) in vec4 vBoneWeights;

layout(std140, set = 0, binding = 0) uniform FrameData 
{
    mat4 view;
    mat4 projection;

    float fTime;
    uint  frameID;
    vec2 _pad0;

    vec3 cameraPos;
    float _pad;
}global;

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

void main(void)
{
	outTexCoord = vTexCoord;

    mat3 normalMatrix = transpose(inverse(mat3(pc.model)));

    outNormal = normalize(normalMatrix * vNormal);
    outTangent = normalize(normalMatrix * vTangent);
    outColor = vec3(outNormal);


    vWorldPos =  (pc.model * vec4(vPosition.xyz,1.0)).xyz;

	gl_Position = global.projection * global.view * vec4(vWorldPos,1.0);
}
