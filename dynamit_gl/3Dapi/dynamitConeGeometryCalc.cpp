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

int main_dynamitConeGeometryCalc()
{
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;

    std::cout << glGetString(GL_VERSION) << std::endl;

    //// Build heart-shaped cone with calculator
    std::vector<float> verts, norms, verts1, norms1;
    std::vector<float> vertsIndexed, normsIndexed, vertsIndexed1, normsIndexed1;
    std::vector<uint32_t> indices, indices1;

    //build Cone (verts, norms, L"1", 6, 4);
    //build Cone (verts, norms, L"1", 0.f, M_PI, 6, 4);
    //build Cone (verts, norms, L"-(PI - theta) / PI", 0.f, M_PI, 6, 4);
    //build Cone (verts, norms, L"-theta / PI", 20, 3);

    std::cout << "Heart cone vertices: " << verts.size() / 3 << " (triangles: " << verts.size() / 9 << ")" << std::endl;

    bool buildHeart = false, buildCardoid = false, buildEllipse = false, 
        build5PointerStar = false, build5PetalRose = false, 
        buildLemniscate = true;
    if (buildHeart)
    {
        Builder::polar() //.turbo(false)
            .formula(L"theta / PI")             //start build first half
            .domain(M_PI)
            .sectors_slices(60, 40)
			.buildCone(verts, norms)            //end build first half
			.formula(L"(2*PI - theta) / PI")    //start build second half
            .domain_shift(2 * M_PI)  // [π, 2π]
            .sectors_slices(8, 10)
			.buildCone(verts, norms)            //end build second half
			.formula(L"theta / PI")             //start build first half indexed
            .domain(static_cast<float>(M_PI))
            .slices_sectors(4, 10)
			.buildConeIndexed(vertsIndexed, normsIndexed, indices) //build first half indexed
            .formula(L"(2*PI - theta) / PI")
            .domain_shift(static_cast<float>(2 * M_PI))
            .slices_sectors(4, 6)
			.buildConeIndexed(vertsIndexed1, normsIndexed1, indices1); //build second half indexed
    }
    else if (buildCardoid)
    {
        Builder::polar() //cardoid
            .formula(L"(1 - cos(theta)) / (PI / 1.55)")
            .sectors(60)
            .buildCone(verts, norms) //build
            .sectors(10)
			.buildConeIndexed(vertsIndexed, normsIndexed, indices); //build indexed
    }
    else if (buildEllipse)
    {
        Builder::polar() //ellipse
            .formula(L"2 / sqrt(4 * sin(theta)**2 + cos(theta)**2) / 2")
            .sectors(10)
            .buildCone(verts, norms) //build
            .sectors_slices(60, 50)
			.buildConeIndexed(vertsIndexed, normsIndexed, indices); //build indexed
    }
    else if (build5PointerStar)
    {
        Builder::polar()  //star // 5-pointed star
			.formula(L"(1 + 0.5 * cos(5 * theta)) / 1.5") //start build 5-petal star
            .sectors_slices(60, 20)
			.buildCone(verts, norms) //end build 5-petal star
            .slices(5)
			.buildConeIndexed(vertsIndexed, normsIndexed, indices); // end build 5 - petal star indexed
    }
    else if (build5PetalRose)
    {
        Builder::polar()  //star // 5-pointed star
			.formula(L"cos(5 * theta)") //start build 5-petal rose
            .sectors_slices(160, 3)
            .buildCone(verts, norms) //end build 5-petal rose
            .sectors_slices(60, 20)
            .buildConeIndexed(vertsIndexed, normsIndexed, indices); // end build 5 - petal rose indexed

        //build Cone Polar(verts, norms, L"cos(3 * theta)", 60); //3-petal rose
        //build Cone Polar(verts, norms, L"cos(2 * theta)", 60); //4-petal rose
        //build Cone Polar(verts, norms, L"cos(5 * theta)", 160); //5-petal rose
    }
    else if (buildLemniscate)
    {
        Builder::polar()  //star // 5-pointed star
			.formula(L"sqrt(abs(cos(2 * theta)))") //start build lemniscate
            .sectors(30)
			.buildCone(verts, norms) // end build lemniscate
            .sectors(60)
			.buildConeIndexed(vertsIndexed, normsIndexed, indices); // end build lemniscate indexed

        //build Cone Polar(verts, norms, L"sqrt(abs(cos(2 * theta)))", 30); //Lemniscate
        //build Cone Polar Indexed(vertsIndexed, normsIndexed, indices, L"sqrt(abs(cos(2 * theta)))", 60); //Lemniscate
    }
    //build Cone Polar(verts, norms, L"1", 0.f, M_PI, 10, 3); //circle
    //build Cone Polar(verts1, norms1, L"1", M_PI, 2 * M_PI, 10, 3); //circle
    //build Cone Polar Indexed(vertsIndexed, normsIndexed, indices, L"1", 0.f, M_PI, 10, 3); //circle
    //build Cone Polar Indexed(vertsIndexed1, normsIndexed1, indices1, L"1", M_PI, 2 *M_PI, 10, 3); //circle

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

    shape.logGeneratedShaders("cone1HeartGeometry2Calc.cpp:");

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
#ifdef __DYNAMIT_CONE_GEOMETRY_CALC_CPP__
int main() { return main_dynamitConeGeometryCalc(); }
#endif