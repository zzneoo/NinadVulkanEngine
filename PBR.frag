#version 460

layout(location = 0) out vec4 fragColor;

layout(location = 0) in vec2 outTexCoord;
layout(location = 1) in vec3 outNormal;
layout(location = 2) in vec3 outColor;
layout(location = 3) in vec3 outTangent;

layout(set = 1, binding = 0) uniform sampler2D tSampler_albedo;
layout(set = 1, binding = 1) uniform sampler2D tSampler_normal;
layout(set = 1, binding = 2) uniform sampler2D tSampler_orx;

vec3 decodeNormalFromBC5_UNORM(vec2 enc) {
    // enc = texture(sampler, uv).rg  -> in [0,1]
    vec2 n = enc * 2.0 - 1.0;        // map -> [-1,1]
    float z2 = 1.0 - dot(n, n);
    float z = (z2 > 0.0) ? sqrt(z2) : 0.0;
    return normalize(vec3(n.x, n.y, z));
}

vec3 decodeBC5Normal_fast_robust(vec2 enc)
{
    vec2 n = enc * 2.0 - 1.0;   // map UNORM -> [-1,1]
    float d = dot(n, n);

    // small tolerance to avoid tiny fp hiccups from compression:
    if (d <= 1.000001) {
        float z = sqrt(max(0.0, 1.0 - d));     // usually valid; cheap for correct data
        return vec3(n, z);                     // already unit-length (within fp error)
    } else {
        // compression pushed into >1: renormalize xy, z = 0
        float invLen = inversesqrt(d);
        return vec3(n * invLen, 0.0);
    }
}

layout(push_constant) uniform PushConstants 
{
    mat4 model;
	vec3 v3Color;
	float fFactor;
} pc;

vec3 Tonemap_ACES(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main(void)
{
	vec3 lightDir = normalize(vec3(0.0, 1.0, 0.0));

    vec3 albedo = texture(tSampler_albedo, outTexCoord).rgb;

	vec3 normal = normalize(outNormal);
    vec3 tangent = normalize(outTangent);
    vec3 bitangent = normalize(cross(normal, tangent));

    //reorthogonalize tangent
    //tangent = normalize(tangent - dot(tangent, normal) * normal);

    mat3 TBN = mat3(tangent, bitangent, normal);

    vec3  normal_map = decodeNormalFromBC5_UNORM(texture(tSampler_normal,outTexCoord).rg);
    vec3 worldNormal = normalize(TBN * normal_map);

	float nDotL = max(dot(worldNormal, lightDir) * mix(1.0,10.0,pc.fFactor), 0.02);

    vec3 finalColor = albedo * nDotL;

	fragColor = vec4(finalColor,1.0);

}
