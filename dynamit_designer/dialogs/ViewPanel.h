#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>

#include <atlbase.h>
#include <atlwin.h>

#include "Theme.h"

#pragma comment(lib, "uxtheme.lib")

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

class ViewPanel : public CWindowImpl<ViewPanel>
{
public:
    DECLARE_WND_CLASS(L"ViewPanelClass")

    ViewPanel(DesignerApp* app) : m_app(app), m_updating(false) {}

    HWND Create(HWND parent)
    {
        RECT rc = { 0, 0, 160, 240 };
        CWindowImpl::Create(parent, rc, L"View Options",
            WS_POPUP | WS_CAPTION | WS_VISIBLE, WS_EX_TOOLWINDOW);

        if (m_hWnd)
        {
            createControls();
            applyTheme();
            m_themeListenerId = Theme::instance().addListener([this]() { applyTheme(); });
        }

        return m_hWnd;
    }

    void updateFromState();

    BEGIN_MSG_MAP(ViewPanel)
        COMMAND_HANDLER(ID_CHK_AXIS_X, BN_CLICKED, OnCheckChanged)
        COMMAND_HANDLER(ID_CHK_AXIS_Y, BN_CLICKED, OnCheckChanged)
        COMMAND_HANDLER(ID_CHK_AXIS_Z, BN_CLICKED, OnCheckChanged)
        COMMAND_HANDLER(ID_CHK_GRID_3D, BN_CLICKED, OnCheckChanged)
        COMMAND_HANDLER(ID_CHK_GRID_XZ, BN_CLICKED, OnCheckChanged)
        COMMAND_HANDLER(ID_CHK_GRID_XY, BN_CLICKED, OnCheckChanged)
        COMMAND_HANDLER(ID_CHK_GRID_YZ, BN_CLICKED, OnCheckChanged)
        COMMAND_HANDLER(ID_CHK_LIGHT_DIR, BN_CLICKED, OnCheckChanged)
        COMMAND_HANDLER(ID_CHK_NORMALS, BN_CLICKED, OnCheckChanged)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor)
        MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColor)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    END_MSG_MAP()

private:
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        ShowWindow(SW_HIDE);
        return 0;
    }

    LRESULT OnCtlColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        return (LRESULT)Theme::instance().onCtlColorStatic((HDC)wParam);
    }

    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(&rc);
        FillRect(hdc, &rc, Theme::instance().backgroundBrush());
        return TRUE;
    }

    LRESULT OnCheckChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        applyChanges();
        return 0;
    }

    void createControls()
    {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        int y = 8;
        int rowH = 22;

        auto createCheck = [&](const wchar_t* text, int id) -> HWND {
            HWND h = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                10, y, 140, 18, m_hWnd, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
            ::SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
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
        ::SendMessage(m_chkAxisX, BM_SETCHECK, BST_CHECKED, 0);
        ::SendMessage(m_chkAxisY, BM_SETCHECK, BST_CHECKED, 0);
        ::SendMessage(m_chkAxisZ, BM_SETCHECK, BST_CHECKED, 0);
        ::SendMessage(m_chkGrid3D, BM_SETCHECK, BST_CHECKED, 0);
        ::SendMessage(m_chkLightDir, BM_SETCHECK, BST_CHECKED, 0);
    }

    void applyChanges();

    void applyTheme()
    {
        if (!m_hWnd) return;
        Theme::applyTitleBar(m_hWnd, Theme::instance().isDark());
        // Disable visual styles on checkboxes so WM_CTLCOLORSTATIC text color works
        ::EnumChildWindows(m_hWnd, [](HWND child, LPARAM) -> BOOL {
            SetWindowTheme(child, L"", L"");
            ::InvalidateRect(child, nullptr, TRUE);
            return TRUE;
        }, 0);
        ::InvalidateRect(m_hWnd, nullptr, TRUE);
    }

    DesignerApp* m_app;
    bool m_updating;
    int m_themeListenerId = 0;

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
    if (!m_hWnd || !m_app) return;

    m_updating = true;

    VisualizationHelpers& viz = m_app->getVisualizationHelpers();
    ::SendMessage(m_chkAxisX, BM_SETCHECK, viz.getShowAxisX() ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(m_chkAxisY, BM_SETCHECK, viz.getShowAxisY() ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(m_chkAxisZ, BM_SETCHECK, viz.getShowAxisZ() ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(m_chkGrid3D, BM_SETCHECK, viz.getShowGrid() ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(m_chkGridXZ, BM_SETCHECK, viz.getShowGridXZ() ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(m_chkGridXY, BM_SETCHECK, viz.getShowGridXY() ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(m_chkGridYZ, BM_SETCHECK, viz.getShowGridYZ() ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(m_chkLightDir, BM_SETCHECK, viz.getShowLight() ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(m_chkNormals, BM_SETCHECK, m_app->getShowNormals() ? BST_CHECKED : BST_UNCHECKED, 0);

    m_updating = false;
}

inline void ViewPanel::applyChanges()
{
    if (m_updating || !m_app) return;

    VisualizationHelpers& viz = m_app->getVisualizationHelpers();
    viz.setShowAxisX(::SendMessage(m_chkAxisX, BM_GETCHECK, 0, 0) == BST_CHECKED);
    viz.setShowAxisY(::SendMessage(m_chkAxisY, BM_GETCHECK, 0, 0) == BST_CHECKED);
    viz.setShowAxisZ(::SendMessage(m_chkAxisZ, BM_GETCHECK, 0, 0) == BST_CHECKED);
    viz.setShowGrid(::SendMessage(m_chkGrid3D, BM_GETCHECK, 0, 0) == BST_CHECKED);
    viz.setShowGridXZ(::SendMessage(m_chkGridXZ, BM_GETCHECK, 0, 0) == BST_CHECKED);
    viz.setShowGridXY(::SendMessage(m_chkGridXY, BM_GETCHECK, 0, 0) == BST_CHECKED);
    viz.setShowGridYZ(::SendMessage(m_chkGridYZ, BM_GETCHECK, 0, 0) == BST_CHECKED);
    viz.setShowLight(::SendMessage(m_chkLightDir, BM_GETCHECK, 0, 0) == BST_CHECKED);
    m_app->setShowNormals(::SendMessage(m_chkNormals, BM_GETCHECK, 0, 0) == BST_CHECKED);
}
