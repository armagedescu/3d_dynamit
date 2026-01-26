#pragma once
#include "TriangleRainbow.h"
#include <glm/glm.hpp> //basic glm math functions

namespace singleshape
{

class TriangleRainbowWithCamera : public TriangleRainbow
{
	unsigned int modelLocationId;
	unsigned int viewLocationId;
	unsigned int projectionLocationId;
public:
	TriangleRainbowWithCamera();
	TriangleRainbowWithCamera(const char* vertexPath, const char* fragmentPath);
	void build();
	void drawInit(glm::mat4& model, glm::mat4& view, glm::mat4& projection);
};

}