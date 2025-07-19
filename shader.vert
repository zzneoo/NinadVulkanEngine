#version 460 core

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vColor;

layout(binding = 0) uniform Camera 
{
	mat4 view;
	mat4 projection;
} cam;

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

	gl_Position = cam.projection * cam.view * pc.model * vec4(vPosition.xyz,1.0);
}
