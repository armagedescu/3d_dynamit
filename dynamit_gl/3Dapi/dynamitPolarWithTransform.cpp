#include "enabler.h"
#ifdef __DYNAMIT_POLAR_WITH_TRANSFORM_CPP__

#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Dynamit.h>
#include <geometry.h>
#include <config.h>
#include <callbacks.h>
#include <builders.h>

using namespace dynamit;
using namespace dynamit::builders;

// Helper to create a translation matrix (column-major)
Matrix4 translationMatrix(float tx, float ty, float tz)
{
    return {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        tx,   ty,   tz,   1.0f
    };
}

// Helper to create a scale matrix (column-major)
Matrix4 scaleMatrix(float sx, float sy, float sz)
{
    return {
        sx,   0.0f, 0.0f, 0.0f,
        0.0f, sy,   0.0f, 0.0f,
        0.0f, 0.0f, sz,   0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

// Helper to create a rotation matrix around Z axis (column-major)
Matrix4 rotationZMatrix(float angle)
{
    float c = std::cos(angle);
    float s = std::sin(angle);
    return {
        c,    s,    0.0f, 0.0f,
        -s,   c,    0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

const char* vertexShaderRotateX = R"(
#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
out vec3 normalVary;

uniform float rotationAngle;

void main()
{
    float c = cos(rotationAngle);
    float s = sin(rotationAngle);
    mat3 rotX = mat3(
        1.0, 0.0, 0.0,
        0.0,   c,  -s,
        0.0,   s,   c
    );
    vec3 rotatedPos = rotX * vertex;
    vec3 rotatedNorm = rotX * normal;
    gl_Position = vec4(rotatedPos, 1.0);
    normalVary = rotatedNorm;
}
)";

const char* fragmentShaderLit = R"(
#version 330 core
precision mediump float;
out vec4 fragColor;
in vec3 normalVary;

const vec4 constColor = vec4(0.0, 1.0, 0.5, 1.0);
const vec3 lightDirection = vec3(-0.577, -0.577, 0.577);

void main()
{
    float prod = -dot(normalize(lightDirection), normalize(normalVary));
    fragColor = vec4(constColor.rgb * prod, 1.0);
}
)";

int main()
{
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;

    std::cout << glGetString(GL_VERSION) << std::endl;

    std::vector<float> verts, norms;
    std::vector<float> vertsIndexed, normsIndexed;
    std::vector<uint32_t> indices;

    // Create transformation matrices for positioning multiple shapes
    Matrix4 leftTransform  = translationMatrix(-0.5f, 0.0f, 0.0f);
    Matrix4 rightTransform = translationMatrix(0.5f, 0.0f, 0.0f);
        
    Matrix4 translate = translationMatrix(-0.5f, 0.0f, 0.0f);
    Matrix4 rotate = rotationZMatrix(M_PI / 4);
    Matrix4 scale = scaleMatrix(0.5f, 0.5f, 1.0f);
    Builder::polar()
        .edged(false).reversed(false).doubleCoated().turbo(true)
        .formula(L"theta / PI")             // first half
        .domain(M_PI)
        .sectors_slices(6, 2)
        .buildCylinder(verts, norms, translate, rotate, scale)
        .formula(L"(2*PI - theta) / PI")    // second half
        .domain_shift(2 * M_PI)
        .buildCylinder(verts, norms, translate, rotate, scale)
        //////// indexed version
        .formula(L"theta / PI")
        .domain(static_cast<float>(M_PI))
        .sectors_slices(10, 5)
        .buildCylinderIndexed(vertsIndexed, normsIndexed, indices)
        .formula(L"(2*PI - theta) / PI")
        .domain_shift(static_cast<float>(2 * M_PI))
        .buildCylinderIndexed(vertsIndexed, normsIndexed, indices)
    ;

    std::cout << "Cylinder vertices: " << verts.size() / 3 << " (triangles: " << verts.size() / 9 << ")" << std::endl;
    std::cout << "Cylinder indexed vertices: " << vertsIndexed.size() / 3 << " (indices: " << indices.size() << ")" << std::endl;

    // Create shape with custom shaders for X rotation
    Dynamit shape, shapeIndexed;

    if (verts.size() > 0)
        shape
            .withShaderSources(vertexShaderRotateX, fragmentShaderLit)
            .withVertices3d(verts)
            .withNormals3d(norms);

    if (vertsIndexed.size() > 0)
        shapeIndexed
            .withShaderSources(vertexShaderRotateX, fragmentShaderLit)
            .withVertices3d(vertsIndexed)
            .withNormals3d(normsIndexed)
            .withIndices(indices);

    // Build programs and get uniform locations
    GLint rotationLocShape = -1;
    GLint rotationLocIndexed = -1;
    if (!verts.empty()) 
    {
        shape.buildProgram();
        rotationLocShape = glGetUniformLocation(shape.program.id, "rotationAngle");
    }
    if (!vertsIndexed.empty())
    {
        shapeIndexed.buildProgram();
        rotationLocIndexed = glGetUniformLocation(shapeIndexed.program.id, "rotationAngle");
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 1.f, 0.9f);

    // Render loop
    double time = glfwGetTime();
	float angle = 0.f;
    while (!glfwWindowShouldClose(window))
    {
		double currentTime = glfwGetTime();
		double deltaTime = currentTime - time;
		time = currentTime;
        if (glfwGetKey(window, GLFW_KEY_F12) == GLFW_PRESS) angle += static_cast<float>(deltaTime) * (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? -0.5 : 0.5f); // slow rotation
        glPolygonMode(GL_FRONT_AND_BACK, glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS ? GL_LINE : GL_FILL);

        processInputs(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //float angle = 0.f; //static_cast<float>(glfwGetTime()) * 0.5f; // slow rotation
        //float angle = static_cast<float>(glfwGetTime()) * 0.5f; // slow rotation

        switch (currentShape)
        {
        case DRAW_1:
            if (verts.size() > 0)
            {
                shape.useProgram();
                glUniform1f(rotationLocShape, angle);
                shape.drawTriangles();
            }
            break;
        case DRAW_2:
            if (vertsIndexed.size() > 0)
            {
                shapeIndexed.useProgram();
                glUniform1f(rotationLocIndexed, angle);
                shapeIndexed.drawTrianglesIndexed();
            }
            break;
        }

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

#endif