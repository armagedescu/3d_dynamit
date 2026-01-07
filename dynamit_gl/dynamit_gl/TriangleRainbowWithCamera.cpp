#include "pch.h"
#include <GL/glew.h>
#include "TriangleRainbowWithCamera.h"
#include <glm/glm.hpp> //basic glm math functions
//#include <glm/gtc/matrix_transform.hpp> //matrix functions
#include <glm/gtc/type_ptr.hpp> //convert glm types to opengl types
#include "config.h"

namespace singleshape
{

TriangleRainbowWithCamera::TriangleRainbowWithCamera()
	: TriangleRainbowWithCamera("shaders/triangleGradientWithCamera.vs", "shaders/triangleGradientWithCamera.fs")
{
}

TriangleRainbowWithCamera::TriangleRainbowWithCamera(const char* vertexPath, const char* fragmentPath)
	: TriangleRainbow(vertexPath, fragmentPath)
{
	build();
}

void TriangleRainbowWithCamera::build()
{
	modelLocationId      = glGetUniformLocation(*this, "model");
	viewLocationId       = glGetUniformLocation(*this, "view");
	projectionLocationId = glGetUniformLocation(*this, "projection");
}

void TriangleRainbowWithCamera::drawInit(glm::mat4& model, glm::mat4& view, glm::mat4& projection)
{
	glUseProgram(*this);
	glUniformMatrix4fv(modelLocationId,      1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLocationId,       1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLocationId, 1, GL_FALSE, glm::value_ptr(projection));
}

}