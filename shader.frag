#version 460 core

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 outColor;

void main(void)
{
	fragColor = vec4(outColor,1.0);
}
