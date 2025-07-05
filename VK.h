#pragma once
//#include"vmath.h"

#define MYICON 101

struct UniformBufferObject_camera
{
    ///glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct VertexData_PositionColor
{
	glm::vec3 pos;
	glm::vec3 color;
};

struct VertexData_Position
{
	glm::vec3 pos;
};

struct PushConstants
{
	glm::mat4 model;
	glm::vec3 v3Color;
	float fFactor;
};
