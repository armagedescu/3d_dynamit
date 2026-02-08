#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <cmath>

#include <atlbase.h>
#include <atlwin.h>

#include "../ShapeManager.h"  // For FormulaSegment

// Control IDs (FE_ prefix to avoid conflicts with BuilderPanel)
#define ID_FE_LIST_SEGMENTS      5001
#define ID_FE_BTN_ADD            5002
#define ID_FE_BTN_REMOVE         5003
#define ID_FE_BTN_MOVE_UP        5004
#define ID_FE_BTN_MOVE_DOWN      5005
#define ID_FE_EDIT_FORMULA       5006
#define ID_FE_EDIT_DOMAIN_START  5007
#define ID_FE_EDIT_DOMAIN_END    5008
#define ID_FE_PREVIEW_CANVAS     5009
#define ID_FE_BTN_OK             5010
#define ID_FE_BTN_CANCEL         5011

class FormulaEditorDialog : public CWindowImpl<FormulaEditorDialog>
{
public:
    DECLARE_WND_CLASS(L"FormulaEditorDialogClass")

    FormulaEditorDialog() : m_result(false), m_selectedIndex(-1), m_updating(false) {}

    // Show modal dialog, returns true if OK pressed
    bool Show(HWND parent, std::vector<FormulaSegment>& segments)
    {
        m_segments = segments;
        m_result = false;

        // Dialog size
        int dlgW = 700;
        int dlgH = 500;

        // Center on parent
        RECT parentRect;
        ::GetWindowRect(parent, &parentRect);
        int x = parentRect.left + ((parentRect.right - parentRect.left) - dlgW) / 2;
        int y = parentRect.top + ((parentRect.bottom - parentRect.top) - dlgH) / 2;

        RECT rc = { x, y, x + dlgW, y + dlgH };
        Create(parent, rc, L"Formula Editor",
            WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
            WS_EX_DLGMODALFRAME);

        if (!m_hWnd)
            return false;

        createControls();
        populateSegmentList();
        if (!m_segments.empty())
            selectSegment(0);
        updatePreview();

        // Disable parent
        ::EnableWindow(parent, FALSE);

        // Modal message loop
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            if (!IsWindow())
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Re-enable parent
        ::EnableWindow(parent, TRUE);
        ::SetForegroundWindow(parent);

        if (m_result)
        {
            segments = m_segments;
        }

        return m_result;
    }

    BEGIN_MSG_MAP(FormulaEditorDialog)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
        MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
        COMMAND_HANDLER(ID_FE_LIST_SEGMENTS, LBN_SELCHANGE, OnSegmentSelChange)
        COMMAND_HANDLER(ID_FE_EDIT_FORMULA, EN_CHANGE, OnFormulaChange)
        COMMAND_HANDLER(ID_FE_EDIT_DOMAIN_START, EN_CHANGE, OnDomainChange)
        COMMAND_HANDLER(ID_FE_EDIT_DOMAIN_END, EN_CHANGE, OnDomainChange)
        COMMAND_ID_HANDLER(ID_FE_BTN_ADD, OnAdd)
        COMMAND_ID_HANDLER(ID_FE_BTN_REMOVE, OnRemove)
        COMMAND_ID_HANDLER(ID_FE_BTN_MOVE_UP, OnMoveUp)
        COMMAND_ID_HANDLER(ID_FE_BTN_MOVE_DOWN, OnMoveDown)
        COMMAND_ID_HANDLER(ID_FE_BTN_OK, OnOK)
        COMMAND_ID_HANDLER(ID_FE_BTN_CANCEL, OnCancel)
    END_MSG_MAP()

private:
    LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_result = false;
        DestroyWindow();
        return 0;
    }

    LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&)
    {
        PostQuitMessage(0);
        return 0;
    }

    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL& bHandled)
    {
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnDrawItem(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        if (wParam == ID_FE_PREVIEW_CANVAS)
        {
            DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
            drawPreview(dis->hDC, dis->rcItem);
            return TRUE;
        }
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnSegmentSelChange(WORD, WORD, HWND, BOOL&)
    {
        int sel = (int)::SendMessage(m_listSegments, LB_GETCURSEL, 0, 0);
        if (sel != LB_ERR)
        {
            saveCurrentSegment();
            selectSegment(sel);
        }
        return 0;
    }

    LRESULT OnFormulaChange(WORD, WORD, HWND, BOOL&)
    {
        if (!m_updating)
        {
            saveCurrentSegment();
            updateSegmentListItem(m_selectedIndex);
            updatePreview();
        }
        return 0;
    }

    LRESULT OnDomainChange(WORD, WORD, HWND, BOOL&)
    {
        if (!m_updating)
        {
            saveCurrentSegment();
            updateSegmentListItem(m_selectedIndex);
            updatePreview();
        }
        return 0;
    }

    LRESULT OnAdd(WORD, WORD, HWND, BOOL&)
    {
        FormulaSegment seg;
        seg.formula = L"1";
        seg.domainStart = 0.0f;
        seg.domainEnd = 6.2832f;

        // If we have segments, start new one where last one ended
        if (!m_segments.empty())
        {
            seg.domainStart = m_segments.back().domainEnd;
            seg.domainEnd = seg.domainStart + 1.0f;
        }

        m_segments.push_back(seg);
        populateSegmentList();
        selectSegment((int)m_segments.size() - 1);
        updatePreview();
        return 0;
    }

    LRESULT OnRemove(WORD, WORD, HWND, BOOL&)
    {
        if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_segments.size())
        {
            m_segments.erase(m_segments.begin() + m_selectedIndex);
            populateSegmentList();
            if (m_selectedIndex >= (int)m_segments.size())
                m_selectedIndex = (int)m_segments.size() - 1;
            if (m_selectedIndex >= 0)
                selectSegment(m_selectedIndex);
            else
                clearEditFields();
            updatePreview();
        }
        return 0;
    }

    LRESULT OnMoveUp(WORD, WORD, HWND, BOOL&)
    {
        if (m_selectedIndex > 0)
        {
            std::swap(m_segments[m_selectedIndex], m_segments[m_selectedIndex - 1]);
            populateSegmentList();
            selectSegment(m_selectedIndex - 1);
            updatePreview();
        }
        return 0;
    }

    LRESULT OnMoveDown(WORD, WORD, HWND, BOOL&)
    {
        if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_segments.size() - 1)
        {
            std::swap(m_segments[m_selectedIndex], m_segments[m_selectedIndex + 1]);
            populateSegmentList();
            selectSegment(m_selectedIndex + 1);
            updatePreview();
        }
        return 0;
    }

    LRESULT OnOK(WORD, WORD, HWND, BOOL&)
    {
        saveCurrentSegment();
        m_result = true;
        DestroyWindow();
        return 0;
    }

    LRESULT OnCancel(WORD, WORD, HWND, BOOL&)
    {
        m_result = false;
        DestroyWindow();
        return 0;
    }

    void createControls()
    {
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        HINSTANCE hInst = GetModuleHandle(nullptr);

        RECT clientRect;
        GetClientRect(&clientRect);
        int cw = clientRect.right;
        int ch = clientRect.bottom;

        int margin = 10;
        int leftPanelW = 280;
        int buttonH = 25;
        int editH = 22;
        int listH = 150;

        int y = margin;

        // Left panel - Segments list
        HWND lblSegments = CreateWindowW(L"STATIC", L"Formula Segments:",
            WS_CHILD | WS_VISIBLE, margin, y, leftPanelW, 18, m_hWnd, nullptr, hInst, nullptr);
        ::SendMessage(lblSegments, WM_SETFONT, (WPARAM)hFont, TRUE);
        y += 20;

        m_listSegments = CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", nullptr,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
            margin, y, leftPanelW, listH, m_hWnd, (HMENU)ID_FE_LIST_SEGMENTS, hInst, nullptr);
        ::SendMessage(m_listSegments, WM_SETFONT, (WPARAM)hFont, TRUE);
        y += listH + 5;

        // Buttons row
        int btnW = 60;
        int btnGap = 5;
        HWND btnAdd = CreateWindowW(L"BUTTON", L"Add",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            margin, y, btnW, buttonH, m_hWnd, (HMENU)ID_FE_BTN_ADD, hInst, nullptr);
        ::SendMessage(btnAdd, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND btnRemove = CreateWindowW(L"BUTTON", L"Remove",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            margin + btnW + btnGap, y, btnW, buttonH, m_hWnd, (HMENU)ID_FE_BTN_REMOVE, hInst, nullptr);
        ::SendMessage(btnRemove, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND btnUp = CreateWindowW(L"BUTTON", L"Up",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            margin + 2 * (btnW + btnGap), y, 40, buttonH, m_hWnd, (HMENU)ID_FE_BTN_MOVE_UP, hInst, nullptr);
        ::SendMessage(btnUp, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND btnDown = CreateWindowW(L"BUTTON", L"Down",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            margin + 2 * (btnW + btnGap) + 45, y, 45, buttonH, m_hWnd, (HMENU)ID_FE_BTN_MOVE_DOWN, hInst, nullptr);
        ::SendMessage(btnDown, WM_SETFONT, (WPARAM)hFont, TRUE);
        y += buttonH + 15;

        // Edit fields for selected segment
        HWND lblFormula = CreateWindowW(L"STATIC", L"Formula:",
            WS_CHILD | WS_VISIBLE, margin, y + 2, 60, 18, m_hWnd, nullptr, hInst, nullptr);
        ::SendMessage(lblFormula, WM_SETFONT, (WPARAM)hFont, TRUE);
        m_editFormula = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            margin + 65, y, leftPanelW - 65, editH, m_hWnd, (HMENU)ID_FE_EDIT_FORMULA, hInst, nullptr);
        ::SendMessage(m_editFormula, WM_SETFONT, (WPARAM)hFont, TRUE);
        y += editH + 8;

        HWND lblStart = CreateWindowW(L"STATIC", L"Start:",
            WS_CHILD | WS_VISIBLE, margin, y + 2, 60, 18, m_hWnd, nullptr, hInst, nullptr);
        ::SendMessage(lblStart, WM_SETFONT, (WPARAM)hFont, TRUE);
        m_editDomainStart = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            margin + 65, y, 80, editH, m_hWnd, (HMENU)ID_FE_EDIT_DOMAIN_START, hInst, nullptr);
        ::SendMessage(m_editDomainStart, WM_SETFONT, (WPARAM)hFont, TRUE);
        y += editH + 8;

        HWND lblEnd = CreateWindowW(L"STATIC", L"End:",
            WS_CHILD | WS_VISIBLE, margin, y + 2, 60, 18, m_hWnd, nullptr, hInst, nullptr);
        ::SendMessage(lblEnd, WM_SETFONT, (WPARAM)hFont, TRUE);
        m_editDomainEnd = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            margin + 65, y, 80, editH, m_hWnd, (HMENU)ID_FE_EDIT_DOMAIN_END, hInst, nullptr);
        ::SendMessage(m_editDomainEnd, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Right panel - Preview canvas (owner-drawn static)
        int previewX = margin + leftPanelW + 20;
        int previewW = cw - previewX - margin;
        int previewH = ch - 60;

        HWND lblPreview = CreateWindowW(L"STATIC", L"Preview (r = f(θ)):",
            WS_CHILD | WS_VISIBLE, previewX, margin, 150, 18, m_hWnd, nullptr, hInst, nullptr);
        ::SendMessage(lblPreview, WM_SETFONT, (WPARAM)hFont, TRUE);

        m_previewCanvas = CreateWindowW(L"STATIC", L"",
            WS_CHILD | WS_VISIBLE | SS_OWNERDRAW | WS_BORDER,
            previewX, margin + 22, previewW, previewH - 22, m_hWnd, (HMENU)ID_FE_PREVIEW_CANVAS, hInst, nullptr);

        // Bottom buttons
        int bottomY = ch - buttonH - margin;
        int okCancelW = 80;
        HWND btnOK = CreateWindowW(L"BUTTON", L"OK",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            cw - margin - 2 * okCancelW - 10, bottomY, okCancelW, buttonH, m_hWnd, (HMENU)ID_FE_BTN_OK, hInst, nullptr);
        ::SendMessage(btnOK, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND btnCancel = CreateWindowW(L"BUTTON", L"Cancel",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            cw - margin - okCancelW, bottomY, okCancelW, buttonH, m_hWnd, (HMENU)ID_FE_BTN_CANCEL, hInst, nullptr);
        ::SendMessage(btnCancel, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    void populateSegmentList()
    {
        ::SendMessage(m_listSegments, LB_RESETCONTENT, 0, 0);
        for (size_t i = 0; i < m_segments.size(); ++i)
        {
            std::wstring display = formatSegmentDisplay((int)i);
            ::SendMessageW(m_listSegments, LB_ADDSTRING, 0, (LPARAM)display.c_str());
        }
    }

    std::wstring formatSegmentDisplay(int index)
    {
        if (index < 0 || index >= (int)m_segments.size())
            return L"";

        const FormulaSegment& seg = m_segments[index];
        wchar_t buf[256];
        swprintf_s(buf, L"%d: %s  [%.2f, %.2f]",
            index + 1, seg.formula.c_str(), seg.domainStart, seg.domainEnd);
        return buf;
    }

    void updateSegmentListItem(int index)
    {
        if (index < 0 || index >= (int)m_segments.size())
            return;

        std::wstring display = formatSegmentDisplay(index);
        ::SendMessage(m_listSegments, LB_DELETESTRING, index, 0);
        ::SendMessageW(m_listSegments, LB_INSERTSTRING, index, (LPARAM)display.c_str());
        ::SendMessage(m_listSegments, LB_SETCURSEL, index, 0);
    }

    void selectSegment(int index)
    {
        m_selectedIndex = index;
        ::SendMessage(m_listSegments, LB_SETCURSEL, index, 0);

        if (index >= 0 && index < (int)m_segments.size())
        {
            m_updating = true;
            const FormulaSegment& seg = m_segments[index];
            ::SetWindowTextW(m_editFormula, seg.formula.c_str());

            wchar_t buf[32];
            swprintf_s(buf, L"%.4f", seg.domainStart);
            ::SetWindowTextW(m_editDomainStart, buf);
            swprintf_s(buf, L"%.4f", seg.domainEnd);
            ::SetWindowTextW(m_editDomainEnd, buf);
            m_updating = false;
        }
    }

    void saveCurrentSegment()
    {
        if (m_selectedIndex < 0 || m_selectedIndex >= (int)m_segments.size())
            return;

        FormulaSegment& seg = m_segments[m_selectedIndex];

        wchar_t buf[256];
        ::GetWindowTextW(m_editFormula, buf, 256);
        seg.formula = buf;

        ::GetWindowTextW(m_editDomainStart, buf, 64);
        seg.domainStart = evaluateExpression(buf);

        ::GetWindowTextW(m_editDomainEnd, buf, 64);
        seg.domainEnd = evaluateExpression(buf);
    }

    // Evaluate a constant expression (like "pi", "2*pi", "3.14") - implemented after includes
    float evaluateExpression(const std::wstring& expr);

    void clearEditFields()
    {
        m_updating = true;
        ::SetWindowTextW(m_editFormula, L"");
        ::SetWindowTextW(m_editDomainStart, L"");
        ::SetWindowTextW(m_editDomainEnd, L"");
        m_updating = false;
    }

    void updatePreview()
    {
        ::InvalidateRect(m_previewCanvas, nullptr, TRUE);
    }

    void drawPreview(HDC hdc, RECT rc)
    {
        // Fill background
        HBRUSH bgBrush = CreateSolidBrush(RGB(40, 40, 50));
        FillRect(hdc, &rc, bgBrush);
        DeleteObject(bgBrush);

        int w = rc.right - rc.left;
        int h = rc.bottom - rc.top;
        int cx = w / 2;
        int cy = h / 2;
        int radius = (std::min)(cx, cy) - 20;

        // Draw grid circles
        HPEN gridPen = CreatePen(PS_DOT, 1, RGB(80, 80, 100));
        HPEN oldPen = (HPEN)SelectObject(hdc, gridPen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        for (int r = radius / 4; r <= radius; r += radius / 4)
        {
            Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
        }

        // Draw axes
        HPEN axisPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 120));
        SelectObject(hdc, axisPen);
        MoveToEx(hdc, cx - radius - 10, cy, nullptr);
        LineTo(hdc, cx + radius + 10, cy);
        MoveToEx(hdc, cx, cy - radius - 10, nullptr);
        LineTo(hdc, cx, cy + radius + 10);

        // Draw formula curves
        if (!m_segments.empty())
        {
            HPEN curvePen = CreatePen(PS_SOLID, 2, RGB(0, 255, 128));
            SelectObject(hdc, curvePen);

            // Find max radius for scaling
            float maxR = 1.0f;
            for (const auto& seg : m_segments)
            {
                float r = evaluateFormula(seg.formula, (seg.domainStart + seg.domainEnd) / 2);
                if (r > maxR) maxR = r;
            }
            float scale = (float)radius / (maxR * 1.2f);

            bool firstPoint = true;
            for (const auto& seg : m_segments)
            {
                int steps = 100;
                float dt = (seg.domainEnd - seg.domainStart) / steps;

                for (int i = 0; i <= steps; ++i)
                {
                    float t = seg.domainStart + i * dt;
                    float r = evaluateFormula(seg.formula, t);

                    // Polar to Cartesian
                    int px = cx + (int)(r * scale * cos(t));
                    int py = cy - (int)(r * scale * sin(t));

                    if (firstPoint)
                    {
                        MoveToEx(hdc, px, py, nullptr);
                        firstPoint = false;
                    }
                    else
                    {
                        LineTo(hdc, px, py);
                    }
                }
            }

            DeleteObject(curvePen);
        }

        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(gridPen);
        DeleteObject(axisPen);

        // Draw scale label
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(150, 150, 170));
        HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        SelectObject(hdc, hFont);
        TextOutW(hdc, 5, 5, L"Polar: r = f(\u03B8)", 15);
    }

    float evaluateFormula(const std::wstring& formula, float t);

    std::vector<FormulaSegment> m_segments;
    bool m_result;
    int m_selectedIndex;
    bool m_updating;

    HWND m_listSegments;
    HWND m_editFormula;
    HWND m_editDomainStart;
    HWND m_editDomainEnd;
    HWND m_previewCanvas;
};

// Include implementation that uses the expression compiler
#include <expression_compiler.h>

inline float FormulaEditorDialog::evaluateFormula(const std::wstring& formula, float t)
{
    try
    {
        expresie_tokenizer::expression_token_compiler compiler;
        std::unique_ptr<expresie_tokenizer::expression> expr = compiler.compile(formula);
        if (!expr)
            return 1.0f;

        // Bind variables
        long double theta = static_cast<long double>(t);
        expr->bind(L"t", &theta);
        expr->bind(L"theta", &theta);

        return static_cast<float>(expr->eval());
    }
    catch (...)
    {
        return 1.0f;  // Default radius on error
    }
}

inline float FormulaEditorDialog::evaluateExpression(const std::wstring& expr)
{
    if (expr.empty())
        return 0.0f;

    // Try simple number first
    wchar_t* endptr = nullptr;
    float val = std::wcstof(expr.c_str(), &endptr);
    if (endptr && *endptr == L'\0')
        return val;

    // Try expression compiler for things like "pi", "2*pi"
    try
    {
        expresie_tokenizer::expression_token_compiler compiler;
        std::unique_ptr<expresie_tokenizer::expression> e = compiler.compile(expr);
        if (e)
            return static_cast<float>(e->eval());
    }
    catch (...) {}

    return 0.0f;
}
