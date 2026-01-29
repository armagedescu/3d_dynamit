#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

class DesignerApp;

// Control IDs
#define ID_BTN_NEW_CONE     1001
#define ID_BTN_NEW_CYLINDER 1002
#define ID_BTN_DELETE       1003
#define ID_BTN_EXPORT_CLIP  1004
#define ID_BTN_EXPORT_FILE  1005
#define ID_CHK_SHOW_NORMALS 1006

class MainToolbar
{
public:
    MainToolbar(DesignerApp* app) : m_app(app), m_hwnd(nullptr) {}
    ~MainToolbar() { if (m_hwnd) DestroyWindow(m_hwnd); }

    HWND Create(HWND parent)
    {
        // Register window class
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
        wc.lpszClassName = L"MainToolbarClass";
        RegisterClassExW(&wc);

        // Create window
        m_hwnd = CreateWindowExW(
            WS_EX_TOOLWINDOW,
            L"MainToolbarClass",
            L"Shapes",
            WS_POPUP | WS_CAPTION | WS_VISIBLE,
            0, 0, 280, 160,
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

        // New Cone button
        HWND btnCone = CreateWindowW(L"BUTTON", L"New Cone",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 5, 80, 25, m_hwnd, (HMENU)ID_BTN_NEW_CONE,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(btnCone, WM_SETFONT, (WPARAM)hFont, TRUE);

        // New Cylinder button
        HWND btnCyl = CreateWindowW(L"BUTTON", L"New Cylinder",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            100, 5, 90, 25, m_hwnd, (HMENU)ID_BTN_NEW_CYLINDER,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(btnCyl, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Delete button
        HWND btnDel = CreateWindowW(L"BUTTON", L"Delete",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            200, 5, 65, 25, m_hwnd, (HMENU)ID_BTN_DELETE,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(btnDel, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Export to Clipboard button
        HWND btnExportClip = CreateWindowW(L"BUTTON", L"Copy Code",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 35, 85, 25, m_hwnd, (HMENU)ID_BTN_EXPORT_CLIP,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(btnExportClip, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Export to File button
        HWND btnExportFile = CreateWindowW(L"BUTTON", L"Save Code...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            100, 35, 90, 25, m_hwnd, (HMENU)ID_BTN_EXPORT_FILE,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(btnExportFile, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Show Normals checkbox
        HWND chkNormals = CreateWindowW(L"BUTTON", L"Show Normals",
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, 65, 120, 20, m_hwnd, (HMENU)ID_CHK_SHOW_NORMALS,
            GetModuleHandle(nullptr), nullptr);
        SendMessage(chkNormals, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        MainToolbar* pThis = nullptr;

        if (msg == WM_CREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (MainToolbar*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        }
        else
        {
            pThis = (MainToolbar*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
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

inline LRESULT MainToolbar::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_BTN_NEW_CONE:
            if (m_app) m_app->newShape(ShapeConfig::Type::Cone);
            return 0;

        case ID_BTN_NEW_CYLINDER:
            if (m_app) m_app->newShape(ShapeConfig::Type::Cylinder);
            return 0;

        case ID_BTN_DELETE:
            if (m_app) m_app->deleteSelectedShape();
            return 0;

        case ID_BTN_EXPORT_CLIP:
            if (m_app)
            {
                ShapeConfig* config = m_app->getSelectedShapeConfig();
                if (config)
                {
                    std::wstring code = CodeExporter::generateCppCode(*config);
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
                    std::wstring code = CodeExporter::generateCppCode(*config);
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

        case ID_CHK_SHOW_NORMALS:
            if (m_app)
            {
                HWND chk = GetDlgItem(hwnd, ID_CHK_SHOW_NORMALS);
                bool checked = (SendMessage(chk, BM_GETCHECK, 0, 0) == BST_CHECKED);
                m_app->setShowNormals(checked);
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
