// ScrollView window
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

// dependencies
#include "../graphics/xwgraphics.h"
#include "../layout/xwlayouts.h"
#include "../xwindow/xwindow.h"
#include "../xgraphicsitem/xwgraphicsitems.h"

#include "xwscrollbarwindow.h"
#include "xwscrollviewwindow.h"

/////////////////////////////////////////////////////////////////////
// XWScrollViewWindow - scroll view

XWScrollViewWindow::XWScrollViewWindow(DWORD dwStyle, XWObject* parent, HWND hWndParent, DWORD dwExStyle) :
    XWindow(dwStyle, parent, hWndParent, dwExStyle),
    m_scrollWindow(0),
    m_verticalScrollBar(0),
    m_horizontalScrollBar(0)
{
    // create scrollbars
    m_verticalScrollBar = new XWScrollBarWindow(eXScrollBarVertical, hwnd(), this);
    m_horizontalScrollBar = new XWScrollBarWindow(eXScrollBarHorizontal, hwnd(), this);

    // init scrollbars with view itself
    m_verticalScrollBar->setScrollItem(this);
    m_horizontalScrollBar->setScrollItem(this);

    // hide by default
    m_verticalScrollBar->hide();
    m_horizontalScrollBar->hide();
}

XWScrollViewWindow::~XWScrollViewWindow()
{
}

/////////////////////////////////////////////////////////////////////
// scrollable window (view doesn't take ownership)
/////////////////////////////////////////////////////////////////////
void XWScrollViewWindow::setScrollWindow(XWHWND* scrollWindow)
{
    // release previous if any
    releaseScrollWindow();

    // copy reference
    m_scrollWindow = scrollWindow;

    // set properties
    if(m_scrollWindow)
    {
        // parent window
        m_scrollWindow->setParent(hwnd());

        // pass to logic
        XWScrollViewLogic::setScrollableItem(m_scrollWindow);

        // listen for window deletion
        addConnectedObject(m_scrollWindow);

        // show
        m_scrollWindow->show();
    }

    // reset visibility
    m_verticalScrollBar->show((m_showVerticalScrollBar == eScrollBarShowAlways) ? SW_SHOW: SW_HIDE);
    m_horizontalScrollBar->show((m_showHorizontalScrollBar == eScrollBarShowAlways) ? SW_SHOW: SW_HIDE);

    // layout
    XWScrollViewLogic::updateScrollView(0, 0, width(), height());

    // update scroll bars 
    m_verticalScrollBar->onScrollContentChanged();
    m_horizontalScrollBar->onScrollContentChanged();

    // repaint 
    repaint();
}

void XWScrollViewWindow::scrollContent(int scrollX, int scrollY)
{
    // pass to scrollbars
    if(m_showVerticalScrollBar != eScrollBarShowNever)
        m_verticalScrollBar->scrollContent(scrollY);
    if(m_showHorizontalScrollBar != eScrollBarShowNever)
        m_horizontalScrollBar->scrollContent(scrollX);
}

/////////////////////////////////////////////////////////////////////
// release ownership from view
/////////////////////////////////////////////////////////////////////
void XWScrollViewWindow::releaseScrollWindow()
{
    // release window if set
    if(m_scrollWindow)
    {
        // remove all connections
        removeConnectedObject(m_scrollWindow);
    }

    // reset reference
    m_scrollWindow = 0;

    // reset ScrollOffsets if any
    XWindow::setScrollOffsetX(0);
    XWindow::setScrollOffsetY(0);
}

/////////////////////////////////////////////////////////////////////
// manipulations (from IXLayoutItem)
/////////////////////////////////////////////////////////////////////
void XWScrollViewWindow::update(int posX, int posY, int width, int height)
{
    // pass to parent first
    XWindow::update(posX, posY, width, height);

    // pass to scroll logic
    XWScrollViewLogic::updateScrollView(0, 0, width, height);

    // NOTE: sometimes after resizing scrollbars do not get painted, do it
    //       once again here
    if(m_verticalScrollBar->isVisible())
        m_verticalScrollBar->repaint();
    if(m_horizontalScrollBar->isVisible())
        m_horizontalScrollBar->repaint();
}

/////////////////////////////////////////////////////////////////////
// scrolling (from XWHWND)
/////////////////////////////////////////////////////////////////////
bool XWScrollViewWindow::canScrollContent()
{
    // can scroll
    return true;
}

int XWScrollViewWindow::contentWidth()
{
    // pass to item if set
    if(m_scrollWindow)
        return m_scrollWindow->contentWidth();

    // pass to parent
    return XWindow::contentWidth();
}

int XWScrollViewWindow::contentHeight()
{
    // pass to item if set
    if(m_scrollWindow)
        return m_scrollWindow->contentHeight();

    // pass to parent
    return XWindow::contentWidth();
}

int XWScrollViewWindow::scrollOffsetX()
{
    // pass to item if set and scrollable
    if(m_scrollWindow && m_scrollWindow->canScrollContent())
        return m_scrollWindow->scrollOffsetX();

    // pass to parent
    return XWindow::scrollOffsetX();
}

int XWScrollViewWindow::scrollOffsetY()
{
    // pass to item if set and scrollable
    if(m_scrollWindow && m_scrollWindow->canScrollContent())
        return m_scrollWindow->scrollOffsetY();

    // pass to parent
    return XWindow::scrollOffsetY();
}

void XWScrollViewWindow::setScrollOffsetX(int scrollOffsetX)
{
    // pass to item if set and scrollable
    if(m_scrollWindow && m_scrollWindow->canScrollContent())
    {
        m_scrollWindow->setScrollOffsetX(scrollOffsetX);
    }
    else
    {
        // scroll item
        _doScrollItem(scrollOffsetX, 0);
    }
}

void XWScrollViewWindow::setScrollOffsetY(int scrollOffsetY)
{
    // pass to item if set and scrollable
    if(m_scrollWindow && m_scrollWindow->canScrollContent())
    {
        m_scrollWindow->setScrollOffsetY(scrollOffsetY);
    }
    else
    {
        // scroll item
        _doScrollItem(0, scrollOffsetY);
    }
}

int XWScrollViewWindow::scrollOffsetForWheel(int wheelDelta)
{
    // pass to item if set and scrollable
    if(m_scrollWindow && m_scrollWindow->canScrollContent())
        return m_scrollWindow->scrollOffsetForWheel(wheelDelta);

    // pass to parent
    return XWindow::scrollOffsetForWheel(wheelDelta);
}

/////////////////////////////////////////////////////////////////////
// common events (from XWindow)
/////////////////////////////////////////////////////////////////////
void XWScrollViewWindow::onContentChanged()
{
    // update layout
    XWScrollViewLogic::updateScrollView(0, 0, width(), height());

    // pass to parent
    XWindow::onContentChanged();
}

/////////////////////////////////////////////////////////////////////
// mouse events (from XWindow)
/////////////////////////////////////////////////////////////////////
void XWScrollViewWindow::onMouseEnter(int posX, int posY)
{
    // check if mouse drag is enabled
    if(m_bMouseDragScrollEnabled)
    {
        // TODO: 
    }
}

void XWScrollViewWindow::onMouseMove(int posX, int posY, WPARAM flags)
{
    // check if mouse drag is enabled
    if(m_bMouseDragScrollEnabled)
    {
        // TODO: 
    }
}

bool XWScrollViewWindow::onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags)
{
    // check if mouse drag is enabled
    if(m_bMouseDragScrollEnabled)
    {
        // TODO: 
    }

    return false;
}

void XWScrollViewWindow::onMouseLeave()
{
    // check if mouse drag is enabled
    if(m_bMouseDragScrollEnabled)
    {
        // TODO: 
    }
}

bool XWScrollViewWindow::onMouseWheel(int wheelDelta, int posX, int posY, WPARAM flags)
{
    // check if mouse wheel scroll is enabled
    if(m_bMouseWheelScrollEnabled)
    {
        if(m_scrollItem && m_verticalScrollBar->isVisible())
        {
            //XWTRACE1("%d", wheelDelta);

            // get scroll offset for delta
            int scrollOffsetY = m_scrollItem->scrollOffsetForWheel(wheelDelta);

            // scroll content (scrollbar will handle possible overflows)
            m_verticalScrollBar->scrollContent(-scrollOffsetY);

            // mark as consumed
            return true;
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////
// keyboard events (from XWindow)
/////////////////////////////////////////////////////////////////////
bool XWScrollViewWindow::onCharEvent(WPARAM charCode, LPARAM flags)
{
    // check if keyboard scroll is enabled
    if(m_bKeyScrollEnabled)
    {
        // TODO:
    }

    return false;
}

/////////////////////////////////////////////////////////////////////
// process messages
/////////////////////////////////////////////////////////////////////
LRESULT XWScrollViewWindow::processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // relay WM_COMMAND notifications to parent
    if(uMsg == WM_COMMAND)
    {
        HWND hwndParent = ::GetParent(XWindow::hwnd());
        if(hwndParent)
            return ::SendMessageW(hwndParent, uMsg, wParam, lParam);
    }

    // pass to parent
    return XWindow::processMessage(hwnd, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////
// scroll logic methods (from XWScrollViewLogic)
/////////////////////////////////////////////////////////////////////
void XWScrollViewWindow::updateScrollItem(int posX, int posY, int width, int height)
{
    if(m_scrollWindow == 0) return;

    if(m_scrollWindow->canScrollContent())
    {
        // item is scrolling
        m_scrollWindow->update(posX, posY, width, height);

    } else
    {
        // view is scrolling
        int scrollWindowWidth = width;
        int scrollWindowHeight = height;

        // update width
        if(isScrollBarVisible(eHorizontalScrollBar))
            scrollWindowWidth = m_scrollWindow->contentWidth();

        // update height
        if(isScrollBarVisible(eVerticalScrollBar))
            scrollWindowHeight = m_scrollWindow->contentHeight();

        m_scrollWindow->update(posX - scrollOffsetX(), posY - scrollOffsetY(), scrollWindowWidth, scrollWindowHeight);
    }
}

void XWScrollViewWindow::updateScrollBar(TScrollOrientation scrollOrient, int posX, int posY, int width, int height)
{
    // NOTE: in cases when scrollbar size doesn't change (e.g. resizing in one direction) window doesn't get
    //       WM_SIZE message and so scroll slider size and position do not get updated. So we handle it here.

    // check orientation
    if(scrollOrient == eVerticalScrollBar)
    {
        bool needsContentUpdate = (m_verticalScrollBar->height() == height);

        m_verticalScrollBar->update(posX, posY, width, height);

        if(needsContentUpdate)
            m_verticalScrollBar->onScrollContentChanged();
    }
    else if(scrollOrient == eHorizontalScrollBar)
    {
        bool needsContentUpdate = (m_verticalScrollBar->width() == width);

        m_horizontalScrollBar->update(posX, posY, width, height);

        if(needsContentUpdate)
            m_horizontalScrollBar->onScrollContentChanged();
    }
}

void XWScrollViewWindow::showScrollBar(TScrollOrientation scrollOrient, bool bShow)
{
    // check orientation
    if(scrollOrient == eVerticalScrollBar)
    {
        m_verticalScrollBar->show(bShow ? SW_SHOW: SW_HIDE);

        // reset scroll offset if scrollbar gets hidden
        if(!bShow)
            setScrollOffsetY(0);
    }
    else if(scrollOrient == eHorizontalScrollBar)
    {
        m_horizontalScrollBar->show(bShow ? SW_SHOW: SW_HIDE);

        // reset scroll offset if scrollbar gets hidden
        if(!bShow)
            setScrollOffsetX(0);
    }
}

bool XWScrollViewWindow::isScrollBarVisible(TScrollOrientation scrollOrient)
{
    // check orientation
    if(scrollOrient == eVerticalScrollBar)
        return m_verticalScrollBar->isVisible();
    else if(scrollOrient == eHorizontalScrollBar)
        return m_horizontalScrollBar->isVisible();

    return false;
}

int XWScrollViewWindow::scrollBarWidth(TScrollOrientation scrollOrient)
{
    // check orientation
    if(scrollOrient == eVerticalScrollBar)
        return m_verticalScrollBar->minWidth();
    else if(scrollOrient == eHorizontalScrollBar)
        return m_horizontalScrollBar->minHeight();

    return 0;
}

/////////////////////////////////////////////////////////////////////
// events
/////////////////////////////////////////////////////////////////////
void XWScrollViewWindow::onConnectedObjectRemoved(XWObject* obj)
{
    // check if scroll window has been removed
    if(m_scrollWindow && obj && m_scrollWindow->xwoid() == obj->xwoid())
    {
        // reset reference
        m_scrollWindow = 0;
    }

    // pass to parent
    XWindow::onConnectedObjectRemoved(obj);
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XWScrollViewWindow::_doScrollItem(int scrollOffsetX, int scrollOffsetY)
{
    // ignore if window is not set 
    if(m_scrollWindow == 0 || m_scrollWindow->hwnd() == 0) return;

    // update offset
    XWindow::setScrollOffsetX(scrollOffsetX);
    XWindow::setScrollOffsetY(scrollOffsetY);

    // move window
    ::SetWindowPos(m_scrollWindow->hwnd(), 0, -scrollOffsetX, -scrollOffsetY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING);
}

// XWScrollViewWindow
/////////////////////////////////////////////////////////////////////
