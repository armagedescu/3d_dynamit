#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atlbase.h>
#include <atlwin.h>

struct RenderOptions
{
    bool cullFace = true;
    bool wireframe = false;
    bool showNormals = true;
    bool cwWinding = false;
    float ambientLight = 0.2f;
};

class OptionsDialog : public CDialogImpl<OptionsDialog>
{
public:
    enum { IDD = 0 }; // No resource, create dynamically

    RenderOptions& m_options;

    OptionsDialog(RenderOptions& opts) : m_options(opts) {}

    BEGIN_MSG_MAP(OptionsDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        COMMAND_ID_HANDLER(IDOK, OnOK)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(101, OnCullFace)
        COMMAND_ID_HANDLER(102, OnWireframe)
        COMMAND_ID_HANDLER(103, OnShowNormals)
        COMMAND_ID_HANDLER(104, OnCWWinding)
        MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        // Create checkboxes
        CreateWindowW(L"BUTTON", L"Cull Face", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            20, 20, 150, 25, m_hWnd, (HMENU)101, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"Wireframe", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            20, 50, 150, 25, m_hWnd, (HMENU)102, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"Show Normals", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            20, 80, 150, 25, m_hWnd, (HMENU)103, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"CW Winding", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            20, 110, 150, 25, m_hWnd, (HMENU)104, nullptr, nullptr);

        // OK button
        CreateWindowW(L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            50, 150, 80, 30, m_hWnd, (HMENU)IDOK, nullptr, nullptr);

        // Set initial states
        CheckDlgButton(101, m_options.cullFace ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(102, m_options.wireframe ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(103, m_options.showNormals ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(104, m_options.cwWinding ? BST_CHECKED : BST_UNCHECKED);

        // Position dialog to the right of the parent window
        HWND hParent = GetParent();
        if (hParent)
        {
            RECT rcParent;
            ::GetWindowRect(hParent, &rcParent);
            SetWindowPos(nullptr,
                rcParent.right + 10,  // 10 pixels to the right of parent
                rcParent.top,         // Same top position
                0, 0,
                SWP_NOSIZE | SWP_NOZORDER);
        }

        return TRUE;
    }

    LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&)
    {
        ShowWindow(SW_HIDE);
        return 0;
    }

    LRESULT OnOK(WORD, WORD, HWND, BOOL&)
    {
        ShowWindow(SW_HIDE);
        return 0;
    }

    LRESULT OnCancel(WORD, WORD, HWND, BOOL&)
    {
        ShowWindow(SW_HIDE);
        return 0;
    }

    LRESULT OnCullFace(WORD, WORD, HWND, BOOL&)
    {
        m_options.cullFace = (IsDlgButtonChecked(101) == BST_CHECKED);
        return 0;
    }

    LRESULT OnWireframe(WORD, WORD, HWND, BOOL&)
    {
        m_options.wireframe = (IsDlgButtonChecked(102) == BST_CHECKED);
        return 0;
    }

    LRESULT OnShowNormals(WORD, WORD, HWND, BOOL&)
    {
        m_options.showNormals = (IsDlgButtonChecked(103) == BST_CHECKED);
        return 0;
    }

    LRESULT OnCWWinding(WORD, WORD, HWND, BOOL&)
    {
        m_options.cwWinding = (IsDlgButtonChecked(104) == BST_CHECKED);
        return 0;
    }

    LRESULT OnKeyDown(UINT, WPARAM wParam, LPARAM, BOOL& bHandled)
    {
        if (wParam == VK_ESCAPE)
        {
            ShowWindow(SW_HIDE);
            return 0;
        }
        bHandled = FALSE;
        return 0;
    }

    // Create modeless dialog with in-memory template
    HWND CreateModeless(HWND parent)
    {
        #pragma pack(push, 4)
        struct DlgTemplate
        {
            DLGTEMPLATE tmpl;
            WORD menu;
            WORD windowClass;
            WCHAR title[16];
        };
        #pragma pack(pop)

        static DlgTemplate dlgData = {};
        dlgData.tmpl.style = WS_POPUP | WS_CAPTION | WS_SYSMENU;// | DS_CENTER;  // Removed DS_SETFONT
        dlgData.tmpl.dwExtendedStyle = 0;
        dlgData.tmpl.cdit = 0;
        dlgData.tmpl.x = 0;
        dlgData.tmpl.y = 0;
        dlgData.tmpl.cx = 120;
        dlgData.tmpl.cy = 110;
        dlgData.menu = 0;
        dlgData.windowClass = 0;
        wcscpy_s(dlgData.title, L"Options");

        ATLASSERT(m_hWnd == NULL);
        _AtlWinModule.AddCreateWndData(&m_thunk.cd, this);

        m_hWnd = ::CreateDialogIndirectParamW(
            _AtlBaseModule.GetModuleInstance(),
            reinterpret_cast<LPCDLGTEMPLATE>(&dlgData),
            parent,
            StartDialogProc,
            0);

        return m_hWnd;
    }
};