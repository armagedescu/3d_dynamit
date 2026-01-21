#include "enabler.h"

#include <Triangle.h>
#include <config.h>

int main_triangle_lib() 
{
	using singleshape::Triangle;
	GLFWwindow* window = openglWindowInit();
	if (!window)
		return -1;
	Triangle triangle;
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		triangle.draw();

		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;

}
#include "enabler.h"
#ifdef  __MAIN_TRIANGLE_LIB__
int main() { return main_triangle_lib(); }
#endif