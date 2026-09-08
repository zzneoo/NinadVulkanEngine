#version 460

layout(location = 0) in vec4 vPosition;

layout(binding = 0) uniform Camera 
{
	mat4 view;
	mat4 projection;
} cam;

layout(push_constant) uniform PushConstants {
    mat4 model;
	vec3 v3Color;
	float fFactor;
} pc;

void main(void)
{

	gl_Position = cam.projection * cam.view * pc.model * vec4(vPosition.xyz,1.0);
}

