#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <string>

#include <atlbase.h>
#include <atlwin.h>

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
#define ID_BTN_EDIT_FORMULA   2012

class BuilderPanel : public CWindowImpl<BuilderPanel>
{
public:
    DECLARE_WND_CLASS(L"BuilderPanelClass")

    BuilderPanel(DesignerApp* app) : m_app(app), m_updating(false) {}

    HWND Create(HWND parent)
    {
        RECT rc = { 0, 0, 280, 310 };
        CWindowImpl::Create(parent, rc, L"Builder Settings",
            WS_POPUP | WS_CAPTION | WS_VISIBLE, WS_EX_TOOLWINDOW);

        if (m_hWnd)
        {
            createControls();
        }

        return m_hWnd;
    }

    void updateFromConfig();

    BEGIN_MSG_MAP(BuilderPanel)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        COMMAND_HANDLER(ID_EDIT_FORMULA, EN_KILLFOCUS, OnFormulaKillFocus)
        COMMAND_HANDLER(ID_EDIT_DOMAIN_START, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_EDIT_DOMAIN_END, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_EDIT_SECTORS, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_EDIT_SLICES, EN_CHANGE, OnEditChange)
        COMMAND_ID_HANDLER(ID_CHECK_SMOOTH, OnCheckClick)
        COMMAND_ID_HANDLER(ID_CHECK_TURBO, OnCheckClick)
        COMMAND_ID_HANDLER(ID_CHECK_DOUBLE_COAT, OnCheckClick)
        COMMAND_ID_HANDLER(ID_CHECK_REVERSED, OnCheckClick)
        COMMAND_ID_HANDLER(ID_BTN_EDIT_FORMULA, OnEditFormulaClick)
        MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
    END_MSG_MAP()

private:
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        ShowWindow(SW_HIDE);
        return 0;
    }

    LRESULT OnFormulaKillFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        applyChanges();
        return 0;
    }

    LRESULT OnEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        applyChanges();
        return 0;
    }

    LRESULT OnCheckClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        if (wNotifyCode == BN_CLICKED)
            applyChanges();
        return 0;
    }

    LRESULT OnEditFormulaClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);  // Implemented after includes

    LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        NMHDR* pnmh = (NMHDR*)lParam;
        if (pnmh->code == UDN_DELTAPOS)
        {
            // Spin control changed - EN_CHANGE will be triggered
        }
        bHandled = FALSE;
        return 0;
    }

    void createControls()
    {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        int y = 5;
        int labelW = 80;
        int editW = 180;
        int rowH = 24;

        auto createLabel = [&](const wchar_t* text, int yPos) {
            HWND h = CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE,
                5, yPos + 3, labelW, 18, m_hWnd, nullptr, GetModuleHandle(nullptr), nullptr);
            ::SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
        };

        auto createEdit = [&](int id, int yPos, int width = 0) -> HWND {
            HWND h = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                labelW + 10, yPos, width > 0 ? width : editW, 20, m_hWnd, (HMENU)(INT_PTR)id,
                GetModuleHandle(nullptr), nullptr);
            ::SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
            return h;
        };

        auto createCheck = [&](const wchar_t* text, int id, int yPos) -> HWND {
            HWND h = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                5, yPos, 130, 20, m_hWnd, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
            ::SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
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
            0, 0, 0, 0, m_hWnd, (HMENU)ID_SPIN_SECTORS, GetModuleHandle(nullptr), nullptr);
        ::SendMessage(m_spinSectors, UDM_SETBUDDY, (WPARAM)m_editSectors, 0);
        ::SendMessage(m_spinSectors, UDM_SETRANGE32, 3, 128);
        y += rowH;

        // Slices
        createLabel(L"Slices:", y);
        m_editSlices = createEdit(ID_EDIT_SLICES, y, 60);
        m_spinSlices = CreateWindowW(UPDOWN_CLASSW, nullptr,
            WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
            0, 0, 0, 0, m_hWnd, (HMENU)ID_SPIN_SLICES, GetModuleHandle(nullptr), nullptr);
        ::SendMessage(m_spinSlices, UDM_SETBUDDY, (WPARAM)m_editSlices, 0);
        ::SendMessage(m_spinSlices, UDM_SETRANGE32, 1, 64);
        y += rowH + 5;

        // Checkboxes in two columns
        m_checkSmooth = createCheck(L"Smooth", ID_CHECK_SMOOTH, y);
        HWND chkTurbo = CreateWindowW(L"BUTTON", L"Turbo", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            140, y, 120, 20, m_hWnd, (HMENU)ID_CHECK_TURBO, GetModuleHandle(nullptr), nullptr);
        ::SendMessage(chkTurbo, WM_SETFONT, (WPARAM)hFont, TRUE);
        m_checkTurbo = chkTurbo;
        y += rowH;

        m_checkDoubleCoat = createCheck(L"Double Coated", ID_CHECK_DOUBLE_COAT, y);
        HWND chkRev = CreateWindowW(L"BUTTON", L"Reversed", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            140, y, 120, 20, m_hWnd, (HMENU)ID_CHECK_REVERSED, GetModuleHandle(nullptr), nullptr);
        ::SendMessage(chkRev, WM_SETFONT, (WPARAM)hFont, TRUE);
        m_checkReversed = chkRev;
        y += rowH + 10;

        // Edit Formula button
        HWND btnEditFormula = CreateWindowW(L"BUTTON", L"Edit Formula...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            5, y, 260, 28, m_hWnd, (HMENU)ID_BTN_EDIT_FORMULA, GetModuleHandle(nullptr), nullptr);
        ::SendMessage(btnEditFormula, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    void applyChanges();

    DesignerApp* m_app;
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
#include "FormulaEditorDialog.h"
#include <sstream>
#include <iomanip>

inline void BuilderPanel::updateFromConfig()
{
    if (!m_hWnd) return;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg) return;

    m_updating = true;

    ::SetWindowTextW(m_editFormula, cfg->formula.c_str());

    wchar_t buf[64];
    swprintf_s(buf, L"%.4f", cfg->domainStart);
    ::SetWindowTextW(m_editDomainStart, buf);
    swprintf_s(buf, L"%.4f", cfg->domainEnd);
    ::SetWindowTextW(m_editDomainEnd, buf);

    ::SendMessage(m_spinSectors, UDM_SETPOS32, 0, cfg->sectors);
    ::SendMessage(m_spinSlices, UDM_SETPOS32, 0, cfg->slices);

    ::SendMessage(m_checkSmooth, BM_SETCHECK, cfg->smooth ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(m_checkTurbo, BM_SETCHECK, cfg->turbo ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(m_checkDoubleCoat, BM_SETCHECK, cfg->doubleCoated ? BST_CHECKED : BST_UNCHECKED, 0);
    ::SendMessage(m_checkReversed, BM_SETCHECK, cfg->reversed ? BST_CHECKED : BST_UNCHECKED, 0);

    m_updating = false;
}

inline void BuilderPanel::applyChanges()
{
    if (m_updating) return;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg) return;

    // Formula
    wchar_t buf[256];
    ::GetWindowTextW(m_editFormula, buf, 256);
    cfg->formula = buf;

    // Domain
    ::GetWindowTextW(m_editDomainStart, buf, 64);
    cfg->domainStart = static_cast<float>(_wtof(buf));

    ::GetWindowTextW(m_editDomainEnd, buf, 64);
    cfg->domainEnd = static_cast<float>(_wtof(buf));

    // Sectors/Slices
    cfg->sectors = static_cast<int>(::SendMessage(m_spinSectors, UDM_GETPOS32, 0, 0));
    cfg->slices = static_cast<int>(::SendMessage(m_spinSlices, UDM_GETPOS32, 0, 0));

    // Checkboxes
    cfg->smooth = ::SendMessage(m_checkSmooth, BM_GETCHECK, 0, 0) == BST_CHECKED;
    cfg->turbo = ::SendMessage(m_checkTurbo, BM_GETCHECK, 0, 0) == BST_CHECKED;
    cfg->doubleCoated = ::SendMessage(m_checkDoubleCoat, BM_GETCHECK, 0, 0) == BST_CHECKED;
    cfg->reversed = ::SendMessage(m_checkReversed, BM_GETCHECK, 0, 0) == BST_CHECKED;

    m_app->onShapeConfigChanged();
}

inline LRESULT BuilderPanel::OnEditFormulaClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (wNotifyCode != BN_CLICKED)
        return 0;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg)
        return 0;

    // Get current segments (or create from legacy single formula)
    std::vector<FormulaSegment> segments = cfg->getEffectiveSegments();

    // Show formula editor dialog
    FormulaEditorDialog dlg;
    if (dlg.Show(m_hWnd, segments))
    {
        // Apply the edited segments
        cfg->setSegments(segments);

        // Update UI to reflect first segment
        updateFromConfig();

        // Rebuild shape
        m_app->onShapeConfigChanged();
    }

    return 0;
}
