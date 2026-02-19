#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <uxtheme.h>
#include <string>

#include <atlbase.h>
#include <atlwin.h>

#include "Theme.h"

#pragma comment(lib, "uxtheme.lib")

class DesignerApp;

// Control IDs for ObjectExplorerPanel (prefix ID_OE_)
#define ID_OE_LIST_SHAPES       7100
#define ID_OE_EDIT_NAME         7101
#define ID_OE_CHK_VISIBLE       7102
#define ID_OE_BTN_DUPLICATE     7103
#define ID_OE_BTN_DELETE        7104
#define ID_OE_BTN_MOVE_UP       7105
#define ID_OE_BTN_MOVE_DOWN     7106
#define ID_OE_BTN_WELD          7107

class ObjectExplorerPanel : public CWindowImpl<ObjectExplorerPanel>
{
public:
    DECLARE_WND_CLASS(L"ObjectExplorerPanelClass")

    ObjectExplorerPanel(DesignerApp* app) : m_app(app), m_updating(false) {}

    HWND Create(HWND parent)
    {
        // Client area: name row(24) + list(80) + buttons row(24) + margins = 138px client height
        RECT rc = { 0, 0, 290, 138 };
        AdjustWindowRect(&rc, WS_POPUP | WS_CAPTION, FALSE);
        rc.right -= rc.left; rc.bottom -= rc.top; rc.left = 0; rc.top = 0;
        CWindowImpl::Create(parent, rc, L"Object Explorer",
            WS_POPUP | WS_CAPTION | WS_VISIBLE, WS_EX_TOOLWINDOW);

        if (m_hWnd)
        {
            createControls();
            applyTheme();
            m_themeListenerId = Theme::instance().addListener([this]() { applyTheme(); });
        }

        return m_hWnd;
    }

    void refreshShapesList();
    void updateShapeSelection();

    BEGIN_MSG_MAP(ObjectExplorerPanel)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
        MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColorBtn)
        MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnCtlColorEdit)
        MESSAGE_HANDLER(WM_CTLCOLORLISTBOX, OnCtlColorListbox)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        COMMAND_HANDLER(ID_OE_LIST_SHAPES, LBN_SELCHANGE, OnShapeSelChange)
        COMMAND_ID_HANDLER(ID_OE_BTN_DUPLICATE, OnDuplicate)
        COMMAND_ID_HANDLER(ID_OE_BTN_DELETE, OnDelete)
        COMMAND_ID_HANDLER(ID_OE_BTN_MOVE_UP, OnMoveUp)
        COMMAND_ID_HANDLER(ID_OE_BTN_MOVE_DOWN, OnMoveDown)
        COMMAND_ID_HANDLER(ID_OE_BTN_WELD, OnWeld)
        COMMAND_ID_HANDLER(ID_OE_CHK_VISIBLE, OnVisibleClick)
        COMMAND_HANDLER(ID_OE_EDIT_NAME, EN_KILLFOCUS, OnNameChange)
    END_MSG_MAP()

private:
    LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&)
    {
        ShowWindow(SW_HIDE);
        return 0;
    }

    LRESULT OnCtlColorStatic(UINT, WPARAM wParam, LPARAM, BOOL&)
    {
        return (LRESULT)Theme::instance().onCtlColorStatic((HDC)wParam);
    }

    LRESULT OnCtlColorBtn(UINT, WPARAM wParam, LPARAM, BOOL&)
    {
        return (LRESULT)Theme::instance().onCtlColorStatic((HDC)wParam);
    }

    LRESULT OnCtlColorEdit(UINT, WPARAM wParam, LPARAM, BOOL&)
    {
        HDC hdc = (HDC)wParam;
        auto& c = Theme::instance().colors();
        SetTextColor(hdc, c.controlText);
        SetBkColor(hdc, c.controlBg);
        return (LRESULT)Theme::instance().controlBrush();
    }

    LRESULT OnCtlColorListbox(UINT, WPARAM wParam, LPARAM, BOOL&)
    {
        HDC hdc = (HDC)wParam;
        auto& c = Theme::instance().colors();
        SetTextColor(hdc, c.controlText);
        SetBkColor(hdc, c.controlBg);
        return (LRESULT)Theme::instance().controlBrush();
    }

    LRESULT OnEraseBkgnd(UINT, WPARAM wParam, LPARAM, BOOL&)
    {
        RECT rc;
        GetClientRect(&rc);
        FillRect((HDC)wParam, &rc, Theme::instance().backgroundBrush());
        return TRUE;
    }

    LRESULT OnShapeSelChange(WORD, WORD, HWND, BOOL&);
    LRESULT OnDuplicate(WORD, WORD, HWND, BOOL&);
    LRESULT OnDelete(WORD, WORD, HWND, BOOL&);
    LRESULT OnMoveUp(WORD, WORD, HWND, BOOL&);
    LRESULT OnMoveDown(WORD, WORD, HWND, BOOL&);
    LRESULT OnWeld(WORD, WORD, HWND, BOOL&);
    LRESULT OnVisibleClick(WORD, WORD, HWND, BOOL&);
    LRESULT OnNameChange(WORD, WORD, HWND, BOOL&);

    void createControls()
    {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        int margin = 8;
        int btnGap = 3;
        int rowH = 24;
        int contentX = margin * 2;
        int labelW = 40;

        int btnDupW = 35, btnDelW = 35, btnUpW = 24, btnDownW = 24, btnWeldW = 42;
        int buttonsRowW = btnDupW + btnDelW + btnUpW + btnDownW + btnWeldW + 4 * btnGap;

        // Panel width derived from button row
        m_panelWidth = buttonsRowW + 4 * margin;
        int contentW = m_panelWidth - 4 * margin;
        int visibleChkW = 55;

        int y = 5;

        // Name + Visible row
        HWND lblName = CreateWindowW(L"STATIC", L"Name:", WS_CHILD | WS_VISIBLE,
            contentX, y + 3, labelW, 18, m_hWnd, nullptr, GetModuleHandle(nullptr), nullptr);
        SendMessage(lblName, WM_SETFONT, (WPARAM)hFont, TRUE);

        int nameEditW = contentW - labelW - visibleChkW - btnGap;
        m_editName = CreateWindowW(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            contentX + labelW, y, nameEditW, 20, m_hWnd,
            (HMENU)ID_OE_EDIT_NAME, GetModuleHandle(nullptr), nullptr);
        SendMessage(m_editName, WM_SETFONT, (WPARAM)hFont, TRUE);

        m_chkVisible = CreateWindowW(L"BUTTON", L"Visible",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            contentX + labelW + nameEditW + btnGap, y, visibleChkW, 20,
            m_hWnd, (HMENU)ID_OE_CHK_VISIBLE, GetModuleHandle(nullptr), nullptr);
        SendMessage(m_chkVisible, WM_SETFONT, (WPARAM)hFont, TRUE);
        y += rowH;

        // Shapes list
        m_listShapes = CreateWindowW(L"LISTBOX", nullptr,
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_EXTENDEDSEL,
            contentX, y, contentW, 75, m_hWnd,
            (HMENU)ID_OE_LIST_SHAPES, GetModuleHandle(nullptr), nullptr);
        SendMessage(m_listShapes, WM_SETFONT, (WPARAM)hFont, TRUE);
        y += 80;

        // Buttons row
        int bx = contentX;
        auto makeBtn = [&](const wchar_t* text, int id, int w) {
            HWND h = CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                bx, y, w, rowH, m_hWnd, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
            SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
            bx += w + btnGap;
        };

        makeBtn(L"Dup",        ID_OE_BTN_DUPLICATE, btnDupW);
        makeBtn(L"Del",        ID_OE_BTN_DELETE,    btnDelW);
        makeBtn(L"\u25B2",     ID_OE_BTN_MOVE_UP,   btnUpW);
        makeBtn(L"\u25BC",     ID_OE_BTN_MOVE_DOWN, btnDownW);
        makeBtn(L"Weld",       ID_OE_BTN_WELD,      btnWeldW);
    }

    std::vector<int> getSelectedIndices() const
    {
        int count = (int)::SendMessage(m_listShapes, LB_GETSELCOUNT, 0, 0);
        if (count <= 0) return {};
        std::vector<int> sel(count);
        ::SendMessage(m_listShapes, LB_GETSELITEMS, count, (LPARAM)sel.data());
        return sel;
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
    bool m_updating;
    int m_themeListenerId = 0;
    int m_panelWidth = 290;

    HWND m_listShapes = nullptr;
    HWND m_editName = nullptr;
    HWND m_chkVisible = nullptr;
};

#include "../DesignerApp.h"
#include "../ShapeManager.h"
#include <algorithm>

inline void ObjectExplorerPanel::refreshShapesList()
{
    if (!m_hWnd || !m_app || !m_listShapes) return;

    m_updating = true;
    ::SendMessage(m_listShapes, LB_RESETCONTENT, 0, 0);

    ShapeManager& mgr = m_app->getShapeManager();
    int shapeCount = mgr.getShapeCount();

    for (int i = 0; i < shapeCount; ++i)
    {
        const ShapeInstance* shape = mgr.getShape(i);
        if (!shape) continue;

        std::wstring item = (shape->config.type == ShapeConfig::Type::Cone) ? L"[C] " : L"[Y] ";
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

    // Clear all selections, then highlight the current single selection
    ::SendMessage(m_listShapes, LB_SETSEL, FALSE, -1);
    int sel = m_app->getSelectedShapeIndex();
    if (sel < 0 && shapeCount > 0) sel = 0;
    if (sel >= 0 && sel < shapeCount)
    {
        ::SendMessage(m_listShapes, LB_SETSEL, TRUE, sel);
        ::SendMessage(m_listShapes, LB_SETCARETINDEX, sel, FALSE);
    }

    ::InvalidateRect(m_listShapes, nullptr, TRUE);
    updateShapeSelection();
    m_updating = false;
}

inline void ObjectExplorerPanel::updateShapeSelection()
{
    if (!m_hWnd || !m_app) return;

    bool prev = m_updating;
    m_updating = true;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (cfg)
    {
        int wsize = MultiByteToWideChar(CP_UTF8, 0, cfg->name.c_str(), -1, nullptr, 0);
        if (wsize > 0)
        {
            std::wstring wname(wsize - 1, 0);
            MultiByteToWideChar(CP_UTF8, 0, cfg->name.c_str(), -1, &wname[0], wsize);
            ::SetWindowTextW(m_editName, wname.c_str());
        }

        ShapeInstance* shape = m_app->getShapeManager().getShape(m_app->getSelectedShapeIndex());
        if (shape)
            ::SendMessage(m_chkVisible, BM_SETCHECK, shape->visible ? BST_CHECKED : BST_UNCHECKED, 0);
    }
    else
    {
        ::SetWindowTextW(m_editName, L"");
        ::SendMessage(m_chkVisible, BM_SETCHECK, BST_UNCHECKED, 0);
    }

    m_updating = prev;
}

inline LRESULT ObjectExplorerPanel::OnShapeSelChange(WORD, WORD, HWND, BOOL&)
{
    if (m_updating) return 0;
    // With LBS_EXTENDEDSEL, use the caret (focus) item as the active shape for the properties panel
    int caret = static_cast<int>(::SendMessage(m_listShapes, LB_GETCARETINDEX, 0, 0));
    if (caret != LB_ERR && caret >= 0 && m_app)
    {
        m_app->selectShape(caret);
        updateShapeSelection();
    }
    return 0;
}

inline LRESULT ObjectExplorerPanel::OnDuplicate(WORD, WORD, HWND, BOOL&)
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
    }
    return 0;
}

inline LRESULT ObjectExplorerPanel::OnDelete(WORD, WORD, HWND, BOOL&)
{
    if (m_app)
    {
        m_app->deleteSelectedShape();
        refreshShapesList();
    }
    return 0;
}

inline LRESULT ObjectExplorerPanel::OnMoveUp(WORD, WORD, HWND, BOOL&)
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

inline LRESULT ObjectExplorerPanel::OnMoveDown(WORD, WORD, HWND, BOOL&)
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

inline LRESULT ObjectExplorerPanel::OnWeld(WORD, WORD, HWND, BOOL&)
{
    if (!m_app) return 0;
    ShapeManager& mgr = m_app->getShapeManager();

    std::vector<int> sel = getSelectedIndices();

    if (sel.size() < 2)
    {
        ::MessageBoxW(m_hWnd,
            L"Select at least 2 shapes to weld.\n(Ctrl+Click or Shift+Click to multi-select)",
            L"Weld", MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    if (::MessageBoxW(m_hWnd,
        L"Merge the selected shapes into one?\nThis cannot be undone.",
        L"Weld Selected", MB_YESNO | MB_ICONQUESTION) != IDYES)
        return 0;

    int insertAt = *std::min_element(sel.begin(), sel.end());

    if (mgr.weldShapes(sel))
    {
        m_app->selectShape(insertAt);
        refreshShapesList();
    }
    else
    {
        ::MessageBoxW(m_hWnd, L"Weld operation failed.", L"Weld", MB_OK | MB_ICONERROR);
    }
    return 0;
}

inline LRESULT ObjectExplorerPanel::OnVisibleClick(WORD, WORD, HWND, BOOL&)
{
    if (m_updating || !m_app) return 0;
    ShapeInstance* shape = m_app->getShapeManager().getShape(m_app->getSelectedShapeIndex());
    if (shape)
    {
        shape->visible = (::SendMessage(m_chkVisible, BM_GETCHECK, 0, 0) == BST_CHECKED);
        refreshShapesList();
    }
    return 0;
}

inline LRESULT ObjectExplorerPanel::OnNameChange(WORD, WORD, HWND, BOOL&)
{
    if (m_updating || !m_app) return 0;
    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (cfg)
    {
        wchar_t buf[256];
        ::GetWindowTextW(m_editName, buf, 256);
        int size = WideCharToMultiByte(CP_UTF8, 0, buf, -1, nullptr, 0, nullptr, nullptr);
        if (size > 0)
        {
            std::string narrow(size - 1, 0);
            WideCharToMultiByte(CP_UTF8, 0, buf, -1, &narrow[0], size, nullptr, nullptr);
            cfg->name = narrow;
        }
        refreshShapesList();
    }
    return 0;
}
