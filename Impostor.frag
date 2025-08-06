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
layout(set = 1, binding = 0) uniform sampler2D tSampler;


void main(void)
{

	//3 Frames

    vec2 uv00 = clamp(v2UV00, 0.0, 1.0)/12.0;

	vec4 color00 = texture(tSampler, uv00);
    vec4 color1001 = texture(tSampler, v2UV1001);
    vec4 color11 = texture(tSampler, v2UV11);
	
	FragColor = color00 * v3Weights.x + color1001 * v3Weights.y + color11 * v3Weights.z;

    FragColor.rgb  = clamp(v3Test.rgb,vec3(0.0),vec3(1.0))/12.0;
    FragColor.rgb  = color00.rgb;
    FragColor.rgb *= 6.0;
    //FragColor.rgb = vec3(1.0);

    //if (FragColor.a < 0.5)
      //  discard;
}
