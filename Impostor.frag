#version 460 core

layout(location = 0) out vec4 FragColor;

layout(location = 0) flat in vec3 v3Weights;

layout(location = 1) in vec2 v2UV00;
layout(location = 2) in vec2 v2UV1001;
layout(location = 3) in vec2 v2UV11;

layout(location = 4) in vec2 out_Impostor_MS_TiledUV;

layout(push_constant) uniform PushConstants {
    mat4 model;
	vec3 v3Color;
	float fFactor;
} pc;


// Descriptor set 1, binding 0 → your texture sampler
layout(set = 1, binding = 0) uniform sampler2D tSampler;

// Maps a hemisphere (upper unit hemisphere) to 2D UV space
vec2 semiOctahedralMap(vec3 dir) 
{
    dir = normalize(dir);
    float sum = abs(dir.x) + abs(dir.y) + abs(dir.z);
    vec2 p = dir.xz / sum; // using X and Z as planar axes, Y as up

    // Reflect direction if below the horizon (hemisphere fold)
    if (dir.y < 0.0) {
        p = (1.0 - abs(p.yx)) * sign(p);
    }

    // Map from [-1,1] to [0,1]
    return p * 0.5 + 0.5;
}


void main(void)
{

	//3 Frames
	vec4 color00 = texture(tSampler, v2UV00);
    vec4 color1001 = texture(tSampler, v2UV1001);
    vec4 color11 = texture(tSampler, v2UV11);
	
	FragColor = color00 * v3Weights.x + color1001 * v3Weights.y + color11 * v3Weights.z;

    FragColor.rgb *= 6.0;
    //FragColor.rgb = vec3(1.0);

    //if (FragColor.a < 0.5)
      //  discard;
}
