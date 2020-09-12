// Vertical boxed layout engine
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xlayoutitem.h"
#include "xlayout.h"
#include "xvboxlayout.h"

/////////////////////////////////////////////////////////////////////
// XVSpaceItem - vertical space layout item 

class XVSpaceItem : public IXLayoutItem
{
public: // construction/destruction
    XVSpaceItem(int height) : m_nHeight(height) {}
    virtual ~XVSpaceItem(){}

public: // resize policy
    TResizePolicy   verticalPolicy() const
    {
        // size is fixed
        return eResizeMinMax;
    }

public: // size constraints
    virtual int minHeight() const   { return m_nHeight; }
    virtual int maxHeight() const   { return m_nHeight; }

private: // data
    int     m_nHeight;
};

// XVSpaceItem
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// XVBoxLayout - vertical layout manager 
XVBoxLayout::XVBoxLayout(XWObject* parent) :
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

XVBoxLayout::~XVBoxLayout()
{
    // delete all space items
    for(size_t idx = 0; idx < m_spaceItems.size(); ++idx)
    {
        delete m_spaceItems.at(idx);
    }
}

/////////////////////////////////////////////////////////////////////
// add items
/////////////////////////////////////////////////////////////////////
void XVBoxLayout::addItem(IXLayoutItem* item, int stretch, TAlignment alignment)
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
void XVBoxLayout::addSpaceItem(int size)
{
    // create new space item
    XVSpaceItem* item = new XVSpaceItem(size);

    // add to space items list
    m_spaceItems.push_back(item);

    // add to layout
    addItem(item);
}

void XVBoxLayout::addStretchItem(int stretch)
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
void XVBoxLayout::setSpacing(int spacing)
{
    m_nSpacing = spacing;
}

/////////////////////////////////////////////////////////////////////
// enum layout items (from IXLayout)
/////////////////////////////////////////////////////////////////////
int XVBoxLayout::layoutItemCount() const
{
    return (int)m_layoutItems.size();
}

void XVBoxLayout::removelayoutItemAt(int idx)
{
    if(idx >= 0 && idx < (int) m_layoutItems.size())
    {
        // remove from index
        m_layoutItems.erase(m_layoutItems.begin() + idx);

    } else
    {
        XWASSERT1(0, "XVBoxLayout::removelayoutItemAt index is out of range");
    }
}

IXLayoutItem* XVBoxLayout::layoutItemAt(int idx) const
{
    if(idx >= 0 && idx < (int) m_layoutItems.size())
    {
        // return item
        return m_layoutItems.at(idx).item;

    } else
    {
        XWASSERT1(0, "XVBoxLayout::layoutItemAt index is out of range");
        return 0;
    }
}

/////////////////////////////////////////////////////////////////////
// layout items by id (from IXLayout)
/////////////////////////////////////////////////////////////////////
void XVBoxLayout::removelayoutItem(unsigned long itemId)
{
    // find item by id
    for(std::vector<_ItemRef>::iterator it = m_layoutItems.begin(); it != m_layoutItems.end(); ++it)
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

IXLayoutItem* XVBoxLayout::layoutItem(unsigned long itemId) const
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
void XVBoxLayout::update(int posX, int posY, int width, int height)
{
    // check if there are any items
    if(m_layoutItems.size() == 0) return;

    // update visible items count
    _updateVisibleCount();

    // update resize policies in case some items have changed
    updateResizePolicies();

    // position items horizontally
    _updateHorizontalLayout(posX, width);

    // position items vertically
    _updateVerticalLayout(posY, height);

    // update final positions
    posY += marginTop();
    for(size_t idx = 0; idx < m_layoutItems.size(); ++idx)
    {
        // ignore not visible items
        if(!m_layoutItems[idx].item->isVisible()) continue;

        m_layoutItems[idx].posY = posY;
        posY += m_layoutItems[idx].height + m_nSpacing;
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
IXLayoutItem::TResizePolicy XVBoxLayout::horizontalPolicy() const
{
    return m_rpHorizontal;
}

IXLayoutItem::TResizePolicy XVBoxLayout::verticalPolicy() const
{
    return m_rpVertical;
}

void XVBoxLayout::updateResizePolicies()
{
    // reset policy
    m_rpHorizontal = eResizeAny;
    m_rpVertical = eResizeAny;
    m_nMinWidth = 0;
    m_nMinHeight = 0;
    m_nMaxWidth = 0;
    m_nMaxHeight = 0;

    // 1. Horizontal policy
    // if any item has min size then whole layout has min size (maximum of those)
    // if any item has max size then whole layout has max size (maximum of those)

    // 2. Vertical policy
    // if any item has min size then whole layout has min size (sum of those)
    // if all items have max size then whole layout has max size (sum of those)

    bool hMinSize = false;
    bool hMaxSize = false;
    bool vMinSize = false;
    bool vMaxSize = true;
    
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
        if(hPolicy == eResizeMax || hPolicy == eResizeMinMax) hMaxSize = true;

        // vertical
        if(vPolicy == eResizeMin || vPolicy == eResizeMinMax) vMinSize = true;
        if(vPolicy == eResizeAny || vPolicy == eResizeMin) vMaxSize = false;

        // sum heights
        m_nMinHeight += m_layoutItems.at(idx).item->minHeight();
        m_nMaxHeight += m_layoutItems.at(idx).item->maxHeight();

        // maximum for widths
        if(m_layoutItems.at(idx).item->minWidth() > m_nMinWidth) m_nMinWidth = m_layoutItems.at(idx).item->minWidth();
        if(m_layoutItems.at(idx).item->maxWidth() > m_nMaxWidth) m_nMaxWidth = m_layoutItems.at(idx).item->maxWidth();
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
int XVBoxLayout::minWidth()  const
{
    return m_nMinWidth + marginLeft() + marginRight();
}

int XVBoxLayout::minHeight() const
{
    if(m_visibleCount > 1)
        return m_nMinHeight + marginTop() + marginBottom() + (m_visibleCount - 1) * m_nSpacing;
    else
        return m_nMinHeight + marginTop() + marginBottom();
}

int XVBoxLayout::maxWidth()  const
{
    return m_nMaxWidth + marginLeft() + marginRight();
}

int XVBoxLayout::maxHeight() const
{
    if(m_visibleCount > 1)
        return m_nMaxHeight + marginTop() + marginBottom() + (m_visibleCount - 1) * m_nSpacing;
    else
        return m_nMaxHeight + marginTop() + marginBottom();
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XVBoxLayout::_onItemAdded(IXLayoutItem* item)
{
    // update policy
    updateResizePolicies();
}

void XVBoxLayout::_onItemRemoved(IXLayoutItem* item)
{
    // update policy
    updateResizePolicies();
}

/////////////////////////////////////////////////////////////////////
// layout methods
/////////////////////////////////////////////////////////////////////
void XVBoxLayout::_updateVisibleCount()
{
    // count visible items
    m_visibleCount = 0;
    for(size_t idx = 0; idx < m_layoutItems.size(); ++idx)
    {
        if(m_layoutItems.at(idx).item->isVisible()) m_visibleCount++;
    }
}

void XVBoxLayout::_updateHorizontalLayout(int posX, int width)
{
    // count available size
    int widthLeft = width - (marginLeft() + marginRight());

    for(size_t idx = 0; idx < m_layoutItems.size(); ++idx)
    {
        // ignore not visible items
        if(!m_layoutItems[idx].item->isVisible()) continue;

        // get item resize constraints
        IXLayoutItem::TResizePolicy policy = m_layoutItems[idx].item->horizontalPolicy();
        int minWidth = m_layoutItems[idx].item->minWidth();
        int maxWidth = m_layoutItems[idx].item->maxWidth();

        // check if item can use full width
        if( (policy == IXLayoutItem::eResizeAny) ||
            (policy == IXLayoutItem::eResizeMin && minWidth <= widthLeft) ||
            (policy == IXLayoutItem::eResizeMax && maxWidth >= widthLeft) ||
            (policy == IXLayoutItem::eResizeMinMax && minWidth <= widthLeft && maxWidth >= widthLeft)
            )
        {
            m_layoutItems[idx].width = widthLeft;

        } else if( (policy == IXLayoutItem::eResizeMin || 
            policy == IXLayoutItem::eResizeMinMax) && minWidth > widthLeft)
        {
            m_layoutItems[idx].width = minWidth;

        } else if( (policy == IXLayoutItem::eResizeMax ||
            policy == IXLayoutItem::eResizeMinMax) && maxWidth < widthLeft)
        {
            m_layoutItems[idx].width = maxWidth;
        }

        // check if item needs to be aligned
        if(m_layoutItems[idx].width == widthLeft || m_layoutItems[idx].alignment == eAlignLeft)
        {
            m_layoutItems[idx].posX = posX + marginLeft();

        } else if(m_layoutItems[idx].alignment == eAlignRight)
        {
            m_layoutItems[idx].posX = posX + marginLeft() + (widthLeft - m_layoutItems[idx].width);

        } else if(m_layoutItems[idx].alignment == eAlignCenter)
        {
            m_layoutItems[idx].posX = posX + marginLeft() + ((widthLeft - m_layoutItems[idx].width) / 2);
        }
    }
}

void XVBoxLayout::_initLayoutCache(int& fixedSize, int& freeItems, int& totalStretch)
{
    // loop over all items
    for(size_t idx = 0; idx < m_layoutItems.size(); ++idx)
    {
        // ignore not visible items
        if(!m_layoutItems[idx].item->isVisible()) continue;

        // get item
        IXLayoutItem* item = m_layoutItems[idx].item;

        // reset initial height
        m_layoutItems[idx].height = 0;

        // check if item has fixed height
        if(item->verticalPolicy() != IXLayoutItem::eResizeAny &&
           item->minHeight() == item->maxHeight())
        {
            // set needed height
            m_layoutItems[idx].height = item->minHeight();

            // item is ready (can't resize)
            m_layoutItems[idx].ready = true;

            // decrease counter
            --freeItems;

            // update reserved space
            fixedSize += m_layoutItems[idx].height;

        } else 
        {
            // reset height with minimum size
            m_layoutItems[idx].height = item->minHeight();

            // reset flag
            m_layoutItems[idx].ready = false;

            // update stretch factor if any
            totalStretch += m_layoutItems[idx].stretch;
        }
    }
}

void XVBoxLayout::_findItemSize(int& sizeLeft, int& itemsLeft, int& stretchItems, bool preserveMinSize)
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
                if((item->verticalPolicy() == IXLayoutItem::eResizeMin ||
                    item->verticalPolicy() == IXLayoutItem::eResizeMinMax) &&
                    proposedSize < item->minHeight())
                {
                    // set item size
                    m_layoutItems[idx].height = item->minHeight();
                    sizeFound = false;
                }

            } else 
            {
                if((item->verticalPolicy() == IXLayoutItem::eResizeMax ||
                    item->verticalPolicy() == IXLayoutItem::eResizeMinMax) &&
                    proposedSize > item->maxHeight())
                {
                    // set item size
                    m_layoutItems[idx].height = item->maxHeight();
                    sizeFound = false;
                }
            }

            // check if search has to stop
            if(!sizeFound)
            {
                // fix item size
                m_layoutItems[idx].ready = true;

                // reduce size
                sizeLeft -= m_layoutItems[idx].height;

                // decrease total strecth factor if any
                stretchItems -= m_layoutItems[idx].stretch;

                // size has to be iterated again
                break;
            }
        }
    }
}

void XVBoxLayout::_updateVerticalLayout(int posY, int height)
{
    // total margins
    int marginY = marginTop() + marginBottom();

    // count spacing needed for all items
    int spacing = m_visibleCount > 0 ? (m_visibleCount - 1) * m_nSpacing : 0;

    // init counters
    int fixedSize = marginY + spacing;
    int itemsLeft = m_visibleCount;
    int stretchItems = 0;

    // init layout cache
    _initLayoutCache(fixedSize, itemsLeft, stretchItems);

    // size left
    int sizeLeft = height - fixedSize;

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
            m_layoutItems[idx].height = (int) (m_layoutItems[idx].stretch * stretchFactor);
        else
            m_layoutItems[idx].height = proposedSize;

        // update size
        sizeLeft -= m_layoutItems[idx].height;
    }

    // distribute rounding errors if any
    for(int loopIdx = 0; loopIdx < (int)m_layoutItems.size() && sizeLeft > 0; ++loopIdx)
    {
        // ignore not visible items
        if(!m_layoutItems[loopIdx].item->isVisible()) continue;

        // add one pixel to each item
        if(!m_layoutItems[loopIdx].ready) m_layoutItems[loopIdx].height++;

        // reduce size
        --sizeLeft;
    }
}

// XVBoxLayout
/////////////////////////////////////////////////////////////////////
