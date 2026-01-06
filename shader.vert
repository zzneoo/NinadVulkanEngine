#version 460

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vColor;

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

layout(location = 0) out vec3 outColor;

void main(void)
{
	outColor = vColor;

	gl_Position = global.projection * global.view * pc.model * vec4(vPosition.xyz,1.0);
}
