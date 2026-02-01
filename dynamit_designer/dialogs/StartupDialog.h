#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <string>

// Control IDs
#define ID_BTN_NEW_PROJECT  2001
#define ID_BTN_OPEN_PROJECT 2002
#define ID_BTN_EXIT         2003

// Result of startup dialog
struct StartupResult
{
    bool success = false;
    bool isNewProject = false;
    std::wstring projectPath;      // Full path to .dproj file
    std::wstring projectDirectory; // Directory containing the project
    std::wstring projectName;      // Just the project name
};

class StartupDialog
{
public:
    StartupDialog() : m_hwnd(nullptr), m_result{} {}
    ~StartupDialog() { if (m_hwnd) DestroyWindow(m_hwnd); }

    // Show modal dialog, returns result
    StartupResult Show(HINSTANCE hInstance)
    {
        m_hInstance = hInstance;
        m_result = {};

        // Register window class
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance;
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
        wc.lpszClassName = L"StartupDialogClass";
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        RegisterClassExW(&wc);

        // Calculate center position
        int screenW = GetSystemMetrics(SM_CXSCREEN);
        int screenH = GetSystemMetrics(SM_CYSCREEN);
        int dlgW = 400;
        int dlgH = 200;
        int x = (screenW - dlgW) / 2;
        int y = (screenH - dlgH) / 2;

        // Create dialog window
        m_hwnd = CreateWindowExW(
            WS_EX_DLGMODALFRAME,
            L"StartupDialogClass",
            L"Dynamit Designer - Welcome",
            WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
            x, y, dlgW, dlgH,
            nullptr, nullptr, hInstance, this);

        if (!m_hwnd)
            return m_result;

        createControls();

        // Modal message loop
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            if (!IsWindow(m_hwnd))
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return m_result;
    }

private:
    void createControls()
    {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

        // Title label
        HWND lblTitle = CreateWindowW(L"STATIC",
            L"Welcome to Dynamit Designer",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            20, 20, 360, 25, m_hwnd, nullptr,
            m_hInstance, nullptr);

        // Create bold font for title
        LOGFONTW lf = {};
        GetObjectW(hFont, sizeof(lf), &lf);
        lf.lfWeight = FW_BOLD;
        lf.lfHeight = -16;
        HFONT hBoldFont = CreateFontIndirectW(&lf);
        SendMessage(lblTitle, WM_SETFONT, (WPARAM)hBoldFont, TRUE);

        // Subtitle
        HWND lblSub = CreateWindowW(L"STATIC",
            L"Create a new project or open an existing one to continue.",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            20, 50, 360, 20, m_hwnd, nullptr,
            m_hInstance, nullptr);
        SendMessage(lblSub, WM_SETFONT, (WPARAM)hFont, TRUE);

        // New Project button
        HWND btnNew = CreateWindowW(L"BUTTON", L"New Project...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            50, 100, 130, 35, m_hwnd, (HMENU)ID_BTN_NEW_PROJECT,
            m_hInstance, nullptr);
        SendMessage(btnNew, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Open Project button
        HWND btnOpen = CreateWindowW(L"BUTTON", L"Open Project...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            220, 100, 130, 35, m_hwnd, (HMENU)ID_BTN_OPEN_PROJECT,
            m_hInstance, nullptr);
        SendMessage(btnOpen, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Exit button
        HWND btnExit = CreateWindowW(L"BUTTON", L"Exit",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            160, 150, 80, 25, m_hwnd, (HMENU)ID_BTN_EXIT,
            m_hInstance, nullptr);
        SendMessage(btnExit, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    bool onNewProject()
    {
        wchar_t fileName[MAX_PATH] = L"NewProject";

        OPENFILENAMEW ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_hwnd;
        ofn.lpstrFilter = L"Dynamit Project\0*\0";
        ofn.lpstrFile = fileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_PATHMUSTEXIST;
        ofn.lpstrTitle = L"Create New Project";

        if (!GetSaveFileNameW(&ofn))
            return false;

        std::wstring projectDir = fileName;

        // Extract project name from path
        size_t lastSlash = projectDir.find_last_of(L"\\/");
        if (lastSlash == std::wstring::npos)
            return false;

        std::wstring projectName = projectDir.substr(lastSlash + 1);

        // Remove any extension the user might have typed
        size_t dotPos = projectName.find_last_of(L'.');
        if (dotPos != std::wstring::npos)
            projectName = projectName.substr(0, dotPos);

        // Create project directory
        if (!CreateDirectoryW(projectDir.c_str(), nullptr))
        {
            DWORD err = GetLastError();
            if (err == ERROR_ALREADY_EXISTS)
            {
                int result = MessageBoxW(m_hwnd,
                    L"A folder with this name already exists. Use it anyway?",
                    L"Folder Exists",
                    MB_YESNO | MB_ICONQUESTION);
                if (result != IDYES)
                    return false;
            }
            else
            {
                MessageBoxW(m_hwnd, L"Failed to create project folder.",
                    L"Error", MB_OK | MB_ICONERROR);
                return false;
            }
        }

        // Set result
        m_result.success = true;
        m_result.isNewProject = true;
        m_result.projectName = projectName;
        m_result.projectDirectory = projectDir;
        m_result.projectPath = projectDir + L"\\" + projectName + L".dproj";

        return true;
    }

    bool onOpenProject()
    {
        wchar_t fileName[MAX_PATH] = L"";

        OPENFILENAMEW ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_hwnd;
        ofn.lpstrFilter = L"Dynamit Project (*.dproj)\0*.dproj\0All Files (*.*)\0*.*\0";
        ofn.lpstrFile = fileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        ofn.lpstrTitle = L"Open Dynamit Project";

        if (!GetOpenFileNameW(&ofn))
            return false;

        // Extract project info from path
        std::wstring fullPath = fileName;

        // Get directory
        size_t lastSlash = fullPath.find_last_of(L"\\/");
        if (lastSlash == std::wstring::npos)
            return false;

        std::wstring dir = fullPath.substr(0, lastSlash);
        std::wstring file = fullPath.substr(lastSlash + 1);

        // Get name (remove .dproj extension)
        std::wstring name = file;
        size_t dotPos = name.find_last_of(L'.');
        if (dotPos != std::wstring::npos)
            name = name.substr(0, dotPos);

        m_result.success = true;
        m_result.isNewProject = false;
        m_result.projectName = name;
        m_result.projectDirectory = dir;
        m_result.projectPath = fullPath;

        return true;
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        StartupDialog* pThis = nullptr;

        if (msg == WM_CREATE)
        {
            CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
            pThis = (StartupDialog*)pCreate->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        }
        else
        {
            pThis = (StartupDialog*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        }

        if (pThis)
        {
            return pThis->handleMessage(hwnd, msg, wParam, lParam);
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case ID_BTN_NEW_PROJECT:
                if (onNewProject())
                {
                    DestroyWindow(hwnd);
                }
                return 0;

            case ID_BTN_OPEN_PROJECT:
                if (onOpenProject())
                {
                    DestroyWindow(hwnd);
                }
                return 0;

            case ID_BTN_EXIT:
                m_result.success = false;
                DestroyWindow(hwnd);
                return 0;
            }
            break;

        case WM_CLOSE:
            m_result.success = false;
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            m_hwnd = nullptr;
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    HINSTANCE m_hInstance;
    HWND m_hwnd;
    StartupResult m_result;
};
