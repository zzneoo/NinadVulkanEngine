#pragma once
//#include"vmath.h"

#define MYICON 101

//Uniform Buffer Objects------------------------------------------------
//struct UniformBufferObject_camera
//{
//    ///glm::mat4 model;
//    glm::mat4 view;
//    glm::mat4 proj;
//};

struct UniformBufferObject_FrameData
{
	glm::mat4 view;
	glm::mat4 proj;
    float fTime;
    uint32_t frameID;
	float pad0[2];

	glm::vec3 cameraPos; // Camera position for rendering
    // pad to 16 bytes (std140 rules)
	float pad1[1];
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

struct VertexData_PositionTexCoordNormalColor
{
	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec3 normal;
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

//win32
struct CamStruct
{
	bool bCameraMoving_Forward;
	bool bCameraMoving_Backward;
	bool bCameraMoving_Right;
	bool bCameraMoving_Left;
	USHORT mouseX;
	USHORT mouseY;
	float CameraTurboSpeed = 1.0f;
};

struct ClientSize
{
	USHORT ClientWidth;
	USHORT ClientHeight;
};
