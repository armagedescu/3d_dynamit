#include "pch.h"
#include <iostream>
#include <GL/glew.h>

#include "config.h"
#include "callbacks.h"

GLFWwindow* openglWindowInit(int width, int height)
{
	using std::cout;
	using std::endl;
	using std::cerr;
	using namespace config;
	if (!glfwInit())
		return 0;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, openGlVersionMaj);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, openGlVersionMin);
	glfwWindowHint(GLFW_OPENGL_PROFILE, openGlProfile);
	
	GLFWwindow* window = glfwCreateWindow(width, height, "Starting GL", NULL, NULL);
	windowWidth = width, windowHeight =  height;
	if (window == NULL)
	{
		cout << "Window failed to build!!" << endl;
		glfwTerminate();
		return 0;
	}

	glfwMakeContextCurrent(window);
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "Error: " << glewGetErrorString(err) << endl;
		return 0;
	}
	glViewport(0, 0, windowWidth, windowHeight);

	glfwSetMouseButtonCallback(window, mouse_key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return window;
}