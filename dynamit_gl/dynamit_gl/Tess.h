#pragma once
#include <GL/glew.h>
#include "Shape.h"
#include <glm/glm.hpp> //basic glm math functions

class Tess : public Shape
{
public:
	Tess();
	Tess(const char* controlShader, const char* evaluationShader, const char* vertexShader, const char* fragmentShader);
};

class TessQuad: public Tess
{

public:
	static GLfloat points[];
	unsigned int vao = 0xffffffff;

	TessQuad();
	void build();
	void draw();
};

class TessTriangle : public Tess
{
public:
	static GLfloat points[];
	unsigned int vao = 0xffffffff;

	TessTriangle();
	void build();
	void draw();
};
class TessTriangleRainbow : public Tess
{
public:
	static GLfloat points[];
	unsigned int vao = 0xffffffff;

	TessTriangleRainbow();
	void build();
	void draw();
};
class TessTriangleIndexed : public Tess
{
public:
	static GLfloat points[];
	static const unsigned int indices[];
	unsigned int vao = 0xffffffff;

	TessTriangleIndexed();
	void build();
	void draw();
};
