// Extended controls implementation helpers
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "../layout/xlayoutitem.h"

#include "../xwindow/xhwndlayoutitem.h"

#include "xctrlmsg.h"
#include "xctrlimplbase.h"

/////////////////////////////////////////////////////////////////////
// XCtrImplBase - base functionality for xcontrol implemnetaiton

XCtrImplBase::XCtrImplBase(HWND hWnd, XWObject* parent) :
    XHWNDLayoutItem(hWnd, parent)
{
}

XCtrImplBase::~XCtrImplBase()
{
}

/////////////////////////////////////////////////////////////////////
// transparent background
/////////////////////////////////////////////////////////////////////
void XCtrImplBase::setTransparent(bool bTransparent)
{
    // current style
    LONG lExtStyle = ::GetWindowLongW(hwnd(), GWL_EXSTYLE);

    // ignore if already same
    if( isTransparent() == bTransparent ) return;

    // new flag
    if(bTransparent)
        lExtStyle |= WS_EX_TRANSPARENT;
    else
        lExtStyle &= ~((LONG)WS_EX_TRANSPARENT);

    // set new flag
    ::SetWindowLongW(hwnd(), GWL_EXSTYLE, lExtStyle);
}

bool XCtrImplBase::isTransparent()
{
    return ((::GetWindowLongW(hwnd(), GWL_EXSTYLE) & WS_EX_TRANSPARENT) == WS_EX_TRANSPARENT);
}

/////////////////////////////////////////////////////////////////////
// layout helpers
/////////////////////////////////////////////////////////////////////
int XCtrImplBase::getWidthHint() const
{
    return (int)::SendMessageW(hwnd(), WM_XCTRL_GET_WIDTH_HINT, 0, 0);
}

int XCtrImplBase::getHeightHint() const
{
    return (int)::SendMessageW(hwnd(), WM_XCTRL_GET_HEIGHT_HINT, 0, 0);
}

int XCtrImplBase::getMinWidthHint() const
{
    return (int)::SendMessageW(hwnd(), WM_XCTRL_GET_MINWIDTH_HINT, 0, 0);
}

int XCtrImplBase::getMinHeightHint() const
{
    return (int)::SendMessageW(hwnd(), WM_XCTRL_GET_MINHEIGHT_HINT, 0, 0);
}

/////////////////////////////////////////////////////////////////////
// colors
/////////////////////////////////////////////////////////////////////
bool XCtrImplBase::setTextColor(COLORREF col, BOOL bRepaint)
{
    return (::SendMessageW(hwnd(), WM_XCTRL_SET_TEXTCOLOR, col, bRepaint) == TRUE);
}

COLORREF XCtrImplBase::getTextColor() const
{
    return (COLORREF)::SendMessageW(hwnd(), WM_XCTRL_GET_TEXTCOLOR, 0, 0);
}

bool XCtrImplBase::setBackgroundColor(COLORREF col, BOOL bRepaint)
{
    // reset transparency if any
    setTransparent(false);

    return (::SendMessageW(hwnd(), WM_XCTRL_SET_BKCOLOR, col, bRepaint) == TRUE);
}

COLORREF XCtrImplBase::getBackgroundColor() const
{
    return (COLORREF)::SendMessageW(hwnd(), WM_XCTRL_GET_BKCOLOR, 0, 0);
}

// XCtrImplBase
/////////////////////////////////////////////////////////////////////
