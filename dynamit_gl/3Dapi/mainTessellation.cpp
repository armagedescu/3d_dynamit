#include "enabler.h"
#ifdef  __MAIN_TESSELLATION_CPP__
#include <GL/glew.h>
#include "config.h"
#include <iostream>

#include <glm/glm.hpp> //basic glm math functions
#include <glm/gtc/matrix_transform.hpp> //matrix functions
#include <glm/gtc/type_ptr.hpp> //convert glm types to opengl types

#include <callbacks.h>
#include <Tess.h>
#include <TriangleRainbow.h>

int main()
{
	using singleshape::TriangleRainbow;
	using std::cout;
	using std::endl;
	srand(time(NULL));
	config::openGlVersionMaj = 4;
	config::openGlVersionMin = 6;
	GLFWwindow* window = openglWindowInit();
	if (!window)
		return -1;
	cout << glGetString(GL_VERSION) << endl;

	TessQuad       tessQuad;
	TessTriangle   tessTri;
	TessTriangleIndexed   tessTriIndexed;
	TessTriangleRainbow   tessTriRainbow;
	TriangleRainbow   triRainbow;

	if (!tessTriRainbow) { cout << tessTriRainbow << endl; return -1; }
	if (!triRainbow) { cout << triRainbow << endl; return -1; }

	glDisable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);

	int currentShape = 0;
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //linii
	while (!glfwWindowShouldClose(window))
	{
		using config::camera;
		processInputs(window);
		if (keyPressed)
		{
			keyPressed = false;
			const int shapes = 5;
			switch (currentDraw)
			{
			case DRAW_1:
				if (keyMod & GLFW_MOD_ALT)currentShape--;
				else currentShape++;
				if (currentShape < 0) currentShape = shapes - 1;
				currentShape %= shapes;
				break;
			}
		}

		glClearColor(0.f, 0.f, 1.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		switch (currentShape)
		{
		case 0: triRainbow.draw();     break;
		case 1: tessTriRainbow.draw(); break;
		case 2: tessTriIndexed.draw(); break;
		case 3: tessQuad.draw();       break;
		case 4: tessTri.draw();        break;
		}

		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}


#endif