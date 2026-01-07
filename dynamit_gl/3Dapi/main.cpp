#include "enabler.h"
#ifdef  __MAIN_MAIN__
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Program.h>
#include <time.h>
#include <TextureLoader.h> //need to load textures

#include <Cube.h>

#include <glm/glm.hpp>                  //basic glm math functions
#include <glm/gtc/matrix_transform.hpp> //matrix functions
#include <glm/gtc/type_ptr.hpp>         //convert glm types to opengl types

#include <Camera.h>
#include <callbacks.h>
#include <config.h>
#include <Triangle.h>
#include <TrianglesPoli.h>
#include <TriangleRainbow.h>
#include <TriangleRainbowWithCamera.h>
#include <RectangleBlink.h>
#include <Square.h>
#include <Terrain.h>
#include <FrameBuffer.h>
#include <util.h>
#include <stb_image.h>

using namespace std;
using namespace config;

int main()
{

	using singleshape::RectangleBlink;
	using singleshape::Triangle;
	using singleshape::TriangleRainbow;
	using singleshape::TrianglesPoli;
	using singleshape::TriangleRainbowWithCamera;
	using singleshape::Square;
	using singleshape::CubeSet;

	srand(time(NULL));
	GLFWwindow* window = openglWindowInit();
	if (!window)
		return -1;
	scope_guard glfwTerminator(glfwTerminate);
	std::cout << glGetString(GL_VERSION) << std::endl;

	//TODO stbi to flip or not to flip, this is the question?!?!?!
	stbi_set_flip_vertically_on_load(true);
	unsigned int texture0crate    = LoadTexture("bitmaps/crate.jpg",    GL_RGB);
	unsigned int texture1airplane = LoadTexture("bitmaps/airplane.png", GL_RGBA);

	Triangle                  triangle;
	TriangleRainbow           triangleRainbow;
	TrianglesPoli             trianglePoli;
	TriangleRainbowWithCamera triangleRainbowWithCamera;
	RectangleBlink            rectangleBlink;
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
	//GAME LOOP
	glEnable(GL_BLEND); //Enable blending.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Set blending function

	while (!glfwWindowShouldClose(window))
	{
		processInputs(window);

		glm::mat4 model      = glm::translate(glm::mat4(1.0), glm::vec3(0.1f, -0.1f, 0.0f));
		glm::mat4 view       = camera.view();
		glm::mat4 projection = camera.perspective();

		glm::vec4 randomcolor =
		{
			(rand() % 101) / 100.0f, //r
			(rand() % 101) / 100.0f, //g
			(rand() % 101) / 100.0f, //b
			1.0f //a
		};
		rectangleBlink.drawInit(randomcolor);
		triangleRainbowWithCamera.drawInit(model, view, projection);

		model = glm::translate(glm::mat4(1.0), pos);
		model = glm::rotate(model, currentTime, glm::vec3(0.0f, 1.0f, 0.0f));
		terrainFrameBuffer.drawInit(model, view, projection);
		terrainFrameBuffer.draw(
			[]
			{
				glEnable(GL_CULL_FACE);
				glEnable(GL_DEPTH_TEST);
				glClearColor(1.f, 0.5f, 1.f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			});

		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		glClearColor(0.5, 0, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		double depthRange[2] = {};

		switch (currentShape)
		{
		case DRAW_TRIANGLE_SIMPLE:
			triangle.draw();
			break;
		case DRAW_RECTANGLE:
			rectangleBlink.draw();
			break;
		case DRAW_RAINBOW_TRIANGLE:
			//triangleRainbow.draw();
			////trianglePoli.draw();
			//glDepthRange(1.0f, 0.0f);
			////glDepthRange(0.0f, 1.0f);
			//glGetDoublev(GL_DEPTH_RANGE, &depthRange[0]);
			trianglePoli.draw();
			break;
		case DRAW_RAINBOW_TRIANGLE_WITH_CAMERA:
			triangleRainbow.draw();
			triangleRainbowWithCamera.draw();
			break;
		case DRAW_SQUARE: //5
			square.drawInit(texture0crate, currentTime, { 0.5f, -0.5f, 0.0f });
			square.draw();
			square.drawInit(texture1airplane, currentTime, { -0.5f, -0.5f, 0.0f });
			square.draw();
			square.drawInit(terrainFrameBuffer.getTexture(), currentTime, { 0.0f, 0.0f, 0.9f });
			square.draw();
			break;
		case DRAW_CUBES:
			cubeSet.drawInit(texture0crate, view, projection);
			cubeSet.draw();
			break;
		case DRAW_7:
			cubeSet.drawInit(terrainFrameBuffer.getTexture(), view, projection);
			cubeSet.draw();
			break;
		case DRAW_8:
			trianglePoli.draw();
			rectangleBlink.draw();
			triangleRainbowWithCamera.draw();

			square.drawInit(texture0crate, currentTime, { 0.5f, -0.5f, 0.0f });
			square.draw();
			square.drawInit(terrainFrameBuffer.getTexture(), currentTime, { -0.5f, -0.5f, 0.0f });
			square.draw();
			break;
		case DRAW_9:
			square.drawInit(texture0crate, currentTime, { 0.5f, -0.5f, 0.0f });
			square.draw();
			square.drawInit(terrainFrameBuffer.getTexture(), currentTime, { -0.5f, -0.5f, 0.0f });
			square.draw();

			glEnable(GL_CULL_FACE);
			terrain.drawInit(model, view, projection, glm::vec4(0, 1, 0, 1));
			terrain.draw();

			break;
		}

		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	//cleanup
	return 0;
}
#endif