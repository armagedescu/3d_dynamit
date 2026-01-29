#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <string>

class DesignerApp;

// Control IDs
#define ID_EDIT_FORMULA       2001
#define ID_EDIT_DOMAIN_START  2002
#define ID_EDIT_DOMAIN_END    2003
#define ID_SPIN_SECTORS       2004
#define ID_EDIT_SECTORS       2005
#define ID_SPIN_SLICES        2006
#define ID_EDIT_SLICES        2007
#define ID_CHECK_SMOOTH       2008
#define ID_CHECK_TURBO        2009
#define ID_CHECK_DOUBLE_COAT  2010
#define ID_CHECK_REVERSED     2011

class BuilderPanel
{
public:
    BuilderPanel(DesignerApp* app) : m_app(app), m_hwnd(nullptr), m_updating(false) {}
    ~BuilderPanel() { if (m_hwnd) DestroyWindow(m_hwnd); }

    HWND Create(HWND parent)
    {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
        wc.lpszClassName = L"BuilderPanelClass";
        RegisterClassExW(&wc);

        m_hwnd = CreateWindowExW(
            WS_EX_TOOLWINDOW,
            L"BuilderPanelClass",
            L"Builder Settings",
            WS_POPUP | WS_CAPTION | WS_VISIBLE,
            0, 0, 280, 280,
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
        int labelW = 80;
        int editW = 180;
        int rowH = 24;

        auto createLabel = [&](const wchar_t* text, int yPos) {
            HWND h = CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE,
                5, yPos + 3, labelW, 18, m_hwnd, nullptr, GetModuleHandle(nullptr), nullptr);
            SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
        };

        auto createEdit = [&](int id, int yPos, int width = 0) -> HWND {
            HWND h = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                labelW + 10, yPos, width > 0 ? width : editW, 20, m_hwnd, (HMENU)(INT_PTR)id,
                GetModuleHandle(nullptr), nullptr);
            SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
            return h;
        };

        auto createCheck = [&](const wchar_t* text, int id, int yPos) -> HWND {
            HWND h = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                5, yPos, 130, 20, m_hwnd, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
            SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
            return h;
        };

        // Formula
        createLabel(L"Formula:", y);
        m_editFormula = createEdit(ID_EDIT_FORMULA, y);
        y += rowH;

        // Domain Start
        createLabel(L"Domain Start:", y);
        m_editDomainStart = createEdit(ID_EDIT_DOMAIN_START, y, 80);
        y += rowH;

        // Domain End
        createLabel(L"Domain End:", y);
        m_editDomainEnd = createEdit(ID_EDIT_DOMAIN_END, y, 80);
        y += rowH;

        // Sectors
        createLabel(L"Sectors:", y);
        m_editSectors = createEdit(ID_EDIT_SECTORS, y, 60);
        m_spinSectors = CreateWindowW(UPDOWN_CLASSW, nullptr,
            WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
            0, 0, 0, 0, m_hwnd, (HMENU)ID_SPIN_SECTORS, GetModuleHandle(nullptr), nullptr);
        SendMessage(m_spinSectors, UDM_SETBUDDY, (WPARAM)m_editSectors, 0);
        SendMessage(m_spinSectors, UDM_SETRANGE32, 3, 128);
        y += rowH;

        // Slices
        createLabel(L"Slices:", y);
        m_editSlices = createEdit(ID_EDIT_SLICES, y, 60);
        m_spinSlices = CreateWindowW(UPDOWN_CLASSW, nullptr,
            WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
            0, 0, 0, 0, m_hwnd, (HMENU)ID_SPIN_SLICES, GetModuleHandle(nullptr), nullptr);
        SendMessage(m_spinSlices, UDM_SETBUDDY, (WPARAM)m_editSlices, 0);
        SendMessage(m_spinSlices, UDM_SETRANGE32, 1, 64);
        y += rowH + 5;

        // Checkboxes in two columns
        m_checkSmooth = createCheck(L"Smooth", ID_CHECK_SMOOTH, y);
        HWND chkTurbo = CreateWindowW(L"BUTTON", L"Turbo", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            140, y, 120, 20, m_hwnd, (HMENU)ID_CHECK_TURBO, GetModuleHandle(nullptr), nullptr);
        SendMessage(chkTurbo, WM_SETFONT, (WPARAM)hFont, TRUE);
        m_checkTurbo = chkTurbo;
        y += rowH;

        m_checkDoubleCoat = createCheck(L"Double Coated", ID_CHECK_DOUBLE_COAT, y);
        HWND chkRev = CreateWindowW(L"BUTTON", L"Reversed", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            140, y, 120, 20, m_hwnd, (HMENU)ID_CHECK_REVERSED, GetModuleHandle(nullptr), nullptr);
        SendMessage(chkRev, WM_SETFONT, (WPARAM)hFont, TRUE);
        m_checkReversed = chkRev;
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        BuilderPanel* pThis = nullptr;

        if (msg == WM_CREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (BuilderPanel*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        }
        else
        {
            pThis = (BuilderPanel*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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

    HWND m_editFormula;
    HWND m_editDomainStart;
    HWND m_editDomainEnd;
    HWND m_editSectors;
    HWND m_spinSectors;
    HWND m_editSlices;
    HWND m_spinSlices;
    HWND m_checkSmooth;
    HWND m_checkTurbo;
    HWND m_checkDoubleCoat;
    HWND m_checkReversed;
};

// Include implementation
#include "../DesignerApp.h"
#include "../ShapeManager.h"
#include <sstream>
#include <iomanip>

inline void BuilderPanel::updateFromConfig()
{
    if (!m_hwnd) return;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg) return;

    m_updating = true;

    SetWindowTextW(m_editFormula, cfg->formula.c_str());

    wchar_t buf[64];
    swprintf_s(buf, L"%.4f", cfg->domainStart);
    SetWindowTextW(m_editDomainStart, buf);

    swprintf_s(buf, L"%.4f", cfg->domainEnd);
    SetWindowTextW(m_editDomainEnd, buf);

    SendMessage(m_spinSectors, UDM_SETPOS32, 0, cfg->sectors);
    SendMessage(m_spinSlices, UDM_SETPOS32, 0, cfg->slices);

    SendMessage(m_checkSmooth, BM_SETCHECK, cfg->smooth ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(m_checkTurbo, BM_SETCHECK, cfg->turbo ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(m_checkDoubleCoat, BM_SETCHECK, cfg->doubleCoated ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(m_checkReversed, BM_SETCHECK, cfg->reversed ? BST_CHECKED : BST_UNCHECKED, 0);

    m_updating = false;
}

inline void BuilderPanel::applyChanges()
{
    if (m_updating) return;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg) return;

    // Formula
    wchar_t buf[256];
    GetWindowTextW(m_editFormula, buf, 256);
    cfg->formula = buf;

    // Domain
    GetWindowTextW(m_editDomainStart, buf, 64);
    cfg->domainStart = static_cast<float>(_wtof(buf));

    GetWindowTextW(m_editDomainEnd, buf, 64);
    cfg->domainEnd = static_cast<float>(_wtof(buf));

    // Sectors/Slices
    cfg->sectors = static_cast<int>(SendMessage(m_spinSectors, UDM_GETPOS32, 0, 0));
    cfg->slices = static_cast<int>(SendMessage(m_spinSlices, UDM_GETPOS32, 0, 0));

    // Checkboxes
    cfg->smooth = SendMessage(m_checkSmooth, BM_GETCHECK, 0, 0) == BST_CHECKED;
    cfg->turbo = SendMessage(m_checkTurbo, BM_GETCHECK, 0, 0) == BST_CHECKED;
    cfg->doubleCoated = SendMessage(m_checkDoubleCoat, BM_GETCHECK, 0, 0) == BST_CHECKED;
    cfg->reversed = SendMessage(m_checkReversed, BM_GETCHECK, 0, 0) == BST_CHECKED;

    m_app->onShapeConfigChanged();
}

inline LRESULT BuilderPanel::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        if (HIWORD(wParam) == EN_CHANGE || HIWORD(wParam) == BN_CLICKED)
        {
            applyChanges();
        }
        return 0;

    case WM_NOTIFY:
    {
        NMHDR* pnmh = (NMHDR*)lParam;
        if (pnmh->code == UDN_DELTAPOS)
        {
            // Spin control changed - will trigger EN_CHANGE
        }
    }
    return 0;

    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
