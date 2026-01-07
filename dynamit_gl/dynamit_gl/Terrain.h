#pragma once
#include "Shape.h"
#include <vector>
#include <glm/glm.hpp>

class Terrain: public Shape
{
	const wchar_t* terrainImgPath;

	int vertsize = 3;
	int normsize = 3;
	int stridesize = vertsize + normsize;

	std::vector<float> vertexes;
	int trianglesCount = -1;
	int vertexesCount  = -1;

	unsigned int vertColorLocation = 2; // 	same as, but faster: vertColorLocation = glGetAttribLocation(*this, "vertColor");

	unsigned int modelLocationId;
	unsigned int viewLocationId;
	unsigned int projectionLocationId;

	glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);

public:
	static const wchar_t* defTerrainImgPath;
	bool doubleCoated = true;
	unsigned int vao;

	Terrain(const wchar_t* heigthsMapPath);
	Terrain(const wchar_t* heigthsMapPath, const char* vertexPath, const char* fragmentPath);
	Terrain(const char* vertexPath, const char* fragmentPath);
	void build();

	void drawInit();
	void drawInit(glm::mat4& model, glm::mat4& view, glm::mat4& projection, const glm::vec4& color);
	void draw();
	int fillHeightMapBuffer(float size, float h);
};