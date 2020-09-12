// Horizontal boxed layout engine
//
/////////////////////////////////////////////////////////////////////

#ifndef _XHBOXLAYOUT_H_
#define _XHBOXLAYOUT_H_

/////////////////////////////////////////////////////////////////////
// XHBoxLayout - horizontal box layout engine 

class XHBoxLayout : public IXLayout,
                    public XWObject
{
public: // construction/destruction
    XHBoxLayout(XWObject* parent = 0);
    virtual ~XHBoxLayout();

public: // alignment
    enum TAlignment
    {
        eAlignTop,
        eAlignBottom,
        eAlignCenter
    };

public: // add items
    void    addItem(IXLayoutItem* item, int stretch = 0, TAlignment alignment = eAlignCenter);

public: // extra items
    void    addSpaceItem(int size);
    void    addStretchItem(int stretch);

public: // item spacing
    void    setSpacing(int spacing);
    int     spacing() const     { return m_nSpacing; }

public: // enum layout items (from IXLayout)
    int             layoutItemCount() const;
    void            removelayoutItemAt(int idx);
    IXLayoutItem*   layoutItemAt(int idx) const;

public: // layout items by id (from IXLayout)
    void            removelayoutItem(unsigned long itemId);
    IXLayoutItem*   layoutItem(unsigned long itemId) const;

public: // manipulations (from IXLayout)
    void    update(int posX, int posY, int width, int height);

public: // resize policy (from IXLayout)
    TResizePolicy   horizontalPolicy() const;
    TResizePolicy   verticalPolicy() const;
    void            updateResizePolicies();

public: // size constraints (from IXLayout)
    int     minWidth()  const;
    int     minHeight() const;
    int     maxWidth()  const;
    int     maxHeight() const;

private: // worker methods
    void    _onItemAdded(IXLayoutItem* item);
    void    _onItemRemoved(IXLayoutItem* item);
    void    _updateResizePolicy();

private: // layout methods
    void    _updateVisibleCount();
    void    _updateVerticalLayout(int posY, int height);
    void    _initLayoutCache(int& fixedSize, int& freeItems, int& totalStretch);
    void    _findItemSize(int& sizeLeft, int& itemsLeft, int& stretchItems, bool preserveMinSize);
    void    _updateHorizontalLayout(int posX, int width);

private: // item reference
    struct _ItemRef
    {
        // layout data
        IXLayoutItem*   item;
        TAlignment      alignment;
        int             stretch;

        // resizing cache
        int     posX, posY;
        int     width, height;
        bool    ready;
    };

private: // items
    std::vector<_ItemRef>       m_layoutItems;
    std::vector<IXLayoutItem*>  m_spaceItems;
    int                         m_visibleCount;

private: // policy
    TResizePolicy   m_rpHorizontal;
    TResizePolicy   m_rpVertical;

private: // data
    int     m_nSpacing;
    int     m_nMinWidth;
    int     m_nMinHeight;
    int     m_nMaxWidth;
    int     m_nMaxHeight;
};

// XHBoxLayout
/////////////////////////////////////////////////////////////////////

#endif // _XHBOXLAYOUT_H_

