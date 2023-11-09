// List graphics item
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"
#include "../../graphics/xwgraphics.h"
#include "../../layout/xlayoutitem.h"
#include "../../layout/xlayout.h"
#include "../../layout/xvboxlayout.h"

#include "../xgraphicsitem.h"

#include "xlistviewitem.h"

/////////////////////////////////////////////////////////////////////
// XListViewItem - list item

XListViewItem::XListViewItem(XGraphicsItem* parent) :
    m_itemAlignment(eAlignCenter),
    m_maxItemWidth(0),
    m_minItemWidth(0),
    m_itemsPosX(0),
    m_itemsWidth(0),
    m_itemsHeight(0),
    m_nMarginLeft(0),
    m_nMarginTop(0),
    m_nMarginRight(0),
    m_nMarginBottom(0),
    m_nItemSpacing(0)
{
    // switch off default scrolling
    enableContentScrolling(false);

    // add to parent
    if(parent)
        parent->addChildItem(this);
}

XListViewItem::~XListViewItem()
{
}

/////////////////////////////////////////////////////////////////////
// items (list takes ownership)
/////////////////////////////////////////////////////////////////////
void XListViewItem::appendListItem(XGraphicsItem* item)
{
    XWASSERT(item);
    if(item == 0) return;

    // check if already added
    if(findListItem(item->xwoid()) != 0)
    {
        XWTRACE("XListViewItem: list item already added, ignored");
        return;
    }

    // add child item
    XGraphicsItem::addChildItem(item);
}

void XListViewItem::deleteListItem(unsigned long itemId)
{
    // delete child item 
    XGraphicsItem::deleteChildItem(itemId);
}

void XListViewItem::deleteAllListItems()
{
    // delete items
    XGraphicsItem::deleteAllChildItems();
}

XGraphicsItem* XListViewItem::findListItem(unsigned long itemId)
{
    // find child item 
    return XGraphicsItem::findChildItem(itemId);
}

/////////////////////////////////////////////////////////////////////
// item list (NOTE: call handleContentChanged after updating list)
/////////////////////////////////////////////////////////////////////
std::list<XGraphicsItem*>& XListViewItem::items() 
{ 
    // NOTE: return XGraphicsItem child list
    return m_childItems; 
}

/////////////////////////////////////////////////////////////////////
// list item width constraints
/////////////////////////////////////////////////////////////////////
void XListViewItem::setMaxItemWidth(int width)
{
    // check input
    XWASSERT(width >= 0);
    if(width < 0) return;

    m_maxItemWidth = width;
}

void XListViewItem::setMinItemWidth(int width)
{
    // check input
    XWASSERT(width >= 0);
    if(width < 0) return;

    m_minItemWidth = width;
}

/////////////////////////////////////////////////////////////////////
// alignment
/////////////////////////////////////////////////////////////////////
void XListViewItem::setItemAlignment(TAlignment alignment)
{
    m_itemAlignment = alignment;
}

/////////////////////////////////////////////////////////////////////
// content 
/////////////////////////////////////////////////////////////////////
void XListViewItem::handleContentChanged()
{
    // layout items
    _layoutItems(m_itemRect.left, m_itemRect.top, width(), height());

    // pass to parent
    XGraphicsItem::handleContentChanged();
}

/////////////////////////////////////////////////////////////////////
// content margins
/////////////////////////////////////////////////////////////////////
void XListViewItem::setContentMargins(int left, int top, int right, int bottom)
{
    // check input
    XWASSERT(left >= 0);
    XWASSERT(top >= 0);
    XWASSERT(right >= 0);
    XWASSERT(bottom >= 0);
    if(left < 0 || top < 0 || right < 0 || bottom < 0) return;

    // copy margins
    m_nMarginLeft = left;
    m_nMarginTop = top;
    m_nMarginRight = right;
    m_nMarginBottom = bottom;
}

void XListViewItem::setItemSpacing(int spacing)
{
    // check input
    XWASSERT(spacing >= 0);
    if(spacing < 0) return;

    // copy spacing
    m_nItemSpacing = spacing;
}

/////////////////////////////////////////////////////////////////////
// manipulations (from IXLayoutItem)
/////////////////////////////////////////////////////////////////////
void XListViewItem::update(int posX, int posY, int width, int height)
{
    // pass to parent
    XGraphicsItem::update(posX, posY, width, height);

    // update items
    _layoutItems(posX, posY, width, height);
}

/////////////////////////////////////////////////////////////////////
// scrolling interface (from IXWScrollable)
/////////////////////////////////////////////////////////////////////
bool XListViewItem::canScrollContent()
{
    // list view will scroll items itself
    return true;
}

int XListViewItem::contentWidth()
{
    return m_itemsWidth + m_nMarginLeft + m_nMarginRight;
}

int XListViewItem::contentHeight()
{
    return m_itemsHeight + m_nMarginTop + m_nMarginBottom;
}

void XListViewItem::setScrollOffsetX(int scrollOffsetX)
{
    // copy scroll offset (NOTE: as default scrolling is disabled 
    // this should not affect items position)
    XGraphicsItem::setScrollOffsetX(scrollOffsetX);

    // move items
    _scrollItems();
}

void XListViewItem::setScrollOffsetY(int scrollOffsetY)
{
    // copy scroll offset (NOTE: as default scrolling is disabled 
    // this should not affect items position)
    XGraphicsItem::setScrollOffsetY(scrollOffsetY);

    // move items
    _scrollItems();
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XListViewItem::_layoutItems(int posX, int posY, int width, int height)
{
    // pass to parent first
    XGraphicsItem::update(posX, posY, width, height);

    // update layout data
    m_itemsWidth = width - m_nMarginLeft - m_nMarginRight;
    m_itemsPosX = posX + m_nMarginLeft;

    // check width limits
    if(m_itemsWidth < m_minItemWidth)
    {
        m_itemsWidth = m_minItemWidth;

    } else if(m_maxItemWidth > 0 && m_itemsWidth > m_maxItemWidth)
    {
        m_itemsWidth = m_maxItemWidth;

        // items need to be laid out as we have more space
        if(m_itemAlignment == eAlignRight)
        {
            m_itemsPosX = posX + (width - m_itemsWidth) - m_nMarginRight;

        } else if(m_itemAlignment == eAlignCenter)
        {
            m_itemsPosX = posX + (width - m_itemsWidth) / 2;
        }
    }

    // item position (including scrolling)
    int itemPosX = m_itemsPosX  - scrollOffsetX();
    int itemPosY = posY + m_nMarginTop - scrollOffsetY();

    // layout items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); 
        it != m_childItems.end(); ++it)
    {
        // get content width for item height
        int itemHeight = (*it)->contentHeightForWidth(m_itemsWidth);

        // layout item
        (*it)->update(m_itemsPosX, itemPosY, m_itemsWidth, itemHeight);

        // update position
        itemPosY += (*it)->height() + m_nItemSpacing;
    }

    // update items height
    m_itemsHeight = itemPosY - posY - m_nMarginTop + scrollOffsetY() - m_nItemSpacing;
}

void XListViewItem::_scrollItems()
{
    // item position (including scrolling)
    int itemPosX = m_itemsPosX  - scrollOffsetX();
    int itemPosY = rect().top + m_nMarginTop - scrollOffsetY();

    // reposition items (do not layout)
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); 
        it != m_childItems.end(); ++it)
    {
        // move items
        (*it)->move(itemPosX, itemPosY);

        // update position
        itemPosY += (*it)->height() + m_nItemSpacing;
    }

    // force repaint for performance
    repaint(true);
}

// XListViewItem
/////////////////////////////////////////////////////////////////////
