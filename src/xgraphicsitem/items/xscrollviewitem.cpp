// Scroll view grapchics item implementation
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"
#include "../../graphics/xwgraphics.h"

#include "../xgraphicsitem.h"
#include "../interfaces/ixscrollbaritem.h"

#include "xscrollbaritem.h"
#include "xscrollviewitem.h"

/////////////////////////////////////////////////////////////////////
// XScrollViewItem - scroll view item implementation

XScrollViewItem::XScrollViewItem(XGraphicsItem* parent) :
    m_scrollGraphicsItem(0)
{
    // create scrollbars
    m_verticalScrollBar = new XScrollBarItem(eXScrollBarVertical, this);
    m_horizontalScrollBar = new XScrollBarItem(eXScrollBarHorizontal, this);

    // default styles
    m_viewStyle = XWUIStyle::scrollViewStyle();
    m_scrollbarStyle = XWUIStyle::scrollBarStyle();

    // init styles
    m_verticalScrollBar->setScrollBarStyle(m_scrollbarStyle);
    m_horizontalScrollBar->setScrollBarStyle(m_scrollbarStyle);

    // init scrollbars with view itself
    m_verticalScrollBar->setScrollItem(this);
    m_horizontalScrollBar->setScrollItem(this);

    // hide scrollbars by default
    m_verticalScrollBar->setVisible(false);
    m_horizontalScrollBar->setVisible(false);

    // add to parent
    if(parent)
        parent->addChildItem(this);
}

XScrollViewItem::~XScrollViewItem()
{
    // reset D2D resources if any
    onResetD2DTarget();
}

/////////////////////////////////////////////////////////////////////
// style
/////////////////////////////////////////////////////////////////////
void XScrollViewItem::setStyle(const XWUIStyle::XScrollViewStyle& style)
{
    // copy style
    m_viewStyle = style;
}

const XWUIStyle::XScrollViewStyle& XScrollViewItem::getStyle() const
{
    return m_viewStyle;
}

/////////////////////////////////////////////////////////////////////
// scrollbar style
/////////////////////////////////////////////////////////////////////
void XScrollViewItem::setScrollbarStyle(const XWUIStyle::XScrollBarStyle& style)
{
    // copy style
    m_scrollbarStyle = style;

    // set style 
    m_verticalScrollBar->setScrollBarStyle(style);
    m_horizontalScrollBar->setScrollBarStyle(style);
}

const XWUIStyle::XScrollBarStyle& XScrollViewItem::getScrollbarStyle() const
{
    return m_scrollbarStyle;
}

/////////////////////////////////////////////////////////////////////
// scrollable item
/////////////////////////////////////////////////////////////////////
void XScrollViewItem::setScrollItem(XGraphicsItem* scrollItem)
{
    // remove previous item if any
    delete m_scrollGraphicsItem;

    // copy reference
    m_scrollGraphicsItem = scrollItem;

    // set properties
    if(scrollItem) 
    {
        // take ownership
        addChildItem(scrollItem);

        // pass to logic
        XWScrollViewLogic::setScrollableItem(scrollItem);

        // listen for window deletion
        addConnectedObject(scrollItem);

        // visibility
        scrollItem->setVisible(isVisible());
    }
    
    // update visibility
    m_verticalScrollBar->setVisible(m_showVerticalScrollBar == eScrollBarShowAlways);
    m_horizontalScrollBar->setVisible(m_showHorizontalScrollBar == eScrollBarShowAlways);

    // layout
    XWScrollViewLogic::updateScrollView(0, 0, width(), height());
}

/////////////////////////////////////////////////////////////////////
// scrolling 
/////////////////////////////////////////////////////////////////////
void XScrollViewItem::scrollContent(int scrollX, int scrollY)
{
    // TODO: check if possible

    // pass to scrollbars
    if(m_showVerticalScrollBar != eScrollBarShowNever)
        m_verticalScrollBar->scrollContent(scrollY);
    if(m_showHorizontalScrollBar != eScrollBarShowNever)
        m_horizontalScrollBar->scrollContent(scrollX);
}

void XScrollViewItem::scrollContentTop()
{
    // pass to scrollbar
    m_verticalScrollBar->scrollContent(0);
}

void XScrollViewItem::scrollContentBottom()
{
    // pass to scrollbar
    m_verticalScrollBar->scrollContent(m_verticalScrollBar->getMaxScrollOffset());
}

void XScrollViewItem::scrollContentLeft()
{
    // pass to scrollbar
    m_horizontalScrollBar->scrollContent(0);
}

void XScrollViewItem::scrollContentRight()
{
    // pass to scrollbar
    m_horizontalScrollBar->scrollContent(m_horizontalScrollBar->getMaxScrollOffset());
}

/////////////////////////////////////////////////////////////////////
// scrolling (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
bool XScrollViewItem::canScrollContent()
{
    // can scroll
    return true;
}

int XScrollViewItem::contentWidth()
{
    // pass to item if set
    if(m_scrollGraphicsItem)
        return m_scrollGraphicsItem->contentWidth();

    // pass to parent
    return XGraphicsItem::contentWidth();
}

int XScrollViewItem::contentHeight()
{
    // pass to item if set
    if(m_scrollGraphicsItem)
        return m_scrollGraphicsItem->contentHeight();

    // pass to parent
    return XGraphicsItem::contentWidth();
}

int XScrollViewItem::scrollOffsetX()
{
    // pass to item if set and scrollable
    if(m_scrollGraphicsItem && m_scrollGraphicsItem->canScrollContent())
        return m_scrollGraphicsItem->scrollOffsetX();

    // pass to parent
    return XGraphicsItem::scrollOffsetX();
}

int XScrollViewItem::scrollOffsetY()
{
    // pass to item if set and scrollable
    if(m_scrollGraphicsItem && m_scrollGraphicsItem->canScrollContent())
        return m_scrollGraphicsItem->scrollOffsetY();

    // pass to parent
    return XGraphicsItem::scrollOffsetY();
}

void XScrollViewItem::setScrollOffsetX(int scrollOffsetX)
{
    // pass to item if set and scrollable
    if(m_scrollGraphicsItem && m_scrollGraphicsItem->canScrollContent())
    {
        m_scrollGraphicsItem->setScrollOffsetX(scrollOffsetX);
    }
    else
    {
        // scroll item
        _doScrollItem(scrollOffsetX, 0);
    }
}

void XScrollViewItem::setScrollOffsetY(int scrollOffsetY)
{
    // pass to item if set and scrollable
    if(m_scrollGraphicsItem && m_scrollGraphicsItem->canScrollContent())
    {
        m_scrollGraphicsItem->setScrollOffsetY(scrollOffsetY);
    }
    else
    {
        // scroll item
        _doScrollItem(0, scrollOffsetY);
    }
}

int XScrollViewItem::scrollOffsetForWheel(int wheelDelta)
{
    // pass to item if set and scrollable
    if(m_scrollGraphicsItem && m_scrollGraphicsItem->canScrollContent())
        return m_scrollGraphicsItem->scrollOffsetForWheel(wheelDelta);

    // pass to parent
    return XGraphicsItem::scrollOffsetForWheel(wheelDelta);
}

/////////////////////////////////////////////////////////////////////
// customize scrollbars
/////////////////////////////////////////////////////////////////////
void XScrollViewItem::setVerticalScrollBar(IXScollBarItem* pIXScollBarItem)
{
    XWASSERT(pIXScollBarItem);
    if(pIXScollBarItem == 0) return;

    // delete old scrollbar if set
    delete m_verticalScrollBar;
    m_verticalScrollBar = pIXScollBarItem;

    // take ownership
    addChildItem(m_verticalScrollBar);

    // set style
    m_verticalScrollBar->setScrollBarStyle(m_scrollbarStyle);

    // init scrollbar with view itself
    m_verticalScrollBar->setScrollItem(this);

    // update visibility
    m_verticalScrollBar->setVisible(m_showVerticalScrollBar == eScrollBarShowAlways);
}

void XScrollViewItem::setHorizontalScrollBar(IXScollBarItem* pIXScollBarItem)
{
    XWASSERT(pIXScollBarItem);
    if(pIXScollBarItem == 0) return;

    // delete old scrollbar if set
    delete m_horizontalScrollBar;
    m_horizontalScrollBar = pIXScollBarItem;
    
    // take ownership
    addChildItem(m_horizontalScrollBar);

    // set style
    m_horizontalScrollBar->setScrollBarStyle(m_scrollbarStyle);

    // init scrollbar with view itself
    m_horizontalScrollBar->setScrollItem(this);

    // update visibility
    m_horizontalScrollBar->setVisible(m_showHorizontalScrollBar == eScrollBarShowAlways);
}

/////////////////////////////////////////////////////////////////////
// manipulations (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XScrollViewItem::update(int posX, int posY, int width, int height)
{
    // pass to parent first
    XGraphicsItem::update(posX, posY, width, height);

    // pass to scroll logic
    XWScrollViewLogic::updateScrollView(0, 0, width, height);
}

/////////////////////////////////////////////////////////////////////
// focus (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
bool XScrollViewItem::isFocusable() const
{
    return true;
}

/////////////////////////////////////////////////////////////////////
// mouse events (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XScrollViewItem::onMouseEnter(int posX, int posY)
{
    // check if mouse drag is enabled
    if(m_bMouseDragScrollEnabled)
    {
        // TODO: 

    } else 
    {
        // pass to child items
        XGraphicsItem::onMouseEnter(posX, posY);
    }
}

void XScrollViewItem::onMouseMove(int posX, int posY, WPARAM flags)
{
    // check if mouse drag is enabled
    if(m_bMouseDragScrollEnabled)
    {
        // TODO: 

    } else 
    {
        // pass to child items
        XGraphicsItem::onMouseMove(posX, posY, flags);
    }
}

bool XScrollViewItem::onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags)
{
    // check if mouse drag is enabled
    if(m_bMouseDragScrollEnabled)
    {
        // TODO: 

    } else 
    {
        // pass to child items
        return XGraphicsItem::onMouseClick(uButtonMsg, posX, posY, flags);
    }

    return false;
}

void XScrollViewItem::onMouseLeave()
{
    // check if mouse drag is enabled
    if(m_bMouseDragScrollEnabled)
    {
        // TODO: 

    } else 
    {
        // pass to child items
        XGraphicsItem::onMouseLeave();
    }
}

bool XScrollViewItem::onMouseWheel(int wheelDelta, int posX, int posY, WPARAM flags)
{
    if(m_bMouseWheelScrollEnabled)
    {
        if(m_scrollGraphicsItem && m_verticalScrollBar != 0 && m_verticalScrollBar->isVisible())
        {
            //XWTRACE1("%d", wheelDelta);

            // get scroll offset for delta
            int scrollOffsetY = m_scrollGraphicsItem->scrollOffsetForWheel(wheelDelta);

            // scroll content (scrollbar will handle possible overflows)
            m_verticalScrollBar->scrollContent(-scrollOffsetY);

            // mark as consumed
            return true;
        }

    } else
    {
        // pass to scroll item
        if(m_scrollGraphicsItem)
        {
            return m_scrollGraphicsItem->onMouseWheel(wheelDelta, posX, posY, flags);
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////
// keyboard events (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
bool XScrollViewItem::onCharEvent(WPARAM charCode, LPARAM flags)
{
    // pass child items
    return XGraphicsItem::onCharEvent(charCode, flags);
}

/////////////////////////////////////////////////////////////////////
// GDI painting
/////////////////////////////////////////////////////////////////////
void XScrollViewItem::onPaintGDI(HDC hdc, const RECT& rcPaint)
{
    // check if both scrollbars are active
    if(m_verticalScrollBar && m_verticalScrollBar->isVisible() &&
        m_horizontalScrollBar && m_horizontalScrollBar->isVisible())
    {
        // paint small rect in bottom right corner
        RECT fillRect = rect();
        fillRect.left = fillRect.right - m_verticalScrollBar->width();
        fillRect.top = fillRect.bottom - m_horizontalScrollBar->height();
        XGdiHelpers::fillRect(hdc, fillRect, 
            XWUIStyle::getStateColor(styleState(), m_viewStyle.fillColors));
    }

    // paint child items
    XGraphicsItem::onPaintGDI(hdc, rcPaint);
}

/////////////////////////////////////////////////////////////////////
// Direct2D painting
/////////////////////////////////////////////////////////////////////
void XScrollViewItem::onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint)
{
    // check if both scrollbars are active
    if(m_verticalScrollBar->isVisible() && m_horizontalScrollBar->isVisible())
    {
        // paint small rect in bottom right corner
        RECT fillRect = rect();
        fillRect.left = fillRect.right - m_verticalScrollBar->width();
        fillRect.top = fillRect.bottom - m_horizontalScrollBar->height();

        // convert item rectangle to DIPs
        D2D1_RECT_F d2dRect;
        XD2DHelpers::gdiRectToD2dRect(fillRect, d2dRect);

        // fill
        ID2D1Brush* backgroundFillBrush = createD2DBrush(pTarget, XWUIStyle::getStateColorD2D(styleState(), m_viewStyle.fillColors));
        if(backgroundFillBrush == 0) return;
        pTarget->FillRectangle(d2dRect, backgroundFillBrush);
        backgroundFillBrush->Release();
    }

    // pass to parent
    XGraphicsItem::onPaintD2D(pTarget, rcPaint);
}

/////////////////////////////////////////////////////////////////////
// message processing
/////////////////////////////////////////////////////////////////////
LRESULT XScrollViewItem::processWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& messageProcessed)
{
    // check if scrolled item is processing window messages
    if(m_scrollGraphicsItem && m_scrollGraphicsItem->processingMessages())
    {
        // protect some messages from to be consumed by scrolled item
        if(m_bMouseWheelScrollEnabled && uMsg == WM_MOUSEWHEEL)
        {
            // allow default processing
            messageProcessed = false;

            return 0;
        }
    }

    // pass to parent
    return XGraphicsItem::processWindowMessage(hwnd, uMsg, wParam, lParam, messageProcessed);
}

/////////////////////////////////////////////////////////////////////
// scroll logic methods (from XWScrollViewLogic)
/////////////////////////////////////////////////////////////////////
void XScrollViewItem::updateScrollItem(int posX, int posY, int width, int height)
{
    if(m_scrollGraphicsItem == 0) return;

    if(m_scrollGraphicsItem->canScrollContent())
    {
        // item is scrolling
        m_scrollGraphicsItem->update(posX, posY, width, height);

    } else
    {
        // view is scrolling
        int scrollWindowWidth = width;
        int scrollWindowHeight = height;

        // update width
        if(isScrollBarVisible(eHorizontalScrollBar))
            scrollWindowWidth = m_scrollGraphicsItem->contentWidth();

        // update height
        if(isScrollBarVisible(eVerticalScrollBar))
            scrollWindowHeight = m_scrollGraphicsItem->contentHeight();

        m_scrollGraphicsItem->update(posX - scrollOffsetX(), posY - scrollOffsetY(), scrollWindowWidth, scrollWindowHeight);
    }
}

void XScrollViewItem::updateScrollBar(TScrollOrientation scrollOrient, int posX, int posY, int width, int height)
{
    // check orientation
    if(scrollOrient == eVerticalScrollBar)
    {
        m_verticalScrollBar->update(posX, posY, width, height);

    } else if(scrollOrient == eHorizontalScrollBar)
    {
        m_horizontalScrollBar->update(posX, posY, width, height);
    }
}

void XScrollViewItem::showScrollBar(TScrollOrientation scrollOrient, bool bShow)
{
    // check orientation
    if(scrollOrient == eVerticalScrollBar)
    {
        m_verticalScrollBar->setVisible(bShow);

        // reset scroll offset if scrollbar gets hidden
        if(!bShow)
            setScrollOffsetY(0);
    }
    else if(scrollOrient == eHorizontalScrollBar)
    {
        m_horizontalScrollBar->setVisible(bShow);

        // reset scroll offset if scrollbar gets hidden
        if(!bShow)
            setScrollOffsetX(0);
    }
}

bool XScrollViewItem::isScrollBarVisible(TScrollOrientation scrollOrient)
{
    // check orientation
    if(scrollOrient == eVerticalScrollBar)
        return m_verticalScrollBar->isVisible();
    else if(scrollOrient == eHorizontalScrollBar)
        return m_horizontalScrollBar->isVisible();

    return false;
}

int XScrollViewItem::scrollBarWidth(TScrollOrientation scrollOrient)
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
void XScrollViewItem::onChildObjectRemoved(XWObject* child)
{
    // check if scroll item has been removed
    if(m_scrollGraphicsItem && child && m_scrollGraphicsItem->xwoid() == child->xwoid())
    {
        // reset reference
        m_scrollGraphicsItem = 0;
    }

    // pass to parent
    XGraphicsItem::onChildObjectRemoved(child);
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XScrollViewItem::_doScrollItem(int scrollOffsetX, int scrollOffsetY)
{
    // ignore if item is not set 
    if(m_scrollGraphicsItem == 0) return;

    // update offset
    XGraphicsItem::setScrollOffsetX(scrollOffsetX);
    XGraphicsItem::setScrollOffsetY(scrollOffsetY);

    // scroll
    m_scrollGraphicsItem->move(-scrollOffsetX, -scrollOffsetY);
}

// XScrollViewItem
/////////////////////////////////////////////////////////////////////

