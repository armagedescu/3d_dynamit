#pragma once
#include "Shape.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <vector>
#include <algorithm>
#include <memory>

// CPU representation of a particle
struct Particle
{
	static std::vector<GLfloat> vertices;

	glm::vec3 pos, speed;
	glm::uvec4 color;
	float size, angle;
	float life = -1.0f; // if <0 : dead and unused.
	float cameradistance = -1.0f; // *Squared* distance to the camera. if dead : -1.0f
	float decreaseLife(float delta);
	void reinit();
	void updatePosColorData(GLfloat* currentSizeData, GLubyte* currentColorData);
	void moveParticle(float delta, glm::vec3& cameraPos);
};

class Particles : public Shape
{
	const long long MaxParticles = 100000;
	int lastUsedParticle = 0;
	std::vector<Particle> particles;
	int particlesCount; // usualy a subset of particles, not the same as particles.size()

	GLuint cameraRightId, cameraUpId, viewProjectionId; // vertex shader
	GLuint particleTextureId; // fragment shader
	GLuint squareVerticesLoc = 0, centerSizeLoc = 1, colorLoc = 2; //[locations = ] from vertex shader

	std::unique_ptr<GLfloat[]> posSizeData;
	std::unique_ptr<GLubyte[]> colorData;

	GLuint vao;
	GLuint particlesVertexVbo, particlesPositionVbo, particlesColorVbo;
	GLuint particlesTexture;

	int reuseParticles(double delta);
	int updateParticles(double delta, GLfloat* posSizeData, GLubyte* colorData);

public:

	Particles();
	Particles(const char* vertexPath, const char* fragmentPath);

	void drawInit(glm::mat4& model, glm::mat4& view, glm::mat4& projection, float dt);
	void draw();
	void build();

};
//// Cleanup VBO and shader
//glDeleteBuffers(1, &particlesColorVbo);
//glDeleteBuffers(1, &particlesPositionVbo);
//glDeleteBuffers(1, &particlesVertexVbo);
//glDeleteProgram(program);
//glDeleteTextures(1, &particlesTexture);
//glDeleteVertexArrays(1, &particlesVao);