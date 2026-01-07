#pragma once
#include "Shape.h"

namespace singleshape
{

class TriangleRainbow: public Shape
{
	static const float vertices[];
	unsigned int vao; //vertex array object

public:

	TriangleRainbow();
	TriangleRainbow(const char* vertexPath, const char* fragmentPath);

	void draw();
	void build();
};

}