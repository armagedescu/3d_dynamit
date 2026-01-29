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
    enum {
        ID_CULL_FACE = 101,
        ID_WIREFRAME = 102,
        ID_SHOW_NORMALS = 103,
        ID_CW_WINDING = 104
    };

    BEGIN_MSG_MAP(OptionsDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_COMMAND, OnCommand)
        MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
        MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        // Create checkboxes
        CreateWindowW(L"BUTTON", L"Cull Face", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 20, 20, 150, 25, m_hWnd, (HMENU)ID_CULL_FACE, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"Wireframe", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,  20, 50, 150, 25, m_hWnd, (HMENU)ID_WIREFRAME, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"Show Normals", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 20, 80, 150, 25, m_hWnd, (HMENU)ID_SHOW_NORMALS, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"CW Winding", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,  20, 110, 150, 25, m_hWnd, (HMENU)ID_CW_WINDING, nullptr, nullptr);

        // OK button
        //CreateWindowW(L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,  50, 150, 80, 30, m_hWnd, (HMENU)IDOK, nullptr, nullptr);

        // Set initial states
        CheckDlgButton(ID_CULL_FACE, m_options.cullFace ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(ID_WIREFRAME, m_options.wireframe ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(ID_SHOW_NORMALS, m_options.showNormals ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(ID_CW_WINDING, m_options.cwWinding ? BST_CHECKED : BST_UNCHECKED);

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
    LRESULT OnShowWindow(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        switch (wParam)
        {
        case TRUE:
            {
            RECT rcParent;
            ::GetWindowRect(GetParent(), &rcParent);
            SetWindowPos(nullptr,
                rcParent.right + 10,  // 10 pixels to the right of parent
                rcParent.top,         // Same top position
                0, 0,
                SWP_NOSIZE | SWP_NOZORDER);
            }
			bHandled = TRUE;
            break;
        }
        return 0;
    }
    LRESULT OnCommand(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
		std::cout << "OnCommand called:" << msg<< " wParam: "<< wParam << std::endl;
        bHandled = TRUE;
        switch (wParam)
        {
        case ID_CULL_FACE:
            m_options.cullFace = (IsDlgButtonChecked(101) == BST_CHECKED);
			return 0;
        case ID_WIREFRAME:
            m_options.wireframe = (IsDlgButtonChecked(102) == BST_CHECKED);
            return 0;
        case ID_SHOW_NORMALS:
            m_options.showNormals = (IsDlgButtonChecked(103) == BST_CHECKED);
            return 0;
        case ID_CW_WINDING:
            m_options.cwWinding = (IsDlgButtonChecked(104) == BST_CHECKED);
            return 0;
        }
        bHandled = FALSE;

        return 0;
	}

    LRESULT OnCancel(WORD, WORD, HWND, BOOL&)
    {
        ShowWindow(SW_HIDE);
        return 0;
    }

    LRESULT OnKeyDown(UINT, WPARAM wParam, LPARAM, BOOL& bHandled)
    {
		std::cout << "key down: " << wParam << std::endl;
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
        dlgData.tmpl.style = WS_POPUP | WS_CAPTION | WS_VISIBLE; //;// //| WS_SYSMENU  | DS_CENTER;  // Removed DS_SETFONT
        dlgData.tmpl.dwExtendedStyle = WS_EX_TOOLWINDOW;
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
        CDialogImpl<OptionsDialog>::

        m_hWnd = ::CreateDialogIndirectParamW(
            _AtlBaseModule.GetModuleInstance(),
            reinterpret_cast<LPCDLGTEMPLATE>(&dlgData),
            parent,
            StartDialogProc,
            0);

        return m_hWnd;
    }
};