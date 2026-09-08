#version 460

#extension GL_GOOGLE_include_directive : require

#include "common.glsl"

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec3 vColor;

layout(push_constant) uniform PushConstants 
{
    mat4 model;
	vec3 v3Color;
	float fFactor;
} pc;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;

void main(void)
{
	outColor = vColor;
    outNormal = mat3(pc.model) * vNormal;

	gl_Position = global.proj * global.view * pc.model * vec4(vPosition.xyz,1.0);
}
