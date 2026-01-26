#pragma once
#ifndef __LAB_1_HEART_CYLLINDER_H__
#define __LAB_1_HEART_CYLLINDER_H__
#include "constante.h"

class HeartCyllinder
{
protected:
	int nh;
	int ns;
	double c[MAXH][MAXS][3];
public:
	HeartCyllinder(int, int);
	void draw();
};

#endif