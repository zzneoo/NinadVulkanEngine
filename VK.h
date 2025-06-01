#pragma once
#include"vmath.h"

#define MYICON 101

struct UniformTransformBufferObject 
{
    vmath::mat4 view;
    vmath::mat4 proj;
};
