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

#include "Theme.h"

#pragma comment(lib, "uxtheme.lib")

// Enable visual styles (modern look)
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Control IDs
#define ID_BTN_NEW_PROJECT  2001
#define ID_BTN_OPEN_PROJECT 2002
#define ID_LIST_RECENT      2004
#define ID_BTN_THEME_LIGHT  2010
#define ID_BTN_THEME_DARK   2011
#define ID_BTN_THEME_AUTO   2012

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

    StartupDialog() {}

    // Show modal dialog, returns result
    StartupResult Show(HINSTANCE hInstance)
    {
        m_hInstance = hInstance;
        m_result = {};

        // Load recent projects
        loadRecentProjects();

        // Desired client area size
        int clientW = 520;
        int clientH = 400;

        // Calculate window size to get desired client area
        RECT rc = { 0, 0, clientW, clientH };
        DWORD style = WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;
        DWORD exStyle = 0;
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
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
        COMMAND_ID_HANDLER(ID_BTN_NEW_PROJECT, OnNewProjectClick)
        COMMAND_ID_HANDLER(ID_BTN_OPEN_PROJECT, OnOpenProjectClick)
        COMMAND_ID_HANDLER(ID_BTN_THEME_LIGHT, OnThemeClick)
        COMMAND_ID_HANDLER(ID_BTN_THEME_DARK, OnThemeClick)
        COMMAND_ID_HANDLER(ID_BTN_THEME_AUTO, OnThemeClick)
        NOTIFY_HANDLER(ID_LIST_RECENT, NM_DBLCLK, OnRecentDblClick)
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

    LRESULT OnRecentDblClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
    {
        if (onRecentProjectDoubleClick())
        {
            DestroyWindow();
        }
        return 0;
    }

    LRESULT OnSize(UINT, WPARAM wParam, LPARAM, BOOL&)
    {
        if (wParam != SIZE_MINIMIZED && m_hListRecent)
            repositionControls();
        return 0;
    }

    LRESULT OnDrawItem(UINT, WPARAM, LPARAM lParam, BOOL& bHandled)
    {
        DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
        if (dis->CtlID == ID_BTN_THEME_LIGHT)
            Theme::drawThemeButton(dis, ThemeMode::Light);
        else if (dis->CtlID == ID_BTN_THEME_DARK)
            Theme::drawThemeButton(dis, ThemeMode::Dark);
        else if (dis->CtlID == ID_BTN_THEME_AUTO)
            Theme::drawThemeButton(dis, ThemeMode::Auto);
        else if (dis->CtlID == ID_BTN_NEW_PROJECT || dis->CtlID == ID_BTN_OPEN_PROJECT)
            Theme::drawButton(dis);
        else
            bHandled = FALSE;
        return TRUE;
    }

    LRESULT OnCtlColorStatic(UINT, WPARAM wParam, LPARAM, BOOL&)
    {
        return (LRESULT)Theme::instance().onCtlColorStatic((HDC)wParam);
    }

    LRESULT OnEraseBkgnd(UINT, WPARAM wParam, LPARAM, BOOL&)
    {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(&rc);
        FillRect(hdc, &rc, Theme::instance().backgroundBrush());
        return TRUE;
    }

    LRESULT OnThemeClick(WORD, WORD wID, HWND, BOOL&)
    {
        ThemeMode mode = ThemeMode::Auto;
        if (wID == ID_BTN_THEME_LIGHT) mode = ThemeMode::Light;
        else if (wID == ID_BTN_THEME_DARK) mode = ThemeMode::Dark;

        Theme::instance().setMode(mode);
        applyTheme();
        return 0;
    }

    void applyTheme()
    {
        Theme::applyTitleBar(m_hWnd, Theme::instance().isDark());
        Theme::applyToListView(m_hListRecent);

        // Repaint everything
        InvalidateRect(nullptr, TRUE);
        // Force repaint of theme buttons
        if (m_hBtnThemeLight) ::InvalidateRect(m_hBtnThemeLight, nullptr, TRUE);
        if (m_hBtnThemeDark) ::InvalidateRect(m_hBtnThemeDark, nullptr, TRUE);
        if (m_hBtnThemeAuto) ::InvalidateRect(m_hBtnThemeAuto, nullptr, TRUE);
        if (m_hBtnNew) ::InvalidateRect(m_hBtnNew, nullptr, TRUE);
        if (m_hBtnOpen) ::InvalidateRect(m_hBtnOpen, nullptr, TRUE);
    }

    void createControls()
    {
        // Create font
        NONCLIENTMETRICSW ncm = { sizeof(NONCLIENTMETRICSW) };
        SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
        m_hFont = CreateFontIndirectW(&ncm.lfMessageFont);

        // Theme chooser buttons
        m_hBtnThemeLight = CreateWindowW(L"BUTTON", L"",
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            0, 0, 0, 0, m_hWnd, (HMENU)ID_BTN_THEME_LIGHT,
            m_hInstance, nullptr);
        m_hBtnThemeDark = CreateWindowW(L"BUTTON", L"",
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            0, 0, 0, 0, m_hWnd, (HMENU)ID_BTN_THEME_DARK,
            m_hInstance, nullptr);
        m_hBtnThemeAuto = CreateWindowW(L"BUTTON", L"",
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            0, 0, 0, 0, m_hWnd, (HMENU)ID_BTN_THEME_AUTO,
            m_hInstance, nullptr);

        // Project buttons (owner-draw for theme support)
        m_hBtnNew = CreateWindowW(L"BUTTON", L"New Project...",
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            0, 0, 0, 0, m_hWnd, (HMENU)ID_BTN_NEW_PROJECT,
            m_hInstance, nullptr);
        ::SendMessage(m_hBtnNew, WM_SETFONT, (WPARAM)m_hFont, TRUE);

        m_hBtnOpen = CreateWindowW(L"BUTTON", L"Open Project...",
            WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
            0, 0, 0, 0, m_hWnd, (HMENU)ID_BTN_OPEN_PROJECT,
            m_hInstance, nullptr);
        ::SendMessage(m_hBtnOpen, WM_SETFONT, (WPARAM)m_hFont, TRUE);

        // Recent Projects label
        m_hLblRecent = CreateWindowW(L"STATIC",
            L"Recent Projects:",
            WS_CHILD | WS_VISIBLE,
            0, 0, 0, 0, m_hWnd, nullptr,
            m_hInstance, nullptr);
        ::SendMessage(m_hLblRecent, WM_SETFONT, (WPARAM)m_hFont, TRUE);

        // Recent Projects list view (report mode with columns)
        m_hListRecent = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            WC_LISTVIEWW,
            nullptr,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER,
            0, 0, 0, 0, m_hWnd, (HMENU)ID_LIST_RECENT,
            m_hInstance, nullptr);
        ::SendMessage(m_hListRecent, WM_SETFONT, (WPARAM)m_hFont, TRUE);

        // Full-row select, no gridlines
        ListView_SetExtendedListViewStyle(m_hListRecent, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

        // Add columns
        LVCOLUMNW col = {};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
        col.fmt = LVCFMT_LEFT;

        col.pszText = const_cast<LPWSTR>(L"Name");
        col.cx = 140;
        ListView_InsertColumn(m_hListRecent, 0, &col);

        col.pszText = const_cast<LPWSTR>(L"Path");
        col.cx = 340;
        ListView_InsertColumn(m_hListRecent, 1, &col);

        // Populate
        populateRecentList();

        // Position everything
        repositionControls();

        // Apply theme
        applyTheme();
    }

    void repositionControls()
    {
        RECT rc;
        GetClientRect(&rc);
        int clientW = rc.right;
        int clientH = rc.bottom;

        const int margin = 10;
        const int buttonH = 26;
        const int buttonW = 100;
        const int spacing = 8;

        const int themeBtnSize = 20;

        int y = margin;

        // Theme buttons on their own row, top-right
        int themeX = clientW - margin - 3 * (themeBtnSize + 2) + 2;
        ::SetWindowPos(m_hBtnThemeLight, nullptr, themeX, y, themeBtnSize, themeBtnSize, SWP_NOZORDER);
        ::SetWindowPos(m_hBtnThemeDark, nullptr, themeX + themeBtnSize + 2, y, themeBtnSize, themeBtnSize, SWP_NOZORDER);
        ::SetWindowPos(m_hBtnThemeAuto, nullptr, themeX + 2 * (themeBtnSize + 2), y, themeBtnSize, themeBtnSize, SWP_NOZORDER);
        y += themeBtnSize + spacing;

        // Recent label on the left, project buttons on the right
        ::SetWindowPos(m_hLblRecent, nullptr, margin, y + 4, 150, 18, SWP_NOZORDER);

        int btnX = clientW - margin - buttonW;
        ::SetWindowPos(m_hBtnOpen, nullptr, btnX, y, buttonW + 10, buttonH, SWP_NOZORDER);
        btnX -= buttonW + spacing;
        ::SetWindowPos(m_hBtnNew, nullptr, btnX, y, buttonW, buttonH, SWP_NOZORDER);
        y += buttonH + spacing;

        // List fills remaining space
        int listH = clientH - y - margin;
        if (listH < 50) listH = 50;
        ::SetWindowPos(m_hListRecent, nullptr, margin, y, clientW - 2 * margin, listH, SWP_NOZORDER);

        // Resize path column to fill available width
        int nameColW = ListView_GetColumnWidth(m_hListRecent, 0);
        int pathColW = clientW - 2 * margin - nameColW - 4;
        if (pathColW < 100) pathColW = 100;
        ListView_SetColumnWidth(m_hListRecent, 1, pathColW);
    }

    void populateRecentList()
    {
        ListView_DeleteAllItems(m_hListRecent);

        if (m_recentProjects.empty())
        {
            LVITEMW item = {};
            item.mask = LVIF_TEXT;
            item.iItem = 0;
            item.pszText = const_cast<LPWSTR>(L"(No recent projects)");
            ListView_InsertItem(m_hListRecent, &item);
            ::EnableWindow(m_hListRecent, FALSE);
        }
        else
        {
            for (int i = 0; i < (int)m_recentProjects.size(); i++)
            {
                LVITEMW item = {};
                item.mask = LVIF_TEXT;
                item.iItem = i;
                item.pszText = const_cast<LPWSTR>(m_recentProjects[i].name.c_str());
                ListView_InsertItem(m_hListRecent, &item);

                ListView_SetItemText(m_hListRecent, i, 1,
                    const_cast<LPWSTR>(m_recentProjects[i].path.c_str()));
            }
        }
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
        int sel = ListView_GetNextItem(m_hListRecent, -1, LVNI_SELECTED);
        if (sel < 0 || sel >= (int)m_recentProjects.size())
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

    HINSTANCE m_hInstance = nullptr;
    HWND m_hListRecent = nullptr;
    HWND m_hBtnNew = nullptr;
    HWND m_hBtnOpen = nullptr;
    HWND m_hLblRecent = nullptr;
    HWND m_hBtnThemeLight = nullptr;
    HWND m_hBtnThemeDark = nullptr;
    HWND m_hBtnThemeAuto = nullptr;
    HFONT m_hFont = nullptr;
    StartupResult m_result;
    std::vector<RecentProject> m_recentProjects;
};
