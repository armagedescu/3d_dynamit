#pragma once
#ifndef __ANGLES_H__
#define __ANGLES_H__
#define _USE_MATH_DEFINES
#include <math.h>
const double RAD2GRAD = 180. / M_PI;
const double GRAD2RAD = M_PI / 180.;

class Radiani
{
	double rads;
public:
	Radiani() : rads(0){}
	Radiani(double rad):rads(rad){}
	double operator = (const double rad)
	{
		rads = rad;
		return rads;
	}
	operator double () {return rads;}
	double rad(){return rads;}
	double grad(){return RAD2GRAD * rads;}
	double pirad(){return rads / M_PI;}

};
class Grade
{
	double grade;
public:
	Grade() : grade(0){}
	Grade(double grad):grade(grad){}
	double operator = (const double grad)
	{
		grade = grad;
		return grade;
	}
	operator double () {return grade;}
	double rad(){return grade;}
	double grad(){return GRAD2RAD * grade;}
	double pirad(){return grade / 180.;}

};
class PiRadiani
{
	double pirads;
public:
	PiRadiani() : pirads(0){}
	PiRadiani(double rad): pirads(rad){}
	double operator = (const double rad)
	{
		pirads = rad;
		return pirads;
	}
	operator double () {return pirads;}
	double rad(){return pirads * M_PI;}
	double grad(){return 180. * pirads;}
	double pirad(){return pirads;}

};
#endif