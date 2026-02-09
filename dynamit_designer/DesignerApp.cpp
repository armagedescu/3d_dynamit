#include "DesignerApp.h"
#include "ProjectManager.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "dialogs/ExportToolbar.h"
#include "dialogs/PropertiesPanel.h"
#include "dialogs/ViewPanel.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cfloat>

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
    , m_exportToolbarHwnd(nullptr)
    , m_propertiesPanelHwnd(nullptr)
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

    // Initialize transform gizmo
    m_gizmo.initialize();

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

    // Initial refresh of shapes list
    if (m_propertiesPanel) m_propertiesPanel->refreshShapesList();

    return true;
}

void DesignerApp::shutdown()
{
    m_shapeManager.clearAll();

    // Destroy dialogs
    if (m_exportToolbarHwnd) { DestroyWindow(m_exportToolbarHwnd); m_exportToolbarHwnd = nullptr; }
    if (m_propertiesPanelHwnd) { DestroyWindow(m_propertiesPanelHwnd); m_propertiesPanelHwnd = nullptr; }
    if (m_viewPanelHwnd) { DestroyWindow(m_viewPanelHwnd); m_viewPanelHwnd = nullptr; }

    m_exportToolbar.reset();
    m_propertiesPanel.reset();
    m_viewPanel.reset();
}

void DesignerApp::createDialogs()
{
    // Get GLFW window handle
    HWND glfwHwnd = glfwGetWin32Window(m_window);

    // Create dialog panels using ATL
    m_exportToolbar = std::make_unique<ExportToolbar>(this);
    m_propertiesPanel = std::make_unique<PropertiesPanel>(this);
    m_viewPanel = std::make_unique<ViewPanel>(this);

    // Create windows
    m_exportToolbarHwnd = m_exportToolbar->Create(glfwHwnd);
    m_propertiesPanelHwnd = m_propertiesPanel->Create(glfwHwnd);
    m_viewPanelHwnd = m_viewPanel->Create(glfwHwnd);

    updateDialogPositions();

    // Show panels
    ShowWindow(m_exportToolbarHwnd, SW_SHOW);
    ShowWindow(m_propertiesPanelHwnd, SW_SHOW);
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
    int exportToolbarHeight = 215;

    if (m_exportToolbarHwnd)
    {
        SetWindowPos(m_exportToolbarHwnd, HWND_TOPMOST, winX + 5, winY + y,
            m_panelWidth - 10, exportToolbarHeight, SWP_NOZORDER);
        y += exportToolbarHeight + panelGap;
    }

    // Properties panel (with integrated shapes list) - let it use its own size
    if (m_propertiesPanelHwnd)
    {
        RECT rc;
        ::GetWindowRect(m_propertiesPanelHwnd, &rc);
        int propHeight = rc.bottom - rc.top;
        int propWidth = rc.right - rc.left;
        SetWindowPos(m_propertiesPanelHwnd, HWND_TOPMOST, winX + 5, winY + y,
            propWidth, propHeight, SWP_NOZORDER);
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

    // Render transform gizmo on selected shape
    if (m_selectedShapeIndex >= 0)
    {
        ShapeConfig* cfg = getSelectedShapeConfig();
        if (cfg)
        {
            glm::vec3 shapePos(cfg->posX, cfg->posY, cfg->posZ);
            m_gizmo.render(viewProjection, shapePos);
        }
    }

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
            if (m_exportToolbarHwnd) ShowWindow(m_exportToolbarHwnd, showCmd);
            if (m_propertiesPanelHwnd) ShowWindow(m_propertiesPanelHwnd, showCmd);
            if (m_viewPanelHwnd) ShowWindow(m_viewPanelHwnd, showCmd);
            std::cout << "Panels: " << (m_panelsVisible ? "visible" : "hidden") << std::endl;
        }
        break;
    case GLFW_KEY_W:
        if (action == GLFW_PRESS)
        {
            m_gizmo.setMode(TransformGizmo::Mode::Translate);
            std::cout << "Gizmo: Translate" << std::endl;
        }
        break;
    case GLFW_KEY_E:
        if (action == GLFW_PRESS)
        {
            m_gizmo.setMode(TransformGizmo::Mode::Rotate);
            std::cout << "Gizmo: Rotate" << std::endl;
        }
        break;
    case GLFW_KEY_R:
        if (action == GLFW_PRESS)
        {
            m_gizmo.setMode(TransformGizmo::Mode::Scale);
            std::cout << "Gizmo: Scale" << std::endl;
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
            // Test gizmo hit first (if a shape is selected)
            if (m_selectedShapeIndex >= 0)
            {
                ShapeConfig* cfg = getSelectedShapeConfig();
                if (cfg)
                {
                    glm::vec3 rayOrigin, rayDir;
                    unprojectRay(x, y, rayOrigin, rayDir);
                    glm::vec3 shapePos(cfg->posX, cfg->posY, cfg->posZ);

                    auto hitAxis = m_gizmo.hitTest(rayOrigin, rayDir, shapePos);
                    if (hitAxis != TransformGizmo::Axis::None)
                    {
                        m_gizmo.beginDrag(hitAxis, rayOrigin, rayDir, shapePos);
                        m_gizmoDragging = true;
                        m_lastMouseX = x;
                        m_lastMouseY = y;
                        return; // Don't start camera drag
                    }
                }
            }

            m_mouseDragging = true;
            m_lastMouseX = x;
            m_lastMouseY = y;
        }
        else if (action == GLFW_RELEASE)
        {
            if (m_gizmoDragging)
            {
                m_gizmo.endDrag();
                m_gizmoDragging = false;
            }
            else
            {
                // Check if this was a click (minimal movement) for shape selection
                double dx = x - m_lastMouseX;
                double dy = y - m_lastMouseY;
                if (dx * dx + dy * dy < 25.0)  // Less than 5 pixels movement
                {
                    // Try to pick a shape
                    int pickedShape = pickShape(x, y);
                    if (pickedShape >= 0)
                    {
                        selectShape(pickedShape);
                        if (m_propertiesPanel) m_propertiesPanel->refreshShapesList();
                    }
                }
                m_mouseDragging = false;
            }
        }
    }
}

void DesignerApp::onMouseMove(double x, double y)
{
    if (m_gizmoDragging)
    {
        glm::vec3 rayOrigin, rayDir;
        unprojectRay(x, y, rayOrigin, rayDir);

        glm::vec3 delta = m_gizmo.updateDrag(rayOrigin, rayDir);

        ShapeConfig* cfg = getSelectedShapeConfig();
        if (cfg && m_gizmo.isDragging())
        {
            auto mode = m_gizmo.getMode();
            if (mode == TransformGizmo::Mode::Translate)
            {
                cfg->posX += delta.x;
                cfg->posY += delta.y;
                cfg->posZ += delta.z;
                glm::vec3 newPos(cfg->posX, cfg->posY, cfg->posZ);
                m_gizmo.beginDrag(m_gizmo.getActiveAxis(), rayOrigin, rayDir, newPos);
            }
            else if (mode == TransformGizmo::Mode::Rotate)
            {
                cfg->rotX += delta.x;
                cfg->rotY += delta.y;
                cfg->rotZ += delta.z;
                // Re-anchor at same position with new start angle
                glm::vec3 shapePos(cfg->posX, cfg->posY, cfg->posZ);
                m_gizmo.beginDrag(m_gizmo.getActiveAxis(), rayOrigin, rayDir, shapePos);
            }
            else if (mode == TransformGizmo::Mode::Scale)
            {
                cfg->scaleX += delta.x;
                cfg->scaleY += delta.y;
                cfg->scaleZ += delta.z;
                glm::vec3 shapePos(cfg->posX, cfg->posY, cfg->posZ);
                m_gizmo.beginDrag(m_gizmo.getActiveAxis(), rayOrigin, rayDir, shapePos);
            }

            onShapeConfigChanged();
            if (m_propertiesPanel) m_propertiesPanel->updateFromConfig();
        }
        return;
    }

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
    m_selectedShapeIndex = index;

    // Refresh UI if panel exists
    if (m_propertiesPanel)
    {
        m_propertiesPanel->refreshShapesList();
        m_propertiesPanel->updateFromConfig();
    }

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

        // Update properties panel with new selection
        if (m_propertiesPanel) m_propertiesPanel->updateFromConfig();
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

void DesignerApp::refreshShapesList()
{
    if (m_propertiesPanel) m_propertiesPanel->refreshShapesList();
}

bool DesignerApp::rayTriangleIntersect(const float* rayOrigin, const float* rayDir,
    const float* v0, const float* v1, const float* v2, float& t)
{
    // Moller-Trumbore intersection algorithm
    const float EPSILON = 0.0000001f;

    float edge1[3] = { v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2] };
    float edge2[3] = { v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2] };

    // h = rayDir x edge2
    float h[3] = {
        rayDir[1] * edge2[2] - rayDir[2] * edge2[1],
        rayDir[2] * edge2[0] - rayDir[0] * edge2[2],
        rayDir[0] * edge2[1] - rayDir[1] * edge2[0]
    };

    float a = edge1[0] * h[0] + edge1[1] * h[1] + edge1[2] * h[2];
    if (a > -EPSILON && a < EPSILON)
        return false;  // Ray parallel to triangle

    float f = 1.0f / a;
    float s[3] = { rayOrigin[0] - v0[0], rayOrigin[1] - v0[1], rayOrigin[2] - v0[2] };
    float u = f * (s[0] * h[0] + s[1] * h[1] + s[2] * h[2]);
    if (u < 0.0f || u > 1.0f)
        return false;

    // q = s x edge1
    float q[3] = {
        s[1] * edge1[2] - s[2] * edge1[1],
        s[2] * edge1[0] - s[0] * edge1[2],
        s[0] * edge1[1] - s[1] * edge1[0]
    };

    float v = f * (rayDir[0] * q[0] + rayDir[1] * q[1] + rayDir[2] * q[2]);
    if (v < 0.0f || u + v > 1.0f)
        return false;

    t = f * (edge2[0] * q[0] + edge2[1] * q[1] + edge2[2] * q[2]);
    return t > EPSILON;
}

void DesignerApp::unprojectRay(double mouseX, double mouseY, glm::vec3& rayOrigin, glm::vec3& rayDir)
{
    float vpX = m_panelsVisible ? static_cast<float>(m_viewportX) : 0.0f;
    float vpW = m_panelsVisible ? static_cast<float>(m_viewportWidth) : static_cast<float>(m_windowWidth);
    float vpH = static_cast<float>(m_windowHeight);

    float localX = static_cast<float>(mouseX) - vpX;
    float ndcX = (2.0f * localX / vpW) - 1.0f;
    float ndcY = 1.0f - (2.0f * static_cast<float>(mouseY) / vpH);

    glm::vec3 cameraPos(
        m_zoom * cos(glm::radians(m_rotationX)) * sin(glm::radians(m_rotationY)),
        m_zoom * sin(glm::radians(m_rotationX)),
        m_zoom * cos(glm::radians(m_rotationX)) * cos(glm::radians(m_rotationY))
    );

    glm::mat4 view = glm::lookAtLH(cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    float aspectRatio = vpW / vpH;
    glm::mat4 projection = glm::perspectiveLH(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

    glm::mat4 invVP = glm::inverse(projection * view);

    glm::vec4 nearPoint = invVP * glm::vec4(ndcX, ndcY, 0.0f, 1.0f);
    glm::vec4 farPoint = invVP * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);

    nearPoint /= nearPoint.w;
    farPoint /= farPoint.w;

    rayOrigin = glm::vec3(nearPoint);
    rayDir = glm::normalize(glm::vec3(farPoint) - glm::vec3(nearPoint));
}

int DesignerApp::pickShape(double mouseX, double mouseY)
{
    glm::vec3 rayOriginVec, rayDirVec;
    unprojectRay(mouseX, mouseY, rayOriginVec, rayDirVec);

    float rayOrigin[3] = { rayOriginVec.x, rayOriginVec.y, rayOriginVec.z };
    float rayDir[3] = { rayDirVec.x, rayDirVec.y, rayDirVec.z };

    // Test intersection with each shape
    int closestShape = -1;
    float closestT = FLT_MAX;

    for (int i = 0; i < m_shapeManager.getShapeCount(); ++i)
    {
        const ShapeInstance* shape = m_shapeManager.getShape(i);
        if (!shape || !shape->visible || shape->verts.empty())
            continue;

        // Get transform matrix for this shape
        std::array<float, 16> transform = m_shapeManager.getTransformMatrix(i);
        glm::mat4 modelMat = glm::make_mat4(transform.data());

        // Test each triangle
        for (size_t t = 0; t < shape->indices.size(); t += 3)
        {
            uint32_t i0 = shape->indices[t];
            uint32_t i1 = shape->indices[t + 1];
            uint32_t i2 = shape->indices[t + 2];

            // Get vertices and transform to world space
            glm::vec4 v0_local(shape->verts[i0 * 3], shape->verts[i0 * 3 + 1], shape->verts[i0 * 3 + 2], 1.0f);
            glm::vec4 v1_local(shape->verts[i1 * 3], shape->verts[i1 * 3 + 1], shape->verts[i1 * 3 + 2], 1.0f);
            glm::vec4 v2_local(shape->verts[i2 * 3], shape->verts[i2 * 3 + 1], shape->verts[i2 * 3 + 2], 1.0f);

            glm::vec3 v0_world = glm::vec3(modelMat * v0_local);
            glm::vec3 v1_world = glm::vec3(modelMat * v1_local);
            glm::vec3 v2_world = glm::vec3(modelMat * v2_local);

            float v0[3] = { v0_world.x, v0_world.y, v0_world.z };
            float v1[3] = { v1_world.x, v1_world.y, v1_world.z };
            float v2[3] = { v2_world.x, v2_world.y, v2_world.z };

            float hitT;
            if (rayTriangleIntersect(rayOrigin, rayDir, v0, v1, v2, hitT))
            {
                if (hitT < closestT)
                {
                    closestT = hitT;
                    closestShape = i;
                }
            }
        }
    }

    return closestShape;
}
