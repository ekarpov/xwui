// Window to render graphic items
//
/////////////////////////////////////////////////////////////////////

#ifndef _XGRAPHICSITEMWINDOW_H_
#define _XGRAPHICSITEMWINDOW_H_

/////////////////////////////////////////////////////////////////////
// XGraphicsItemWindow - graphic item rendering

class XGraphicsItemWindow : public XWindow
{
public: // construction/destruction
    XGraphicsItemWindow(XWObject* parent = 0);
    XGraphicsItemWindow(DWORD dwStyle, XWObject* parent = 0, HWND hWndParent = 0, DWORD dwExStyle = 0);
    ~XGraphicsItemWindow();

public: // graphics item
    void            setGraphicsItem(XGraphicsItem* pXGraphicsItem);
    XGraphicsItem*  releaseGraphicsItem();

public: // options
    void    enableGDIDoubleBuffering(bool bEnable);
    void    forceGDIRendering(bool bForce);
    void    enableContentScrolling(bool bEnable);

public: // shared caches
    void    setSharedGDICache(XGdiResourcesCache* pSharedGDICache);
    int     getSharedGDICache(XGdiResourcesCache** pSharedGDICache);

public: // state (from XWHWND)
    void    enable(BOOL bEnable);
    bool    isEnabled();

public: // scrolling (from XWHWND)
    bool    canScrollContent();
    int     contentWidth();
    int     contentHeight();
    int     scrollOffsetX();
    int     scrollOffsetY();
    void    setScrollOffsetX(int scrollOffsetX);
    void    setScrollOffsetY(int scrollOffsetY);
    int     scrollOffsetForWheel(int wheelDelta);

public: // resize policy (from XWHWND)
    TResizePolicy   horizontalPolicy() const;
    TResizePolicy   verticalPolicy() const;

public: // size constraints (from XWHWND)
    int     minWidth()  const;
    int     minHeight() const;
    int     maxWidth()  const;
    int     maxHeight() const;

protected: // events
    void    onCreate(HWND hwnd, const CREATESTRUCT* pCreateStruct);
    void    onResize(int type, int width, int height);
    void    onDestroy();
    bool    onTimer(WPARAM uIDEvent);

protected: // mouse events
    void    onMouseEnter(int posX, int posY);
    void    onMouseMove(int posX, int posY, WPARAM flags);
    void    onMouseHover(int posX, int posY);
    bool    onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags);
    void    onMouseLeave();
    bool    onMouseWheel(int wheelDelta, int posX, int posY, WPARAM flags);
    void    onMouseCaptureReset();
    bool    onSetCursor();

protected: // keyboard events (sent only to focused item)
    bool    onCharEvent(WPARAM charCode, LPARAM flags);

protected: // context menu
    void    onContextMenuItem(UINT itemId);
    bool    onContextMenu(HWND hwndContext, int posX, int posY);

protected: // message processing
    LRESULT processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private: // paint events (from XWindow)
    void    onPaint(HDC hdc, PAINTSTRUCT& ps);

private: // hide background settings (from XWindow)
    void    enableEraseBackground(bool bEnable);
    void    setBackgroundColor(COLORREF color);

private: // hide layout setting (from XWindow)
    void    setLayout(IXLayout* pLayout);

private: // events
    void    onChildObjectRemoved(XWObject* child);

private: // paint methods
    void    _onPaintWindowGDI(HDC hdc, PAINTSTRUCT& ps);
    void    _initGDICache(HDC hdc);
    void    _resetGDICache();
    void    _onPaintWindowD2D(const RECT& rcPaint);
    bool    _initD2DTarget();
    void    _resetD2DTarget();
    void    _setSharedGDICacheToChildren();
    void    _getSharedGDICacheFromParent();

private: // worker methods
    void    _closeResources();
    void    _initGraphicsItem(XGraphicsItem* pXGraphicsItem, HWND hwnd);
    void    _updateGraphicsItem();
    void    _updateGraphicsItem(int width, int height);
    void    _closeGraphicsItem(XGraphicsItem* pXGraphicsItem);
    bool    _onItemEvent(DWORD itemId, LPARAM param);
    void    _onAnimationTimerEvent(DWORD id);
    void    _onAnimationValueEvent(DWORD id);
    void    _onAnimationCompleted(DWORD id);
    void    _onContentLoaded(DWORD id, const WCHAR* path);
    void    _onContentLoadFailed(DWORD id, DWORD reason);
    void    _setMouseCapture(HWND hwnd, XGraphicsItem* pXGraphicsItem);
    void    _resetMouseCapture();
    void    _showContextMenu(WPARAM wItemId, int posX, int posY);
    XGraphicsItem*  _findItemId(WPARAM wItemId);

protected: //  caches
    XD2DResourcesCache*     m_pD2DResourcesCache;
    XGdiResourcesCache*     m_pGDIResourcesCache;
    XGdiResourcesCache*     m_pSharedGDIResourcesCache;

private: //  data
    XGraphicsItem*      m_pXGraphicsItem;
    XGraphicsItem*      m_pMouseCaptureItem;
    XGraphicsItem*      m_pContextMenuItem;
    bool                m_bDirect2DPaint;
    bool                m_bGDIDoubleBuffering;
    bool                m_bContentScrolling;

private: //  Direct2D data
    ID2D1HwndRenderTarget*  m_pRenderTarget;
};

// XGraphicsItemWindow
/////////////////////////////////////////////////////////////////////

#endif // _XGRAPHICSITEMWINDOW_H_


