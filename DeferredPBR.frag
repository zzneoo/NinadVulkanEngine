#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require

#include "common.glsl"

layout(location = 0) in vec2 v2TexCoord;
layout(location = 0) out vec4 fragColor;

layout(set = 1, binding = 0) uniform sampler2D gAlbedo;
layout(set = 1, binding = 1) uniform sampler2D gNormal;
layout(set = 1, binding = 2) uniform sampler2D gORM;
layout(set = 1, binding = 3) uniform sampler2D gDepth;

const float PI = 3.14159265359;

float DistributionGGX(float nDotH, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float denominator = nDotH * nDotH * (a2 - 1.0) + 1.0;
	return a2 / max(PI * denominator * denominator, 1e-5);
}

float GeometrySchlickGGX(float nDotV, float k)
{
	return nDotV / (nDotV * (1.0 - k) + k);
}

float GeometrySmith(float nDotV, float nDotL, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;
	return GeometrySchlickGGX(nDotV, k) * GeometrySchlickGGX(nDotL, k);
}

vec3 FresnelSchlick(float cosTheta, vec3 f0)
{
	return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

vec3 TonemapACES(vec3 x)
{
	const float a = 2.51;
	const float b = 0.03;
	const float c = 2.43;
	const float d = 0.59;
	const float e = 0.14;
	return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
	vec4 albedoSample = texture(gAlbedo, v2TexCoord);
	vec3 normal = normalize(texture(gNormal, v2TexCoord).xyz * 2.0 - 1.0);
	vec3 orm = texture(gORM, v2TexCoord).rgb;
	float depth = texture(gDepth, v2TexCoord).r;

	if (depth >= 1.0)
	{
		fragColor = vec4(0.01, 0.01, 0.02, 1.0);
		return;
	}

	mat4 inverseViewProjection = inverse(global.proj * global.view);
	vec4 worldPosition = inverseViewProjection * vec4(v2TexCoord * 2.0 - 1.0, depth, 1.0);
	worldPosition /= worldPosition.w;

	vec3 albedo = albedoSample.rgb;
	float ao = orm.r;
	float roughness = clamp(orm.g, 0.045, 1.0);
	float metallic = clamp(orm.b, 0.0, 1.0);

	vec3 viewDirection = normalize(global.cameraPos - worldPosition.xyz);
	vec3 lightDirection = normalize(global.sunDir);
	vec3 halfway = normalize(viewDirection + lightDirection);

	float nDotL = max(dot(normal, lightDirection), 0.0);
	float nDotV = max(dot(normal, viewDirection), 0.0001);
	float nDotH = max(dot(normal, halfway), 0.0);
	float hDotV = max(dot(halfway, viewDirection), 0.0);

	vec3 f0 = mix(vec3(0.04), albedo, metallic);
	float distribution = DistributionGGX(nDotH, roughness);
	float geometry = GeometrySmith(nDotV, nDotL, roughness);
	vec3 fresnel = FresnelSchlick(hDotV, f0);

	vec3 specular = distribution * geometry * fresnel /
		max(4.0 * nDotV * nDotL, 1e-4);
	vec3 diffuse = (vec3(1.0) - fresnel) * (1.0 - metallic) * albedo / PI;
	vec3 radiance = vec3(1.0) * mix(1.0, 10.0, 0.5);
	vec3 lighting = (diffuse + specular) * radiance * nDotL;
	vec3 ambient = albedo * ao * 0.04;

	fragColor = vec4(TonemapACES(lighting + ambient), 1.0);
}
