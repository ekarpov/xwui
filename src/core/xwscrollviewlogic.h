// Scroll View logic implementation
//
/////////////////////////////////////////////////////////////////////

#ifndef _SCROLLVIEWLOGIC_H_
#define _SCROLLVIEWLOGIC_H_

/////////////////////////////////////////////////////////////////////
// XWScrollViewLogic - common scroll view logic

class XWScrollViewLogic
{
public: // construction/destruction
    XWScrollViewLogic();
    virtual ~XWScrollViewLogic();

public: // scrollbars
    enum TScrollOrientation
    {
        eVerticalScrollBar,
        eHorizontalScrollBar
    };

    enum TShowScrollbar
    {
        eScrollBarShowAuto,
        eScrollBarShowAlways,
        eScrollBarShowNever
    };

    void    setShowScrollBar(TScrollOrientation scrollOrient, TShowScrollbar showScrollbar);
    void    setShowVerticalScrollBar(TShowScrollbar showScrollbar);
    void    setShowHorizontalScrollBar(TShowScrollbar showScrollbar);

public: // properties
    void    enableMouseDragScrolling(bool bEnable);
    void    enableMouseWheelScrolling(bool bEnable);
    void    enableKeyScrolling(bool bEnable);

protected: // worker interface (called by derived view)
    void    setScrollableItem(IXWScrollable* scrollItem);
    void    updateScrollView(int posX, int posY, int width, int height);
    void    scrollMouseWheel(int wheelDelta, int posX, int posY);

protected: // worker methods (implemented in derived view)
    virtual void    updateScrollItem(int posX, int posY, int width, int height) = 0;
    virtual void    updateScrollBar(TScrollOrientation scrollOrient, int posX, int posY, int width, int height) = 0;
    virtual void    showScrollBar(TScrollOrientation scrollOrient, bool bShow) = 0;
    virtual bool    isScrollBarVisible(TScrollOrientation scrollOrient) = 0;
    virtual int     scrollBarWidth(TScrollOrientation scrollOrient) = 0;

protected: // data
    IXWScrollable*      m_scrollItem;

protected: // properties
    TShowScrollbar      m_showVerticalScrollBar;
    TShowScrollbar      m_showHorizontalScrollBar;
    bool                m_bMouseDragScrollEnabled;
    bool                m_bMouseWheelScrollEnabled;
    bool                m_bKeyScrollEnabled;
};

// XWScrollViewLogic
/////////////////////////////////////////////////////////////////////

#endif // _SCROLLVIEWLOGIC_H_

