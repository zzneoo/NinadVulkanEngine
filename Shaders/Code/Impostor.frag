#version 460

layout(location = 0) out vec4 FragColor;

layout(location = 0) flat in vec3 v3Weights;

layout(location = 1) in vec2 v2UV00;
layout(location = 2) in vec2 v2UV1001;
layout(location = 3) in vec2 v2UV11;

layout(location = 4) in vec2 out_Impostor_MS_TiledUV;
layout(location = 5) in vec3 v3Test;

layout(location = 6) in vec2 v2Parallax_First;
layout(location = 7) in vec2 v2Parallax_Second;
layout(location = 8) in vec2 v2Parallax_Third;

layout(push_constant) uniform PushConstants {
    mat4 model;
	vec3 v3Color;
	float fFactor;
} pc;


// Descriptor set 1, binding 0 → your texture sampler
layout(set = 1, binding = 0) uniform sampler2D tSampler_albedo;
layout(set = 1, binding = 1) uniform sampler2D tSampler_normal;


vec3 Tonemap_Hejl(vec3 x) {
    x = max(vec3(0.0), x - 0.004);
    return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
}

vec3 Tonemap_ACES(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 Tonemap_Uncharted2(vec3 x) {
    // Constants from Hable’s filmic curve
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;
    const float W = 11.2; // white point

    vec3 curr = ((x * (A * x + C * B) + D * E) /
                (x * (A * x + B) + D * F)) - E / F;
    vec3 whiteScale = ((vec3(W) * (A * vec3(W) + C * B) + D * E) /
                      (vec3(W) * (A * vec3(W) + B) + D * F)) - E / F;
    return curr / whiteScale;
}

vec3 Tonemap_ReinhardWhite(vec3 x, float whitePoint) {
    return (x * (1.0 + x / (whitePoint * whitePoint))) / (1.0 + x);
}

vec3 Tonemap_Reinhard(vec3 x) {
    return x / (1.0 + x);
}

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

    float depth00 =  0.5-texture(tSampler_normal, uv00).a;
    float depth1001 =  0.5-texture(tSampler_normal, uv1001).a;
    float depth11 =  0.5-texture(tSampler_normal, uv11).a;

    vec2 parallaxOffset00 = depth00 * v2Parallax_First;
    vec2 parallaxOffset1001 = depth1001 * v2Parallax_Second;
    vec2 parallaxOffset11 = depth11 * v2Parallax_Third;

	vec4 albedo00 = texture(tSampler_albedo, uv00 + parallaxOffset00);
    vec4 albedo1001 = texture(tSampler_albedo, uv1001 + parallaxOffset1001);
    vec4 albedo11 = texture(tSampler_albedo, uv11 + parallaxOffset11);

    vec4 blendedAlbedo = albedo00 * v3Weights.x + albedo1001 * v3Weights.y + albedo11 * v3Weights.z;

    //float edgeWidth = fwidth(blendedAlbedo.a) * 0.5;
    //float edgeFactor = smoothstep(0.5 - edgeWidth, 0.5 + edgeWidth, blendedAlbedo.a);
    //edgeFactor = max(edgeFactor, 1.0/255.0); // Ensure edgeFactor is non-negative

    if (blendedAlbedo.a < 0.489) discard;//489

    vec4 normal00 = texture(tSampler_normal, uv00 + parallaxOffset00);
    vec4 normal1001 = texture(tSampler_normal, uv1001 + parallaxOffset1001);
    vec4 normal11 = texture(tSampler_normal, uv11 + parallaxOffset11);


    vec4 blendedNormal = normal00 * v3Weights.x + normal1001 * v3Weights.y + normal11 * v3Weights.z;
	
    vec3 albedo = blendedAlbedo.rgb; //
    vec3 normal = normalize(blendedNormal.rgb * 2.0 - 1.0);

    vec3 finalColor = vec3(albedo) * max(dot(normal,vec3(0.0,1.0,0.0)),0.1) ;
    //finalColor = 1.0 - exp(-finalColor * 5.0);

	FragColor.rgb = Tonemap_ACES(finalColor *1.5);
	//FragColor.rgb = normal;
	//FragColor.rgb = vec3(depth00);
	FragColor.a = 1.0;

}
