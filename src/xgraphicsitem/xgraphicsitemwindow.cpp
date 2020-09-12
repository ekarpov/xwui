// Window to render graphic items
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "../layout/xlayoutitem.h"
#include "../layout/xlayout.h"
#include "../layout/xvboxlayout.h"
#include "../layout/xhboxlayout.h"

#include "../xwindow/xhwnd.h"
#include "../xwindow/xwindow.h"

#include "../graphics/xwgraphics.h"

#include "xgraphicsitem.h"
#include "xgraphicsitemwindow.h"

/////////////////////////////////////////////////////////////////////
// XGraphicsItemWindow - graphic item rendering

XGraphicsItemWindow::XGraphicsItemWindow(XWObject* parent) :
    XWindow(parent),
    m_pD2DResourcesCache(0),
    m_pGDIResourcesCache(0),
    m_pSharedGDIResourcesCache(0),
    m_pXGraphicsItem(0),
    m_pMouseCaptureItem(0),
    m_pContextMenuItem(0),
    m_bDirect2DPaint(false),
    m_bGDIDoubleBuffering(false),
    m_bContentScrolling(true),
    m_pRenderTarget(0)
{
    // check default painter type
    m_bDirect2DPaint = (sXWUIDefaultPainter() == XWUI_PAINTER_D2D);
}

XGraphicsItemWindow::XGraphicsItemWindow(DWORD dwStyle, XWObject* parent, HWND hWndParent, DWORD dwExStyle) :
    XWindow(parent),
    m_pD2DResourcesCache(0),
    m_pGDIResourcesCache(0),
    m_pSharedGDIResourcesCache(0),
    m_pXGraphicsItem(0),
    m_pMouseCaptureItem(0),
    m_pContextMenuItem(0),
    m_bDirect2DPaint(false),
    m_bGDIDoubleBuffering(false),
    m_bContentScrolling(true),
    m_pRenderTarget(0)
{
    // check default painter type
    m_bDirect2DPaint = (sXWUIDefaultPainter() == XWUI_PAINTER_D2D);

    // create window
    XWindow::create(dwStyle, hWndParent, dwExStyle);
}

XGraphicsItemWindow::~XGraphicsItemWindow()
{
    // close all resources if any
    _closeResources();
}

/////////////////////////////////////////////////////////////////////
// graphics
/////////////////////////////////////////////////////////////////////
void XGraphicsItemWindow::setGraphicsItem(XGraphicsItem* pXGraphicsItem)
{
    // destroy old item if set
    delete m_pXGraphicsItem;

    // set new item
    m_pXGraphicsItem = pXGraphicsItem;
    m_pXGraphicsItem->setParentObject(this);

    // init item
    _initGraphicsItem(m_pXGraphicsItem, hwnd());
}

XGraphicsItem* XGraphicsItemWindow::releaseGraphicsItem()
{
    // keep copy
    XGraphicsItem* item = m_pXGraphicsItem;

    if(m_pXGraphicsItem)
    {
        _closeGraphicsItem(m_pXGraphicsItem);
    }

    // reset reference
    m_pXGraphicsItem = 0;

    return item;
}

/////////////////////////////////////////////////////////////////////
// options
/////////////////////////////////////////////////////////////////////
void XGraphicsItemWindow::enableGDIDoubleBuffering(bool bEnable)
{
    // set flag
    m_bGDIDoubleBuffering = bEnable;

    // reset double buffering buffer if any
    if(m_pGDIResourcesCache && !m_bGDIDoubleBuffering)
    {
        m_pGDIResourcesCache->closeDoubleBufferDC();
    }
}

void XGraphicsItemWindow::forceGDIRendering(bool bForce)
{
    bool bOldDirect2DPaint = m_bDirect2DPaint;

    // update rendering method
    if(bForce)
    {
        m_bDirect2DPaint = false;

    } else
    {
        // check default painter type
        m_bDirect2DPaint = (sXWUIDefaultPainter() == XWUI_PAINTER_D2D);
    }

    // check if graphics item needs to be updated
    if(m_pXGraphicsItem && bOldDirect2DPaint != m_bDirect2DPaint)
    {
        // close item
        _closeGraphicsItem(m_pXGraphicsItem);

        // init item with new method
        _initGraphicsItem(m_pXGraphicsItem, hwnd());
    }
}

void XGraphicsItemWindow::enableContentScrolling(bool bEnable)
{
    // copy flag
    m_bContentScrolling = bEnable;
}

/////////////////////////////////////////////////////////////////////
// shared caches
/////////////////////////////////////////////////////////////////////
void XGraphicsItemWindow::setSharedGDICache(XGdiResourcesCache* pSharedGDICache)
{
    // reset previous if any
    if(m_pSharedGDIResourcesCache) 
        m_pSharedGDIResourcesCache->Release();

    // copy
    m_pSharedGDIResourcesCache = pSharedGDICache;

    // add reference
    if(m_pSharedGDIResourcesCache) 
        m_pSharedGDIResourcesCache->AddRef();

    // reset GDI cache if any
    _resetGDICache();

    // propagate to children
    _setSharedGDICacheToChildren();
}

int XGraphicsItemWindow::getSharedGDICache(XGdiResourcesCache** pSharedGDICache)
{
    // check arguments
    if(pSharedGDICache == 0) return ERROR_BAD_ARGUMENTS;

    // check if we have cache
    if(m_pSharedGDIResourcesCache == 0) 
    {
        // try to request from parent window
        _getSharedGDICacheFromParent();
    }

    // copy cache reference if any
    if(m_pSharedGDIResourcesCache) 
    {
        *pSharedGDICache = m_pSharedGDIResourcesCache;
        (*pSharedGDICache)->AddRef();

        return NOERROR;
    }

    // no cache is set
    return ERROR_NOT_FOUND;
}

/////////////////////////////////////////////////////////////////////
// state (from XWHWND)
/////////////////////////////////////////////////////////////////////
void XGraphicsItemWindow::enable(BOOL bEnable)
{
    // pass to item if set
    if(m_pXGraphicsItem)
        m_pXGraphicsItem->setEnabled(bEnable == TRUE);

    // pass to parent
    XWindow::enable(bEnable);
}

bool XGraphicsItemWindow::isEnabled()
{
    // pass to item if set
    if(m_pXGraphicsItem)
        return m_pXGraphicsItem->isEnabled();

    // pass to parent
    return XWindow::isEnabled();
}

/////////////////////////////////////////////////////////////////////
// scrolling (from XWHWND)
/////////////////////////////////////////////////////////////////////
bool XGraphicsItemWindow::canScrollContent()
{
    // check if item can scroll
    if(m_pXGraphicsItem && m_pXGraphicsItem->canScrollContent())
        return true;

    // check flag
    return m_bContentScrolling;
}

int XGraphicsItemWindow::contentWidth()
{
    // pass to item if set
    if(m_pXGraphicsItem)
        return m_pXGraphicsItem->contentWidth();

    // pass to parent
    return XWindow::contentWidth();
}

int XGraphicsItemWindow::contentHeight()
{
    // pass to item if set
    if(m_pXGraphicsItem)
        return m_pXGraphicsItem->contentHeight();

    // pass to parent
    return XWindow::contentHeight();
}

int XGraphicsItemWindow::scrollOffsetX()
{
    // check if item can scroll
    if(m_pXGraphicsItem && m_pXGraphicsItem->canScrollContent())
    {
       return m_pXGraphicsItem->scrollOffsetX();
    }

    // pass to parent
    return XWindow::scrollOffsetX();
}

int XGraphicsItemWindow::scrollOffsetY()
{
    // check if item can scroll
    if(m_pXGraphicsItem && m_pXGraphicsItem->canScrollContent())
    {
        return m_pXGraphicsItem->scrollOffsetY();
    }

    // pass to parent
    return XWindow::scrollOffsetY();
}

void XGraphicsItemWindow::setScrollOffsetX(int scrollOffsetX)
{
    // check if item is scrolling content
    if(m_pXGraphicsItem && m_pXGraphicsItem->canScrollContent())
    {
        // pass to item 
        m_pXGraphicsItem->setScrollOffsetX(scrollOffsetX);
    }
    else
    {
        XWindow::setScrollOffsetX(scrollOffsetX);

        // update item 
        _updateGraphicsItem();

        // repaint
        repaint();
    }
}

void XGraphicsItemWindow::setScrollOffsetY(int scrollOffsetY)
{
    // check if item is scrolling content
    if(m_pXGraphicsItem && m_pXGraphicsItem->canScrollContent())
    {
        // pass to item 
        m_pXGraphicsItem->setScrollOffsetY(scrollOffsetY);
    }
    else
    {
        XWindow::setScrollOffsetY(scrollOffsetY);

        // update item 
        _updateGraphicsItem();

        // repaint
        repaint();
    }
}

int XGraphicsItemWindow::scrollOffsetForWheel(int wheelDelta)
{
    // check if item can scroll
    if(m_pXGraphicsItem && m_pXGraphicsItem->canScrollContent())
        return m_pXGraphicsItem->scrollOffsetForWheel(wheelDelta);

    // pass to parent
    return XWindow::scrollOffsetForWheel(wheelDelta);
}

/////////////////////////////////////////////////////////////////////
// resize policy (from XWHWND)
/////////////////////////////////////////////////////////////////////
IXLayoutItem::TResizePolicy XGraphicsItemWindow::horizontalPolicy() const
{
    // use item policy if set or no graphics item
    if(XWHWND::horizontalPolicy() != eResizeAny || m_pXGraphicsItem == 0)
    {
        return XWHWND::horizontalPolicy();
    }

    // use graphics item
    return m_pXGraphicsItem->horizontalPolicy();
}

IXLayoutItem::TResizePolicy XGraphicsItemWindow::verticalPolicy() const
{
    // use item policy if set or no graphics item
    if(XWHWND::verticalPolicy() != eResizeAny || m_pXGraphicsItem == 0)
    {
        return XWHWND::verticalPolicy();
    }

    // use graphics item
    return m_pXGraphicsItem->verticalPolicy();
}

/////////////////////////////////////////////////////////////////////
// size constraints (from XWHWND)
/////////////////////////////////////////////////////////////////////
int XGraphicsItemWindow::minWidth()  const
{
    // use item width if set or no graphics item
    if(XWHWND::horizontalPolicy() == eResizeMin ||
       XWHWND::horizontalPolicy() == eResizeMinMax ||
       m_pXGraphicsItem == 0)
    {
        return XWHWND::minWidth();
    }

    // use graphics item 
    return m_pXGraphicsItem->minWidth();
}

int XGraphicsItemWindow::minHeight() const
{
    // use item width if set or no graphics item
    if(XWHWND::verticalPolicy() == eResizeMin ||
       XWHWND::verticalPolicy() == eResizeMinMax ||
       m_pXGraphicsItem == 0)
    {
        return XWHWND::minHeight();
    }

    // use graphics item 
    return m_pXGraphicsItem->minHeight();
}

int XGraphicsItemWindow::maxWidth()  const
{
    // use item width if set or no graphics item
    if(XWHWND::horizontalPolicy() == eResizeMax ||
       XWHWND::horizontalPolicy() == eResizeMinMax ||
       m_pXGraphicsItem == 0)
    {
        return XWHWND::maxWidth();
    }

    // use graphics item 
    return m_pXGraphicsItem->maxWidth();
}

int XGraphicsItemWindow::maxHeight() const
{
    // use item width if set or no graphics item
    if(XWHWND::verticalPolicy() == eResizeMax ||
       XWHWND::verticalPolicy() == eResizeMinMax ||
       m_pXGraphicsItem == 0)
    {
        return XWHWND::maxHeight();
    }

    // use graphics item 
    return m_pXGraphicsItem->maxHeight();
}

/////////////////////////////////////////////////////////////////////
// events
/////////////////////////////////////////////////////////////////////
void XGraphicsItemWindow::onCreate(HWND hwnd, const CREATESTRUCT* pCreateStruct)
{
    // init graphics item if set
    _initGraphicsItem(m_pXGraphicsItem, hwnd);
}

void XGraphicsItemWindow::onResize(int type, int width, int height)
{
    // update item
    _updateGraphicsItem(width, height);

    // update D2D render target
    if(m_bDirect2DPaint && m_pRenderTarget != 0)
    {
        // try to resize
        if (FAILED(m_pRenderTarget->Resize(D2D1::SizeU(width, height))))
        {
            _resetD2DTarget();
        }
    }
}

void XGraphicsItemWindow::onDestroy()
{
    // close all resources if any
    _closeResources();

    // pass to parent
    XWindow::onDestroy();
}

bool XGraphicsItemWindow::onTimer(WPARAM uIDEvent)
{
    // find item by id
    XGraphicsItem* timerItem = _findItemId(uIDEvent);
    if(timerItem)
    {
        // pass to item
        if(!timerItem->onTimerEvent())
        {
            // item ignored event, stop timer
            XWTRACE("XGraphicsItemWindow: item ignored timer event, timer will be stopped");
            stopTimer(uIDEvent);
        }

    } else
    {
        // stop unknown timer
        XWTRACE("XGraphicsItemWindow: received uknown timer event, timer will be stopped");
        stopTimer(uIDEvent);
    }

    // always consume event
    return true;
}

/////////////////////////////////////////////////////////////////////
// mouse events
/////////////////////////////////////////////////////////////////////
void XGraphicsItemWindow::onMouseEnter(int posX, int posY)
{
    // ignore if not enabled
    if(!isEnabled()) return;

    // process mouse enter
    if(m_pMouseCaptureItem)
        m_pMouseCaptureItem->onMouseEnter(posX, posY);
    else if(m_pXGraphicsItem)
        m_pXGraphicsItem->onMouseEnter(posX, posY);
}

void XGraphicsItemWindow::onMouseMove(int posX, int posY, WPARAM flags)
{
    // ignore if not enabled
    if(!isEnabled()) return;

    // process mouse move
    if(m_pMouseCaptureItem)
        m_pMouseCaptureItem->onMouseMove(posX, posY, flags);
    else if(m_pXGraphicsItem) 
        m_pXGraphicsItem->onMouseMove(posX, posY, flags);
}

void XGraphicsItemWindow::onMouseHover(int posX, int posY)
{
    // ignore if not enabled
    if(!isEnabled()) return;

    // handle mouse hover
    if(m_pMouseCaptureItem)
        m_pMouseCaptureItem->onMouseHover(posX, posY);
    else if(m_pXGraphicsItem) 
        m_pXGraphicsItem->onMouseHover(posX, posY);
}

bool XGraphicsItemWindow::onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags)
{
    // ignore if not enabled
    if(!isEnabled()) return false;

    // handle mouse click
    if(m_pMouseCaptureItem)
    {
        return m_pMouseCaptureItem->onMouseClick(uButtonMsg, posX, posY, flags);
    }
    else if(m_pXGraphicsItem) 
    {
        return m_pXGraphicsItem->onMouseClick(uButtonMsg, posX, posY, flags);
    }

    // ignore by default
    return false;
}

void XGraphicsItemWindow::onMouseLeave()
{
    // ignore if not enabled
    if(!isEnabled()) return;

    // process mouse leave
    if(m_pMouseCaptureItem)
        m_pMouseCaptureItem->onMouseLeave();
    else if(m_pXGraphicsItem)
        m_pXGraphicsItem->onMouseLeave();
}

bool XGraphicsItemWindow::onMouseWheel(int wheelDelta, int posX, int posY, WPARAM flags)
{
    // ignore if not enabled
    if(!isEnabled()) return false;

    // handle mouse wheel event
    if(m_pMouseCaptureItem)
    {
        return m_pMouseCaptureItem->onMouseWheel(wheelDelta, posX, posY, flags);
    }
    else if(m_pXGraphicsItem) 
    {
        return m_pXGraphicsItem->onMouseWheel(wheelDelta, posX, posY, flags);
    }

    // ignore by default
    return false;
}

void XGraphicsItemWindow::onMouseCaptureReset()
{
    // process mouse capture reset
    if(m_pMouseCaptureItem)
        m_pMouseCaptureItem->onMouseCaptureReset();
    else if(m_pXGraphicsItem)
        m_pXGraphicsItem->onMouseCaptureReset();

    // reset capture if any
    _resetMouseCapture();
}

bool XGraphicsItemWindow::onSetCursor()
{
    // pass to item
    if(m_pXGraphicsItem)
    {
        return m_pXGraphicsItem->onSetCursor();
    }

    // ignore by default
    return false;
}

/////////////////////////////////////////////////////////////////////
// keyboard events (sent only to focused item)
/////////////////////////////////////////////////////////////////////
bool XGraphicsItemWindow::onCharEvent(WPARAM charCode, LPARAM flags)
{
    // ignore if not enabled
    if(!isEnabled()) return false;

    // pass to item
    if(m_pXGraphicsItem)
    {
        return m_pXGraphicsItem->onCharEvent(charCode, flags);
    }

    // ignore by default
    return false;
}

/////////////////////////////////////////////////////////////////////
// context menu
/////////////////////////////////////////////////////////////////////
void XGraphicsItemWindow::onContextMenuItem(UINT itemId)
{
    // pass to context menu item
    if(m_pContextMenuItem)
        m_pContextMenuItem->onContextMenuItem(itemId);
    else
        XWindow::onContextMenuItem(itemId);
}

bool XGraphicsItemWindow::onContextMenu(HWND hwndContext, int posX, int posY)
{
    // ignore if not enabled
    if(!isEnabled()) return false;

    // pass to item if set
    if(m_pXGraphicsItem)
        if(m_pXGraphicsItem->onContextMenu(posX, posY)) return true;

    // pass to parent
    return XWindow::onContextMenu(hwndContext, posX, posY);
}

/////////////////////////////////////////////////////////////////////
// message processing
/////////////////////////////////////////////////////////////////////
LRESULT XGraphicsItemWindow::processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // check for common events and pass them to graphics item
    switch(uMsg)
    {
    case WM_DISPLAYCHANGE:
        // render D2D
        if(m_bDirect2DPaint && m_pXGraphicsItem) 
        {
            _onPaintWindowD2D(m_pXGraphicsItem->rect());
        }
        // reset GDI cache if any
        _resetGDICache();
        break;

    case WM_PALETTECHANGED:
        // reset GDI cache if any
        _resetGDICache();
        break;

    case WM_SETFOCUS:
        if(m_pXGraphicsItem && m_pXGraphicsItem->isFocusable()) 
        {
            m_pXGraphicsItem->setFocus(true);
            return 0;
        }
        break;

    case WM_KILLFOCUS:
        if(m_pXGraphicsItem && m_pXGraphicsItem->isFocusable()) 
        {
            m_pXGraphicsItem->setFocus(false);
            return 0;
        }
        break;

    case WM_ENABLE:

        // enable item
        if(m_pXGraphicsItem) m_pXGraphicsItem->setEnabled(wParam == TRUE);

        return 0;
        break;

    case WM_MOUSEACTIVATE:
        // set focus to itself if possible
        if(m_pXGraphicsItem && m_pXGraphicsItem->isFocusable())
        {
            // set focus to hanle key events
            ::SetFocus(hwnd);
        }
        break;

    case WM_CAPTURECHANGED:
        // handle mouse capture reset
        onMouseCaptureReset();
        return 0;
        break;

    ///// XWUI messages
    case WM_XWUI_ANIMATION_TIMER_EVENT:
        // process event
        _onAnimationTimerEvent((DWORD)wParam);
        break;

    case WM_XWUI_ANIMATION_VALUE_EVENT:
        // process event
        _onAnimationValueEvent((DWORD)wParam);
        break;

    case WM_XWUI_ANIMATION_COMPLETED:
        // process event
        _onContentLoaded((DWORD)wParam, (const WCHAR*)lParam);
        break;

    case WM_XWUI_URL_CONTENT_LOADED:
        // process event
        _onContentLoadFailed((DWORD)wParam, (DWORD)lParam);
        break;

    case WM_XWUI_URL_CONTENT_LOAD_FAILED:
        // process event
        _onAnimationCompleted((DWORD)wParam);
        break;

    ///// special graphics item messages
    
    case WM_XWUI_GITEM_EVENT:
        // set capture
        if(_onItemEvent((DWORD)wParam, lParam)) 
            return TRUE;
        else
            return FALSE;
        break;

    case WM_XWUI_GITEM_SET_MOUSE_CAPTURE:
        // set capture
        _setMouseCapture(hwnd, _findItemId(wParam));
        break;

    case WM_XWUI_GITEM_RESET_MOUSE_CAPTURE:
        // reset mouse capture
        _resetMouseCapture();
        break;

    case WM_XWUI_GITEM_CONTENT_CHANGED:
        // update item layout
        _updateGraphicsItem();

        // request repaint
        repaint();
        break;

    case WM_XWUI_GITEM_SET_SHARED_GDI_CACHE:
        // set shared cache
        setSharedGDICache((XGdiResourcesCache*)lParam);

        return 0;
        break;

    case WM_XWUI_GITEM_GET_SHARED_GDI_CACHE:
        return getSharedGDICache((XGdiResourcesCache**)lParam);
        break;

    case WM_XWUI_GITEM_SHOW_TOOLTIP:
        // show tooltip
        showToolTip((const wchar_t*)lParam);

        return 0;
        break;

    case WM_XWUI_GITEM_SHOW_CONTEXT_MENU:
        // show menu
        _showContextMenu(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

        return 0;
        break;
    }

    // check if graphics item needs raw message processing
    if(m_pXGraphicsItem)
    {
        // pass to item
        bool messageProcessed = false;
        LRESULT msgResult = m_pXGraphicsItem->processWindowMessage(hwnd, uMsg, wParam, lParam, messageProcessed);

        // stop processing if message has been consumed
        if(messageProcessed) return msgResult;
    }

    // pass to parent
    return XWindow::processMessage(hwnd, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////
// paint events (from XWindow)
/////////////////////////////////////////////////////////////////////
void XGraphicsItemWindow::onPaint(HDC hdc, PAINTSTRUCT& ps)
{
    // ignore if item not set or not visible
    if(m_pXGraphicsItem == 0 || !m_pXGraphicsItem->isVisible()) return;

    // check what render method is in use
    if(m_bDirect2DPaint)
    {
        // Direct2D rendering
        _onPaintWindowD2D(ps.rcPaint);

    } else
    {
        // GDI rendering
        _onPaintWindowGDI(hdc, ps);
    }
}

/////////////////////////////////////////////////////////////////////
// hide background settings (from XWindow)
/////////////////////////////////////////////////////////////////////
void XGraphicsItemWindow::enableEraseBackground(bool /*bEnable*/)
{
    // should not be used in XGraphicsItemWindow
    XWASSERT(false);
}

void XGraphicsItemWindow::setBackgroundColor(COLORREF /*color*/)
{
    // should not be used in XGraphicsItemWindow
    XWASSERT(false);
}

/////////////////////////////////////////////////////////////////////
// hide layout setting (from XWindow)
/////////////////////////////////////////////////////////////////////
void XGraphicsItemWindow::setLayout(IXLayout* /*pLayout*/)
{
    // should not be used in XGraphicsItemWindow
    XWASSERT(false);
}

/////////////////////////////////////////////////////////////////////
// events
/////////////////////////////////////////////////////////////////////
void XGraphicsItemWindow::onChildObjectRemoved(XWObject* child)
{
    // check if graphics item has been removed
    if(m_pXGraphicsItem && child && m_pXGraphicsItem->xwoid() == child->xwoid())
    {
        // reset reference
        m_pXGraphicsItem = 0;
    }

    // pass to parent
    XWindow::onChildObjectRemoved(child);
}

/////////////////////////////////////////////////////////////////////
// paint methods
/////////////////////////////////////////////////////////////////////
void XGraphicsItemWindow::_onPaintWindowGDI(HDC hdc, PAINTSTRUCT& ps)
{
    HDC hPaintDC = hdc;
    HDC hDoubleBufferDC = 0;

    // NOTE: we assume that HDC we get is compatible unless there were WM_DISPLAYCHANGE or
    //       WM_PALETTECHANGED. 
    // http://stackoverflow.com/questions/2074294/win32-does-a-window-have-the-same-hdc-for-its-entire-lifetime

    // init cache (if not there already)
    _initGDICache(hdc);

    // client area size
    RECT rcClient;
    ::GetClientRect(hwnd(), &rcClient);

    // width and height
    int width = rcClient.right - rcClient.left;
    int height = rcClient.bottom - rcClient.top;

    // create double buffering DC if needed
    if(m_bGDIDoubleBuffering && m_pGDIResourcesCache)
    {
        // get double buffering DC from cache (or create new)
        hDoubleBufferDC = m_pGDIResourcesCache->getDoubleBufferDC(hdc, width, height);
    }

    // check what DC to use
    hPaintDC = (hDoubleBufferDC != 0) ? hDoubleBufferDC: hdc;

    // paint graphics
    if(m_pXGraphicsItem)
        m_pXGraphicsItem->onPaintGDI(hPaintDC, ps.rcPaint);

    // finish double buffering and release resources
    if(hDoubleBufferDC)
    {
        //// copy double buffer to screen
        ::BitBlt(hdc, 0, 0, width, height, hDoubleBufferDC, 0, 0, SRCCOPY);
    }
}

void XGraphicsItemWindow::_initGDICache(HDC hdc)
{
    // ignore if there is cache already
    if(m_pGDIResourcesCache != 0) return;

    // try to read shared cache from parent if not set
    if(m_pSharedGDIResourcesCache == 0)
    {
        // try to request from parent
        _getSharedGDICacheFromParent();
    }

    // check if we have shared cache set
    if(m_pSharedGDIResourcesCache)
    {
        // copy reference
        m_pGDIResourcesCache = m_pSharedGDIResourcesCache;
        m_pGDIResourcesCache->AddRef();

    } else
    {
        // create resource cache 
        m_pGDIResourcesCache = new XGdiResourcesCache;
        m_pGDIResourcesCache->AddRef();

        // init cache
        m_pGDIResourcesCache->init(hdc);
    }

    // set cache reference to item
    if(m_pXGraphicsItem)
    {
        m_pXGraphicsItem->setGDIResourcesCache(m_pGDIResourcesCache);
        m_pXGraphicsItem->onInitGDIResources(hdc);
    }
}

void XGraphicsItemWindow::_resetGDICache()
{
    // reset item
    if(m_pXGraphicsItem) 
    {
        m_pXGraphicsItem->setGDIResourcesCache(0);
    }

    // release resource cache
    if(m_pGDIResourcesCache) 
    {
        m_pGDIResourcesCache->Release();
        m_pGDIResourcesCache = 0;
    }
}

void XGraphicsItemWindow::_onPaintWindowD2D(const RECT& rcPaint)
{
    // init target
    if(_initD2DTarget())
    {
        if(0 == (D2D1_WINDOW_STATE_OCCLUDED & m_pRenderTarget->CheckWindowState()))
        {
            // start rendering
            m_pRenderTarget->BeginDraw();
//            m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
//            m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Red));
 
            // paint graphics
            if(m_pXGraphicsItem)
                m_pXGraphicsItem->onPaintD2D(m_pRenderTarget, rcPaint);

            // check if target has to be reset
            if(D2DERR_RECREATE_TARGET == m_pRenderTarget->EndDraw())
            {
                _resetD2DTarget();
            }
        }
    }
}

bool XGraphicsItemWindow::_initD2DTarget()
{
    // ignore if there is target already
    if(m_pRenderTarget != 0) return true;

    // client area size
    RECT rcClient;
    ::GetClientRect(hwnd(), &rcClient);

    // D2D size
    D2D1_SIZE_U size = D2D1::SizeU(rcClient.right - rcClient.left, 
        rcClient.bottom - rcClient.top);

    // get static factory
    ID2D1Factory* pID2D1Factory = XD2DHelpers::getDirect2DFactory();
    XWASSERT(pID2D1Factory);
    if(pID2D1Factory == 0) return false;
    
    // NOTE: Using D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE also allows sharing render target bitmaps
    //       More details: http://msdn.microsoft.com/en-us/library/dd756757(v=vs.85).aspx#sharing_render_target_resources

    // NOTE: use GDI compatible targets to allow rendering GDI items
    //       using Direct2D methods
    D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
    rtProps.usage =  D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

    // TODO: if window transparency needed pixel format has to be changed => add to class interface ?
    //rtProps.pixelFormat = D2D1::PixelFormat( DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED );
    
    // create render target
    HRESULT hr = pID2D1Factory->CreateHwndRenderTarget(rtProps,
        D2D1::HwndRenderTargetProperties(hwnd(), size), &m_pRenderTarget);

    if(SUCCEEDED(hr))
    {
        // create resource cache 
        if(m_pD2DResourcesCache) m_pD2DResourcesCache->Release();
        m_pD2DResourcesCache = new XD2DResourcesCache;
        m_pD2DResourcesCache->AddRef();

        // init render target for cache
        m_pD2DResourcesCache->init(m_pRenderTarget);

        // init item
        if(m_pXGraphicsItem) 
        {
            // set cache reference to item
            m_pXGraphicsItem->setD2DResourcesCache(m_pD2DResourcesCache);
            m_pXGraphicsItem->onInitD2DTarget(m_pRenderTarget);
        }
        return true;
    }

    // failed to init traget
    XWASSERT(false);
    return false;
}

void XGraphicsItemWindow::_resetD2DTarget()
{
    // reset item
    if(m_pXGraphicsItem) 
    {
        m_pXGraphicsItem->setD2DResourcesCache(0);
    }

    // release resource cache
    if(m_pD2DResourcesCache) 
    {
        m_pD2DResourcesCache->Release();
        m_pD2DResourcesCache = 0;
    }

    // release target
    if(m_pRenderTarget)
    {
        m_pRenderTarget->Release();
        m_pRenderTarget = 0;
    }
}

// helper callback
static BOOL CALLBACK _setSharedGDICacheCallbackProc(HWND hwnd, LPARAM lParam)
{
    // set to child window
    ::SendMessageW(hwnd, WM_XWUI_GITEM_SET_SHARED_GDI_CACHE, 0, lParam);

    return TRUE;
}

void XGraphicsItemWindow::_setSharedGDICacheToChildren()
{
    // ignore if window not ready
    if(hwnd() == 0) return;

    // set current cache to all children
    ::EnumChildWindows(m_hWnd, _setSharedGDICacheCallbackProc, (LPARAM)m_pSharedGDIResourcesCache);
}

void XGraphicsItemWindow::_getSharedGDICacheFromParent()
{
    // ignore if window not ready
    if(hwnd() == 0) return;

    // ignore if no parent
    HWND hParent = ::GetParent(hwnd());
    if(hParent == 0) return;

    // request GDI cache from parent window
    XGdiResourcesCache* pGdiResourcesCache = 0;
    if(::SendMessageW(hParent, WM_XWUI_GITEM_GET_SHARED_GDI_CACHE, 0, (LPARAM)&pGdiResourcesCache) == NOERROR)
    {
        // check if set
        if(pGdiResourcesCache)
        {
            // set new cache to itself
            setSharedGDICache(pGdiResourcesCache);

            // release this reference
            pGdiResourcesCache->Release();
        }
    }
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XGraphicsItemWindow::_closeResources()
{
    // close graphics resources
    _closeGraphicsItem(m_pXGraphicsItem);
    
    // delete graphic item
    delete m_pXGraphicsItem;
    m_pXGraphicsItem = 0;

    // reset caches
    _resetGDICache();
    _resetD2DTarget();

    // reset shared caches if any
    if(m_pSharedGDIResourcesCache) m_pSharedGDIResourcesCache->Release();
    m_pSharedGDIResourcesCache = 0;
}

void XGraphicsItemWindow::_initGraphicsItem(XGraphicsItem* pItem, HWND hwnd)
{
    // ignore if no item set or window is not created yet
    if(pItem == 0 || hwnd == 0) return;

    // set parent window
    pItem->setParentWindow(hwnd);

    // init painter type
    if(m_bDirect2DPaint)
    {
        // reset GDI cache if any
        _resetGDICache();

        // set painter type
        pItem->setPainterType(XWUI_PAINTER_D2D);

        // set cache reference to item
        if(m_pRenderTarget && m_pD2DResourcesCache)
        {        
            m_pXGraphicsItem->setD2DResourcesCache(m_pD2DResourcesCache);
            m_pXGraphicsItem->onInitD2DTarget(m_pRenderTarget);

        } else
        {
            // init D2D resources 
            _initD2DTarget();
        }
    }
    else
    {
        // reset D2D if any
        _resetD2DTarget();

        // init painter type
        pItem->setPainterType(XWUI_PAINTER_GDI);

        // window DC
        HDC hdc = ::GetDC(hwnd);

        // set cache reference to item
        if(m_pGDIResourcesCache)
        {        
            pItem->setGDIResourcesCache(m_pGDIResourcesCache);
            pItem->onInitGDIResources(hdc);

        } else
        {
            // init cache 
            _initGDICache(hdc);
        }

        // release DC
        ::ReleaseDC(hwnd, hdc);
    }

    // update item size
    _updateGraphicsItem();
}

void XGraphicsItemWindow::_updateGraphicsItem()
{
    int width, height;

    // read client area size
    getClientSize(width, height);

    // update
    _updateGraphicsItem(width, height);
}

void XGraphicsItemWindow::_updateGraphicsItem(int width, int height)
{
    // ignore zero size
    if(width == 0 || height == 0) return;

    if(m_pXGraphicsItem)
    {
        // check if window is scrolling content
        if(m_bContentScrolling)
            m_pXGraphicsItem->update(-1 * XWindow::scrollOffsetX(), -1 * XWindow::scrollOffsetY(), width, height);
        else
            m_pXGraphicsItem->update(0, 0, width, height);
    }
}

void XGraphicsItemWindow::_closeGraphicsItem(XGraphicsItem* pItem)
{
    // ignore if no item set or window is not created yet
    if(pItem == 0 || hwnd() == 0) return;

    if(m_bDirect2DPaint)
    {
        // reset Direct2D cache (will reset resources as well)
        pItem->setD2DResourcesCache(0);

    } else
    {
        // reset GDI cache (will reset resources as well)
        pItem->setGDIResourcesCache(0);
    }

    // reset item window
    pItem->setParentWindow(0);

    // reset state
    m_pMouseCaptureItem = 0;
}

bool XGraphicsItemWindow::_onItemEvent(DWORD itemId, LPARAM param)
{
    // ignore if item not set
    if(m_pXGraphicsItem == 0) return false;

    // check if event is broadcast
    if(itemId)
    {
        // find item
        XGraphicsItem* eventItem = _findItemId(itemId);

        // check if found
        if(eventItem == 0) return false;

        // pass to item
        eventItem->onItemEvent(param);

    } else
    {
        // deliver to all items
        m_pXGraphicsItem->onBroadcastEvent(param);
    }

    // delivered
    return true;
}

void XGraphicsItemWindow::_onAnimationTimerEvent(DWORD id)
{
    XGraphicsItem* animationItem = 0;

    // find animation item
    if(m_pXGraphicsItem)
        animationItem = m_pXGraphicsItem->findAnimationItem(id);

    // report event if found
    if(animationItem)
        animationItem->onAnimationTimer(id);

    // stop if animation item not found
    if(animationItem == 0)
    {
        XWTRACE("XGraphicsItemWindow: unknown timer animation stopped");

        // stop unknown animation
        XWAnimationTimer::instance()->stopAnimation(id);
    }
}

void XGraphicsItemWindow::_onAnimationValueEvent(DWORD id)
{
    XGraphicsItem* animationItem = 0;

    // find animation item
    if(m_pXGraphicsItem)
        animationItem = m_pXGraphicsItem->findAnimationItem(id);

    // report event if found
    if(animationItem)
    {
        float value;

        // report value
        if(XWAnimationTimer::instance()->getAnimationValue(id, value))
            animationItem->onAnimationValue(id, value);
    }

    // stop if animation item not found
    if(animationItem == 0)
    {
        XWTRACE("XGraphicsItemWindow: unknown value animation stopped");

        // stop unknown animation
        XWAnimationTimer::instance()->stopAnimation(id);
    }
}

void XGraphicsItemWindow::_onAnimationCompleted(DWORD id)
{
    XGraphicsItem* animationItem = 0;

    // find animation item
    if(m_pXGraphicsItem)
        animationItem = m_pXGraphicsItem->findAnimationItem(id);

    // report
    if(animationItem)
        animationItem->onAnimationCompleted(id);
}

void XGraphicsItemWindow::_onContentLoaded(DWORD id, const WCHAR* path)
{
    XGraphicsItem* contentItem = 0;

    // find conten item
    if(m_pXGraphicsItem)
        contentItem = m_pXGraphicsItem->findContentItem(id);

    if(contentItem)
    {
        // report event
        contentItem->onUrlContentLoaded(id, path);

        // mark as completed
        contentItem->onUrlContentCompleted(id);

    } else
    {
        XWTRACE("XGraphicsItemWindow: unknown content loaded event ignored");
    }
}

void XGraphicsItemWindow::_onContentLoadFailed(DWORD id, DWORD reason)
{
    XGraphicsItem* contentItem = 0;

    // find conten item
    if(m_pXGraphicsItem)
        contentItem = m_pXGraphicsItem->findContentItem(id);

    if(contentItem)
    {
        // report event
        contentItem->onUrlContentLoadFailed(id, reason);

        // mark as completed
        contentItem->onUrlContentCompleted(id);

    } else
    {
        XWTRACE("XGraphicsItemWindow: unknown content load failed event ignored");
    }
}

void XGraphicsItemWindow::_setMouseCapture(HWND hwnd, XGraphicsItem* pXGraphicsItem)
{
    // ignore if no item set
    if(pXGraphicsItem == 0)
    {
        XWTRACE("XGraphicsItemWindow: failed to set mouse capture, unknown item");
        return;
    }

    // set mouse capture item
    m_pMouseCaptureItem = pXGraphicsItem;

    // set capture
    ::SetCapture(hwnd);
}

void XGraphicsItemWindow::_resetMouseCapture()
{
    // stop mouse capture
    if(!::ReleaseCapture())
    {
        XWTRACE_WERR_LAST("XGraphicsItemWindow: failed to release mouse capture");
    }

    // reset mouse capture item
    m_pMouseCaptureItem = 0;
}

void XGraphicsItemWindow::_showContextMenu(WPARAM wItemId, int posX, int posY)
{
    // find context menu item
    m_pContextMenuItem = _findItemId(wItemId);
    if(m_pContextMenuItem == 0)
    {
        XWTRACE("XGraphicsItemWindow: context menu event ignored, uknown item id");
        return;
    }

    // check if context item has menu
    if(m_pContextMenuItem->contextMenu() == 0)
    {
        XWTRACE("XGraphicsItemWindow: context menu event ignored, item doesn't have context menu");
        m_pContextMenuItem = 0;
        return;
    }

    // show menu
    doShowContextMenu(m_pContextMenuItem->contextMenu(), posX, posY);

    // reset context menu item
    m_pContextMenuItem = 0;
}

XGraphicsItem* XGraphicsItemWindow::_findItemId(WPARAM wItemId)
{
    if(m_pXGraphicsItem)
    {
        // check if item itself has this id
        if(m_pXGraphicsItem->xwoid() == wItemId) return m_pXGraphicsItem;

        // try to find from child items
        return m_pXGraphicsItem->findChildItem((unsigned long)wItemId);
    }

    // not found
    return 0;
}

// XGraphicsItemWindow
/////////////////////////////////////////////////////////////////////
