#include "enabler.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Dynamit.h>
#include <config.h>
#include <callbacks.h>

using namespace dynamit;

const char* vertexShaderShape2 = R"glsl(#version 330 core
layout (location = 0) in vec3 vertex;
const vec4 constTranslation = vec4(0.5, 0.5, 0, 0);
//hello override vs

void main() //vs
{
    gl_Position = vec4(vertex, 1) + constTranslation;
}
)glsl";

const char* fragmentShaderShape2 = R"glsl(#version 330 core
precision mediump float;
out vec4 fragColor;
const vec4 constColor = vec4(0, 1, 0, 1);
//hello override fs

void main() //fs
{
    fragColor = constColor;
}
)glsl";

int main_draw0()
{
    srand(time(NULL));
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;
        
    std::cout << glGetString(GL_VERSION) << std::endl;
    
    // Shape 1 - 2D vertices (6 triangles worth of data)
    std::vector<float> verts1 = {
        0.0f, -0.5f,   -0.8f, -0.3f,   -0.5f, -0.8f,
        0.0f, -0.5f,    0.8f, -0.1f,   -0.4f,  0.3f
    };
    
    Dynamit shape1;
    shape1.withVertices2d(verts1);// .withConstColor(0.0f, 0.0f, 1.0f, 1.0f);
    //shape1.logGeneratedShaders("draw0 shape1 translate");

    // Shape 2 - 3D vertices with translation and color
    std::vector<float> verts2 = {
        0.0f, 0.0f, 0.0f,  -1.0f, 0.4f, 2.0f,   -0.5f, -0.6f,  2.0f,
        0.0f, 0.0f, 0.0f,   0.4f, 0.4f, 2.0f,   -0.4f,  0.5f, -0.0f
    };
    
    Dynamit shape2;
    shape2.withVertices3d(verts2)
          .withConstTranslation(0.5f, 0.5f, 0.0f, 0.0f)
          .withConstColor(0.0f, 1.0f, 0.0f, 1.0f);
    shape2.withShaderSources(vertexShaderShape2, fragmentShaderShape2);
    // Optional: log generated shaders
    // shape1.logGeneratedShaders("draw0 shape1 static draw");
    //shape2.logGeneratedShaders("draw0 shape2 translate");
    //shape2.logShaders("draw0 shape2 translate");
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.5f, 0.5f, 0.9f);
    
    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        processInputs(window);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        shape1.drawTriangles();
        shape2.drawTriangles();
        
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    return 0;
}
#include "enabler.h"
#ifdef __DRAW0_CPP__
int main() { return main_draw0(); }
#endif