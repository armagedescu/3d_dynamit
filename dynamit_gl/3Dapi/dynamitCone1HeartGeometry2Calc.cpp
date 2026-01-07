#include "enabler.h"
#ifdef __DYNAMIT_CONE1_HEART_GEOMETRY2_CALC_CPP__

#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Dynamit.h>
#include <geometry.h>
#include <config.h>
#include <callbacks.h>
#include <builders.h>
#include <iostream>

using namespace dynamit;
using namespace dynamit::builders;



int main()
{
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;
        
    std::cout << glGetString(GL_VERSION) << std::endl;

    //// Build heart-shaped cone with calculator
    std::vector<float> verts, norms;
    //buildCone (verts, norms, L"1", 6, 4);
    std::cout << "Heart cone vertices: " << verts.size() / 3 << " (triangles: " << verts.size() / 9 << ")" << std::endl;
    
    // Create shape
    Dynamit shape;
    shape.withConstColor(0.0f, 1.0f, 0.5f, 1.0f)
         .withConstLightDirection(-1.0f, -1.0f, 1.0f);

	// First half of first half
    buildConePolar(verts, norms, L"theta / PI", 0.f, M_PI_2, 6, 4);
	shape
        .withVertices3d(verts)
        .withNormals3d(norms);
    // Second half of first half
    buildConePolar(verts, norms, L"theta / PI", M_PI_2, M_PI, 3, 2);
	shape[1]
        .withVertices3d(verts)
        .withNormals3d(norms);
	// Second half
    buildConePolar(verts, norms, L"(2*PI - theta) / PI", M_PI, 2 * M_PI, 10, 10);
	shape[2]
        .withVertices3d(verts)
        .withNormals3d(norms);
    
    shape.logGeneratedShaders("cone1HeartGeometry2Calc.js:");
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 1.f, 0.9f);
    
    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        glPolygonMode(GL_FRONT_AND_BACK, glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS ? GL_LINE : GL_FILL);

        processInputs(window);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        shape.drawTriangles();
        
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    return 0;
}

#endif