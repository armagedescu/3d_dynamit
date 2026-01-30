#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

class DesignerApp;

// Control IDs
#define ID_BTN_EXPORT_CLIP      2001
#define ID_BTN_EXPORT_FILE      2002
#define ID_CHK_INCLUDE_DYNAMIT  2003
#define ID_CHK_COMPLETE_APP     2004
#define ID_CHK_INCLUDE_NORMALS  2005

class ExportToolbar
{
public:
    ExportToolbar(DesignerApp* app) : m_app(app), m_hwnd(nullptr) {}
    ~ExportToolbar() { if (m_hwnd) DestroyWindow(m_hwnd); }

    HWND Create(HWND parent)
    {
        // Register window class
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
        wc.lpszClassName = L"ExportToolbarClass";
        RegisterClassExW(&wc);

        // Create window
        m_hwnd = CreateWindowExW(
            WS_EX_TOOLWINDOW,
            L"ExportToolbarClass",
            L"Export",
            WS_POPUP | WS_CAPTION | WS_VISIBLE,
            0, 0, 280, 180,
            parent, nullptr, GetModuleHandle(nullptr), this);

        if (m_hwnd)
        {
            createControls();
        }

        return m_hwnd;
    }

    HWND GetHwnd() const { return m_hwnd; }

private:
    void createControls()
    {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        // Export Options label
        HWND lblOptions = CreateWindowW(L"STATIC", L"Export Options:",
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 10, 100, 20, m_hwnd, nullptr,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(lblOptions, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Include Dynamit Setup checkbox
        HWND chkDynamit = CreateWindowW(L"BUTTON", L"Include Dynamit Renderer Setup",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, 35, 250, 20, m_hwnd, (HMENU)ID_CHK_INCLUDE_DYNAMIT,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(chkDynamit, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(chkDynamit, BM_SETCHECK, BST_CHECKED, 0); // Default checked

        // Generate Complete App checkbox
        HWND chkApp = CreateWindowW(L"BUTTON", L"Generate Complete Standalone App",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, 60, 250, 20, m_hwnd, (HMENU)ID_CHK_COMPLETE_APP,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(chkApp, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Include Normals Highlighter checkbox
        HWND chkNormals = CreateWindowW(L"BUTTON", L"Include Normals Highlighter",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, 85, 250, 20, m_hwnd, (HMENU)ID_CHK_INCLUDE_NORMALS,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(chkNormals, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Copy Code button
        HWND btnExportClip = CreateWindowW(L"BUTTON", L"Copy Code",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 115, 120, 30, m_hwnd, (HMENU)ID_BTN_EXPORT_CLIP,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(btnExportClip, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Save Code button
        HWND btnExportFile = CreateWindowW(L"BUTTON", L"Save Code...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            140, 115, 120, 30, m_hwnd, (HMENU)ID_BTN_EXPORT_FILE,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(btnExportFile, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        ExportToolbar* pThis = nullptr;

        if (msg == WM_CREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (ExportToolbar*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        }
        else
        {
            pThis = (ExportToolbar*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }

        if (pThis)
        {
            return pThis->handleMessage(hwnd, msg, wParam, lParam);
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    DesignerApp* m_app;
    HWND m_hwnd;
};

// Include implementation inline to avoid separate cpp
#include "../DesignerApp.h"
#include "../ShapeManager.h"
#include "../CodeExporter.h"

inline LRESULT ExportToolbar::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_BTN_EXPORT_CLIP:
            if (m_app)
            {
                ShapeConfig* config = m_app->getSelectedShapeConfig();
                if (config)
                {
                    // Get checkbox states
                    bool includeDynamit = (SendMessage(GetDlgItem(hwnd, ID_CHK_INCLUDE_DYNAMIT), BM_GETCHECK, 0, 0) == BST_CHECKED);
                    bool completeApp = (SendMessage(GetDlgItem(hwnd, ID_CHK_COMPLETE_APP), BM_GETCHECK, 0, 0) == BST_CHECKED);
                    bool includeNormals = (SendMessage(GetDlgItem(hwnd, ID_CHK_INCLUDE_NORMALS), BM_GETCHECK, 0, 0) == BST_CHECKED);

                    // Determine export mode
                    CodeExporter::ExportMode mode;
                    if (completeApp)
                        mode = CodeExporter::ExportMode::StandaloneApplication;
                    else if (includeDynamit)
                        mode = CodeExporter::ExportMode::WithDynamitSetup;
                    else
                        mode = CodeExporter::ExportMode::GeometryOnly;

                    std::wstring code = CodeExporter::generateCppCode(*config, mode, includeNormals);
                    if (CodeExporter::copyToClipboard(code))
                    {
                        MessageBoxW(hwnd, L"C++ code copied to clipboard!", L"Export", MB_OK | MB_ICONINFORMATION);
                    }
                    else
                    {
                        MessageBoxW(hwnd, L"Failed to copy to clipboard", L"Error", MB_OK | MB_ICONERROR);
                    }
                }
                else
                {
                    MessageBoxW(hwnd, L"No shape selected", L"Export", MB_OK | MB_ICONWARNING);
                }
            }
            return 0;

        case ID_BTN_EXPORT_FILE:
            if (m_app)
            {
                ShapeConfig* config = m_app->getSelectedShapeConfig();
                if (config)
                {
                    // Get checkbox states
                    bool includeDynamit = (SendMessage(GetDlgItem(hwnd, ID_CHK_INCLUDE_DYNAMIT), BM_GETCHECK, 0, 0) == BST_CHECKED);
                    bool completeApp = (SendMessage(GetDlgItem(hwnd, ID_CHK_COMPLETE_APP), BM_GETCHECK, 0, 0) == BST_CHECKED);
                    bool includeNormals = (SendMessage(GetDlgItem(hwnd, ID_CHK_INCLUDE_NORMALS), BM_GETCHECK, 0, 0) == BST_CHECKED);

                    // Determine export mode
                    CodeExporter::ExportMode mode;
                    if (completeApp)
                        mode = CodeExporter::ExportMode::StandaloneApplication;
                    else if (includeDynamit)
                        mode = CodeExporter::ExportMode::WithDynamitSetup;
                    else
                        mode = CodeExporter::ExportMode::GeometryOnly;

                    std::wstring code = CodeExporter::generateCppCode(*config, mode, includeNormals);
                    if (CodeExporter::saveToFile(hwnd, code))
                    {
                        MessageBoxW(hwnd, L"C++ code saved successfully!", L"Export", MB_OK | MB_ICONINFORMATION);
                    }
                }
                else
                {
                    MessageBoxW(hwnd, L"No shape selected", L"Export", MB_OK | MB_ICONWARNING);
                }
            }
            return 0;
        }
        break;

    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
