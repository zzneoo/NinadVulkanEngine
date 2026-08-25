#pragma once
#include"MyWin32.h"


	CamStruct MyWin32::myCamStruct = {};
	ClientSize  MyWin32::myClientSize = {};
	BOOL MyWin32::bFirstFrame = TRUE;
	BOOL MyWin32::isGUI = TRUE;

	BOOL MyWin32::bRecentResize = TRUE;


	uint32_t MyWin32::iFrameID = 0;
	//unsigned char MyWin32::iBlockPixelID = 0;
	
	glm::mat4 MyWin32::gProjectionMatrix = glm::mat4(1.0);
	glm::mat4 MyWin32::gPrevViewProjMatrix = glm::mat4(1.0);
	//vmath::mat4 MyWin32::gInvProjectionMatrix = vmath::mat4::identity();
	
	glm::mat4 MyWin32::gViewMatrix = glm::mat4(1.0);
	//vmath::mat4 MyWin32::gInvViewMatrix = vmath::mat4::identity();
	
	//vmath::vec3 MyWin32::gPrevCameraPosition = vmath::vec3(0.0);
	glm::vec2 MyWin32::gNearFarFrustum = glm::vec2(0.01f,1000.0f);
	float MyWin32::fovY = 80.0f;
	float MyWin32::fDeltaTime = 0.0f;
	double MyWin32::dTotalElapsedTime = 0.0;

	//


