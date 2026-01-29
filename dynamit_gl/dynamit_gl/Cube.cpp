#include "pch.h"

#include <GL/glew.h>
#include "Cube.h"
#include "config.h"
#include <glm/glm.hpp>                   //basic glm math functions
#include <glm/gtc/matrix_transform.hpp>  //matrix functions
#include <glm/gtc/type_ptr.hpp>          //convert glm types to opengl types

namespace singleshape
{

glm::vec3 CubeSet::cubePositions[] =
{
	glm::vec3( 0.0f,    0.0f,    0.0f),
	glm::vec3( 2.0f,    5.0f,  -15.0f),
	glm::vec3(-1.5f,   -2.2f,   -2.5f),
	glm::vec3(-3.8f,   -2.0f,  -12.3f),
	glm::vec3( 2.4f,   -0.4f,   -3.5f),
	glm::vec3(-1.7f,    3.0f,   -7.5f),
	glm::vec3( 1.3f,   -2.0f,   -2.5f),
	glm::vec3( 1.5f,    2.0f,   -2.5f),
	glm::vec3( 1.5f,    0.2f,   -1.5f),
	glm::vec3(-1.3f,    1.0f,   -1.5f)
};

//cube verts
float CubeSet::textureVertices[] =
{
	 //VERTEX PPOSITION       //TEXTURE COORDINATES
	 0.0f,  1.0f,  0.0f,      0.5f, 0.0f, 1.0f,     // Top R Vertex 0
	-1.0f, -1.0f,  1.0f,      0.0f, 1.0f, 0.5f,     // bottom left front vertex 1
	 1.0f, -1.0f,  1.0f,      1.0f, 0.5f, 0.0f,     // Bottom right front Vertex 2

	 0.0f,  1.0f,  0.0f,      0.5f, 0.0f, 1.0f,     // Top R Vertex 0
	 1.0f, -1.0f,  1.0f,      0.0f, 1.0f, 0.5f,     // bottom left front vertex 1
	 1.0f, -1.0f, -1.0f,      1.0f, 0.5f, 0.0f,     // bottom left back vertex 4

	 0.0f,  1.0f,  0.0f,      0.5f, 0.0f, 1.0f,     // Top R Vertex 0
	 1.0f, -1.0f, -1.0f,      0.0f, 1.0f, 0.5f,     // bottom left back vertex 4
	-1.0f, -1.0f, -1.0f,      1.0f, 0.5f, 0.0f,     // bottom right back vertex 3

	 0.0f,  1.0f,  0.0f,      0.5f, 0.0f, 1.0f,     // Top R Vertex 0
	-1.0f, -1.0f, -1.0f,      0.0f, 1.0f, 0.5f,     // bottom right back vertex 3
	-1.0f, -1.0f,  1.0f,      1.0f, 0.5f, 0.0f,     // Bottom right front Vertex 2

	-1.0f, -1.0f,  1.0f,      0.1f, 0.0f, 1.0f,     // bottom left front vertex 1
	 1.0f, -1.0f,  1.0f,      0.1f, 0.0f, 1.0f,     // bottom left back vertex 4
	 1.0f, -1.0f, -1.0f,      0.1f, 0.0f, 1.0f,     // bottom right back vertex 3

	-1.0f, -1.0f, -1.0f,      0.1f, 0.0f, 1.0f,
};

CubeSet::CubeSet() : CubeSet("shaders/dynamicPyramid.vs", "shaders/dynamicPyramid.fs") {}
CubeSet::CubeSet(const char* vertexPath, const char* fragmentPath): Shape(vertexPath, fragmentPath)
{
	build();
}

void CubeSet::build()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureVertices), textureVertices, GL_STATIC_DRAW);

	//vertex positions, and texture coordinates, in the vbo the same both
	//note attribute location=1 exists in shader but used is not.
	//location = 0 X Y Z
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//location = 2 Texture coordinates
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	texture0LocationId = glGetUniformLocation(*this, "textureForCube");
	//texture1LocationId = glGetUniformLocation(*this, "textureForCube");

	modelLocationId      = glGetUniformLocation(*this, "model");
	viewLocationId       = glGetUniformLocation(*this, "view");
	projectionLocationId = glGetUniformLocation(*this, "projection");

}

void CubeSet::init(int n)
{
	cubes.reserve(n);
	for (int i = 0; i < n; i++)
		cubes.push_back( { cubePositions[i % std::size(cubePositions)] });
}
void CubeSet::drawInit(unsigned int texture, glm::mat4& view, glm::mat4& projection)
{
	glUseProgram(*this);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(texture0LocationId, 0);
	////set other texture
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, texture1airplane);
	//glUniform1i(texture1LocationId, 1);//<==== set to slot one

	glUniformMatrix4fv(viewLocationId,       1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLocationId, 1, GL_FALSE, glm::value_ptr(projection));
}

void CubeSet::draw()
{
	glUseProgram(program);

	for (Cube& cube : cubes)
	{
		using namespace config;
		glm::mat4 model = glm::translate(glm::mat4(1.0), cube.pos);
		model = glm::rotate(model, (float)(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLocationId,      1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

}