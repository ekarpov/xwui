// Extended text label implementation
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"
#include "../../core/xwutils.h"
#include "../xctrlmsg.h"

#include "xtextlabelwnd.h"

/////////////////////////////////////////////////////////////////////
// constants
#define XWUI_TEXTLABEL_CLASS_NAME     L"XWUI_TEXTLABEL_CLASS"

/////////////////////////////////////////////////////////////////////
// XTextLabelWindow - text label window implementation

XTextLabelWindow::XTextLabelWindow()
{
}

XTextLabelWindow::~XTextLabelWindow()
{
}

/////////////////////////////////////////////////////////////////////
// window class
/////////////////////////////////////////////////////////////////////
const wchar_t* XTextLabelWindow::windowClass()
{
    return XWUI_TEXTLABEL_CLASS_NAME;
}

/////////////////////////////////////////////////////////////////////
// window procedure
/////////////////////////////////////////////////////////////////////
LRESULT XTextLabelWindow::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_PAINT:
        // paint label
        _onPaint(hwnd);
        break;

    case WM_DISPLAYCHANGE:
        // TODO: repaint
        break;

    case WM_DESTROY:
        // clear all caches for this window (if any)
        break;

    case WM_SETFONT:
        // TODO: use new font
        break;
    }

    // pass to base control
    return XCtrlWndBase::baseWindowProc(hwnd, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////
// layout helpers
/////////////////////////////////////////////////////////////////////
int XTextLabelWindow::getMinWidthHint(HWND hwnd) const
{
    LONG lWidth = 0;

    // get DC
    HDC hdc = ::GetDC(hwnd);

    // select font if any
    HFONT oldFont = 0;
    if(xctrl_hFont)
        oldFont = (HFONT) ::SelectObject(hdc, xctrl_hFont);

    // compute text width
    SIZE sz;
    if(::GetTextExtentPoint32W(hdc, xctrl_strText.c_str(), (UINT)xctrl_strText.length(), &sz) == TRUE)
    {
        // width
        lWidth = sz.cx;
    }

    // put old font back
    ::SelectObject(hdc, oldFont);

    // release DC
    ::ReleaseDC(hwnd, hdc);
     
    return lWidth;
}

int XTextLabelWindow::getMinHeightHint(HWND hwnd) const
{
    LONG lHeight = 0;

    // get DC
    HDC hdc = ::GetDC(hwnd);

    // select font if any
    HFONT oldFont = 0;
    if(xctrl_hFont)
        oldFont = (HFONT) ::SelectObject(hdc, xctrl_hFont);

    // compute text metrics
    TEXTMETRIC tm;
    if(::GetTextMetricsW(hdc, &tm) == TRUE)
    {
        // get height
        lHeight = tm.tmHeight;
    }

    // put old font back
    ::SelectObject(hdc, oldFont);

    // release DC
    ::ReleaseDC(hwnd, hdc);
     
    return lHeight;
}

/////////////////////////////////////////////////////////////////////
// implementation
/////////////////////////////////////////////////////////////////////
void XTextLabelWindow::_onPaint(HWND hwnd)
{
    PAINTSTRUCT  ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    // select font if any
    HFONT oldFont = 0;
    if(xctrl_hFont)
        oldFont = (HFONT) ::SelectObject(hdc, xctrl_hFont);

    // text color
    COLORREF oldFntClr = ::SetTextColor(hdc, xctrl_textColor);

    // check if control is transparent
    if(::GetWindowLongW(hwnd, GWL_EXSTYLE) & WS_EX_TRANSPARENT)
    {
        // ignore background
        ::SetBkMode(hdc,TRANSPARENT);

        // draw text 
        ::ExtTextOutW(hdc, 0, 0, 0, &ps.rcPaint, xctrl_strText.c_str(), (UINT)xctrl_strText.length(), 0);

    } else
    {
        // background color
        COLORREF oldBkClr = ::SetBkColor(hdc, xctrl_bkColor);

        // draw text 
        ::ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &ps.rcPaint, xctrl_strText.c_str(), (UINT)xctrl_strText.length(), 0);

        // set color back
        ::SetBkColor(hdc, oldBkClr);
    }


    // set color back
    ::SetTextColor(hdc, oldFntClr);            

    // put old font back
    ::SelectObject(hdc, oldFont);

    EndPaint(hwnd, &ps);
}

// XTextLabelWindow
/////////////////////////////////////////////////////////////////////


