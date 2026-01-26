#pragma once
#include<iostream>
#include "Program.h"
class Shape
{
public:
	Program program;
	operator unsigned int() { return program.operator unsigned int(); }
	operator bool() { return program.operator bool(); }
	Shape() {} //manual
	Shape(const char* vertexPath, const char* fragmentPath) : program(vertexPath, fragmentPath) {}
};
inline std::ostream& operator <<(std::ostream& os, Shape& sh) 
{
	return operator << (os, sh.program);
}