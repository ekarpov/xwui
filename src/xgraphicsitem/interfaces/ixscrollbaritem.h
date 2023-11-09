// Scroll bar interface
//
/////////////////////////////////////////////////////////////////////

#ifndef _IXSCROLLBARITEM_H_
#define _IXSCROLLBARITEM_H_

/////////////////////////////////////////////////////////////////////
// IXScollBarItem - scroll bar interface

class IXScollBarItem : public XGraphicsItem
{
public: // construction/destruction
    IXScollBarItem(XGraphicsItem* parent = 0) : XGraphicsItem(parent)  {}
    virtual ~IXScollBarItem() {}

public: // interface
    virtual void    setScrollBarStyle(const XWUIStyle::XScrollBarStyle& style) = 0;
    virtual void    setScrollItem(IXWScrollable* pScrollItem) = 0;
    virtual void    scrollContent(int scrollLen) = 0;
    virtual int     getMaxScrollOffset() = 0;
};

// IXScollBarItem
/////////////////////////////////////////////////////////////////////

#endif // _IXSCROLLBARITEM_H_

