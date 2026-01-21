#include "enabler.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

#include <Config.h>
#include <Particles.h>
#include <Terrain.h>
#include <TerrainIndexed.h>
#include <GoogleMapTerrainIndexed.h>
#include <TerrainIndexDraw.h>
#include <GoogleMapTerrain.h>

using namespace glm;

int main_particles_terrain()
{
	GLFWwindow* window = openglWindowInit();
	if (!window)
		return -1;
	Particles particles;
	//TerrainIndexed terrainIndexed(L"bitmaps/heightmap.bmp");
	TerrainIndexDraw terrainIndexed(L"bitmaps/heightmap.bmp");
	Terrain terrain("shaders/terrain.vs", "shaders/terrain.fs");
	GoogleMapTerrainIndexed rudiTerrain(L"bitmaps/rudi.png");
	GoogleMapTerrainIndexed tipovaTerrain(L"bitmaps/tipova.png");
	GoogleMapTerrainIndexed butuceniTerrain(L"bitmaps/butuceni.png");
	GoogleMapTerrainIndexed craterTerrain(L"bitmaps/craterArizona.png");
	GoogleMapTerrainIndexed craterTerrain2(L"bitmaps/craterArizona2.png");

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS); // Accept fragment if it closer to the camera than the former one
	bool pause = true;
	glm::vec3 pos(0.0f, 0.0f, 0.0f);
	std::cout << "Press Ctrl+Alt to toggle pause" << std::endl;
	std::cout << "    Now pause is " << (pause ? "On": "Off") << std::endl;

	
	while (!glfwWindowShouldClose(window))
	{
		//break;
		using config::camera;
		using config::windowWidth;
		using config::windowHeight;
		processInputs(window);

		glm::mat4 model = glm::translate(glm::mat4(1.0), pos), model_pause = glm::translate(glm::mat4(1.0), pos);
		model = glm::rotate(model, lastFrame, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 view = camera.view();
		glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)windowWidth / windowHeight, 0.1f, 100.0f);


		glClearColor(0.f, 0.f, 0.4f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		if ( checkAll (keyMod, MOD_CTRLALT) && !checkAll (keyModAlt, MOD_CTRLALT) ) 
			pause = !pause;
		if (pause) deltaTime = 0;

		switch (currentDraw)
		{
		case DRAW_1:
			particles.drawInit(model, view, projection, deltaTime);
			particles.draw();
			break;
		case DRAW_2:
			terrainIndexed.drawInit(model, view, projection, glm::vec4(1, 0, 0, 1));
			terrainIndexed.draw();
			break;
		case DRAW_3:
			particles.drawInit(model, view, projection, deltaTime);
			particles.draw();
			terrain.drawInit(model, view, projection, glm::vec4(1, 0, 0, 1));
			terrain.draw();
			break;
		case DRAW_4:
			rudiTerrain.drawInit(model, view, projection, glm::vec4(1, 0, 0, 1));
			rudiTerrain.draw();
			break;
		case DRAW_5:
			tipovaTerrain.drawInit(model, view, projection, glm::vec4(1, 0, 0, 1));
			tipovaTerrain.draw();
			break;
		case DRAW_6:
			butuceniTerrain.drawInit(model, view, projection, glm::vec4(1, 0, 0, 1));
			butuceniTerrain.draw();
			break;
		case DRAW_7:
			craterTerrain.drawInit(model, view, projection, glm::vec4(1, 0, 0, 1));
			craterTerrain.draw();
			break;
		case DRAW_8:
			craterTerrain2.drawInit(model_pause, view, projection, glm::vec4(1, 0, 0, 1));
			craterTerrain2.draw();
			break;
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
#include "enabler.h"
#ifdef __MAIN_PARTICLES_TERRAIN_CPP__
int main() { return main_particles_terrain(); }
#endif