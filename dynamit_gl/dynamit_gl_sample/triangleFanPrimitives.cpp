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
    verts.clear();
    norms.clear();
    
    // Center point
    verts.insert(verts.end(), {0.0f, 0.0f, 0.0f});
    norms.insert(norms.end(), {0.0f, 0.0f, 1.0f});
    
    // Circle points
    for (int i = 0; i <= ns; i++)
    {
        float angle = 2.0f * static_cast<float>(M_PI) * i / ns;
        float x = dr * std::cos(angle) / 2 - .5f;
        float y = dr * std::sin(angle) / 2 - .5f;
        float z = -1.0f;
        
        verts.insert(verts.end(), {x, y, z});
        norms.insert(norms.end(), {0.0f, -1.0f, 1.0f});
    }
}

// Generate triangle fan geometry (disc/circle)
void buildTrianglesGeometry(std::vector<float>& verts, std::vector<float>& norms,
    int ns = 16, float dr = 0.6f)
{
    verts.clear();
    norms.clear();


    // Circle points
    for (int i = 0; i <= ns; i++)
    {
        // Center point
        verts.insert(verts.end(), { 0.0f, 0.0f, 0.0f });
        norms.insert(norms.end(), { 0.0f, 0.0f, 1.0f });

        float angle = 2.0f * static_cast<float>(M_PI) * i / ns;
        float x = dr * std::cos(angle) / 2 + .5f;
        float y = dr * std::sin(angle) / 2 + .5f;
        float z = -1.0f;
        verts.insert(verts.end(), { x, y, z });
        norms.insert(norms.end(), { 0.0f, -1.0f, 1.0f });

        angle = 2.0f * static_cast<float>(M_PI) * (i + 1) / ns;
        x = dr * std::cos(angle) / 2 + .5f;
        y = dr * std::sin(angle) / 2 + .5f;
        verts.insert(verts.end(), { x, y, z });
        norms.insert(norms.end(), { 0.0f, -1.0f, 1.0f });
    }
}

int main_dynamitTriangleFanPrimitives()
{
    srand(time(NULL));
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;
        
    std::cout << glGetString(GL_VERSION) << std::endl;
    
    // Build triangle fan geometry
    std::vector<float> verts, norms;
    std::vector<float> verts1, norms1;
    buildTriangleFanGeometry(verts, norms, 16, 0.6f);
    buildTrianglesGeometry(verts1, norms1, 16, 0.6f);
    
    std::cout << "Triangle fan vertices: " << verts.size() / 3 << std::endl;
    std::cout << "First vertex: (" << verts[0] << ", " << verts[1] << ", " << verts[2] << ")" << std::endl;
    
    // Create shape with lighting (no normalization)
    Dynamit shape;
    shape.withPrimitive(GL_TRIANGLE_FAN)
         .withVertices3d(verts)
         .withNormals3d(norms)
         .withConstColor(0.0f, 1.0f, 0.0f, 0.1f)  // Green with low alpha
         .withConstLightDirection(0.0f, 1.2f, 0.0f, false);  // No normalization
    shape[1].withPrimitive(GL_TRIANGLES)
            .withVertices3d(verts1)
            .withNormals3d(norms1);
    
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
        shape.drawTriangles(); // drawTriangleFan();
        
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    return 0;
}

#ifdef __TRIANGLE_FAN_PRIMITIVES_CPP__
int main() { return main_dynamitTriangleFanPrimitives(); }
#endif