#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>

#pragma comment(lib, "comdlg32.lib")

class DesignerApp;

// Control IDs
#define ID_EDIT_OUTER_R   4001
#define ID_EDIT_OUTER_G   4002
#define ID_EDIT_OUTER_B   4003
#define ID_EDIT_OUTER_A   4004
#define ID_EDIT_INNER_R   4005
#define ID_EDIT_INNER_G   4006
#define ID_EDIT_INNER_B   4007
#define ID_EDIT_INNER_A   4008
#define ID_BTN_OUTER_PICK 4009
#define ID_BTN_INNER_PICK 4010

class ColorPanel
{
public:
    ColorPanel(DesignerApp* app) : m_app(app), m_hwnd(nullptr), m_updating(false) {}
    ~ColorPanel() { if (m_hwnd) DestroyWindow(m_hwnd); }

    HWND Create(HWND parent)
    {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
        wc.lpszClassName = L"ColorPanelClass";
        RegisterClassExW(&wc);

        m_hwnd = CreateWindowExW(
            WS_EX_TOOLWINDOW,
            L"ColorPanelClass",
            L"Colors",
            WS_POPUP | WS_CAPTION | WS_VISIBLE,
            0, 0, 280, 200,
            parent, nullptr, GetModuleHandle(nullptr), this);

        if (m_hwnd)
        {
            createControls();
        }

        return m_hwnd;
    }

    HWND GetHwnd() const { return m_hwnd; }

    void updateFromConfig();

private:
    void createControls()
    {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        int y = 5;
        int labelW = 55;
        int editW = 45;
        int gap = 3;

        auto createLabel = [&](const wchar_t* text, int xPos, int yPos, int width = 0) {
            HWND h = CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE,
                xPos, yPos + 3, width > 0 ? width : labelW, 18, m_hwnd, nullptr, GetModuleHandle(nullptr), nullptr);
            SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
        };

        auto createEdit = [&](int id, int xPos, int yPos) -> HWND {
            HWND h = CreateWindowW(L"EDIT", L"1.0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                xPos, yPos, editW, 20, m_hwnd, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
            SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
            return h;
        };

        // Header row
        createLabel(L"", 5, y);
        createLabel(L"R", labelW + 15, y, 30);
        createLabel(L"G", labelW + 15 + editW + gap, y, 30);
        createLabel(L"B", labelW + 15 + 2 * (editW + gap), y, 30);
        createLabel(L"A", labelW + 15 + 3 * (editW + gap), y, 30);
        y += 22;

        // Outer color
        createLabel(L"Outer:", 5, y);
        m_editOuterR = createEdit(ID_EDIT_OUTER_R, labelW + 10, y);
        m_editOuterG = createEdit(ID_EDIT_OUTER_G, labelW + 10 + editW + gap, y);
        m_editOuterB = createEdit(ID_EDIT_OUTER_B, labelW + 10 + 2 * (editW + gap), y);
        m_editOuterA = createEdit(ID_EDIT_OUTER_A, labelW + 10 + 3 * (editW + gap), y);
        y += 26;

        // Outer color preview/picker
        m_outerPreview = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_NOTIFY | WS_BORDER,
            labelW + 10, y, 80, 20, m_hwnd, (HMENU)ID_BTN_OUTER_PICK, GetModuleHandle(nullptr), nullptr);
        HWND btnOuterPick = CreateWindowW(L"BUTTON", L"Pick...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            labelW + 100, y, 50, 20, m_hwnd, (HMENU)ID_BTN_OUTER_PICK, GetModuleHandle(nullptr), nullptr);
        SendMessage(btnOuterPick, WM_SETFONT, (WPARAM)hFont, TRUE);
        y += 30;

        // Inner color
        createLabel(L"Inner:", 5, y);
        m_editInnerR = createEdit(ID_EDIT_INNER_R, labelW + 10, y);
        m_editInnerG = createEdit(ID_EDIT_INNER_G, labelW + 10 + editW + gap, y);
        m_editInnerB = createEdit(ID_EDIT_INNER_B, labelW + 10 + 2 * (editW + gap), y);
        m_editInnerA = createEdit(ID_EDIT_INNER_A, labelW + 10 + 3 * (editW + gap), y);
        y += 26;

        // Inner color preview/picker
        m_innerPreview = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_NOTIFY | WS_BORDER,
            labelW + 10, y, 80, 20, m_hwnd, (HMENU)ID_BTN_INNER_PICK, GetModuleHandle(nullptr), nullptr);
        HWND btnInnerPick = CreateWindowW(L"BUTTON", L"Pick...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            labelW + 100, y, 50, 20, m_hwnd, (HMENU)ID_BTN_INNER_PICK, GetModuleHandle(nullptr), nullptr);
        SendMessage(btnInnerPick, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        ColorPanel* pThis = nullptr;

        if (msg == WM_CREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (ColorPanel*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        }
        else
        {
            pThis = (ColorPanel*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }

        if (pThis)
        {
            return pThis->handleMessage(hwnd, msg, wParam, lParam);
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void applyChanges();
    void updatePreviewColors();
    void pickColor(bool outer);

    DesignerApp* m_app;
    HWND m_hwnd;
    bool m_updating;

    HWND m_editOuterR, m_editOuterG, m_editOuterB, m_editOuterA;
    HWND m_editInnerR, m_editInnerG, m_editInnerB, m_editInnerA;
    HWND m_outerPreview, m_innerPreview;

    HBRUSH m_outerBrush = nullptr;
    HBRUSH m_innerBrush = nullptr;
};

// Include implementation
#include "../DesignerApp.h"
#include "../ShapeManager.h"

inline void ColorPanel::updateFromConfig()
{
    if (!m_hwnd) return;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg) return;

    m_updating = true;

    wchar_t buf[32];

    swprintf_s(buf, L"%.2f", cfg->outerColor[0]);
    SetWindowTextW(m_editOuterR, buf);
    swprintf_s(buf, L"%.2f", cfg->outerColor[1]);
    SetWindowTextW(m_editOuterG, buf);
    swprintf_s(buf, L"%.2f", cfg->outerColor[2]);
    SetWindowTextW(m_editOuterB, buf);
    swprintf_s(buf, L"%.2f", cfg->outerColor[3]);
    SetWindowTextW(m_editOuterA, buf);

    swprintf_s(buf, L"%.2f", cfg->innerColor[0]);
    SetWindowTextW(m_editInnerR, buf);
    swprintf_s(buf, L"%.2f", cfg->innerColor[1]);
    SetWindowTextW(m_editInnerG, buf);
    swprintf_s(buf, L"%.2f", cfg->innerColor[2]);
    SetWindowTextW(m_editInnerB, buf);
    swprintf_s(buf, L"%.2f", cfg->innerColor[3]);
    SetWindowTextW(m_editInnerA, buf);

    updatePreviewColors();

    m_updating = false;
}

inline void ColorPanel::applyChanges()
{
    if (m_updating) return;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg) return;

    wchar_t buf[32];

    auto clamp01 = [](float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); };

    GetWindowTextW(m_editOuterR, buf, 32);
    cfg->outerColor[0] = clamp01(static_cast<float>(_wtof(buf)));
    GetWindowTextW(m_editOuterG, buf, 32);
    cfg->outerColor[1] = clamp01(static_cast<float>(_wtof(buf)));
    GetWindowTextW(m_editOuterB, buf, 32);
    cfg->outerColor[2] = clamp01(static_cast<float>(_wtof(buf)));
    GetWindowTextW(m_editOuterA, buf, 32);
    cfg->outerColor[3] = clamp01(static_cast<float>(_wtof(buf)));

    GetWindowTextW(m_editInnerR, buf, 32);
    cfg->innerColor[0] = clamp01(static_cast<float>(_wtof(buf)));
    GetWindowTextW(m_editInnerG, buf, 32);
    cfg->innerColor[1] = clamp01(static_cast<float>(_wtof(buf)));
    GetWindowTextW(m_editInnerB, buf, 32);
    cfg->innerColor[2] = clamp01(static_cast<float>(_wtof(buf)));
    GetWindowTextW(m_editInnerA, buf, 32);
    cfg->innerColor[3] = clamp01(static_cast<float>(_wtof(buf)));

    updatePreviewColors();
    m_app->onShapeConfigChanged();
}

inline void ColorPanel::updatePreviewColors()
{
    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg) return;

    // Delete old brushes
    if (m_outerBrush) DeleteObject(m_outerBrush);
    if (m_innerBrush) DeleteObject(m_innerBrush);

    // Create new brushes
    m_outerBrush = CreateSolidBrush(RGB(
        static_cast<BYTE>(cfg->outerColor[0] * 255),
        static_cast<BYTE>(cfg->outerColor[1] * 255),
        static_cast<BYTE>(cfg->outerColor[2] * 255)));

    m_innerBrush = CreateSolidBrush(RGB(
        static_cast<BYTE>(cfg->innerColor[0] * 255),
        static_cast<BYTE>(cfg->innerColor[1] * 255),
        static_cast<BYTE>(cfg->innerColor[2] * 255)));

    InvalidateRect(m_outerPreview, nullptr, TRUE);
    InvalidateRect(m_innerPreview, nullptr, TRUE);
}

inline void ColorPanel::pickColor(bool outer)
{
    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg) return;

    auto& color = outer ? cfg->outerColor : cfg->innerColor;

    static COLORREF customColors[16] = { 0 };

    CHOOSECOLORW cc = {};
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = m_hwnd;
    cc.rgbResult = RGB(
        static_cast<BYTE>(color[0] * 255),
        static_cast<BYTE>(color[1] * 255),
        static_cast<BYTE>(color[2] * 255));
    cc.lpCustColors = customColors;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColorW(&cc))
    {
        color[0] = GetRValue(cc.rgbResult) / 255.0f;
        color[1] = GetGValue(cc.rgbResult) / 255.0f;
        color[2] = GetBValue(cc.rgbResult) / 255.0f;

        updateFromConfig();
        m_app->onShapeConfigChanged();
    }
}

inline LRESULT ColorPanel::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        if (HIWORD(wParam) == EN_CHANGE)
        {
            applyChanges();
        }
        else if (HIWORD(wParam) == BN_CLICKED)
        {
            int id = LOWORD(wParam);
            if (id == ID_BTN_OUTER_PICK)
            {
                pickColor(true);
            }
            else if (id == ID_BTN_INNER_PICK)
            {
                pickColor(false);
            }
        }
        return 0;

    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wParam;
        HWND ctrl = (HWND)lParam;
        if (ctrl == m_outerPreview && m_outerBrush)
        {
            return (LRESULT)m_outerBrush;
        }
        else if (ctrl == m_innerPreview && m_innerBrush)
        {
            return (LRESULT)m_innerBrush;
        }
    }
    break;

    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        return 0;

    case WM_DESTROY:
        if (m_outerBrush) { DeleteObject(m_outerBrush); m_outerBrush = nullptr; }
        if (m_innerBrush) { DeleteObject(m_innerBrush); m_innerBrush = nullptr; }
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
