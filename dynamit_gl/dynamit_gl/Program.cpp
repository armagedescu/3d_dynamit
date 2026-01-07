#include "pch.h"
#include "Program.h"
#include <GL\glew.h>
#include <iostream>
#include <algorithm>

Program::Program() : id(glCreateProgram()) {}
Program::~Program()
{
	glDeleteProgram(id);
}
Program::Program(const char* vertexPath, const char* fragmentPath) : Program()
{
	buildVertexFragmentShaders(vertexPath, fragmentPath);
}
Program& Program::buildVertexFragmentShaders(const char* vertexPath, const char* fragmentPath)
{
	addShader(vertexPath,   GL_VERTEX_SHADER);
	addShader(fragmentPath, GL_FRAGMENT_SHADER);
	build();
	return *this;
}

Program& Program::addShader(const char* shaderPath, unsigned int shaderType)
{
	//if (!shaderPath)  shaderPath = "";
	//if (!*shaderPath) isfile     = false;
	Shader shader(shaderPath, shaderType);
	shaders[shader.type] = shader;
	return *this;
}

//true if success
bool Program::precheck()
{
	if (shaders.size() == 0)
	{
		std::cout << "Error: nothing to attach" << std::endl;
		return false;
	}
	bool haveFailedShaders = false;
	for (auto& s : shaders)
	{
		Shader& sh = s.second;
		if (!sh.success)
		{
			std::cout << "Error: having failed shaders: " << sh.shaderdesc[sh.type][1] << std::endl;
			haveFailedShaders = true;
		}
	}
	return !haveFailedShaders; //no shader failed: success = true
}

bool Program::build()
{
	success = false;
	if (!precheck()) return false;

	for (auto& shader : shaders)
		glAttachShader(id, shader.second);

	glLinkProgram(id);

	for (auto& shader : shaders)
		glDeleteShader(shader.second);
	success = !reportLinkErrors();
	return success;
}

std::string Program::glGetInfoLog()
{
	char infoLog[1024];
	glGetProgramInfoLog(id, 1024, 0, infoLog);
	return std::string(infoLog);
}

bool Program::haveLinkErrors()
{
	int linkSucceeded = 0;
	glGetProgramiv(id, GL_LINK_STATUS, &linkSucceeded);
	return !linkSucceeded;
}
bool Program::reportLinkErrors()
{
	int linkSucceeded = 0;
	glGetProgramiv(id, GL_LINK_STATUS, &linkSucceeded);
	if (haveLinkErrors())
	{
		std::cout << "Program Linking Error: " << glGetInfoLog() << std::endl;
		return true;
	}
	return false;
}

std::ostream& operator<<(std::ostream& os, Program& sh)
{
	if (sh.haveLinkErrors())
		os << "Shaders linker error:" << std::endl;
	return os << sh.glGetInfoLog();
}
