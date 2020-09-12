// ScrollView window
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWSCROLLVIEWWINDOW_H_
#define _XWSCROLLVIEWWINDOW_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
class XWScrollBarWindow;

/////////////////////////////////////////////////////////////////////
// XWScrollViewWindow - scroll view

class XWScrollViewWindow : public XWindow,
                           public XWScrollViewLogic
{
public: // construction/destruction
    XWScrollViewWindow(DWORD dwStyle, XWObject* parent = 0, HWND hWndParent = 0, DWORD dwExStyle = 0);
    ~XWScrollViewWindow();

public: // scrollable window (view doesn't take ownership)
    void    setScrollWindow(XWHWND* scrollWindow);
    void    scrollContent(int scrollX, int scrollY);

public: // release window from view
    void    releaseScrollWindow();

public: // manipulations (from IXLayoutItem)
    void    update(int posX, int posY, int width, int height);

public: // scrolling (from XWHWND)
    bool    canScrollContent();
    int     contentWidth();
    int     contentHeight();
    int     scrollOffsetX();
    int     scrollOffsetY();
    void    setScrollOffsetX(int scrollOffsetX);
    void    setScrollOffsetY(int scrollOffsetY);
    int     scrollOffsetForWheel(int wheelDelta);

protected: // common events (from XWindow)
    void    onContentChanged();

protected: // mouse events (from XWindow)
    void    onMouseEnter(int posX, int posY);
    void    onMouseMove(int posX, int posY, WPARAM flags);
    bool    onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags);
    void    onMouseLeave();
    bool    onMouseWheel(int wheelDelta, int posX, int posY, WPARAM flags);

protected: // keyboard events (from XWindow)
    bool    onCharEvent(WPARAM charCode, LPARAM flags);

protected: // process messages
    LRESULT processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private: // scroll logic methods (from XWScrollViewLogic)
    void    updateScrollItem(int posX, int posY, int width, int height);
    void    updateScrollBar(TScrollOrientation scrollOrient, int posX, int posY, int width, int height);
    void    showScrollBar(TScrollOrientation scrollOrient, bool bShow);
    bool    isScrollBarVisible(TScrollOrientation scrollOrient);
    int     scrollBarWidth(TScrollOrientation scrollOrient);

private: // events
    void    onConnectedObjectRemoved(XWObject* child);

private: // worker methods
    void    _doScrollItem(int scrollOffsetX, int scrollOffsetY);

private: // data
    XWHWND*                 m_scrollWindow;
    XWScrollBarWindow*      m_verticalScrollBar;
    XWScrollBarWindow*      m_horizontalScrollBar;
};

// XWScrollViewWindow
/////////////////////////////////////////////////////////////////////

#endif // _XWSCROLLVIEWWINDOW_H_

