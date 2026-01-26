#pragma once
#ifndef __HEART_CONE_NORMAL_METHOD_1_H___
#define __HEART_CONE_NORMAL_METHOD_1_H___
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "constante.h"
#include "MemoryHelpers.h"

class HeartConeNormMethod1
{
protected:
	int nh;
	int ns;
	MultiArray<double, 3> points;
	MultiArray<double, 2> norms;
public:
	HeartConeNormMethod1(int, int);
	void calculate();
	void draw();
};
#endif