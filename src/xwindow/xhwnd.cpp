// Window handle wrapper with layout support
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "../layout/xlayoutitem.h"

#include "xhwnd.h"

/////////////////////////////////////////////////////////////////////
// XWHWND - layout item for Windows handle (HWND)
/////////////////////////////////////////////////////////////////////
XWHWND::XWHWND(HWND hWnd, XWObject* parent) :
    m_hWnd(hWnd),
    XWObject(parent),
    m_rpHorizontal(eResizeAny),
    m_rpVertical(eResizeAny),
    m_isSubclassed(false),
    m_nMinWidth(0),
    m_nMinHeight(0),
    m_nMaxWidth(0),
    m_nMaxHeight(0),
    m_bFocusable(false),
    m_contentStart(0),
    m_contentEnd(0)
{
}

XWHWND::~XWHWND()
{
}

/////////////////////////////////////////////////////////////////////
// parent window
/////////////////////////////////////////////////////////////////////
void XWHWND::setParent(HWND hWnd)
{
    XWASSERT(m_hWnd);

    // set parent for windows
    if(m_hWnd)
    {
        // set parent
        ::SetParent(m_hWnd, hWnd);
    }
}

/////////////////////////////////////////////////////////////////////
// visibility
/////////////////////////////////////////////////////////////////////
bool XWHWND::isVisible() const
{
    return (::IsWindowVisible(m_hWnd) == TRUE);
}

void XWHWND::show(int nCmdShow)
{
    // show window
    ::ShowWindow(m_hWnd, nCmdShow);
}

void XWHWND::hide()
{
    // hide window
    ::ShowWindow(m_hWnd, SW_HIDE);
}

/////////////////////////////////////////////////////////////////////
// focus
/////////////////////////////////////////////////////////////////////
void XWHWND::setFocusable(bool bFocusable)
{
    m_bFocusable = bFocusable;
}

bool XWHWND::isFocusable() const
{
    return m_bFocusable;
}

void XWHWND::setFocus()
{
    if(m_hWnd && ::SetFocus(m_hWnd) == 0)
    {
        XWTRACE_WERR_LAST("XWHWND: failed to set focus");
    }
}

bool XWHWND::hasFocus() const
{
    // check if focused window is current
    return m_hWnd && (::GetFocus() == m_hWnd);
}

/////////////////////////////////////////////////////////////////////
// window text
/////////////////////////////////////////////////////////////////////
void XWHWND::setText(const wchar_t* text)
{
    // set window text
    ::SendMessageW(hwnd(), WM_SETTEXT, 0, (LPARAM)text);
}

int XWHWND::getText(wchar_t* text, int maxSize) const
{
    // get window text
    return (int)::SendMessageW(hwnd(), WM_GETTEXT, maxSize, (LPARAM)text);
}

int XWHWND::getTextLength() const
{
    // get window text length
    return (int)::SendMessageW(hwnd(), WM_GETTEXTLENGTH, 0, 0);
}

/////////////////////////////////////////////////////////////////////
// fonts
/////////////////////////////////////////////////////////////////////
void XWHWND::setDefaultFont()
{
    // ignore if window not set
    if(!hwnd()) return;

    // get default font
    HFONT hFont = XWUtils::sGetDefaultFont();

    // set control default font
    ::SendMessageW(hwnd(), WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
}

void XWHWND::setFont(HFONT hFont, BOOL bRepaint)
{
    // set control font
    ::SendMessageW(hwnd(), WM_SETFONT, (WPARAM)hFont, MAKELPARAM(bRepaint, 0));
}

HFONT XWHWND::getFont()
{
    // get control font
    return (HFONT)::SendMessageW(hwnd(), WM_GETFONT, 0, 0);
}

/////////////////////////////////////////////////////////////////////
// style
/////////////////////////////////////////////////////////////////////
LONG XWHWND::style() const
{
    // get style from window
    LONG style = ::GetWindowLongW(hwnd(), GWL_STYLE);

    if(style == 0)
    {
        // fatal error
        XWTRACE_WERR_LAST("XWHWND: Failed to get window style from window");
    }

    return style;
}

bool XWHWND::setStyle(LONG lStyle)
{
    // set new style
    if(::SetWindowLongPtrW(hwnd(), GWL_STYLE, lStyle) == 0)
    {
        // fatal error
        XWTRACE_WERR_LAST("XWHWND: Failed to set window style for window");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////
// WM_COMMAND based notifications
/////////////////////////////////////////////////////////////////////
XWEventMask XWHWND::mkCommandEvent(WORD wNotifyCode) const
{
    XWEventMask mask;

    // message mask
    mask.setWindowMessage(WM_COMMAND);
    mask.setWParamHiWord(wNotifyCode);
    mask.setLParam((LPARAM)hwnd());

    return mask;
}

XWEventMask XWHWND::mkMenuEvent(UINT itemId) const
{
    XWEventMask mask;

    // message mask
    mask.setWindowMessage(WM_COMMAND);
    mask.setWParamHiWord(0);
    mask.setWParamLoWord(itemId);

    return mask;
}

void XWHWND::notifyParentWindow(WORD wNotifyCode, WORD wControlId)
{
    // check if window is ready
    if(hwnd())
    {
        // notify parent if set
        HWND hwndParent = ::GetParent(hwnd());

        if(hwndParent)
            ::SendMessageW(hwndParent, WM_COMMAND, MAKEWPARAM(wControlId, wNotifyCode), (LPARAM)hwnd());
    }
}

void XWHWND::notifyParentMenuItem(UINT itemId)
{
    // check if window is ready
    if(hwnd())
    {
        // notify parent if set
        HWND hwndParent = ::GetParent(hwnd());

        if(hwndParent)
            ::SendMessageW(hwndParent, WM_COMMAND, MAKEWPARAM(itemId, 0), 0);
    }
}

/////////////////////////////////////////////////////////////////////
// set size constraints
/////////////////////////////////////////////////////////////////////
void XWHWND::setMinWidth(int minWidth)
{
    XWASSERT(minWidth >= 0);

    // verify window size
    if(width() < minWidth)
    {
        // resize window
        resize(minWidth, height());
    }

    // set minimum width
    m_nMinWidth = minWidth;

    // update policy
    m_rpHorizontal = (m_rpHorizontal == eResizeMax) ? eResizeMinMax : eResizeMin;
}

void XWHWND::setMinHeight(int minHeight)
{
    XWASSERT(minHeight >= 0);

    // verify window size
    if(height() < minHeight)
    {
        // resize window
        resize(width(), minHeight);
    }

    // set height
    m_nMinHeight = minHeight;

    // update policy
    m_rpVertical = (m_rpVertical == eResizeMax) ? eResizeMinMax : eResizeMin;
}

void XWHWND::setMaxWidth(int maxWidth)
{
    XWASSERT(maxWidth >= 0);

    // verify window size
    if(width() > maxWidth)
    {
        // resize window
        resize(maxWidth, height());
    }

    // set width
    m_nMaxWidth = maxWidth;

    // update policy
    m_rpHorizontal = (m_rpHorizontal == eResizeMin) ? eResizeMinMax : eResizeMax;
}

void XWHWND::setMaxHeight(int maxHeight)
{
    XWASSERT(maxHeight >= 0);

    // verify window size
    if(height() > maxHeight)
    {
        // resize window
        resize(width(), maxHeight);
    }

    // set height
    m_nMaxHeight = maxHeight;

    // update policy
    m_rpVertical = (m_rpVertical == eResizeMin) ? eResizeMinMax : eResizeMax;
}

/////////////////////////////////////////////////////////////////////
// fix size
/////////////////////////////////////////////////////////////////////
void XWHWND::setFixedWidth(int width)
{
    // set limits
    m_nMinWidth = width;
    m_nMaxWidth = width;

    // update policy
    m_rpHorizontal = eResizeMinMax;

    // resize window
    resize(width, height());
}

void XWHWND::setFixedHeight(int height)
{
    // set limits
    m_nMinHeight = height;
    m_nMaxHeight = height;

    // update policy
    m_rpVertical = eResizeMinMax;

    // resize window
    resize(width(), height);
}

void XWHWND::setFixedSize(int width, int height)
{
    // set limits
    m_nMinWidth = width;
    m_nMaxWidth = width;
    m_nMinHeight = height;
    m_nMaxHeight = height;

    // update policy
    m_rpHorizontal = eResizeMinMax;
    m_rpVertical = eResizeMinMax;

    // resize window
    resize(width, height);
}

/////////////////////////////////////////////////////////////////////
// reset size policy
/////////////////////////////////////////////////////////////////////
void XWHWND::resetSizePolicy()
{
    m_rpHorizontal = eResizeAny;
    m_rpVertical = eResizeAny;
}

/////////////////////////////////////////////////////////////////////
// size (from IXLayoutItem)
/////////////////////////////////////////////////////////////////////
void XWHWND::getSize(int& width, int& height)
{
    XWASSERT(m_hWnd);

    // get size
    RECT rec;
    if(::GetWindowRect(m_hWnd, &rec))
    {
        // count size
        width = (rec.right - rec.left);
        height = (rec.bottom - rec.top);

    } else
    {
        XWTRACE("XWHWND::getSize failed to get window size");

        // reset size
        width = 0;
        height = 0;
    }
}

int XWHWND::width()
{
    int width, height;

    // read size
    getSize(width, height);

    // return width
    return width;
}

int XWHWND::height()
{
    int width, height;

    // read size
    getSize(width, height);

    // return height
    return height;
}

/////////////////////////////////////////////////////////////////////
// window position
/////////////////////////////////////////////////////////////////////
void XWHWND::getRect(RECT& rect)
{
    XWASSERT(m_hWnd);

    // get rect
    if(!::GetWindowRect(m_hWnd, &rect))
    {
        XWTRACE("XWHWND::getRect failed to get window rect");

        // reset output
        ::ZeroMemory(&rect, sizeof(RECT));
    }
}

void XWHWND::getPos(LONG& posX, LONG& posY)
{
    // get rect
    RECT rect;
    getRect(rect);

    // window position
    posX = rect.left;
    posY = rect.top;
}

/////////////////////////////////////////////////////////////////////
// window position relative to other window
/////////////////////////////////////////////////////////////////////
void XWHWND::getRelativeRect(HWND hwnd, RECT& rect)
{
    // get rect in screen coordinates
    getRect(rect);

    // convert position to other window coordinates
    XWUtils::screenToClient(hwnd, rect.left, rect.top);
    XWUtils::screenToClient(hwnd, rect.right, rect.bottom);
}

void XWHWND::getRelativePos(HWND hwnd, LONG& posX, LONG& posY)
{
    // get position in screen coordinates
    getPos(posX, posY);

    // convert position to other window coordinates
    XWUtils::screenToClient(hwnd, posX, posY);
}

/////////////////////////////////////////////////////////////////////
// client size 
/////////////////////////////////////////////////////////////////////
void XWHWND::getClientSize(int& width, int& height)
{
    XWASSERT(m_hWnd);

    // get size
    RECT rec;
    if(::GetClientRect(m_hWnd, &rec))
    {
        // count size
        width = (rec.right - rec.left);
        height = (rec.bottom - rec.top);

    } else
    {
        XWTRACE("XWHWND::getClientSize failed to get window client size");

        // reset size
        width = 0;
        height = 0;
    }
}

int XWHWND::clientWidth()
{
    int width, height;

    // read size
    getClientSize(width, height);

    // return width
    return width;
}

int XWHWND::clientHeight()
{
    int width, height;

    // read size
    getClientSize(width, height);

    // return height
    return height;
}

/////////////////////////////////////////////////////////////////////
// state
/////////////////////////////////////////////////////////////////////
void XWHWND::enable(BOOL bEnable)
{
    // enable window
    ::EnableWindow(m_hWnd, bEnable);

    // repaint
    repaint();
}

bool XWHWND::isEnabled()
{
    return (::IsWindowEnabled(m_hWnd) == TRUE);
}

/////////////////////////////////////////////////////////////////////
// window methods
/////////////////////////////////////////////////////////////////////
void XWHWND::resize(int width, int height)
{
    XWASSERT(m_hWnd);

    // respect width constrains
    if(m_rpHorizontal != eResizeAny)
    {
        // check policy
        if((m_rpHorizontal == eResizeMinMax || 
           m_rpHorizontal == eResizeMin) && width < minWidth())
        {
            // use minimum width
            width = minWidth();

        } else if((m_rpHorizontal == eResizeMinMax || 
           m_rpHorizontal == eResizeMax) && width > maxWidth())
        {
            // use maximum width
            width = maxWidth();
        }
    }

    // respect height constrains
    if(m_rpVertical != eResizeAny)
    {
        // check policy
        if((m_rpVertical == eResizeMinMax || 
           m_rpVertical == eResizeMin) && height < minHeight())
        {
            // use minimum height
            height = minHeight();

        } else if((m_rpVertical == eResizeMinMax || 
           m_rpVertical == eResizeMax) && height > maxHeight())
        {
            // use maximum height
            height = maxHeight();
        }
    }

    // get current size
    RECT rec;
    if(::GetWindowRect(m_hWnd, &rec))
    {
        POINT pt;
        pt.x = rec.left;
        pt.y = rec.top;

        // convert position to parent window coordinates
        if(::GetWindowLong(m_hWnd, GWL_STYLE) & WS_CHILD)
        {
            ::ScreenToClient(::GetParent(m_hWnd), &pt);
        }

        // resize
        if(::MoveWindow(m_hWnd, pt.x, pt.y, width, height, FALSE)) return;
    }

    XWTRACE("XWHWND::resize failed to resize window");
}

void XWHWND::move(int posX, int posY)
{
    XWASSERT(m_hWnd);
    if(m_hWnd == 0) return;

    // move window
    if(!::SetWindowPos(m_hWnd, 0, posX, posY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOREDRAW))
    {
        XWTRACE("XWHWND::move failed to move window");
    }
}

// helper callback
static BOOL CALLBACK _repaintCallbackProc(HWND hwnd, LPARAM lParam)
{
    // request WM_PAINT
    ::InvalidateRect(hwnd, 0, FALSE);

    return TRUE;
}

void XWHWND::repaint(BOOL paintNow)
{
    // ignore if window not set
    if(m_hWnd == 0) return;

    if(paintNow)
    {
        // redraw window and its schildren
        ::RedrawWindow(m_hWnd, 0, 0, RDW_UPDATENOW | RDW_ALLCHILDREN);

    } else
    {
        // process main window first
        ::InvalidateRect(m_hWnd, 0, FALSE);

        // NOTE: as MSDN states: https://msdn.microsoft.com/en-us/library/windows/desktop/dd183426(v=vs.85).aspx
        //       Similarly, an application cannot generate a WM_PAINT message for the child by invalidating 
        //       a portion of the parent's client area that lies entirely under the child window. In such 
        //       cases, neither window receives a WM_PAINT message.


        // request WM_PAINT for all child windows as well
        ::EnumChildWindows(m_hWnd, _repaintCallbackProc, 0);
    }
}

/////////////////////////////////////////////////////////////////////
// scrolling
/////////////////////////////////////////////////////////////////////
int XWHWND::contentWidth()
{
    // reset content counters
    m_contentStart = 0;
    m_contentEnd = 0;

    // compute content width
    ::EnumChildWindows(m_hWnd, _contentWidthCallbackProc, (LPARAM)this);

    // in case there are no child windows use width
    return (m_contentEnd > m_contentStart) ? (m_contentEnd - m_contentStart) : width();
}

int XWHWND::contentHeight()
{
    // reset content counters
    m_contentStart = 0;
    m_contentEnd = 0;

    // compute content width
    ::EnumChildWindows(m_hWnd, _contentHeightCallbackProc, (LPARAM)this);

    // in case there are no child windows use height
    return (m_contentEnd > m_contentStart) ? (m_contentEnd - m_contentStart) : height();
}

void XWHWND::setScrollOffsetX(int scrollOffsetX)
{
    // pass to parent
    IXWScrollable::setScrollOffsetX(scrollOffsetX);
}

void XWHWND::setScrollOffsetY(int scrollOffsetY)
{
    // pass to parent
    IXWScrollable::setScrollOffsetY(scrollOffsetY);
}

/////////////////////////////////////////////////////////////////////
// manipulations (from IXLayoutItem)
/////////////////////////////////////////////////////////////////////
void XWHWND::update(int posX, int posY, int width, int height)
{
    XWASSERT(m_hWnd);

    // update size
    if(::MoveWindow(m_hWnd, posX, posY, width, height, TRUE) == 0)
    {
        XWTRACE_WERR_LAST("XWHWND: failed to move window");
    }
}

/////////////////////////////////////////////////////////////////////
// resize policy (from IXLayoutItem)
/////////////////////////////////////////////////////////////////////
IXLayoutItem::TResizePolicy XWHWND::horizontalPolicy()  const
{
    return m_rpHorizontal;
}

IXLayoutItem::TResizePolicy XWHWND::verticalPolicy()  const
{
    return m_rpVertical;
}

/////////////////////////////////////////////////////////////////////
// size constraints (from IXLayoutItem)
/////////////////////////////////////////////////////////////////////
int XWHWND::minWidth()  const
{
    return m_nMinWidth;
}

int XWHWND::minHeight()  const
{
    return m_nMinHeight;
}

int XWHWND::maxWidth()  const
{
    return (m_nMaxWidth > 0) ? m_nMaxWidth : IXLayoutItem::maxWidth();
}

int XWHWND::maxHeight()  const
{
    return (m_nMaxHeight > 0) ? m_nMaxHeight : IXLayoutItem::maxHeight();
}

/////////////////////////////////////////////////////////////////////
// subclassing
/////////////////////////////////////////////////////////////////////
bool XWHWND::isSubclassed() const
{
    return m_isSubclassed;
}

bool XWHWND::subclassWindow()
{
    XWASSERT(m_hWnd);

    // ignore if already subclassed
    if(isSubclassed()) return true;

    // subclass
    if(!::SetWindowSubclass(m_hWnd, sXSubclassWindowProc, xwoid(), (DWORD_PTR)this))
    {
        XWTRACE_WERR_LAST("XWHWND: failed to subclass window");
        return false;
    }

    // set flag
    m_isSubclassed = true;

    return true;
}

bool XWHWND::removeSubclass()
{
    XWASSERT(m_hWnd);

    // ignore if not subclassed
    if(!isSubclassed()) return true;

    // remove subclass
    if(!::RemoveWindowSubclass(m_hWnd, sXSubclassWindowProc, xwoid()))
    {
        XWTRACE_WERR_LAST("XWHWND: failed to remove window subclass");
        return false;
    }

    // set flag
    m_isSubclassed = false;

    return true;
}

/////////////////////////////////////////////////////////////////////
// process messages (in case window has been subclassed)
/////////////////////////////////////////////////////////////////////
LRESULT XWHWND::processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // pass to default window procedure
    return ::DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////
// subclass window procedure
/////////////////////////////////////////////////////////////////////
LRESULT CALLBACK XWHWND::sXSubclassWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    // get pointer
    XWHWND* xhwnd = (XWHWND*)dwRefData;
    if(xhwnd) 
    {
        // process messages
        return xhwnd->processMessage(hwnd, uMsg, wParam, lParam);
    }

    // pass to default window procedure
    return ::DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////
// content callbacks
/////////////////////////////////////////////////////////////////////
BOOL CALLBACK XWHWND::_contentWidthCallbackProc(HWND hwnd, LPARAM lParam)
{
    XWASSERT(hwnd);
    XWASSERT(lParam);

    XWHWND* xwhwnd = (XWHWND*)lParam;
    if(xwhwnd == 0 || hwnd == 0) return FALSE;

    // get child window rect
    RECT rect;
    if(::GetWindowRect(hwnd, &rect))
    {
        // convert to parent window coordinates
        XWUtils::screenToClient(xwhwnd->hwnd(), rect.left, rect.top);
        XWUtils::screenToClient(xwhwnd->hwnd(), rect.right, rect.bottom);

        // update content counters
        if(xwhwnd->m_contentStart > rect.left ) xwhwnd->m_contentStart = rect.left;
        if(xwhwnd->m_contentEnd < rect.right) xwhwnd->m_contentEnd = rect.right;
    }

    return TRUE;
}

BOOL CALLBACK XWHWND::_contentHeightCallbackProc(HWND hwnd, LPARAM lParam)
{
    XWASSERT(hwnd);
    XWASSERT(lParam);

    XWHWND* xwhwnd = (XWHWND*)lParam;
    if(xwhwnd == 0 || hwnd == 0) return FALSE;

    // get child window rect
    RECT rect;
    if(::GetWindowRect(hwnd, &rect))
    {
        // convert to parent window coordinates
        XWUtils::screenToClient(xwhwnd->hwnd(), rect.left, rect.top);
        XWUtils::screenToClient(xwhwnd->hwnd(), rect.right, rect.bottom);

        // update content counters
        if(xwhwnd->m_contentStart > rect.top ) xwhwnd->m_contentStart = rect.top;
        if(xwhwnd->m_contentEnd < rect.bottom) xwhwnd->m_contentEnd = rect.bottom;
    }

    return TRUE;
}

// XWHWND
/////////////////////////////////////////////////////////////////////

