#include "pch.h"
#include "TrianglesPoli.h"
#include <GL/glew.h>

namespace singleshape
{

const vertexcolor<float, 3, 4> TrianglesPoli::obj[] =
{
	//  x      y       z        r    g    b    a

	{-0.6f, -0.6f,   0.99,    0.0, 0.0, 1.0, 0.3},
	{ 0.4f, -0.6f,   0.99,    0.0, 0.0, 1.0, 0.3},
	{-0.1f,  0.4f,   0.99,    0.0, 0.0, 1.0, 0.3},

	{-0.5f, -0.5f,   0.00,    0.0, 1.0, 0.0, 1.0}, // base
	{ 0.5f, -0.5f,   0.00,    0.0, 1.0, 0.0, 1.0}, // base
	{ 0.0f,  0.5f,   0.00,    0.0, 1.0, 0.0, 1.0}, // base

	{-0.4f, -0.4f,  -0.10,    0.0, 0.0, 1.0, 0.3},
	{ 0.6f, -0.4f,  -0.10,    0.0, 0.0, 1.0, 0.3},
	{ 0.1f,  0.6f,  -0.10,    0.0, 0.0, 1.0, 0.3},

	{-0.3f, -0.3f,  -0.99,    1.0, 0.0, 0.0, 0.3},
	{ 0.7f, -0.3f,  -0.99,    1.0, 0.0, 0.0, 0.3},
	{ 0.2f,  0.7f,  -0.99,    1.0, 0.0, 0.0, 0.3},

};

TrianglesPoli::TrianglesPoli() : TrianglesPoli("shaders/trianglePoli.vs", "shaders/trianglePoli.fs")
{
}
TrianglesPoli::TrianglesPoli(const char* vertexPath, const char* fragmentPath) : Shape(vertexPath, fragmentPath)
{
	build();
}

void TrianglesPoli::build()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(obj), obj, GL_STATIC_DRAW);

	glVertexAttribPointer(0, vcl::vt::num, GL_FLOAT, GL_FALSE, sizeof(vcl), vcl::vstart);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, vcl::ct::num, GL_FLOAT, GL_FALSE, sizeof(vcl), vcl::cstart);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}
void TrianglesPoli::draw()
{
	glUseProgram(program);
	glBindVertexArray(vao);

	glDrawArrays(GL_TRIANGLES, 0,  std::size(obj));
}


}