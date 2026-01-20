#include "enabler.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Dynamit.h>
#include <geometry.h>
#include <config.h>
#include <callbacks.h>

using namespace dynamit;

// Build heart-shaped parametric cone
void buildConeHeart(std::vector<float>& verts, std::vector<float>& norms, int nh = 2, int ns = 40)
{
    // Ensure even number of sectors for symmetry
    if (ns & 1) ns++;
    
    verts.clear();
    norms.clear();
    
    // First ring - tip to base
    for (int i = 0; i < ns; i++)
    {
        // Parametric radius (creates heart shape)
        float dr = 2.0f * i / ns;
        float drd = 2.0f * (i + 1) / ns;
        if (dr > 1.0f) dr = 2.0f - dr;      // Decrease after PI
        if (drd > 1.0f) drd = 2.0f - drd;   // Decrease after PI
        
        std::array<float, 3> ps[3] = {
            {0.0f, 0.0f, -1.0f},  // Tip
            {(dr / nh) * std::cos(2.0f * static_cast<float>(M_PI) * i / ns),
             (dr / nh) * std::sin(2.0f * static_cast<float>(M_PI) * i / ns),
             -(1.0f - 1.0f / nh)},
            {(drd / nh) * std::cos(2.0f * static_cast<float>(M_PI) * (i + 1) / ns),
             (drd / nh) * std::sin(2.0f * static_cast<float>(M_PI) * (i + 1) / ns),
             -(1.0f - 1.0f / nh)}
        };
        
        auto cr = cross3pl(ps[0], ps[1], ps[2]);
        
        verts.insert(verts.end(), {ps[0][0], ps[0][1], ps[0][2]});
        verts.insert(verts.end(), {ps[1][0], ps[1][1], ps[1][2]});
        verts.insert(verts.end(), {ps[2][0], ps[2][1], ps[2][2]});
        
        norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
        norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
        norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
    }
    
    // Body rings
    for (int h = 1; h < nh; h++)
    {
        for (int i = 0; i < ns; i++)
        {
            float dr = 2.0f * i / ns;
            float drd = 2.0f * (i + 1) / ns;
            if (dr > 1.0f) dr = 2.0f - dr;
            if (drd > 1.0f) drd = 2.0f - drd;
            
            std::array<float, 3> ps[4] = {
                {(h * dr / nh) * std::cos(2.0f * static_cast<float>(M_PI) * i / ns),
                 (h * dr / nh) * std::sin(2.0f * static_cast<float>(M_PI) * i / ns),
                 -(1.0f - h * 1.0f / nh)},
                {(h * drd / nh) * std::cos(2.0f * static_cast<float>(M_PI) * (i + 1) / ns),
                 (h * drd / nh) * std::sin(2.0f * static_cast<float>(M_PI) * (i + 1) / ns),
                 -(1.0f - h * 1.0f / nh)},
                {((h + 1) * dr / nh) * std::cos(2.0f * static_cast<float>(M_PI) * i / ns),
                 ((h + 1) * dr / nh) * std::sin(2.0f * static_cast<float>(M_PI) * i / ns),
                 -(1.0f - (h + 1) * 1.0f / nh)},
                {((h + 1) * drd / nh) * std::cos(2.0f * static_cast<float>(M_PI) * (i + 1) / ns),
                 ((h + 1) * drd / nh) * std::sin(2.0f * static_cast<float>(M_PI) * (i + 1) / ns),
                 -(1.0f - (h + 1) * 1.0f / nh)}
            };
            
            // First triangle
            auto cr = cross3pl(ps[0], ps[2], ps[3]);
            
            verts.insert(verts.end(), {ps[0][0], ps[0][1], ps[0][2]});
            verts.insert(verts.end(), {ps[2][0], ps[2][1], ps[2][2]});
            verts.insert(verts.end(), {ps[3][0], ps[3][1], ps[3][2]});
            
            norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
            norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
            norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
            
            // Second triangle
            cr = cross3pl(ps[0], ps[3], ps[1]);
            
            verts.insert(verts.end(), {ps[0][0], ps[0][1], ps[0][2]});
            verts.insert(verts.end(), {ps[1][0], ps[1][1], ps[1][2]});
            verts.insert(verts.end(), {ps[3][0], ps[3][1], ps[3][2]});
            
            norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
            norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
            norms.insert(norms.end(), {cr[0], cr[1], cr[2]});
        }
    }
}

int main_dynamitCone1HeartGeometry1()
{
    srand(time(NULL));
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;
        
    std::cout << glGetString(GL_VERSION) << std::endl;
    
    // Build heart-shaped cone
    std::vector<float> verts, norms;
    buildConeHeart(verts, norms, 3, 20);
    
    std::cout << "Heart cone vertices: " << verts.size() / 3 << " (triangles: " << verts.size() / 9 << ")" << std::endl;
    
    // Create shape
    Dynamit shape;
    shape.withConstColor(0.0f, 1.0f, 0.0f, 1.0f)
         .withVertices3d(verts)
         .withNormals3d(norms)
         .withConstLightDirection(-1.0f, -1.0f, 1.0f);
    
    shape.logGeneratedShaders("cone1HeartGeometry1.js:");
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
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
#ifdef __DYNAMIT_CONE1_HEART_GEOMETRY1_CPP__
int main() { return main_dynamitCone1HeartGeometry1(); }
#endif