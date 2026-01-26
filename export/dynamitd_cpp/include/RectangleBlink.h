#pragma once
#include "Shape.h"
#include <glm/glm.hpp> //basic glm math functions

namespace singleshape
{

class RectangleBlink: public Shape
{
	static const float vertices[];
	static const unsigned int indices[];
	unsigned int colorLocationId;
	unsigned int vao;
public:

	explicit RectangleBlink();
	explicit RectangleBlink(const char* vertexPath, const char* fragmentPath);

	void drawInit(glm::vec4& color);
	void draw();
	void build();
};

}