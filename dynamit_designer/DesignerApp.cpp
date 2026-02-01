#include "DesignerApp.h"
#include "ProjectManager.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "dialogs/MainToolbar.h"
#include "dialogs/ExportToolbar.h"
#include "dialogs/BuilderPanel.h"
#include "dialogs/TransformPanel.h"
#include "dialogs/ColorPanel.h"
#include "dialogs/ViewPanel.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

DesignerApp::DesignerApp(HINSTANCE hInstance, GLFWwindow* window, int width, int height, int panelWidth)
    : m_hInstance(hInstance)
    , m_window(window)
    , m_windowWidth(width)
    , m_windowHeight(height)
    , m_panelWidth(panelWidth)
    , m_viewportX(panelWidth)
    , m_viewportWidth(width - panelWidth)
    , m_rotationX(17.0f)
    , m_rotationY(132.0f)
    , m_zoom(6.0f)
    , m_wireframeMode(false)
    , m_panelsVisible(true)
    , m_showNormals(false)
    , m_mouseDragging(false)
    , m_lastMouseX(0.0)
    , m_lastMouseY(0.0)
    , m_selectedShapeIndex(-1)
    , m_mainToolbarHwnd(nullptr)
    , m_exportToolbarHwnd(nullptr)
    , m_builderPanelHwnd(nullptr)
    , m_transformPanelHwnd(nullptr)
    , m_colorPanelHwnd(nullptr)
    , m_viewPanelHwnd(nullptr)
    , m_projectManager(std::make_unique<ProjectManager>(this))
{
}

DesignerApp::~DesignerApp()
{
    shutdown();
}

void DesignerApp::initProject(const std::wstring& projectPath,
                              const std::wstring& projectDir,
                              const std::wstring& projectName,
                              bool isNew)
{
    m_projectManager->initProject(projectPath, projectDir, projectName, isNew);
}

void DesignerApp::updateWindowTitle()
{
    std::wstring wtitle = L"Dynamit Designer - ";
    wtitle += m_projectManager->getProjectName();

    // Convert wide string to UTF-8
    int size = WideCharToMultiByte(CP_UTF8, 0, wtitle.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string title(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wtitle.c_str(), -1, &title[0], size, nullptr, nullptr);

    glfwSetWindowTitle(m_window, title.c_str());
}

bool DesignerApp::initialize()
{
    // No manual shader init needed - Dynamit auto-generates shaders!
    std::cout << "Using Dynamit for rendering - shaders auto-generated per shape" << std::endl;

    // Initialize visualization helpers (axes, light direction, grid)
    m_vizHelpers.initialize();

    createDialogs();

    // Handle project loading/creation
    if (m_projectManager->hasProject())
    {
        if (m_projectManager->isNewProject())
        {
            std::cout << "Created new project" << std::endl;

            // Create initial shape for new projects
            newShape(ShapeConfig::Type::Cone);

            // Save initial state
            if (!m_projectManager->saveProject())
            {
                std::wcerr << L"Failed to save project: " << m_projectManager->getLastError() << std::endl;
            }
        }
        else
        {
            // Existing project - load it
            if (!m_projectManager->loadFromFile(m_projectManager->getCurrentProjectPath()))
            {
                std::cerr << "Failed to load project" << std::endl;
                return false;
            }
            std::cout << "Loaded existing project" << std::endl;
        }

        updateWindowTitle();
    }
    else
    {
        // No project (shouldn't happen with startup dialog)
        newShape(ShapeConfig::Type::Cone);
    }

    return true;
}

void DesignerApp::shutdown()
{
    m_shapeManager.clearAll();

    // Destroy dialogs
    if (m_mainToolbarHwnd) { DestroyWindow(m_mainToolbarHwnd); m_mainToolbarHwnd = nullptr; }
    if (m_exportToolbarHwnd) { DestroyWindow(m_exportToolbarHwnd); m_exportToolbarHwnd = nullptr; }
    if (m_builderPanelHwnd) { DestroyWindow(m_builderPanelHwnd); m_builderPanelHwnd = nullptr; }
    if (m_transformPanelHwnd) { DestroyWindow(m_transformPanelHwnd); m_transformPanelHwnd = nullptr; }
    if (m_colorPanelHwnd) { DestroyWindow(m_colorPanelHwnd); m_colorPanelHwnd = nullptr; }
    if (m_viewPanelHwnd) { DestroyWindow(m_viewPanelHwnd); m_viewPanelHwnd = nullptr; }

    m_mainToolbar.reset();
    m_exportToolbar.reset();
    m_builderPanel.reset();
    m_transformPanel.reset();
    m_colorPanel.reset();
    m_viewPanel.reset();
}

void DesignerApp::createDialogs()
{
    // Get GLFW window handle
    HWND glfwHwnd = glfwGetWin32Window(m_window);

    // Create dialog panels using ATL
    m_mainToolbar = std::make_unique<MainToolbar>(this);
    m_exportToolbar = std::make_unique<ExportToolbar>(this);
    m_builderPanel = std::make_unique<BuilderPanel>(this);
    m_transformPanel = std::make_unique<TransformPanel>(this);
    m_colorPanel = std::make_unique<ColorPanel>(this);
    m_viewPanel = std::make_unique<ViewPanel>(this);

    // Create windows
    m_mainToolbarHwnd = m_mainToolbar->Create(glfwHwnd);
    m_exportToolbarHwnd = m_exportToolbar->Create(glfwHwnd);
    m_builderPanelHwnd = m_builderPanel->Create(glfwHwnd);
    m_transformPanelHwnd = m_transformPanel->Create(glfwHwnd);
    m_colorPanelHwnd = m_colorPanel->Create(glfwHwnd);
    m_viewPanelHwnd = m_viewPanel->Create(glfwHwnd);

    updateDialogPositions();

    // Show panels
    ShowWindow(m_mainToolbarHwnd, SW_SHOW);
    ShowWindow(m_exportToolbarHwnd, SW_SHOW);
    ShowWindow(m_builderPanelHwnd, SW_SHOW);
    ShowWindow(m_transformPanelHwnd, SW_SHOW);
    ShowWindow(m_colorPanelHwnd, SW_SHOW);
    ShowWindow(m_viewPanelHwnd, SW_SHOW);
}

void DesignerApp::updateDialogPositions()
{
    // Get GLFW window position
    int winX, winY;
    glfwGetWindowPos(m_window, &winX, &winY);

    // Stack panels vertically on the left
    int y = 30; // Start below title bar area
    int panelGap = 5;
    int shapesToolbarHeight = 120;
    int exportToolbarHeight = 215;
    int builderHeight = 280;
    int transformHeight = 180;
    int colorHeight = 200;

    if (m_mainToolbarHwnd)
    {
        SetWindowPos(m_mainToolbarHwnd, HWND_TOPMOST, winX + 5, winY + y,
            m_panelWidth - 10, shapesToolbarHeight, SWP_NOZORDER);
        y += shapesToolbarHeight + panelGap;
    }

    if (m_exportToolbarHwnd)
    {
        SetWindowPos(m_exportToolbarHwnd, HWND_TOPMOST, winX + 5, winY + y,
            m_panelWidth - 10, exportToolbarHeight, SWP_NOZORDER);
        y += exportToolbarHeight + panelGap;
    }

    if (m_builderPanelHwnd)
    {
        SetWindowPos(m_builderPanelHwnd, HWND_TOPMOST, winX + 5, winY + y,
            m_panelWidth - 10, builderHeight, SWP_NOZORDER);
        y += builderHeight + panelGap;
    }

    if (m_transformPanelHwnd)
    {
        SetWindowPos(m_transformPanelHwnd, HWND_TOPMOST, winX + 5, winY + y,
            m_panelWidth - 10, transformHeight, SWP_NOZORDER);
        y += transformHeight + panelGap;
    }

    if (m_colorPanelHwnd)
    {
        SetWindowPos(m_colorPanelHwnd, HWND_TOPMOST, winX + 5, winY + y,
            m_panelWidth - 10, colorHeight, SWP_NOZORDER);
    }

    // View panel on the right side
    int viewPanelWidth = 160;
    int viewPanelHeight = 240;
    if (m_viewPanelHwnd)
    {
        SetWindowPos(m_viewPanelHwnd, HWND_TOPMOST,
            winX + m_windowWidth - viewPanelWidth - 5, winY + 30,
            viewPanelWidth, viewPanelHeight, SWP_NOZORDER);
    }
}

void DesignerApp::update(float deltaTime)
{
    // Rebuild any dirty shapes
    m_shapeManager.rebuildAllDirty();

    // Update dialog positions if window moved
    static int lastWinX = 0, lastWinY = 0;
    int winX, winY;
    glfwGetWindowPos(m_window, &winX, &winY);
    if (winX != lastWinX || winY != lastWinY)
    {
        updateDialogPositions();
        lastWinX = winX;
        lastWinY = winY;
    }
}

std::array<float, 16> DesignerApp::computeViewProjection()
{
    // Left-handed coordinate system: +X right, +Y up, +Z into screen
    glm::vec3 cameraPos(
        m_zoom * cos(glm::radians(m_rotationX)) * sin(glm::radians(m_rotationY)),
        m_zoom * sin(glm::radians(m_rotationX)),
        m_zoom * cos(glm::radians(m_rotationX)) * cos(glm::radians(m_rotationY))
    );

    glm::mat4 view = glm::lookAtLH(cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // Calculate projection matrix (left-handed)
    float aspectRatio = m_panelsVisible
        ? static_cast<float>(m_viewportWidth) / static_cast<float>(m_windowHeight)
        : static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight);

    glm::mat4 projection = glm::perspectiveLH(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

    // Combine view and projection
    glm::mat4 vp = projection * view;

    std::array<float, 16> result;
    memcpy(result.data(), glm::value_ptr(vp), 16 * sizeof(float));
    return result;
}

void DesignerApp::render()
{
    // Set viewport for 3D rendering (right side of window)
    if (m_panelsVisible)
    {
        glViewport(m_viewportX, 0, m_viewportWidth, m_windowHeight);
    }
    else
    {
        glViewport(0, 0, m_windowWidth, m_windowHeight);
    }

    // Set polygon mode
    if (m_wireframeMode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Compute view-projection matrix
    std::array<float, 16> viewProjection = computeViewProjection();

    // Render visualization helpers (axes, light, grid)
    m_vizHelpers.render(viewProjection);

    // Render all shapes - Dynamit handles shaders automatically!
    m_shapeManager.render(viewProjection, m_showNormals);

    // Reset polygon mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void DesignerApp::onResize(int width, int height)
{
    m_windowWidth = width;
    m_windowHeight = height;
    m_viewportWidth = width - m_panelWidth;

    updateDialogPositions();
}

void DesignerApp::onKey(int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;

    const float rotSpeed = 5.0f;

    switch (key)
    {
    case GLFW_KEY_LEFT:
        m_rotationY -= rotSpeed;
        break;
    case GLFW_KEY_RIGHT:
        m_rotationY += rotSpeed;
        break;
    case GLFW_KEY_UP:
        m_rotationX += rotSpeed;
        if (m_rotationX > 89.0f) m_rotationX = 89.0f;
        break;
    case GLFW_KEY_DOWN:
        m_rotationX -= rotSpeed;
        if (m_rotationX < -89.0f) m_rotationX = -89.0f;
        break;
    case GLFW_KEY_F11:
        if (action == GLFW_PRESS)
        {
            m_wireframeMode = !m_wireframeMode;
            std::cout << "Wireframe mode: " << (m_wireframeMode ? "ON" : "OFF") << std::endl;
        }
        break;
    case GLFW_KEY_F9:
        if (action == GLFW_PRESS)
        {
            m_panelsVisible = !m_panelsVisible;
            int showCmd = m_panelsVisible ? SW_SHOW : SW_HIDE;
            if (m_mainToolbarHwnd) ShowWindow(m_mainToolbarHwnd, showCmd);
            if (m_exportToolbarHwnd) ShowWindow(m_exportToolbarHwnd, showCmd);
            if (m_builderPanelHwnd) ShowWindow(m_builderPanelHwnd, showCmd);
            if (m_transformPanelHwnd) ShowWindow(m_transformPanelHwnd, showCmd);
            if (m_colorPanelHwnd) ShowWindow(m_colorPanelHwnd, showCmd);
            if (m_viewPanelHwnd) ShowWindow(m_viewPanelHwnd, showCmd);
            std::cout << "Panels: " << (m_panelsVisible ? "visible" : "hidden") << std::endl;
        }
        break;
    }
}

void DesignerApp::onMouseButton(int button, int action, int mods, double x, double y)
{
    // Only handle clicks in viewport area
    if (m_panelsVisible && x < m_panelWidth)
        return;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            m_mouseDragging = true;
            m_lastMouseX = x;
            m_lastMouseY = y;
        }
        else if (action == GLFW_RELEASE)
        {
            m_mouseDragging = false;
        }
    }
}

void DesignerApp::onMouseMove(double x, double y)
{
    if (m_mouseDragging)
    {
        double dx = x - m_lastMouseX;
        double dy = y - m_lastMouseY;

        m_rotationY += static_cast<float>(dx) * 0.5f;
        m_rotationX += static_cast<float>(dy) * 0.5f;

        if (m_rotationX > 89.0f) m_rotationX = 89.0f;
        if (m_rotationX < -89.0f) m_rotationX = -89.0f;

        m_lastMouseX = x;
        m_lastMouseY = y;
    }
}

void DesignerApp::onScroll(double xoffset, double yoffset)
{
    m_zoom -= static_cast<float>(yoffset) * 0.5f;
    if (m_zoom < 1.0f) m_zoom = 1.0f;
    if (m_zoom > 20.0f) m_zoom = 20.0f;
}

void DesignerApp::newShape(ShapeConfig::Type type)
{
    ShapeConfig config;
    config.type = type;
    config.name = (type == ShapeConfig::Type::Cone) ? "Cone" : "Cylinder";
    config.name += " " + std::to_string(m_shapeManager.getShapeCount() + 1);

    int index = m_shapeManager.addShape(config);
    selectShape(index);

    std::cout << "Created new " << config.name << std::endl;
}

void DesignerApp::deleteSelectedShape()
{
    if (m_selectedShapeIndex >= 0)
    {
        std::cout << "Deleted shape " << m_selectedShapeIndex << std::endl;
        m_shapeManager.removeShape(m_selectedShapeIndex);

        if (m_shapeManager.getShapeCount() > 0)
        {
            selectShape(std::min(m_selectedShapeIndex, m_shapeManager.getShapeCount() - 1));
        }
        else
        {
            m_selectedShapeIndex = -1;
        }
    }
}

void DesignerApp::selectShape(int index)
{
    if (index >= 0 && index < m_shapeManager.getShapeCount())
    {
        m_selectedShapeIndex = index;

        // Update panels with new selection
        if (m_builderPanel) m_builderPanel->updateFromConfig();
        if (m_transformPanel) m_transformPanel->updateFromConfig();
        if (m_colorPanel) m_colorPanel->updateFromConfig();
    }
}

ShapeConfig* DesignerApp::getSelectedShapeConfig()
{
    ShapeInstance* shape = m_shapeManager.getShape(m_selectedShapeIndex);
    return shape ? &shape->config : nullptr;
}

void DesignerApp::onShapeConfigChanged()
{
    ShapeInstance* shape = m_shapeManager.getShape(m_selectedShapeIndex);
    if (shape)
    {
        shape->dirty = true;
    }
}
