#include "enabler.h"

#define _USE_MATH_DEFINES  // Add this BEFORE including cmath
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Dynamit.h>
#include <config.h>
#include <callbacks.h>

using namespace dynamit;

// Generate triangle fan geometry (disc/circle)
static void buildTriangleFanGeometry(std::vector<float>& verts, std::vector<float>& norms, 
                              int ns = 16, float dr = 0.6f)
{
    // Center point
    verts.insert(verts.end(), {0.8f, 0.8f, 0.0f});
    norms.insert(norms.end(), {0.0f, 0.0f, 1.0f});
    
    // Circle points
    for (int i = 0; i <= ns; i++)
    {
        float angle = 2.0f * static_cast<float>(M_PI) * i / ns;
        float x = dr * std::cos(angle);
        float y = dr * std::sin(angle);
        float z = -1.0f;
        
        verts.insert(verts.end(), {x, y, z});
        norms.insert(norms.end(), {0.0f, -1.0f, 1.0f});
    }
}

int main_triangleFan()
{
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;
        
    std::cout << glGetString(GL_VERSION) << std::endl;
    
    // Build triangle fan geometry
    std::vector<float> verts, norms;
    buildTriangleFanGeometry(verts, norms, 16, 0.6f);
    
    std::cout << "Triangle fan vertices: " << verts.size() / 3 << std::endl;
    std::cout << "First vertex: (" << verts[0] << ", " << verts[1] << ", " << verts[2] << ")" << std::endl;
    
    // Create shape with lighting (no normalization)
    Dynamit shape;
    shape.withVertices3d(verts)
         .withNormals3d(norms)
         .withConstColor(0.0f, 1.0f, 0.0f, 0.1f)  // Green with low alpha
         .withConstLightDirection(0.0f, 1.2f, 0.0f, false);  // No normalization
    
    shape.logGeneratedShaders("triangleFan.js:");
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.5f, 0.5f, 0.5f, 0.9f);
    
    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        processInputs(window);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Draw using triangle fan mode
        shape.drawTriangleFan();
        
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    return 0;
}
#include "enabler.h"
#ifdef __TRIANGLE_FAN_CPP__
int main() { return main_triangleFan(); }
#endif