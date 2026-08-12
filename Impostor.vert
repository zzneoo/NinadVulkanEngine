#version 460

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoord;


layout(std140, set = 0, binding = 0) uniform FrameData 
{
    mat4 view;
    mat4 projection;

    float fTime;
    uint  frameID;
    vec2 _pad0;

    vec3 cameraPos;
    float _pad;
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

layout(location = 6) out vec2 v2Parallax_First;
layout(location = 7) out vec2 v2Parallax_Second;
layout(location = 8) out vec2 v2Parallax_Third;


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
	
	float rightAxisDotIntersection = -dot(planeRightAxis,intersectionPosition);
	float upAxisDotIntersection = dot(planeUpAxis,intersectionPosition);
	
	vec2 appendVector2 = vec2(rightAxisDotIntersection,upAxisDotIntersection);

	outUV = (intersectionTime>0.0) ?appendVector2 : vec2(0.0,0.0);

	outUV= (outUV/UVscale)+0.5; // scale the UV coordinates
	//outUV += vec2(0.5); // shift to [0,1] range 

	return outUV;
}

vec3 ImpostorFrameTransform_setup(vec2 v2Frame,vec2 XY_Frames)
{
	vec3 worldUp = vec3(0.0, 0.0, 1.0); // assuming Z is up in world space
	vec2 Impostor_MS_Floored = v2Frame/ (XY_Frames - 1.0); // scale to atlas size)
	Impostor_MS_Floored = Impostor_MS_Floored * 2.0 - 1.0; // remap to [-1,1] range
	//vec3 ImpostorFrameTransform_Right = normalize(cross(ImpostorFrameTransform_WorldZ,worldUp));
	//vec3 ImpostorFrameTransform_Right = -normalize(vec3(ImpostorFrameTransform_WorldZ.y,-ImpostorFrameTransform_WorldZ.x,0.0));//flip uv.y
	//vec3 ImpostorFrameTransform_Up = normalize(cross(ImpostorFrameTransform_WorldZ, ImpostorFrameTransform_Right));
	
	return semiOctDecode(Impostor_MS_Floored);
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
    vec2 tileCoordF = directionUV * (atlasDims - 1.0);

    // Integer base tile index (bottom-left corner)
    ivec2 baseTile = ivec2(floor(tileCoordF));

    // Interpolation weights
    //vec2 frac = fract(tileCoordF);

	//float eq_a = (1.0-max(frac.x,frac.y));
	//float eq_c = (1.0-max(1.0-frac.x,1.0-frac.y));
	//float eq_b = (abs(frac.y -frac.x)); 
	//float eq_d = (1.0-step(frac.x,frac.y));
	
	/*
	vec2 inv = 1.0 - frac;
	// eq_a = 1.0 - max(frac.x, frac.y)
	float eq_a = min(inv.x, inv.y);
	// eq_c = 1.0 - max(inv.x, inv.y)  →  min(frac.x, frac.y)
	float eq_c = min(frac.x, frac.y);
	// eq_b = abs(frac.y - frac.x)
	float eq_b = abs(frac.y - frac.x);
	// eq_d = 1.0 - step(frac.x, frac.y)  →  strictly (frac.y < frac.x)
	float eq_d = float(frac.y < frac.x);
	*/
	
	//v3Weights = vec3(eq_a,eq_b,eq_c);
	
	
	//----------------------tileCoords--------------------------
	/*
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
	*/
	//--------------------------------position-----------------------------
	vec3 worldUp = vec3(0.0, 0.0, 1.0); // assuming Z is up in world space
	vec2 scaledTexCoord = vTexCoord.xy * atlasDims * 10.0; // scale to atlas size, adjust as needed

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

	//worldSpace 
	Impostor_MS_viewVector = normalize( Impostor_MS_viewVector);
	Impostor_MS_viewVector.z =  max(Impostor_MS_viewVector.z,0.001);//new line
	vec2 Impostor_MS_UV = clamp(semiOct(normalize(Impostor_MS_viewVector)),0.0,1.0);
	out_Impostor_MS_TiledUV = Impostor_MS_UV * (atlasDims - 1.0) ; // scale to atlas size)

	vec2 flooredUV = floor(out_Impostor_MS_TiledUV);

	// Interpolation weights
    vec2 frac = fract(out_Impostor_MS_TiledUV);

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

	//Frame_First
	vec3 ImpostorFrameTransform_WorldZ_First = -ImpostorFrameTransform_setup(flooredUV, atlasDims);
  //vec3 ImpostorFrameTransform_Right_First = normalize(cross(ImpostorFrameTransform_WorldZ_First,worldUp));
	vec3 ImpostorFrameTransform_Right_First = normalize(vec3(ImpostorFrameTransform_WorldZ_First.y,-ImpostorFrameTransform_WorldZ_First.x,0.0));//flip uv.y
	vec3 ImpostorFrameTransform_Up_First = normalize(cross(ImpostorFrameTransform_WorldZ_First, ImpostorFrameTransform_Right_First));

	//Frame Second
	vec2 choiceTile = (eq_d < 0.49) ? vec2(0.0, 1.0) : vec2(1.0, 0.0); // top/right

	vec3 ImpostorFrameTransform_WorldZ_Second = -ImpostorFrameTransform_setup(flooredUV + choiceTile, atlasDims);//bottom left
  //vec3 ImpostorFrameTransform_Right_Second = normalize(cross(ImpostorFrameTransform_WorldZ_Second,worldUp));
	vec3 ImpostorFrameTransform_Right_Second = normalize(vec3(ImpostorFrameTransform_WorldZ_Second.y,-ImpostorFrameTransform_WorldZ_Second.x,0.0));//flip uv.y
	vec3 ImpostorFrameTransform_Up_Second = normalize(cross(ImpostorFrameTransform_WorldZ_Second, ImpostorFrameTransform_Right_Second));


	//Frame Third
	vec3 ImpostorFrameTransform_WorldZ_Third = -ImpostorFrameTransform_setup(flooredUV + 1.0, atlasDims);//top right
  //vec3 ImpostorFrameTransform_Right_Third = normalize(cross(ImpostorFrameTransform_WorldZ_Third,worldUp));
	vec3 ImpostorFrameTransform_Right_Third = normalize(vec3(ImpostorFrameTransform_WorldZ_Third.y,-ImpostorFrameTransform_WorldZ_Third.x,0.0));//flip uv.y
	vec3 ImpostorFrameTransform_Up_Third = normalize(cross(ImpostorFrameTransform_WorldZ_Third, ImpostorFrameTransform_Right_Third));

	//--------------------------------

	vec3 absoluteWorldSpacePositionWithoutMaterialOffset = worldSpacePosition.xyz + Sprite_PositionBased_WorldPositionOffset;
	vec3 rayOriginLocal = Impostor_MS_camPos;
	vec3 rayDirLocal = absoluteWorldSpacePositionWithoutMaterialOffset - global.cameraPos;

	v2UV00 = VirtualPlaneCoordinates_Impostor(ImpostorFrameTransform_WorldZ_First, ImpostorFrameTransform_Right_First, ImpostorFrameTransform_Up_First, vec2(ImpostorInfo_scaledSize) ,rayOriginLocal , rayDirLocal );
	v2UV1001 = VirtualPlaneCoordinates_Impostor(ImpostorFrameTransform_WorldZ_Second, ImpostorFrameTransform_Right_Second, ImpostorFrameTransform_Up_Second, vec2(ImpostorInfo_scaledSize) ,rayOriginLocal , rayDirLocal );
	v2UV11 = VirtualPlaneCoordinates_Impostor(ImpostorFrameTransform_WorldZ_Third, ImpostorFrameTransform_Right_Third, ImpostorFrameTransform_Up_Third, vec2(ImpostorInfo_scaledSize) ,rayOriginLocal , rayDirLocal );

	v3Test = vec3(ImpostorInfo_PivotOffsetVector);
	//v3Test = vec3(outUV.x,outUV.y,0.0);


	//parallax
	vec3 parllaxDir = normalize(rayDirLocal);
	v2Parallax_First = vec2(dot(parllaxDir,-ImpostorFrameTransform_Right_First), dot(parllaxDir,ImpostorFrameTransform_Up_First));
	v2Parallax_First *= 0.0625 * 0.5;

	v2Parallax_Second = vec2(dot(parllaxDir,-ImpostorFrameTransform_Right_Second), dot(parllaxDir,ImpostorFrameTransform_Up_Second));
	v2Parallax_Second *= 0.083333 *0.5;

	v2Parallax_Third = vec2(dot(parllaxDir,-ImpostorFrameTransform_Right_Third), dot(parllaxDir,ImpostorFrameTransform_Up_Third));
	v2Parallax_Third *= 0.083333 *0.5;

}
