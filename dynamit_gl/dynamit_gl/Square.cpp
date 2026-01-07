#include "pch.h"
#include "Square.h"
#include <GL/glew.h>
#include "config.h"
#include <glm/glm.hpp> //basic glm math functions
#include <glm/gtc/matrix_transform.hpp> //matrix functions
#include <glm/gtc/type_ptr.hpp> //convert glm types to opengl types

namespace singleshape
{
//TEXTURED RECTANGLE
const float Square::vertices[] =
{
	 // positions         // colors           // texture coords
     //x    y     z       R     G    B        Ts   Tt
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right          0
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right       1
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left        2
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left           3
};
// order of indexes to help build triangles
const unsigned int Square::indices[] =
{
	0, 3, 1, // first triangle,  make ccw order: GL_CULL_FACE
	1, 3, 2  // second triangle, make ccw order: GL_CULL_FACE
};

Square::Square() : Square("shaders/squareMovingWithTexture.vs", "shaders/squareMovingWithTexture.fs")
{}
Square::Square(const char* vertexPath, const char* fragmentPath) : Shape(vertexPath, fragmentPath)
{
	build();
}
void Square::build()
{
	glGenVertexArrays(1, &vao); //helps tell how VBO data goes into vertex shader
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo); //buffer vertices into VRAM
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	unsigned int ebo;
	glGenBuffers(1, &ebo); //buffer vertex indices for triangle building
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//location = 0 XYZ
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//location = 1 RGB
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//location = 2 Ts Tt (texture coordinates)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	transformUniformLocation = glGetUniformLocation(*this, "transform");
	texture0UniformLocation  = glGetUniformLocation(*this, "texture0crate");

}

void Square::drawInit(unsigned int texture, float time, const glm::vec3& location)
{
	glm::mat4 transform = glm::mat4(1.0f);
	transform = glm::translate (transform, location);
	transform = glm::rotate    (transform, time, glm::vec3(0.0, 0.0, 1.0));
	transform = glm::scale     (transform, glm::vec3(0.5, 0.5, 0.5));

	glUseProgram       (*this);
	glUniformMatrix4fv (transformUniformLocation, 1, GL_FALSE, glm::value_ptr(transform));

	glActiveTexture (GL_TEXTURE0);
	glBindTexture   (GL_TEXTURE_2D, texture);
	glUniform1i(texture0UniformLocation, 0);//<==== set to slot zero
}
void Square::draw()
{
	glUseProgram(*this);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

}