#include "enabler.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Dynamit.h>
#include <config.h>
#include <callbacks.h>

using namespace dynamit;

// Build cone geometry - variant 2 with different normals
void buildCone2(std::vector<float>& verts, std::vector<float>& norms, int ns = 5)
{
    verts.clear();
    norms.clear();
    
    float dr = 0.6f;
    
    for (int i = 0; i < ns; i++)
    {
        // Calculate base vertices first (needed for normals)
        float angle1 = 2.0f * static_cast<float>(M_PI) * i / ns;
        float x1 = dr * std::cos(angle1);
        float y1 = dr * std::sin(angle1);
        
        float angle2 = 2.0f * static_cast<float>(M_PI) * (i + 1) / ns;
        float x2 = dr * std::cos(angle2);
        float y2 = dr * std::sin(angle2);
        
        // Tip of cone (points toward viewer)
        verts.insert(verts.end(), {0.0f, 0.0f, -0.7f});
        norms.insert(norms.end(), {x1, y1, -0.7f});  // Different: uses first base vertex normal
        
        // Base point 1
        verts.insert(verts.end(), {x1, y1, 0.0f});
        norms.insert(norms.end(), {x1, y1, -0.7f});
        
        // Base point 2
        verts.insert(verts.end(), {x2, y2, 0.0f});
        norms.insert(norms.end(), {x2, y2, -0.7f});
    }
}

int main_cone1Animate2()
{
    srand(time(NULL));
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;
        
    std::cout << glGetString(GL_VERSION) << std::endl;
    
    // Build cone geometry (variant 2)
    std::vector<float> verts, norms;
    buildCone2(verts, norms, 5);
    
    std::cout << "Cone vertices: " << verts.size() / 3 << std::endl;
    
    // Create shape with dynamic light direction
    Dynamit shape;
    shape.withConstColor(0.0f, 1.0f, 0.0f, 1.0f)
         .withVertices3d(verts)
         .withNormals3d(norms)
         .withLightDirection3f(1.0f, 0.0f, 1.0f);
    
    shape.logGeneratedShaders("cone1Animate2.js:");
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.5f, 0.5f, 0.9f);
    
    // Initial draw
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shape.bind();
    shape.lightDirection3f(1.0f, 0.0f, 1.0f);
    shape.drawTriangles();
    
    double startTime = glfwGetTime();
    
    // Animation loop
    while (!glfwWindowShouldClose(window))
    {
        processInputs(window);
        
        double currentTime = (glfwGetTime() - startTime) * 1000.0;
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Update light direction - rotates in circle
        float lx = std::cos(currentTime * 0.002f);
        float ly = std::sin(currentTime * 0.002f);
        float lz = 1.0f;
        
        shape.lightDirection3f(lx, ly, lz);
        shape.drawTriangles();
        
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    return 0;
}
#include "enabler.h"
#ifdef __CONE1_ANIMATE2_CPP__
int main() { return main_cone1Animate2(); }
#endif