#include "enabler.h"
#ifdef __DYNAMIT_DRAW_STRIDED_CPP__

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Dynamit.h>
#include <config.h>
#include <callbacks.h>

using namespace dynamit;

int main()
{

    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;

    std::cout << glGetString(GL_VERSION) << std::endl;

    Dynamit shape;
    {
        // Interleaved data: position(3) + normal(3) per vertex
        std::vector<float> interleavedData = {
            // x, y, z, nx, ny, nz
            0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
            0.5f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
        };

        std::vector<float> interleavedData1 = {
            // x, y, z, nx, ny, nz
            0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
           -0.5f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
           -1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        };

        shape.withStride(interleavedData, 6 * sizeof(float))
            .withStrideVertices(3)     // 3 floats at offset 0
            .withStrideColors(3);      // 3 floats at offset 12
        shape[1].withStride(interleavedData1, 6 * sizeof(float));
    }
    shape.logGeneratedShaders("draw strided");

    Dynamit shape1;
    {
        // Indexed mesh data
        std::vector<float> vertices = {
            // Unique vertices only
            0.0f, 0.0f, 0.0f,  // 0
            1.0f, 0.0f, 0.0f,  // 1
            1.0f, 1.0f, 0.0f,  // 2
            0.0f, 1.0f, 0.0f,  // 3
        };
        std::vector<float> vertices1 = {
            // Unique vertices only
            0.0f, 0.0f, 0.0f,  // 0
            0.0f, 1.0f, 0.0f,  // 3
            -1.0f, 1.0f, 0.0f,  // 2
            -1.0f, 0.0f, 0.0f,  // 1
        };

        std::vector<uint32_t> indices = {
            0, 1, 2,  // Triangle 1
            0, 2, 3,  // Triangle 2
        };
        std::vector<uint32_t> indices1 = {
            //0, 1, 2,  // Triangle 1
            0, 2, 3,  // Triangle 2
        };

        shape1.withVertices3d(vertices)
            .withIndices(indices)
            .withConstColor(0.0f, 1.0f, 0.0f, 1.0f);
        shape1[1].withVertices3d(vertices1)
            .withIndices(indices1);
    }

    Dynamit shape2;
    {

        std::vector<uint32_t> indices = {
            0, 1, 2,  // Triangle 1
        };
        // Interleaved data: position(3) + normal(3) per vertex
        std::vector<float> interleavedData = {
            // x, y, z, nx, ny, nz
            0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
            0.5f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
        };

        std::vector<float> interleavedData1 = {
            // x, y, z, nx, ny, nz
            0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
           -0.5f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
           -1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        };
        shape2.withStride(interleavedData, 24)
            .withStrideVertices(3)
			.withStrideOffset(3 * sizeof(float))
            .withStrideColors(3)
            .withIndices(indices);
        shape2[1].withStride(interleavedData1, 24)
            .withIndices(indices);


    }
	shape2.buildProgram(); //can do it explicitly, but not really necessary

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.5f, 0.5f, 0.9f);
    
    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        glPolygonMode(GL_FRONT_AND_BACK, glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS ? GL_LINE : GL_FILL);

        processInputs(window);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shape.drawTriangles();
        //shape1.drawTrianglesIndexed();
        //shape2.drawTrianglesIndexed();

        
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    return 0;
}

#endif