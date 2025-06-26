#version 460 core

layout(location = 0) in vec4 vPosition;

layout(binding = 0) uniform Transform 
{
	mat4 model;
	mat4 view;
	mat4 projection;
} transform;

void main(void)
{
	gl_Position = transform.projection * vPosition;
}
