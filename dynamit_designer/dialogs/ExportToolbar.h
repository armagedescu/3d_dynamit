#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <uxtheme.h>

#include <atlbase.h>
#include <atlwin.h>

#include "Theme.h"

#pragma comment(lib, "uxtheme.lib")

class DesignerApp;

// Control IDs
#define ID_BTN_EXPORT_CLIP      2001
#define ID_BTN_EXPORT_FILE      2002
#define ID_CHK_INCLUDE_DYNAMIT  2003
#define ID_CHK_COMPLETE_APP     2004
#define ID_CHK_INCLUDE_NORMALS  2005
#define ID_BTN_SAVE_PROJECT     2006
#define ID_BTN_LOAD_PROJECT     2007
#define ID_BTN_ET_THEME_LIGHT   2010
#define ID_BTN_ET_THEME_DARK    2011
#define ID_BTN_ET_THEME_AUTO    2012

class ExportToolbar : public CWindowImpl<ExportToolbar>
{
public:
    DECLARE_WND_CLASS(L"ExportToolbarClass")

    ExportToolbar(DesignerApp* app) : m_app(app) {}

    HWND Create(HWND parent)
    {
        RECT rc = { 0, 0, 290, 195 };
        CWindowImpl::Create(parent, rc, L"Export / Project",
            WS_POPUP | WS_CAPTION | WS_VISIBLE, WS_EX_TOOLWINDOW);

        if (m_hWnd)
        {
            createControls();
            applyTheme();
            m_themeListenerId = Theme::instance().addListener([this]() { applyTheme(); });
        }

        return m_hWnd;
    }

    BEGIN_MSG_MAP(ExportToolbar)
        COMMAND_ID_HANDLER(ID_BTN_EXPORT_CLIP, OnExportClip)
        COMMAND_ID_HANDLER(ID_BTN_EXPORT_FILE, OnExportFile)
        COMMAND_ID_HANDLER(ID_BTN_SAVE_PROJECT, OnSaveProject)
        COMMAND_ID_HANDLER(ID_BTN_LOAD_PROJECT, OnSaveAs)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColor)
        MESSAGE_HANDLER(WM_CTLCOLORBTN, OnCtlColor)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
    END_MSG_MAP()

private:
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        ShowWindow(SW_HIDE);
        return 0;
    }

    LRESULT OnCtlColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        return (LRESULT)Theme::instance().onCtlColorStatic((HDC)wParam);
    }

    LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(&rc);
        FillRect(hdc, &rc, Theme::instance().backgroundBrush());
        return TRUE;
    }

    LRESULT OnExportClip(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnExportFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSaveProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSaveAs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    void createControls()
    {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        // Include Dynamit Setup checkbox
        HWND chkDynamit = CreateWindowW(L"BUTTON", L"Include Dynamit Renderer Setup",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, 10, 265, 18, m_hWnd, (HMENU)ID_CHK_INCLUDE_DYNAMIT,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(chkDynamit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(chkDynamit, BM_SETCHECK, BST_CHECKED, 0); // Default checked

        // Generate Complete App checkbox
        HWND chkApp = CreateWindowW(L"BUTTON", L"Generate Complete Standalone App",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, 32, 265, 18, m_hWnd, (HMENU)ID_CHK_COMPLETE_APP,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(chkApp, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Include Normals Highlighter checkbox
        HWND chkNormals = CreateWindowW(L"BUTTON", L"Include Normals Highlighter",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, 54, 265, 18, m_hWnd, (HMENU)ID_CHK_INCLUDE_NORMALS,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(chkNormals, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Copy Code button
        HWND btnExportClip = CreateWindowW(L"BUTTON", L"Copy Code",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 82, 125, 26, m_hWnd, (HMENU)ID_BTN_EXPORT_CLIP,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(btnExportClip, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Save Code button
        HWND btnExportFile = CreateWindowW(L"BUTTON", L"Save Code...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            145, 82, 125, 26, m_hWnd, (HMENU)ID_BTN_EXPORT_FILE,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(btnExportFile, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Separator line (static text)
        HWND separator = CreateWindowW(L"STATIC", L"\u2500\u2500 Project \u2500\u2500",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            10, 115, 265, 16, m_hWnd, nullptr,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(separator, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Save Project button
        HWND btnSaveProject = CreateWindowW(L"BUTTON", L"Save Project...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 135, 125, 26, m_hWnd, (HMENU)ID_BTN_SAVE_PROJECT,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(btnSaveProject, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Save As button
        HWND btnLoadProject = CreateWindowW(L"BUTTON", L"Save As...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            145, 135, 125, 26, m_hWnd, (HMENU)ID_BTN_LOAD_PROJECT,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(btnLoadProject, WM_SETFONT, (WPARAM)hFont, TRUE);
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
    int m_themeListenerId = 0;
};

// Include implementation inline to avoid separate cpp
#include "../DesignerApp.h"
#include "../ShapeManager.h"
#include "../CodeExporter.h"
#include "../ProjectManager.h"

inline LRESULT ExportToolbar::OnExportClip(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_app)
    {
        ShapeConfig* config = m_app->getSelectedShapeConfig();
        if (config)
        {
            bool includeDynamit = (::SendMessage(GetDlgItem(ID_CHK_INCLUDE_DYNAMIT), BM_GETCHECK, 0, 0) == BST_CHECKED);
            bool completeApp = (::SendMessage(GetDlgItem(ID_CHK_COMPLETE_APP), BM_GETCHECK, 0, 0) == BST_CHECKED);
            bool includeNormals = (::SendMessage(GetDlgItem(ID_CHK_INCLUDE_NORMALS), BM_GETCHECK, 0, 0) == BST_CHECKED);

            CodeExporter::ExportMode mode;
            if (completeApp)
                mode = CodeExporter::ExportMode::StandaloneApplication;
            else if (includeDynamit)
                mode = CodeExporter::ExportMode::WithDynamitSetup;
            else
                mode = CodeExporter::ExportMode::GeometryOnly;

            std::wstring code = CodeExporter::generateCppCode(*config, mode, includeNormals);
            if (CodeExporter::copyToClipboard(code))
                MessageBoxW(L"C++ code copied to clipboard!", L"Export", MB_OK | MB_ICONINFORMATION);
            else
                MessageBoxW(L"Failed to copy to clipboard", L"Error", MB_OK | MB_ICONERROR);
        }
        else
        {
            MessageBoxW(L"No shape selected", L"Export", MB_OK | MB_ICONWARNING);
        }
    }
    return 0;
}

inline LRESULT ExportToolbar::OnExportFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_app)
    {
        ShapeConfig* config = m_app->getSelectedShapeConfig();
        if (config)
        {
            bool includeDynamit = (::SendMessage(GetDlgItem(ID_CHK_INCLUDE_DYNAMIT), BM_GETCHECK, 0, 0) == BST_CHECKED);
            bool completeApp = (::SendMessage(GetDlgItem(ID_CHK_COMPLETE_APP), BM_GETCHECK, 0, 0) == BST_CHECKED);
            bool includeNormals = (::SendMessage(GetDlgItem(ID_CHK_INCLUDE_NORMALS), BM_GETCHECK, 0, 0) == BST_CHECKED);

            CodeExporter::ExportMode mode;
            if (completeApp)
                mode = CodeExporter::ExportMode::StandaloneApplication;
            else if (includeDynamit)
                mode = CodeExporter::ExportMode::WithDynamitSetup;
            else
                mode = CodeExporter::ExportMode::GeometryOnly;

            std::wstring code = CodeExporter::generateCppCode(*config, mode, includeNormals);
            if (CodeExporter::saveToFile(m_hWnd, code))
                MessageBoxW(L"C++ code saved successfully!", L"Export", MB_OK | MB_ICONINFORMATION);
        }
        else
        {
            MessageBoxW(L"No shape selected", L"Export", MB_OK | MB_ICONWARNING);
        }
    }
    return 0;
}

inline LRESULT ExportToolbar::OnSaveProject(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_app)
    {
        ProjectManager& pm = m_app->getProjectManager();
        if (pm.saveProject())
            MessageBoxW(L"Project saved successfully!", L"Save Project", MB_OK | MB_ICONINFORMATION);
        else if (!pm.getLastError().empty())
            MessageBoxW(pm.getLastError().c_str(), L"Error", MB_OK | MB_ICONERROR);
    }
    return 0;
}

inline LRESULT ExportToolbar::OnSaveAs(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (m_app)
    {
        ProjectManager& pm = m_app->getProjectManager();
        if (pm.saveProjectAs(m_hWnd))
        {
            MessageBoxW(L"Project saved successfully!", L"Save As", MB_OK | MB_ICONINFORMATION);
            m_app->updateWindowTitle();
        }
        else if (!pm.getLastError().empty())
        {
            MessageBoxW(pm.getLastError().c_str(), L"Error", MB_OK | MB_ICONERROR);
        }
    }
    return 0;
}
