#include "pch.h"
#include "Shader.h"
#include <GL\glew.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

std::map<unsigned int, std::vector<std::string>> Shader::shaderdesc =
{
	{GL_VERTEX_SHADER,          {"Vertex",                 "GL_VERTEX_SHADER"}},
	{GL_FRAGMENT_SHADER,        {"Fragment",               "GL_FRAGMENT_SHADER"}},
	{GL_TESS_CONTROL_SHADER,    {"Tesselation Control",    "GL_TESS_CONTROL_SHADER"}},
	{GL_TESS_EVALUATION_SHADER, {"Tesselation Evaluation", "GL_TESS_EVALUATION_SHADER"}},
	{GL_GEOMETRY_SHADER,        {"Geometry",               "GL_GEOMETRY_SHADER"}}
};
#include <filesystem>
Shader::Shader(const char* shaderSrc, unsigned int shaderType): type(shaderType)
{
	if (std::filesystem::exists(shaderSrc)) // if shaderSrc file path exists, then it is a file no matter what isfile says
		loadFromFile(shaderSrc);
	else
		shaderCode = shaderSrc;


	build();
}

std::string Shader::glGetShaderInfoLog()
{
	char infoLog[1024];
	::glGetShaderInfoLog(id, 1024, 0, infoLog);
	return std::string(infoLog);
}

unsigned int Shader::build()
{
	using std::string;

	this->success = false;

	id = glCreateShader(type);
	const char* shader = shaderCode.c_str();

	glShaderSource(id, 1, &shader, NULL);
	glCompileShader(id);
	if (haveCompileErrors())
	{
		glDeleteShader(id);
		id = 0xffffffff;
	}

	return id;
}

Shader& Shader::loadFromFile(const char* shaderPath)
{
	filePath = shaderPath;

	using std::cout;
	using std::endl;
	using std::string;
	using std::ifstream;
	using std::stringstream;

	ifstream shaderFile;
	shaderFile.exceptions(ifstream::failbit | ifstream::badbit);
	try
	{
		shaderFile.open(filePath);
		stringstream shaderStream;
		shaderStream << shaderFile.rdbuf();
		shaderFile.close();
		shaderCode = shaderStream.str();
	}
	catch (ifstream::failure e)
	{
		cout << "Shader file not successfully read: " << e.what() << endl << "path: " << filePath << endl;
		throw e;
	}

	return *this;
}

//return true if errors
bool Shader::haveCompileErrors()
{
	using std::cout;
	using std::endl;
	int success;
	//check what error was for individual shaders being compiled
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if (success)
	{
		this->success = true;
		return false; //have compile error? no
	}

	std::string desc = "Unknown Type";
	auto it = shaderdesc.find(type);
	if (it != shaderdesc.end()) desc = it->second[0];

	cout << "------  Error Source Info: " << endl;
	cout << "Shader File Path: " << filePath << endl;
	cout << "Shader Source:\n" << shaderCode << endl;
	cout << "------ " << desc << " Shader Compile Error:" << endl;
	cout << glGetShaderInfoLog() << endl;
	cout << "-------------------------------------" << endl;
	return true;

}
