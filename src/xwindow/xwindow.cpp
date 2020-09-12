// xWindow functionality
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "../layout/xlayoutitem.h"
#include "../layout/xlayout.h"

#include "xhwnd.h"
#include "xwindow.h"

// helper controls
#include "../ctrls/xpopupmenu.h"
#include "../ctrls/xtooltip.h"

/////////////////////////////////////////////////////////////////////
// constants
#define XWUI_CLASS_NAME     L"XWUI_WINDOW_CLASS"

/////////////////////////////////////////////////////////////////////
// XWindow - window functionality
/////////////////////////////////////////////////////////////////////

XWindow::XWindow(XWObject* parent) :
    XWHWND(0, parent),
    m_nSysMarginX(0),
    m_nSysMarginY(0),
    m_contextMenu(0),
    m_toolTip(0),
    m_showContextMenu(true),
    m_contextMenuActive(false),
    m_pLayout(0),
    m_bgColor(XWUtils::sGetDefaultBackgroundColor()), // default background color
    m_bEraseBackground(false),
    m_bMessageHandled(false),
    m_bMouseOver(false),
    m_bMouseTracking(false),
    m_mouseHoverX(0),
    m_mouseHoverY(0)
{
    // do nothing in default constructor
}

XWindow::XWindow(DWORD dwStyle, XWObject* parent, HWND hWndParent, DWORD dwExStyle) :
    XWHWND(0, parent),
    m_nSysMarginX(0),
    m_nSysMarginY(0),
    m_contextMenu(0),
    m_toolTip(0),
    m_showContextMenu(true),
    m_contextMenuActive(false),
    m_pLayout(0),
    m_bgColor(XWUtils::sGetDefaultBackgroundColor()), // default background color
    m_bEraseBackground(false),
    m_bMessageHandled(false),
    m_bMouseOver(false),
    m_bMouseTracking(false),
    m_mouseHoverX(0),
    m_mouseHoverY(0)
{
    // create window
    create(dwStyle, hWndParent, dwExStyle);
}

XWindow::~XWindow()
{
    // destroy window
    destroy();

    // close all resources if any
    _closeResources();
}

/////////////////////////////////////////////////////////////////////
// window creation
/////////////////////////////////////////////////////////////////////
bool XWindow::create(DWORD dwStyle, HWND hWndParent, DWORD dwExStyle)
{
    // register class
    if(!sRegisterXWindowClass()) 
    {
        // fatal error
        return false;
    }

    // create window
    m_hWnd = CreateWindowExW(
        dwExStyle,
        XWUI_CLASS_NAME,
        L"",
        dwStyle,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hWndParent, 
        NULL, 
        ::GetModuleHandle(NULL), 
        this);

    if(m_hWnd == NULL)
    {
        // fatal error
        XWTRACE_WERR_LAST("XWindow Failed to create window");
        return false;
    }

    // update margins
    _updateSystemsMargins();

    // TODO: update default background color

    return true;
}

void XWindow::destroy()
{
    // destroy window if set
    if(m_hWnd)
    {
        // destroy window (will send WM_DESTROY)
        ::DestroyWindow(m_hWnd);
    }
}

/////////////////////////////////////////////////////////////////////
// layout
/////////////////////////////////////////////////////////////////////
void XWindow::setLayout(IXLayout* pLayout)
{
    // destroy old layout if set
    delete m_pLayout;
    m_pLayout = 0;

    // set new layout
    m_pLayout = pLayout;

    // layout items
    onContentChanged();
}

/////////////////////////////////////////////////////////////////////
// background
/////////////////////////////////////////////////////////////////////
void XWindow::enableEraseBackground(bool bEnable)
{
    // set flag
    m_bEraseBackground = bEnable;
}

void XWindow::setBackgroundColor(COLORREF color)
{
    // set color
    m_bgColor = color;
}

/////////////////////////////////////////////////////////////////////
// content 
/////////////////////////////////////////////////////////////////////
void XWindow::handleContentChanged()
{
    // update layout
    onContentChanged();

    if(m_hWnd)
    {
        // check if window has a parent
        HWND hWndParent = ::GetParent(m_hWnd);
        if(hWndParent)
        {
            // inform parent
            ::PostMessageW(hWndParent, WM_XWUI_CHILD_CONTENT_CHANGED, 0, 0);
        }
    }
}

/////////////////////////////////////////////////////////////////////
// request event
/////////////////////////////////////////////////////////////////////
void XWindow::postEventRequest(DWORD dwEventId)
{
    if(m_hWnd)
    {
        // request event
        ::PostMessageW(m_hWnd, WM_XWUI_CALLBACK_EVENT_REQUEST, dwEventId, 0);

    } else
    {
        // send event directly
        sendEvent(dwEventId);
    }
}

/////////////////////////////////////////////////////////////////////
// scrolling (from IXWScrollable)
/////////////////////////////////////////////////////////////////////
int XWindow::contentWidth()
{
    // check if layout is set
    if(m_pLayout)
    {
        // use minimum width from layout
        return m_pLayout->minWidth();

    } else
    {
        // pass to parent
        return XWHWND::contentWidth();
    }
}

int XWindow::contentHeight()
{
    // check if layout is set
    if(m_pLayout)
    {
        // use minimum height from layout
        return m_pLayout->minHeight();

    } else
    {
        // pass to parent
        return XWHWND::contentHeight();
    }
}

/////////////////////////////////////////////////////////////////////
// visibility (from XWHWND)
/////////////////////////////////////////////////////////////////////
void XWindow::show(int nCmdShow)
{
    // pass to parent first
    XWHWND::show(nCmdShow);

    // update layout (if window was hidden it may not have updated its layout properly)
    onContentChanged();

    // NOTE: sometimes there are strange problems when window is first shown 
    //       or restored from minimzed state: some child controls do not get WM_PAINT 
    //       message and so are not re-painted.

    // request re-paint for all child windows
    repaint(FALSE);
}

/////////////////////////////////////////////////////////////////////
// context menu
/////////////////////////////////////////////////////////////////////
void XWindow::setContextMenu(XPopupMenu* contextMenu)
{
    // delete old menu if any
    if(m_contextMenu) delete m_contextMenu;

    // copy menu (may be null)
    m_contextMenu = contextMenu;

    // init menu
    if(m_contextMenu)
    {
        // update parent object
        m_contextMenu->setParentObject(this);
    }
}

void XWindow::enableContextMenu(bool enable)
{
    // set flag
    m_showContextMenu = enable;
}

XPopupMenu* XWindow::contextMenu()
{
    return m_contextMenu;
}

/////////////////////////////////////////////////////////////////////
// tooltip
/////////////////////////////////////////////////////////////////////
void XWindow::showToolTip(const wchar_t* text)
{
    // create tool tip if needed
    if(m_toolTip == 0)
    {
        m_toolTip = new XToolTip(hwnd(), this);
        m_toolTip->enableToolLessMode();
    }

    // show
    m_toolTip->showToolLess(text);
}

void XWindow::hideToolTip()
{
    // ignore if not set or not visible
    if(m_toolTip == 0 || !m_toolTip->isShownToolLess()) return;

    // hide
    m_toolTip->hideToolLess();
}

/////////////////////////////////////////////////////////////////////
// resize policy (from XWHWND)
/////////////////////////////////////////////////////////////////////
IXLayoutItem::TResizePolicy XWindow::horizontalPolicy() const
{
    // use item policy if set or no layout
    if(XWHWND::horizontalPolicy() != eResizeAny || m_pLayout == 0)
    {
        return XWHWND::horizontalPolicy();
    }

    // use layout
    return m_pLayout->horizontalPolicy();
}

IXLayoutItem::TResizePolicy XWindow::verticalPolicy() const
{
    // use item policy if set or no layout
    if(XWHWND::verticalPolicy() != eResizeAny || m_pLayout == 0)
    {
        return XWHWND::verticalPolicy();
    }

    // use layout
    return m_pLayout->verticalPolicy();
}

void XWindow::updateResizePolicies()
{
    // pass to layout if set
    if(m_pLayout)
    {
        m_pLayout->updateResizePolicies();
    }
}

/////////////////////////////////////////////////////////////////////
// size constraints (from XWHWND)
/////////////////////////////////////////////////////////////////////
int XWindow::minWidth()  const
{
    // use item width if set or no layout
    if(XWHWND::horizontalPolicy() == eResizeMin ||
       XWHWND::horizontalPolicy() == eResizeMinMax ||
       m_pLayout == 0)
    {
        return XWHWND::minWidth();
    }

    // use layout 
    return m_pLayout->minWidth();
}

int XWindow::minHeight() const
{
    // use item width if set or no layout
    if(XWHWND::verticalPolicy() == eResizeMin ||
       XWHWND::verticalPolicy() == eResizeMinMax ||
       m_pLayout == 0)
    {
        return XWHWND::minHeight();
    }

    // use layout 
    return m_pLayout->minHeight();
}

int XWindow::maxWidth()  const
{
    // use item width if set or no layout
    if(XWHWND::horizontalPolicy() == eResizeMax ||
       XWHWND::horizontalPolicy() == eResizeMinMax ||
       m_pLayout == 0)
    {
        return XWHWND::maxWidth();
    }

    // use layout 
    return m_pLayout->maxWidth();
}

int XWindow::maxHeight() const
{
    // use item width if set or no layout
    if(XWHWND::verticalPolicy() == eResizeMax ||
       XWHWND::verticalPolicy() == eResizeMinMax ||
       m_pLayout == 0)
    {
        return XWHWND::maxHeight();
    }

    // use layout 
    return m_pLayout->maxHeight();
}

/////////////////////////////////////////////////////////////////////
// add message events
/////////////////////////////////////////////////////////////////////
void XWindow::connect(const XWEventMask& mask, const XWEventDelegate& handler)
{
    // add to map
    m_xEventMap.addEventHandler(mask, handler);
}

void XWindow::connect(const UINT uMsg, const XWEventDelegate& handler)
{
    // connect mask
    connect(XWEventMask(uMsg), handler);
}

/////////////////////////////////////////////////////////////////////
// common events
/////////////////////////////////////////////////////////////////////
void XWindow::onCreate(HWND hwnd, const CREATESTRUCT* pCreateStruct)
{
    // empty default implementation
}

void XWindow::onPaint()
{
    // init painting
    PAINTSTRUCT  ps;
    HDC hdc = ::BeginPaint(hwnd(), &ps);
        
    // erase background if needed
    if(m_bEraseBackground)
    {
        eraseBackground(hdc, ps.rcPaint, m_bgColor);
    }

    // handle window painting
    onPaint(hdc, ps);

    ::EndPaint(hwnd(), &ps);
}

void XWindow::onPaint(HDC hdc, PAINTSTRUCT& ps)
{
    // empty default implementation
}

void XWindow::onMove(int posX, int posY)
{
    // empty default implementation
}

void XWindow::onResize(int type, int width, int height)
{
    // empty default implementation
}

void XWindow::onDestroy()
{
    // close all resources if any
    _closeResources();
}

void XWindow::onContentChanged()
{
    // update layout if set
    if(m_pLayout)
    {
        // get current size
        RECT rec;
        if(::GetClientRect(m_hWnd, &rec))
        {
            // adjust child windows 
            m_pLayout->update(0, 0, rec.right - rec.left, rec.bottom - rec.top);
        }
    }

    // repaint
    repaint();
}

bool XWindow::onTimer(WPARAM uIDEvent)
{
    // ignore event
    return false;
}

/////////////////////////////////////////////////////////////////////
// mouse events
/////////////////////////////////////////////////////////////////////
void XWindow::onMouseEnter(int posX, int posY)
{
    // empty default implementation
}

void XWindow::onMouseMove(int posX, int posY, WPARAM flags)
{
    // empty default implementation
}

void XWindow::onMouseHover(int posX, int posY)
{
    // empty default implementation
}

bool XWindow::onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags)
{
    // empty default implementation
    return false;
}

bool XWindow::onMouseCaptureChanged()
{
    // empty default implementation
    return false;
}

void XWindow::onMouseLeave()
{
    // empty default implementation
}

bool XWindow::onMouseWheel(int wheelDelta, int posX, int posY, WPARAM flags)
{
    // empty default implementation
    return false;
}

bool XWindow::onSetCursor()
{
    // empty default implementation
    return false;
}

/////////////////////////////////////////////////////////////////////
// focus events
/////////////////////////////////////////////////////////////////////
void XWindow::onSetFocus()
{
    // empty default implementation
}

void XWindow::onKillFocus()
{
    // empty default implementation
}

/////////////////////////////////////////////////////////////////////
// keyboard events (sent only to focused item)
/////////////////////////////////////////////////////////////////////
bool XWindow::onCharEvent(WPARAM charCode, LPARAM flags)
{
    // empty default implementation
    return false;
}

/////////////////////////////////////////////////////////////////////
// context menu
/////////////////////////////////////////////////////////////////////
void XWindow::onContextMenuItem(UINT itemId)
{
    // notify parent by default
    notifyParentMenuItem(itemId);
}

bool XWindow::onContextMenu(HWND hwndContext, int posX, int posY)
{
    // ignore if menu not set
    if(m_contextMenu == 0 || !m_showContextMenu) return false;

    // show menu
    doShowContextMenu(m_contextMenu, posX, posY);

    // consume event
    return true;
}

void XWindow::doShowContextMenu(XPopupMenu* contextMenu, int posX, int posY)
{
    XWASSERT(contextMenu);
    if(contextMenu == 0) return;

    // change cursor
    HCURSOR originalCursor = ::SetCursor(XWUtils::getSystemCursor(XWUtils::eCursorArrow));

    // set flag
    m_contextMenuActive = true;

    // track menu
    UINT itemId = 0;
    if(contextMenu->track(hwnd(), posX, posY, itemId))
    {
        // process selected item
        onContextMenuItem(itemId);
    }

    // set original cursor back
    if(originalCursor)
        ::SetCursor(originalCursor);

    // reset flag
    m_contextMenuActive = false;
}

/////////////////////////////////////////////////////////////////////
// message processing
/////////////////////////////////////////////////////////////////////
LRESULT XWindow::processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // handle common events
    switch(uMsg)
    {
    case WM_MOUSEMOVE:
        // check if mouse just entered
        if(!m_bMouseOver)
        {
            // mark flag
            m_bMouseOver = true;

            // start tracking for mouse hover and leave messages
            _startMouseTracking();

            // handle mouse enter
            onMouseEnter(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

        } else if(!m_bMouseTracking)
        {
            // start tracking only if mouse has moved from last hover point
            if(GET_X_LPARAM(lParam) != m_mouseHoverX || 
               GET_Y_LPARAM(lParam) != m_mouseHoverY)
            {
                _startMouseTracking();
            }
        }

        // hide tool tip if any
        if(m_toolTip && m_toolTip->isShownToolLess())
        {
            // assume tooltip is shown in mousehover event
            if(GET_X_LPARAM(lParam) != m_mouseHoverX || 
               GET_Y_LPARAM(lParam) != m_mouseHoverY)
            {
                // hide
                m_toolTip->hideToolLess();
            }
        }

        // handle mouse move
        onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);

        // consume
        return 0;
        break;

    case WM_MOUSELEAVE:
        // handle mouse leave
        onMouseLeave();

        // mark flag
        m_bMouseOver = false;

        // NOTE: all tracking requested by TrackMouseEvent is canceled when this message is generated
        m_bMouseTracking = false;

        // consume
        return 0;
        break;

    case WM_MOUSEHOVER:
        // handle mouse hover
        onMouseHover(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

        // remember position
        m_mouseHoverX = GET_X_LPARAM(lParam);
        m_mouseHoverY = GET_Y_LPARAM(lParam);

        // NOTE: as MSDN states mouse tracking stops after WM_MOUSEHOVER
        m_bMouseTracking = false;

        // consume
        return 0;
        break;

    case WM_CAPTURECHANGED:
        if(onMouseCaptureChanged()) return 0;
        break;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_XBUTTONDBLCLK:
        // handle mouse click
        if(onMouseClick(uMsg, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam)) return 0;
        break;

    case WM_MOUSEWHEEL:
        // handle mouse wheel event
        if(onMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam), 
                        GET_X_LPARAM(lParam), 
                        GET_Y_LPARAM(lParam), 
                        GET_KEYSTATE_WPARAM(wParam))) return 0;
        break;

    case WM_CONTEXTMENU:
        // process message
        if(onContextMenu((HWND)wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))) return 0;
        break;

    case WM_SETCURSOR:
        // block cursor from changing while content menu is active
        if(m_contextMenuActive) return TRUE;

        // handle event
        if(onSetCursor()) return TRUE;
        break;

    case WM_CHAR:
        // handle event
        if(onCharEvent(wParam, lParam)) return 0;
        break;

    case WM_SETFOCUS:
        // check if focusable
        if(isFocusable())
        {
            // handle event
            onSetFocus();

            // consume 
            return 0;
        }
        break;

    case WM_KILLFOCUS:
        // check if focusable
        if(isFocusable())
        {
            // handle event
            onKillFocus();

            // consume 
            return 0;
        }
        break;

    case WM_TIMER:
        // handle event
        if(onTimer(wParam)) return 0;
        break;

    ///// special XWindow messages
    
    case WM_XWUI_CHILD_CONTENT_CHANGED:
        // handle content changed
        onContentChanged();
        break;

    case WM_XWUI_CALLBACK_EVENT_REQUEST:
        // send event
        sendEvent((unsigned long)wParam);

        // consume
        return 0;
        break;
    }

    // pass to default function
    return ::DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////
// timers
/////////////////////////////////////////////////////////////////////
bool XWindow::startTimer(WPARAM uIDEvent, UINT uElapseMs)
{
    // ignore if window not set
    if(m_hWnd == 0) return false;

    if(::SetTimer(m_hWnd, uIDEvent, uElapseMs, 0) == 0)
    {
        XWTRACE_WERR_LAST("XWindow: failed to start timer");
        return false;
    }

    return true;
}

bool XWindow::stopTimer(WPARAM uIDEvent)
{
    // ignore if window not set
    if(m_hWnd == 0) return false;

    if(::KillTimer(m_hWnd, uIDEvent) == 0)
    {
        XWTRACE_WERR_LAST("XWindow: failed to stop timer");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////
// mark message as processed
/////////////////////////////////////////////////////////////////////
void XWindow::setMessageProcessed()
{
    // mark message as processed
    m_bMessageHandled = true;
}

bool XWindow::isMessageProcessed()
{
    return m_bMessageHandled;
}

/////////////////////////////////////////////////////////////////////
// background
/////////////////////////////////////////////////////////////////////
void XWindow::eraseBackground(HDC hdc, RECT& rect, COLORREF color)
{
    // store old color
    COLORREF oldcr = ::SetBkColor(hdc, color);

    // use text out as fastest way to fill background
    ::ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &rect, L"", 0, 0);

    // set color back
    ::SetBkColor(hdc, oldcr);
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XWindow::_closeResources()
{
    // destroy layout
    delete m_pLayout;
    m_pLayout = 0;
}

void XWindow::_updateSystemsMargins()
{
    RECT clientRect, windowRect;

    // get metrics
    if(::GetWindowRect(m_hWnd, &windowRect) == 0 ||
       ::GetClientRect(m_hWnd, &clientRect) == 0)
    {
        XWTRACE_WERR_LAST("XWindow Failed to get system metrics");

        // reset margins
        m_nSysMarginX = 0;
        m_nSysMarginY = 0;
        return;
    }

    // update margins
    m_nSysMarginX = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
    m_nSysMarginY = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);
}

void XWindow::_onWindowResized(int width, int height)
{
    // update layout if set
    if(m_pLayout)
    {
        // adjust windows
        m_pLayout->update(0, 0, width, height);
    }
}

void XWindow::_onMinMaxInfoRequested(WPARAM wParam, LPARAM lParam)
{
    MINMAXINFO* pMinMaxInfo = (MINMAXINFO*)lParam;

    // check if horizontal policy is set
    if(horizontalPolicy() != eResizeAny)
    {
        if(horizontalPolicy() == eResizeMin ||
           horizontalPolicy() == eResizeMinMax)
        {
            // set minimum width
            pMinMaxInfo->ptMinTrackSize.x = minWidth() + m_nSysMarginX;
        }

        if(horizontalPolicy() == eResizeMax ||
           horizontalPolicy() == eResizeMinMax)
        {
            // set maximum width
            pMinMaxInfo->ptMaxTrackSize.x = maxWidth() + m_nSysMarginX;
        }
    }

    // check if vertical policy is set
    if(verticalPolicy() != eResizeAny)
    {
        if(verticalPolicy() == eResizeMin ||
           verticalPolicy() == eResizeMinMax)
        {
            // set minimum height
            pMinMaxInfo->ptMinTrackSize.y = minHeight() + m_nSysMarginY;
        }

        if(verticalPolicy() == eResizeMax ||
           verticalPolicy() == eResizeMinMax)
        {
            // set maximum height
            pMinMaxInfo->ptMaxTrackSize.y = maxHeight() + m_nSysMarginY;
        }
    }
}

void XWindow::_startMouseTracking()
{
    // start mouse tracking
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(TRACKMOUSEEVENT);
    tme.dwFlags = TME_LEAVE | TME_HOVER;
    tme.hwndTrack = hwnd();
    tme.dwHoverTime = HOVER_DEFAULT;

    // track mouse events
    if(!::TrackMouseEvent(&tme))
    {
        XWTRACE("XWindow: failed to start mouse tracking");
    }

    // mark flag
    m_bMouseTracking = true;
}

LRESULT XWindow::_doProcessMessageHandlers(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // reset flag
    m_bMessageHandled = false;

    // message event
    XWEvent xwEvent(hwnd, uMsg, wParam, lParam);

    // check if we have any events for this message
    std::vector<XWEventDelegate> handlers = m_xEventMap.findHandlers(xwEvent);

    // process events if any
    if(handlers.size())
    {
        // process all handlers
        LRESULT retVal = 0;
        for(unsigned int idx = 0; idx < handlers.size(); ++idx)
        {
            // call handler
            retVal = handlers.at(idx)(xwEvent);
            
            // block other handlers if message has been processed
            if(m_bMessageHandled) return retVal;
        }
    }

    // if message is not marked as processed return value will be ignored anyway
    return 0;
}

LRESULT XWindow::_windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT res = 0;

    // process known messages
    switch(uMsg)
    {

    ///// special events
        
    case WM_CREATE:
        onCreate(hwnd, (CREATESTRUCT*)lParam);
        break;

    case WM_MOVE:
        onMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        break;

    case WM_SIZE:
        // update layout items first
        _onWindowResized(LOWORD(lParam), HIWORD(lParam));

        onResize((int)wParam, LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_GETMINMAXINFO:
        // set min and max information
        _onMinMaxInfoRequested(wParam, lParam);
        break;

    case WM_DESTROY:
        // reset pointer to class instance
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);

        // reset window handle to avoid destroying this window from destructor
        m_hWnd = 0;
        
        // pass event
        onDestroy();
        break;

    case WM_ERASEBKGND:
        // ignore erase background to avoid flickering while resizing
        return 1;
        break;

    case WM_PAINT:
        onPaint();
        return 0;
        break;

    default:
        // process message handlers if any
        res = _doProcessMessageHandlers(hwnd, uMsg, wParam, lParam);
        if(isMessageProcessed()) 
        {
            // message has been processed
            return res;
        }
        // pass to derived classes
        return processMessage(hwnd, uMsg, wParam, lParam);
        break;
    }

    // pass to default function
    return ::DefWindowProcW(hwnd, uMsg, wParam, lParam);

    // mark message as processed
//    return 0;
}

/////////////////////////////////////////////////////////////////////
// window class
/////////////////////////////////////////////////////////////////////
bool XWindow::sRegisterXWindowClass()
{
    static bool IsRegistered = false;

    // check if class already registered
    if(IsRegistered) return true;

    WNDCLASSEX wc;

    // class information
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = sXWindowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = ::GetModuleHandle(NULL);
    wc.hIcon         = ::LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = 0;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = XWUI_CLASS_NAME;
    wc.hIconSm       = ::LoadIcon(NULL, IDI_APPLICATION);

    // register class
    if(::RegisterClassExW(&wc)) IsRegistered = true;

    // trace error
    if(!IsRegistered)
    {
        XWTRACE_WERR_LAST("XWindow::sRegisterXWindowClass failed to register window class");
    }

    return IsRegistered;
}

/////////////////////////////////////////////////////////////////////
// window procedure
/////////////////////////////////////////////////////////////////////
LRESULT CALLBACK XWindow::sXWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // check message
    if(uMsg == WM_NCCREATE)
    {
        // store pointer in window structure
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)(((LPCREATESTRUCT)lParam)->lpCreateParams));

        // message processed
        return TRUE;
    }

    // get window pointer
    XWindow* pWnd = (XWindow*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

    // check if we have window handle
    if(pWnd == 0)
    {
        // ignore any messages prior WM_NCCREATE
        return ::DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }

    // process message
    return pWnd->_windowProc(hwnd, uMsg, wParam, lParam);
}

// XWindow
/////////////////////////////////////////////////////////////////////


