#pragma once
#include "Shape.h"
#include <glm/glm.hpp>                  //basic glm math functions

namespace shapes
{
class Cube
{
public:
	static const float vertices[];
	unsigned int vao;
	void build();
	void draw(glm::mat4& model);
};
}

class CubeScene : public Shape
{
public:
	unsigned int modelLocationId;
	unsigned int lightSpaceMatrixLocationId;
	unsigned int projectionLocationId;
	unsigned int viewLocationId;
	unsigned int viewPosLocationId;
	unsigned int lightPosLocationId;

	static const float vertices[];
	unsigned int vao; //vertex array object
	unsigned int woodTexture;
	shapes::Cube cube;

	CubeScene();
	CubeScene(const char* vertexPath, const char* fragmentPath);

	void drawInit(unsigned int depthTexture, glm::mat4& projection, glm::mat4& view, glm::vec3& viewPos, glm::vec3& lightPos, glm::mat4& lightSpaceMatrix);
	void drawInit(glm::mat4& lightSpaceMatrix);
	void draw();
	void build();
	void postBuild();
};