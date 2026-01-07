#include "pch.h"
#include <GL/glew.h>
#include "Triangle.h"

namespace singleshape
{

const char Triangle::triangleVertexShaderSource[] =
"#version 330 core"                                     "\n"
"layout(location = 0) in vec3 aPos;"                    "\n"//try pass xyz values for point into this location
"void main()"                                           "\n"
"{"                                                     "\n"
"    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);"  "\n"//vertex shader must set vec4 gl_position for shape assembly
"}"                                                     "\n";

//Fragment Shader Source Code
const char Triangle::triangleFragmentShaderSource[] =
"#version 330 core"                              "\n"
"out vec4 FragColor;"                            "\n"//fragment shader must out a vec4 for rendering a pixel
"void main()"                                    "\n"
"{"                                              "\n"
"    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);"  "\n"//output 1 colour per pixel drawn
"}"                                              "\n";

const float Triangle::triangleVertices[] =
{
	//X    Y    Z
	-0.5, -0.5, 0, //bottom left
	 0.5, -0.5, 0, // bottom right
	 0.0,  0.5, 0  //top
};

Triangle::Triangle() : Triangle(triangleVertexShaderSource, triangleFragmentShaderSource)
{
}
Triangle::Triangle(const char* vertexPath, const char* fragmentPath)
	: Shape(vertexPath, fragmentPath)
{
	build();
}
void Triangle::build()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

void Triangle::draw()
{
	glUseProgram(*this);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

}