#include "enabler.h"
#ifdef __DYNAMIT_ANIMATE_CPP__

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <Dynamit.h>
#include <config.h>
#include <callbacks.h>

using namespace dynamit;

int main()
{
    srand(time(NULL));
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;

    std::cout << glGetString(GL_VERSION) << std::endl;

    // Shape with dynamic translation (uniform)
    std::vector<float> verts_initial = {
        0.0f, 0.0f, 0.0f,   -1.0f,  0.4f,  2.0f,    -0.5f, -0.3f,  2.0f
    };

    Dynamit shape;
    shape.withVertices3d(verts_initial)
        .withTranslation4f();  // Creates translation uniform
	shape.logGeneratedShaders("Generated shaders for animated shape:");

    // Triangle 1 with fixed vertices
    std::vector<float> tri1_verts = {
        0.0f, 0.0f, 0.0f,   -0.3f, -0.5f,  2.0f,     0.5f, -0.6f,  2.0f
    };

    Dynamit triangle1;
    triangle1.withVertices3d(tri1_verts)
        .withTranslation4f();

    // Triangle 2 with fixed vertices
    std::vector<float> tri2_verts = {
        0.0f, 0.0f, 0.0f,    0.3f,  0.4f,  2.0f,    -0.5f,  0.6f, -0.0f
    };

    Dynamit triangle2;
    triangle2.withVertices3d(tri2_verts)
        .withTranslation4f();

    glEnable(GL_DEPTH_TEST);

    double startTime = glfwGetTime();

    // Animation loop
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_BLEND);
    while (!glfwWindowShouldClose(window))
    {
        processInputs(window);

        double currentTime = (glfwGetTime() - startTime) * 1000.0; // milliseconds

        glClearColor(0.5f, 0.5f, 0.5f, 0.9f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Animated shape with vertex buffer updates
        shape.bind();
        float tx = 0.5f * std::cos(currentTime * 0.005f);
        float ty = 0.5f * std::sin(currentTime * 0.005f);
        float tz = 0.5f * std::sin(currentTime * 0.005f);
        shape.translate4f(tx, ty, tz, 0.0f);

        // Draw first triangle with updated vertices
        std::vector<float> verts1 = {
            0.0f, 0.0f, 0.0f,   -1.0f,  0.4f,  2.0f,    -0.5f, -0.3f,  2.0f
        };
        shape.updateVertices(verts1);
        shape.drawTriangles();

        // Draw second triangle with updated vertices
        std::vector<float> verts2 = {
            0.0f, 0.0f, 0.0f,    0.6f, -0.3f,  2.0f,     0.4f,  0.3f, -0.0f
        };
        shape.updateVertices(verts2);
        shape.drawTriangles();

        // Two separate shapes with shared transformation
        triangle1.bind();
        triangle1.translate4f(tx, ty, tz, 0.0f);
        triangle1.drawTriangles();

        triangle2.bind();
        triangle2.translate4f(tx, ty, tz, 0.0f);
        triangle2.drawTriangles();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

#endif