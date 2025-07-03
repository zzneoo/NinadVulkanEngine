#version 460 core

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vColor;

layout(binding = 0) uniform Transform 
{
	mat4 model;
	mat4 view;
	mat4 projection;
} transform;

layout(location = 0) out vec3 outColor;

void main(void)
{
	outColor = vColor;

	gl_Position = transform.projection * transform.model * vec4(vPosition.xyz,1.0);
}
