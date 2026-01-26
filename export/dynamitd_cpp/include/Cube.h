#pragma once
#include <cmath>
#include "Shape.h"
#include <glm/glm.hpp>
#include <vector>

namespace singleshape
{

class Cube
{
public:
	glm::vec3 pos;
	Cube() = delete;
	Cube(const glm::vec3& _pos) : pos(_pos) {}
};

class CubeSet: public Shape
{
	static glm::vec3 cubePositions[];
	static float textureVertices[];
	unsigned int vao;
	unsigned int texture0LocationId;
	//unsigned int texture1LocationId;
	unsigned int modelLocationId;
	unsigned int viewLocationId;
	unsigned int projectionLocationId;
public:
	std::vector<Cube> cubes;
	CubeSet();
	CubeSet(const char* vertexPath, const char* fragmentPath);
	void build();
	void init(int n);
	void drawInit(unsigned int texture, glm::mat4& view, glm::mat4& projection);
	void draw();
};


}