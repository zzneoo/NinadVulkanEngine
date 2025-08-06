#version 460

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec3 outColor;

layout(push_constant) uniform PushConstants 
{
    mat4 model;
	vec3 v3Color;
	float fFactor;
} pc;

void main(void)
{
	fragColor = vec4(mix(vec3(1.0),outColor,pc.fFactor) ,1.0);
}
