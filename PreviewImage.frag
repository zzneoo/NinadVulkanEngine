#version 460

layout(location = 0) in  vec2 v2TexCoord;
layout(location = 0) out vec4 fragColor;

// Descriptor set 0, binding 0 → your texture sampler
layout(set = 0, binding = 0) uniform sampler2D tSampler;

void main() 
{
	vec4 color = texture(tSampler,v2TexCoord);
    fragColor = vec4(color.rgb,1.0);
}

