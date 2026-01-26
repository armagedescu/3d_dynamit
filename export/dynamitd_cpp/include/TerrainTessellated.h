#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "tess.h"
class TerrainTessellated : public Tess
{
	const wchar_t* terrainImgPath;

	int vertsize = 3, normsize = 3;
	int stridesize = vertsize + normsize;

	std::vector<float> vertexes;
	std::vector<int>   indexes;
	int trianglesCount = -1, vertexesCount = -1;

	// harcoded location in shader: same as, but faster: = glGetAttribLocation(progid, "vertColor");
	const unsigned int vertLocation = 0, normLocation = 1, vertColorLocation = 2;

	unsigned int modelLocationId;
	unsigned int viewLocationId;
	unsigned int projectionLocationId;

	glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
public:
	static const wchar_t* defTerrainImgPath;
	bool doubleCoated = true;

	unsigned int vao;
	//unsigned int ebo;

	TerrainTessellated(const wchar_t* heigthsMapPath);
	TerrainTessellated(const wchar_t* heigthsMapPath, const char* controlShader, const char* evaluationShader, const char* vertexPath, const char* fragmentPath);
	TerrainTessellated(const char* controlShader, const char* evaluationShader, const char* vertexPath, const char* fragmentPath);
	void build();

	void drawInit();
	void drawInit(glm::mat4& model, glm::mat4& view, glm::mat4& projection, const glm::vec4& color);
	void draw();
	int fillHeightMapBuffer(float size, float h);
};

