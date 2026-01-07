#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <GLFW/glfw3.h> //for GLtypes
#include <glm/glm.hpp>

class Camera
{
	static const float YAW, PITCH, SPEED, SENSITIVITY, ZOOM;
public:
	enum class Movement
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};
	glm::vec3 position, front, up, right, worldUp;
	
	float yaw, pitch; // Euler Angles yaw/pitch/roll (down y/right x/ forward z axes respectively)
	float movementSpeed, mouseSensitivity;
	float zoom; // Camera options

	//Camera(pos {0.0f, 0.0f, 0.0f}, up{0.0f, 1.0f, 0.0f}, float yaw = -90.0f, float pitch = 0.0f);

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	glm::mat4 view();
	glm::mat4 perspective (float znear = 0.1f, float zfar = 100.0f);
	glm::mat4 perspective (float windowWidth, float windowHeight, float znear, float zfar);
	void onKeyboard    (Camera::Movement direction, float deltaTime);
	void onMouseMove   (float xoffset, float yoffset, GLboolean constrainPitch = true);
	void onMouseScroll (float yoffset);

	void updateVectors();
};
#endif