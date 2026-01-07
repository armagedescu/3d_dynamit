#include "enabler.h"
#ifdef __MAIN_TERRAIN_BITMAP_SHADERS_CPP__
#include <GL/glew.h>
#include <stdlib.h>
#include <iostream>

#include <glm/glm.hpp>                  //basic glm math functions
#include <glm/gtc/matrix_transform.hpp> //matrix functions
#include <glm/gtc/type_ptr.hpp>         //convert glm types to opengl types

#include <config.h>
#include <Terrain.h>

using namespace std;

int main()
{
	using namespace std;
	srand(time(NULL));
	GLFWwindow* window = openglWindowInit();
	if (!window)
		return -1;

	Terrain terrain("shaders/terrain.vs", "shaders/terrain.fs");
	if (!terrain)
	{
		char infoLog[512];
		glGetProgramInfoLog(terrain, 512, NULL, infoLog);
		std::cout << "shaders failed\n" << infoLog << std::endl;
		return -1;
	}

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glm::vec3 pos(0.0f, 0.0f, 0.0f);

    //mesh visualizer
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(window))
	{
		using config::camera;
		using config::windowWidth;
		using config::windowHeight;

		processInputs(window);
		// transformations
		glm::mat4 model = glm::mat4(1.0);
		model = glm::translate(model, pos);
		model = glm::rotate(model, (float)(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 view = camera.view();
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(camera.zoom), (float)windowWidth / windowHeight, 0.1f, 100.0f);

		glClearColor(0.f, 0.f, 1.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		terrain.drawInit(model, view, projection, glm::vec4(1, 0, 0, 1));
		terrain.draw();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}


#endif