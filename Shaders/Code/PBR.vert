#version 460

#extension GL_GOOGLE_include_directive : require

#include "common.glsl"

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec4 vTangent;


layout(push_constant) uniform PushConstants 
{
    mat4 model;
	vec3 v3Color;
	float fFactor;
} pc;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outColor;
layout(location = 3) out vec4 outTangent;
layout(location = 4) out vec3 vWorldPos;

void main(void)
{
	outTexCoord = vTexCoord;
    outTexCoord.y = 1.0 - outTexCoord.y;

    mat3 normalMatrix = transpose(inverse(mat3(pc.model)));

    outNormal = normalize(normalMatrix * vNormal);
    outTangent.xyz = normalize(normalMatrix * vTangent.xyz);
    outTangent.w = vTangent.w;

    outColor = vec3(outNormal);

    vWorldPos =  (pc.model * vec4(vPosition,1.0)).xyz;

	gl_Position = global.proj * global.view * vec4(vWorldPos,1.0);
}
