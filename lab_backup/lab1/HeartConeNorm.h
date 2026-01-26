#pragma once
#ifndef __HEART_CONE_NORMAL_H___
#define __HEART_CONE_NORMAL_H___
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "constante.h"
#include "MemoryHelpers.h"

class HeartConeNorm
{
protected:
	int nh;
	int ns;
	MultiArray<double, 3> points;
	MultiArray<double, 2> norms;
	static GLdouble *norm(const GLdouble p1[3], const GLdouble p2[3], const GLdouble p3[3]);
public:
	HeartConeNorm(int, int);
	void calculate();
	void draw();
};
#endif