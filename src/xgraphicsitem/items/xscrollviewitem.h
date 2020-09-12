// Scroll view grapchics item implementation
//
/////////////////////////////////////////////////////////////////////

#ifndef _XSCROLLVIEWITEM_H_
#define _XSCROLLVIEWITEM_H_

/////////////////////////////////////////////////////////////////////
// XScrollViewItem - scroll view item implementation

class XScrollViewItem : public XGraphicsItem,
                                public XWScrollViewLogic
{
public: // construction/destruction
    XScrollViewItem(XGraphicsItem* parent = 0);
    ~XScrollViewItem();

public: // scrollbars
    IXScollBarItem*     verticalScrollBar()     { return m_verticalScrollBar; };
    IXScollBarItem*     horizontalScrollBar()   { return m_horizontalScrollBar; };

public: // style
    void    setStyle(const XWUIStyle::XScrollViewStyle& style);
    const XWUIStyle::XScrollViewStyle& getStyle() const; 

public: // scrollbar style
    void    setScrollbarStyle(const XWUIStyle::XScrollBarStyle& style);
    const XWUIStyle::XScrollBarStyle& getScrollbarStyle() const; 

public: // scrollable item (view takes ownership)
    void    setScrollItem(XGraphicsItem* scrollItem);

public: // scrolling 
    void    scrollContent(int scrollX, int scrollY);
    void    scrollContentTop();
    void    scrollContentBottom();
    void    scrollContentLeft();
    void    scrollContentRight();

public: // scrolling (from XGraphicsItem)
    bool    canScrollContent();
    int     contentWidth();
    int     contentHeight();
    int     scrollOffsetX();
    int     scrollOffsetY();
    void    setScrollOffsetX(int scrollOffsetX);
    void    setScrollOffsetY(int scrollOffsetY);
    int     scrollOffsetForWheel(int wheelDelta);

public: // customize scrollbars (view takes ownership)
    void    setVerticalScrollBar(IXScollBarItem* pIXScollBarItem);
    void    setHorizontalScrollBar(IXScollBarItem* pIXScollBarItem);

public: // manipulations (from XGraphicsItem)
    void    update(int posX, int posY, int width, int height);

public: // focus (from XGraphicsItem)
    bool    isFocusable() const;

public: // mouse events (from XGraphicsItem)
    void    onMouseEnter(int posX, int posY);
    void    onMouseMove(int posX, int posY, WPARAM flags);
    bool    onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags);
    void    onMouseLeave();
    bool    onMouseWheel(int wheelDelta, int posX, int posY, WPARAM flags);

public: // keyboard events (from XGraphicsItem)
    bool    onCharEvent(WPARAM charCode, LPARAM flags);

public: // GDI painting
    void    onPaintGDI(HDC hdc, const RECT& rcPaint);   

public: // Direct2D painting
    void    onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint); 

public: // message processing
    LRESULT processWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& messageProcessed);

private: // scroll logic methods (from XWScrollViewLogic)
    void    updateScrollItem(int posX, int posY, int width, int height);
    void    updateScrollBar(TScrollOrientation scrollOrient, int posX, int posY, int width, int height);
    void    showScrollBar(TScrollOrientation scrollOrient, bool bShow);
    bool    isScrollBarVisible(TScrollOrientation scrollOrient);
    int     scrollBarWidth(TScrollOrientation scrollOrient);

private: // events
    void    onChildObjectRemoved(XWObject* child);

private: // worker methods
    void    _doScrollItem(int scrollOffsetX, int scrollOffsetY);

private: // data
    XGraphicsItem*      m_scrollGraphicsItem;
    IXScollBarItem*     m_verticalScrollBar;
    IXScollBarItem*     m_horizontalScrollBar;

private: // style
    XWUIStyle::XScrollViewStyle     m_viewStyle;
    XWUIStyle::XScrollBarStyle      m_scrollbarStyle;
};

// XScrollViewItem
/////////////////////////////////////////////////////////////////////

#endif // _XSCROLLVIEWITEM_H_

