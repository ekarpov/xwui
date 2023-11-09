// List graphics item
//
/////////////////////////////////////////////////////////////////////

#ifndef _XLISTVIEWITEM_H_
#define _XLISTVIEWITEM_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
class XVBoxLayout;

/////////////////////////////////////////////////////////////////////
// XListViewItem - list item

class XListViewItem : public XGraphicsItem
{
public: // construction/destruction
    XListViewItem(XGraphicsItem* parent = 0);
    ~XListViewItem();

public: // items (list takes ownership)
    void    appendListItem(XGraphicsItem* item);
    void    deleteListItem(unsigned long itemId);
    void    deleteAllListItems();
    XGraphicsItem*  findListItem(unsigned long itemId);

public: // item list (NOTE: call handleContentChanged after updating list)
    std::list<XGraphicsItem*>&   items();

public: // list item width constraints
    void    setMaxItemWidth(int width);
    void    setMinItemWidth(int width);

public: // alignment
    enum TAlignment
    {
        eAlignLeft,
        eAlignRight,
        eAlignCenter
    };

    void    setItemAlignment(TAlignment alignment);

public: // content 
    void    handleContentChanged(); 

public: // content margins
    void    setContentMargins(int left, int top, int right, int bottom);
    void    setItemSpacing(int spacing);

public: // manipulations (from IXLayoutItem)
    void    update(int posX, int posY, int width, int height);

public: // scrolling interface (from IXWScrollable)
    bool    canScrollContent();
    int     contentWidth();
    int     contentHeight();
    void    setScrollOffsetX(int scrollOffsetX);
    void    setScrollOffsetY(int scrollOffsetY);

private: // worker methods
    void    _layoutItems(int posX, int posY, int width, int height);
    void    _scrollItems();

private: // data
    TAlignment  m_itemAlignment;
    int         m_maxItemWidth;
    int         m_minItemWidth;
    int         m_itemsPosX;
    int         m_itemsWidth;
    int         m_itemsHeight;

private: // content margins
    int         m_nMarginLeft;
    int         m_nMarginTop;
    int         m_nMarginRight;
    int         m_nMarginBottom;
    int         m_nItemSpacing;
};

// XListViewItem
/////////////////////////////////////////////////////////////////////

#endif // _XLISTVIEWITEM_H_

