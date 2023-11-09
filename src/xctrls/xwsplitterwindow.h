// Splitter window
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWSPLITTERWINDOW_H_
#define _XWSPLITTERWINDOW_H_

/////////////////////////////////////////////////////////////////////
// splitter window orientation
enum TXWSplitOrientation
{
    XWUI_SPLIT_VERTICALLY,
    XWUI_SPLIT_HORIZONTALLY
};

/////////////////////////////////////////////////////////////////////
// XWSplitterWindow - splitter window

class XWSplitterWindow : public XWindow
{
public: // construction/destruction
    XWSplitterWindow(DWORD dwStyle, XWObject* parent = 0, HWND hWndParent = 0, DWORD dwExStyle = 0);
    ~XWSplitterWindow();

public: // windows to split (window will take ownership)
    // NOTE: ltWindow stands for left or top window (depending on orientation)
    //       rbWindow is right or bottom window (depending on orientation)
    void    setSplitWindows(TXWSplitOrientation orientation, XWHWND* ltWindow, XWHWND* rbWindow);

public: // interface
    void    setSplitterWidth(int width);
    void    setSplitterFillColor(const COLORREF& color);
    void    moveSplitter(int pos);

protected: // splitter painting
    virtual void    onPaintSplitter(HDC hdc, const RECT& paintRect);

protected: // window events (from XWindow)
    void    onResize(int type, int width, int height);
    void    onPaint(HDC hdc, PAINTSTRUCT& ps);

protected: // mouse events (from XWindow)
    void    onMouseEnter(int posX, int posY);
    void    onMouseMove(int posX, int posY, WPARAM flags);
    bool    onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags);
    bool    onMouseCaptureChanged();
    void    onMouseLeave();
    bool    onSetCursor();

private: // events
    void    onConnectedObjectRemoved(XWObject* child);

private: // worker methods
    void    _updateLayout();
    void    _updateRatio();
    bool    _validatePosition(int splitterPos, int& validPosition);
    bool    _isOverSplitter(int posX, int posY);
    void    _setSplitterPos(int pos);
    void    _setResizeCursor();
    void    _resetCursor();
    void    _getSplitterRect(RECT& rcSplitter);
    void    _doPaintSplitter();

private: // data
    TXWSplitOrientation m_orientation;
    COLORREF            m_fillColor;
    XWHWND*             m_ltWindow;
    XWHWND*             m_rbWindow;
    HCURSOR             m_originalCursor;
    double              m_splitterRatio;
    int                 m_splitterPos;
    int                 m_splitterWidth;
    bool                m_mouseMoveActive;
    int                 m_mousePosX;
    int                 m_mousePosY;
};

// XWSplitterWindow
/////////////////////////////////////////////////////////////////////

#endif // _XWSPLITTERWINDOW_H_

