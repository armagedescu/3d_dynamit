#include "enabler.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Program.h>
#include <time.h>

#include <glm/glm.hpp>                  //basic glm math functions
#include <glm/gtc/matrix_transform.hpp> //matrix functions
#include <glm/gtc/type_ptr.hpp>         //convert glm types to opengl types

#include <TextureLoader.h> //need to load textures
#include <Cube.h>
#include <Camera.h>
#include <callbacks.h>
#include <config.h>
#include <Triangle.h>
#include <TriangleRainbow.h>
#include <TriangleRainbowWithCamera.h>
#include <RectangleBlink.h>
#include <Square.h>
#include <Terrain.h>
#include <FrameBuffer.h>

#include <stb_image.h>
#include <util.h>

using namespace std;
using namespace config;

int main_camera ()
{
	using singleshape::TriangleRainbowWithCamera;
	using singleshape::Square;
	using singleshape::CubeSet;

	srand(time(NULL));
	GLFWwindow* window = openglWindowInit();
	if (!window)
		return -1;
	scope_guard glfwTerminator2(glfwTerminate);


	//TODO stbi to flip or not to flip, this is the question?!?!?!
	stbi_set_flip_vertically_on_load(true);
	unsigned int texture0crate    = LoadTexture("bitmaps/crate.jpg",    GL_RGB);
	unsigned int texture1airplane = LoadTexture("bitmaps/airplane.png", GL_RGBA);

	TriangleRainbowWithCamera triangleRainbowWithCamera;
	Square                    square;

	FrameBuffer<Terrain> terrainFrameBuffer("shaders/framebuffer/terrain.framebuffer.vs", "shaders/framebuffer/terrain.framebuffer.fs");
	Terrain terrain("shaders/terrain.vs", "shaders/terrain.fs");

	CubeSet cubeSet;
	cubeSet.init(3);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	////glFrontFace(GL_CW);

	glm::vec3 pos(0.0f, 0.0f, 0.0f);
	//glm::vec3 pos(0.0f, 0.0f, 3.0f);
	//GAME LOOP
	while (!glfwWindowShouldClose(window))
	{
		processInputs(window);

		glm::mat4 model = glm::translate(glm::mat4(1.0), glm::vec3(0.1f, -0.1f, 0.0f));
		glm::mat4 view = camera.view();
		glm::mat4 projection = camera.perspective();

		triangleRainbowWithCamera.drawInit(model, view, projection);

		model = glm::translate(glm::mat4(1.0), pos);
		model = glm::rotate(model, currentTime, glm::vec3(0.0f, 1.0f, 0.0f));
		terrainFrameBuffer.drawInit(model, view, projection);
		terrainFrameBuffer.draw(
			[]
			{
				glEnable(GL_CULL_FACE);
				glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
				glClearColor(0.f, 0.f, 1.f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			});

		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		glClearColor(0.5, 0, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		switch (currentShape)
		{
		case DRAW_1: // DRAW_RAINBOW_TRIANGLE_WITH_CAMERA - DRAW_RAINBOW_TRIANGLE:
			triangleRainbowWithCamera.draw(); //TODO: does not work
			break;
		case DRAW_2: // DRAW_SQUARE - DRAW_RAINBOW_TRIANGLE:
			square.drawInit(texture0crate, currentTime, { 0.5f, -0.5f, 0.0f });
			square.draw();
			square.drawInit(texture1airplane, currentTime, { -0.5f, -0.5f, 0.0f });
			square.draw();
			square.drawInit(terrainFrameBuffer.getTexture(), currentTime, { 0.0f, 0.0f, 0.9f }); //TODO: partially probably does not work
			square.draw();
			break;
		case DRAW_3: // DRAW_CUBES - DRAW_RAINBOW_TRIANGLE:
			cubeSet.drawInit(texture0crate, view, projection);
			cubeSet.draw();
			break;
		case DRAW_4: // DRAW_7 - DRAW_RAINBOW_TRIANGLE:
			cubeSet.drawInit(terrainFrameBuffer.getTexture(), view, projection);
			cubeSet.draw();
			break;
		case DRAW_5: // DRAW_8 - DRAW_RAINBOW_TRIANGLE:
			triangleRainbowWithCamera.draw();

			square.drawInit(texture0crate, currentTime, { 0.5f, -0.5f, 0.0f });
			square.draw();
			square.drawInit(terrainFrameBuffer.getTexture(), currentTime, { -0.5f, -0.5f, 0.0f });
			square.draw();
			break;
		case DRAW_6: // DRAW_9 - DRAW_RAINBOW_TRIANGLE:
			square.drawInit(texture0crate, currentTime, { 0.5f, -0.5f, 0.0f });
			square.draw();
			square.drawInit(terrainFrameBuffer.getTexture(), currentTime, { -0.5f, -0.5f, 0.0f });
			square.draw();

			//glEnable(GL_CULL_FACE);
			terrain.drawInit(model, view, projection, glm::vec4(0, 1, 0, 1));
			terrain.draw();

			break;
		}

		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	//cleanup
	glfwTerminate();
	return 0;
}
#include "enabler.h"
#ifdef __MAIN_CAMERA_CPP__
int main() { return main_camera(); }
#endif