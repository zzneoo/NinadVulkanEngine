#pragma once
#include "framework.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  // for lookAt, perspective, etc.
#include <glm/gtc/type_ptr.hpp>          // if passing data to OpenGL/Vulkan
#include "VK.h"

struct MyWin32
{
public:

	MyWin32() {};
	~MyWin32() {};


	//variables
	static glm::mat4 gProjectionMatrix;
	//static vmath::mat4 gInvProjectionMatrix;

	static glm::mat4 gViewMatrix;
	//static vmath::mat4 gInvViewMatrix;

	static glm::vec2 gNearFarFrustum;
	static float fovY;
	static float fDeltaTime;
	static double dTotalElapsedTime;

	static CamStruct myCamStruct;
	static ClientSize myClientSize;
	static 	BOOL bFirstFrame;
	static 	BOOL bRecentResize;
	static BOOL isGUI;

	static unsigned int iFrameID;


	//Made static for Vulkan surface creation
	static HWND hwnd;


private:

};