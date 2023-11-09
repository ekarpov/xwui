// ScrollBar window
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWSCROLLBARWINDOW_H_
#define _XWSCROLLBARWINDOW_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
class XScrollBarItem;

/////////////////////////////////////////////////////////////////////
// XWScrollBarWindow - scroll bar window implementation

class XWScrollBarWindow : public XGraphicsItemWindow
{
public: // construction/destruction
    XWScrollBarWindow(TXScrollBarOrientation orientation, HWND hWndParent, XWObject* parent = 0);
    ~XWScrollBarWindow();

public: // interface
    void    setScrollItem(IXWScrollable* pScrollItem);
    void    scrollContent(int scrollLen);

public: // orientaiton
    TXScrollBarOrientation orientaiton() const;

public: // style
    void    setStyle(const XWUIStyle::XScrollBarStyle& style);
    const XWUIStyle::XScrollBarStyle& getStyle() const; 

public: // scrolling interface
    bool    isScrollingActive() const;
    void    onScrollContentChanged();

private: // hide some interfaces
    void    setGraphicsItem(XGraphicsItem* pXGraphicsItem);
    bool    create(DWORD dwStyle, HWND hWndParent = 0, DWORD dwExStyle = 0);

private: // data
    XScrollBarItem*     m_scrollBarItem;
};

// XWScrollBarWindow
/////////////////////////////////////////////////////////////////////

#endif // _XWSCROLLBARWINDOW_H_

