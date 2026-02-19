#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <uxtheme.h>

#include <atlbase.h>
#include <atlwin.h>

#include "Theme.h"

#pragma comment(lib, "uxtheme.lib")

#define ID_TT_THEME_LIGHT  9010
#define ID_TT_THEME_DARK   9011
#define ID_TT_THEME_AUTO   9012

class ThemeToolbar : public CWindowImpl<ThemeToolbar>
{
public:
    DECLARE_WND_CLASS(L"ThemeToolbarClass")

    ThemeToolbar() {}

    HWND Create(HWND parent)
    {
        const int tbs = 20;
        int w = 3 * (tbs + 2) + 6;
        int h = tbs + 6;

        RECT rc = { 0, 0, w, h };
        CWindowImpl::Create(parent, rc, nullptr,
            WS_POPUP | WS_VISIBLE, WS_EX_TOOLWINDOW | WS_EX_TOPMOST);

        if (m_hWnd)
        {
            createControls();
            applyTheme();
            m_themeListenerId = Theme::instance().addListener([this]() { applyTheme(); });
        }

        return m_hWnd;
    }

    BEGIN_MSG_MAP(ThemeToolbar)
        MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
        COMMAND_ID_HANDLER(ID_TT_THEME_LIGHT, OnThemeButton)
        COMMAND_ID_HANDLER(ID_TT_THEME_DARK, OnThemeButton)
        COMMAND_ID_HANDLER(ID_TT_THEME_AUTO, OnThemeButton)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
    END_MSG_MAP()

private:
    LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
        if (dis->CtlID == ID_TT_THEME_LIGHT)
            Theme::drawThemeButton(dis, ThemeMode::Light);
        else if (dis->CtlID == ID_TT_THEME_DARK)
            Theme::drawThemeButton(dis, ThemeMode::Dark);
        else if (dis->CtlID == ID_TT_THEME_AUTO)
            Theme::drawThemeButton(dis, ThemeMode::Auto);
        return TRUE;
    }

    LRESULT OnThemeButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        ThemeMode mode = ThemeMode::Auto;
        if (wID == ID_TT_THEME_LIGHT) mode = ThemeMode::Light;
        else if (wID == ID_TT_THEME_DARK) mode = ThemeMode::Dark;
        Theme::instance().setMode(mode);
        return 0;
    }

    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(&rc);
        FillRect(hdc, &rc, Theme::instance().backgroundBrush());
        return TRUE;
    }

    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        ShowWindow(SW_HIDE);
        return 0;
    }

    void createControls()
    {
        const int tbs = 20;
        int x = 3;
        int y = 3;

        m_hBtnLight = CreateWindowW(L"BUTTON", L"",
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            x, y, tbs, tbs, m_hWnd, (HMENU)ID_TT_THEME_LIGHT,
            GetModuleHandle(nullptr), nullptr);
        x += tbs + 2;

        m_hBtnDark = CreateWindowW(L"BUTTON", L"",
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            x, y, tbs, tbs, m_hWnd, (HMENU)ID_TT_THEME_DARK,
            GetModuleHandle(nullptr), nullptr);
        x += tbs + 2;

        m_hBtnAuto = CreateWindowW(L"BUTTON", L"",
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            x, y, tbs, tbs, m_hWnd, (HMENU)ID_TT_THEME_AUTO,
            GetModuleHandle(nullptr), nullptr);
    }

    void applyTheme()
    {
        if (!m_hWnd) return;
        ::InvalidateRect(m_hWnd, nullptr, TRUE);
        if (m_hBtnLight) ::InvalidateRect(m_hBtnLight, nullptr, TRUE);
        if (m_hBtnDark) ::InvalidateRect(m_hBtnDark, nullptr, TRUE);
        if (m_hBtnAuto) ::InvalidateRect(m_hBtnAuto, nullptr, TRUE);
    }

    HWND m_hBtnLight = nullptr;
    HWND m_hBtnDark = nullptr;
    HWND m_hBtnAuto = nullptr;
    int m_themeListenerId = 0;
};
