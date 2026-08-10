#version 460

layout(location = 0) in vec4 inNormal;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(pow(inColor.xyz,vec3(2.2)),1.0);
}
