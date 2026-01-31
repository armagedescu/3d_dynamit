#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

class DesignerApp;

// Control IDs
#define ID_CHK_AXIS_X       5001
#define ID_CHK_AXIS_Y       5002
#define ID_CHK_AXIS_Z       5003
#define ID_CHK_GRID_3D      5004
#define ID_CHK_GRID_XZ      5005
#define ID_CHK_GRID_XY      5006
#define ID_CHK_GRID_YZ      5007
#define ID_CHK_LIGHT_DIR    5008
#define ID_CHK_NORMALS      5009

class ViewPanel
{
public:
    ViewPanel(DesignerApp* app) : m_app(app), m_hwnd(nullptr), m_updating(false) {}
    ~ViewPanel() { if (m_hwnd) DestroyWindow(m_hwnd); }

    HWND Create(HWND parent)
    {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
        wc.lpszClassName = L"ViewPanelClass";
        RegisterClassExW(&wc);

        m_hwnd = CreateWindowExW(
            WS_EX_TOOLWINDOW,
            L"ViewPanelClass",
            L"View Options",
            WS_POPUP | WS_CAPTION | WS_VISIBLE,
            0, 0, 160, 240,
            parent, nullptr, GetModuleHandle(nullptr), this);

        if (m_hwnd)
        {
            createControls();
        }

        return m_hwnd;
    }

    HWND GetHwnd() const { return m_hwnd; }

    void updateFromState();

private:
    void createControls()
    {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        int y = 8;
        int rowH = 22;

        auto createCheck = [&](const wchar_t* text, int id) -> HWND {
            HWND h = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                10, y, 140, 18, m_hwnd, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
            SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
            y += rowH;
            return h;
        };

        m_chkAxisX = createCheck(L"X Axis (Red)", ID_CHK_AXIS_X);
        m_chkAxisY = createCheck(L"Y Axis (Green)", ID_CHK_AXIS_Y);
        m_chkAxisZ = createCheck(L"Z Axis (Blue)", ID_CHK_AXIS_Z);
        m_chkGrid3D = createCheck(L"3D Grid", ID_CHK_GRID_3D);
        m_chkGridXZ = createCheck(L"XZ Grid", ID_CHK_GRID_XZ);
        m_chkGridXY = createCheck(L"XY Grid", ID_CHK_GRID_XY);
        m_chkGridYZ = createCheck(L"YZ Grid", ID_CHK_GRID_YZ);
        m_chkLightDir = createCheck(L"Light Direction", ID_CHK_LIGHT_DIR);
        m_chkNormals = createCheck(L"Show Normals", ID_CHK_NORMALS);

        // Set initial state (2D grids off by default)
        SendMessage(m_chkAxisX, BM_SETCHECK, BST_CHECKED, 0);
        SendMessage(m_chkAxisY, BM_SETCHECK, BST_CHECKED, 0);
        SendMessage(m_chkAxisZ, BM_SETCHECK, BST_CHECKED, 0);
        SendMessage(m_chkGrid3D, BM_SETCHECK, BST_CHECKED, 0);
        SendMessage(m_chkLightDir, BM_SETCHECK, BST_CHECKED, 0);
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        ViewPanel* pThis = nullptr;

        if (msg == WM_CREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (ViewPanel*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        }
        else
        {
            pThis = (ViewPanel*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }

        if (pThis)
        {
            return pThis->handleMessage(hwnd, msg, wParam, lParam);
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void applyChanges();

    DesignerApp* m_app;
    HWND m_hwnd;
    bool m_updating;

    HWND m_chkAxisX;
    HWND m_chkAxisY;
    HWND m_chkAxisZ;
    HWND m_chkGrid3D;
    HWND m_chkGridXZ;
    HWND m_chkGridXY;
    HWND m_chkGridYZ;
    HWND m_chkLightDir;
    HWND m_chkNormals;
};

// Include implementation
#include "../DesignerApp.h"
#include "../VisualizationHelpers.h"

inline void ViewPanel::updateFromState()
{
    if (!m_hwnd || !m_app) return;

    m_updating = true;

    VisualizationHelpers& viz = m_app->getVisualizationHelpers();
    SendMessage(m_chkAxisX, BM_SETCHECK, viz.getShowAxisX() ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(m_chkAxisY, BM_SETCHECK, viz.getShowAxisY() ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(m_chkAxisZ, BM_SETCHECK, viz.getShowAxisZ() ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(m_chkGrid3D, BM_SETCHECK, viz.getShowGrid() ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(m_chkGridXZ, BM_SETCHECK, viz.getShowGridXZ() ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(m_chkGridXY, BM_SETCHECK, viz.getShowGridXY() ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(m_chkGridYZ, BM_SETCHECK, viz.getShowGridYZ() ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(m_chkLightDir, BM_SETCHECK, viz.getShowLight() ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(m_chkNormals, BM_SETCHECK, m_app->getShowNormals() ? BST_CHECKED : BST_UNCHECKED, 0);

    m_updating = false;
}

inline void ViewPanel::applyChanges()
{
    if (m_updating || !m_app) return;

    VisualizationHelpers& viz = m_app->getVisualizationHelpers();
    viz.setShowAxisX(SendMessage(m_chkAxisX, BM_GETCHECK, 0, 0) == BST_CHECKED);
    viz.setShowAxisY(SendMessage(m_chkAxisY, BM_GETCHECK, 0, 0) == BST_CHECKED);
    viz.setShowAxisZ(SendMessage(m_chkAxisZ, BM_GETCHECK, 0, 0) == BST_CHECKED);
    viz.setShowGrid(SendMessage(m_chkGrid3D, BM_GETCHECK, 0, 0) == BST_CHECKED);
    viz.setShowGridXZ(SendMessage(m_chkGridXZ, BM_GETCHECK, 0, 0) == BST_CHECKED);
    viz.setShowGridXY(SendMessage(m_chkGridXY, BM_GETCHECK, 0, 0) == BST_CHECKED);
    viz.setShowGridYZ(SendMessage(m_chkGridYZ, BM_GETCHECK, 0, 0) == BST_CHECKED);
    viz.setShowLight(SendMessage(m_chkLightDir, BM_GETCHECK, 0, 0) == BST_CHECKED);
    m_app->setShowNormals(SendMessage(m_chkNormals, BM_GETCHECK, 0, 0) == BST_CHECKED);
}

inline LRESULT ViewPanel::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED)
        {
            applyChanges();
        }
        return 0;

    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
