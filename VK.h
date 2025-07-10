#pragma once
//#include"vmath.h"

#define MYICON 101

//Uniform Buffer Objects------------------------------------------------
struct UniformBufferObject_camera
{
    ///glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct UniformBufferObject_FrameData
{
    float fDeltaTime;
    uint32_t frameID;
    // pad to 16 bytes (std140 rules)
    float    _pad[2];
};

//----------------------------------------------------------------------

struct VertexData_PositionColor
{
	glm::vec3 pos;
	glm::vec3 color;
};

struct VertexData_PositionTexCoord
{
	glm::vec3 pos;
	glm::vec2 texCoord;
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
