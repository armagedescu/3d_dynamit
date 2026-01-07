#pragma once
#include "Shader.h"
#include <map>
#include <iostream>

class Program
{
	friend std::ostream& operator <<(std::ostream& os, Program& sh);
public:
	bool success = false;
	unsigned int id = 0xffffffff;
	operator unsigned int() { return id; }
	operator bool() { return success; }

	Program();
	~Program();
	Program(const char* vertexPath, const char* fragmentPath);
	Program& buildVertexFragmentShaders(const char* vertexPath, const char* fragmentPath);
	Program& addShader(const char* shaderPath, unsigned int shaderType);
	bool build();
	std::string glGetInfoLog();
	//Get shader by type
	Shader& operator[] (int shaderType) { return shaders[shaderType]; }
private:
	bool haveLinkErrors();
	bool reportLinkErrors();
	bool precheck();
	std::map<unsigned int, Shader> shaders;
};
std::ostream& operator <<(std::ostream& os, Program& sh);


