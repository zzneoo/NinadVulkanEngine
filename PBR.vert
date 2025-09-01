#version 460

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec3 vTangent;

layout(std140, set = 0, binding = 0) uniform FrameData 
{
    mat4 view;        // offset 0    64 bytes
    mat4 projection;  // offset 64   64 bytes
    float fTime;      // offset 128
    uint  frameID;    // offset 132
    vec3 cameraPos;   // offset 144  needs to be aligned to 16 bytes
    float _pad;       // offset 156  pad to 160 (multiple of 16)
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
