#pragma once
#ifndef __CYLLINDER_H__
#define __CYLLINDER_H__
#include <math.h>
#include "constante.h"

class cyllinder0
{
protected:
	int nh;
	int ns;
	double c[MAXH][MAXS][3];
public:
	cyllinder0(int, int);
	void draw();
};


#endif