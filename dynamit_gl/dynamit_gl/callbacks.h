#pragma once
#include <GLFW/glfw3.h>
#include "Camera.h"

extern void processInputs(GLFWwindow* window);

extern void mouse_key_callback(GLFWwindow* window, int button, int action, int mods);
extern void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
extern void mouse_callback(GLFWwindow* window, double xpos, double ypos);
extern void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
extern void framebuffer_size_callback(GLFWwindow* window, int width, int height);
