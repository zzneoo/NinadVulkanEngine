#version 460

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vColor;

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

layout(location = 0) out vec3 outColor;

void main(void)
{
	outColor = vColor;

	gl_Position = global.projection * global.view * pc.model * vec4(vPosition.xyz,1.0);
}
