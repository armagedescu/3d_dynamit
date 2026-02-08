#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include <atlbase.h>
#include <atlwin.h>

class DesignerApp;

// Control IDs
#define ID_BTN_NEW_CONE     1001
#define ID_BTN_NEW_CYLINDER 1002
#define ID_BTN_DELETE       1003
#define ID_CHK_SHOW_NORMALS 1004

class MainToolbar : public CWindowImpl<MainToolbar>
{
public:
    DECLARE_WND_CLASS(L"MainToolbarClass")

    MainToolbar(DesignerApp* app) : m_app(app) {}

    HWND Create(HWND parent)
    {
        RECT rc = { 0, 0, 280, 120 };
        CWindowImpl::Create(parent, rc, L"Shapes",
            WS_POPUP | WS_CAPTION | WS_VISIBLE, WS_EX_TOOLWINDOW);

        if (m_hWnd)
        {
            createControls();
        }

        return m_hWnd;
    }

    BEGIN_MSG_MAP(MainToolbar)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        COMMAND_ID_HANDLER(ID_BTN_NEW_CONE, OnNewCone)
        COMMAND_ID_HANDLER(ID_BTN_NEW_CYLINDER, OnNewCylinder)
        COMMAND_ID_HANDLER(ID_BTN_DELETE, OnDelete)
        COMMAND_ID_HANDLER(ID_CHK_SHOW_NORMALS, OnShowNormals)
    END_MSG_MAP()

private:
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        ShowWindow(SW_HIDE);
        return 0;
    }

    LRESULT OnNewCone(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnNewCylinder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnShowNormals(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    void createControls()
    {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        // New Cone button
        HWND btnCone = CreateWindowW(L"BUTTON", L"New Cone",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 5, 80, 25, m_hWnd, (HMENU)ID_BTN_NEW_CONE,
            GetModuleHandle(nullptr), nullptr);
        ::SendMessage(btnCone, WM_SETFONT, (WPARAM)hFont, TRUE);

        // New Cylinder button
        HWND btnCyl = CreateWindowW(L"BUTTON", L"New Cylinder",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            100, 5, 90, 25, m_hWnd, (HMENU)ID_BTN_NEW_CYLINDER,
            GetModuleHandle(nullptr), nullptr);
        ::SendMessage(btnCyl, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Delete button
        HWND btnDel = CreateWindowW(L"BUTTON", L"Delete",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            200, 5, 65, 25, m_hWnd, (HMENU)ID_BTN_DELETE,
            GetModuleHandle(nullptr), nullptr);
        ::SendMessage(btnDel, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Show Normals checkbox
        HWND chkNormals = CreateWindowW(L"BUTTON", L"Show Normals",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, 40, 120, 20, m_hWnd, (HMENU)ID_CHK_SHOW_NORMALS,
            GetModuleHandle(nullptr), nullptr);
        ::SendMessage(chkNormals, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    DesignerApp* m_app;
};

// Include implementation inline to avoid separate cpp
#include "../DesignerApp.h"
#include "../ShapeManager.h"

inline LRESULT MainToolbar::OnNewCone(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_app) m_app->newShape(ShapeConfig::Type::Cone);
    return 0;
}

inline LRESULT MainToolbar::OnNewCylinder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_app) m_app->newShape(ShapeConfig::Type::Cylinder);
    return 0;
}

inline LRESULT MainToolbar::OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_app) m_app->deleteSelectedShape();
    return 0;
}

inline LRESULT MainToolbar::OnShowNormals(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_app)
    {
        HWND chk = GetDlgItem(ID_CHK_SHOW_NORMALS);
        bool checked = (::SendMessage(chk, BM_GETCHECK, 0, 0) == BST_CHECKED);
        m_app->setShowNormals(checked);
    }
    return 0;
}
