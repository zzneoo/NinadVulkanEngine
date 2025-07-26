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
	dir.y = max(dir.y,0.0);//new line
    N.xy /= dot( vec3(1.0), abs(dir) );
	return vec2( N.x + N.y, N.x - N.y ) * 0.5 +0.5;
}

vec2 getAtlasUV(ivec2 tileIdx,
                vec2 texcoord,
                vec2 tileSize,
                float padding)
{
    // 1) Lower‑left corner of this tile
    vec2 tileOffset = vec2(tileIdx) * tileSize;

    // 2) Map [0…1] texcoord into [0…tileSize]
    vec2 localUV = texcoord * tileSize;

    // 3) Shift into this tile
    vec2 uv = tileOffset + localUV;

    // 4) Clamp to avoid bleeding
    vec2 minUV = tileOffset + vec2(padding);
    vec2 maxUV = tileOffset + tileSize - vec2(padding);
    return clamp(uv, minUV, maxUV);
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

// helper to wrap around
ivec2 wrap(ivec2 idx ,ivec2 atlasDims) 
{
    return ivec2((idx.x % atlasDims.x + atlasDims.x) % atlasDims.x,
                 (idx.y % atlasDims.y + atlasDims.y) % atlasDims.y);
}

ivec2 ClampV2(ivec2 tileIdx,ivec2 atlasDims)
{
    // wrap X (yaw)
    //int u = (tileIdx.x % atlasDims.x + atlasDims.x) % atlasDims.x;
    // clamp Y (elevation)
	
	int u = clamp(tileIdx.x, 0, atlasDims.x - 1);
    int v = clamp(tileIdx.y, 0, atlasDims.y - 1);
    return ivec2(u, v);
}

void main(void)
{

    const int viewsU = 12;
    const int viewsV = 12;

    vec2 atlasDims = vec2(float(viewsU), float(viewsV));
    ivec2 iAtlasDims = ivec2(viewsU, viewsV);
	
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
    //baseTile = clamp(baseTile, ivec2(0), ivec2(viewsU - 2, viewsV - 2));
	//baseTile.x = clamp(
	
	//baseTile.x = (baseTile.x % 12 + 12) % 12;
	//baseTile.x = clamp(baseTile.x, 0, 10);
	//baseTile.y = clamp(baseTile.y, 0, 10);
	
	float eq_a = (1.0-max(frac.x,frac.y));
	float eq_c = (1.0-max(1.0-frac.x,1.0-frac.y));
	float eq_b = (abs(frac.y -frac.x));
	float eq_d = (1.0-step(frac.x,frac.y));
	
	// standard bilinear weights
    float w00 = (1.0 - frac.x) * (1.0 - frac.y);
    float w10 = frac.x         * (1.0 - frac.y);
    float w01 = (1.0 - frac.x) * frac.y;
    float w11 = frac.x         * frac.y;

	ivec2 tileId00 = baseTile;
	ivec2 tileId10 = baseTile + ivec2(1, 0);
	ivec2 tileId01 = baseTile + ivec2(0, 1);
	ivec2 tileId11 = baseTile + ivec2(1, 1);
	
	ivec2 choiceTile = (eq_d < 0.5) ? tileId01 : tileId10;
	
	
    // Gather the 4 closest tiles
	float padding = 1.0 / textureSize(tSampler, 0).x;
    vec4 color00 = texture(tSampler, getAtlasUV(ClampV2(tileId00,iAtlasDims), outTexCoord, tileSize,padding));
    vec4 color1001 = texture(tSampler, getAtlasUV(ClampV2(choiceTile,iAtlasDims), outTexCoord, tileSize,padding));
    vec4 color11 = texture(tSampler, getAtlasUV(ClampV2(tileId11,iAtlasDims), outTexCoord, tileSize,padding));
	
	// Gather the 4 closest tiles

    //vec4 color00 = texture(tSampler, getAtlasUV(baseTile, outTexCoord, tileSize, padding));
    //vec4 color10 = texture(tSampler, getAtlasUV(baseTile + ivec2(1, 0), outTexCoord, tileSize, padding));
    //vec4 color01 = texture(tSampler, getAtlasUV(baseTile + ivec2(0, 1), outTexCoord, tileSize, padding));
    //vec4 color11 = texture(tSampler, getAtlasUV(baseTile + ivec2(1, 1),outTexCoord, tileSize, padding));

    // Bilinear blend (you can reduce to 3 samples if desired using weights heuristically)
    //float fractX = smoothstep(0.0,1.0,frac.x);
    //float fractY = smoothstep(0.0,1.0,frac.y);

	float fractX = frac.x;
    float fractY = frac.y;
	
    //vec4 colorX0 = mix(color00, color10, fractX);
    //vec4 colorX1 = mix(color01, color11, fractX);
    //vec4 blendedColor = mix(colorX0, colorX1, fractY);

    //FragColor = blendedColor;
	
	FragColor = color00 * eq_a + color1001 * eq_b + color11 * eq_c;
	
	    // blend
    //FragColor = color00 * w00 +
    //            color10 * w10 +
    //            color01 * w01 +
    //            color11 * w11;

    FragColor.rgb *= 6.0;

	//FragColor.rgb = eq_d;

    if (FragColor.a < 0.45)
        discard;
}
