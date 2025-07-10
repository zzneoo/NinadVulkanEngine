#version 460

layout(location = 0) in  vec2 v2TexCoord;
layout(location = 0) out vec4 fragColor;

void main() 
{
    fragColor = vec4(v2TexCoord,0.0,1.0);
}

