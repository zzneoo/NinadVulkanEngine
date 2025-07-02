#pragma once
//#include"vmath.h"

#define MYICON 101

struct UniformTransformBufferObject 
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
