#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <string>
#include <vector>

#include <atlbase.h>
#include <atlwin.h>

#pragma comment(lib, "comdlg32.lib")

class DesignerApp;

// Control IDs for PropertiesPanel (unique prefix ID_PP_)
// Shapes section
#define ID_PP_LIST_SHAPES        6050
#define ID_PP_BTN_NEW_CONE       6051
#define ID_PP_BTN_NEW_CYLINDER   6052
#define ID_PP_BTN_DELETE         6053
#define ID_PP_BTN_DUPLICATE      6054
#define ID_PP_BTN_MOVE_UP        6055
#define ID_PP_BTN_MOVE_DOWN      6056
#define ID_PP_BTN_WELD           6057
#define ID_PP_CHK_VISIBLE        6058
#define ID_PP_EDIT_NAME          6059

// Builder section
#define ID_PP_EDIT_FORMULA       6001
#define ID_PP_EDIT_DOMAIN_START  6002
#define ID_PP_EDIT_DOMAIN_END    6003
#define ID_PP_SPIN_SECTORS       6004
#define ID_PP_EDIT_SECTORS       6005
#define ID_PP_SPIN_SLICES        6006
#define ID_PP_EDIT_SLICES        6007
#define ID_PP_CHECK_SMOOTH       6008
#define ID_PP_CHECK_TURBO        6009
#define ID_PP_CHECK_DOUBLE_COAT  6010
#define ID_PP_CHECK_REVERSED     6011
#define ID_PP_BTN_EDIT_FORMULA   6012

// Transform section
#define ID_PP_EDIT_POS_X         6020
#define ID_PP_EDIT_POS_Y         6021
#define ID_PP_EDIT_POS_Z         6022
#define ID_PP_EDIT_ROT_X         6023
#define ID_PP_EDIT_ROT_Y         6024
#define ID_PP_EDIT_ROT_Z         6025
#define ID_PP_EDIT_SCALE_X       6026
#define ID_PP_EDIT_SCALE_Y       6027
#define ID_PP_EDIT_SCALE_Z       6028

// Color section
#define ID_PP_EDIT_OUTER_R       6030
#define ID_PP_EDIT_OUTER_G       6031
#define ID_PP_EDIT_OUTER_B       6032
#define ID_PP_EDIT_OUTER_A       6033
#define ID_PP_EDIT_INNER_R       6034
#define ID_PP_EDIT_INNER_G       6035
#define ID_PP_EDIT_INNER_B       6036
#define ID_PP_EDIT_INNER_A       6037
#define ID_PP_BTN_OUTER_PICK     6038
#define ID_PP_BTN_INNER_PICK     6039
#define ID_PP_STATIC_OUTER_PREVIEW 6040
#define ID_PP_STATIC_INNER_PREVIEW 6041

// Section header IDs
#define ID_PP_HDR_SHAPES         6100
#define ID_PP_HDR_BUILDER        6101
#define ID_PP_HDR_TRANSFORM      6102
#define ID_PP_HDR_COLORS         6103

// Section indices
enum Section { SEC_SHAPES = 0, SEC_BUILDER, SEC_TRANSFORM, SEC_COLORS, SEC_COUNT };

class PropertiesPanel : public CWindowImpl<PropertiesPanel>
{
public:
    DECLARE_WND_CLASS(L"PropertiesPanelClass")

    PropertiesPanel(DesignerApp* app) : m_app(app), m_updating(false),
        m_outerBrush(nullptr), m_innerBrush(nullptr)
    {
        for (int i = 0; i < SEC_COUNT; ++i)
            m_collapsed[i] = false;
    }

    HWND Create(HWND parent)
    {
        // Initial size - will be adjusted after createControls()
        RECT rc = { 0, 0, 300, 600 };
        CWindowImpl::Create(parent, rc, L"Properties",
            WS_POPUP | WS_CAPTION | WS_VISIBLE, WS_EX_TOOLWINDOW);

        if (m_hWnd)
        {
            createControls();
            // Resize window to fit calculated content
            RECT windowRect;
            ::GetWindowRect(m_hWnd, &windowRect);
            RECT clientRect;
            ::GetClientRect(m_hWnd, &clientRect);
            int borderW = (windowRect.right - windowRect.left) - clientRect.right;
            int borderH = (windowRect.bottom - windowRect.top) - clientRect.bottom;
            ::SetWindowPos(m_hWnd, nullptr, 0, 0, m_panelWidth + borderW, 600, SWP_NOMOVE | SWP_NOZORDER);
        }

        return m_hWnd;
    }

    void updateFromConfig();
    void refreshShapesList();
    void updateShapeSelection();

    BEGIN_MSG_MAP(PropertiesPanel)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
        MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
        // Section header click handlers (STN_CLICKED from SS_NOTIFY statics)
        COMMAND_ID_HANDLER(ID_PP_HDR_SHAPES, OnSectionHeaderClick)
        COMMAND_ID_HANDLER(ID_PP_HDR_BUILDER, OnSectionHeaderClick)
        COMMAND_ID_HANDLER(ID_PP_HDR_TRANSFORM, OnSectionHeaderClick)
        COMMAND_ID_HANDLER(ID_PP_HDR_COLORS, OnSectionHeaderClick)
        // Shapes handlers
        COMMAND_HANDLER(ID_PP_LIST_SHAPES, LBN_SELCHANGE, OnShapeSelChange)
        COMMAND_ID_HANDLER(ID_PP_BTN_NEW_CONE, OnNewCone)
        COMMAND_ID_HANDLER(ID_PP_BTN_NEW_CYLINDER, OnNewCylinder)
        COMMAND_ID_HANDLER(ID_PP_BTN_DELETE, OnDelete)
        COMMAND_ID_HANDLER(ID_PP_BTN_DUPLICATE, OnDuplicate)
        COMMAND_ID_HANDLER(ID_PP_BTN_MOVE_UP, OnMoveUp)
        COMMAND_ID_HANDLER(ID_PP_BTN_MOVE_DOWN, OnMoveDown)
        COMMAND_ID_HANDLER(ID_PP_BTN_WELD, OnWeld)
        COMMAND_ID_HANDLER(ID_PP_CHK_VISIBLE, OnVisibleClick)
        COMMAND_HANDLER(ID_PP_EDIT_NAME, EN_KILLFOCUS, OnNameChange)
        // Builder handlers
        COMMAND_HANDLER(ID_PP_EDIT_FORMULA, EN_KILLFOCUS, OnFormulaKillFocus)
        COMMAND_HANDLER(ID_PP_EDIT_DOMAIN_START, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_PP_EDIT_DOMAIN_END, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_PP_EDIT_SECTORS, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_PP_EDIT_SLICES, EN_CHANGE, OnEditChange)
        COMMAND_ID_HANDLER(ID_PP_CHECK_SMOOTH, OnCheckClick)
        COMMAND_ID_HANDLER(ID_PP_CHECK_TURBO, OnCheckClick)
        COMMAND_ID_HANDLER(ID_PP_CHECK_DOUBLE_COAT, OnCheckClick)
        COMMAND_ID_HANDLER(ID_PP_CHECK_REVERSED, OnCheckClick)
        COMMAND_ID_HANDLER(ID_PP_BTN_EDIT_FORMULA, OnEditFormulaClick)
        // Transform handlers
        COMMAND_HANDLER(ID_PP_EDIT_POS_X, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_PP_EDIT_POS_Y, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_PP_EDIT_POS_Z, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_PP_EDIT_ROT_X, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_PP_EDIT_ROT_Y, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_PP_EDIT_ROT_Z, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_PP_EDIT_SCALE_X, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_PP_EDIT_SCALE_Y, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_PP_EDIT_SCALE_Z, EN_CHANGE, OnEditChange)
        // Color handlers
        COMMAND_HANDLER(ID_PP_EDIT_OUTER_R, EN_CHANGE, OnColorChange)
        COMMAND_HANDLER(ID_PP_EDIT_OUTER_G, EN_CHANGE, OnColorChange)
        COMMAND_HANDLER(ID_PP_EDIT_OUTER_B, EN_CHANGE, OnColorChange)
        COMMAND_HANDLER(ID_PP_EDIT_OUTER_A, EN_CHANGE, OnColorChange)
        COMMAND_HANDLER(ID_PP_EDIT_INNER_R, EN_CHANGE, OnColorChange)
        COMMAND_HANDLER(ID_PP_EDIT_INNER_G, EN_CHANGE, OnColorChange)
        COMMAND_HANDLER(ID_PP_EDIT_INNER_B, EN_CHANGE, OnColorChange)
        COMMAND_HANDLER(ID_PP_EDIT_INNER_A, EN_CHANGE, OnColorChange)
        COMMAND_ID_HANDLER(ID_PP_BTN_OUTER_PICK, OnOuterPickClick)
        COMMAND_ID_HANDLER(ID_PP_BTN_INNER_PICK, OnInnerPickClick)
    END_MSG_MAP()

private:
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        ShowWindow(SW_HIDE);
        return 0;
    }

    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if (m_outerBrush) { DeleteObject(m_outerBrush); m_outerBrush = nullptr; }
        if (m_innerBrush) { DeleteObject(m_innerBrush); m_innerBrush = nullptr; }
        if (m_hdrBrush) { DeleteObject(m_hdrBrush); m_hdrBrush = nullptr; }
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        HWND ctrl = (HWND)lParam;
        HDC hdc = (HDC)wParam;

        // Color previews
        if (ctrl == m_outerPreview && m_outerBrush)
            return (LRESULT)m_outerBrush;
        else if (ctrl == m_innerPreview && m_innerBrush)
            return (LRESULT)m_innerBrush;

        // Section headers - custom background
        for (int i = 0; i < SEC_COUNT; ++i)
        {
            if (ctrl == m_sectionHeaders[i])
            {
                SetBkColor(hdc, RGB(220, 220, 220));
                SetTextColor(hdc, RGB(0, 0, 0));
                if (!m_hdrBrush)
                    m_hdrBrush = CreateSolidBrush(RGB(220, 220, 220));
                return (LRESULT)m_hdrBrush;
            }
        }

        bHandled = FALSE;
        return 0;
    }

    LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        NMHDR* pnmh = (NMHDR*)lParam;
        if (pnmh->code == UDN_DELTAPOS)
        {
            // Spin control changed
        }
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnSectionHeaderClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        // Map control ID to section index
        int section = -1;
        switch (wID)
        {
        case ID_PP_HDR_SHAPES:    section = SEC_SHAPES; break;
        case ID_PP_HDR_BUILDER:   section = SEC_BUILDER; break;
        case ID_PP_HDR_TRANSFORM: section = SEC_TRANSFORM; break;
        case ID_PP_HDR_COLORS:    section = SEC_COLORS; break;
        }

        if (section >= 0)
        {
            toggleSection(section);
        }
        return 0;
    }

    // Shapes handlers
    LRESULT OnShapeSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnNewCone(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnNewCylinder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDuplicate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnMoveUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnMoveDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnWeld(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnVisibleClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnNameChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    // Builder handlers
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

    LRESULT OnColorChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        applyColorChanges();
        return 0;
    }

    LRESULT OnEditFormulaClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnOuterPickClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnInnerPickClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    void toggleSection(int section)
    {
        m_collapsed[section] = !m_collapsed[section];
        updateSectionHeaderText(section);
        repositionControls();
    }

    void updateSectionHeaderText(int section)
    {
        const wchar_t* names[] = { L"Shapes", L"Builder", L"Transform", L"Colors" };
        std::wstring text = m_collapsed[section] ? L"\u25B6 " : L"\u25BC ";
        text += names[section];
        ::SetWindowTextW(m_sectionHeaders[section], text.c_str());
    }

    void repositionControls();
    void createControls();
    void applyChanges();
    void applyColorChanges();
    void updatePreviewColors();
    void pickColor(bool outer);

    DesignerApp* m_app;
    bool m_updating;
    int m_panelWidth;
    int m_frameX;
    int m_frameW;
    int m_contentX;
    int m_labelW;
    int m_rowH;

    // Section state
    bool m_collapsed[SEC_COUNT];
    HWND m_sectionHeaders[SEC_COUNT];
    std::vector<HWND> m_sectionControls[SEC_COUNT];
    std::vector<int> m_sectionControlOrigY[SEC_COUNT];  // Original Y positions
    int m_sectionContentHeight[SEC_COUNT];
    int m_sectionOrigStartY[SEC_COUNT];  // Original start Y for each section
    HBRUSH m_hdrBrush = nullptr;

    // Shapes controls
    HWND m_listShapes;
    HWND m_editName;
    HWND m_chkVisible;

    // Builder controls
    HWND m_editFormula;
    HWND m_btnEditFormula;
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

    // Transform controls
    HWND m_editPosX, m_editPosY, m_editPosZ;
    HWND m_editRotX, m_editRotY, m_editRotZ;
    HWND m_editScaleX, m_editScaleY, m_editScaleZ;

    // Color controls
    HWND m_editOuterR, m_editOuterG, m_editOuterB, m_editOuterA;
    HWND m_editInnerR, m_editInnerG, m_editInnerB, m_editInnerA;
    HWND m_outerPreview, m_innerPreview;
    HBRUSH m_outerBrush;
    HBRUSH m_innerBrush;
};

// Include implementation
#include "../DesignerApp.h"
#include "../ShapeManager.h"
#include "FormulaEditorDialog.h"
#include <sstream>
#include <iomanip>

inline void PropertiesPanel::createControls()
{
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    HFONT hBoldFont = CreateFontW(-12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"MS Shell Dlg");

    // Layout constants
    int margin = 8;
    int btnGap = 3;
    int btnConeW = 48, btnCylW = 40, btnDupW = 35, btnDelW = 35;
    int btnUpW = 24, btnDownW = 24, btnWeldW = 42;
    int chkSmoothW = 60, chkTurboW = 55, chkDoubleCoatW = 65, chkReversedW = 70;

    // Derive panel width from the widest fixed-content row
    int buttonsRowW = btnConeW + btnCylW + btnDupW + btnDelW + btnUpW + btnDownW + btnWeldW + 6 * btnGap;
    int checkboxRowW = chkSmoothW + chkTurboW + chkDoubleCoatW + chkReversedW + 3 * btnGap;
    int minContentW = buttonsRowW > checkboxRowW ? buttonsRowW : checkboxRowW;

    m_labelW = 55;
    m_rowH = 24;
    m_frameX = margin;
    m_contentX = m_frameX + margin;
    m_panelWidth = minContentW + 4 * margin;
    m_frameW = m_panelWidth - 2 * margin;

    int y = 3;
    int hdrH = 22;

    auto addToSection = [&](int sec, HWND h, int yPos) {
        m_sectionControls[sec].push_back(h);
        m_sectionControlOrigY[sec].push_back(yPos);
    };

    auto createLabel = [&](int sec, const wchar_t* text, int xPos, int yPos, int width = 0) -> HWND {
        HWND h = CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE,
            xPos, yPos + 3, width > 0 ? width : m_labelW, 18, m_hWnd, nullptr, GetModuleHandle(nullptr), nullptr);
        ::SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
        addToSection(sec, h, yPos + 3);
        return h;
    };

    auto createEdit = [&](int sec, int id, int xPos, int yPos, int width) -> HWND {
        HWND h = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            xPos, yPos, width, 20, m_hWnd, (HMENU)(INT_PTR)id,
            GetModuleHandle(nullptr), nullptr);
        ::SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
        addToSection(sec, h, yPos);
        return h;
    };

    auto createButton = [&](int sec, const wchar_t* text, int id, int xPos, int yPos, int width, int height = 24) -> HWND {
        HWND h = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            xPos, yPos, width, height, m_hWnd, (HMENU)(INT_PTR)id,
            GetModuleHandle(nullptr), nullptr);
        ::SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
        addToSection(sec, h, yPos);
        return h;
    };

    auto createCheck = [&](int sec, const wchar_t* text, int id, int xPos, int yPos, int width = 100) -> HWND {
        HWND h = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            xPos, yPos, width, 20, m_hWnd, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
        ::SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
        addToSection(sec, h, yPos);
        return h;
    };

    auto createSectionHeader = [&](int sec, const wchar_t* text, int id) {
        std::wstring headerText = L"\u25BC ";
        headerText += text;
        HWND h = CreateWindowW(L"STATIC", headerText.c_str(),
            WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_CENTERIMAGE,
            m_frameX, y, m_frameW, hdrH, m_hWnd, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
        ::SendMessage(h, WM_SETFONT, (WPARAM)hBoldFont, TRUE);
        m_sectionHeaders[sec] = h;
        y += hdrH;
        m_sectionOrigStartY[sec] = y;  // Store where content starts
    };

    // ===== SHAPES SECTION =====
    createSectionHeader(SEC_SHAPES, L"Shapes", ID_PP_HDR_SHAPES);
    int shapesStartY = y;

    // Calculate available content width
    int contentW = m_frameW - 2 * margin;
    int visibleChkW = 55;

    // Name and visible row
    createLabel(SEC_SHAPES, L"Name:", m_contentX, y);
    int nameEditW = contentW - m_labelW - visibleChkW - btnGap;
    m_editName = createEdit(SEC_SHAPES, ID_PP_EDIT_NAME, m_contentX + m_labelW, y, nameEditW);
    m_chkVisible = createCheck(SEC_SHAPES, L"Visible", ID_PP_CHK_VISIBLE, m_contentX + m_labelW + nameEditW + btnGap, y, visibleChkW);
    y += m_rowH;

    // List box - full content width
    m_listShapes = CreateWindowW(L"LISTBOX", nullptr,
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
        m_contentX, y, contentW, 80, m_hWnd, (HMENU)(INT_PTR)ID_PP_LIST_SHAPES,
        GetModuleHandle(nullptr), nullptr);
    ::SendMessage(m_listShapes, WM_SETFONT, (WPARAM)hFont, TRUE);
    addToSection(SEC_SHAPES, m_listShapes, y);
    y += 85;

    // Buttons row - positions calculated from button widths
    int bx = m_contentX;
    createButton(SEC_SHAPES, L"+Cone", ID_PP_BTN_NEW_CONE, bx, y, btnConeW); bx += btnConeW + btnGap;
    createButton(SEC_SHAPES, L"+Cyl", ID_PP_BTN_NEW_CYLINDER, bx, y, btnCylW); bx += btnCylW + btnGap;
    createButton(SEC_SHAPES, L"Dup", ID_PP_BTN_DUPLICATE, bx, y, btnDupW); bx += btnDupW + btnGap;
    createButton(SEC_SHAPES, L"Del", ID_PP_BTN_DELETE, bx, y, btnDelW); bx += btnDelW + btnGap;
    createButton(SEC_SHAPES, L"\u25B2", ID_PP_BTN_MOVE_UP, bx, y, btnUpW); bx += btnUpW + btnGap;
    createButton(SEC_SHAPES, L"\u25BC", ID_PP_BTN_MOVE_DOWN, bx, y, btnDownW); bx += btnDownW + btnGap;
    createButton(SEC_SHAPES, L"Weld", ID_PP_BTN_WELD, bx, y, btnWeldW);
    y += m_rowH + 3;

    m_sectionContentHeight[SEC_SHAPES] = y - shapesStartY;
    y += 2;

    // ===== BUILDER SECTION =====
    createSectionHeader(SEC_BUILDER, L"Builder", ID_PP_HDR_BUILDER);
    int builderStartY = y;

    // Formula
    createLabel(SEC_BUILDER, L"Formula:", m_contentX, y);
    int btnFormulaW = 24;
    int formulaEditW = m_frameW - 10 - m_labelW - btnFormulaW - btnGap;
    m_editFormula = createEdit(SEC_BUILDER, ID_PP_EDIT_FORMULA, m_contentX + m_labelW, y, formulaEditW);
    m_btnEditFormula = CreateWindowW(L"BUTTON", L"...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        m_contentX + m_labelW + formulaEditW + btnGap, y, btnFormulaW, 20, m_hWnd, (HMENU)ID_PP_BTN_EDIT_FORMULA,
        GetModuleHandle(nullptr), nullptr);
    ::SendMessage(m_btnEditFormula, WM_SETFONT, (WPARAM)hFont, TRUE);
    addToSection(SEC_BUILDER, m_btnEditFormula, y + 1);
    y += m_rowH;

    // Domain
    createLabel(SEC_BUILDER, L"Domain:", m_contentX, y);
    m_editDomainStart = createEdit(SEC_BUILDER, ID_PP_EDIT_DOMAIN_START, m_contentX + m_labelW, y, 80);
    createLabel(SEC_BUILDER, L"to", m_contentX + m_labelW + 85, y, 20);
    m_editDomainEnd = createEdit(SEC_BUILDER, ID_PP_EDIT_DOMAIN_END, m_contentX + m_labelW + 105, y, 80);
    y += m_rowH;

    // Sectors/Slices on same row
    createLabel(SEC_BUILDER, L"Sectors:", m_contentX, y);
    m_editSectors = createEdit(SEC_BUILDER, ID_PP_EDIT_SECTORS, m_contentX + m_labelW, y, 50);
    m_spinSectors = CreateWindowW(UPDOWN_CLASSW, nullptr,
        WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
        0, 0, 0, 0, m_hWnd, (HMENU)ID_PP_SPIN_SECTORS, GetModuleHandle(nullptr), nullptr);
    ::SendMessage(m_spinSectors, UDM_SETBUDDY, (WPARAM)m_editSectors, 0);
    ::SendMessage(m_spinSectors, UDM_SETRANGE32, 3, 128);
    addToSection(SEC_BUILDER, m_spinSectors, y);

    createLabel(SEC_BUILDER, L"Slices:", m_contentX + m_labelW + 60, y, 45);
    m_editSlices = createEdit(SEC_BUILDER, ID_PP_EDIT_SLICES, m_contentX + m_labelW + 110, y, 50);
    m_spinSlices = CreateWindowW(UPDOWN_CLASSW, nullptr,
        WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
        0, 0, 0, 0, m_hWnd, (HMENU)ID_PP_SPIN_SLICES, GetModuleHandle(nullptr), nullptr);
    ::SendMessage(m_spinSlices, UDM_SETBUDDY, (WPARAM)m_editSlices, 0);
    ::SendMessage(m_spinSlices, UDM_SETRANGE32, 1, 64);
    addToSection(SEC_BUILDER, m_spinSlices, y);
    y += m_rowH;

    // Checkboxes - positions calculated from checkbox widths
    int cx = m_contentX;
    m_checkSmooth = createCheck(SEC_BUILDER, L"Smooth", ID_PP_CHECK_SMOOTH, cx, y, chkSmoothW); cx += chkSmoothW + btnGap;
    m_checkTurbo = createCheck(SEC_BUILDER, L"Turbo", ID_PP_CHECK_TURBO, cx, y, chkTurboW); cx += chkTurboW + btnGap;
    m_checkDoubleCoat = createCheck(SEC_BUILDER, L"DblCoat", ID_PP_CHECK_DOUBLE_COAT, cx, y, chkDoubleCoatW); cx += chkDoubleCoatW + btnGap;
    m_checkReversed = createCheck(SEC_BUILDER, L"Reversed", ID_PP_CHECK_REVERSED, cx, y, chkReversedW);
    y += m_rowH + 3;

    m_sectionContentHeight[SEC_BUILDER] = y - builderStartY;
    y += 2;

    // ===== TRANSFORM SECTION =====
    createSectionHeader(SEC_TRANSFORM, L"Transform", ID_PP_HDR_TRANSFORM);
    int transformStartY = y;

    int col1 = m_contentX + m_labelW;
    int col2 = col1 + 60;
    int col3 = col2 + 60;
    int tfEditW = 55;

    // Header X Y Z
    createLabel(SEC_TRANSFORM, L"X", col1 + 18, y, 20);
    createLabel(SEC_TRANSFORM, L"Y", col2 + 18, y, 20);
    createLabel(SEC_TRANSFORM, L"Z", col3 + 18, y, 20);
    y += 18;

    // Position
    createLabel(SEC_TRANSFORM, L"Position:", m_contentX, y);
    m_editPosX = createEdit(SEC_TRANSFORM, ID_PP_EDIT_POS_X, col1, y, tfEditW);
    m_editPosY = createEdit(SEC_TRANSFORM, ID_PP_EDIT_POS_Y, col2, y, tfEditW);
    m_editPosZ = createEdit(SEC_TRANSFORM, ID_PP_EDIT_POS_Z, col3, y, tfEditW);
    y += m_rowH;

    // Rotation
    createLabel(SEC_TRANSFORM, L"Rotation:", m_contentX, y);
    m_editRotX = createEdit(SEC_TRANSFORM, ID_PP_EDIT_ROT_X, col1, y, tfEditW);
    m_editRotY = createEdit(SEC_TRANSFORM, ID_PP_EDIT_ROT_Y, col2, y, tfEditW);
    m_editRotZ = createEdit(SEC_TRANSFORM, ID_PP_EDIT_ROT_Z, col3, y, tfEditW);
    y += m_rowH;

    // Scale
    createLabel(SEC_TRANSFORM, L"Scale:", m_contentX, y);
    m_editScaleX = createEdit(SEC_TRANSFORM, ID_PP_EDIT_SCALE_X, col1, y, tfEditW);
    m_editScaleY = createEdit(SEC_TRANSFORM, ID_PP_EDIT_SCALE_Y, col2, y, tfEditW);
    m_editScaleZ = createEdit(SEC_TRANSFORM, ID_PP_EDIT_SCALE_Z, col3, y, tfEditW);
    y += m_rowH + 3;

    m_sectionContentHeight[SEC_TRANSFORM] = y - transformStartY;
    y += 2;

    // ===== COLORS SECTION =====
    createSectionHeader(SEC_COLORS, L"Colors", ID_PP_HDR_COLORS);
    int colorsStartY = y;

    int clrEditW = 45;
    int clrGap = 3;
    int clrCol1 = m_contentX + m_labelW;

    // Header R G B A
    createLabel(SEC_COLORS, L"R", clrCol1 + 12, y, 20);
    createLabel(SEC_COLORS, L"G", clrCol1 + clrEditW + clrGap + 12, y, 20);
    createLabel(SEC_COLORS, L"B", clrCol1 + 2 * (clrEditW + clrGap) + 12, y, 20);
    createLabel(SEC_COLORS, L"A", clrCol1 + 3 * (clrEditW + clrGap) + 12, y, 20);
    y += 18;

    // Outer color
    createLabel(SEC_COLORS, L"Outer:", m_contentX, y);
    m_editOuterR = createEdit(SEC_COLORS, ID_PP_EDIT_OUTER_R, clrCol1, y, clrEditW);
    m_editOuterG = createEdit(SEC_COLORS, ID_PP_EDIT_OUTER_G, clrCol1 + clrEditW + clrGap, y, clrEditW);
    m_editOuterB = createEdit(SEC_COLORS, ID_PP_EDIT_OUTER_B, clrCol1 + 2 * (clrEditW + clrGap), y, clrEditW);
    m_editOuterA = createEdit(SEC_COLORS, ID_PP_EDIT_OUTER_A, clrCol1 + 3 * (clrEditW + clrGap), y, clrEditW);
    y += m_rowH;

    // Outer preview
    m_outerPreview = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_NOTIFY | WS_BORDER,
        clrCol1, y, 100, 20, m_hWnd, (HMENU)ID_PP_STATIC_OUTER_PREVIEW, GetModuleHandle(nullptr), nullptr);
    addToSection(SEC_COLORS, m_outerPreview, y);
    HWND btnOuterPick = createButton(SEC_COLORS, L"...", ID_PP_BTN_OUTER_PICK, clrCol1 + 105, y, 24, 20);
    y += m_rowH + 2;

    // Inner color
    createLabel(SEC_COLORS, L"Inner:", m_contentX, y);
    m_editInnerR = createEdit(SEC_COLORS, ID_PP_EDIT_INNER_R, clrCol1, y, clrEditW);
    m_editInnerG = createEdit(SEC_COLORS, ID_PP_EDIT_INNER_G, clrCol1 + clrEditW + clrGap, y, clrEditW);
    m_editInnerB = createEdit(SEC_COLORS, ID_PP_EDIT_INNER_B, clrCol1 + 2 * (clrEditW + clrGap), y, clrEditW);
    m_editInnerA = createEdit(SEC_COLORS, ID_PP_EDIT_INNER_A, clrCol1 + 3 * (clrEditW + clrGap), y, clrEditW);
    y += m_rowH;

    // Inner preview
    m_innerPreview = CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_NOTIFY | WS_BORDER,
        clrCol1, y, 100, 20, m_hWnd, (HMENU)ID_PP_STATIC_INNER_PREVIEW, GetModuleHandle(nullptr), nullptr);
    addToSection(SEC_COLORS, m_innerPreview, y);
    HWND btnInnerPick = createButton(SEC_COLORS, L"...", ID_PP_BTN_INNER_PICK, clrCol1 + 105, y, 24, 20);
    y += m_rowH + 3;

    m_sectionContentHeight[SEC_COLORS] = y - colorsStartY;
}

inline void PropertiesPanel::repositionControls()
{
    int y = 3;
    int hdrH = 22;

    for (int sec = 0; sec < SEC_COUNT; ++sec)
    {
        // Move header
        ::SetWindowPos(m_sectionHeaders[sec], nullptr, m_frameX, y, m_frameW, hdrH, SWP_NOZORDER);
        y += hdrH;

        if (m_collapsed[sec])
        {
            // Hide all controls in this section
            for (HWND h : m_sectionControls[sec])
            {
                ::ShowWindow(h, SW_HIDE);
            }
        }
        else
        {
            // Show controls at their positions relative to new section start
            int deltaY = y - m_sectionOrigStartY[sec];

            for (size_t i = 0; i < m_sectionControls[sec].size(); ++i)
            {
                HWND h = m_sectionControls[sec][i];
                int origY = m_sectionControlOrigY[sec][i];
                int newY = origY + deltaY;

                RECT rc;
                ::GetWindowRect(h, &rc);
                ::ScreenToClient(m_hWnd, (LPPOINT)&rc);

                ::SetWindowPos(h, nullptr, rc.left, newY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
                ::ShowWindow(h, SW_SHOW);
            }

            y += m_sectionContentHeight[sec];
        }

        y += 2; // Gap between sections
    }

    // Resize window to fit content
    RECT clientRect;
    ::GetClientRect(m_hWnd, &clientRect);
    if (y + 5 != clientRect.bottom)
    {
        RECT windowRect;
        ::GetWindowRect(m_hWnd, &windowRect);
        int windowHeight = (windowRect.bottom - windowRect.top) - clientRect.bottom + y + 5;
        ::SetWindowPos(m_hWnd, nullptr, 0, 0, m_panelWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER);
    }

    ::InvalidateRect(m_hWnd, nullptr, TRUE);
}

inline void PropertiesPanel::refreshShapesList()
{
    if (!m_hWnd || !m_app || !m_listShapes) return;

    m_updating = true;

    ::SendMessage(m_listShapes, LB_RESETCONTENT, 0, 0);

    ShapeManager& mgr = m_app->getShapeManager();
    int shapeCount = mgr.getShapeCount();

    for (int i = 0; i < shapeCount; ++i)
    {
        const ShapeInstance* shape = mgr.getShape(i);
        if (shape)
        {
            std::wstring item;
            if (shape->config.type == ShapeConfig::Type::Cone)
                item = L"[C] ";
            else
                item = L"[Y] ";

            if (!shape->visible)
                item = L"(" + item.substr(0, 3) + L") ";

            const std::string& name = shape->config.name;
            int wsize = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
            if (wsize > 0)
            {
                std::wstring wname(wsize - 1, 0);
                MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, &wname[0], wsize);
                item += wname;
            }

            ::SendMessageW(m_listShapes, LB_ADDSTRING, 0, (LPARAM)item.c_str());
        }
    }

    int selectedIndex = m_app->getSelectedShapeIndex();
    if (selectedIndex >= 0 && selectedIndex < shapeCount)
    {
        ::SendMessage(m_listShapes, LB_SETCURSEL, selectedIndex, 0);
    }
    else if (shapeCount > 0)
    {
        ::SendMessage(m_listShapes, LB_SETCURSEL, 0, 0);
    }

    // Force repaint
    ::InvalidateRect(m_listShapes, nullptr, TRUE);

    updateShapeSelection();
    m_updating = false;
}

inline void PropertiesPanel::updateShapeSelection()
{
    if (!m_hWnd || !m_app) return;

    bool wasUpdating = m_updating;
    m_updating = true;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (cfg)
    {
        const std::string& name = cfg->name;
        int wsize = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
        if (wsize > 0)
        {
            std::wstring wname(wsize - 1, 0);
            MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, &wname[0], wsize);
            ::SetWindowTextW(m_editName, wname.c_str());
        }

        ShapeInstance* shape = m_app->getShapeManager().getShape(m_app->getSelectedShapeIndex());
        if (shape)
        {
            ::SendMessage(m_chkVisible, BM_SETCHECK, shape->visible ? BST_CHECKED : BST_UNCHECKED, 0);
        }
    }
    else
    {
        ::SetWindowTextW(m_editName, L"");
        ::SendMessage(m_chkVisible, BM_SETCHECK, BST_UNCHECKED, 0);
    }

    m_updating = wasUpdating;
}

inline void PropertiesPanel::updateFromConfig()
{
    if (!m_hWnd) return;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg) return;

    m_updating = true;

    // Update shapes selection
    updateShapeSelection();

    // === Builder section ===
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

    // === Transform section ===
    swprintf_s(buf, L"%.2f", cfg->posX);
    ::SetWindowTextW(m_editPosX, buf);
    swprintf_s(buf, L"%.2f", cfg->posY);
    ::SetWindowTextW(m_editPosY, buf);
    swprintf_s(buf, L"%.2f", cfg->posZ);
    ::SetWindowTextW(m_editPosZ, buf);

    swprintf_s(buf, L"%.1f", cfg->rotX);
    ::SetWindowTextW(m_editRotX, buf);
    swprintf_s(buf, L"%.1f", cfg->rotY);
    ::SetWindowTextW(m_editRotY, buf);
    swprintf_s(buf, L"%.1f", cfg->rotZ);
    ::SetWindowTextW(m_editRotZ, buf);

    swprintf_s(buf, L"%.2f", cfg->scaleX);
    ::SetWindowTextW(m_editScaleX, buf);
    swprintf_s(buf, L"%.2f", cfg->scaleY);
    ::SetWindowTextW(m_editScaleY, buf);
    swprintf_s(buf, L"%.2f", cfg->scaleZ);
    ::SetWindowTextW(m_editScaleZ, buf);

    // === Colors section ===
    swprintf_s(buf, L"%.2f", cfg->outerColor[0]);
    ::SetWindowTextW(m_editOuterR, buf);
    swprintf_s(buf, L"%.2f", cfg->outerColor[1]);
    ::SetWindowTextW(m_editOuterG, buf);
    swprintf_s(buf, L"%.2f", cfg->outerColor[2]);
    ::SetWindowTextW(m_editOuterB, buf);
    swprintf_s(buf, L"%.2f", cfg->outerColor[3]);
    ::SetWindowTextW(m_editOuterA, buf);

    swprintf_s(buf, L"%.2f", cfg->innerColor[0]);
    ::SetWindowTextW(m_editInnerR, buf);
    swprintf_s(buf, L"%.2f", cfg->innerColor[1]);
    ::SetWindowTextW(m_editInnerG, buf);
    swprintf_s(buf, L"%.2f", cfg->innerColor[2]);
    ::SetWindowTextW(m_editInnerB, buf);
    swprintf_s(buf, L"%.2f", cfg->innerColor[3]);
    ::SetWindowTextW(m_editInnerA, buf);

    updatePreviewColors();

    m_updating = false;
}

inline void PropertiesPanel::applyChanges()
{
    if (m_updating) return;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg) return;

    wchar_t buf[256];

    ::GetWindowTextW(m_editFormula, buf, 256);
    cfg->formula = buf;

    ::GetWindowTextW(m_editDomainStart, buf, 64);
    cfg->domainStart = static_cast<float>(_wtof(buf));
    ::GetWindowTextW(m_editDomainEnd, buf, 64);
    cfg->domainEnd = static_cast<float>(_wtof(buf));

    cfg->sectors = static_cast<int>(::SendMessage(m_spinSectors, UDM_GETPOS32, 0, 0));
    cfg->slices = static_cast<int>(::SendMessage(m_spinSlices, UDM_GETPOS32, 0, 0));

    cfg->smooth = ::SendMessage(m_checkSmooth, BM_GETCHECK, 0, 0) == BST_CHECKED;
    cfg->turbo = ::SendMessage(m_checkTurbo, BM_GETCHECK, 0, 0) == BST_CHECKED;
    cfg->doubleCoated = ::SendMessage(m_checkDoubleCoat, BM_GETCHECK, 0, 0) == BST_CHECKED;
    cfg->reversed = ::SendMessage(m_checkReversed, BM_GETCHECK, 0, 0) == BST_CHECKED;

    ::GetWindowTextW(m_editPosX, buf, 32);
    cfg->posX = static_cast<float>(_wtof(buf));
    ::GetWindowTextW(m_editPosY, buf, 32);
    cfg->posY = static_cast<float>(_wtof(buf));
    ::GetWindowTextW(m_editPosZ, buf, 32);
    cfg->posZ = static_cast<float>(_wtof(buf));

    ::GetWindowTextW(m_editRotX, buf, 32);
    cfg->rotX = static_cast<float>(_wtof(buf));
    ::GetWindowTextW(m_editRotY, buf, 32);
    cfg->rotY = static_cast<float>(_wtof(buf));
    ::GetWindowTextW(m_editRotZ, buf, 32);
    cfg->rotZ = static_cast<float>(_wtof(buf));

    ::GetWindowTextW(m_editScaleX, buf, 32);
    cfg->scaleX = static_cast<float>(_wtof(buf));
    if (cfg->scaleX < 0.01f) cfg->scaleX = 0.01f;
    ::GetWindowTextW(m_editScaleY, buf, 32);
    cfg->scaleY = static_cast<float>(_wtof(buf));
    if (cfg->scaleY < 0.01f) cfg->scaleY = 0.01f;
    ::GetWindowTextW(m_editScaleZ, buf, 32);
    cfg->scaleZ = static_cast<float>(_wtof(buf));
    if (cfg->scaleZ < 0.01f) cfg->scaleZ = 0.01f;

    m_app->onShapeConfigChanged();
}

inline void PropertiesPanel::applyColorChanges()
{
    if (m_updating) return;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg) return;

    wchar_t buf[32];
    auto clamp01 = [](float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); };

    ::GetWindowTextW(m_editOuterR, buf, 32);
    cfg->outerColor[0] = clamp01(static_cast<float>(_wtof(buf)));
    ::GetWindowTextW(m_editOuterG, buf, 32);
    cfg->outerColor[1] = clamp01(static_cast<float>(_wtof(buf)));
    ::GetWindowTextW(m_editOuterB, buf, 32);
    cfg->outerColor[2] = clamp01(static_cast<float>(_wtof(buf)));
    ::GetWindowTextW(m_editOuterA, buf, 32);
    cfg->outerColor[3] = clamp01(static_cast<float>(_wtof(buf)));

    ::GetWindowTextW(m_editInnerR, buf, 32);
    cfg->innerColor[0] = clamp01(static_cast<float>(_wtof(buf)));
    ::GetWindowTextW(m_editInnerG, buf, 32);
    cfg->innerColor[1] = clamp01(static_cast<float>(_wtof(buf)));
    ::GetWindowTextW(m_editInnerB, buf, 32);
    cfg->innerColor[2] = clamp01(static_cast<float>(_wtof(buf)));
    ::GetWindowTextW(m_editInnerA, buf, 32);
    cfg->innerColor[3] = clamp01(static_cast<float>(_wtof(buf)));

    updatePreviewColors();
    m_app->onShapeConfigChanged();
}

inline void PropertiesPanel::updatePreviewColors()
{
    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg) return;

    if (m_outerBrush) DeleteObject(m_outerBrush);
    if (m_innerBrush) DeleteObject(m_innerBrush);

    m_outerBrush = CreateSolidBrush(RGB(
        static_cast<BYTE>(cfg->outerColor[0] * 255),
        static_cast<BYTE>(cfg->outerColor[1] * 255),
        static_cast<BYTE>(cfg->outerColor[2] * 255)));

    m_innerBrush = CreateSolidBrush(RGB(
        static_cast<BYTE>(cfg->innerColor[0] * 255),
        static_cast<BYTE>(cfg->innerColor[1] * 255),
        static_cast<BYTE>(cfg->innerColor[2] * 255)));

    ::InvalidateRect(m_outerPreview, nullptr, TRUE);
    ::InvalidateRect(m_innerPreview, nullptr, TRUE);
}

inline void PropertiesPanel::pickColor(bool outer)
{
    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg) return;

    auto& color = outer ? cfg->outerColor : cfg->innerColor;

    static COLORREF customColors[16] = { 0 };

    CHOOSECOLORW cc = {};
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = m_hWnd;
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

// Shapes handlers
inline LRESULT PropertiesPanel::OnShapeSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_updating) return 0;

    int sel = static_cast<int>(::SendMessage(m_listShapes, LB_GETCURSEL, 0, 0));
    if (sel != LB_ERR && m_app)
    {
        m_app->selectShape(sel);
        updateFromConfig();
    }
    return 0;
}

inline LRESULT PropertiesPanel::OnNewCone(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_app)
    {
        m_app->newShape(ShapeConfig::Type::Cone);
        refreshShapesList();
        updateFromConfig();
    }
    return 0;
}

inline LRESULT PropertiesPanel::OnNewCylinder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_app)
    {
        m_app->newShape(ShapeConfig::Type::Cylinder);
        refreshShapesList();
        updateFromConfig();
    }
    return 0;
}

inline LRESULT PropertiesPanel::OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_app)
    {
        m_app->deleteSelectedShape();
        refreshShapesList();
        updateFromConfig();
    }
    return 0;
}

inline LRESULT PropertiesPanel::OnDuplicate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (!m_app) return 0;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (cfg)
    {
        ShapeConfig newCfg = *cfg;
        newCfg.name = cfg->name + " Copy";
        newCfg.posX += 0.5f;
        newCfg.posY += 0.5f;

        m_app->getShapeManager().addShape(newCfg);
        m_app->selectShape(m_app->getShapeManager().getShapeCount() - 1);
        refreshShapesList();
        updateFromConfig();
    }
    return 0;
}

inline LRESULT PropertiesPanel::OnMoveUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (!m_app) return 0;

    int index = m_app->getSelectedShapeIndex();
    if (index > 0)
    {
        m_app->getShapeManager().swapShapes(index, index - 1);
        m_app->selectShape(index - 1);
        refreshShapesList();
    }
    return 0;
}

inline LRESULT PropertiesPanel::OnMoveDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (!m_app) return 0;

    int index = m_app->getSelectedShapeIndex();
    if (index >= 0 && index < m_app->getShapeManager().getShapeCount() - 1)
    {
        m_app->getShapeManager().swapShapes(index, index + 1);
        m_app->selectShape(index + 1);
        refreshShapesList();
    }
    return 0;
}

inline LRESULT PropertiesPanel::OnWeld(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (!m_app) return 0;

    ShapeManager& mgr = m_app->getShapeManager();
    if (mgr.getShapeCount() < 2)
    {
        MessageBoxW(L"Need at least 2 shapes to weld.", L"Weld", MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    if (MessageBoxW(L"This will merge all shapes into a single shape.\nThis operation cannot be undone.\n\nContinue?",
        L"Weld All Shapes", MB_YESNO | MB_ICONQUESTION) != IDYES)
    {
        return 0;
    }

    if (mgr.weldAllShapes())
    {
        m_app->selectShape(0);
        refreshShapesList();
        updateFromConfig();
        MessageBoxW(L"All shapes welded successfully.", L"Weld", MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        MessageBoxW(L"Weld operation failed.", L"Weld", MB_OK | MB_ICONERROR);
    }

    return 0;
}

inline LRESULT PropertiesPanel::OnVisibleClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_updating || !m_app) return 0;

    ShapeInstance* shape = m_app->getShapeManager().getShape(m_app->getSelectedShapeIndex());
    if (shape)
    {
        bool checked = (::SendMessage(m_chkVisible, BM_GETCHECK, 0, 0) == BST_CHECKED);
        shape->visible = checked;
        refreshShapesList();
    }
    return 0;
}

inline LRESULT PropertiesPanel::OnNameChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_updating || !m_app) return 0;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (cfg)
    {
        wchar_t buf[256];
        ::GetWindowTextW(m_editName, buf, 256);

        std::wstring wname(buf);
        int size = WideCharToMultiByte(CP_UTF8, 0, wname.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size > 0)
        {
            std::string narrow(size - 1, 0);
            WideCharToMultiByte(CP_UTF8, 0, wname.c_str(), -1, &narrow[0], size, nullptr, nullptr);
            cfg->name = narrow;
        }

        refreshShapesList();
    }
    return 0;
}

inline LRESULT PropertiesPanel::OnEditFormulaClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (wNotifyCode != BN_CLICKED)
        return 0;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg)
        return 0;

    std::vector<FormulaSegment> segments = cfg->getEffectiveSegments();

    FormulaEditorDialog dlg;
    if (dlg.Show(m_hWnd, segments))
    {
        cfg->setSegments(segments);
        updateFromConfig();
        m_app->onShapeConfigChanged();
    }

    return 0;
}

inline LRESULT PropertiesPanel::OnOuterPickClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (wNotifyCode == BN_CLICKED)
        pickColor(true);
    return 0;
}

inline LRESULT PropertiesPanel::OnInnerPickClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (wNotifyCode == BN_CLICKED)
        pickColor(false);
    return 0;
}
