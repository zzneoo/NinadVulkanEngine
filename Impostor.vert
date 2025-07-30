#version 460 core

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;


layout(std140, set = 0, binding = 0) uniform FrameData 
{
    mat4 view;        // offset 0   → 64 bytes
    mat4 projection;  // offset 64  → 64 bytes
    float fTime;      // offset 128
    uint  frameID;    // offset 132
    vec3 cameraPos;   // offset 144 → needs to be aligned to 16 bytes
    float _pad;       // offset 156 → pad to 160 (multiple of 16)
}global;

layout(push_constant) uniform PushConstants {
    mat4 model;
	vec3 v3Color;
	float fFactor;
} pc;


layout(location = 0) flat out vec3 v3Weights;

layout(location = 1) out vec2 v2UV00;
layout(location = 2) out vec2 v2UV1001;
layout(location = 3) out vec2 v2UV11;

vec2 semiOct(vec3 dir)
{
    vec2 N = vec2(-dir.x,dir.z);
	dir.y = max(dir.y,0.001);//new line
    N.xy /= dot( vec3(1.0), abs(dir) );
	return vec2( N.x + N.y, N.x - N.y ) * 0.5 +0.5;
}

	//N.xy /= dot( 1, abs(N) );
	//return float2( N.x + N.y, N.x - N.y );

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

ivec2 ClampV2(ivec2 tileIdx,ivec2 atlasDims)
{
    // wrap X (yaw)
    //int u = (tileIdx.x % atlasDims.x + atlasDims.x) % atlasDims.x;
    // clamp Y (elevation)
	
	int u = clamp(tileIdx.x, 0, atlasDims.x - 1);
    int v = clamp(tileIdx.y, 0, atlasDims.y - 1);
    return ivec2(u, v);
}


vec3 semiOctDecode(vec2 uv)
{
    // 1. Remap from [0,1] to [-1,1]
    vec2 Oct = uv;

	Oct = vec2( Oct.x + Oct.y, Oct.x - Oct.y ) * 0.5;
	vec3 N = vec3( Oct, 1.0 - dot( vec2(1.0), abs(Oct) ) );
	return normalize(N);
}

// assuming Y is the up axis.
vec3 octDecodeYUp(vec2 uv)
{
    // unwrap the octahedral projection
    vec2 oct = vec2(uv.x + uv.y, uv.x - uv.y) * 0.5;

    // this is now the Y component (up)
    float z = 1.0 - dot(abs(oct), vec2(1.0));

    return normalize(vec3(-oct.x, z, oct.y));
}


void main(void)
{
	vec4 worldPosition = pc.model * vec4(0.0,0.0,0.0,1.0);
	vec3 viewVector = normalize(global.cameraPos - worldPosition.xyz);
	
	//---------------------------Weights-------------------------------
	 const int viewsU = 12;
    const int viewsV = 12;

    vec2 atlasDims = vec2(float(viewsU), float(viewsV));
    ivec2 iAtlasDims = ivec2(viewsU, viewsV);
	
    vec2 tileSize = 1.0 / atlasDims;
	
    // Map view direction to [0,1]² using semi-octahedral projection
    vec2 directionUV = clamp(semiOct(viewVector),0.0,1.0);

    // Floating-point tile index
    vec2 tileCoordF = directionUV * atlasDims;

    // Integer base tile index (bottom-left corner)
    ivec2 baseTile = ivec2(floor(tileCoordF));

    // Interpolation weights
    vec2 frac = fract(tileCoordF);

	//float eq_a = (1.0-max(frac.x,frac.y));
	//float eq_c = (1.0-max(1.0-frac.x,1.0-frac.y));
	//float eq_b = (abs(frac.y -frac.x)); 
	//float eq_d = (1.0-step(frac.x,frac.y));
	
	vec2 inv = 1.0 - frac;
	// eq_a = 1.0 - max(frac.x, frac.y)
	float eq_a = min(inv.x, inv.y);
	// eq_c = 1.0 - max(inv.x, inv.y)  →  min(frac.x, frac.y)
	float eq_c = min(frac.x, frac.y);
	// eq_b = abs(frac.y - frac.x)
	float eq_b = abs(frac.y - frac.x);
	// eq_d = 1.0 - step(frac.x, frac.y)  →  strictly (frac.y < frac.x)
	float eq_d = float(frac.y < frac.x);

	
	v3Weights = vec3(eq_a,eq_b,eq_c);
	
	
	//----------------------tileCoords--------------------------
		ivec2 tileId00 = baseTile;
	ivec2 tileId10 = baseTile + ivec2(1, 0);
	ivec2 tileId01 = baseTile + ivec2(0, 1);
	ivec2 tileId11 = baseTile + ivec2(1, 1);
	
	ivec2 choiceTile = (eq_d < 0.5) ? tileId01 : tileId10; // top/right
	
	
    // Gather the 4 closest tiles
	//float padding = 1.0 / textureSize(tSampler, 0).x;
	float padding = 1.0 / 2048.0;

	v2UV00 =  getAtlasUV(ClampV2(tileId00,iAtlasDims), vTexCoord, tileSize,padding);
	v2UV1001 = getAtlasUV(ClampV2(choiceTile,iAtlasDims), vTexCoord, tileSize,padding);
	v2UV11 = getAtlasUV(ClampV2(tileId11,iAtlasDims), vTexCoord, tileSize,padding);
	
	//--------------------------------position-----------------------------
    // 4) Shift to *cell center* (0.5 texel), remap to signed domain and decode:
    vec2 centerUV = (vec2(baseTile) + 0.5) / (atlasDims);  // → [0…1] center of tile
    vec2 signedUV = centerUV * 2.0 - 1.0;                // → [−1…1]
    vec3 lockedView = octDecodeYUp(signedUV);           // your “baked” view dir
	
	// now rebuild right/up from lockedView:
	vec3 worldUp = vec3(0.0,1.0,0.0);
	vec3 right = normalize(cross(worldUp, lockedView));
	vec3 up    = normalize(cross(lockedView, right));
	 right = vec3(global.view[0][0],global.view[1][0],global.view[2][0]); // use camera right vector
	 up = vec3(global.view[0][1],global.view[1][1],global.view[2][1]); // use camera up vector
	
	float halfWidth  = 1.0;  // adjust to your sprite’s half‑size
    float halfHeight = 1.0;
	
	vec3 worldCenter = worldPosition.xyz;
	
    vec3 worldPos = worldCenter
                  + right * (vPosition.x * halfWidth)
                  + up    * (vPosition.y * halfHeight);
	
	//mat4 m4ModelView= global.view * pc.model;
	
	// zero out camera rotation
   // m4ModelView[0].xyz = vec3(1,0,0);
    //m4ModelView[1].xyz = vec3(0,1,0);
    //m4ModelView[2].xyz = vec3(0,0,1);

	gl_Position = global.projection * global.view * vec4(worldPos,1.0);
	//gl_Position = global.projection * m4ModelView * vec4(vPosition,1.0);
	
}
