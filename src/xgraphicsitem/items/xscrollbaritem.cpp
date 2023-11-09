// Scroll bar grapchics item implementation
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"
#include "../../graphics/xwgraphics.h"

#include "../xgraphicsitem.h"
#include "../interfaces/ixscrollbaritem.h"

#include "xscrollbaritem.h"

/////////////////////////////////////////////////////////////////////
// XScrollBarItem - scroll bar implementation

XScrollBarItem::XScrollBarItem(TXScrollBarOrientation orientation, XGraphicsItem* parent) :
    IXScollBarItem(parent),
    m_scrollSliderSize(0),
    m_scrollSliderOffset(0),
    m_scrollItemSize(0),
    m_scrollContentSize(0),
    m_scrollStep(0.0f),
    m_hasScrollContent(false),
    m_mouseMoveActive(false),
    m_mousePosX(0),
    m_mousePosY(0),
    m_scrollPos(0),
    m_scrollItem(0),
    m_orientaiton(orientation)
{
    // set default style
    setStyle(XWUIStyle::scrollBarStyle());
}

XScrollBarItem::~XScrollBarItem()
{
    // reset D2D resources if any
    onResetD2DTarget();
}

/////////////////////////////////////////////////////////////////////
// style
/////////////////////////////////////////////////////////////////////
void XScrollBarItem::setStyle(const XWUIStyle::XScrollBarStyle& style)
{
    // copy style 
    m_scrollBarStyle = style;

    // apply metrics
    if(m_orientaiton == eXScrollBarVertical)
    {
        setFixedWidth(m_scrollBarStyle.barWidth);

    } else if(m_orientaiton == eXScrollBarHorizontal)
    {
        setFixedHeight(m_scrollBarStyle.barWidth);
    }
}

const XWUIStyle::XScrollBarStyle& XScrollBarItem::getStyle() const
{
    return m_scrollBarStyle;
}

/////////////////////////////////////////////////////////////////////
// scrolling interface
/////////////////////////////////////////////////////////////////////
void XScrollBarItem::onScrollContentChanged()
{
    // update size properties
    _updateScrollProperties();

    // auto scroll item if size has increased
    if(m_orientaiton == eXScrollBarVertical)
    {
        // check size
        if(m_scrollItem->scrollOffsetY() > 0 && m_scrollItemSize - m_scrollItem->scrollOffsetY() < height())
        {
            // scroll item to fit whole space
            m_scrollItem->setScrollOffsetY(m_scrollContentSize);

        } else if(m_scrollContentSize <= 0 && m_scrollItem->scrollOffsetY() != 0)
        {
            // move item on top
            m_scrollItem->setScrollOffsetY(0);
        }

    } else if(m_orientaiton == eXScrollBarHorizontal)
    {
        // check size
        if(m_scrollItem->scrollOffsetX() > 0 && m_scrollItemSize - m_scrollItem->scrollOffsetX() < width())
        {
            // scroll item to fit whole space
            m_scrollItem->setScrollOffsetX(m_scrollContentSize);

        } else if(m_scrollContentSize <= 0 && m_scrollItem->scrollOffsetX() != 0)
        {
            // move item to the left
            m_scrollItem->setScrollOffsetX(0);
        }
    }
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XScrollBarItem::setScrollBarStyle(const XWUIStyle::XScrollBarStyle& style)
{
    setStyle(style);
}

void XScrollBarItem::setScrollItem(IXWScrollable* scrollItem)
{
    // set item reference
    m_scrollItem = scrollItem;

    // update sizes
    _updateScrollProperties();
}

void XScrollBarItem::scrollContent(int scrollLen)
{
    // ignore if nothing to scroll
    if(!m_hasScrollContent) return;

    if(m_orientaiton == eXScrollBarVertical)
    {
        // check if content can be scrolled
        int scrollOffsetY = m_scrollItem->scrollOffsetY() + scrollLen;
        if(scrollOffsetY > m_scrollContentSize)
        {
            scrollOffsetY = m_scrollContentSize;

        } else if(scrollOffsetY < 0) 
        {
            scrollOffsetY = 0;
        }

        // scroll
        m_scrollItem->setScrollOffsetY(scrollOffsetY);

        // update
        handleContentChanged();

    } else if(m_orientaiton == eXScrollBarHorizontal)
    {
        // check if content can be scrolled
        int scrollOffsetX = m_scrollItem->scrollOffsetX() + scrollLen;
        if(scrollOffsetX > m_scrollContentSize)
        {
            scrollOffsetX = m_scrollContentSize;

        } else if(scrollOffsetX < 0)
        {
            scrollOffsetX = 0;
        }

        // scroll
        m_scrollItem->setScrollOffsetX(scrollOffsetX);

        // update
        handleContentChanged();
    }
}

int XScrollBarItem::getMaxScrollOffset()
{
    return (m_scrollContentSize > 0) ? m_scrollContentSize : 0;
}

/////////////////////////////////////////////////////////////////////
// manipulations (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XScrollBarItem::update(int posX, int posY, int itemWidth, int itemHeight)
{
    // pass to parent first (to lay out child items if any)
    XGraphicsItem::update(posX, posY, itemWidth, itemHeight);

    // stop if no item set
    if(m_scrollItem == 0) return;

    // update size and position
    onScrollContentChanged();
}

/////////////////////////////////////////////////////////////////////
// focus (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
bool XScrollBarItem::isFocusable() const
{
    // can have focus
    return true;
}

/////////////////////////////////////////////////////////////////////
// mouse events (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XScrollBarItem::onMouseEnter(int posX, int posY)
{
    // pass to child items first
    XGraphicsItem::onMouseEnter(posX, posY);

    // update UI
    repaint();
}

void XScrollBarItem::onMouseMove(int posX, int posY, WPARAM flags)
{
    // pass to child items first
    XGraphicsItem::onMouseMove(posX, posY, flags);

    // check if there is scrolling ongoing
    if(m_hasScrollContent && m_mouseMoveActive && m_scrollItem)
    {
        bool needsRepaint = false;

        // check orientation
        if(m_orientaiton == eXScrollBarVertical)
        {
            // move scroll bar
            needsRepaint = _moveVerticalScrollBar(posX, posY);

        } else if(m_orientaiton == eXScrollBarHorizontal)
        {
            // move scroll bar
            needsRepaint = _moveHorizontalScrollBar(posX, posY);
        }

        // repaint if needed
        if(needsRepaint)
        {
            // force repaint for performance
            repaint(true);
        }
    }

    // update mouse position
    m_mousePosX = posX;
    m_mousePosY = posY;
}

bool XScrollBarItem::onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags)
{
    // pass to child items first
    XGraphicsItem::onMouseClick(uButtonMsg, posX, posY, flags);

    // ignore if point is outside
    if(uButtonMsg == WM_LBUTTONDOWN && !isInside(posX, posY)) return false;

    // update mouse position
    m_mousePosX = posX;
    m_mousePosY = posY;

    // check if there is anything to scroll
    if(!m_hasScrollContent || m_scrollItem == 0) return false;

    // check button event
    if(uButtonMsg == WM_LBUTTONDOWN)
    {
        // check orientation
        if(m_orientaiton == eXScrollBarVertical)
        {
            // check click position
            if(posY < m_itemRect.top + m_scrollSliderOffset)
            {
                // scroll page up
                if(m_scrollItem->scrollOffsetY() > height())
                    m_scrollItem->setScrollOffsetY(m_scrollItem->scrollOffsetY() - height());
                else
                    m_scrollItem->setScrollOffsetY(0);

                // update position from item content
                 _updateScrollProperties();

            } else if(posY >= m_itemRect.top + m_scrollSliderOffset && 
                        posY <= m_itemRect.top + m_scrollSliderOffset + m_scrollSliderSize)
            {
                // start scrolling
                m_mouseMoveActive = true;

                // capture mouse
                setMouseCapture();

            } else
            {
                // count how much content left to scroll
                int scrollLeft = m_scrollContentSize - m_scrollItem->scrollOffsetY();

                // scroll page down
                if(scrollLeft > height())
                    m_scrollItem->setScrollOffsetY(m_scrollItem->scrollOffsetY() + height());
                else
                    m_scrollItem->setScrollOffsetY(m_scrollContentSize);
                
                // update position from item content
                _updateScrollProperties();
            }

            // init scroll position
            m_scrollPos = posY;

        } else
        {
            // check click position
            if(posX < m_itemRect.left + m_scrollSliderOffset)
            {
                // scroll page up
                if(m_scrollItem->scrollOffsetX() > width())
                    m_scrollItem->setScrollOffsetX(m_scrollItem->scrollOffsetX() - width());
                else
                    m_scrollItem->setScrollOffsetX(0);

                // update position from item content
                 _updateScrollProperties();

            } else if(posX >= m_itemRect.left + m_scrollSliderOffset && 
                        posX <= m_itemRect.left + m_scrollSliderOffset + m_scrollSliderSize)
            {
                // start scrolling
                m_mouseMoveActive = true;

                // capture mouse
                setMouseCapture();

            } else
            {
                // count how much content left to scroll
                int scrollLeft = m_scrollContentSize - m_scrollItem->scrollOffsetX();

                // scroll page down
                if(scrollLeft > width())
                    m_scrollItem->setScrollOffsetX(m_scrollItem->scrollOffsetX() + width());
                else
                    m_scrollItem->setScrollOffsetX(m_scrollContentSize);
                
                // update position from item content
                _updateScrollProperties();
            }

            // init scroll position
            m_scrollPos = posX;
        }

        // update UI
        repaint();
    }

    // stop scrolling if any
    if(uButtonMsg == WM_LBUTTONUP && m_mouseMoveActive)
    {
        // reset flag
        m_mouseMoveActive = false;

        // stop mouse capture
        resetMouseCapture();

        // update size just in case item hasn't actually scrolled
        _updateScrollProperties();

        // update UI
        repaint();
    }

    // consume message
    return true;
}

void XScrollBarItem::onMouseCaptureReset()
{
    // stop scrolling if any
    if(m_mouseMoveActive)
    {
        // reset flag
        m_mouseMoveActive = false;

        // update size just in case item hasn't actually scrolled
        _updateScrollProperties();

        // update UI
        repaint();
    }
}

void XScrollBarItem::onMouseLeave()
{
    // pass to child items first
    XGraphicsItem::onMouseLeave();

    // update UI
    repaint();
}

/////////////////////////////////////////////////////////////////////
// keyboard events (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
bool XScrollBarItem::onCharEvent(WPARAM charCode, LPARAM flags)
{
    // TODO:
    return false;
}

/////////////////////////////////////////////////////////////////////
// GDI painting
/////////////////////////////////////////////////////////////////////
void XScrollBarItem::onPaintGDI(HDC hdc, const RECT& rcPaint)
{
    // check if nothing to scroll
    if(!isEnabled() || !m_hasScrollContent || m_scrollItem == 0)
    {
        // just fill paint area (ignore state)
        XGdiHelpers::fillRect(hdc, m_itemRect, XWUIStyle::getStateColor(XWUIStyle::eStyleStateDefault, m_scrollBarStyle.fillColors));

    } else
    {
        RECT paintRect = m_itemRect;

        // area before slider
        if(m_orientaiton == eXScrollBarVertical)
        {
            paintRect.bottom = paintRect.top + m_scrollSliderOffset;

        } else if(m_orientaiton == eXScrollBarHorizontal) 
        {
            paintRect.right = paintRect.left + m_scrollSliderOffset;
        }

        // fill
        XGdiHelpers::fillRect(hdc, paintRect, XWUIStyle::getStateColor(styleState(), m_scrollBarStyle.fillColors));

        // slider area
        if(m_orientaiton == eXScrollBarVertical)
        {
            paintRect.top = paintRect.bottom;
            paintRect.bottom = paintRect.top + m_scrollSliderSize;

        } else if(m_orientaiton == eXScrollBarHorizontal) 
        {
            paintRect.left = paintRect.right;
            paintRect.right = paintRect.left + m_scrollSliderSize;
        }

        // paint slider
        if(m_mouseMoveActive)
            XGdiHelpers::fillRect(hdc, paintRect, m_scrollBarStyle.barColors.pressed);
        else
            XGdiHelpers::fillRect(hdc, paintRect, XWUIStyle::getStateColor(styleState(), m_scrollBarStyle.barColors));

        // area after slider
        if(m_orientaiton == eXScrollBarVertical)
        {
            paintRect.top = paintRect.bottom;
            paintRect.bottom = m_itemRect.bottom;

        } else if(m_orientaiton == eXScrollBarHorizontal) 
        {
            paintRect.left = paintRect.right;
            paintRect.right = m_itemRect.right;
        }

        // fill
        XGdiHelpers::fillRect(hdc, paintRect, XWUIStyle::getStateColor(styleState(), m_scrollBarStyle.fillColors));
    }

    // paint child items (buttons) if any
    XGraphicsItem::onPaintGDI(hdc, rcPaint);
}

/////////////////////////////////////////////////////////////////////
// Direct2D painting
/////////////////////////////////////////////////////////////////////
void XScrollBarItem::onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint)
{
    // convert item rectangle to DIPs
    D2D1_RECT_F itemRect;
    XD2DHelpers::gdiRectToD2dRect(m_itemRect, itemRect);

    // fill background (use brush cache)
    ID2D1Brush* backgroundFillBrush = createD2DBrush(pTarget, XWUIStyle::getStateColorD2D(styleState(), m_scrollBarStyle.fillColors));
    if(backgroundFillBrush == 0) return;
    pTarget->FillRectangle(itemRect, backgroundFillBrush);
    backgroundFillBrush->Release();

    // paint scroll bar if needed
    if(m_hasScrollContent && m_scrollItem)
    {
        if(m_orientaiton == eXScrollBarVertical)
        {
            // append current scroll offset
            itemRect.top += XD2DHelpers::pixelsToDipsY(m_scrollSliderOffset);

            // scroll bar height
            itemRect.bottom = itemRect.top + XD2DHelpers::pixelsToDipsY(m_scrollSliderSize);

        } else if(m_orientaiton == eXScrollBarHorizontal)
        {
            // append current scroll offset
            itemRect.left += XD2DHelpers::pixelsToDipsY(m_scrollSliderOffset);

            // scroll bar height
            itemRect.right = itemRect.left + XD2DHelpers::pixelsToDipsY(m_scrollSliderSize);
        }

        // paint scroll bar
        ID2D1Brush* barFillBrush = 0;
        if(m_mouseMoveActive)
        {            
            D2D1_COLOR_F d2dBrushColor;
            XD2DHelpers::colorrefToD2dColor(m_scrollBarStyle.barColors.pressed, d2dBrushColor);
            barFillBrush = createD2DBrush(pTarget, d2dBrushColor);
        }
        else
        {
            barFillBrush = createD2DBrush(pTarget, XWUIStyle::getStateColorD2D(styleState(), m_scrollBarStyle.barColors));
        }

        if(barFillBrush == 0) return;
        pTarget->FillRectangle(itemRect, barFillBrush);
        barFillBrush->Release();
    }

    // paint child items (buttons) if any
    XGraphicsItem::onPaintD2D(pTarget, rcPaint);
}

/////////////////////////////////////////////////////////////////////
// helper methods
/////////////////////////////////////////////////////////////////////
bool XScrollBarItem::_moveVerticalScrollBar(int mousePosX, int mousePosY)
{
    // ignore if horizontally mouse is too far away
    if(mousePosX < m_itemRect.left - 10 * width() || mousePosX > m_itemRect.right + 10 * width()) return false;

    // ignore if vertically mouse is not in range
    if(mousePosY < m_itemRect.top || mousePosY > m_itemRect.bottom) return false;

    // check if there has been any change
    if(mousePosY == m_scrollPos) return false;

    // copy previous offset
    int previousOffset = m_scrollSliderOffset;

    // move slider
    m_scrollSliderOffset += (mousePosY - m_scrollPos);

    // copy last used scroll position
    m_scrollPos = mousePosY;

    // scroll bar height
    int scrollBarHeight = _scrollBarLength();

    // adjust scroll offset in case it is out of bounds
    if(m_scrollSliderOffset < 0)
    {
        m_scrollSliderOffset = 0;

        // scroll content up
        m_scrollItem->setScrollOffsetY(0);

    } else if(m_scrollSliderOffset + m_scrollSliderSize > scrollBarHeight)
    {
        m_scrollSliderOffset = scrollBarHeight - m_scrollSliderSize;

        // scroll content bottom
        m_scrollItem->setScrollOffsetY(m_scrollContentSize);

    } else if(previousOffset != m_scrollSliderOffset)
    {
        int scrollOffsetY = (int) (m_scrollSliderOffset * m_scrollStep);

        // ignore if the same
        if(scrollOffsetY == m_scrollItem->scrollOffsetY()) return false;

        // scroll content
        m_scrollItem->setScrollOffsetY(scrollOffsetY);
    }

    // request repaint
    return true;
}

bool XScrollBarItem::_moveHorizontalScrollBar(int mousePosX, int mousePosY)
{
    // ignore if vertically mouse is too far away
    if(mousePosY < m_itemRect.top - 10 * height() || mousePosY > m_itemRect.bottom + 10 * height()) return false;

    // check if there has been any change
    if(mousePosX == m_scrollPos) return false;

    // copy previous offset
    int previousOffset = m_scrollSliderOffset;

    // move slider
    m_scrollSliderOffset += (mousePosX - m_scrollPos);

    // copy last used scroll position
    m_scrollPos = mousePosX;

    // scroll bar height
    int scrollBarHeight = _scrollBarLength();

    // adjust scroll offset in case it is out of bounds
    if(m_scrollSliderOffset < 0)
    {
        m_scrollSliderOffset = 0;

        // scroll content left
        m_scrollItem->setScrollOffsetX(0);

    } else if(m_scrollSliderOffset + m_scrollSliderSize > scrollBarHeight)
    {
        m_scrollSliderOffset = scrollBarHeight - m_scrollSliderSize;

        // scroll content bottom
        m_scrollItem->setScrollOffsetX(m_scrollContentSize);

    } else if(previousOffset != m_scrollSliderOffset)
    {
        int scrollOffsetX = (int) (m_scrollSliderOffset * m_scrollStep);

        // ignore if the same
        if(scrollOffsetX == m_scrollItem->scrollOffsetX()) return false;

        // scroll content
        m_scrollItem->setScrollOffsetX(scrollOffsetX);
    }

    // request repaint
    return true;
}

int XScrollBarItem::_scrollBarLength()
{
    // check orientation
    if(m_orientaiton == eXScrollBarVertical)
    {
        return height();

    } else if(m_orientaiton == eXScrollBarHorizontal)
    {
        return width();
    }

    return 0;
}

void XScrollBarItem::_updateScrollProperties()
{
    // reset flags
    m_hasScrollContent = false;
    
    // check if scroll item is set
    if(m_scrollItem == 0) return;

    // init sizes
    if(m_orientaiton == eXScrollBarVertical)
    {
        m_scrollItemSize = m_scrollItem->contentHeight();
        m_scrollContentSize = m_scrollItemSize - height();

    } else if(m_orientaiton == eXScrollBarHorizontal)
    {
        m_scrollItemSize = m_scrollItem->contentWidth();
        m_scrollContentSize = m_scrollItemSize - width();
    }

    // check if there is anything to scroll
    if(m_scrollContentSize > 0)
    {
        // scroll bar height
        int scrollBarLength = _scrollBarLength();

        // check if something left still
        if(scrollBarLength > 0)
        {
            // there is content to scroll
            m_hasScrollContent = true;

            // slider size
            if(m_orientaiton == eXScrollBarVertical)
            {
                m_scrollSliderSize = (int)((float)scrollBarLength * (float)height() / (float)m_scrollItemSize);

            } else if(m_orientaiton == eXScrollBarHorizontal)
            {
                m_scrollSliderSize = (int)((float)scrollBarLength * (float)width() / (float)m_scrollItemSize);
            }

            // use minimum value
            if(m_scrollSliderSize < m_scrollBarStyle.minBarSize)
                m_scrollSliderSize = m_scrollBarStyle.minBarSize;

            // scroll step
            m_scrollStep = (float)m_scrollContentSize / (float)(scrollBarLength - m_scrollSliderSize);

            // check for overflows
            if(m_scrollStep <= 0) m_scrollStep = 1;

            // slider scroll offset
            if(m_orientaiton == eXScrollBarVertical)
            {
                m_scrollSliderOffset = (int)(m_scrollItem->scrollOffsetY() / m_scrollStep);

            } else if(m_orientaiton == eXScrollBarHorizontal)
            {
                m_scrollSliderOffset = (int)(m_scrollItem->scrollOffsetX() / m_scrollStep);
            }

            // make sure it doesn't overlap
            if(m_scrollSliderOffset + m_scrollSliderSize > scrollBarLength)
                m_scrollSliderOffset = scrollBarLength - m_scrollSliderSize;
        }
    }

    // set enabled if there is scroll content
    setEnabled(m_hasScrollContent);
}

// XScrollBarItem
/////////////////////////////////////////////////////////////////////


