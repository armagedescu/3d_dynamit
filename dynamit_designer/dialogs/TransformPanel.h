#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>

#include <atlbase.h>
#include <atlwin.h>

class DesignerApp;

// Control IDs
#define ID_EDIT_POS_X   3001
#define ID_EDIT_POS_Y   3002
#define ID_EDIT_POS_Z   3003
#define ID_EDIT_ROT_X   3004
#define ID_EDIT_ROT_Y   3005
#define ID_EDIT_ROT_Z   3006
#define ID_EDIT_SCALE_X 3007
#define ID_EDIT_SCALE_Y 3008
#define ID_EDIT_SCALE_Z 3009

class TransformPanel : public CWindowImpl<TransformPanel>
{
public:
    DECLARE_WND_CLASS(L"TransformPanelClass")

    TransformPanel(DesignerApp* app) : m_app(app), m_updating(false) {}

    HWND Create(HWND parent)
    {
        RECT rc = { 0, 0, 280, 180 };
        CWindowImpl::Create(parent, rc, L"Transform",
            WS_POPUP | WS_CAPTION | WS_VISIBLE, WS_EX_TOOLWINDOW);

        if (m_hWnd)
        {
            createControls();
        }

        return m_hWnd;
    }

    void updateFromConfig();

    BEGIN_MSG_MAP(TransformPanel)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        COMMAND_HANDLER(ID_EDIT_POS_X, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_EDIT_POS_Y, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_EDIT_POS_Z, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_EDIT_ROT_X, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_EDIT_ROT_Y, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_EDIT_ROT_Z, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_EDIT_SCALE_X, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_EDIT_SCALE_Y, EN_CHANGE, OnEditChange)
        COMMAND_HANDLER(ID_EDIT_SCALE_Z, EN_CHANGE, OnEditChange)
    END_MSG_MAP()

private:
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        ShowWindow(SW_HIDE);
        return 0;
    }

    LRESULT OnEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        applyChanges();
        return 0;
    }

    void createControls()
    {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        int y = 5;
        int labelW = 55;
        int editW = 55;
        int gap = 5;

        auto createLabel = [&](const wchar_t* text, int xPos, int yPos) {
            HWND h = CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE,
                xPos, yPos + 3, labelW, 18, m_hWnd, nullptr, GetModuleHandle(nullptr), nullptr);
            ::SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
        };

        auto createEdit = [&](int id, int xPos, int yPos) -> HWND {
            HWND h = CreateWindowW(L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                xPos, yPos, editW, 20, m_hWnd, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
            ::SendMessage(h, WM_SETFONT, (WPARAM)hFont, TRUE);
            return h;
        };

        // Header row
        createLabel(L"", 5, y);
        createLabel(L"X", labelW + 20, y);
        createLabel(L"Y", labelW + 20 + editW + gap, y);
        createLabel(L"Z", labelW + 20 + 2 * (editW + gap), y);
        y += 22;

        // Position row
        createLabel(L"Position:", 5, y);
        m_editPosX = createEdit(ID_EDIT_POS_X, labelW + 10, y);
        m_editPosY = createEdit(ID_EDIT_POS_Y, labelW + 10 + editW + gap, y);
        m_editPosZ = createEdit(ID_EDIT_POS_Z, labelW + 10 + 2 * (editW + gap), y);
        y += 26;

        // Rotation row
        createLabel(L"Rotation:", 5, y);
        m_editRotX = createEdit(ID_EDIT_ROT_X, labelW + 10, y);
        m_editRotY = createEdit(ID_EDIT_ROT_Y, labelW + 10 + editW + gap, y);
        m_editRotZ = createEdit(ID_EDIT_ROT_Z, labelW + 10 + 2 * (editW + gap), y);
        y += 26;

        // Scale row
        createLabel(L"Scale:", 5, y);
        m_editScaleX = createEdit(ID_EDIT_SCALE_X, labelW + 10, y);
        m_editScaleY = createEdit(ID_EDIT_SCALE_Y, labelW + 10 + editW + gap, y);
        m_editScaleZ = createEdit(ID_EDIT_SCALE_Z, labelW + 10 + 2 * (editW + gap), y);
    }

    void applyChanges();

    DesignerApp* m_app;
    bool m_updating;

    HWND m_editPosX, m_editPosY, m_editPosZ;
    HWND m_editRotX, m_editRotY, m_editRotZ;
    HWND m_editScaleX, m_editScaleY, m_editScaleZ;
};

// Include implementation
#include "../DesignerApp.h"
#include "../ShapeManager.h"

inline void TransformPanel::updateFromConfig()
{
    if (!m_hWnd) return;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg) return;

    m_updating = true;

    wchar_t buf[32];

    swprintf_s(buf, L"%.2f", cfg->posX);
    ::SetWindowTextW(m_editPosX, buf);
    swprintf_s(buf, L"%.2f", cfg->posY);
    ::SetWindowTextW(m_editPosY, buf);
    swprintf_s(buf, L"%.2f", cfg->posZ);
    ::SetWindowTextW(m_editPosZ, buf);
    swprintf_s(buf, L"%.1f", cfg->rotX);
    ::SetWindowTextW(m_editRotX, buf);
    swprintf_s(buf, L"%.1f", cfg->rotY);
    ::SetWindowTextW(m_editRotY, buf);
    swprintf_s(buf, L"%.1f", cfg->rotZ);
    ::SetWindowTextW(m_editRotZ, buf);

    swprintf_s(buf, L"%.2f", cfg->scaleX);
    ::SetWindowTextW(m_editScaleX, buf);
    swprintf_s(buf, L"%.2f", cfg->scaleY);
    ::SetWindowTextW(m_editScaleY, buf);
    swprintf_s(buf, L"%.2f", cfg->scaleZ);
    ::SetWindowTextW(m_editScaleZ, buf);
    m_updating = false;
}

inline void TransformPanel::applyChanges()
{
    if (m_updating) return;

    ShapeConfig* cfg = m_app->getSelectedShapeConfig();
    if (!cfg) return;

    wchar_t buf[32];

    ::GetWindowTextW(m_editPosX, buf, 32);
    cfg->posX = static_cast<float>(_wtof(buf));
    ::GetWindowTextW(m_editPosY, buf, 32);
    cfg->posY = static_cast<float>(_wtof(buf));
    ::GetWindowTextW(m_editPosZ, buf, 32);
    cfg->posZ = static_cast<float>(_wtof(buf));

    ::GetWindowTextW(m_editRotX, buf, 32);
    cfg->rotX = static_cast<float>(_wtof(buf));
    ::GetWindowTextW(m_editRotY, buf, 32);
    cfg->rotY = static_cast<float>(_wtof(buf));
    ::GetWindowTextW(m_editRotZ, buf, 32);
    cfg->rotZ = static_cast<float>(_wtof(buf));

    ::GetWindowTextW(m_editScaleX, buf, 32);
    cfg->scaleX = static_cast<float>(_wtof(buf));
    if (cfg->scaleX < 0.01f) cfg->scaleX = 0.01f;
    ::GetWindowTextW(m_editScaleY, buf, 32);
    cfg->scaleY = static_cast<float>(_wtof(buf));
    if (cfg->scaleY < 0.01f) cfg->scaleY = 0.01f;
    ::GetWindowTextW(m_editScaleZ, buf, 32);
    cfg->scaleZ = static_cast<float>(_wtof(buf));
    if (cfg->scaleZ < 0.01f) cfg->scaleZ = 0.01f;

    m_app->onShapeConfigChanged();
}
