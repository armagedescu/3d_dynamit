#include "enabler.h"
#ifdef __DYNAMIT_CYLINDER_GEOMETRY_CALC_CPP__

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

    bool buildHeart = false, buildCardoid = false, buildEllipse = false, 
        build5PointerStar = false, build5PetalRose = false, buildLemniscate = true;
    if (buildHeart)
    {
        Builder::polar()
            .formula(L"theta / PI")             // first half
            .domain(M_PI)
            .sectors_slices(60, 10)
            .buildCylinder(verts, norms)
            .formula(L"(2*PI - theta) / PI")    // second half
            .domain_shift(2 * M_PI)
            .sectors_slices(60, 10)
            .buildCylinder(verts, norms)
            // indexed version
            .formula(L"theta / PI")
            .domain(static_cast<float>(M_PI))
            .sectors_slices(30, 5)
            .buildCylinderIndexed(vertsIndexed, normsIndexed, indices)
            .formula(L"(2*PI - theta) / PI")
            .domain_shift(static_cast<float>(2 * M_PI))
            .sectors_slices(30, 5)
            .buildCylinderIndexed(vertsIndexed, normsIndexed, indices);
    }
    else if (buildCardoid)
    {
        Builder::polar()
            .formula(L"(1 - cos(theta)) / (PI / 1.55)")
            .sectors_slices(60, 10)
            .buildCylinder(verts, norms)
            .sectors_slices(30, 5)
            .buildCylinderIndexed(vertsIndexed, normsIndexed, indices);
    }
    else if (buildEllipse)
    {
        Builder::polar()
            .formula(L"2 / sqrt(4 * sin(theta)**2 + cos(theta)**2) / 2")
            .sectors_slices(60, 10)
            .buildCylinder(verts, norms)
            .sectors_slices(30, 5)
            .buildCylinderIndexed(vertsIndexed, normsIndexed, indices);
    }
    else if (build5PointerStar)
    {
        Builder::polar()
            .formula(L"(1 + 0.5 * cos(5 * theta)) / 1.5")
            .sectors_slices(60, 10)
            .buildCylinder(verts, norms)
            .sectors_slices(30, 5)
            .buildCylinderIndexed(vertsIndexed, normsIndexed, indices);
    }
    else if (build5PetalRose)
    {
        Builder::polar()
            .formula(L"cos(5 * theta)")
            .sectors_slices(160, 10)
            .buildCylinder(verts, norms)
            .sectors_slices(60, 5)
            .buildCylinderIndexed(vertsIndexed, normsIndexed, indices);
    }
    else if (buildLemniscate)
    {
        Builder::polar()
            .formula(L"sqrt(abs(cos(2 * theta)))")
            .sectors_slices(60, 10)
            .buildCylinder(verts, norms)
            .sectors_slices(30, 5)
            .buildCylinderIndexed(vertsIndexed, normsIndexed, indices);
    }

    std::cout << "Cylinder vertices: " << verts.size() / 3 << " (triangles: " << verts.size() / 9 << ")" << std::endl;
    std::cout << "Cylinder indexed vertices: " << vertsIndexed.size() / 3 << " (indices: " << indices.size() << ")" << std::endl;

    // Create shape with custom shaders for X rotation
    Dynamit shape, shapeIndexed;

    if (verts.size() > 0)
    {
        shape
            .withShaderSources(vertexShaderRotateX, fragmentShaderLit)
            .withVertices3d(verts)
            .withNormals3d(norms);
    }

    if (vertsIndexed.size() > 0)
    {
        shapeIndexed
            .withShaderSources(vertexShaderRotateX, fragmentShaderLit)
            .withVertices3d(vertsIndexed)
            .withNormals3d(normsIndexed)
            .withIndices(indices);
    }

    // Build programs and get uniform locations
    shape.buildProgram();
    shapeIndexed.buildProgram();
    GLint rotationLocShape = glGetUniformLocation(shape.program.id, "rotationAngle");
    GLint rotationLocIndexed = glGetUniformLocation(shapeIndexed.program.id, "rotationAngle");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 1.f, 0.9f);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        glPolygonMode(GL_FRONT_AND_BACK, glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS ? GL_LINE : GL_FILL);

        processInputs(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float angle = static_cast<float>(glfwGetTime()) * 0.5f; // slow rotation

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