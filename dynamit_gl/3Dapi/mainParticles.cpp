#include "enabler.h"
#ifdef __MAIN_PARTICLES_CPP__

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

#include <Config.h>
#include <Particles.h>

using namespace glm;

int main()
{
	GLFWwindow* window = openglWindowInit();
	if (!window)
		return -1;
	Particles particles;

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS); // Accept fragment if it closer to the camera than the former one
	glm::vec3 pos(0.0f, 0.0f, 0.0f);

	while (!glfwWindowShouldClose(window))
	{
		using config::camera;

		processInputs(window);

		glm::mat4 model = glm::translate(glm::mat4(1.0), pos);
		model = glm::rotate(model, currentTime, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 view       = camera.view();
		glm::mat4 projection = camera.perspective();

		glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//draw begin
		particles.drawInit(model, view, projection, deltaTime);
		particles.draw();
		//draw finish

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

#endif