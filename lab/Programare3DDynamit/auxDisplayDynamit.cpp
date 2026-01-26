#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include <Dynamit.h>
#include <builders.h>
#include <geometry.h>
#include <config.h>
#include <callbacks.h>

using namespace dynamit;
using namespace dynamit::builders;
using namespace dynamit::geo;

namespace auxDisplayDynamit
{
// ============================================================================
// Geometry Builders
// ============================================================================

// Build axis lines (X=red, Y=green, Z=blue)
void buildAxisGeometry(std::vector<float>& verts, std::vector<float>& colors)
{
    // X axis - red
    verts.insert(verts.end(), { 0.0f, 0.0f, 0.0f,  5.5f, 0.0f, 0.0f });
    colors.insert(colors.end(), { 1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f });
    
    // Y axis - green
    verts.insert(verts.end(), { 0.0f, 0.0f, 0.0f,  0.0f, 5.5f, 0.0f });
    colors.insert(colors.end(), { 0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f });
    
    // Z axis - blue
    verts.insert(verts.end(), { 0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 5.5f });
    colors.insert(colors.end(), { 0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f });
}

// Build a central point (small cube/point)
void buildCentralPointGeometry(std::vector<float>& verts)
{
    // A small point at origin - 6 points for 2 triangles
    float s = 0.05f;
    verts.insert(verts.end(), {
        -s, -s, 0.0f,   s, -s, 0.0f,   s,  s, 0.0f,
        -s, -s, 0.0f,   s,  s, 0.0f,  -s,  s, 0.0f
    });
}

// ============================================================================
// Axis Cones Shape Class
// ============================================================================
class AxisCones
{
public:
    Dynamit xCone, yCone, zCone;
    std::vector<float> coneVerts, coneNorms;
    
    void build()
    {
		std::wcout << L"Building axis cones..." << coneVerts.size() << L" vertices, " << coneNorms.size() << L" normals." << std::endl;
        // Build cone geometry (radius=0.1, height=0.2 scaled)
        // auxSolidCone(0.1, 0.2) -> scale accordingly
        Builder::polar()
            .sectors_slices(16, 4)
            .doubleCoated()
            .buildCone(coneVerts, coneNorms);
        
        // Scale cone to match auxSolidCone(0.1f, 0.2f)
        // Default cone is radius=1, height=1 along -Z
        mat4<float> coneScale = scaleMatrix(0.1f, 0.1f, 0.2f);
        applyTransformToRange(coneScale, coneVerts, coneNorms, 0);
        
        // X cone (red) - translated to (5.3, 0, 0), rotated 90° around Y
        xCone.withVertices3d(coneVerts)
             .withNormals3d(coneNorms)
             .withConstColor(1.0f, 0.0f, 0.0f, 1.0f)
             .withConstLightDirection(-0.577f, -0.577f, 0.577f)
             .withTransformMatrix4f();
        
        // Y cone (green) - translated to (0, 5.3, 0), rotated -90° around X
        yCone.withVertices3d(coneVerts)
             .withNormals3d(coneNorms)
             .withConstColor(0.0f, 1.0f, 0.0f, 1.0f)
             .withConstLightDirection(-0.577f, -0.577f, 0.577f)
             .withTransformMatrix4f();
        
        // Z cone (blue) - translated to (0, 0, 5.3), no rotation needed
        zCone.withVertices3d(coneVerts)
             .withNormals3d(coneNorms)
             .withConstColor(0.0f, 0.0f, 1.0f, 1.0f)
             .withConstLightDirection(-0.577f, -0.577f, 0.577f)
             .withTransformMatrix4f();
    }
    
    void draw(const mat4<float>& viewProjection)
    {
        mat4<float> transform;
        
        // X cone: translate then rotate 90° around Y (points along +X)
        transform = identity_mat4<float>();
        rotate_y_mat4(static_cast<float>(M_PI / 2), transform);
        mat4<float> translateX = translation_mat4(5.3f, 0.0f, 0.0f);
        multiply_mat4(translateX, transform);
        multiply_mat4(viewProjection, transform);
        xCone.transformMatrix4f(transform);
        xCone.drawTriangles();
        
        // Y cone: translate then rotate -90° around X (points along +Y)
        transform = identity_mat4<float>();
        rotate_x_mat4(static_cast<float>(-M_PI / 2), transform);
        mat4<float> translateY = translation_mat4(0.0f, 5.3f, 0.0f);
        multiply_mat4(translateY, transform);
        multiply_mat4(viewProjection, transform);
        yCone.transformMatrix4f(transform);
        yCone.drawTriangles();
        
        // Z cone: translate only (already points along -Z, flip to +Z)
        transform = identity_mat4<float>();
        rotate_x_mat4(static_cast<float>(M_PI), transform); // Flip to point +Z
        mat4<float> translateZ = translation_mat4(0.0f, 0.0f, 5.3f);
        multiply_mat4(translateZ, transform);
        multiply_mat4(viewProjection, transform);
        zCone.transformMatrix4f(transform);
        zCone.drawTriangles();
    }
};

// ============================================================================
// Lines Shape (for axes)
// ============================================================================
class AxisLines
{
public:
    Dynamit lines;
    
    void build()
    {
        std::vector<float> verts, colors;
        buildAxisGeometry(verts, colors);
        
        lines.withVertices3d(verts)
             .withColors3d(colors)
             .withTransformMatrix4f()
             .withPrimitive(GL_LINES);
    }
    
    void draw(const mat4<float>& viewProjection)
    {
        lines.transformMatrix4f(viewProjection);
        lines.useProgram();
        lines.bindVertexArray();
        glDrawArrays(GL_LINES, 0, 6);
    }
};

// ============================================================================
// Central Point Shape
// ============================================================================
class CentralPoint
{
public:
    Dynamit point;
    
    void build()
    {
        std::vector<float> verts;
        buildCentralPointGeometry(verts);
        
        point.withVertices3d(verts)
             .withConstColor(1.0f, 1.0f, 1.0f, 1.0f)
             .withTransformMatrix4f();
    }
    
    void draw(const mat4<float>& viewProjection)
    {
        point.transformMatrix4f(viewProjection);
        point.drawTriangles();
    }
};
} // namespace auxDisplayDynamit
// ============================================================================
// Main Application
// ============================================================================
int main_auxDisplayDynamit()
{
    // Initialize GLFW + OpenGL context
    GLFWwindow* window = openglWindowInit(800, 600);
    if (!window)
        return -1;
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    
    // Build shapes
    auxDisplayDynamit::AxisCones axisCones;
    axisCones.build();
    
    auxDisplayDynamit::AxisLines axisLines;
    axisLines.build();
    
    auxDisplayDynamit::CentralPoint centralPoint;
    centralPoint.build();
    
    // Setup OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    
    // Simple view/projection setup (replaces spacePrepare)
    // Orthographic projection similar to original
    float aspect = 800.0f / 600.0f;
    float size = 8.0f;
    
    // Render loop
    TimeController tc(static_cast<float>(glfwGetTime()));
    float rotationY = 0.0f;
    
    while (!glfwWindowShouldClose(window))
    {
        tc.update(static_cast<float>(glfwGetTime()));
        
        // Handle rotation with arrow keys
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            rotationY -= static_cast<float>(tc.deltaTime) * 1.0f;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            rotationY += static_cast<float>(tc.deltaTime) * 1.0f;
        
        processInputs(window);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Build view-projection matrix (simple orthographic + rotation)
        mat4<float> viewProjection = identity_mat4<float>();
        
        // Scale to fit view
        mat4<float> scale = scaleMatrix(1.0f / size, 1.0f / size * aspect, 1.0f / size);
        multiply_mat4(scale, viewProjection);
        
        // Rotate view
        rotate_y_mat4(rotationY, viewProjection);
        rotate_x_mat4(0.3f, viewProjection); // Slight tilt for 3D view
        
        // Draw all elements
        centralPoint.draw(viewProjection);
        axisLines.draw(viewProjection);
        axisCones.draw(viewProjection);
        
        // TODO: Add lab1GlDisplay() equivalent here
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}
#include "enabler.h"
#ifdef __MAIN_AUXDISPLAYDYNAMIT_CPP__
int main() {return main_auxDisplayDynamit();}
#endif