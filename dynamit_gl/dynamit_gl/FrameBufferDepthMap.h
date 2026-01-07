#pragma once

#include <glm/glm.hpp> //basic glm math functions
#include <functional>
#include "Program.h"

template <class T> class FrameBufferDepthMap
{
	unsigned int framebuffer;
	unsigned int texture;

	T runner;
public:
	unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	Program& getProgram() { return runner.program; }
	FrameBufferDepthMap(T& shape);
	FrameBufferDepthMap(const char* vertexPath, const char* fragmentPath);
	void build();
	void drawInit(glm::mat4& lightSpaceMatrix);
	void draw(std::function<void(void)> drawInit = [] {});

	unsigned int getTexture() { return texture; }
};