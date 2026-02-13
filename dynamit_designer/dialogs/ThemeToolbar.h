#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <uxtheme.h>

#include "Theme.h"

#pragma comment(lib, "uxtheme.lib")

#define ID_TT_THEME_LIGHT  9010
#define ID_TT_THEME_DARK   9011
#define ID_TT_THEME_AUTO   9012

class ThemeToolbar
{
public:
    ThemeToolbar() {}
    ~ThemeToolbar() { if (m_hwnd) DestroyWindow(m_hwnd); }

    HWND Create(HWND parent)
    {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hbrBackground = nullptr;
        wc.lpszClassName = L"ThemeToolbarClass";
        RegisterClassExW(&wc);

        const int tbs = 20;
        int w = 3 * (tbs + 2) + 6;
        int h = tbs + 6;

        m_hwnd = CreateWindowExW(
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
            L"ThemeToolbarClass",
            nullptr,
            WS_POPUP | WS_VISIBLE,
            0, 0, w, h,
            parent, nullptr, GetModuleHandle(nullptr), this);

        if (m_hwnd)
        {
            createControls();
            applyTheme();
            m_themeListenerId = Theme::instance().addListener([this]() { applyTheme(); });
        }

        return m_hwnd;
    }

    HWND GetHwnd() const { return m_hwnd; }

private:
    void createControls()
    {
        const int tbs = 20;
        int x = 3;
        int y = 3;

        m_hBtnLight = CreateWindowW(L"BUTTON", L"",
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            x, y, tbs, tbs, m_hwnd, (HMENU)ID_TT_THEME_LIGHT,
            GetModuleHandle(nullptr), nullptr);
        x += tbs + 2;

        m_hBtnDark = CreateWindowW(L"BUTTON", L"",
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            x, y, tbs, tbs, m_hwnd, (HMENU)ID_TT_THEME_DARK,
            GetModuleHandle(nullptr), nullptr);
        x += tbs + 2;

        m_hBtnAuto = CreateWindowW(L"BUTTON", L"",
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            x, y, tbs, tbs, m_hwnd, (HMENU)ID_TT_THEME_AUTO,
            GetModuleHandle(nullptr), nullptr);
    }

    void applyTheme()
    {
        if (!m_hwnd) return;
        ::InvalidateRect(m_hwnd, nullptr, TRUE);
        if (m_hBtnLight) ::InvalidateRect(m_hBtnLight, nullptr, TRUE);
        if (m_hBtnDark) ::InvalidateRect(m_hBtnDark, nullptr, TRUE);
        if (m_hBtnAuto) ::InvalidateRect(m_hBtnAuto, nullptr, TRUE);
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        ThemeToolbar* pThis = nullptr;

        if (msg == WM_CREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (ThemeToolbar*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        }
        else
        {
            pThis = (ThemeToolbar*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }

        if (pThis)
            return pThis->handleMessage(hwnd, msg, wParam, lParam);

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_DRAWITEM:
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

        case WM_COMMAND:
        {
            WORD id = LOWORD(wParam);
            if (id == ID_TT_THEME_LIGHT || id == ID_TT_THEME_DARK || id == ID_TT_THEME_AUTO)
            {
                ThemeMode mode = ThemeMode::Auto;
                if (id == ID_TT_THEME_LIGHT) mode = ThemeMode::Light;
                else if (id == ID_TT_THEME_DARK) mode = ThemeMode::Dark;
                Theme::instance().setMode(mode);
                return 0;
            }
            break;
        }

        case WM_ERASEBKGND:
        {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, Theme::instance().backgroundBrush());
            return TRUE;
        }

        case WM_CLOSE:
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    HWND m_hwnd = nullptr;
    HWND m_hBtnLight = nullptr;
    HWND m_hBtnDark = nullptr;
    HWND m_hBtnAuto = nullptr;
    int m_themeListenerId = 0;
};
