#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include <atlbase.h>
#include <atlwin.h>

#pragma comment(lib, "uxtheme.lib")

// Enable visual styles (modern look)
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Control IDs
#define ID_BTN_NEW_PROJECT  2001
#define ID_BTN_OPEN_PROJECT 2002
#define ID_LIST_RECENT      2004

// Result of startup dialog
struct StartupResult
{
    bool success = false;
    bool isNewProject = false;
    std::wstring projectPath;      // Full path to .dproj file
    std::wstring projectDirectory; // Directory containing the project
    std::wstring projectName;      // Just the project name
};

// Recent project entry
struct RecentProject
{
    std::wstring name;
    std::wstring path;
};

class StartupDialog : public CWindowImpl<StartupDialog>
{
public:
    DECLARE_WND_CLASS(L"StartupDialogClass")

    StartupDialog() : m_hListRecent(nullptr), m_result{} {}

    // Show modal dialog, returns result
    StartupResult Show(HINSTANCE hInstance)
    {
        m_hInstance = hInstance;
        m_result = {};

        // Load recent projects
        loadRecentProjects();

        // Desired client area size
        int clientW = 480;
        int clientH = 380;

        // Calculate window size to get desired client area
        RECT rc = { 0, 0, clientW, clientH };
        DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU;
        DWORD exStyle = WS_EX_DLGMODALFRAME;
        AdjustWindowRectEx(&rc, style, FALSE, exStyle);
        int dlgW = rc.right - rc.left;
        int dlgH = rc.bottom - rc.top;

        // Center on screen
        int screenW = GetSystemMetrics(SM_CXSCREEN);
        int screenH = GetSystemMetrics(SM_CYSCREEN);
        int x = (screenW - dlgW) / 2;
        int y = (screenH - dlgH) / 2;

        // Create dialog window using ATL
        RECT createRect = { x, y, x + dlgW, y + dlgH };
        Create(nullptr, createRect, L"Dynamit Designer", style, exStyle);

        if (!m_hWnd)
            return m_result;

        createControls();
        ShowWindow(SW_SHOW);

        // Modal message loop
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            if (!IsWindow())
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return m_result;
    }

    // Add a project to recent list (call after successful open/save)
    static void AddToRecentProjects(const std::wstring& path, const std::wstring& name)
    {
        std::vector<RecentProject> recent;
        loadRecentProjectsStatic(recent);

        // Remove if already exists
        recent.erase(
            std::remove_if(recent.begin(), recent.end(),
                [&path](const RecentProject& p) { return p.path == path; }),
            recent.end());

        // Add to front
        RecentProject proj;
        proj.name = name;
        proj.path = path;
        recent.insert(recent.begin(), proj);

        // Keep max 10
        if (recent.size() > 10)
            recent.resize(10);

        saveRecentProjectsStatic(recent);
    }

    // ATL message map
    BEGIN_MSG_MAP(StartupDialog)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
        COMMAND_ID_HANDLER(ID_BTN_NEW_PROJECT, OnNewProjectClick)
        COMMAND_ID_HANDLER(ID_BTN_OPEN_PROJECT, OnOpenProjectClick)
        COMMAND_HANDLER(ID_LIST_RECENT, LBN_DBLCLK, OnRecentDblClick)
    END_MSG_MAP()

private:
    // Message handlers
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        m_result.success = false;
        DestroyWindow();
        return 0;
    }

    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        PostQuitMessage(0);
        return 0;
    }

    LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if (wParam == VK_ESCAPE)
        {
            m_result.success = false;
            DestroyWindow();
            return 0;
        }
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnNewProjectClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        if (onNewProject())
        {
            DestroyWindow();
        }
        return 0;
    }

    LRESULT OnOpenProjectClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        if (onOpenProject())
        {
            DestroyWindow();
        }
        return 0;
    }

    LRESULT OnRecentDblClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        if (onRecentProjectDoubleClick())
        {
            DestroyWindow();
        }
        return 0;
    }

    void createControls()
    {
        // Get client area dimensions
        RECT clientRect;
        GetClientRect(&clientRect);
        int clientW = clientRect.right;
        int clientH = clientRect.bottom;

        // Margins and spacing
        const int margin = 20;
        const int buttonH = 40;
        const int spacing = 15;

        // Create fonts
        NONCLIENTMETRICSW ncm = { sizeof(NONCLIENTMETRICSW) };
        SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        HFONT hFont = CreateFontIndirectW(&ncm.lfMessageFont);

        LOGFONTW lfTitle = ncm.lfMessageFont;
        lfTitle.lfWeight = FW_SEMIBOLD;
        lfTitle.lfHeight = -18;
        HFONT hTitleFont = CreateFontIndirectW(&lfTitle);

        LOGFONTW lfButton = ncm.lfMessageFont;
        lfButton.lfHeight = -14;
        HFONT hButtonFont = CreateFontIndirectW(&lfButton);

        int y = margin;

        // Title label
        HWND lblTitle = CreateWindowW(L"STATIC",
            L"Welcome to Dynamit Designer",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            margin, y, clientW - 2 * margin, 28, m_hWnd, nullptr,
            m_hInstance, nullptr);
        ::SendMessage(lblTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
        y += 28 + spacing;

        // Buttons row
        int buttonW = (clientW - 2 * margin - spacing) / 2;

        HWND btnNew = CreateWindowW(L"BUTTON", L"New Project...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            margin, y, buttonW, buttonH, m_hWnd, (HMENU)ID_BTN_NEW_PROJECT,
            m_hInstance, nullptr);
        ::SendMessage(btnNew, WM_SETFONT, (WPARAM)hButtonFont, TRUE);

        HWND btnOpen = CreateWindowW(L"BUTTON", L"Open Existing Project...",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            margin + buttonW + spacing, y, buttonW, buttonH, m_hWnd, (HMENU)ID_BTN_OPEN_PROJECT,
            m_hInstance, nullptr);
        ::SendMessage(btnOpen, WM_SETFONT, (WPARAM)hButtonFont, TRUE);
        y += buttonH + spacing + 5;

        // Recent Projects label
        HWND lblRecent = CreateWindowW(L"STATIC",
            L"Recent Projects:",
            WS_CHILD | WS_VISIBLE,
            margin, y, 150, 20, m_hWnd, nullptr,
            m_hInstance, nullptr);
        ::SendMessage(lblRecent, WM_SETFONT, (WPARAM)hFont, TRUE);
        y += 22;

        // Calculate listbox height (fill remaining space minus hint)
        int hintH = 20;
        int listH = clientH - y - margin - hintH - spacing;

        // Recent Projects listbox
        m_hListRecent = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"LISTBOX",
            nullptr,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
            margin, y, clientW - 2 * margin, listH, m_hWnd, (HMENU)ID_LIST_RECENT,
            m_hInstance, nullptr);
        ::SendMessage(m_hListRecent, WM_SETFONT, (WPARAM)hFont, TRUE);
        y += listH + spacing;

        // Populate recent projects list
        if (m_recentProjects.empty())
        {
            ::SendMessageW(m_hListRecent, LB_ADDSTRING, 0, (LPARAM)L"(No recent projects)");
            ::EnableWindow(m_hListRecent, FALSE);
        }
        else
        {
            for (const auto& proj : m_recentProjects)
            {
                std::wstring display = proj.name + L"  -  " + proj.path;
                ::SendMessageW(m_hListRecent, LB_ADDSTRING, 0, (LPARAM)display.c_str());
            }
        }

        // Hint at bottom
        HWND lblHint = CreateWindowW(L"STATIC",
            L"Double-click a recent project to open it, or press Escape to exit.",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            margin, y, clientW - 2 * margin, hintH, m_hWnd, nullptr,
            m_hInstance, nullptr);
        ::SendMessage(lblHint, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    bool onNewProject()
    {
        wchar_t fileName[MAX_PATH] = L"NewProject";

        OPENFILENAMEW ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_hWnd;
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
                int result = MessageBoxW(
                    L"A folder with this name already exists. Use it anyway?",
                    L"Folder Exists",
                    MB_YESNO | MB_ICONQUESTION);
                if (result != IDYES)
                    return false;
            }
            else
            {
                MessageBoxW(L"Failed to create project folder.",
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
        ofn.hwndOwner = m_hWnd;
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

    bool onRecentProjectDoubleClick()
    {
        int sel = (int)::SendMessageW(m_hListRecent, LB_GETCURSEL, 0, 0);
        if (sel == LB_ERR || sel < 0 || sel >= (int)m_recentProjects.size())
            return false;

        const RecentProject& proj = m_recentProjects[sel];

        // Verify file exists
        if (GetFileAttributesW(proj.path.c_str()) == INVALID_FILE_ATTRIBUTES)
        {
            MessageBoxW(L"This project file no longer exists.",
                L"File Not Found", MB_OK | MB_ICONWARNING);
            return false;
        }

        // Extract directory from path
        std::wstring dir = proj.path;
        size_t lastSlash = dir.find_last_of(L"\\/");
        if (lastSlash != std::wstring::npos)
            dir = dir.substr(0, lastSlash);

        m_result.success = true;
        m_result.isNewProject = false;
        m_result.projectName = proj.name;
        m_result.projectPath = proj.path;
        m_result.projectDirectory = dir;

        return true;
    }

    void loadRecentProjects()
    {
        loadRecentProjectsStatic(m_recentProjects);
    }

    static std::wstring getRecentProjectsFilePath()
    {
        wchar_t appData[MAX_PATH];
        DWORD len = GetEnvironmentVariableW(L"APPDATA", appData, MAX_PATH);
        if (len > 0 && len < MAX_PATH)
        {
            std::wstring dir = std::wstring(appData) + L"\\DynamitDesigner";
            CreateDirectoryW(dir.c_str(), nullptr);
            return dir + L"\\recent.txt";
        }
        return L"recent.txt";
    }

    static void loadRecentProjectsStatic(std::vector<RecentProject>& projects)
    {
        projects.clear();
        std::wstring path = getRecentProjectsFilePath();

        std::wifstream file(path);
        if (!file.is_open())
            return;

        std::wstring line;
        while (std::getline(file, line) && projects.size() < 10)
        {
            if (line.empty())
                continue;

            // Format: name|path
            size_t sep = line.find(L'|');
            if (sep == std::wstring::npos)
                continue;

            RecentProject proj;
            proj.name = line.substr(0, sep);
            proj.path = line.substr(sep + 1);

            // Only add if file exists
            if (GetFileAttributesW(proj.path.c_str()) != INVALID_FILE_ATTRIBUTES)
            {
                projects.push_back(proj);
            }
        }
    }

    static void saveRecentProjectsStatic(const std::vector<RecentProject>& projects)
    {
        std::wstring path = getRecentProjectsFilePath();

        std::wofstream file(path);
        if (!file.is_open())
            return;

        for (const auto& proj : projects)
        {
            file << proj.name << L"|" << proj.path << L"\n";
        }
    }

    HINSTANCE m_hInstance;
    HWND m_hListRecent;
    StartupResult m_result;
    std::vector<RecentProject> m_recentProjects;
};
