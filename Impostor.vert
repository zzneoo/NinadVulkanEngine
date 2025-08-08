#version 460

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
layout(location = 5) out vec3 v3Test;


vec2 semiOct(vec3 dir)
{
    vec2 N = vec2(dir.x,dir.y);
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

ivec2 ClampV2(ivec2 tileIdx,ivec2 atlasDims)
{
    // wrap X (yaw)
    //int u = (tileIdx.x % atlasDims.x + atlasDims.x) % atlasDims.x;
    // clamp Y (elevation)
	
	int u = clamp(tileIdx.x, 0, atlasDims.x - 1);
    int v = clamp(tileIdx.y, 0, atlasDims.y - 1);
    return ivec2(u, v);
}


// assuming Z is the up axis.
vec3 semiOctDecode(vec2 uv)
{
    // unwrap the octahedral projection
    vec2 oct = vec2(uv.x + uv.y, uv.x - uv.y) * 0.5;

    // this is now the Y component (up)
    float z = 1.0 - dot(abs(oct), vec2(1.0));

    return normalize(vec3(oct.x,oct.y,z));
}


vec2 VirtualPlaneCoordinates_Impostor(vec3 planeNormalAxis, vec3 planeRightAxis, vec3 planeUpAxis,vec2 UVscale,vec3 rayOriginLocal,vec3 rayDirectionLocal)
{
	vec2 outUV;
	
	float planeDistance = dot(planeNormalAxis,rayOriginLocal);
		
	float normalDotRayDirection = dot(planeNormalAxis,rayDirectionLocal);
	
	float intersectionTime = -planeDistance/normalDotRayDirection;
	
	vec3 intersectionPosition = rayOriginLocal + rayDirectionLocal * intersectionTime;
	
	float rightAxisDotIntersection = dot(planeRightAxis,intersectionPosition);
	float upAxisDotIntersection = dot(planeUpAxis,intersectionPosition);
	
	vec2 appendVector2 = vec2(rightAxisDotIntersection,upAxisDotIntersection);

	outUV = (intersectionTime>0.0) ?appendVector2 : vec2(0.0,0.0);

	outUV= (outUV/UVscale)+0.5; // scale the UV coordinates
	//outUV += vec2(0.5); // shift to [0,1] range 

	return outUV;
}

mat3 BuildImpostorTBN(vec3 worldCenter, vec3 cameraPos_world, vec3 worldUp, bool lockUpright) {
    // viewVector points from the surface center TO the camera (camera - center)
    vec3 viewVector = cameraPos_world - worldCenter;
    float vlen = length(viewVector);
    if (vlen < 1e-6) {
        // camera basically at the center — pick a sane default forward
        viewVector = vec3(0.0, 0.0, 1.0); // or camera forward if available
    } else {
        viewVector /= vlen;
    }

    // Choose tangent/right via worldUp x viewVector (safe fallback handled below).
    // Using cross(worldUp, viewVector) makes "right" point roughly along +X when camera
    // is looking from +X; this is a common choice for upright billboards.
    vec3 right;
    if (lockUpright) {
        float dp = abs(dot(worldUp, viewVector));
        if (dp > 0.999) {
            // near-parallel: pick an arbitrary axis to form a stable right vector
            vec3 alt = abs(worldUp.x) < 0.99 ? vec3(1.0,0.0,0.0) : vec3(0.0,1.0,0.0);
            right = normalize(cross(alt, viewVector));
        } else {
            right = normalize(cross(worldUp, viewVector));
        }
    } else {
        // full-face camera-facing: allow full rotation (may tilt the quad)
        float dp = abs(dot(worldUp, viewVector));
        if (dp > 0.999) {
            vec3 alt = abs(worldUp.x) < 0.99 ? vec3(1.0,0.0,0.0) : vec3(0.0,1.0,0.0);
            right = normalize(cross(alt, viewVector));
        } else {
            right = normalize(cross(worldUp, viewVector));
        }
    }

    // Gram-Schmidt: make right orthogonal to viewVector and normalize
    right = normalize(right - viewVector * dot(viewVector, right));

    // up (bitangent) consistent with right and viewVector
    vec3 up = normalize(cross(viewVector, right)); // note cross(view, right) to keep handedness

    // Final TBN matrix: columns = (T=right, B=up, N=viewVector)
    return mat3(right, up, viewVector);
}

void main(void)
{
	vec4 worldSpaceCenterPosition = pc.model * vec4(0.0,0.0,0.0,1.0);
	vec4 worldSpacePosition = pc.model * vec4(vPosition,1.0);

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
	vec3 worldUp = vec3(0.0, 0.0, 1.0); // assuming Z is up in world space

	 float objectScale = 0.2; //1.0 scale factor for the object, from modelMatrix
	 float centimeterToMeter = 0.01; // conversion factor from centimeters to meters

	 	//ImpostorInfo
	vec3 ImpostorInfo_worldSpaceCenterPosition = worldSpaceCenterPosition.xyz; // world space center position
	vec3 ImpostorInfo_PivotOffsetVector = (( (vec3(62.708435,29.885986,864.380981)* centimeterToMeter))  *objectScale); // pivot offset in meters
	vec3 ImpostorInfo_position = ImpostorInfo_worldSpaceCenterPosition + ImpostorInfo_PivotOffsetVector; // add pivot offset to the center position
	float ImpostorInfo_scaledSize = 2038.516968 * objectScale * centimeterToMeter; // default size of the mesh in centimeters

	//Sprite_PositionBased
	vec3 Sprite_PositionBased_viewVector = normalize(global.cameraPos - ImpostorInfo_position);
	vec3 Sprite_PositionBased_right = normalize(cross(Sprite_PositionBased_viewVector,worldUp));
	vec3 Sprite_PositionBased_up = normalize(cross(Sprite_PositionBased_viewVector, Sprite_PositionBased_right));
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
	vec3 Impostor_MS_camPos = global.cameraPos - ImpostorInfo_position; // camera position relative to the impostor center
	vec3 Impostor_MS_viewVector = (Impostor_MS_camPos);

	//worldSpace to tangent space
	Impostor_MS_viewVector = normalize( Impostor_MS_viewVector);
	Impostor_MS_viewVector.z =  max(Impostor_MS_viewVector.z,0.001);//new line
	vec2 Impostor_MS_UV = clamp(semiOct(normalize(Impostor_MS_viewVector)),0.0,1.0);
	out_Impostor_MS_TiledUV = Impostor_MS_UV * (atlasDims - 1.0) ; // scale to atlas size)

	vec2 Impostor_MS_Floored = floor(out_Impostor_MS_TiledUV)/ (atlasDims - 1.0); // scale to atlas size)
	Impostor_MS_Floored = Impostor_MS_Floored * 2.0 - 1.0; // remap to [-1,1] range

	vec3 ImpostorFrameTransform_WorldZ = semiOctDecode(Impostor_MS_Floored);
	//vec3 ImpostorFrameTransform_Right = normalize(cross(ImpostorFrameTransform_WorldZ,worldUp));
	vec3 ImpostorFrameTransform_Right = -normalize(vec3(ImpostorFrameTransform_WorldZ.y,-ImpostorFrameTransform_WorldZ.x,0.0));//flip uv.y
	vec3 ImpostorFrameTransform_Up = normalize(cross(ImpostorFrameTransform_WorldZ, ImpostorFrameTransform_Right));

	//--------------------------------

	vec3 absoluteWorldSpacePositionWithoutMaterialOffset = worldSpacePosition.xyz + Sprite_PositionBased_WorldPositionOffset;

	vec3 rayOriginLocal = Impostor_MS_camPos;
	vec3 rayDirLocal = absoluteWorldSpacePositionWithoutMaterialOffset - global.cameraPos;

	vec2 outUV = VirtualPlaneCoordinates_Impostor(ImpostorFrameTransform_WorldZ, ImpostorFrameTransform_Right, ImpostorFrameTransform_Up, vec2(ImpostorInfo_scaledSize) ,rayOriginLocal , rayDirLocal );

	v2UV00 = outUV.xy; // output the UV coordinates for the fragment shader

	v3Test = vec3(ImpostorInfo_PivotOffsetVector);
	//v3Test = vec3(outUV.x,outUV.y,0.0);
}
