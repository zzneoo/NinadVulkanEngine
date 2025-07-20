#version 460 core

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vColor;

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

layout(location = 0) out vec3 outColor;

void main(void)
{
	outColor = vColor;

	gl_Position = global.projection * global.view * pc.model * vec4(vPosition.xyz,1.0);
}
