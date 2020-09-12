// Scrollable interface
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWSCROLLABLE_H_
#define _XWSCROLLABLE_H_

// NOTE: some items are more efficient to scroll their content (e.g. list view will 
//       show only visible items, text editor may not need to render text outside of 
//       scroll area etc.), while some items do not have such functionality and so 
//       they need to be scrolled by scrollview itself (e.g. move window and hide 
//       not visible parts). In later case contentSize functions should return total
//       size and scrollview will use scrollOffset to position item.

/////////////////////////////////////////////////////////////////////
// IXWScrollable - scrollable interface

class IXWScrollable
{
public: // construction/destruction
    IXWScrollable();
    virtual ~IXWScrollable();

public: // properties
    virtual bool    canScrollContent();

public: // content size
    virtual int     contentWidth() = 0;
    virtual int     contentHeight() = 0;

public: // offset
    virtual int     scrollOffsetX();
    virtual int     scrollOffsetY();

public: // interface
    virtual void    setScrollOffsetX(int scrollOffsetX);
    virtual void    setScrollOffsetY(int scrollOffsetY);
    virtual int     scrollOffsetForWheel(int wheelDelta);

protected: // data
    int     m_scrollOffsetX;
    int     m_scrollOffsetY;
};

// IXWScrollable
/////////////////////////////////////////////////////////////////////

#endif // _XWSCROLLABLE_H_

