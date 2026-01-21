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

// Build cone geometry - same as cone1Animate2 but with configurable sectors
void buildCone1(std::vector<float>& verts, std::vector<float>& norms, int ns = 20)
{
    float dr = 0.6f;
    
    for (int i = 0; i < ns; i++)
    {
        // Calculate base vertices
        float angle1 = 2.0f * static_cast<float>(M_PI) * i / ns;
        float x1 = dr * std::cos(angle1);
        float y1 = dr * std::sin(angle1);
        
        float angle2 = 2.0f * static_cast<float>(M_PI) * (i + 1) / ns;
        float x2 = dr * std::cos(angle2);
        float y2 = dr * std::sin(angle2);
        
        // Tip of cone (points toward viewer)
        verts.insert(verts.end(), {0.0f, 0.0f, -0.7f});
        norms.insert(norms.end(), {x1, y1, -0.7f});
        
        // Base point 1
        verts.insert(verts.end(), {x1, y1, 0.0f});
        norms.insert(norms.end(), {x1, y1, -0.7f});
        
        // Base point 2
        verts.insert(verts.end(), {x2, y2, 0.0f});
        norms.insert(norms.end(), {x2, y2, -0.7f});
    }
}

int main_cone1Geometry1()
{
    srand(time(NULL));
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;
        
    std::cout << glGetString(GL_VERSION) << std::endl;
    
    // Build cone geometry with 20 sectors (smoother)
    std::vector<float> verts, norms;
    buildCone1(verts, norms, 20);
    
    std::cout << "Cone vertices: " << verts.size() / 3 << " (triangles: " << verts.size() / 9 << ")" << std::endl;
    
    // Create shape with const light direction (no animation)
    Dynamit shape;
    shape.withConstColor(0.0f, 1.0f, 0.0f, 1.0f)
         .withVertices3d(verts)
         .withNormals3d(norms)
         .withConstLightDirection(-1.0f, -1.0f, 1.0f);  // Const, not uniform
    
    shape.logGeneratedShaders("cone1Geometry1.js:");
    
    glEnable(GL_DEPTH_TEST);
    
    // Render loop (static - no animation)
    while (!glfwWindowShouldClose(window))
    {
        glPolygonMode(GL_FRONT_AND_BACK, glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS ? GL_LINE : GL_FILL);

        processInputs(window);
        
        glClearColor(0.5f, 0.5f, 0.5f, 0.9f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        shape.drawTriangles();
        
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    return 0;
}
#include "enabler.h"
#ifdef __CONE1_GEOMETRY1_CPP__
int main() { return main_cone1Geometry1(); }
#endif