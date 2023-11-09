// ScrollBar window
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

// dependencies
#include "../graphics/xwgraphics.h"
#include "../layout/xwlayouts.h"
#include "../xwindow/xwindow.h"
#include "../xgraphicsitem/xwgraphicsitems.h"

#include "xwscrollbarwindow.h"

/////////////////////////////////////////////////////////////////////
// XWScrollBarWindow - scroll bar window implementation

XWScrollBarWindow::XWScrollBarWindow(TXScrollBarOrientation orientation, HWND hWndParent, XWObject* parent) :
    XGraphicsItemWindow(WS_VISIBLE | WS_CHILD, parent, hWndParent),
    m_scrollBarItem(0)
{
    // create scroll bar item
    m_scrollBarItem = new XScrollBarItem(orientation);

    // set item
    XGraphicsItemWindow::setGraphicsItem(m_scrollBarItem);

    // set fixed size
    if(orientation == eXScrollBarVertical)
    {
        setFixedWidth(m_scrollBarItem->minWidth());
    } else
    {
        setFixedHeight(m_scrollBarItem->minHeight());
    }
}

XWScrollBarWindow::~XWScrollBarWindow()
{
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XWScrollBarWindow::setScrollItem(IXWScrollable* pScrollItem)
{
    XWASSERT(m_scrollBarItem);
    if(m_scrollBarItem == 0) return;

    // pass to item
    m_scrollBarItem->setScrollItem(pScrollItem);
}

void XWScrollBarWindow::scrollContent(int scrollLen)
{
    XWASSERT(m_scrollBarItem);
    if(m_scrollBarItem == 0) return;

    // pass to item
    m_scrollBarItem->scrollContent(scrollLen);
}

/////////////////////////////////////////////////////////////////////
// orientaiton
/////////////////////////////////////////////////////////////////////
TXScrollBarOrientation XWScrollBarWindow::orientaiton() const
{
    XWASSERT(m_scrollBarItem);
    if(m_scrollBarItem == 0) return eXScrollBarVertical;

    // pass to item
    return m_scrollBarItem->orientaiton();
}

/////////////////////////////////////////////////////////////////////
// style
/////////////////////////////////////////////////////////////////////
void XWScrollBarWindow::setStyle(const XWUIStyle::XScrollBarStyle& style)
{
    XWASSERT(m_scrollBarItem);
    if(m_scrollBarItem == 0) return;

    // pass to item
    m_scrollBarItem->setStyle(style);
}

const XWUIStyle::XScrollBarStyle& XWScrollBarWindow::getStyle() const
{
    XWASSERT(m_scrollBarItem);
    if(m_scrollBarItem == 0) return XWUIStyle::scrollBarStyle();

    // pass to item
    return m_scrollBarItem->getStyle();
}

/////////////////////////////////////////////////////////////////////
// scrolling interface
/////////////////////////////////////////////////////////////////////
bool XWScrollBarWindow::isScrollingActive() const
{
    // pass to item
    if(m_scrollBarItem)
        return m_scrollBarItem->isScrollingActive();

    return false;
}

void XWScrollBarWindow::onScrollContentChanged()
{
    // pass to item
    if(m_scrollBarItem)
        m_scrollBarItem->onScrollContentChanged();
}

/////////////////////////////////////////////////////////////////////
// hide some interfaces
/////////////////////////////////////////////////////////////////////
void XWScrollBarWindow::setGraphicsItem(XGraphicsItem* pXGraphicsItem)
{
    XWASSERT1(0, "XWScrollBarWindow::setGraphicsItem must not be used from outside");

    // pass to parent
    XGraphicsItemWindow::setGraphicsItem(pXGraphicsItem);
}

bool XWScrollBarWindow::create(DWORD dwStyle, HWND hWndParent, DWORD dwExStyle)
{
    XWASSERT1(0, "XWScrollBarWindow::create must not be used from outside");

    // pass to parent
    return XGraphicsItemWindow::create(dwStyle, hWndParent, dwExStyle);
}

// XWScrollBarWindow
/////////////////////////////////////////////////////////////////////
