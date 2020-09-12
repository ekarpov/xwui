// Scroll bar grapchics item implementation
//
/////////////////////////////////////////////////////////////////////

#ifndef _XSCROLLBARITEM_H_
#define _XSCROLLBARITEM_H_

/////////////////////////////////////////////////////////////////////
// scrollbar orientaiton
enum TXScrollBarOrientation
{
    eXScrollBarVertical,
    eXScrollBarHorizontal,
};

/////////////////////////////////////////////////////////////////////
// XScrollBarItem - scroll bar implementation

class XScrollBarItem : public IXScollBarItem
{
public: // construction/destruction
    XScrollBarItem(TXScrollBarOrientation orientation, XGraphicsItem* parent = 0);
    ~XScrollBarItem();

public: // orientaiton
    TXScrollBarOrientation orientaiton() const { return m_orientaiton; };

public: // style
    void    setStyle(const XWUIStyle::XScrollBarStyle& style);
    const XWUIStyle::XScrollBarStyle& getStyle() const; 

public: // scrolling interface
    bool    isScrollingActive() const { return m_mouseMoveActive; }
    void    onScrollContentChanged();

public: // interface (from IXScollBarItem)
    void    setScrollBarStyle(const XWUIStyle::XScrollBarStyle& style);
    void    setScrollItem(IXWScrollable* scrollItem);
    void    scrollContent(int scrollLen);
    int     getMaxScrollOffset();

public: // manipulations (from XGraphicsItem)
    void    update(int posX, int posY, int width, int height);

public: // focus (from XGraphicsItem)
    bool    isFocusable() const;

public: // mouse events (from XGraphicsItem)
    void    onMouseEnter(int posX, int posY);
    void    onMouseMove(int posX, int posY, WPARAM flags);
    bool    onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags);
    void    onMouseCaptureReset();
    void    onMouseLeave();

public: // keyboard events (from XGraphicsItem)
    bool    onCharEvent(WPARAM charCode, LPARAM flags);

public: // GDI painting
    void    onPaintGDI(HDC hdc, const RECT& rcPaint);   

public: // Direct2D painting
    void    onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint); 

private: // helper methods
    bool    _moveVerticalScrollBar(int mousePosX, int mousePosY);
    bool    _moveHorizontalScrollBar(int mousePosX, int mousePosY);
    int     _scrollBarLength();
    void    _updateScrollProperties();

protected: //  scrollbar properties
    int         m_scrollSliderSize;
    int         m_scrollSliderOffset;
    int         m_scrollItemSize;
    int         m_scrollContentSize;
    float       m_scrollStep;
    bool        m_hasScrollContent;

protected: //  properties
    bool        m_mouseMoveActive;
    int         m_mousePosX;
    int         m_mousePosY;
    int         m_scrollPos;

protected: //  style
    XWUIStyle::XScrollBarStyle  m_scrollBarStyle;

protected: //  data
    IXWScrollable*          m_scrollItem;
    TXScrollBarOrientation  m_orientaiton;
};

// XScrollBarItem
/////////////////////////////////////////////////////////////////////

#endif // _XSCROLLBARITEM_H_

