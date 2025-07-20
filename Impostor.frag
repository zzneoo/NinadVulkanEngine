#version 460 core

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 outTexCoord;
layout(location = 1)flat in vec3 outViewVector;

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

vec2 semiOct(vec3 dir)
{
    vec2 N = dir.xz;
    N.xy /= dot( vec3(1.0), abs(dir) );
	return vec2( N.x + N.y, N.x - N.y ) * 0.5 +0.5;
}

vec2 getAtlasUV(ivec2 tileIdx, vec2 offset, vec2 texcoord, vec2 tileSize) {
    vec2 tileOffset = vec2(tileIdx) * tileSize;
    float padding = 0.001; // prevent bleeding
    vec2 uv = tileOffset + vec2(texcoord.x, texcoord.y) * tileSize;
    return clamp(uv, tileOffset + padding, tileOffset + tileSize - padding);
}

vec2 semiOctVulkanRaw(vec3 dir)
{
    vec2 N = dir.xz;
    N /= dot(abs(dir), vec3(1.0));

    vec2 o = vec2(
        N.x + N.y,
        N.x - N.y
    );

    // flip only the Y‐axis
    o.y = -o.y;

    return o;
}

void main(void)
{

    const int viewsU = 12;
    const int viewsV = 12;

    vec2 atlasDims = vec2(float(viewsU), float(viewsV));
    vec2 tileSize = 1.0 / atlasDims;
	
	vec3 viewVector = normalize(outViewVector);

    // Map view direction to [0,1]² using semi-octahedral projection
    vec2 directionUV = semiOct(viewVector);

    // Floating-point tile index
    vec2 tileCoordF = directionUV * atlasDims;

    // Integer base tile index (bottom-left corner)
    ivec2 baseTile = ivec2(floor(tileCoordF));

    // Interpolation weights
    vec2 frac = fract(tileCoordF);

    // Clamp base tile to avoid out-of-bounds
    baseTile = clamp(baseTile, ivec2(0), ivec2(viewsU - 2, viewsV - 2));

    // Gather the 4 closest tiles
    vec4 color00 = texture(tSampler, getAtlasUV(baseTile, vec2(0.0, 0.0), outTexCoord, tileSize));
    vec4 color10 = texture(tSampler, getAtlasUV(baseTile + ivec2(1, 0), vec2(0.0, 0.0), outTexCoord, tileSize));
    vec4 color01 = texture(tSampler, getAtlasUV(baseTile + ivec2(0, 1), vec2(0.0, 0.0), outTexCoord, tileSize));
    vec4 color11 = texture(tSampler, getAtlasUV(baseTile + ivec2(1, 1), vec2(0.0, 0.0), outTexCoord, tileSize));

    // Bilinear blend (you can reduce to 3 samples if desired using weights heuristically)
    float fractX = smoothstep(0.0,1.0,frac.x);
    float fractY = smoothstep(0.0,1.0,frac.y);

    vec4 colorX0 = mix(color00, color10, fractX);
    vec4 colorX1 = mix(color01, color11, fractX);
    vec4 blendedColor = mix(colorX0, colorX1, fractY);

    FragColor = blendedColor;

    //FragColor.rgb *= 2.0;

    if (FragColor.a < 0.45)
        discard;
}
