#include "enabler.h"

#include <GL/glew.h>
#include "config.h"
#include <stdlib.h>
#include <cmath>
#include <iostream>
#include <time.h>

#include <glm/glm.hpp> //basic glm math functions
#include <glm/gtc/matrix_transform.hpp> //matrix functions
#include <glm/gtc/type_ptr.hpp> //convert glm types to opengl types

#include <Camera.h>
#include <callbacks.h>
#include <TerrainIndexed.h>
#include <Terrain.h>
#include <TerrainTessellated.h>
#include <Tess.h>

using namespace std;

//terrain
int main_terrain_bitmap_double_coated_vineet()
{
	using namespace std;
	srand(time(NULL));
	config::openGlVersionMaj = 4;
	//config::openGlVersionMin = 3;
	config::openGlVersionMin = 6;
	GLFWwindow* window = openglWindowInit();
	if (!window)
		return -1;
	cout << glGetString(GL_VERSION) << endl;

	TerrainIndexed terrainIndexed(TerrainIndexed::defTerrainImgPath);// "shaders/terrainIndexed.vs", "shaders/terrainIndexed.fs");
	TerrainTessellated terrainTesselated(TerrainTessellated::defTerrainImgPath);

	TessQuad  tessQuad;
	TessTriangle   tessTri;
	TessTriangleIndexed   tessTriIndexed;
	TessTriangleRainbow   tessTriRainbow;

	if (!terrainIndexed)
	{
		char infoLog[512];
		glGetProgramInfoLog(terrainIndexed, 512, NULL, infoLog);
		std::cout << "shaders failed\n" << infoLog << std::endl;
		return -1;
	}
	if (!tessTriRainbow)
	{
		char infoLog[512];
		glGetProgramInfoLog(terrainIndexed, 512, NULL, infoLog);
		std::cout << "shaders failed\n" << infoLog << std::endl;
		return -1;
	}
	if (!tessQuad)
	{
		char infoLog[512];
		glGetProgramInfoLog(tessQuad, 512, NULL, infoLog);
		std::cout << "shaders failed\n" << infoLog << std::endl;
		return -1;
	}

	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glm::vec3 pos(0.0f, 0.0f, 0.0f);
	// position of the tesselated terrain 
	//glm::vec3 position( 6.0f, -2.0f, 0.0f);

	int currentShape = 0;
	int saveDraw = currentDraw;
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	while (!glfwWindowShouldClose(window))
	{
		using config::camera;
		processInputs(window);
		////////////////////////////////////////////////
		// terrain
		if (keyPressed)
		{
			keyPressed = false;
			GLint polygonMode[2]   = { 0, 0 };
			GLint frontfaceMode[1] = {};
			const int shapes = 6;
			switch (currentDraw)
			{
			case DRAW_1:
				if (keyMod & GLFW_MOD_ALT)currentShape--;
				else currentShape++;
				if (currentShape < 0) currentShape = shapes - 1;
				currentShape %= shapes;
				break;
			case DRAW_2:
				glGetIntegerv(GL_POLYGON_MODE, polygonMode);
				glPolygonMode(GL_FRONT_AND_BACK, polygonMode[0] == GL_FILL ? GL_LINE : GL_FILL);
				break;
			case DRAW_3:
				if (glIsEnabled(GL_CULL_FACE)) glDisable(GL_CULL_FACE);
				else glEnable(GL_CULL_FACE);
				break;
			case DRAW_4:
				glGetIntegerv(GL_FRONT_FACE, frontfaceMode);
				glFrontFace(frontfaceMode[0] == GL_CCW ? GL_CW : GL_CCW);
			}
		}

		glm::mat4 model = glm::mat4(1.0);
		model = glm::translate(model, pos);
		//model = glm::rotate(model, currentTime, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 view = camera.view();
		glm::mat4 projection = camera.perspective();

		glClearColor(0.f, 0.f, 1.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		terrainIndexed.drawInit(model, view, projection, glm::vec4(1, 0, 0, 1));
		terrainTesselated.drawInit(model, view, projection, glm::vec4(1, 0, 0, 1));

		//switch (currentShape)
		switch (currentDraw)
		{
		case 0:
			tessTriRainbow.draw();
			break;
		case 1:
			tessTriIndexed.draw();
			break;
		case 2:
			tessQuad.draw();
			break;
		case 3:
			tessTri.draw();
			break;
		case 4:
			cout << "Drawing Terrain Indexed" << endl;
			terrainIndexed.draw();
			break;
		case 5:
			terrainTesselated.draw();
			break;
		}


		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

#include "enabler.h"
#ifdef __MAIN_TERRAIN_BITMAP_DOUBLE_COATED_VINEET_CPP__
int main(){ return main_terrain_bitmap_double_coated_vineet(); }
#endif