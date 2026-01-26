#pragma once
#ifndef __CONSTANTE_H__
#define __CONSTANTE_H__
#define _USE_MATH_DEFINES
#include <math.h>

const double RADGRAD = M_PI / 180;
const int MAXH = 150;
const int MAXS = 150;
const int MAXH2 = 150 - MAXH % 2;
const int MAXS2 = 150 - MAXS % 2;
const int MAXH4 = 150 - MAXH % 4;
const int MAXS4 = 150 - MAXS % 4;
const int MAXH8 = 150 - MAXH % 8;
const int MAXS8 = 150 - MAXS % 8;
const int MINH2 = 10;
const int MINS2 = 10;
const int MINH4 = 12;
const int MINS4 = 12;
const int MINH8 = 16;
const int MINS8 = 16;
#endif