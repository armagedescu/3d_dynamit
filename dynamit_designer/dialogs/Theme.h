#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dwmapi.h>
#include <uxtheme.h>
#include <string>
#include <fstream>
#include <functional>
#include <vector>
#include <cmath>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")

enum class ThemeMode { Light, Dark, Auto };

struct ThemeColors
{
    COLORREF background;
    COLORREF text;
    COLORREF controlBg;
    COLORREF controlText;
    COLORREF border;
    COLORREF accent;
    COLORREF buttonBg;
    COLORREF buttonText;
    COLORREF hoverBg;
    COLORREF disabledText;
};

class Theme
{
public:
    static Theme& instance()
    {
        static Theme s;
        return s;
    }

    void setMode(ThemeMode mode)
    {
        m_mode = mode;
        save();
        notifyAll();
    }

    ThemeMode getMode() const { return m_mode; }

    bool isDark() const
    {
        if (m_mode == ThemeMode::Dark) return true;
        if (m_mode == ThemeMode::Light) return false;
        return isOsDark();
    }

    const ThemeColors& colors() const
    {
        return isDark() ? darkColors() : lightColors();
    }

    HBRUSH backgroundBrush()
    {
        ensureBrushes();
        return isDark() ? m_darkBgBrush : m_lightBgBrush;
    }

    HBRUSH controlBrush()
    {
        ensureBrushes();
        return isDark() ? m_darkCtrlBrush : m_lightCtrlBrush;
    }

    // Apply dark/light title bar to a window
    static void applyTitleBar(HWND hwnd, bool dark)
    {
        BOOL useDark = dark ? TRUE : FALSE;
        // DWMWA_USE_IMMERSIVE_DARK_MODE = 20
        DwmSetWindowAttribute(hwnd, 20, &useDark, sizeof(useDark));
    }

    // Register a callback for theme changes
    int addListener(std::function<void()> callback)
    {
        int id = m_nextListenerId++;
        m_listeners.push_back({ id, callback });
        return id;
    }

    void removeListener(int id)
    {
        m_listeners.erase(
            std::remove_if(m_listeners.begin(), m_listeners.end(),
                [id](const Listener& l) { return l.id == id; }),
            m_listeners.end());
    }

    void load()
    {
        std::wstring path = settingsPath();
        std::ifstream file(path);
        if (!file.is_open())
            return;

        std::string line;
        if (std::getline(file, line))
        {
            if (line == "dark") m_mode = ThemeMode::Dark;
            else if (line == "light") m_mode = ThemeMode::Light;
            else m_mode = ThemeMode::Auto;
        }
    }

    // Owner-draw button painting
    static void drawButton(DRAWITEMSTRUCT* dis)
    {
        auto& t = instance();
        auto& c = t.colors();

        HBRUSH bg;
        if (dis->itemState & ODS_SELECTED)
            bg = CreateSolidBrush(c.accent);
        else if (dis->itemState & ODS_HOTLIGHT)
            bg = CreateSolidBrush(c.hoverBg);
        else
            bg = CreateSolidBrush(c.buttonBg);

        FillRect(dis->hDC, &dis->rcItem, bg);
        DeleteObject(bg);

        // Border
        HPEN pen = CreatePen(PS_SOLID, 1, c.border);
        HPEN oldPen = (HPEN)SelectObject(dis->hDC, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(dis->hDC, GetStockObject(NULL_BRUSH));
        Rectangle(dis->hDC, dis->rcItem.left, dis->rcItem.top,
            dis->rcItem.right, dis->rcItem.bottom);
        SelectObject(dis->hDC, oldPen);
        SelectObject(dis->hDC, oldBrush);
        DeleteObject(pen);

        // Text
        SetBkMode(dis->hDC, TRANSPARENT);
        SetTextColor(dis->hDC, (dis->itemState & ODS_SELECTED) ? RGB(255, 255, 255) : c.buttonText);

        wchar_t text[128];
        GetWindowTextW(dis->hwndItem, text, 128);
        DrawTextW(dis->hDC, text, -1, &dis->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        // Focus rect
        if (dis->itemState & ODS_FOCUS)
        {
            RECT focusRect = dis->rcItem;
            InflateRect(&focusRect, -3, -3);
            DrawFocusRect(dis->hDC, &focusRect);
        }
    }

    // Draw a theme chooser button (sun/moon/auto icon)
    static void drawThemeButton(DRAWITEMSTRUCT* dis, ThemeMode buttonMode)
    {
        auto& t = instance();
        auto& c = t.colors();

        bool active = (t.getMode() == buttonMode);

        HBRUSH bg;
        if (active)
            bg = CreateSolidBrush(c.accent);
        else if (dis->itemState & ODS_SELECTED)
            bg = CreateSolidBrush(c.hoverBg);
        else
            bg = CreateSolidBrush(c.buttonBg);

        FillRect(dis->hDC, &dis->rcItem, bg);
        DeleteObject(bg);

        // Border
        HPEN pen = CreatePen(PS_SOLID, 1, active ? c.accent : c.border);
        HPEN oldPen = (HPEN)SelectObject(dis->hDC, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(dis->hDC, GetStockObject(NULL_BRUSH));
        Rectangle(dis->hDC, dis->rcItem.left, dis->rcItem.top,
            dis->rcItem.right, dis->rcItem.bottom);
        SelectObject(dis->hDC, oldPen);
        SelectObject(dis->hDC, oldBrush);
        DeleteObject(pen);

        SetBkMode(dis->hDC, TRANSPARENT);

        COLORREF textColor;
        if (active)
            textColor = RGB(255, 255, 255);
        else if (buttonMode == ThemeMode::Light || buttonMode == ThemeMode::Dark)
            textColor = RGB(200, 170, 40);  // golden yellow for sun/moon
        else
            textColor = c.buttonText;
        SetTextColor(dis->hDC, textColor);

        // Draw icon using GDI shapes instead of Unicode
        int cx = (dis->rcItem.left + dis->rcItem.right) / 2;
        int cy = (dis->rcItem.top + dis->rcItem.bottom) / 2;
        int r = 5;

        HPEN oldPen2 = (HPEN)SelectObject(dis->hDC, GetStockObject(NULL_PEN));
        HBRUSH iconBrush = CreateSolidBrush(textColor);
        HBRUSH oldBrush2 = (HBRUSH)SelectObject(dis->hDC, iconBrush);

        if (buttonMode == ThemeMode::Light)
        {
            // Sun: circle + rays
            Ellipse(dis->hDC, cx - r, cy - r, cx + r + 1, cy + r + 1);
            HPEN rayPen = CreatePen(PS_SOLID, 1, textColor);
            SelectObject(dis->hDC, rayPen);
            for (int i = 0; i < 8; i++)
            {
                double angle = i * 3.14159265 / 4.0;
                int x1 = cx + (int)((r + 2) * cos(angle));
                int y1 = cy + (int)((r + 2) * sin(angle));
                int x2 = cx + (int)((r + 4) * cos(angle));
                int y2 = cy + (int)((r + 4) * sin(angle));
                MoveToEx(dis->hDC, x1, y1, nullptr);
                LineTo(dis->hDC, x2, y2);
            }
            SelectObject(dis->hDC, GetStockObject(NULL_PEN));
            DeleteObject(rayPen);
        }
        else if (buttonMode == ThemeMode::Dark)
        {
            // Moon: circle with background-colored circle overlapping
            Ellipse(dis->hDC, cx - r, cy - r, cx + r + 1, cy + r + 1);
            // Mask with background
            HBRUSH maskBrush = CreateSolidBrush(active ? c.accent : c.buttonBg);
            SelectObject(dis->hDC, maskBrush);
            Ellipse(dis->hDC, cx - r + 3, cy - r - 1, cx + r + 4, cy + r);
            DeleteObject(maskBrush);
        }
        else
        {
            // Auto: half-filled circle
            Ellipse(dis->hDC, cx - r, cy - r, cx + r + 1, cy + r + 1);
            // Cover right half with background
            HBRUSH maskBrush = CreateSolidBrush(active ? c.accent : c.buttonBg);
            RECT halfRect = { cx, cy - r - 1, cx + r + 2, cy + r + 2 };
            FillRect(dis->hDC, &halfRect, maskBrush);
            DeleteObject(maskBrush);
            // Redraw outline
            HPEN outlinePen = CreatePen(PS_SOLID, 1, textColor);
            SelectObject(dis->hDC, outlinePen);
            SelectObject(dis->hDC, GetStockObject(NULL_BRUSH));
            Ellipse(dis->hDC, cx - r, cy - r, cx + r + 1, cy + r + 1);
            SelectObject(dis->hDC, GetStockObject(NULL_PEN));
            DeleteObject(outlinePen);
        }

        SelectObject(dis->hDC, oldBrush2);
        SelectObject(dis->hDC, oldPen2);
        DeleteObject(iconBrush);
    }

    // Apply theme colors to a ListView
    static void applyToListView(HWND hList)
    {
        auto& c = instance().colors();
        ListView_SetBkColor(hList, c.controlBg);
        ListView_SetTextBkColor(hList, c.controlBg);
        ListView_SetTextColor(hList, c.controlText);
        InvalidateRect(hList, nullptr, TRUE);
    }

    // Handle WM_CTLCOLORSTATIC
    HBRUSH onCtlColorStatic(HDC hdc)
    {
        auto& c = colors();
        SetTextColor(hdc, c.text);
        SetBkColor(hdc, c.background);
        return backgroundBrush();
    }

private:
    Theme() { load(); }

    static const ThemeColors& lightColors()
    {
        static ThemeColors c = {
            RGB(240, 240, 240),   // background
            RGB(30, 30, 30),      // text
            RGB(255, 255, 255),   // controlBg
            RGB(30, 30, 30),      // controlText
            RGB(180, 180, 180),   // border
            RGB(60, 120, 216),    // accent
            RGB(225, 225, 225),   // buttonBg
            RGB(30, 30, 30),      // buttonText
            RGB(210, 210, 210),   // hoverBg
            RGB(140, 140, 140),   // disabledText
        };
        return c;
    }

    static const ThemeColors& darkColors()
    {
        static ThemeColors c = {
            RGB(32, 32, 38),      // background
            RGB(220, 225, 235),   // text
            RGB(42, 42, 50),      // controlBg
            RGB(210, 215, 225),   // controlText
            RGB(65, 65, 75),      // border
            RGB(80, 140, 255),    // accent
            RGB(50, 50, 58),      // buttonBg
            RGB(210, 215, 225),   // buttonText
            RGB(60, 60, 70),      // hoverBg
            RGB(100, 100, 115),   // disabledText
        };
        return c;
    }

    static bool isOsDark()
    {
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            DWORD value = 1, size = sizeof(DWORD);
            RegQueryValueExW(hKey, L"AppsUseLightTheme", nullptr, nullptr,
                (LPBYTE)&value, &size);
            RegCloseKey(hKey);
            return value == 0;
        }
        return false;
    }

    void save()
    {
        std::wstring path = settingsPath();
        std::ofstream file(path);
        if (!file.is_open()) return;

        if (m_mode == ThemeMode::Dark) file << "dark";
        else if (m_mode == ThemeMode::Light) file << "light";
        else file << "auto";
    }

    static std::wstring settingsPath()
    {
        wchar_t appData[MAX_PATH];
        DWORD len = GetEnvironmentVariableW(L"APPDATA", appData, MAX_PATH);
        if (len > 0 && len < MAX_PATH)
        {
            std::wstring dir = std::wstring(appData) + L"\\DynamitDesigner";
            CreateDirectoryW(dir.c_str(), nullptr);
            return dir + L"\\theme.txt";
        }
        return L"theme.txt";
    }

    void ensureBrushes()
    {
        if (!m_darkBgBrush)
        {
            m_darkBgBrush = CreateSolidBrush(darkColors().background);
            m_darkCtrlBrush = CreateSolidBrush(darkColors().controlBg);
            m_lightBgBrush = CreateSolidBrush(lightColors().background);
            m_lightCtrlBrush = CreateSolidBrush(lightColors().controlBg);
        }
    }

    void notifyAll()
    {
        for (auto& l : m_listeners)
            l.callback();
    }

    struct Listener
    {
        int id;
        std::function<void()> callback;
    };

    ThemeMode m_mode = ThemeMode::Auto;
    std::vector<Listener> m_listeners;
    int m_nextListenerId = 1;

    HBRUSH m_darkBgBrush = nullptr;
    HBRUSH m_darkCtrlBrush = nullptr;
    HBRUSH m_lightBgBrush = nullptr;
    HBRUSH m_lightCtrlBrush = nullptr;
};
