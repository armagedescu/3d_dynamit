#include "pch.h"
#include "Particles.h"
#include "config.h"
#include "TextureLoader.h" //need to load textures

float Particle::decreaseLife(float delta)
{
	life -= delta;
	if (life <= 0.f)
		cameradistance = -1.0f;
	return life;
}

void Particle::reinit()
{
	life = 5.0f; // This particle will live 5 seconds.
	pos = glm::vec3(0, 0, -20.0f);

	float spread = 1.5f;
	// Very bad way to generate a random direction;
	// See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
	// combined with some user-controlled parameters (main direction, spread, etc)
	glm::vec3 randomdir = glm::vec3(
		(rand() % 2000 - 1000.0f) / 1000.0f, // [-1.0f, 1.0f)
		(rand() % 2000 - 1000.0f) / 1000.0f,
		(rand() % 2000 - 1000.0f) / 1000.0f
	);

	//xyz = 0 + 1.5 * [-1f, 1f),   10 + 1.5 * [-1f, 1f),   0 + 1.5 * [-1f, 1f)
	glm::vec3 maindir = glm::vec3(0.0f, 10.0f, 0.0f);
	speed = maindir + randomdir * spread;
	color = glm::u8vec4(rand() % 256, rand() % 256, rand() % 256, (rand() % 256) / 3);
	size = (rand() % 1000) / 2000.0f + 0.1f; //[0.1, 0.6)
}

void Particle::updatePosColorData(GLfloat* currentSizeData, GLubyte* currentColorData)
{
	// Fill the GPU buffer
	currentSizeData[0] = pos.x;
	currentSizeData[1] = pos.y;
	currentSizeData[2] = pos.z;

	currentSizeData[3] = size;

	currentColorData[0] = color.r;
	currentColorData[1] = color.g;
	currentColorData[2] = color.b;
	currentColorData[3] = color.a;
}

void Particle::moveParticle(float delta, glm::vec3& cameraPos)
{
	// Simulate simple physics : gravity only, no collisions
	speed += glm::vec3(0.0f, -9.81f, 0.0f) * (float)delta * 0.5f;
	pos   += speed * (float)delta;
	cameradistance = glm::length2(pos - cameraPos);
}

std::vector<GLfloat> Particle::vertices =
{
	 -0.5f, -0.5f, 0.0f,
	  0.5f, -0.5f, 0.0f,
	 -0.5f,  0.5f, 0.0f,
	  0.5f,  0.5f, 0.0f,
};

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

Particles::Particles () : Particles("shaders/particle/particle2.vs", "shaders/particle/particle2.fs") {}
Particles::Particles (const char* vertexPath, const char* fragmentPath) : Shape(vertexPath, fragmentPath)
{
	build ();
}
void Particles::build ()
{
	particlesTexture = LoadTexture("bitmaps/particle.jpg", GL_RGB);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &particlesVertexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, particlesVertexVbo);
	glBufferData(GL_ARRAY_BUFFER, Particle::vertices.size() * sizeof(float), Particle::vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(squareVerticesLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); //3 = xyz   per vertex, false no stride
	glEnableVertexAttribArray(squareVerticesLoc);

	// Buffer orphaning, improve streaming perf. glBufferData (... NULL ... )
	// http://www.opengl.org/wiki/Buffer_Object_Streaming
	glGenBuffers(1, &particlesPositionVbo);
	glBindBuffer(GL_ARRAY_BUFFER, particlesPositionVbo);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); //orphane buffer: bind to null

	glGenBuffers(1, &particlesColorVbo);
	glBindBuffer(GL_ARRAY_BUFFER, particlesColorVbo);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); //orphane buffer: bind to null

	glBindVertexArray(0); //unbind particles vertex array object

	// attributes from shaders
	cameraRightId     = glGetUniformLocation(program, "cameraRight");     // Vertex shader
	cameraUpId        = glGetUniformLocation(program, "cameraUp");        // Vertex shader
	viewProjectionId  = glGetUniformLocation(program, "viewProjection");  // Vertex shader
	particleTextureId = glGetUniformLocation(program, "particleTexture"); // fragment shader

	posSizeData = std::make_unique<GLfloat[]>(MaxParticles * 4);
	colorData   = std::make_unique<GLubyte[]>(MaxParticles * 4);
	particles.resize(MaxParticles);
	lastUsedParticle = 0;
}

void Particles::drawInit(glm::mat4& model, glm::mat4& view, glm::mat4& projection, float deltaTime)
{
	glm::mat4 viewProjectionMatrix = projection * view;

	lastUsedParticle = reuseParticles  (deltaTime);
	particlesCount   = updateParticles (deltaTime, posSizeData.get(), colorData.get());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, particlesTexture);
	glUniform1i(particleTextureId, 0);

	glUniform3f(cameraRightId, view[0][0], view[1][0], view[2][0]);
	glUniform3f(cameraUpId,    view[0][1], view[1][1], view[2][1]);
	glUniformMatrix4fv(viewProjectionId, 1, GL_FALSE, &viewProjectionMatrix[0][0]);

}
void Particles::draw()
{
	glUseProgram(*this);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, particlesPositionVbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particlesCount * sizeof(GLfloat) * 4, posSizeData.get());
	glVertexAttribPointer(centerSizeLoc, 4, GL_FLOAT, GL_FALSE, 0, (void*)0); //4 = xyzw xyz centers, w size /stride:GL_TRUE also works

	glBindBuffer(GL_ARRAY_BUFFER, particlesColorVbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particlesCount * sizeof(GLubyte) * 4, colorData.get());
	glVertexAttribPointer(colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, (void*)0); //4 = rgba  /stride:GL_FALSE does not work, why???

	glEnableVertexAttribArray(squareVerticesLoc);
	glEnableVertexAttribArray(centerSizeLoc);
	glEnableVertexAttribArray(colorLoc);

	// These functions are specific to glDrawArrays*Instanced*.
	// The first parameter is the attribute buffer we're talking about.
	// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
	// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
	glVertexAttribDivisor(squareVerticesLoc, 0); // always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(centerSizeLoc, 1); // one per quad (its center)        -> 1
	glVertexAttribDivisor(colorLoc, 1); // one per quad                     -> 1

	// /* glDrawArraysInstanced faster than */ for (particles) glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particlesCount);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

}
int Particles::reuseParticles(double deltaTime)
{
	using std::vector;
	// Generate 10 particles/ms, limit to 16 ms (60 fps),
	// newparticles will be huge and the next frame even longer.
	if (deltaTime > 0.016f) deltaTime = 0.016f;
	int newparticles = (int)(deltaTime * 10000.0);
	auto isalive = [](const Particle& p)->bool {return p.life < 0; };

	vector<Particle>::iterator itc = particles.begin() + lastUsedParticle;
	for (int i = 0; i < newparticles; i++)
	{
		//find unused particles
		vector<Particle>::iterator p = std::find_if(itc, particles.end(), isalive);
		if (p >= particles.end())
		{
			p = std::find_if(particles.begin(), itc, isalive);
			if (p >= itc) break;
		}
		lastUsedParticle = p - particles.begin();
		p->reinit();
	}
	return lastUsedParticle;
}

int Particles::updateParticles(double deltaTime, GLfloat* posSizeData, GLubyte* colorData)
{
	int particlesCount = 0;
	for (auto& p : particles)
	{
		if (p.life < 0.0f) continue;

		if (p.decreaseLife(deltaTime) > 0.0f)
		{
			p.moveParticle(deltaTime, config::camera.position);
			p.updatePosColorData(posSizeData + 4ll * particlesCount, colorData + 4ll * particlesCount);
		}
		particlesCount++;
	}
	//start draw far particles, then neear particles, move unused particles to the end
	std::sort(particles.begin(), particles.end(), [](Particle& a, Particle& b) { return a.cameradistance > b.cameradistance; });

	return particlesCount;
}
