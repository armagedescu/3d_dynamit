#pragma once
#include <GL\glew.h>

#include <string>
#include <vector>
#include <map>
struct Shader
{
	bool success = false;
	unsigned int type = 0xffffffff;
	std::string shaderCode;
	std::string filePath;
	unsigned int id = 0xffffffff;
	static std::map<unsigned int, std::vector<std::string>> shaderdesc;

	unsigned int build();
	Shader& loadFromFile(const char* shaderPath);
	bool haveCompileErrors();
	Shader() {}
	// shaderSrc can be a file path or the actual shader code as string
	// if shaderSrc file path exists, then isfile is redundant, this it is a file no matter what isfile says
	Shader(const char* shaderSrc, unsigned int shaderType);
	std::string glGetShaderInfoLog();
	operator unsigned int(){return id;}
};
