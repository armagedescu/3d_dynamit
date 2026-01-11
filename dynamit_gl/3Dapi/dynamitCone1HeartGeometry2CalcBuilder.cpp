#include "enabler.h"
#ifdef __DYNAMIT_CONE1_HEART_GEOMETRY2_CALC_BUILDER_CPP__

//TODO: Consider to remove current. This is identical to dynamitCone1HeartGeometry2Calc.cpp

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
    PolarBuilder()
        .formula(L"theta / PI")
        .domain(0.f, static_cast<float>(M_PI_2))
        .sectors(6)
        .slices(4)
        .buildCone(verts, norms);
    shape
        .withVertices3d(verts)
        .withNormals3d(norms);

    // Second half of first half
    PolarBuilder()
        .formula(L"theta / PI")
        .domain(static_cast<float>(M_PI_2), static_cast<float>(M_PI))
        .sectors(3)
        .slices(2)
        .buildCone(verts, norms);
    shape[1]
        .withVertices3d(verts)
        .withNormals3d(norms);

    // Second half
    PolarBuilder()
        .formula(L"(2*PI - theta) / PI")
        .domain(static_cast<float>(M_PI), static_cast<float>(2 * M_PI))
        .sectors(10)
        .slices(10)
        .buildCone(verts, norms);
    shape[2]
        .withVertices3d(verts)
        .withNormals3d(norms);
    
    shape.logGeneratedShaders("cone1HeartGeometry2CalcBuilder.js:");
    
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