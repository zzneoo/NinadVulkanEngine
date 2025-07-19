#version 460 core

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec2 outTexCoord;

layout(push_constant) uniform PushConstants {
    mat4 model;
	vec3 v3Color;
	float fFactor;
} pc;


// Descriptor set 0, binding 1 → your texture sampler
layout(set = 0, binding = 1) uniform sampler2D tSampler;

void main(void)
{
	fragColor = vec4(outTexCoord.xy, 0.0 ,1.0);
	fragColor = texture(tSampler,outTexCoord.xy);
}
