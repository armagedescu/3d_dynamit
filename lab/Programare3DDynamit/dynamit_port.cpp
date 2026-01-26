#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <array>

#include <Dynamit.h>
#include <builders.h>
#include <geometry.h>
#include <config.h>
#include <callbacks.h>

using namespace dynamit;
using namespace dynamit::builders;
using namespace dynamit::geo;

namespace dynamit_port
{
// ============================================================================
// Constants (from constante.h)
// ============================================================================
constexpr int MAXH = 150;
constexpr int MAXS = 150;

// ============================================================================
// Axis Lines Shape
// ============================================================================
class AxisLines
{
public:
    Dynamit lines;
    
    void build()
    {
        // X axis (black to red), Y axis (black to green), Z axis (black to blue)
        std::vector<float> verts = {
            -5.5f, 0.0f, 0.0f,   5.5f, 0.0f, 0.0f,  // X
             0.0f,-5.5f, 0.0f,   0.0f, 5.5f, 0.0f,  // Y
             0.0f, 0.0f,-5.5f,   0.0f, 0.0f, 5.5f   // Z
        };
        std::vector<float> colors = {
            0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,  // X: black to red
            0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,  // Y: black to green
            0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f   // Z: black to blue
        };
        
        lines.withVertices3d(verts)
             .withColors3d(colors)
             .withTransformMatrix4f();
    }
    
    void draw(const mat4<float>& transform)
    {
        lines.transformMatrix4f(transform);
        lines.useProgram();
        lines.bindVertexArray();
        glDrawArrays(GL_LINES, 0, 6);
    }
};

// ============================================================================
// Axis Cones
// ============================================================================
class AxisCones
{
public:
    Dynamit xCone, yCone, zCone;
    
    void build()
    {
        std::vector<float> verts, norms;
        
        // Build small cone for axis tips (like auxSolidCone(0.1, 0.2))
        Builder::polar()
            .sectors_slices(16, 4)
            .doubleCoated()
            .buildCone(verts, norms,
                       scaleMatrix(0.1f, 0.1f, 0.2f));
        
        // X cone (red)
        xCone.withVertices3d(verts)
             .withNormals3d(norms)
             .withConstColor(1.0f, 0.0f, 0.0f, 1.0f)
             .withConstLightDirection(-0.577f, -0.577f, 0.577f)
             .withTransformMatrix4f();
        
        // Y cone (green)
        yCone.withVertices3d(verts)
             .withNormals3d(norms)
             .withConstColor(0.0f, 1.0f, 0.0f, 1.0f)
             .withConstLightDirection(-0.577f, -0.577f, 0.577f)
             .withTransformMatrix4f();
        
        // Z cone (blue)
        zCone.withVertices3d(verts)
             .withNormals3d(norms)
             .withConstColor(0.0f, 0.0f, 1.0f, 1.0f)
             .withConstLightDirection(-0.577f, -0.577f, 0.577f)
             .withTransformMatrix4f();
    }
    
    void draw(const mat4<float>& viewProj)
    {
        mat4<float> t;
        
        // X cone: at (5.3, 0, 0), rotated 90° around Y
        t = identity_mat4<float>();
        rotate_y_mat4(static_cast<float>(M_PI / 2), t);
        multiply_mat4(translation_mat4(5.3f, 0.0f, 0.0f), t);
        multiply_mat4(viewProj, t);
        xCone.transformMatrix4f(t);
        xCone.drawTriangles();
        
        // Y cone: at (0, 5.3, 0), rotated -90° around X
        t = identity_mat4<float>();
        rotate_x_mat4(static_cast<float>(-M_PI / 2), t);
        multiply_mat4(translation_mat4(0.0f, 5.3f, 0.0f), t);
        multiply_mat4(viewProj, t);
        yCone.transformMatrix4f(t);
        yCone.drawTriangles();
        
        // Z cone: at (0, 0, 5.3), rotated 180° around X (flip direction)
        t = identity_mat4<float>();
        rotate_x_mat4(static_cast<float>(M_PI), t);
        multiply_mat4(translation_mat4(0.0f, 0.0f, 5.3f), t);
        multiply_mat4(viewProj, t);
        zCone.transformMatrix4f(t);
        zCone.drawTriangles();
    }
};

// ============================================================================
// Central Point
// ============================================================================
class CentralPoint
{
public:
    Dynamit point;
    
    void build()
    {
        // Small quad at origin (simulates GL_POINTS)
        float s = 0.08f;
        std::vector<float> verts = {
            -s, -s, 0.0f,   s, -s, 0.0f,   s,  s, 0.0f,
            -s, -s, 0.0f,   s,  s, 0.0f,  -s,  s, 0.0f
        };
        
        point.withVertices3d(verts)
             .withConstColor(0.0f, 0.0f, 0.0f, 1.0f)
             .withTransformMatrix4f();
    }
    
    void draw(const mat4<float>& transform)
    {
        point.transformMatrix4f(transform);
        point.drawTriangles();
    }
};

// ============================================================================
// Heart Cone Shape (main display object from lab1)
// ============================================================================
const char* vertexCameraShaderSource = R"glsl(#version 330 core
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
out vec3 normalVary;
uniform mat4 transformMatrix;

void main()
{
    gl_Position = transformMatrix * vec4(vertex, 1);
    normalVary = mat3(transformMatrix) * normal;
})glsl";
//== = FRAGMENT SHADER(GENERATED) == =
const char* fragmentCameraShaderSource = R"glsl(#version 330 core
precision mediump float;
out vec4 fragColor;
in vec3 normalVary;
const vec4 constColor = vec4(1, 0, 1, 1);
const vec3 lightDirection = vec3(-0.577, -0.577, 0.577);

void main()
{
    float prod = -dot(normalize (lightDirection), normalize(normalVary)  );
    fragColor = vec4(constColor.rgb * prod, 1.0);
})glsl";

class HeartConeShape
{
public:
    Dynamit cone;
    
    void build(int nh = 8, int ns = 15)
    {
        std::vector<float> verts, norms;
        
        // Apply the shear/scale matrix from original draw()
        // s = { 1, 0, 0, 0,  0, 0.95, 0, 0,  0.5, 0, 1, 0,  0, 0, 0, 1 }
        mat4<float> shear = {
            1.0f,  0.0f,  0.0f, 0.0f,
            0.0f,  1.0f,  0.0f, 0.0f,
           -0.0f,  0.0f,  1.0f, 0.0f,
            0.0f,  0.0f,  0.0f, 1.0f
        };
        
        std::vector<float> dummyNorms = norms;

        Builder::polar().doubleCoated().reversed()
            .formula(L"theta / PI")
            .domain(M_PI)                       // first half
            .sectors_slices(10, 10)
            .buildCone(verts, norms) //, shear)
            .formula(L"(2*PI - theta) / PI")    // second half
            .domain_shift(2 * M_PI)
            .buildCone(verts, norms) //, shear)
            ;

		cone //.withShaderSources(vertexCameraShaderSource, fragmentCameraShaderSource)
            .withVertices3d(verts)
            .withNormals3d(norms)
            .withConstColor(1.0f, 0.0f, 1.0f, 1.0f)  // Magenta like original
            .withConstLightDirection(-0.577f, -0.577f, 0.577f)
            .withTransformMatrix4f();
		cone.logGeneratedShaders("HeartConeShape shaders: ");
    }
    
    void draw(const mat4<float>& transform)
    {
        cone.transformMatrix4f(transform);
        cone.drawTriangles();
    }
};

// ============================================================================
// Lab1 Scene - Combines all elements
// ============================================================================
class Lab1Scene
{
public:
    AxisLines axisLines;
    AxisCones axisCones;
    CentralPoint centralPoint;
    HeartConeShape heartCone;
    
    void build()
    {
        axisLines.build();
        axisCones.build();
        centralPoint.build();
        heartCone.build(8, 15);
    }
    
    void drawHeartConeInstance(const mat4<float>& viewProj,
                               float translateZ, float scaleXY, float scaleZ,
                               float rotX = 0.0f, float rotZ = 0.0f)
    {
        mat4<float> t = identity_mat4<float>();
        //multiply_mat4(scaleMatrix(scaleXY, scaleXY, scaleZ), t);
        if (rotX != 0.0f) rotate_x_mat4(rotX, t);
        if (rotZ != 0.0f) rotate_z_mat4(rotZ, t);
        multiply_mat4(translation_mat4(0.0f, 0.0f, translateZ), t);
        multiply_mat4(viewProj, t);
        heartCone.draw(t);
    }
    
    void draw(const mat4<float>& viewProj)
    {
        // Draw axes
        centralPoint.draw(viewProj);
        axisLines.draw(viewProj);
        axisCones.draw(viewProj);
        
        // Draw heart cone instances (from lab1GlDisplay)
        // directDisplay: translate(0,0,0), scale(2,2,3)
        drawHeartConeInstance(viewProj, 0.0f, 2.0f, 3.0f);
        
        //// mirrorDisplay: rotate(180,1,0,0), rotate(180,0,0,1)
        drawHeartConeInstance(viewProj, 0.0f, 2.0f, 3.0f, static_cast<float>(M_PI), static_cast<float>(M_PI));
        
        //// mirrorDisplay2: rotate(90,1,0,0)
        //drawHeartConeInstance(viewProj, 0.0f, 2.0f, 3.0f,
        //                      static_cast<float>(M_PI / 2), 0.0f);
        //
        //// mirrorDisplay3: rotate(270,1,0,0), rotate(180,0,0,1)
        //drawHeartConeInstance(viewProj, 0.0f, 2.0f, 3.0f,
        //                      static_cast<float>(3 * M_PI / 2), static_cast<float>(M_PI));
    }
};
} // namespace dynamit_port
// ============================================================================
// Main Application
// ============================================================================


int main_dynamit_port()
{
    GLFWwindow* window = openglWindowInit(800, 600);
    if (!window)
        return -1;
    scope_guard deleter = scope_guard(glfwTerminate);

    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // Build scene
    dynamit_port::Lab1Scene scene;
    scene.build();
    //return 0;
    
    // OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.4f, 0.5f, 0.6f, 1.0f);
    
    // Animation state (replaces getDFi())
    float anglex = 0.0f, angley = 0.0f;
    TimeController tc(static_cast<float>(glfwGetTime()));
    
    while (!glfwWindowShouldClose(window))
    {
        tc.update(static_cast<float>(glfwGetTime()));
        //rotationAngle += 0.01f;  // Same increment as getDFi()
        //angley += static_cast<float>(tc.deltaTime) * 0.5f;  // Same increment as getDFi()
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) anglex += static_cast<float>(tc.deltaTime) * 0.5f; // slow rotation
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) anglex += static_cast<float>(tc.deltaTime) * -0.5f; // slow rotation
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) angley += static_cast<float>(tc.deltaTime) * 0.5f; // slow rotation
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) angley += static_cast<float>(tc.deltaTime) * -0.5f; // slow rotation

        processInputs(window);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Build view-projection (replaces spacePrepare + glOrtho)
        // Original: glOrtho(-6.2, 6.2, -6.2, 6.2, 2, 12), translate(0,0,-6), rotate(35+dFi, 1,0,0), rotate(-35+dFi, 0,1,0)
        //mat4<float> viewProj = identity_mat4<float>();
        float orthoScale = 1.0f / 2.f;// 6.2f;
        mat4<float> viewProj = scaleMatrix(orthoScale, orthoScale, orthoScale);// / 2.0f);
        
        // Orthographic scale (maps -6.2..6.2 to -1..1)
        //multiply_mat4(scaleMatrix(orthoScale, orthoScale, orthoScale / 2.0f), viewProj);
        
        // View rotation (like spacePrepare)
        rotate_x_mat4(angley, viewProj);
        //rotate_x_mat4((1.0f + rotationAngle) * static_cast<float>(M_PI) / 180.0f, viewProj);
        //rotate_y_mat4((-1.0f + rotationAngle) * static_cast<float>(M_PI) / 180.0f, viewProj);
		mat4<float> accum = identity_mat4<float>();
		multiply_mat4(scaleMatrix(orthoScale, orthoScale, orthoScale / 2.0f), accum);
		rotate_x_mat4(anglex, accum);
        // Draw everything
        scene.draw(accum);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    return 0;
}

#include "enabler.h"
#ifdef __DYNAMIT_PORT_CPP__
int main() { return main_dynamit_port(); }
#endif