#include "enabler.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Dynamit.h>
#include <config.h>
#include <callbacks.h>

using namespace dynamit;

int main_normalsAndColors2Progs()
{
    srand(time(NULL));
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;
        
    std::cout << glGetString(GL_VERSION) << std::endl;
    
    // Shape 1 - Uses normals + const color for lighting
    std::vector<float> verts1 = {
        0.0f, 0.0f, 0.0f,   -1.0f,  0.4f,  2.0f,    -0.5f, -0.3f,  2.0f,
        0.0f, 0.0f, 0.0f,    0.6f, -0.3f,  2.0f,     0.4f,  0.3f, -0.0f
    };
    
    std::vector<float> normals1 = {
        1.0f,  1.0f, -1.0f,   1.0f, 1.0f, -1.0f,     1.0f,  1.0f, -1.0f,
        1.0f,  0.0f, -1.0f,   1.0f, 0.0f, -1.0f,     1.0f,  0.0f, -1.0f
    };
    
    Dynamit shape1;
    shape1.withConstColor(0.0f, 1.0f, 0.0f, 1.0f)
          .withVertices3d(verts1)
          .withNormals3d(normals1);
    
    shape1.logGeneratedShaders("normals_and_colors_2progs shape1 (normals):");
    
    // Shape 2 - Uses per-vertex colors (no lighting)
    std::vector<float> verts2 = {
        0.0f, 0.0f, 0.0f,   -0.3f, -0.5f,  2.0f,     0.5f, -0.6f,  2.0f,
        0.0f, 0.0f, 0.0f,    0.3f,  0.4f,  2.0f,    -0.5f,  0.6f, -0.0f
    };
    
    std::vector<float> colors2 = {
        0.0f, 1.0f, 0.0f,      0.0f, 1.0f, 0.0f,      0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,      1.0f, 0.0f, 0.0f,      1.0f, 0.0f, 0.0f
    };
    
    Dynamit shape2;
    shape2.withVertices3d(verts2)
          .withColors3d(colors2);
    
    shape2.logGeneratedShaders("normals_and_colors_2progs shape2 (colors):");
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.5f, 0.5f, 0.9f);
    
    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        processInputs(window);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Draw both shapes - each uses its own shader program
        shape1.drawTriangles();
        shape2.drawTriangles();
        
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    return 0;
}
#include "enabler.h"
#ifdef __NORMALS_AND_COLORS_2PROGS_CPP__
int main() { return main_normalsAndColors2Progs(); }
#endif