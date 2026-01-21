#include "enabler.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Dynamit.h>
#include <geometry.h>  // Added - for cross3pl
#include <config.h>
#include <callbacks.h>
#include <array>

using namespace dynamit;

// Cone geometry - direct translation from JavaScript cone1Geometry2.js
void buildConeGeometry(std::vector<float>& verts, std::vector<float>& norms, 
                       int ns = 20, int nh = 2)
{ 
    float dr = 1.0f;
    
    for (int i = 0; i < ns; i++)
    {
        std::array<float, 3> ps[3] = {
            {0.0f,                                        0.0f,                                         -1.0f},
            {(dr/nh) * std::cos(2.0 * M_PI *     i / ns), (dr/nh) * std::sin(2.0 * M_PI *     i / ns), (-1.0 + 1.0/nh)},
            {(dr/nh) * std::cos(2.0 * M_PI * (i+1) / ns), (dr/nh) * std::sin(2.0 * M_PI * (i+1) / ns), (-1.0 + 1.0/nh)}
        };
        auto cr = cross3pl(ps[0], ps[1], ps[2]);
        
        verts.insert(verts.end(), {ps[0][0], ps[0][1], ps[0][2]});
        verts.insert(verts.end(), {ps[1][0], ps[1][1], ps[1][2]});
        verts.insert(verts.end(), {ps[2][0], ps[2][1], ps[2][2]});
        
        norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
        norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
        norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
    }
    
    //1 triangle = 3 points * 3 coordinates
    for (int h = 1; h < nh; h++)
    {
        for (int i = 0; i < ns; i++)
        {
            std::array<float, 3> ps[4] = {
                {    (h*dr/nh) * std::cos(2.0 * M_PI *     i/ns),     (h*dr/nh) * std::sin(2.0 * M_PI     * i / ns), (-1.0 +     h*1.0/nh)},
                {    (h*dr/nh) * std::cos(2.0 * M_PI * (i+1)/ns),     (h*dr/nh) * std::sin(2.0 * M_PI * (i+1) / ns), (-1.0 +     h*1.0/nh)},
                {((h+1)*dr/nh) * std::cos(2.0 * M_PI *     i/ns), ((h+1)*dr/nh) * std::sin(2.0 * M_PI *     i / ns), (-1.0 + (h+1)*1.0/nh)},
                {((h+1)*dr/nh) * std::cos(2.0 * M_PI * (i+1)/ns), ((h+1)*dr/nh) * std::sin(2.0 * M_PI * (i+1) / ns), (-1.0 + (h+1)*1.0/nh)}
            };
            
            auto cr = cross3pl(ps[0], ps[2], ps[3]);
            
            verts.insert(verts.end(), {ps[0][0], ps[0][1], ps[0][2]});
            verts.insert(verts.end(), {ps[2][0], ps[2][1], ps[2][2]});
            verts.insert(verts.end(), {ps[3][0], ps[3][1], ps[3][2]});
            
            norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
            norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
            norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
            
            cr = cross3pl(ps[0], ps[3], ps[1]);
            
            verts.insert(verts.end(), {ps[0][0], ps[0][1], ps[0][2]});
            verts.insert(verts.end(), {ps[3][0], ps[3][1], ps[3][2]});
            verts.insert(verts.end(), {ps[1][0], ps[1][1], ps[1][2]});
            
            norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
            norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
            norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
        }
    }
}

int main_hello ()
{
    srand(time(NULL));
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;
        
    std::cout << glGetString(GL_VERSION) << std::endl;
    
    // Build geometry
    std::vector<float> verts, norms;
    buildConeGeometry(verts, norms, 20, 2);
    
    // Create dynamit shape with automatic shader generation
    Dynamit shape;
    shape.withVertices3d(verts)
        .withNormals3d(norms)
        .withConstColor(0.0f, 1.0f, 0.0f, 1.0f)
        .withConstLightDirection(-1.0f, -1.0f, 1.0f);
    
    //shape.logGeneratedShaders("Generated shaders for cone:");
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.5f, 0.5f, 0.9f);
    
    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        processInputs(window);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        shape.drawTriangles();
        
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    return 0;
}

#include "enabler.h"
#ifdef __HELLO_CPP__
int main() { return main_hello(); }
#endif