#version 460

#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require

#include "common.glsl"

#define saturate(x) clamp(x, 0.0, 1.0)

const float PI = 3.14159265359;

// Per-vertex outputs
layout(location = 0) flat in uint inMaterialID;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec4 inTangent;
layout(location = 5) in vec3 inWorldPos;
layout(location = 6) noperspective in vec2 inVelocity;


layout(set = 1, binding = 0) uniform sampler2D textures[];

//layout(location = 0) out vec4 FragColor;

// G-buffer outputs
layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outORM;
layout(location = 3) out vec2 outVelocity;



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



void main()
{
     //Per meshlet color
    //vec3 meshletColor = pow(inColor.xyz,vec3(2.2));

    vec3 albedo = texture(textures[nonuniformEXT(inMaterialID)], inUV).rgb;
    vec3 normal_map = decodeBC5Normal_fast_robust(texture(textures[nonuniformEXT(inMaterialID + 1)], inUV).rg);
    vec3 orm = texture(textures[nonuniformEXT(inMaterialID + 2)], inUV).rgb;


    vec3 normal = normalize(inNormal);

    vec3 tangent = normalize(inTangent.xyz);
    tangent = normalize(tangent - normal * dot(normal, tangent));

    vec3 bitangent = cross(normal, tangent) * inTangent.w;

    mat3 TBN = mat3(tangent, bitangent, normal);
    //-------------------------------------------------------------------
    vec3 N = normalize(TBN * normal_map);

    outAlbedo = vec4(albedo,1.0);
    outNormal = vec4(N * 0.5 + 0.5, 1.0);
    outORM = vec4(orm,1.0);
    outVelocity = inVelocity;
}

