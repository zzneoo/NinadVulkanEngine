#pragma once
#include "framework.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  // for lookAt, perspective, etc.
#include <glm/gtc/type_ptr.hpp>          // if passing data to OpenGL/Vulkan
#include"MyWin32.h"

struct Camera
{
public:
	Camera();
	~Camera();

	void UpdateViewMatrix(HWND hwnd);
	//void UpdateInfiniteViewMatrix(void);
	//void UpdateLightViewMatrix(glm::vec3 sunPos, glm::vec3 Target);

	//getters
	float GetYaw(void)
	{
		return Yaw;
	}
	float GetPitch(void)
	{
		return Pitch;
	}

	glm::vec3 GetCameraPos(void)
	{
		return CameraPosition;
	}
	void SetCameraPos(glm::vec3 newPos)
	{
		CameraPosition = glm::vec3(newPos);
	}

	glm::vec3 GetCameraDirection(void)
	{
		return CameraForward;
	}

	float GetCameraMagnitude(void)
	{
		return glm::length(CameraPosition);
	}

	glm::vec3 GetInitialCameraPosition(void)
	{
		return InitialCameraPosition;
	}

	glm::mat4 GetViewMatrix(void)
	{
		return ViewMatrix;
	}

	//glm::mat4 GetLightViewMatrix(void)
	//{
	//	return LightViewMatrix;
	//}

	//glm::mat4 GetInfiniteViewMatrix(void)
	//{
	//	return InfiniteViewMatrix;
	//}

	glm::vec3 GetCameraRightVector(void)
	{
		return CameraRight;
	}

	//setter
	void SetYaw(float fYaw)
	{
		Yaw = fYaw;
	}
	void SetPitch(float fPitch)
	{
		Pitch = fPitch;
	}

	void CameraTurboSpeed(uint32_t Speed, bool bSpaceGroundToggle);

private:
	glm::vec3 CameraPosition;

	glm::vec3 CameraForward;
	glm::vec3 CameraRight;
	glm::vec3 CameraUp;

	glm::mat4 ViewMatrix;
	//glm::mat4 LightViewMatrix;
	//glm::mat4 InfiniteViewMatrix;

	const glm::vec3 InitialCameraPosition{ glm::vec3(0.0f, 0.0f, 0.0f) };

	float CameraSpeed;

	float Yaw{ 270.0f };
	float Pitch{ 0.0f };
};

