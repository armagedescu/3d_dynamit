#include "enabler.h"
#ifdef __MAIN_SHADOW__
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <config.h>
#include <Shape.h>
#include <callbacks.h>
#include <TextureLoader.h>
#include <TextureShower.h>
#include <FrameBufferDepthMap.h>
#include <CubeScene.h>

using namespace std;

int main()
{
    GLFWwindow* window = openglWindowInit();
    if (!window)
        return -1;

    FrameBufferDepthMap<CubeScene> depthMapper  ("shaders/shadows/3.1.2/"  "shadow_mapping.vs", "shaders/shadows/3.1.2/"  "shadow_mapping.fs");
    FrameBufferDepthMap<CubeScene> depthMappera ("shaders/shadows/3.1.2a/" "shadow_mapping.vs", "shaders/shadows/3.1.2a/" "shadow_mapping.fs");
    CubeScene     scene       ("shaders/shadows/3.1.2/"  "shadow_scene.vs", "shaders/shadows/3.1.2/"  "shadow_scene.fs");
    CubeScene     scenea      ("shaders/shadows/3.1.2a/" "shadow_scene.vs", "shaders/shadows/3.1.2a/" "shadow_scene.fs");
    TextureShower depthShower ("shaders/shadows/3.1.2/debug_quad.vs",   "shaders/shadows/3.1.2/debug_quad.fs");

    scene.postBuild();
    scenea.postBuild();

    glEnable(GL_DEPTH_TEST);

    glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);
    while (!glfwWindowShouldClose(window))
    {
        using config::camera;
        processInputs(window);

        float near_plane = 1.0f, far_plane = 7.5f;
        glm::mat4 lightProjection  = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 lightView        = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
 
        // 1. render depth of scene to texture (from light's perspective)
        depthMapper.drawInit(lightSpaceMatrix);
        depthMapper.draw(
            []
            {
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                glClear(GL_DEPTH_BUFFER_BIT);
            });
        depthMappera.drawInit(lightSpaceMatrix);
        depthMappera.draw(
            []
            {
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                glClear(GL_DEPTH_BUFFER_BIT);
            });


        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)config::windowWidth / (float)config::windowHeight, 0.1f, 100.0f);
        glm::mat4 view       = camera.view();

        // reset viewport
        glViewport(0, 0, config::windowWidth, config::windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene.drawInit(depthMapper.getTexture(), projection, view, camera.position, lightPos, lightSpaceMatrix);
        scenea.drawInit(depthMapper.getTexture(), projection, view, camera.position, lightPos, lightSpaceMatrix);
        depthShower.drawInit(depthMapper.getTexture(), near_plane, far_plane);

        switch (currentDraw)
        {
        case DRAW_1:
            scene.draw();
            break;
        case DRAW_2:
            scenea.draw();
            break;
        case DRAW_3:
            depthShower.draw();
            break;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

#endif