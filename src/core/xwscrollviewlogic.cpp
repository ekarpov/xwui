// Scroll View logic implementation
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xwscrollable.h"
#include "xwscrollviewlogic.h"

/////////////////////////////////////////////////////////////////////
// XWScrollViewLogic - common scroll view logic

XWScrollViewLogic::XWScrollViewLogic() :
    m_scrollItem(0),
    m_showVerticalScrollBar(eScrollBarShowNever),
    m_showHorizontalScrollBar(eScrollBarShowNever),
    m_bMouseDragScrollEnabled(false),
    m_bMouseWheelScrollEnabled(true),
    m_bKeyScrollEnabled(true)
{
}

XWScrollViewLogic::~XWScrollViewLogic()
{
}

/////////////////////////////////////////////////////////////////////
// scrollbars
/////////////////////////////////////////////////////////////////////
void XWScrollViewLogic::setShowScrollBar(TScrollOrientation scrollOrient, TShowScrollbar showScrollbar)
{
    // update
    if(scrollOrient == eVerticalScrollBar)
    {
        m_showVerticalScrollBar = showScrollbar;

    } else if(scrollOrient == eHorizontalScrollBar)
    {
        m_showHorizontalScrollBar = showScrollbar;
    }

    // show or hide scrollbar if needed
    if(showScrollbar == eScrollBarShowNever)
    {
        showScrollBar(scrollOrient, false);

    } else if(showScrollbar == eScrollBarShowAlways)
    {
        showScrollBar(scrollOrient, true);
    }
}

void XWScrollViewLogic::setShowVerticalScrollBar(TShowScrollbar showScrollbar)
{
    setShowScrollBar(eVerticalScrollBar, showScrollbar);
}

void XWScrollViewLogic::setShowHorizontalScrollBar(TShowScrollbar showScrollbar)
{
    setShowScrollBar(eHorizontalScrollBar, showScrollbar);
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
void XWScrollViewLogic::enableMouseDragScrolling(bool bEnable)
{
    // copy flag
    m_bMouseDragScrollEnabled = bEnable;
}

void XWScrollViewLogic::enableMouseWheelScrolling(bool bEnable)
{
    // copy flag
    m_bMouseWheelScrollEnabled = bEnable;
}

void XWScrollViewLogic::enableKeyScrolling(bool bEnable)
{
    // copy flag
    m_bKeyScrollEnabled = bEnable;
}

/////////////////////////////////////////////////////////////////////
// worker interface (called by derived view)
/////////////////////////////////////////////////////////////////////
void XWScrollViewLogic::setScrollableItem(IXWScrollable* scrollItem)
{
    // set item
    m_scrollItem = scrollItem;
}

void XWScrollViewLogic::updateScrollView(int posX, int posY, int width, int height)
{
    // scrollbar offsets
    int verticalOffset = 0;
    int horizontalOffset = 0;
    if(isScrollBarVisible(eVerticalScrollBar))
    {
        horizontalOffset = scrollBarWidth(eVerticalScrollBar);
    }
    if(isScrollBarVisible(eHorizontalScrollBar))
    {
        verticalOffset = scrollBarWidth(eHorizontalScrollBar);
    }

    // check if we have scroll item
    if(m_scrollItem)
    {
        bool needsUpdate = false;

        // update item to have updated content sizes
        updateScrollItem(posX, posY, width - horizontalOffset, height - verticalOffset);

        // check if we need to auto hide or show scrollbars
        if(m_showVerticalScrollBar == eScrollBarShowAuto)
        {
            // check if we need to show scrollbar
            if(!isScrollBarVisible(eVerticalScrollBar) && m_scrollItem->contentHeight() > height)
            {
                // show scrollbar                 
                showScrollBar(eVerticalScrollBar, true);
                horizontalOffset = scrollBarWidth(eVerticalScrollBar);
                needsUpdate = true;

            } else if(isScrollBarVisible(eVerticalScrollBar) && m_scrollItem->contentHeight() <= height)
            {
                // hide scrollbar
                showScrollBar(eVerticalScrollBar, false);
                horizontalOffset = 0;
                needsUpdate = true;
            }
        }

        // check if we need to auto hide or show scrollbars
        if(m_showHorizontalScrollBar == eScrollBarShowAuto)
        {
            // check if we need to show scrollbar
            if(!isScrollBarVisible(eHorizontalScrollBar) && m_scrollItem->contentWidth() > width)
            {
                // show scrollbar                
                showScrollBar(eHorizontalScrollBar, true);
                verticalOffset = scrollBarWidth(eHorizontalScrollBar);
                needsUpdate = true;

            } else if(isScrollBarVisible(eHorizontalScrollBar) && m_scrollItem->contentWidth() <= width)
            {
                // hide scrollbar
                showScrollBar(eHorizontalScrollBar, false);
                verticalOffset = 0;
                needsUpdate = true;
            }
        }

        // set new size
        if(needsUpdate)
        {
            // update item
            updateScrollItem(posX, posY, width - horizontalOffset, height - verticalOffset);
        }
    }

    // position scrollbars
    if(isScrollBarVisible(eVerticalScrollBar))
    {
        updateScrollBar(eVerticalScrollBar, posX + width - scrollBarWidth(eVerticalScrollBar), 
            posY, scrollBarWidth(eVerticalScrollBar), height - verticalOffset);
    }
    if(isScrollBarVisible(eHorizontalScrollBar))
    {
        updateScrollBar(eHorizontalScrollBar, posX, posY + height - scrollBarWidth(eHorizontalScrollBar), 
            width - horizontalOffset, scrollBarWidth(eHorizontalScrollBar));
    }

}

void XWScrollViewLogic::scrollMouseWheel(int wheelDelta, int posX, int posY)
{
    // wheel scrolling only works with vertical scrollbar
}

// XWScrollViewLogic
/////////////////////////////////////////////////////////////////////
