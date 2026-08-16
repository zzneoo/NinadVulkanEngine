#version 460

#extension GL_EXT_nonuniform_qualifier : require

#define saturate(x) clamp(x, 0.0, 1.0)

const float PI = 3.14159265359;

// Per-vertex outputs
layout(location = 0) flat in uint inMaterialID;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec4 inTangent;
layout(location = 5) in vec3 inWorldPos;


layout(set = 1, binding = 0) uniform sampler2D textures[];
layout(location = 0) out vec4 FragColor;

layout(std140, set = 0, binding = 0) uniform FrameData 
{
    mat4 view;        // offset 0    (64 bytes)
    mat4 projection;  // offset 64   (64 bytes)
    float fTime;      // offset 128  (4 bytes)
    uint  frameID;    // offset 132  (4 bytes)
    vec2  _pad0;      // offset 136  (8 bytes) -> explicitly aligns cameraPos to 144
    vec3  cameraPos;  // offset 144  (12 bytes)
    float _pad1;      // offset 156  (4 bytes) -> rounds struct total to 160
} global;

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

//PBR
// Microfacet helpers (GGX/Trowbridge-Reitz + Smith + Schlick Fresnel)
float DistributionGGX(float NdotH, float roughness) 
{
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / max(PI * denom * denom, 1e-5);
}

float GeometrySchlickGGX(float NdotV, float k) 
{
    return NdotV / (NdotV * (1.0 - k) + k);
}


float GeometrySmith(float NdotV, float NdotL, float roughness) 
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // direct lighting optimization
    return GeometrySchlickGGX(NdotV, k) * GeometrySchlickGGX(NdotL, k);
}


vec3 FresnelSchlick(float cosTheta, vec3 F0) 
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

//ToneMap
vec3 Tonemap_ACES(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    //FragColor = vec4(pow(inColor.xyz,vec3(2.2)),1.0); //Per meshlet color

    vec3 albedo = texture(textures[nonuniformEXT(inMaterialID)], inUV).rgb;
    vec3 normal_map = decodeBC5Normal_fast_robust(texture(textures[nonuniformEXT(inMaterialID + 1)], inUV).rg);
    vec3 orx = texture(textures[nonuniformEXT(inMaterialID + 2)], inUV).rgb;

    float ao = orx.r;
    float roughness = clamp(orx.g, 0.045, 1.0);
    float metallic = saturate(orx.b);

    vec3 normal = normalize(inNormal);

    vec3 tangent = normalize(inTangent.xyz);
    tangent = normalize(tangent - normal * dot(normal, tangent));

    vec3 bitangent = cross(normal, tangent) * inTangent.w;

    mat3 TBN = mat3(tangent, bitangent, normal);
    //-------------------------------------------------------------------


    vec3 N = normalize(TBN * normal_map);
    //vec3 N = normalize(normal);
    vec3 V = normalize(global.cameraPos - inWorldPos);
    vec3 L = normalize(vec3(0.0, 1.0, 0.4));
    vec3 H = normalize(V + L);

    vec3 radiance = vec3(1.0) * mix(1.0,10.0,0.5);//pc.v3Color

    // Base reflectance
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0001);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    float D = DistributionGGX(NdotH, roughness);
    float G = GeometrySmith(NdotV, NdotL, roughness);
    vec3 F = FresnelSchlick(HdotV, F0);

    vec3 numerator = D * G * F;
    float denom = max(4.0 * NdotV * NdotL, 1e-4);
    vec3 specular = numerator / denom;


    // Energy-conserving diffuse term (Lambert)
    vec3 kS = F; // specular amount
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;

    vec3 ambient = albedo * ao * 0.04;
    vec3 Lo = (diffuse + specular) * radiance * NdotL;

    vec3 finalColor = Lo + ambient;
    finalColor = Tonemap_ACES(finalColor);

    FragColor = vec4(vec3(finalColor),1.0);
}

