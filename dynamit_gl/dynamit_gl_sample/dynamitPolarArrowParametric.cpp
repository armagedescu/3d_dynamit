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


int main_dynamitPolarArrowParametric()
{
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;

    std::cout << glGetString(GL_VERSION) << std::endl;

    std::vector<float> verts, norms, verts1, norms1, verts2, norms2;
    std::vector<uint32_t> indices, indices1, indices2 ;

    // Create transformation matrices for positioning multiple shapes

	float arrowHeadHeight = 0.1f, arrowHeadWidth = 0.05f, arrowShaftWidth = 0.025f;
	//float arrowHeadHeight = 0.2f, arrowHeadWidth = 0.1f, arrowShaftWidth = 0.025f;
    mat4<float> arrowShaftTranslate = translation_mat4(0.0f, 0.0f, 1.0f);
    mat4<float> arrowShaftScale = scaleMatrix(arrowShaftWidth, arrowShaftWidth, 2.0f - arrowHeadHeight);
    mat4<float> arrowTipScale = scaleMatrix(arrowHeadWidth, arrowHeadWidth, arrowHeadHeight);
    mat4<float> arrowTipTranslate = translation_mat4(0.0f, 0.0f, -1.0f + arrowHeadHeight);
    bool buildCircle = false, buildHeart = false, build5PetalRose = true;

    PolarBuilder builder = Builder::polar();

    if (buildCircle)
    {
        builder.doubleCoated()
            .sectors_slices(100, 100)
			.buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(-M_PI / 2))
            .buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(-M_PI / 2))
			.buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate, rotation_y_mat4(-M_PI / 2))
            .buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate, rotation_y_mat4(-M_PI / 2))
			.buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(M_PI))
            .buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(M_PI))
            ;
    }else  if (buildHeart)
    {
        builder.doubleCoated()
            .formula(L"theta / PI")
            .domain(M_PI)                       // first half
            .sectors_slices(100, 100)
            //.buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate)
            //.buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate)
            .buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(-M_PI / 2))
            .buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(-M_PI / 2))
            .buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate, rotation_y_mat4(-M_PI / 2))
            .buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate, rotation_y_mat4(-M_PI / 2))
            .buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(M_PI))
            .buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(M_PI))

            .formula(L"(2*PI - theta) / PI")    // second half
            .domain_shift(2 * M_PI)
            //.buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate)
            //.buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate)
            .buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(-M_PI / 2))
            .buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(-M_PI / 2))
            .buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate, rotation_y_mat4(-M_PI / 2))
            .buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate, rotation_y_mat4(-M_PI / 2))
            .buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(M_PI))
            .buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(M_PI))

            ;
    }else if (build5PetalRose)
    {
        builder.doubleCoated()
            .formula(L"cos(5 * theta)")
            .sectors_slices(100, 100)
            //.buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate)
            //.buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate)
            .buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(-M_PI / 2))
            .buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(-M_PI / 2))
            .buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate, rotation_y_mat4(-M_PI / 2))
            .buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate, rotation_y_mat4(-M_PI / 2))
            .buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(M_PI))
            .buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(M_PI))

            ;
    }

    // Setup:
    Dynamit shape;
    shape
        .withVertices3d(verts)
        .withNormals3d(norms)
        .withIndices(indices)
        .withConstColor({ 0.0, 1.0, 0.5, 1.0 })
		.withConstLightDirection({ -0.577f, -0.577f, 0.577f })
        .withTransformMatrix4f()
        //.withTransformMatrix3f()
        ;
	shape.logGeneratedShaders();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 1.f, 0.9f);

    mat4<float> mat4Transform = {};
    // Render loop
    float anglex = 0.f, angley = 0.f, anglez = 0.f;
	TimeController tc(glfwGetTime());
    while (!glfwWindowShouldClose(window))
    {
		tc.update(glfwGetTime());
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) anglex += static_cast<float>(tc.deltaTime) * 0.5f; // slow rotation
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) anglex += static_cast<float>(tc.deltaTime) * -0.5f; // slow rotation
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) angley += static_cast<float>(tc.deltaTime) * 0.5f; // slow rotation
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) angley += static_cast<float>(tc.deltaTime) * -0.5f; // slow rotation
        glPolygonMode(GL_FRONT_AND_BACK, glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS ? GL_LINE : GL_FILL);

        processInputs(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shape.useProgram();

        rotation_x_mat4(anglex, mat4Transform);
		rotate_y_mat4(angley, mat4Transform);

        shape.transformMatrix4f(mat4Transform);
        shape.drawTrianglesIndexed();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
#ifdef __DYNAMIT_POLAR_ARROW_PARAMETRIC_CPP__
int main() { return main_dynamitPolarArrowParametric(); }
#endif