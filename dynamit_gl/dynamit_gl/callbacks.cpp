#include "pch.h"
#include <GLFW/glfw3.h>
#include "Camera.h"
#include "config.h"
#include <iostream>
//time management

using config::camera;
using config::lastX;
using config::lastY;

//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
void mouse_key_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT:
			std::cout << "lbutton click" << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			std::cout << "rbutton click" << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			std::cout << "mbutton click" << std::endl;
			break;
		}
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	keyModAlt = keyMod;
	if (action == GLFW_PRESS)
		keyMod = mods;
	if (action == GLFW_PRESS)
	{
		//std::cout << "Key pressed: " << key << " Scancode: " << scancode << " Mods: " << mods << "\n";
		//throw;
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, true);
			break;
		case GLFW_KEY_1:
			keyPressed = true;
			currentDraw = 1;
			break;
		case GLFW_KEY_2:
			keyPressed = true;
			currentDraw = 2;
			break;
		case GLFW_KEY_3:
			keyPressed = true;
			currentDraw = 3;
			break;
		case GLFW_KEY_4:
			keyPressed = true;
			currentDraw = 4;
			break;
		case GLFW_KEY_5:
			keyPressed = true;
			currentDraw = 5;
			break;
		case GLFW_KEY_6:
			keyPressed = true;
			currentDraw = 6;
			break;
		case GLFW_KEY_7:
			keyPressed = true;
			currentDraw = 7;
			break;
		case GLFW_KEY_8:
			keyPressed = true;
			currentDraw = 8;
			break;
		case GLFW_KEY_9:
			keyPressed = true;
			currentDraw = 9;
			break;
		case GLFW_KEY_0:
			keyPressed = true;
			currentDraw = 10;
			break;
		default:
			keyPressed = false;
			break;
		}
	}
}

void processInputs(GLFWwindow* window)
{
	deltaTime = (float)glfwGetTime() - lastFrame;
	lastFrame += deltaTime;
	//CAMERA STUFF WASD movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.onKeyboard(Camera::Movement::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.onKeyboard(Camera::Movement::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.onKeyboard(Camera::Movement::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.onKeyboard(Camera::Movement::RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	//work out pixel difference between lastX and Y and current mouse X and Y
	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos;//need to reverse to get correct movement

	lastX = xpos;
	lastY = ypos;

	camera.onMouseMove(xOffset, yOffset);

}
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
	camera.onMouseScroll(yOffset);//changes FOV of camera
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	config::windowWidth  = width;
	config::windowHeight = height;
	glViewport(0, 0, config::windowWidth, config::windowHeight);
}