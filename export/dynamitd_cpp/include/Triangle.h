#pragma once
#include "Shape.h"
#include "Program.h"

namespace singleshape
{

class Triangle: public Shape
{
	static const char  triangleVertexShaderSource[];
	static const char  triangleFragmentShaderSource[];
	static const float triangleVertices[];
public:
	unsigned int vao;

	Triangle();
	Triangle(const char* vertexPath, const char* fragmentPath);

	void draw();
	void build();
};

}