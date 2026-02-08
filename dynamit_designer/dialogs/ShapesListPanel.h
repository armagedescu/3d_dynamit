#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <string>

#include <atlbase.h>
#include <atlwin.h>

class DesignerApp;

// Control IDs for ShapesListPanel
#define ID_SLP_LIST_SHAPES      3001
#define ID_SLP_BTN_NEW_CONE     3002
#define ID_SLP_BTN_NEW_CYLINDER 3003
#define ID_SLP_BTN_DELETE       3004
#define ID_SLP_BTN_DUPLICATE    3005
#define ID_SLP_BTN_MOVE_UP      3006
#define ID_SLP_BTN_MOVE_DOWN    3007
#define ID_SLP_BTN_WELD         3008
#define ID_SLP_CHK_VISIBLE      3009
#define ID_SLP_EDIT_NAME        3010

class ShapesListPanel : public CWindowImpl<ShapesListPanel>
{
public:
    DECLARE_WND_CLASS(L"ShapesListPanelClass")

    ShapesListPanel(DesignerApp* app) : m_app(app), m_updating(false) {}

    HWND Create(HWND parent)
    {
        RECT rc = { 0, 0, 280, 200 };
        CWindowImpl::Create(parent, rc, L"Shapes",
            WS_POPUP | WS_CAPTION | WS_VISIBLE, WS_EX_TOOLWINDOW);

        if (m_hWnd)
        {
            createControls();
        }

        return m_hWnd;
    }

    void refreshList();
    void updateSelection();

    BEGIN_MSG_MAP(ShapesListPanel)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        COMMAND_HANDLER(ID_SLP_LIST_SHAPES, LBN_SELCHANGE, OnListSelChange)
        COMMAND_HANDLER(ID_SLP_LIST_SHAPES, LBN_DBLCLK, OnListDblClick)
        COMMAND_ID_HANDLER(ID_SLP_BTN_NEW_CONE, OnNewCone)
        COMMAND_ID_HANDLER(ID_SLP_BTN_NEW_CYLINDER, OnNewCylinder)
        COMMAND_ID_HANDLER(ID_SLP_BTN_DELETE, OnDelete)
        COMMAND_ID_HANDLER(ID_SLP_BTN_DUPLICATE, OnDuplicate)
        COMMAND_ID_HANDLER(ID_SLP_BTN_MOVE_UP, OnMoveUp)
        COMMAND_ID_HANDLER(ID_SLP_BTN_MOVE_DOWN, OnMoveDown)
        COMMAND_ID_HANDLER(ID_SLP_BTN_WELD, OnWeld)
        COMMAND_ID_HANDLER(ID_SLP_CHK_VISIBLE, OnVisibleClick)
        COMMAND_HANDLER(ID_SLP_EDIT_NAME, EN_KILLFOCUS, OnNameChange)
    END_MSG_MAP()

private:
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        ShowWindow(SW_HIDE);
        return 0;
    }

    LRESULT OnListSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnListDblClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnNewCone(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnNewCylinder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDuplicate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnMoveUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnMoveDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnWeld(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnVisibleClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnNameChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    void createControls()
    {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        int y = 5;

        // Shape name edit
        HWND lblName = CreateWindowW(L"STATIC", L"Name:",
            WS_CHILD | WS_VISIBLE,
            5, y + 3, 40, 18, m_hWnd, nullptr, GetModuleHandle(nullptr), nullptr);
        ::SendMessage(lblName, WM_SETFONT, (WPARAM)hFont, TRUE);

        m_editName = CreateWindowW(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            50, y, 150, 20, m_hWnd, (HMENU)(INT_PTR)ID_SLP_EDIT_NAME,
            GetModuleHandle(nullptr), nullptr);
        ::SendMessage(m_editName, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Visible checkbox
        m_chkVisible = CreateWindowW(L"BUTTON", L"Visible",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            210, y, 60, 20, m_hWnd, (HMENU)(INT_PTR)ID_SLP_CHK_VISIBLE,
            GetModuleHandle(nullptr), nullptr);
        ::SendMessage(m_chkVisible, WM_SETFONT, (WPARAM)hFont, TRUE);
        y += 25;

        // List box for shapes
        m_listShapes = CreateWindowW(L"LISTBOX", nullptr,
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
            5, y, 265, 90, m_hWnd, (HMENU)(INT_PTR)ID_SLP_LIST_SHAPES,
            GetModuleHandle(nullptr), nullptr);
        ::SendMessage(m_listShapes, WM_SETFONT, (WPARAM)hFont, TRUE);
        y += 95;

        // Buttons row 1: New Cone, New Cylinder, Delete
        HWND btnCone = CreateWindowW(L"BUTTON", L"+ Cone",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            5, y, 60, 24, m_hWnd, (HMENU)ID_SLP_BTN_NEW_CONE,
            GetModuleHandle(nullptr), nullptr);
        ::SendMessage(btnCone, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND btnCyl = CreateWindowW(L"BUTTON", L"+ Cylinder",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            70, y, 70, 24, m_hWnd, (HMENU)ID_SLP_BTN_NEW_CYLINDER,
            GetModuleHandle(nullptr), nullptr);
        ::SendMessage(btnCyl, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND btnDup = CreateWindowW(L"BUTTON", L"Duplicate",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            145, y, 65, 24, m_hWnd, (HMENU)ID_SLP_BTN_DUPLICATE,
            GetModuleHandle(nullptr), nullptr);
        ::SendMessage(btnDup, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND btnDel = CreateWindowW(L"BUTTON", L"Delete",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            215, y, 55, 24, m_hWnd, (HMENU)ID_SLP_BTN_DELETE,
            GetModuleHandle(nullptr), nullptr);
        ::SendMessage(btnDel, WM_SETFONT, (WPARAM)hFont, TRUE);
        y += 28;

        // Buttons row 2: Move Up, Move Down, Weld All
        HWND btnUp = CreateWindowW(L"BUTTON", L"\u25B2 Up",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            5, y, 55, 24, m_hWnd, (HMENU)ID_SLP_BTN_MOVE_UP,
            GetModuleHandle(nullptr), nullptr);
        ::SendMessage(btnUp, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND btnDown = CreateWindowW(L"BUTTON", L"\u25BC Down",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            65, y, 60, 24, m_hWnd, (HMENU)ID_SLP_BTN_MOVE_DOWN,
            GetModuleHandle(nullptr), nullptr);
        ::SendMessage(btnDown, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND btnWeld = CreateWindowW(L"BUTTON", L"Weld All",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            185, y, 85, 24, m_hWnd, (HMENU)ID_SLP_BTN_WELD,
            GetModuleHandle(nullptr), nullptr);
        ::SendMessage(btnWeld, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    DesignerApp* m_app;
    bool m_updating;

    HWND m_listShapes;
    HWND m_editName;
    HWND m_chkVisible;
};

// Include implementation
#include "../DesignerApp.h"
#include "../ShapeManager.h"

inline void ShapesListPanel::refreshList()
{
    if (!m_hWnd || !m_app) return;

    m_updating = true;

    // Remember current selection
    int currentSel = static_cast<int>(::SendMessage(m_listShapes, LB_GETCURSEL, 0, 0));

    // Clear list
    ::SendMessage(m_listShapes, LB_RESETCONTENT, 0, 0);

    // Add all shapes
    ShapeManager& mgr = m_app->getShapeManager();
    for (int i = 0; i < mgr.getShapeCount(); ++i)
    {
        const ShapeInstance* shape = mgr.getShape(i);
        if (shape)
        {
            // Format: "[C] Name" or "[Y] Name" for Cone/Cylinder
            std::wstring item;
            if (shape->config.type == ShapeConfig::Type::Cone)
                item = L"[C] ";
            else
                item = L"[Y] ";

            // Add visibility indicator
            if (!shape->visible)
                item = L"(" + item.substr(0, 3) + L") ";

            // Convert name to wide string using MultiByteToWideChar
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

    // Restore selection
    int selectedIndex = m_app->getSelectedShapeIndex();
    if (selectedIndex >= 0 && selectedIndex < mgr.getShapeCount())
    {
        ::SendMessage(m_listShapes, LB_SETCURSEL, selectedIndex, 0);
    }

    updateSelection();

    m_updating = false;
}

inline void ShapesListPanel::updateSelection()
{
    if (!m_hWnd || !m_app) return;

    m_updating = true;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (cfg)
    {
        // Update name edit using MultiByteToWideChar
        const std::string& name = cfg->name;
        int wsize = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
        if (wsize > 0)
        {
            std::wstring wname(wsize - 1, 0);
            MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, &wname[0], wsize);
            ::SetWindowTextW(m_editName, wname.c_str());
        }

        // Update visibility checkbox
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

    m_updating = false;
}

inline LRESULT ShapesListPanel::OnListSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_updating) return 0;

    int sel = static_cast<int>(::SendMessage(m_listShapes, LB_GETCURSEL, 0, 0));
    if (sel != LB_ERR && m_app)
    {
        m_app->selectShape(sel);
        updateSelection();
    }
    return 0;
}

inline LRESULT ShapesListPanel::OnListDblClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    // Double-click to focus on shape (could center view on it)
    // For now, just select it
    return OnListSelChange(wNotifyCode, wID, hWndCtl, bHandled);
}

inline LRESULT ShapesListPanel::OnNewCone(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_app)
    {
        m_app->newShape(ShapeConfig::Type::Cone);
        refreshList();
    }
    return 0;
}

inline LRESULT ShapesListPanel::OnNewCylinder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_app)
    {
        m_app->newShape(ShapeConfig::Type::Cylinder);
        refreshList();
    }
    return 0;
}

inline LRESULT ShapesListPanel::OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_app)
    {
        m_app->deleteSelectedShape();
        refreshList();
    }
    return 0;
}

inline LRESULT ShapesListPanel::OnDuplicate(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (!m_app) return 0;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (cfg)
    {
        // Create a copy with slightly different position
        ShapeConfig newCfg = *cfg;
        newCfg.name = cfg->name + " Copy";
        newCfg.posX += 0.5f;
        newCfg.posY += 0.5f;

        m_app->getShapeManager().addShape(newCfg);
        m_app->selectShape(m_app->getShapeManager().getShapeCount() - 1);
        refreshList();
    }
    return 0;
}

inline LRESULT ShapesListPanel::OnMoveUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (!m_app) return 0;

    int index = m_app->getSelectedShapeIndex();
    if (index > 0)
    {
        m_app->getShapeManager().swapShapes(index, index - 1);
        m_app->selectShape(index - 1);
        refreshList();
    }
    return 0;
}

inline LRESULT ShapesListPanel::OnMoveDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (!m_app) return 0;

    int index = m_app->getSelectedShapeIndex();
    if (index >= 0 && index < m_app->getShapeManager().getShapeCount() - 1)
    {
        m_app->getShapeManager().swapShapes(index, index + 1);
        m_app->selectShape(index + 1);
        refreshList();
    }
    return 0;
}

inline LRESULT ShapesListPanel::OnWeld(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (!m_app) return 0;

    ShapeManager& mgr = m_app->getShapeManager();
    if (mgr.getShapeCount() < 2)
    {
        MessageBoxW(L"Need at least 2 shapes to weld.", L"Weld", MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    // Confirm
    if (MessageBoxW(L"This will merge all shapes into a single shape.\nThis operation cannot be undone.\n\nContinue?",
        L"Weld All Shapes", MB_YESNO | MB_ICONQUESTION) != IDYES)
    {
        return 0;
    }

    // Perform weld
    if (mgr.weldAllShapes())
    {
        m_app->selectShape(0);
        refreshList();
        MessageBoxW(L"All shapes welded successfully.", L"Weld", MB_OK | MB_ICONINFORMATION);
    }
    else
    {
        MessageBoxW(L"Weld operation failed.", L"Weld", MB_OK | MB_ICONERROR);
    }

    return 0;
}

inline LRESULT ShapesListPanel::OnVisibleClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_updating || !m_app) return 0;

    ShapeInstance* shape = m_app->getShapeManager().getShape(m_app->getSelectedShapeIndex());
    if (shape)
    {
        bool checked = (::SendMessage(m_chkVisible, BM_GETCHECK, 0, 0) == BST_CHECKED);
        shape->visible = checked;
        refreshList();
    }
    return 0;
}

inline LRESULT ShapesListPanel::OnNameChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_updating || !m_app) return 0;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (cfg)
    {
        wchar_t buf[256];
        ::GetWindowTextW(m_editName, buf, 256);

        // Convert wide to narrow using WideCharToMultiByte
        std::wstring wname(buf);
        int size = WideCharToMultiByte(CP_UTF8, 0, wname.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size > 0)
        {
            std::string narrow(size - 1, 0);
            WideCharToMultiByte(CP_UTF8, 0, wname.c_str(), -1, &narrow[0], size, nullptr, nullptr);
            cfg->name = narrow;
        }

        refreshList();
    }
    return 0;
}
