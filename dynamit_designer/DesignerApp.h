#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <array>

#include "ShapeManager.h"
#include "VisualizationHelpers.h"

// Forward declarations for dialog panels
class MainToolbar;
class ExportToolbar;
class BuilderPanel;
class TransformPanel;
class ColorPanel;
class ViewPanel;

class DesignerApp
{
public:
    DesignerApp(HINSTANCE hInstance, GLFWwindow* window, int width, int height, int panelWidth);
    ~DesignerApp();

    bool initialize();
    void shutdown();
    void update(float deltaTime);
    void render();

    // Event handlers
    void onResize(int width, int height);
    void onKey(int key, int scancode, int action, int mods);
    void onMouseButton(int button, int action, int mods, double x, double y);
    void onMouseMove(double x, double y);
    void onScroll(double xoffset, double yoffset);

    // Shape management
    void newShape(ShapeConfig::Type type);
    void deleteSelectedShape();
    void selectShape(int index);

    // Getters
    ShapeManager& getShapeManager() { return m_shapeManager; }
    int getSelectedShapeIndex() const { return m_selectedShapeIndex; }
    ShapeConfig* getSelectedShapeConfig();

    // Update notifications from panels
    void onShapeConfigChanged();

    // Normals visualization
    bool getShowNormals() const { return m_showNormals; }
    void setShowNormals(bool show) { m_showNormals = show; }

    // Visualization helpers (axes, light, grid)
    VisualizationHelpers& getVisualizationHelpers() { return m_vizHelpers; }

    // View state (for project save/load)
    float getRotationX() const { return m_rotationX; }
    float getRotationY() const { return m_rotationY; }
    float getZoom() const { return m_zoom; }
    void setRotationX(float v) { m_rotationX = v; }
    void setRotationY(float v) { m_rotationY = v; }
    void setZoom(float v) { m_zoom = v; }

    // Get GLFW window for dialogs
    GLFWwindow* getWindow() const { return m_window; }

private:
    void createDialogs();
    void updateDialogPositions();
    std::array<float, 16> computeViewProjection();

    HINSTANCE m_hInstance;
    GLFWwindow* m_window;

    int m_windowWidth;
    int m_windowHeight;
    int m_panelWidth;
    int m_viewportX;
    int m_viewportWidth;

    // View state
    float m_rotationX;
    float m_rotationY;
    float m_zoom;
    bool m_wireframeMode;
    bool m_panelsVisible;
    bool m_showNormals;

    // Mouse state
    bool m_mouseDragging;
    double m_lastMouseX;
    double m_lastMouseY;

    // Shape management
    ShapeManager m_shapeManager;
    int m_selectedShapeIndex;

    // Visualization helpers
    VisualizationHelpers m_vizHelpers;

    // Dialog panels
    std::unique_ptr<MainToolbar> m_mainToolbar;
    std::unique_ptr<ExportToolbar> m_exportToolbar;
    std::unique_ptr<BuilderPanel> m_builderPanel;
    std::unique_ptr<TransformPanel> m_transformPanel;
    std::unique_ptr<ColorPanel> m_colorPanel;
    std::unique_ptr<ViewPanel> m_viewPanel;

    HWND m_mainToolbarHwnd;
    HWND m_exportToolbarHwnd;
    HWND m_builderPanelHwnd;
    HWND m_transformPanelHwnd;
    HWND m_colorPanelHwnd;
    HWND m_viewPanelHwnd;
};
