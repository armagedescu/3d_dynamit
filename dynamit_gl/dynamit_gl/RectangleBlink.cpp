#include "pch.h"
#include "RectangleBlink.h"
#include <glm/glm.hpp> //basic glm math functions
#include <glm/gtc/type_ptr.hpp> //convert glm types to opengl types

namespace singleshape
{
const float RectangleBlink::vertices[] =
{
	// x  y   z
	 0.5,  0.5, 0, // top right      index: 0
	 0.5, -0.5, 0, // bottom right   index: 1
	-0.5, -0.5, 0, // bottom left    index: 2
	-0.5,  0.5, 0  // top left       index: 3
};
// order of indexes to help build triangles
const unsigned int RectangleBlink::indices[] =
{
	0, 3, 1, // first triangle,  build counter clockwise order
	1, 3, 2  // second triangle, build counter clockwise order
};

RectangleBlink::RectangleBlink() : RectangleBlink("shaders/rectangleBlink.vs", "shaders/rectangleBlink.fs")
{
}
RectangleBlink::RectangleBlink(const char* vertexPath, const char* fragmentPath) : Shape(vertexPath, fragmentPath)
{
	build();
};
void RectangleBlink::build()
{

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//bind ebo data
	unsigned int ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	glUseProgram(*this);
	colorLocationId = glGetUniformLocation(*this, "ziziIn");
}
void RectangleBlink::drawInit(glm::vec4& color)
{
	glUseProgram(*this);
	glUniform4fv(colorLocationId, 1, glm::value_ptr(color));
}

void RectangleBlink::draw()
{
	glUseProgram(*this);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}


}