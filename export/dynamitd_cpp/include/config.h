#pragma once
#include <GLFW/glfw3.h>
#include "Camera.h"
#include "util.h"

namespace config
{
	inline Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));//make camera at xyz pos

	inline int windowWidth = 1280, windowHeight = 720;
	inline float lastX = windowWidth / 2, lastY = windowWidth / 2; //center of the screen

	inline int openGlVersionMaj = 3;
	inline int openGlVersionMin = 3;
	inline int openGlProfile = GLFW_OPENGL_CORE_PROFILE;
}


class TimeController {
public:
	double time;
	double currentTime;
	double deltaTime;
	TimeController() :TimeController(0) {}
	TimeController(float start) : time(start), currentTime(0), deltaTime(0) {}
	void update(float newtime) {
		currentTime = newtime;
		deltaTime = currentTime - time;
		time = currentTime;
	}
};

inline float deltaTime = 0, lastFrame = 0;  //time since last frame //time of last frame
const inline float& currentTime = lastFrame;  //time since last frame //time of last frame

inline bool firstMouse = true; //help make sure first mouse read doesn't jump the camera pos

inline bool       keyPressed = false;
inline int        keyMod = 0;
inline int        keyModAlt = keyMod;
const int MOD_CTRLALT = GLFW_MOD_CONTROL | GLFW_MOD_ALT;
inline int        currentDraw = 1;
inline const int& currentShape = currentDraw;


//define draw actions
const int DRAW_1                 =  1;
const int DRAW_2                 =  2;
const int DRAW_3                 =  3;
const int DRAW_4                 =  4;
const int DRAW_5                 =  5;
const int DRAW_6                 =  6;
const int DRAW_7                 =  7;
const int DRAW_8                 =  8;
const int DRAW_9                 =  9;
const int DRAW_0                 = 10;

const int DRAW_TRIANGLE_SIMPLE                 = 1;
const int DRAW_RECTANGLE                       = 2;
const int DRAW_RAINBOW_TRIANGLE                = 3;
const int DRAW_RAINBOW_TRIANGLE_WITH_CAMERA    = 4;
const int DRAW_SQUARE                          = 5;
const int DRAW_CUBES                           = 6;
extern GLFWwindow* openglWindowInit(int width = config::windowWidth, int height = config::windowHeight);
extern void processInputs(GLFWwindow* window);
