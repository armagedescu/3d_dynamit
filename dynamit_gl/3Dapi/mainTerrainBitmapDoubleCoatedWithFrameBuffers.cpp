#include "enabler.h"
#ifdef  __MAIN_TERRAIN_BITMAP_DOUBLE_COATED_FRAME_BUFFERS__

#include <GL/glew.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>

#include <glm/glm.hpp>                  //basic glm math functions
#include <glm/gtc/matrix_transform.hpp> //matrix functions
#include <glm/gtc/type_ptr.hpp>         //convert glm types to opengl types

#include <Program.h>
#include <Terrain.h>
#include <FrameBuffer.h>
#include <TextureShower.h>
#include <config.h>

using namespace std;

//terrain
int main()
{
	using namespace std;
	srand(time(NULL));
	GLFWwindow* window = openglWindowInit();
	if (!window)
		return -1;

	FrameBuffer<Terrain> terrainFrameBuffer   ("shaders/framebuffer/terrain.framebuffer.vs", "shaders/framebuffer/terrain.framebuffer.fs");
	TextureShower textureShower;

	if (!terrainFrameBuffer)
	{
		char infoLog[512];
		glGetProgramInfoLog(terrainFrameBuffer, 512, NULL, infoLog);
		std::cout << "shaders failed\n" << infoLog << std::endl;
		return -1;
	}

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	// draw as wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glm::vec3 pos(0.0f, 0.0f, 0.0f);

	while (!glfwWindowShouldClose(window))
	{
		using config::camera;
		using config::windowWidth;
		using config::windowHeight;
		processInputs(window);

		//    terrain matrices
		glm::mat4 model = glm::translate(glm::mat4(1.0), pos);
		model = glm::rotate(model, (float)(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 view = camera.view();
		glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)windowWidth / windowHeight, 0.1f, 100.0f);

		terrainFrameBuffer.drawInit(model, view, projection);
		terrainFrameBuffer.draw(
			[]
			{
				glEnable(GL_CULL_FACE);
				glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
				glClearColor(0.f, 0.f, 1.f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			});


		glDisable(GL_CULL_FACE);  // disable depth test so screen-space quad isn't discarded due to depth test.
		glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
		// clear all relevant buffers
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessary actually, since we won't be able to see behind the quad anyways)
		glClear(GL_COLOR_BUFFER_BIT);

		textureShower.drawInit(terrainFrameBuffer.getTexture());
		textureShower.draw();

		//https://learnopengl.com/code_viewer_gh.php?code=src/5.advanced_lighting/3.1.1.shadow_mapping_depth/shadow_mapping_depth.cpp

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

#endif