#ifndef FRAME_DATA_GLSL
#define FRAME_DATA_GLSL

layout(set = 0, binding = 0, std140) uniform FrameData
{
    mat4 view;
    mat4 proj;

    vec3 sunDir;
    float fTime;

    vec3 cameraPos;
    uint frameID;

    vec4 frustumPlanes[6];
} global;

#endif