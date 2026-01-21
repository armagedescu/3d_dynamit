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
using namespace dynamit::geo;

int main_polarArrowCombined()
{
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;

    std::cout << glGetString(GL_VERSION) << std::endl;

    std::vector<float> verts, norms;
    std::vector<uint32_t> indices;

    // Create transformation matrices for positioning multiple shapes
    mat4<float> leftTransform  = translation_mat4(-0.5f, 0.0f, 0.0f);
    mat4<float> rightTransform = translation_mat4(0.5f, 0.0f, 0.0f);

	float arrowHeadHeight = 0.1f, arrowHeadWidth = 0.05f, arrowShaftWidth = 0.025f;
    mat4<float> arrowShaftTranslate = translation_mat4(0.0f, 0.0f, 1.0f);
    mat4<float> arrowShaftScale = scaleMatrix(arrowShaftWidth, arrowShaftWidth, 2.0f - arrowHeadHeight);
    mat4<float> arrowTipScale = scaleMatrix(arrowHeadWidth, arrowHeadWidth, arrowHeadHeight);
    mat4<float> arrowTipTranslate = translation_mat4(0.0f, 0.0f, -1.0f + arrowHeadHeight);

	Builder::polar()
        .formula(L"1")
        .sectors_slices(10, 5)
        .buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate)
        .buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate)
    ;

    // Create shape with custom shaders for X rotation
    Dynamit shape;
    shape
        .withVertices3d(verts)
        .withNormals3d(norms)
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
	TimeController tc(glfwGetTime());
	float angle = 0.f;
    while (!glfwWindowShouldClose(window))
    {
		tc.update(glfwGetTime());
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) angle += static_cast<float>(tc.deltaTime) * 0.5f; // slow rotation
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) angle += static_cast<float>(tc.deltaTime) * -0.5f; // slow rotation

        glPolygonMode(GL_FRONT_AND_BACK, glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS ? GL_LINE : GL_FILL);

        processInputs(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shape.useProgram();

        rotation_x_mat4(angle, mat4Transform);

        shape.transformMatrix4f(mat4Transform);
        shape.drawTrianglesIndexed();

        shape.drawTrianglesIndexed();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
#include "enabler.h"
#ifdef __POLAR_ARROW_COMBINED_CPP__
int main() { return main_polarArrowCombined(); }
#endif