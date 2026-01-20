#include "enabler.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Dynamit.h>
#include <config.h>
#include <callbacks.h>

#include <expression_compiler.h>

using namespace dynamit;
using namespace expresie_tokenizer;

// Build cone geometry with analytical normals from gradient, mc stands for manyal with calculator
void buildmcConemcPolar(std::vector<float>& verts, std::vector<float>& norms, const std::wstring& formula, int ns = 5)
{
    expression_token_compiler compiler;

    verts.clear();
    norms.clear();

    long double theta = 0.0L;

    // Compile r(theta) expression
    std::unique_ptr<expression> expr_r = compiler.compile(formula);
    expr_r->bind(L"theta", &theta);
    
    // Compute dr/dtheta symbolically
    std::unique_ptr<expression> expr_dr = simplify(expr_r->derivative(L"theta"));
    expr_dr->bind(L"theta", &theta);

    const float z_tip = -0.7f;

    for (int i = 0; i < ns; i++)
    {
        // Tip of cone
        verts.insert(verts.end(), { 0.0f, 0.0f, z_tip });
        norms.insert(norms.end(), { 0.0f, 0.0f, 0.0f });

        // Base point 1
        theta = 2.0L * M_PI * i / ns;
        float r1  = static_cast<float>(expr_r->eval());
        float dr1 = static_cast<float>(expr_dr->eval());
        float x1 = static_cast<float>(expr_r->cyl_x(theta));
        float y1 = static_cast<float>(expr_r->cyl_y(theta));
        
        // Normal from gradient: perpendicular to tangent
        // Tangent T = (dr*cos - r*sin, dr*sin + r*cos)
        // Normal N = (dr*sin + r*cos, -(dr*cos - r*sin), z_component)
        float cos_t = std::cos(static_cast<float>(theta));
        float sin_t = std::sin(static_cast<float>(theta));
        float nx1 =   dr1 * sin_t + r1 * cos_t;
        float ny1 = -(dr1 * cos_t - r1 * sin_t);
        float nz1 = z_tip;  // z component for cone slope
        
        verts.insert(verts.end(), { x1, y1, 0.0f });
        norms.insert(norms.end(), { nx1, ny1, nz1 });

        // Base point 2
        theta = 2.0L * M_PI * (i + 1) / ns;
        float r2 = static_cast<float>(expr_r->eval());
        float dr2 = static_cast<float>(expr_dr->eval());
        float x2 = static_cast<float>(expr_r->cyl_x(theta));
        float y2 = static_cast<float>(expr_r->cyl_y(theta));
        
        cos_t = std::cos(static_cast<float>(theta));
        sin_t = std::sin(static_cast<float>(theta));
        float nx2 =   dr2 * sin_t + r2 * cos_t;
        float ny2 = -(dr2 * cos_t - r2 * sin_t);
        float nz2 = z_tip;
        
        verts.insert(verts.end(), { x2, y2, 0.0f });
        norms.insert(norms.end(), { nx2, ny2, nz2 });
    }
}

int main_cone1Animate1Calc()
{
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;

    std::cout << glGetString(GL_VERSION) << std::endl;

    // Build cone geometry
    // Circle: r = 0.6 (constant, dr/dtheta = 0)
    // Try also: L"0.3 + 0.1 * theta" for spiral
    // Or: L"0.4 + 0.1 * cos(5 * theta)" for flower
    std::vector<float> verts, norms;
    //buildCone(verts, norms, L"1", 5);
    buildmcConemcPolar (verts, norms, L"1", 6);
    //buildCone(verts, norms, L"theta / PI", 20);

    std::cout << "Cone vertices: " << verts.size() / 3 << std::endl;

    Dynamit shape;
    shape.withConstColor(0.0f, 1.0f, 0.0f, 1.0f)
        .withVertices3d(verts)
        .withNormals3d(norms)
        .withLightDirection3f(1.0f, 0.0f, 1.0f);

    shape.logGeneratedShaders("cone1Animate1Calc.js:");


    // Animated light
    long double t = 0.0L;


    expression_token_compiler compiler;
    // These now work:
    auto expr1 = compiler.compile(L"2 * PI * r");
    auto expr2 = compiler.compile(L"sin(M_PI_2)");  // evaluates to 1
    auto expr3 = compiler.compile(L"E ** x");
    auto expr4 = compiler.compile(L"TAU * freq");
    long double r = 1.0L;
    expr1->bind(L"r", &r);
    std::cout << "circle radius " << r << " perimeter: " << expr1->eval() << std::endl;
    std::cout << "sin (pi/2) = " << expr2->eval() << std::endl;

    std::unique_ptr<expression> expr_lx = compiler.compile(L"cos(t * 0.002)");
    std::unique_ptr<expression> expr_ly = compiler.compile(L"sin(t * 0.002)");
    std::unique_ptr<expression> expr_lz = compiler.compile(L"1");
    expr_lx->bind(L"t", &t);
    expr_ly->bind(L"t", &t);

    std::cout << "Light animation: lx=cos(t*0.002), ly=sin(t*0.002), lz=1" << std::endl;

    double startTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {

        glPolygonMode(GL_FRONT_AND_BACK, glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS? GL_LINE : GL_FILL);

        processInputs(window);

        t = static_cast<long double>((glfwGetTime() - startTime) * 1000.0);

        glClearColor(0.5f, 0.5f, 0.5f, 0.9f);
        glClear(GL_COLOR_BUFFER_BIT);

        float lx = static_cast<float>(expr_lx->eval());
        float ly = static_cast<float>(expr_ly->eval());
        float lz = static_cast<float>(expr_lz->eval());

        shape.lightDirection3f(lx, ly, lz);
        shape.drawTriangles();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
#ifdef __CONE1_ANIMATE1_CALC_CPP__
int main() { return main_cone1Animate1Calc(); }
#endif