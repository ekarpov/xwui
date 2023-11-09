// Horizontal boxed layout engine
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xlayoutitem.h"
#include "xlayout.h"
#include "xhboxlayout.h"

/////////////////////////////////////////////////////////////////////
// XHSpaceItem - horizontal space layout item 

class XHSpaceItem : public IXLayoutItem
{
public: // construction/destruction
    XHSpaceItem(int width) : m_nWidth(width) {}
    virtual ~XHSpaceItem(){}

public: // resize policy
    TResizePolicy   horizontalPolicy() const
    {
        // size is fixed
        return eResizeMinMax;
    }

public: // size constraints
    virtual int minWidth() const   { return m_nWidth; }
    virtual int maxWidth() const   { return m_nWidth; }

private: // data
    int     m_nWidth;
};

// XHSpaceItem
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// XHBoxLayout - horizontal layout manager 
XHBoxLayout::XHBoxLayout(XWObject* parent) :
    XWObject(parent),
    m_nSpacing(0),
    m_visibleCount(0),
    m_rpHorizontal(eResizeAny),
    m_rpVertical(eResizeAny),
    m_nMinWidth(0),
    m_nMinHeight(0),
    m_nMaxWidth(0),
    m_nMaxHeight(0)
{
}

XHBoxLayout::~XHBoxLayout()
{
    // delete all space items
    for(size_t idx = 0; idx < m_spaceItems.size(); ++idx)
    {
        delete m_spaceItems.at(idx);
    }
}

/////////////////////////////////////////////////////////////////////
// add items (NOTE: layout takes ownership)
/////////////////////////////////////////////////////////////////////
void XHBoxLayout::addItem(IXLayoutItem* item, int stretch, TAlignment alignment)
{
    _ItemRef itemRef;

    // fill item
    itemRef.item = item;
    itemRef.stretch = stretch;
    itemRef.alignment = alignment;

    // add to index
    m_layoutItems.push_back(itemRef);

    // process item
    _onItemAdded(item);
}

/////////////////////////////////////////////////////////////////////
// extra items
/////////////////////////////////////////////////////////////////////
void XHBoxLayout::addSpaceItem(int size)
{
    // create new space item
    XHSpaceItem* item = new XHSpaceItem(size);

    // add to space items list
    m_spaceItems.push_back(item);

    // add to layout
    addItem(item);
}

void XHBoxLayout::addStretchItem(int stretch)
{
    // create dummy item
    IXLayoutItem* item = new IXLayoutItem;

    // add to space items list
    m_spaceItems.push_back(item);

    // add to layout
    addItem(item, stretch);
}

/////////////////////////////////////////////////////////////////////
// item spacing
/////////////////////////////////////////////////////////////////////
void XHBoxLayout::setSpacing(int spacing)
{
    m_nSpacing = spacing;
}

/////////////////////////////////////////////////////////////////////
// enum layout items (from IXLayout)
/////////////////////////////////////////////////////////////////////
int XHBoxLayout::layoutItemCount() const
{
    return (int)m_layoutItems.size();
}

void XHBoxLayout::removelayoutItemAt(int idx)
{
    if(idx >= 0 && idx < (int) m_layoutItems.size())
    {
        // remove from index
        m_layoutItems.erase(m_layoutItems.begin() + idx);

    } else
    {
        XWASSERT1(0, "XHBoxLayout::removelayoutItemAt index is out of range");
    }
}

IXLayoutItem* XHBoxLayout::layoutItemAt(int idx) const
{
    if(idx >= 0 && idx < (int) m_layoutItems.size())
    {
        // return item
        return m_layoutItems.at(idx).item;

    } else
    {
        XWASSERT1(0, "XHBoxLayout::layoutItemAt index is out of range");
        return 0;
    }
}

/////////////////////////////////////////////////////////////////////
// layout items by id (from IXLayout)
/////////////////////////////////////////////////////////////////////
void XHBoxLayout::removelayoutItem(unsigned long itemId)
{
    // find item by id
    for(std::vector<_ItemRef>::const_iterator it = m_layoutItems.begin(); it != m_layoutItems.end(); ++it)
    {
        if(it->item->layoutItemId() == itemId)
        {
            // remove item
            m_layoutItems.erase(it);

            // stop
            break;
        }
    }
}

IXLayoutItem* XHBoxLayout::layoutItem(unsigned long itemId) const
{
    // find item by id
    for(std::vector<_ItemRef>::const_iterator it = m_layoutItems.begin(); it != m_layoutItems.end(); ++it)
    {
        if(it->item->layoutItemId() == itemId)
        {
            // found 
            return it->item;
        }
    }

    // not found
    return 0;
}

/////////////////////////////////////////////////////////////////////
// manipulations (from IXLayout)
/////////////////////////////////////////////////////////////////////
void XHBoxLayout::update(int posX, int posY, int width, int height)
{
    // check if there are any items
    if(m_layoutItems.size() == 0) return;

    // update visible items count
    _updateVisibleCount();

    // update resize policies in case some items have changed
    updateResizePolicies();

    // position items vertically
    _updateVerticalLayout(posY, height);

    // position items horizontally
    _updateHorizontalLayout(posX, width);

    // update final positions
    posX += marginLeft();
    for(size_t idx = 0; idx < m_layoutItems.size(); ++idx)
    {
        // ignore not visible items
        if(!m_layoutItems[idx].item->isVisible()) continue;

        m_layoutItems[idx].posX = posX;
        posX += m_layoutItems[idx].width + m_nSpacing;
    }

    // update items
    for(size_t idx = 0; idx < m_layoutItems.size(); ++idx)
    {
        // ignore not visible items
        if(!m_layoutItems[idx].item->isVisible()) continue;

        m_layoutItems[idx].item->update(m_layoutItems[idx].posX, m_layoutItems[idx].posY,
            m_layoutItems[idx].width, m_layoutItems[idx].height);
    }
}

/////////////////////////////////////////////////////////////////////
// resize policy (from IXLayout)
/////////////////////////////////////////////////////////////////////
IXLayoutItem::TResizePolicy XHBoxLayout::horizontalPolicy() const
{
    return m_rpHorizontal;
}

IXLayoutItem::TResizePolicy XHBoxLayout::verticalPolicy() const
{
    return m_rpVertical;
}

void XHBoxLayout::updateResizePolicies()
{
    // reset policy
    m_rpHorizontal = eResizeAny;
    m_rpVertical = eResizeAny;
    m_nMinWidth = 0;
    m_nMinHeight = 0;
    m_nMaxWidth = 0;
    m_nMaxHeight = 0;

    // 1. Horizontal policy
    // if any item has min size then whole layout has min size (sum of those)
    // if all items have max size then whole layout has max size (sum of those)

    // 2. Vertical policy
    // if any item has min size then whole layout has min size (maximum of those)
    // if any item has max size then whole layout has max size (maximum of those)

    bool hMinSize = false;
    bool hMaxSize = true;
    bool vMinSize = false;
    bool vMaxSize = false;
    
    for(size_t idx = 0; idx < m_layoutItems.size(); ++idx)
    {
        // ignore not visible items
        if(!m_layoutItems[idx].item->isVisible()) continue;

        // update policies for item first
        m_layoutItems.at(idx).item->updateResizePolicies();

        // policies
        TResizePolicy hPolicy = m_layoutItems.at(idx).item->horizontalPolicy();
        TResizePolicy vPolicy = m_layoutItems.at(idx).item->verticalPolicy();

        // horizontal
        if(hPolicy == eResizeMin || hPolicy == eResizeMinMax) hMinSize = true;
        if(hPolicy == eResizeAny || hPolicy == eResizeMin) hMaxSize = false;

        // vertical
        if(vPolicy == eResizeMin || vPolicy == eResizeMinMax) vMinSize = true;
        if(vPolicy == eResizeMax || vPolicy == eResizeMinMax) vMaxSize = true;

        // sum widths
        m_nMinWidth += m_layoutItems.at(idx).item->minWidth();
        m_nMaxWidth += m_layoutItems.at(idx).item->maxWidth();

        // maximum for heights
        if(m_layoutItems.at(idx).item->minHeight() > m_nMinHeight) m_nMinHeight = m_layoutItems.at(idx).item->minHeight();
        if(m_layoutItems.at(idx).item->maxHeight() > m_nMaxHeight) m_nMaxHeight = m_layoutItems.at(idx).item->maxHeight();
    }

    // horizontal
    if(hMinSize && hMaxSize)
        m_rpHorizontal = eResizeMinMax;
    else if(hMinSize)
        m_rpHorizontal = eResizeMin;
    else if(hMaxSize)
        m_rpHorizontal = eResizeMax;

    // vertical
    if(vMinSize && vMaxSize)
        m_rpVertical = eResizeMinMax;
    else if(vMinSize)
        m_rpVertical = eResizeMin;
    else if(vMaxSize)
        m_rpVertical = eResizeMax;
}

/////////////////////////////////////////////////////////////////////
// size constraints (from IXLayout)
/////////////////////////////////////////////////////////////////////
int XHBoxLayout::minWidth()  const
{
    if(m_visibleCount > 1)
        return m_nMinWidth + marginLeft() + marginRight() + (m_visibleCount - 1) * m_nSpacing;
    else
        return m_nMinWidth + marginLeft() + marginRight();
}

int XHBoxLayout::minHeight() const
{
    return m_nMinHeight + marginTop() + marginBottom();
}

int XHBoxLayout::maxWidth()  const
{
    if(m_visibleCount > 1)
        return m_nMaxWidth + marginLeft() + marginRight() + (m_visibleCount - 1) * m_nSpacing;
    else
        return m_nMaxWidth + marginLeft() + marginRight();
}

int XHBoxLayout::maxHeight() const
{
    return m_nMaxHeight + marginTop() + marginBottom();
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XHBoxLayout::_onItemAdded(IXLayoutItem* item)
{
    // update policy
    updateResizePolicies();
}

void XHBoxLayout::_onItemRemoved(IXLayoutItem* item)
{
    // update policy
    updateResizePolicies();
}

/////////////////////////////////////////////////////////////////////
// layout methods
/////////////////////////////////////////////////////////////////////
void XHBoxLayout::_updateVisibleCount()
{
    // count visible items
    m_visibleCount = 0;
    for(size_t idx = 0; idx < m_layoutItems.size(); ++idx)
    {
        if(m_layoutItems.at(idx).item->isVisible()) m_visibleCount++;
    }
}

void XHBoxLayout::_updateVerticalLayout(int posY, int height)
{
    // count available size
    int heightLeft = height - (marginTop() + marginBottom());

    for(size_t idx = 0; idx < m_layoutItems.size(); ++idx)
    {
        // ignore not visible items
        if(!m_layoutItems[idx].item->isVisible()) continue;

        // get item resize constraints
        IXLayoutItem::TResizePolicy policy = m_layoutItems[idx].item->verticalPolicy();
        int minHeight = m_layoutItems[idx].item->minHeight();
        int maxHeight = m_layoutItems[idx].item->maxHeight();

        // check if item can use full height
        if( (policy == IXLayoutItem::eResizeAny) ||
            (policy == IXLayoutItem::eResizeMin && minHeight <= heightLeft) ||
            (policy == IXLayoutItem::eResizeMax && maxHeight >= heightLeft) ||
            (policy == IXLayoutItem::eResizeMinMax && minHeight <= heightLeft && maxHeight >= heightLeft)
            )
        {
            m_layoutItems[idx].height = heightLeft;

        } else if( (policy == IXLayoutItem::eResizeMin || 
			policy == IXLayoutItem::eResizeMinMax) && minHeight > heightLeft)
        {
			m_layoutItems[idx].height = minHeight;

        } else if( (policy == IXLayoutItem::eResizeMax ||
            policy == IXLayoutItem::eResizeMinMax) && maxHeight < heightLeft)
        {
            m_layoutItems[idx].height = maxHeight;
		}

        // check if item needs to be aligned
        if(m_layoutItems[idx].height == heightLeft || m_layoutItems[idx].alignment == eAlignTop)
        {
            m_layoutItems[idx].posY = posY + marginTop();

        } else if(m_layoutItems[idx].alignment == eAlignBottom)
        {
            m_layoutItems[idx].posY = posY + marginTop() + (heightLeft - m_layoutItems[idx].height);

        } else if(m_layoutItems[idx].alignment == eAlignCenter)
        {
            m_layoutItems[idx].posY = posY + marginTop() + ((heightLeft - m_layoutItems[idx].height) / 2);
        }
    }
}

void XHBoxLayout::_initLayoutCache(int& fixedSize, int& freeItems, int& totalStretch)
{
    for(size_t idx = 0; idx < m_layoutItems.size(); ++idx)
    {
        // ignore not visible items
        if(!m_layoutItems[idx].item->isVisible()) continue;

        // get item
        IXLayoutItem* item = m_layoutItems[idx].item;

        // reset initial width
        m_layoutItems[idx].width = 0;

        // check if item has fixed width
        if(item->horizontalPolicy() != IXLayoutItem::eResizeAny && 
           item->minWidth() == item->maxWidth())
        {
            // set needed width
            m_layoutItems[idx].width = item->minWidth();

            // item is ready (can't resize)
            m_layoutItems[idx].ready = true;

            // decrease counter
            --freeItems;

            // update reserved space
            fixedSize += m_layoutItems[idx].width;

        } else 
        {
            // reset width with minimum size
            m_layoutItems[idx].width = item->minWidth();

            // reset flag
            m_layoutItems[idx].ready = false;

            // update stretch factor if any
            totalStretch += m_layoutItems[idx].stretch;
        }
    }
}

void XHBoxLayout::_findItemSize(int& sizeLeft, int& itemsLeft, int& stretchItems, bool preserveMinSize)
{
    // init counters
    int proposedSize = 0;
    float stretchFactor = 0;
    bool sizeFound = false;

    // loop over all items to make sure size is preserved
    while(sizeLeft > 0 && itemsLeft > 0 && !sizeFound)
    {
        // check if there are stretch items
        if(stretchItems)
        {
            // only stretchable items can grow -> compute stretching multiplier
            stretchFactor = (float)sizeLeft / (float)stretchItems;

        } else
        {
            // no stretching rules set-> all items can get same size
            proposedSize = sizeLeft / itemsLeft;
        }
        
        // check items
        sizeFound = true;
        for(size_t idx = 0; idx < m_layoutItems.size(); ++idx)
        {
            // ignore not visible items
            if(!m_layoutItems[idx].item->isVisible()) continue;

            // ignore if ready
            if(m_layoutItems[idx].ready) continue;

            // get item
            IXLayoutItem* item = m_layoutItems[idx].item;

            // update proposed size for item if stretching is in use
            if(stretchItems) proposedSize = (int)(m_layoutItems[idx].stretch * stretchFactor);

            // check if size fits item constraints
            if(preserveMinSize)
            {
                if((item->horizontalPolicy() == IXLayoutItem::eResizeMin ||
                    item->horizontalPolicy() == IXLayoutItem::eResizeMinMax) &&
                    proposedSize < item->minWidth())
                {
                    // set item size
                    m_layoutItems[idx].width = item->minWidth();
                    sizeFound = false;
                }

            } else 
            {
                if((item->horizontalPolicy() == IXLayoutItem::eResizeMax ||
                    item->horizontalPolicy() == IXLayoutItem::eResizeMinMax) &&
                    proposedSize > item->maxWidth())
                {
                    // set item size
                    m_layoutItems[idx].width = item->maxWidth();
                    sizeFound = false;
                }
            }

            // check if search has to stop
            if(!sizeFound)
            {
                // fix item size
                m_layoutItems[idx].ready = true;

                // reduce size
                sizeLeft -= m_layoutItems[idx].width;

                // decrease total strecth factor if any
                stretchItems -= m_layoutItems[idx].stretch;

                // size has to be iterated again
                break;
            }
        }
    }
}

void XHBoxLayout::_updateHorizontalLayout(int posX, int width)
{
    // total margins
    int marginX = marginLeft() + marginRight();

    // count spacing needed for all items
    int spacing = m_visibleCount > 0 ? (m_visibleCount - 1) * m_nSpacing : 0;

    // init counters
    int fixedSize = marginX + spacing;
    int itemsLeft = m_visibleCount;
    int stretchItems = 0;

    // init layout cache
    _initLayoutCache(fixedSize, itemsLeft, stretchItems);

    // size left
    int sizeLeft = width - fixedSize;

    // check if there is any size left to position items
    if(sizeLeft <= 0 || itemsLeft <= 0)
    {
        // NOTE: all minimum and fixed sized items where set in _initLayoutCache, 
        //       others will have zero height
        return;
    }

    // loop over all items to make sure minimum size is preserved
    _findItemSize(sizeLeft, itemsLeft, stretchItems, true);

    // loop over all items to make sure maximum size is preserved
    _findItemSize(sizeLeft, itemsLeft, stretchItems, false);

    // check if there is any size left to position items
    if(sizeLeft <= 0) return;

    // init counters
    int proposedSize = 0;
    float stretchFactor = 0;

    // check if there are stretch items
    if(stretchItems)
    {
        // only stretchable items can grow -> compute stretching multiplier
        stretchFactor = (float)sizeLeft / (float)stretchItems;

    } else
    {
        // no stretching rules set-> all items can get same size
        proposedSize = sizeLeft / itemsLeft;
    }

    // layout items
    for(size_t idx = 0; idx < m_layoutItems.size(); ++idx)
    {
        // ignore not visible items
        if(!m_layoutItems[idx].item->isVisible()) continue;

        // ignore if ready
        if(m_layoutItems[idx].ready) continue;

        // set item size
        if(stretchItems)
            m_layoutItems[idx].width = (int) (m_layoutItems[idx].stretch * stretchFactor);
        else
            m_layoutItems[idx].width = proposedSize;

        // update size
        sizeLeft -= m_layoutItems[idx].width;
    }

    // distribute rounding errors if any
    for(int loopIdx = 0; loopIdx < (int)m_layoutItems.size() && sizeLeft > 0; ++loopIdx)
    {
        // ignore not visible items
        if(!m_layoutItems[loopIdx].item->isVisible()) continue;

        // add one pixel to each item
        if(!m_layoutItems[loopIdx].ready) m_layoutItems[loopIdx].width++;

        // reduce size
        --sizeLeft;
    }
}

// XHBoxLayout
/////////////////////////////////////////////////////////////////////
