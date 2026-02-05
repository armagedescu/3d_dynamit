#include "enabler.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <Dynamit.h>
#include <builders.h>
#include <config.h>

//#include <geometry.h>
using namespace dynamit;
using namespace dynamit::builders;


int main_cartesianFirstDraw()
{
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;

    std::cout << glGetString(GL_VERSION) << std::endl;

	std::vector<float> verts, norms, colors;
    std::vector<uint32_t> indices;

    // Build a surface z = sin(x) * cos(y)
    Builder::cartesian()
        //.formula("sin(x) * cos(y)")
        .formula("x*x")
        .domain(-3.14, 3.14) //, -3.14, 3.14)
        //.divisions(10, 10)
        .sectors_slices(100, 40)
        .smooth(true)
        .color(std::array<float, 3>{ 0.2f, 0.6f, 1.0f })
        //.buildConeIndexedWithColor(verts, norms, colors, indices);
        .buildCylinderIndexedWithColor(verts, norms, colors, indices);

    //// Build a simple plane
    //Builder::cartesian()
    //    .domain(-1.0f, 1.0f, -1.0f, 1.0f)
    //    .color(std::array<float, 3>{ 0.5f, 0.5f, 0.5f })
    //    .buildPlaneIndexedWithColor(verts, norms, colors, indices, scaleMatrix(2.0f, 2.0f, 1.0f));
    //
    //// Build a box
    //Builder::cartesian()
    //    .domain(-0.5f, 0.5f, -0.5f, 0.5f)
    //    .color(std::array<float, 3>{ 1.0f, 0.0f, 0.0f })
    //    .buildBoxIndexedWithColor(verts, norms, colors, indices);

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
    //glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 1.f, 0.9f);

    mat4<float> mat4Transform = {};

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
#ifdef __MAIN_CARTESIAN_FIRST_DRAW_CPP__
int main() { return main_cartesianFirstDraw(); }
#endif