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

layout(location = 4) out vec2 out_Impostor_MS_TiledUV;

vec2 semiOct(vec3 dir)
{
    vec2 N = vec2(-dir.x,dir.z);
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

vec2 VirtualPlaneCoordinates_Impostor(vec3 planeNormalAxis,vec3  planeXAxis,vec3 planeYAxis, vec3 rayDirectionLocal, vec3 rayOriginLocal)
{

	return vec2(0.0, 0.0); // Placeholder for the function implementation
}

void main(void)
{
	vec4 worldSpaceCenterPosition = pc.model * vec4(0.0,0.0,0.0,1.0);
	vec4 worldSpacePosition = pc.model * vec4(vPosition,1.0);
	vec3 worldSpaceCenterVector = mat3(pc.model) * vec3(0.0, 0.0, 0.0);

	vec3 viewVector = normalize(global.cameraPos - worldSpaceCenterPosition.xyz);
	
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

	vec2 scaledTexCoord = vTexCoord.xy * atlasDims * 10.0; // scale to atlas size, adjust as needed

	v2UV00 =  getAtlasUV(ClampV2(tileId00,iAtlasDims), scaledTexCoord, tileSize,padding);
	v2UV1001 = getAtlasUV(ClampV2(choiceTile,iAtlasDims), scaledTexCoord, tileSize,padding);
	v2UV11 = getAtlasUV(ClampV2(tileId11,iAtlasDims), scaledTexCoord, tileSize,padding);
	
	//--------------------------------position-----------------------------

		// now rebuild right/up from lockedView:
	 vec3 worldUp = vec3(0.0,1.0,0.0);
	 //vec3 right = normalize(cross(worldUp, viewVector));
	 //vec3 up    = normalize(cross(viewVector, right));
	 vec3 right = vec3(global.view[0][0],global.view[1][0],global.view[2][0]); // use camera right vector
	 vec3 up = vec3(global.view[0][1],global.view[1][1],global.view[2][1]); // use camera up vector
	 //vec3 forward = vec3(global.view[0][2],global.view[1][2],global.view[2][2]); // use camera forward vector

	 mat3 TBN = mat3(right, up, viewVector);
	 float objectScale = 0.2; //1.0 scale factor for the object, from modelMatrix
	 float centimeterToMeter = 0.01; // conversion factor from centimeters to meters

	 	//ImpostorInfo
	vec3 ImpostorInfo_worldSpaceCenterPosition = worldSpaceCenterPosition.xyz; // world space center position
	vec3 ImpostorInfo_PivotOffsetVector = TBN * (vec3(62.708435, 29.885986,864.380981) * centimeterToMeter *objectScale); // pivot offset in meters
	vec3 ImpostorInfo_position = ImpostorInfo_worldSpaceCenterPosition + ImpostorInfo_PivotOffsetVector; // add pivot offset to the center position
	float ImpostorInfo_scaledSize = 2038.516968 * objectScale * centimeterToMeter; // default size of the mesh in centimeters

	//Sprite_PositionBased
	vec3 Sprite_PositionBased_viewVector = normalize(global.cameraPos - ImpostorInfo_position);
	vec3 Sprite_PositionBased_right = normalize(cross(worldUp, Sprite_PositionBased_viewVector));
	vec3 Sprite_PositionBased_up = -normalize(cross(Sprite_PositionBased_viewVector, Sprite_PositionBased_right));
	vec3 Sprite_PositionBased_WorldPositionOffset = ImpostorInfo_position 
				  + Sprite_PositionBased_right * ((scaledTexCoord.x - 0.5) * ImpostorInfo_scaledSize)
				  + Sprite_PositionBased_up    * ((scaledTexCoord.y - 0.5) * ImpostorInfo_scaledSize)
				  - worldSpacePosition.xyz; // subtract the model position to get the correct world position

	//--------------------------------------------------------------------------
	//Frame View Transform

	vec3 A = Sprite_PositionBased_WorldPositionOffset + worldSpacePosition.xyz;
	vec3 B = normalize(global.cameraPos - A);
	vec3 C = B * ImpostorInfo_scaledSize * 0.5; // scale the direction vector by the size of the impostor
	vec3 FinalWorldPositionOffset = C + Sprite_PositionBased_WorldPositionOffset; // add the scaled direction vector to the object position

	gl_Position = global.projection * global.view * vec4(worldSpacePosition.xyz + FinalWorldPositionOffset,1.0);

	//--------------------------------------------------------------------------
	//For fragment shader
	vec3 Impostor_MS_viewVector = normalize(global.cameraPos - ImpostorInfo_position);

	//worldSpace to tangent space
	Impostor_MS_viewVector = normalize(transpose(TBN) * Impostor_MS_viewVector);
	Impostor_MS_viewVector.y =  max(Impostor_MS_viewVector.y,0.001);//new line
	vec2 Impostor_MS_UV = clamp(semiOct(normalize(Impostor_MS_viewVector)),0.0,1.0);
	out_Impostor_MS_TiledUV = Impostor_MS_UV * (atlasDims - 1.0) ; // scale to atlas size)

	vec2 Impostor_MS_Floored = floor(out_Impostor_MS_TiledUV)/ (atlasDims - 1.0); // scale to atlas size)
	Impostor_MS_Floored = Impostor_MS_Floored * 2.0 - 1.0; // remap to [-1,1] range

	vec3 ImpostorFrameTransform_Setup = octDecodeYUp(Impostor_MS_Floored);

	vec3 ImpostorFrameTransform_Right = normalize(cross(worldUp, ImpostorFrameTransform_Setup));
	vec3 ImpostorFrameTransform_Up = -normalize(cross(ImpostorFrameTransform_Setup, ImpostorFrameTransform_Right));
}
