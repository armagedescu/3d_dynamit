#pragma once
#ifndef __LAB_1_HEART_CONE_H__
#define __LAB_1_HEART_CONE_H__
#include "constante.h"
#include "MemoryHelpers.h"
class HeartCone
{
protected:
	int nh;
	int ns;
	void calculate1();
	void calculate2();
	void calculate3();
	points3 points;
public:
	HeartCone(int, int);
	void draw();
	void draw1();
	void draw2();
};

#endif