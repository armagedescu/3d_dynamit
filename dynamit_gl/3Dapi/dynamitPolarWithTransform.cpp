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
#include <builders.h>

using namespace dynamit;
using namespace dynamit::builders;

int main_dynamitPolarWithTransform()
{
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;

    std::cout << glGetString(GL_VERSION) << std::endl;

    std::vector<float> verts, norms;
    std::vector<float> vertsIndexed, normsIndexed;
    std::vector<uint32_t> indices;

    //// Create transformation matrices for positioning multiple shapes

    mat4<float> translate = translation_mat4(-0.5f, 0.0f, 0.0f);
    mat4<float> rotate = rotation_z_mat4(M_PI / 4);
    mat4<float> scale = scaleMatrix(0.5f, 0.5f, 1.0f);
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
            .withVertices3d(verts)
            .withNormals3d(norms)
            .withConstColor({ 0.0, 1.0, 0.5, 1.0 })
            .withConstLightDirection({ -0.577f, -0.577f, 0.577f })
            .withTransformMatrix4f()
        ;

    if (vertsIndexed.size() > 0)
        shapeIndexed
            .withVertices3d(vertsIndexed)
            .withNormals3d(normsIndexed)
            .withIndices(indices)
            .withConstColor({ 0.0, 1.0, 0.5, 1.0 })
            .withConstLightDirection({ -0.577f, -0.577f, 0.577f })
            .withTransformMatrix4f()
        ;


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 1.f, 0.9f);

    mat4<float> mat4Transform = {};

    // Render loop
	float angle = 0.f;
	TimeController tc(glfwGetTime());
    while (!glfwWindowShouldClose(window))
    {
        tc.update(glfwGetTime());

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) angle += static_cast<float>(tc.deltaTime) * 0.5f; // slow rotation
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) angle += static_cast<float>(tc.deltaTime) * -0.5f; // slow rotation

        glPolygonMode(GL_FRONT_AND_BACK, glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS ? GL_LINE : GL_FILL);

        processInputs(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        rotation_x_mat4(angle, mat4Transform);

        switch (currentShape)
        {
        case DRAW_1:
            if (verts.size() > 0)
            {
                shape.transformMatrix4f(mat4Transform);
                shape.drawTriangles();
            }
            break;
        case DRAW_2:
            if (vertsIndexed.size() > 0)
            {
                shapeIndexed.transformMatrix4f(mat4Transform);
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
#ifdef __DYNAMIT_POLAR_WITH_TRANSFORM_CPP__
int main() { return main_dynamitPolarWithTransform(); }
#endif