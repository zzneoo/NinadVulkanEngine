#version 460

#extension GL_GOOGLE_include_directive : require

#include "common.glsl"

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vColor;

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
