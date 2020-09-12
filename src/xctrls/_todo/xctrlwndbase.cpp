// Extended controls common functionality
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"
#include "../core/xwutils.h"

#include "xctrlmsg.h"
#include "xctrlwndbase.h"

/////////////////////////////////////////////////////////////////////
// XCtrlWndBase - base functionality for xcontrol window

XCtrlWndBase::XCtrlWndBase() :
    xctrl_textColor(0),
    xctrl_bkColor(0),
    xctrl_hFont(0)
{
}

XCtrlWndBase::~XCtrlWndBase()
{
}

/////////////////////////////////////////////////////////////////////
// window procedure
/////////////////////////////////////////////////////////////////////
LRESULT XCtrlWndBase::baseWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // handle common xcontrol events
    switch(uMsg)
    {
    case WM_CREATE:
        return _onCreate((CREATESTRUCT*)lParam);
        break;

    case WM_SETTEXT:
        return _onSetText((const wchar_t*)lParam);
        break;

    case WM_GETTEXT:
        return _onGetText();
        break;

    case WM_GETTEXTLENGTH:
        return _onGetTextLength();
        break;

    case WM_ERASEBKGND:
        // ignore erase background to avoid flickering
        return 1;
        break;

    case WM_SETFONT:

        // copy font
        xctrl_hFont = (HFONT)wParam;

        // repaint if needed
        if(LOWORD(lParam) == TRUE)
            ::InvalidateRect(hwnd, 0, FALSE);

        break;

    case WM_GETFONT:
        // return active font
        return (LRESULT)xctrl_hFont;
        break;

    ///// layout helpers
    case WM_XCTRL_GET_MINWIDTH_HINT:
        return getMinWidthHint(hwnd);
        break;

    case WM_XCTRL_GET_MINHEIGHT_HINT:
        return getMinHeightHint(hwnd);
        break;

    case WM_XCTRL_GET_WIDTH_HINT:
        return getWidthHint(hwnd);
        break;

    case WM_XCTRL_GET_HEIGHT_HINT:
        return getHeightHint(hwnd);
        break;

    ///// color helpers
    case WM_XCTRL_SET_TEXTCOLOR:

        // set color
        xctrl_textColor = (COLORREF)wParam;

        // repaint if needed
        if(lParam == TRUE)
            ::InvalidateRect(hwnd, 0, FALSE);

        break;

    case WM_XCTRL_GET_TEXTCOLOR:
        // return current color
        return (LRESULT)xctrl_textColor;
        break;

    case WM_XCTRL_SET_BKCOLOR:

        // set color
        xctrl_bkColor = (COLORREF)wParam;

        // repaint if needed
        if(lParam == TRUE)
            ::InvalidateRect(hwnd, 0, FALSE);

        break;

    case WM_XCTRL_GET_BKCOLOR:
        // return current color
        return (LRESULT)xctrl_bkColor;
        break;

    }

    // pass to default window procedure
    return ::DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////
// layout helpers
/////////////////////////////////////////////////////////////////////
int XCtrlWndBase::getWidthHint(HWND hwnd) const
{
    // empty in default implementation
    return getMinWidthHint(hwnd);
}

int XCtrlWndBase::getHeightHint(HWND hwnd) const
{
    // empty in default implementation
    return getMinHeightHint(hwnd);
}

int XCtrlWndBase::getMinWidthHint(HWND hwnd) const
{
    // empty in default implementation
    return 0;
}

int XCtrlWndBase::getMinHeightHint(HWND hwnd) const
{
    // empty in default implementation
    return 0;
}

/////////////////////////////////////////////////////////////////////
// message handlers
/////////////////////////////////////////////////////////////////////
LRESULT XCtrlWndBase::_onCreate(const CREATESTRUCT* lpCreate)
{
    XWASSERT(lpCreate);

    // set default text
    xctrl_strText = lpCreate->lpszName;

    // default colors
    xctrl_textColor = XWUtils::sGetDefaultTextColor();
    xctrl_bkColor = XWUtils::sGetDefaultBackgroundColor();

    return TRUE;
}

LRESULT XCtrlWndBase::_onSetText(const wchar_t* text)
{
    // set new text
    xctrl_strText = text;

    // text has been set
    return TRUE;
}

LRESULT XCtrlWndBase::_onGetText()
{
    // copy text
    return TRUE;
}

LRESULT XCtrlWndBase::_onGetTextLength()
{
    // text length
    return xctrl_strText.length();
}

// XCtrlWndBase
/////////////////////////////////////////////////////////////////////
