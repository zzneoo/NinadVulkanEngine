#include "Camera.h"

Camera::Camera()
{
	CameraSpeed = 5.0f;
	CameraPosition = InitialCameraPosition;

	CameraForward = glm::vec3(0.0);
	CameraRight = glm::vec3(0.0);
	CameraUp = glm::vec3(0.0);

	ViewMatrix = glm::mat4(1.0f);
	//InfiniteViewMatrix = glm::mat4(1.0f);


		//Yaw = 0.0f;
		//Pitch = -90.0f;

	Yaw = -90.0f;
	Pitch = 0.0f;
}


Camera::~Camera()
{
}

void Camera::CameraTurboSpeed(uint32_t Speed, bool bSpaceGroundToggle)
{
	if (bSpaceGroundToggle)
	{
		switch (Speed)
		{
		case 0:
			CameraSpeed = 200.0f;
			break;
		case 1:
			CameraSpeed = 4000.0f;
			break;
		case 2:
			CameraSpeed = 8000.0f;
			break;
		case 3:
			CameraSpeed = 16000.0f;
			break;
		case 4:
			CameraSpeed = 64000.0f;
			break;
		case 5:
			CameraSpeed = 256000.0f;
			break;
		case 6:
			CameraSpeed = 1024000.0f;
			break;
		case 7:
			CameraSpeed = 4096000.0f;
			break;
		case 8:
			CameraSpeed = 16384000.0f;
			break;
		case 9:
			CameraSpeed = 64384000.0f;
			break;

		default:
			CameraSpeed = 8000.0f;
			break;
		}
	}
	else
	{
		switch (Speed)
		{
		case 0:
			CameraSpeed = 0.1f;
			break;
		case 1:
			CameraSpeed = 1.0f;
			break;
		case 2:
			CameraSpeed = 2.0f;
			break;
		case 3:
			CameraSpeed = 4.0f;
			break;
		case 4:
			CameraSpeed = 8.0f;
			break;
		case 5:
			CameraSpeed = 16.0f;
			break;
		case 6:
			CameraSpeed = 32.0f;
			break;
		case 7:
			CameraSpeed = 64.0f;
			break;
		case 8:
			CameraSpeed = 128.0f;
			break;
		case 9:
			CameraSpeed = 256.0f;
			break;

		default:
			CameraSpeed = 1.0f;
			break;
		}
	}

}

void Camera::UpdateViewMatrix(HWND hwnd)
{

	POINT ClientCenter = { MyWin32::myClientSize.ClientWidth / 2,MyWin32::myClientSize.ClientHeight / 2 };
	POINT ClientToScreenCenter = ClientCenter;
	ClientToScreen(hwnd, (LPPOINT)&ClientToScreenCenter);

	if (TRUE == MyWin32::bFirstFrame)  //firstFrame is true if toggleFullscreen
	{
		MyWin32::myCamStruct.mouseX = (MyWin32::myClientSize.ClientWidth / 2);
		MyWin32::myCamStruct.mouseY = (MyWin32::myClientSize.ClientHeight / 2);
		MyWin32::bFirstFrame = FALSE;
	}

	SetCursorPos(ClientToScreenCenter.x, ClientToScreenCenter.y);

	float dx = (float)(MyWin32::myCamStruct.mouseX - ClientCenter.x);
	float dy = (float)(ClientCenter.y - MyWin32::myCamStruct.mouseY);

	float MouseSensitivity = MyWin32::fDeltaTime * 17.0f;
	 //MouseSensitivity = 0.01f;//temp

	dx *= MouseSensitivity;
	dy *= MouseSensitivity;

	Yaw += dx;
	Pitch += dy;

	if (Pitch > 80.0f)
		Pitch = 80.0f;
	if (Pitch < -80.0f)
		Pitch = -80.0f;

	//Pi/180=0.01745329251

	float fPitch = Pitch * 0.01745329251f;
	float fYaw = Yaw * 0.01745329251f;


	glm::vec3 front{};
	front.x = cosf(fPitch) * cosf(fYaw);
	front.y = sinf(fPitch);
	front.z = cosf(fPitch) * sinf(fYaw);

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	CameraForward = glm::normalize(front);
	CameraRight = glm::normalize(glm::cross( CameraForward, up));
	CameraUp = glm::cross( CameraRight, CameraForward);


	//Camera Position
	{
		if (MyWin32::myCamStruct.bCameraMoving_Forward )//W
		{
			CameraPosition += CameraForward * MyWin32::fDeltaTime * CameraSpeed * MyWin32::myCamStruct.CameraTurboSpeed;
		}
		if (MyWin32::myCamStruct.bCameraMoving_Backward)//S
		{
			CameraPosition -= CameraForward * MyWin32::fDeltaTime * CameraSpeed * MyWin32::myCamStruct.CameraTurboSpeed;
		}
		if (MyWin32::myCamStruct.bCameraMoving_Left)//A
		{
			CameraPosition -= CameraRight * MyWin32::fDeltaTime * CameraSpeed * MyWin32::myCamStruct.CameraTurboSpeed;
		}
		if (MyWin32::myCamStruct.bCameraMoving_Right)//D
		{
			CameraPosition += CameraRight * MyWin32::fDeltaTime * CameraSpeed * MyWin32::myCamStruct.CameraTurboSpeed;
		}
	}

	//	CameraPosition[1] = 0.0f;

		//ViewMatrix
		//short for matrix multiplication C(angles) x T(position)
	float RdotE = glm::dot(CameraRight, CameraPosition);
	float UdotE = glm::dot(CameraUp, CameraPosition);
	float FdotE = glm::dot(CameraForward, CameraPosition);


	ViewMatrix = glm::mat4(
		glm::vec4(CameraRight[0], CameraUp[0], -CameraForward[0], 0.0f),
		glm::vec4(CameraRight[1], CameraUp[1], -CameraForward[1], 0.0f),
		glm::vec4(CameraRight[2], CameraUp[2], -CameraForward[2], 0.0f),
		glm::vec4(glm::vec3(-RdotE, -UdotE, FdotE), 1.0f));

	//ViewMatrix=vmath::lookat(CameraPosition, CameraPosition+ CameraForward, vmath::vec3(0.0f, 1.0f, 0.0f));
}

//void Camera::UpdateInfiniteViewMatrix(void)
//{
//	float RdotE = glm::dot(CameraRight, InitialCameraPosition);
//	float UdotE = glm::dot(CameraUp, InitialCameraPosition);
//	float FdotE = glm::dot(CameraForward, InitialCameraPosition);
//
//	InfiniteViewMatrix = glm::mat4(glm::vec4(CameraRight[0], CameraUp[0], -CameraForward[0], 0.0),
//		glm::vec4(CameraRight[1], CameraUp[1], -CameraForward[1], 0.0),
//		glm::vec4(CameraRight[2], CameraUp[2], -CameraForward[2], 0.0),
//		glm::vec4(glm::vec3(-RdotE, -UdotE, FdotE), 1.0));
//}
//
//void Camera::UpdateLightViewMatrix(glm::vec3 sunPos, glm::vec3 Target)
//{
//
//	glm::vec3 UniversalUp = glm::vec3(0.0, 1.0, 0.0);
//
//	glm::vec3 SunForward = normalize(Target - sunPos);
//	glm::vec3 SunRight = glm::normalize(glm::cross(SunForward, UniversalUp));
//	//vmath::vec3 SunUp = vmath::cross(SunRight, SunForward);
//	glm::vec3 SunUp = glm::cross(SunRight, SunForward);
//
//	float RdotS = glm::dot(SunRight, sunPos);
//	float UdotS = glm::dot(SunUp, sunPos);
//	float FdotS = glm::dot(SunForward, sunPos);
//
//	LightViewMatrix = glm::mat4(glm::vec4(SunRight[0], SunUp[0], SunForward[0], 0.0),
//		glm::vec4(SunRight[1], SunUp[1], SunForward[1], 0.0),
//		glm::vec4(SunRight[2], SunUp[2], SunForward[2], 0.0),
//		glm::vec4(glm::vec3(-RdotS, -UdotS, -FdotS), 1.0));
//}

