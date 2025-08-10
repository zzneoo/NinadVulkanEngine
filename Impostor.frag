#version 460

layout(location = 0) out vec4 FragColor;

layout(location = 0) flat in vec3 v3Weights;

layout(location = 1) in vec2 v2UV00;
layout(location = 2) in vec2 v2UV1001;
layout(location = 3) in vec2 v2UV11;

layout(location = 4) in vec2 out_Impostor_MS_TiledUV;

layout(location = 5) in vec3 v3Test;

layout(push_constant) uniform PushConstants {
    mat4 model;
	vec3 v3Color;
	float fFactor;
} pc;


// Descriptor set 1, binding 0 → your texture sampler
layout(set = 1, binding = 0) uniform sampler2D tSampler_albedo;
layout(set = 1, binding = 1) uniform sampler2D tSampler_normal;


void main(void)
{

    	// Interpolation weights
    vec2 frac = fract(out_Impostor_MS_TiledUV);
    const float scaleFactor = 0.08333333; // 1/12.0

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

	//3 Frames
    vec2 uv00 = clamp(v2UV00, 0.0, 1.0) * scaleFactor;// 1.0//12.0
    vec2 floored00 = floor(out_Impostor_MS_TiledUV)* scaleFactor;
    uv00 = (uv00 + floored00);

    vec2 uv1001 = clamp(v2UV1001, 0.0, 1.0) * scaleFactor;
    vec2 choiceTile = mix(vec2(0.0,1.0), vec2(1.0,0.0), eq_d);
    vec2 floored1001 = floor(out_Impostor_MS_TiledUV + choiceTile)* scaleFactor;
    uv1001 = (uv1001 + floored1001);

    vec2 uv11 = clamp(v2UV11, 0.0, 1.0)* scaleFactor;
    vec2 floored11 = floor(out_Impostor_MS_TiledUV + vec2(1.0))* scaleFactor;
    uv11 = (uv11 + floored11);


    vec3 v3Weights = vec3(eq_a, eq_b, eq_c);

	vec4 albedo00 = texture(tSampler_albedo, uv00);
    vec4 albedo1001 = texture(tSampler_albedo, uv1001);
    vec4 albedo11 = texture(tSampler_albedo, uv11);
    vec4 blendedAlbedo = albedo00 * v3Weights.x + albedo1001 * v3Weights.y + albedo11 * v3Weights.z;
    if (blendedAlbedo.a < 0.486)
    discard;

    vec4 normal00 = texture(tSampler_normal, uv00);
    vec4 normal1001 = texture(tSampler_normal, uv1001);
    vec4 normal11 = texture(tSampler_normal, uv11);
    vec4 blendedNormal = normal00 * v3Weights.x + normal1001 * v3Weights.y + normal11 * v3Weights.z;
	
    vec3 albedo = blendedAlbedo.rgb; //
    vec3 normal = normalize(blendedNormal.rgb * 2.0 - 1.0);

    vec3 finalColor = albedo * max(dot(normal,vec3(0.0,1.0,0.0)),0.1) ;
    finalColor = pow(finalColor, vec3(0.454545)); // linear to sRGB

	FragColor.rgb = finalColor ;
	FragColor.a = 1.0;

}
