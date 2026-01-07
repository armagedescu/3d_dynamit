#include "pch.h"
#include <GL/glew.h>
#include "Tess.h"
#include <glm/glm.hpp> //basic glm math functions
#include <glm/gtc/matrix_transform.hpp> //matrix functions
#include <glm/gtc/type_ptr.hpp> //convert glm types to opengl types

#include "config.h"


Tess::Tess() {}

Tess::Tess(const char* controlShader, const char* evaluationShader,
	       const char* vertexShader,  const char* fragmentShader)
{
	program
		.addShader(controlShader,    GL_TESS_CONTROL_SHADER)
		.addShader(evaluationShader, GL_TESS_EVALUATION_SHADER)
		.addShader(vertexShader,     GL_VERTEX_SHADER)
		.addShader(fragmentShader,   GL_FRAGMENT_SHADER)
		.build();
}

GLfloat TessQuad::points[] =
{
	-0.8f, -0.8f, 0.0f,
	 0.8f, -0.8f, 0.0f,
	 0.8f,  0.8f, 0.0f,
	-0.8f,  0.8f, 0.0f,
	-0.8f, -0.8f, 0.0f
};

TessQuad::TessQuad() :
	Tess("shaders/tessellation/quad.tcs", "shaders/tessellation/quad.tes",
		 "shaders/tessellation/quad.vs",  "shaders/tessellation/quad.fs")
{
	build();
}

void TessQuad::build()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glBindVertexArray(0);

}

void TessQuad::draw()
{
	glUseProgram(*this);
	glPatchParameteri(GL_PATCH_VERTICES, 4); //comment for tri patch

	glBindVertexArray(vao);
	glDrawArrays(GL_PATCHES, 0, 4);
}

GLfloat TessTriangle::points[] =
{
	-0.7f, -0.7f, 0.0f,
	 0.7f, -0.7f, 0.0f,
	 0.7f,  0.7f, 0.0f,
	-0.7f,  0.7f, 0.0f,
	-0.7f, -0.7f, 0.0f
};

TessTriangle::TessTriangle() 
	: Tess ("shaders/tessellation/triangle.tcs", "shaders/tessellation/triangle.tes",
		    "shaders/tessellation/triangle.vs",  "shaders/tessellation/triangle.fs")
{
	build();
}

void TessTriangle::build()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glBindVertexArray(0);

}

void TessTriangle::draw()
{
	glUseProgram(*this);
	glPatchParameteri(GL_PATCH_VERTICES, 3); //comment for tri patch


	glBindVertexArray(vao);
	glDrawArrays(GL_PATCHES, 0, 3);
	glDrawArrays(GL_PATCHES, 2, 3);
}

GLfloat TessTriangleRainbow::points[] =
{
	//x      y       z       r    g    b  //  a
  -0.5f, -0.5f,   0.0f,   0.0f, 0.f, 1.f, //1.f,
   0.5f, -0.5f,   0.0f,   1.0f, 0.f, 0.f, //1.f,
   0.0f,  0.5f,   0.0f,   0.0f, 1.f, 0.f, //1.f
};

TessTriangleRainbow::TessTriangleRainbow()
	: Tess ("shaders/tessellation/triangleRainbow.tcs", "shaders/tessellation/triangleRainbow.tes",
		    "shaders/tessellation/triangleRainbow.vs",  "shaders/tessellation/triangleRainbow.fs")
{
	build();
}

void TessTriangleRainbow::build()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);

	//get XYZ values into location 0 on vertex shader
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//RGBA to location 1
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	glBindVertexArray(0);
}
void TessTriangleRainbow::draw()
{
	glUseProgram(*this);
	glPatchParameteri(GL_PATCH_VERTICES, 3);

	glBindVertexArray(vao);
	glDrawArrays(GL_PATCHES, 0, 3);
}

GLfloat TessTriangleIndexed::points[] =
{
	-1.0f / 2, -1.0f / 2, 0.0f,
	 1.0f / 2, -1.0f / 2, 0.0f,
	 1.0f / 2,  1.0f / 2, 0.0f,
	-1.0f / 2,  1.0f / 2, 0.0f
};
const unsigned int TessTriangleIndexed::indices[] =
{
	0, 1, 2,
	2, 3, 0
};

TessTriangleIndexed::TessTriangleIndexed()
	: Tess("shaders/tessellation/triangle.tcs", "shaders/tessellation/triangle.tes",
		   "shaders/tessellation/triangle.vs",  "shaders/tessellation/triangle.fs")
{
	build();
}
void TessTriangleIndexed::build()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points, GL_STATIC_DRAW);

	unsigned int ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glBindVertexArray(0);
}
void TessTriangleIndexed::draw()
{
	glUseProgram(*this);
	glPatchParameteri(GL_PATCH_VERTICES, 3); //comment for tri patch

	glBindVertexArray(vao);
	glDrawElements(GL_PATCHES, 6, GL_UNSIGNED_INT, 0);
}
