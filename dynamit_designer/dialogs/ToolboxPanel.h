#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <uxtheme.h>

#include <atlbase.h>
#include <atlwin.h>

#include "Theme.h"

#pragma comment(lib, "uxtheme.lib")

class DesignerApp;

// Control IDs for ToolboxPanel
#define ID_TB_BTN_CONE      7001
#define ID_TB_BTN_CYLINDER  7002

class ToolboxPanel : public CWindowImpl<ToolboxPanel>
{
public:
    DECLARE_WND_CLASS(L"ToolboxPanelClass")

    ToolboxPanel(DesignerApp* app) : m_app(app) {}

    HWND Create(HWND parent)
    {
        // Client area: buttons at y=6, height=24, bottom margin=6 → client height=36
        RECT rc = { 0, 0, 290, 36 };
        AdjustWindowRect(&rc, WS_POPUP | WS_CAPTION, FALSE);
        rc.right -= rc.left; rc.bottom -= rc.top; rc.left = 0; rc.top = 0;
        CWindowImpl::Create(parent, rc, L"Toolbox",
            WS_POPUP | WS_CAPTION | WS_VISIBLE, WS_EX_TOOLWINDOW);

        if (m_hWnd)
        {
            createControls();
            applyTheme();
            m_themeListenerId = Theme::instance().addListener([this]() { applyTheme(); });
        }

        return m_hWnd;
    }

    BEGIN_MSG_MAP(ToolboxPanel)
        COMMAND_ID_HANDLER(ID_TB_BTN_CONE, OnNewCone)
        COMMAND_ID_HANDLER(ID_TB_BTN_CYLINDER, OnNewCylinder)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor)
        MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColor)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    END_MSG_MAP()

private:
    LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&)
    {
        ShowWindow(SW_HIDE);
        return 0;
    }

    LRESULT OnCtlColor(UINT, WPARAM wParam, LPARAM, BOOL&)
    {
        return (LRESULT)Theme::instance().onCtlColorStatic((HDC)wParam);
    }

    LRESULT OnEraseBkgnd(UINT, WPARAM wParam, LPARAM, BOOL&)
    {
        RECT rc;
        GetClientRect(&rc);
        FillRect((HDC)wParam, &rc, Theme::instance().backgroundBrush());
        return TRUE;
    }

    LRESULT OnNewCone(WORD, WORD, HWND, BOOL&);
    LRESULT OnNewCylinder(WORD, WORD, HWND, BOOL&);

    void createControls()
    {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        int x = 8, y = 6, btnW = 60, btnH = 24, gap = 6;

        HWND btnCone = CreateWindowW(L"BUTTON", L"+Cone",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            x, y, btnW, btnH, m_hWnd, (HMENU)ID_TB_BTN_CONE,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(btnCone, WM_SETFONT, (WPARAM)hFont, TRUE);
        x += btnW + gap;

        HWND btnCyl = CreateWindowW(L"BUTTON", L"+Cylinder",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            x, y, 70, btnH, m_hWnd, (HMENU)ID_TB_BTN_CYLINDER,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(btnCyl, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    void applyTheme()
    {
        if (!m_hWnd) return;
        Theme::applyTitleBar(m_hWnd, Theme::instance().isDark());
        ::EnumChildWindows(m_hWnd, [](HWND child, LPARAM) -> BOOL {
            SetWindowTheme(child, L"", L"");
            ::InvalidateRect(child, nullptr, TRUE);
            return TRUE;
        }, 0);
        ::InvalidateRect(m_hWnd, nullptr, TRUE);
    }

    DesignerApp* m_app;
    int m_themeListenerId = 0;
};

#include "../DesignerApp.h"
#include "../ShapeManager.h"

inline LRESULT ToolboxPanel::OnNewCone(WORD, WORD, HWND, BOOL&)
{
    if (m_app) m_app->newShape(ShapeConfig::Type::Cone);
    return 0;
}

inline LRESULT ToolboxPanel::OnNewCylinder(WORD, WORD, HWND, BOOL&)
{
    if (m_app) m_app->newShape(ShapeConfig::Type::Cylinder);
    return 0;
}
