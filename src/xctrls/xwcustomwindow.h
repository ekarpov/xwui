// Custom window base implementation
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWCUSTOMWINDOW_H_
#define _XWCUSTOMWINDOW_H_

/////////////////////////////////////////////////////////////////////
// XWCustomWindow - custom window helpers

class XWCustomWindow : public XWindow
{
public: // construction/destruction
    XWCustomWindow(XWObject* parent = 0);
    XWCustomWindow(DWORD dwStyle, XWObject* parent = 0, HWND hWndParent = 0, DWORD dwExStyle = 0);
    ~XWCustomWindow();

public: // moving (window will respond to WM_NCHITTEST)
    void    enableMoving(bool enable);
    void    setMovingHitArea(const RECT& area);

public: // resize (window will respond to WM_NCHITTEST)
    void    enableResize(bool enable);
    void    setBorderSize(int size);

protected: // process messages
    LRESULT processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private: // worker methods
    bool    _isBorderPoint(int posX, int posY, const RECT& windowRect);

private: // data
    bool        m_bMovingEnabled;
    bool        m_bResizeEnabled;
    RECT        m_rcMovingHitArea;
    int         m_nBorderSize;
};

// XWCustomWindow
/////////////////////////////////////////////////////////////////////

#endif // _XWCUSTOMWINDOW_H_

