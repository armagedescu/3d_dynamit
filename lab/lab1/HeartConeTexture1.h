#pragma once
#ifndef __HEART_CONE_TEXTURE_1_H___
#define __HEART_CONE_TEXTURE_1_H___
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "constante.h"
#include "MemoryHelpers.h"

class HeartConeTexture1
{
protected:
	int nh;
	int ns;
	MultiArray<double, 3> points;



	static const int  mx = 1024;
	static const int  ny = 1024;
	unsigned char p[mx][ny][3];
	GLuint tex_phobos;
	GLuint tex_generated;

	void draw(GLuint texture);

	//MultiArray<double, 2> norms;
public:
	HeartConeTexture1(int, int);
	void calculate();
	void draw();
	void drawInverse();
};
#endif