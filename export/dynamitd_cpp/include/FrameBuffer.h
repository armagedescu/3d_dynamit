#pragma once
#include <glm/glm.hpp> //basic glm math functions
#include <functional>
#include "Program.h"
template <class T> class FrameBuffer
{
	unsigned int framebuffer;
	unsigned int texture;
public:
	T runner;

	Program& getProgram() { return runner.program; }
	FrameBuffer (const char* vertexPath, const char* fragmentPath);
	void build  ();
	void drawInit(glm::mat4& model, glm::mat4& view, glm::mat4& projection);
	void draw(std::function<void(void)> _drawInit = [] {});
	unsigned int getTexture() { return texture; }
	operator unsigned int()  { return runner.program.operator unsigned int();  }
	operator bool() { return runner.program.operator bool(); }
};