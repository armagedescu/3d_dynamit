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

// Build heart-shaped parametric cone with proper CCW winding and analytical normals
void buildConeHeart2(std::vector<float>& verts, std::vector<float>& norms, int nh = 2, int ns = 20)
{
    if (ns & 1) ns++;
    
    verts.clear();
    norms.clear();
    
    const float PI_2 = 2.0f * static_cast<float>(M_PI);
    const float FI_S = PI_2 / ns;  // Angular size of one sector
    const float R_S = 2.0f / ns;   // Radius step (normalized)
    const float D_H = 1.0f / nh;   // Height step
    
    // First ring - tip to base
    for (int i = 0; i < ns; i++)
    {
        std::array<float, 3> ps[3];
        std::array<float, 3> cr[3];
        
        if (i < ns / 2)  // First half
        {
            float fi1 = i * FI_S;
            float fi2 = fi1 + FI_S;
            
            float rc1 = i * R_S;
            float rc2 = rc1 + R_S;
            
            float r1 = rc1 / nh;
            float r2 = rc2 / nh;
            
            ps[0] = {0.0f, 0.0f, -1.0f};
            ps[1] = {r1 * std::cos(fi1), r1 * std::sin(fi1), -(1.0f - D_H)};
            ps[2] = {r2 * std::cos(fi2), r2 * std::sin(fi2), -(1.0f - D_H)};
            
            cr[0] = {0.0f, 0.0f, 0.0f};
            cr[1] = {std::sin(fi1) + fi1 * std::cos(fi1), -(std::cos(fi1) - fi1 * std::sin(fi1)), -(1.0f + fi1)};
            cr[2] = {std::sin(fi2) + fi2 * std::cos(fi2), -(std::cos(fi2) - fi2 * std::sin(fi2)), -(1.0f + fi2)};
        }
        else  // Second half - reversed winding
        {
            int i2 = i - ns / 2;
            float fi1 = -i2 * FI_S;
            float fi2 = fi1 - FI_S;
            
            float rc1 = i2 * R_S;
            float rc2 = rc1 + R_S;
            
            float r1 = rc1 / nh;
            float r2 = rc2 / nh;
            
            ps[0] = {0.0f, 0.0f, -1.0f};
            ps[1] = {r2 * std::cos(fi2), r2 * std::sin(fi2), -(1.0f - D_H)};  // Swapped
            ps[2] = {r1 * std::cos(fi1), r1 * std::sin(fi1), -(1.0f - D_H)};
            
            cr[0] = {0.0f, 0.0f, 0.0f};
            cr[1] = {-(std::sin(fi2) + fi2 * std::cos(fi2)), std::cos(fi2) - fi2 * std::sin(fi2), -(1.0f + fi2)};
            cr[2] = {-(std::sin(fi1) + fi1 * std::cos(fi1)), std::cos(fi1) - fi1 * std::sin(fi1), -(1.0f + fi1)};
        }
        
        verts.insert(verts.end(), {ps[0][0], ps[0][1], ps[0][2]});
        verts.insert(verts.end(), {ps[1][0], ps[1][1], ps[1][2]});
        verts.insert(verts.end(), {ps[2][0], ps[2][1], ps[2][2]});
        
        norms.insert(norms.end(), {cr[0][0], cr[0][1], cr[0][2]});
        norms.insert(norms.end(), {cr[1][0], cr[1][1], cr[1][2]});
        norms.insert(norms.end(), {cr[2][0], cr[2][1], cr[2][2]});
    }
    
    // Body rings
    for (int h = 1; h < nh; h++)
    {
        float h1n = static_cast<float>(h) / nh;
        float h2n = static_cast<float>(h + 1) / nh;
        
        for (int i = 0; i < ns; i++)
        {
            std::array<float, 3> ps[4];
            std::array<float, 3> cr[4];
            
            if (i < ns / 2)  // First half
            {
                float fi1 = i * FI_S;
                float fi2 = fi1 + FI_S;
                
                float rc1 = i * R_S;
                float rc2 = rc1 + R_S;
                
                float r11 = rc1 * h1n;
                float r12 = rc2 * h1n;
                float r21 = rc1 * h2n;
                float r22 = rc2 * h2n;
                
                ps[0] = {r11 * std::cos(fi1), r11 * std::sin(fi1), -(1.0f - h1n)};
                ps[1] = {r12 * std::cos(fi2), r12 * std::sin(fi2), -(1.0f - h1n)};
                ps[2] = {r21 * std::cos(fi1), r21 * std::sin(fi1), -(1.0f - h2n)};
                ps[3] = {r22 * std::cos(fi2), r22 * std::sin(fi2), -(1.0f - h2n)};
                
                cr[0] = {std::sin(fi1) + fi1 * std::cos(fi1), -(std::cos(fi1) - fi1 * std::sin(fi1)), -(1.0f + fi1)};
                cr[1] = {std::sin(fi2) + fi2 * std::cos(fi2), -(std::cos(fi2) - fi2 * std::sin(fi2)), -(1.0f + fi2)};
                cr[2] = cr[0];
                cr[3] = cr[1];
            }
            else  // Second half
            {
                int i2 = i - ns / 2;
                float fi1 = -i2 * FI_S;
                float fi2 = fi1 - FI_S;
                
                float rc1 = i2 * R_S;
                float rc2 = rc1 + R_S;
                
                float r11 = rc1 * h1n;
                float r12 = rc2 * h1n;
                float r21 = rc1 * h2n;
                float r22 = rc2 * h2n;
                
                ps[0] = {r11 * std::cos(fi1), r11 * std::sin(fi1), -(1.0f - h1n)};
                ps[1] = {r12 * std::cos(fi2), r12 * std::sin(fi2), -(1.0f - h1n)};
                ps[2] = {r21 * std::cos(fi1), r21 * std::sin(fi1), -(1.0f - h2n)};
                ps[3] = {r22 * std::cos(fi2), r22 * std::sin(fi2), -(1.0f - h2n)};
                
                cr[0] = {-(std::sin(fi1) + fi1 * std::cos(fi1)), std::cos(fi1) - fi1 * std::sin(fi1), -(1.0f + fi1)};
                cr[1] = {-(std::sin(fi2) + fi2 * std::cos(fi2)), std::cos(fi2) - fi2 * std::sin(fi2), -(1.0f + fi2)};
                cr[2] = cr[0];
                cr[3] = cr[1];
            }
            
            // First triangle: [0, 2, 3]
            verts.insert(verts.end(), {ps[0][0], ps[0][1], ps[0][2]});
            verts.insert(verts.end(), {ps[2][0], ps[2][1], ps[2][2]});
            verts.insert(verts.end(), {ps[3][0], ps[3][1], ps[3][2]});
            
            norms.insert(norms.end(), {cr[0][0], cr[0][1], cr[0][2]});
            norms.insert(norms.end(), {cr[2][0], cr[2][1], cr[2][2]});
            norms.insert(norms.end(), {cr[3][0], cr[3][1], cr[3][2]});
            
            // Second triangle: [0, 1, 3] (Note: different order than geometry1!)
            verts.insert(verts.end(), {ps[0][0], ps[0][1], ps[0][2]});
            verts.insert(verts.end(), {ps[1][0], ps[1][1], ps[1][2]});
            verts.insert(verts.end(), {ps[3][0], ps[3][1], ps[3][2]});
            
            norms.insert(norms.end(), {cr[0][0], cr[0][1], cr[0][2]});
            norms.insert(norms.end(), {cr[1][0], cr[1][1], cr[1][2]});
            norms.insert(norms.end(), {cr[3][0], cr[3][1], cr[3][2]});
        }
    }
}

int main_cone1HeartGeometry2()
{
    srand(time(NULL));
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;
        
    std::cout << glGetString(GL_VERSION) << std::endl;
    
    // Build heart-shaped cone with proper CCW winding
    std::vector<float> verts, norms;
    buildConeHeart2(verts, norms, 2, 20);
    
    std::cout << "Heart cone vertices: " << verts.size() / 3 << " (triangles: " << verts.size() / 9 << ")" << std::endl;
    
    // Create shape
    Dynamit shape;
    shape.withConstColor(0.0f, 1.0f, 0.0f, 1.0f)
         .withVertices3d(verts)
         .withNormals3d(norms)
         .withConstLightDirection(-1.0f, -1.0f, 1.0f);
    
    shape.logGeneratedShaders("cone1HeartGeometry2.js:");
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.5f, 0.5f, 0.9f);
    
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
#ifdef __CONE1_HEART_GEOMETRY2_CPP__
int main() { return main_cone1HeartGeometry2(); }
#endif