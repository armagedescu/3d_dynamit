#pragma once
#include <glm/glm.hpp>
#include "Shape.h"

namespace singleshape
{

class Square : public Shape
{
	static const float vertices[];
	static const unsigned int indices[];
	unsigned int transformUniformLocation;
	unsigned int texture0UniformLocation;

	unsigned int vao; //vertex array object

public:

	Square();
	Square(const char* vertexPath, const char* fragmentPath);

	void build();
	void drawInit(unsigned int texture, float time, const glm::vec3& location);
	void draw();
};

}