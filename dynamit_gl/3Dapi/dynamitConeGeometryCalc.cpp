#include "enabler.h"
#ifdef __DYNAMIT_CONE_GEOMETRY_CALC_CPP__

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

int main()
{
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;

    std::cout << glGetString(GL_VERSION) << std::endl;

    //// Build heart-shaped cone with calculator
    std::vector<float> verts, norms, verts1, norms1;
    std::vector<float> vertsIndexed, normsIndexed, vertsIndexed1, normsIndexed1;
    std::vector<uint32_t> indices, indices1;

    //buildCone (verts, norms, L"1", 6, 4);
    //buildCone (verts, norms, L"1", 0.f, M_PI, 6, 4);
    //buildCone (verts, norms, L"-(PI - theta) / PI", 0.f, M_PI, 6, 4);
    //buildCone (verts, norms, L"-theta / PI", 20, 3);

    std::cout << "Heart cone vertices: " << verts.size() / 3 << " (triangles: " << verts.size() / 9 << ")" << std::endl;

    bool buildHeart = false, buildCardoid = false, buildEllipse = false, build5PointerStar = false, build5PetalRose = false, buildLemniscate = true;
    if (buildHeart)
    {
        buildConePolar(verts, norms, L"theta / PI", 0.f, M_PI, 6, 4);               // hearth cone, First half:
        buildConePolar(verts1, norms1, L"(2*PI - theta) / PI", M_PI, 2 * M_PI, 10, 10); // hearth cone, Second half:
        buildConePolarIndexed(vertsIndexed, normsIndexed, indices, L"theta / PI", 0.f, M_PI, 10, 4);    // hearth cone, First half:
        buildConePolarIndexed(vertsIndexed1, normsIndexed1, indices1, L"(2*PI - theta) / PI", M_PI, 2 * M_PI, 10, 4);    // hearth cone, First half:
    }
    else if (buildCardoid)
    {
        buildConePolar(verts, norms, L"(1 - cos(theta)) / (PI / 1.55)", 60); //cardoid
        buildConePolarIndexed(vertsIndexed, normsIndexed, indices, L"(1 - cos(theta)) / (PI / 1.55)", 10); //cardoid
    }
    else if (buildEllipse)
    {
        buildConePolar(verts, norms, L"2 / sqrt(4 * sin(theta)**2 + cos(theta)**2) / 2", 10);// ellipse
        buildConePolarIndexed(vertsIndexed, normsIndexed, indices, L"2 / sqrt(4 * sin(theta)**2 + cos(theta)**2) / 2", 60, 50);// ellipse
    }
    else if (build5PointerStar)
    {
        buildConePolar(verts, norms, L"(1 + 0.5 * cos(5 * theta)) / 1.5", 60, 20); //star // 5-pointed star
        buildConePolarIndexed(vertsIndexed, normsIndexed, indices, L"(1 + 0.5 * cos(5 * theta)) / 1.5", 60, 20); //star // 5-pointed star
    }
    else if (build5PetalRose)
    {
        buildConePolar(verts, norms, L"cos(5 * theta)", 160, 3);  //star // 5-pointed star
        buildConePolarIndexed(vertsIndexed, normsIndexed, indices, L"cos(5 * theta)", 60, 20); //star // 5-pointed star
        //buildConePolar(verts, norms, L"cos(3 * theta)", 60); //3-petal rose
        //buildConePolar(verts, norms, L"cos(2 * theta)", 60); //4-petal rose
        //buildConePolar(verts, norms, L"cos(5 * theta)", 160); //5-petal rose
    }
    else if (buildLemniscate)
    {
        buildConePolar(verts, norms, L"sqrt(abs(cos(2 * theta)))", 60); //Lemniscate
        buildConePolarIndexed(vertsIndexed, normsIndexed, indices, L"sqrt(abs(cos(2 * theta)))", 60); //Lemniscate
    }
    //buildConePolar(verts, norms, L"1", 0.f, M_PI, 10, 3); //circle
    //buildConePolar(verts1, norms1, L"1", M_PI, 2 * M_PI, 10, 3); //circle
    //buildConePolarIndexed(vertsIndexed, normsIndexed, indices, L"1", 0.f, M_PI, 10, 3); //circle
    //buildConePolarIndexed(vertsIndexed1, normsIndexed1, indices1, L"1", M_PI, 2 *M_PI, 10, 3); //circle


    // Create shape
    Dynamit shape, shapeIndexed;
    shape.withConstColor(0.0f, 1.0f, 0.5f, 1.0f)
        .withConstLightDirection(-1.0f, -1.0f, 1.0f);
    shapeIndexed.withConstColor(0.0f, 1.0f, 0.5f, 1.0f)
        .withConstLightDirection(-1.0f, -1.0f, 1.0f);
    if (verts.size() > 0)
    {
        shape
            .withVertices3d(verts)
            .withNormals3d(norms);
        if (verts1.size() > 0)
            shape[1]
            .withVertices3d(verts1)
            .withNormals3d(norms1);
    }

    if (vertsIndexed.size() > 0)
    {
        shapeIndexed
            .withVertices3d(vertsIndexed)
            .withNormals3d(normsIndexed)
            .withIndices(indices);
        if (vertsIndexed1.size() > 0)
            shapeIndexed[1]
            .withVertices3d(vertsIndexed1)
            .withNormals3d(normsIndexed1)
            .withIndices(indices1);
    }

    shape.logGeneratedShaders("cone1HeartGeometry2Calc.js:");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 1.f, 0.9f);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        glPolygonMode(GL_FRONT_AND_BACK, glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS ? GL_LINE : GL_FILL);

        processInputs(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        switch (currentShape)
        {
        case DRAW_1:
            if (verts.size() > 0)
                shape.drawTriangles();
            break;
        case DRAW_2:
            if (vertsIndexed.size() > 0)
                shapeIndexed.drawTrianglesIndexed();
            break;
        }

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

#endif