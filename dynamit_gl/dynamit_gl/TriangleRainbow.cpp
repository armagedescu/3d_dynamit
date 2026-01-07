#include "pch.h"
#include "TriangleRainbow.h"
#include <GL/glew.h>

namespace singleshape
{

const float TriangleRainbow::vertices[] =
{
	  //x      y      z      r  g  b
	-0.5f, -0.5f,   1.0,     0, 1, 0,
	 0.5f, -0.5f,   0.0,     1, 0, 0,
	 0.0f,  0.5f,  -1.0,     0, 0, 1
};

TriangleRainbow::TriangleRainbow() : TriangleRainbow("shaders/triangleGradient.vs", "shaders/triangleGradient.fs")
{
}
TriangleRainbow::TriangleRainbow(const char* vertexPath, const char* fragmentPath) : Shape(vertexPath, fragmentPath)
{
	build();
}

void TriangleRainbow::build()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//get XYZ values into location 0 on vertex shader
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//RGB to location 1
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}
void TriangleRainbow::draw()
{
	glUseProgram(program);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}


}