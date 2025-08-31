#version 460

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec3 vTangent;

layout(std140, set = 0, binding = 0) uniform FrameData 
{
    mat4 view;        // 64 bytes
    mat4 projection;  // 64 bytes
    float fTime;      // 4 bytes
    uint frameID;     // 4 bytes
    vec2 _pad;        // 8 bytes (padding, same as float[2] in C++)
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

void main(void)
{
	outTexCoord = vTexCoord;

    mat3 normalMatrix = transpose(inverse(mat3(pc.model)));

    outNormal = normalize(normalMatrix * vNormal);
    outTangent = normalize(normalMatrix * vTangent);
    outColor = vec3(outNormal);

	gl_Position = global.projection * global.view * pc.model * vec4(vPosition.xyz,1.0);
}
