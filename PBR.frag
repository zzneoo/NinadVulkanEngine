#version 460

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec2 outTexCoord;
layout(location = 1) in vec3 outNormal;
layout(location = 2) in vec3 outColor;

layout(set = 1, binding = 0) uniform sampler2D tSampler_albedo;

layout(push_constant) uniform PushConstants 
{
    mat4 model;
	vec3 v3Color;
	float fFactor;
} pc;

void main(void)
{
	vec3 lightDir = normalize(vec3(0.0, 1.0, 0.0));
	vec3 normal = normalize(outNormal);
	float nDotL = max(dot(normal, lightDir), 0.02);

	fragColor = vec4(texture(tSampler_albedo,outTexCoord).xyz,1.0);
}
