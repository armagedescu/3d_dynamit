#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

#include <atlbase.h>
#include <atlwin.h>

class SplashScreen : public CWindowImpl<SplashScreen>
{
public:
    DECLARE_WND_CLASS(L"DynamitSplashClass")

    void Show(HINSTANCE hInstance, int durationMs = 2500)
    {
        m_hInstance = hInstance;
        m_durationMs = durationMs;

        int clientW = 420;
        int clientH = 220;

        RECT rc = { 0, 0, clientW, clientH };
        DWORD style = WS_POPUP;
        DWORD exStyle = WS_EX_TOPMOST;
        AdjustWindowRectEx(&rc, style, FALSE, exStyle);
        int dlgW = rc.right - rc.left;
        int dlgH = rc.bottom - rc.top;

        int screenW = GetSystemMetrics(SM_CXSCREEN);
        int screenH = GetSystemMetrics(SM_CYSCREEN);
        int x = (screenW - dlgW) / 2;
        int y = (screenH - dlgH) / 2;

        RECT createRect = { x, y, x + dlgW, y + dlgH };
        Create(nullptr, createRect, nullptr, style, exStyle);

        if (!m_hWnd)
            return;

        ShowWindow(SW_SHOW);
        UpdateWindow();

        // Timer for auto-dismiss
        SetTimer(1, m_durationMs, nullptr);

        // Modal message loop
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            if (!IsWindow())
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    BEGIN_MSG_MAP(SplashScreen)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnClick)
        MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    END_MSG_MAP()

private:
    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&)
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(&ps);

        RECT rc;
        GetClientRect(&rc);
        int w = rc.right;
        int h = rc.bottom;

        // Background - dark gradient-like solid
        HBRUSH bgBrush = CreateSolidBrush(RGB(20, 20, 35));
        FillRect(hdc, &rc, bgBrush);
        DeleteObject(bgBrush);

        // Accent line at top
        RECT accentRect = { 0, 0, w, 3 };
        HBRUSH accentBrush = CreateSolidBrush(RGB(80, 140, 255));
        FillRect(hdc, &accentRect, accentBrush);
        DeleteObject(accentBrush);

        SetBkMode(hdc, TRANSPARENT);

        // App name
        LOGFONTW lfTitle = {};
        lfTitle.lfHeight = -36;
        lfTitle.lfWeight = FW_BOLD;
        wcscpy_s(lfTitle.lfFaceName, L"Segoe UI");
        HFONT hTitleFont = CreateFontIndirectW(&lfTitle);
        HFONT hOldFont = (HFONT)SelectObject(hdc, hTitleFont);

        SetTextColor(hdc, RGB(230, 235, 255));
        RECT titleRect = { 0, 50, w, 100 };
        DrawTextW(hdc, L"Dynamit Designer", -1, &titleRect, DT_CENTER | DT_SINGLELINE);

        // Subtitle
        LOGFONTW lfSub = {};
        lfSub.lfHeight = -14;
        lfSub.lfWeight = FW_NORMAL;
        wcscpy_s(lfSub.lfFaceName, L"Segoe UI");
        HFONT hSubFont = CreateFontIndirectW(&lfSub);
        SelectObject(hdc, hSubFont);

        SetTextColor(hdc, RGB(140, 150, 180));
        RECT subRect = { 0, 108, w, 135 };
        DrawTextW(hdc, L"Visual Shape Builder", -1, &subRect, DT_CENTER | DT_SINGLELINE);

        // "Click to continue" hint
        LOGFONTW lfHint = {};
        lfHint.lfHeight = -11;
        lfHint.lfWeight = FW_NORMAL;
        wcscpy_s(lfHint.lfFaceName, L"Segoe UI");
        HFONT hHintFont = CreateFontIndirectW(&lfHint);
        SelectObject(hdc, hHintFont);

        SetTextColor(hdc, RGB(90, 95, 110));
        RECT hintRect = { 0, h - 30, w, h - 10 };
        DrawTextW(hdc, L"Click or press any key to continue", -1, &hintRect, DT_CENTER | DT_SINGLELINE);

        SelectObject(hdc, hOldFont);
        DeleteObject(hTitleFont);
        DeleteObject(hSubFont);
        DeleteObject(hHintFont);

        EndPaint(&ps);
        return 0;
    }

    LRESULT OnTimer(UINT, WPARAM wParam, LPARAM, BOOL&)
    {
        if (wParam == 1)
            dismiss();
        return 0;
    }

    LRESULT OnClick(UINT, WPARAM, LPARAM, BOOL&)
    {
        dismiss();
        return 0;
    }

    LRESULT OnKeyDown(UINT, WPARAM, LPARAM, BOOL&)
    {
        dismiss();
        return 0;
    }

    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&)
    {
        KillTimer(1);
        PostQuitMessage(0);
        return 0;
    }

    void dismiss()
    {
        KillTimer(1);
        DestroyWindow();
    }

    HINSTANCE m_hInstance = nullptr;
    int m_durationMs = 2500;
};
