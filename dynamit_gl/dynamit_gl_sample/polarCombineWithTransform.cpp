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

int main_polarCombineWithTransform()
{
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;

    std::cout << glGetString(GL_VERSION) << std::endl;

    std::vector<float> verts, norms;
    std::vector<float> vertsIndexed, normsIndexed;
    std::vector<uint32_t> indices;

    float arrowHeadHeight = 0.1f, arrowHeadWidth = 0.05f, arrowShaftWidth = 0.025f;
    mat4<float> arrowShaftTranslate = translation_mat4(0.0f, 0.0f, 1.0f);
    mat4<float> arrowShaftScale = scaleMatrix(arrowShaftWidth, arrowShaftWidth, 2.0f - arrowHeadHeight);
    mat4<float> arrowTipScale = scaleMatrix(arrowHeadWidth, arrowHeadWidth, arrowHeadHeight);
    mat4<float> arrowTipTranslate = translation_mat4(0.0f, 0.0f, -1.0f + arrowHeadHeight);

	Builder::polar().doubleCoated()//.reversed(true).edged()
        .formula(L"1")             // first half
        //.formula(L"theta / PI")             // first half
        .sectors_slices(6, 2)
        .buildCone(verts, norms, arrowTipScale, arrowTipTranslate)
        .buildCylinder(verts, norms, arrowShaftScale, arrowShaftTranslate)// , translate, scale)
        //////// indexed version
        .sectors_slices(10, 5)
        .buildConeIndexed(vertsIndexed, normsIndexed, indices, arrowTipScale, arrowTipTranslate)
        .buildCylinderIndexed(vertsIndexed, normsIndexed, indices, arrowShaftScale, arrowShaftTranslate)
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
#include "enabler.h"
#ifdef __POLAR_COMBINE_WITH_TRANSFORM_CPP__
int main() { return main_polarCombineWithTransform(); }
#endif