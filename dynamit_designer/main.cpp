// Dynamit Designer - Visual Shape Designer
// Entry point, window initialization, and render loop

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <iostream>
#include <string>

#pragma comment(lib, "comctl32.lib")

#include "DesignerApp.h"

// Window dimensions
constexpr int WINDOW_WIDTH = 1600;
constexpr int WINDOW_HEIGHT = 900;
constexpr int PANEL_WIDTH = 300;

// Global application instance
static DesignerApp* g_app = nullptr;

// GLFW callbacks
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    if (g_app)
    {
        g_app->onResize(width, height);
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (g_app)
    {
        g_app->onKey(key, scancode, action, mods);
    }

    // Close on Escape
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (g_app)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        g_app->onMouseButton(button, action, mods, xpos, ypos);
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (g_app)
    {
        g_app->onMouseMove(xpos, ypos);
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (g_app)
    {
        g_app->onScroll(xoffset, yoffset);
    }
}

// Initialize GLFW and create window
GLFWwindow* initGLFW()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
        "Dynamit Designer - Shape Editor", nullptr, nullptr);

    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

    return window;
}

// Initialize GLEW
bool initGLEW()
{
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cerr << "GLEW initialization failed: " << glewGetErrorString(err) << std::endl;
        return false;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    return true;
}

// Process Windows messages for ATL dialogs
void processWindowsMessages()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Initialize common controls (for spin controls etc.)
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES | ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&icex);

    // Allocate console for debug output
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);

    std::cout << "=== Dynamit Designer ===" << std::endl;
    std::cout << "Visual Shape Designer for PolarBuilder" << std::endl;
    std::cout << std::endl;

    // Initialize GLFW
    GLFWwindow* window = initGLFW();
    if (!window)
    {
        return -1;
    }

    // Initialize GLEW
    if (!initGLEW())
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Create application
    DesignerApp app(hInstance, window, WINDOW_WIDTH, WINDOW_HEIGHT, PANEL_WIDTH);
    g_app = &app;

    if (!app.initialize())
    {
        std::cerr << "Failed to initialize application" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Arrow Keys: Rotate view" << std::endl;
    std::cout << "  Mouse Drag: Rotate view (in viewport)" << std::endl;
    std::cout << "  Scroll: Zoom in/out" << std::endl;
    std::cout << "  F11: Toggle wireframe" << std::endl;
    std::cout << "  F9: Toggle panel visibility" << std::endl;
    std::cout << "  Escape: Exit" << std::endl;
    std::cout << std::endl;

    // Enable depth testing and blending
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Main render loop
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        // Calculate delta time
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // Process Windows messages (for ATL dialogs)
        processWindowsMessages();

        // Poll GLFW events
        glfwPollEvents();

        // Update application
        app.update(deltaTime);

        // Clear screen
        glClearColor(0.15f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render
        app.render();

        // Swap buffers
        glfwSwapBuffers(window);
    }

    // Cleanup
    app.shutdown();
    g_app = nullptr;

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

// Alternative console entry point for debugging
int main()
{
    return WinMain(GetModuleHandle(nullptr), nullptr, GetCommandLineA(), SW_SHOWDEFAULT);
}
