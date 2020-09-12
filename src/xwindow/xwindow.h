// Layout based custom windows
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWINDOW_H_
#define _XWINDOW_H_

/////////////////////////////////////////////////////////////////////
// includes
#include "xhwnd.h"

/////////////////////////////////////////////////////////////////////
// forward declarations
class XPopupMenu;
class XToolTip;

/////////////////////////////////////////////////////////////////////
// XWindow - window functionality

class XWindow : public XWHWND
{
public: // construction/destruction
    XWindow(XWObject* parent = 0);
    XWindow(DWORD dwStyle, XWObject* parent = 0, HWND hWndParent = 0, DWORD dwExStyle = 0);
    virtual ~XWindow();

public: // window creation
    bool    create(DWORD dwStyle, HWND hWndParent = 0, DWORD dwExStyle = 0);
    void    destroy();

public: // layout
    void    setLayout(IXLayout* pLayout);

public: // background
    void    enableEraseBackground(bool bEnable);
    void    setBackgroundColor(COLORREF color);

public: // content 
    void    handleContentChanged();

public: // request event
    void    postEventRequest(DWORD dwEventId);

public: // scrolling (from IXWScrollable)
    int     contentWidth();
    int     contentHeight();

public: // visibility (from XWHWND)
    void    show(int nCmdShow = SW_SHOW);

public: // context menu
    void    setContextMenu(XPopupMenu* contextMenu);
    void    enableContextMenu(bool enable);
    XPopupMenu* contextMenu();

public: // tooltip
    void    showToolTip(const wchar_t* text);
    void    hideToolTip();

public: // resize policy (from XWHWND)
    TResizePolicy   horizontalPolicy() const;
    TResizePolicy   verticalPolicy() const;
    void            updateResizePolicies();

public: // size constraints (from XWHWND)
    int     minWidth()  const;
    int     minHeight() const;
    int     maxWidth()  const;
    int     maxHeight() const;

    // NOTE: message event handlers should set message as processed if they do not want
    // this message to processed later. If message will not be marked as processed it
    // will be delivered later to processMessage().
public: // add message events
    void    connect(const XWEventMask& mask, const XWEventDelegate& handler);
    void    connect(const UINT uMsg, const XWEventDelegate& handler);

protected: // common events
    virtual void    onCreate(HWND hwnd, const CREATESTRUCT* pCreateStruct);
    virtual void    onPaint();
    virtual void    onPaint(HDC hdc, PAINTSTRUCT& ps);
    virtual void    onMove(int posX, int posY);
    virtual void    onResize(int type, int width, int height);
    virtual void    onDestroy();
    virtual void    onContentChanged();
    virtual bool    onTimer(WPARAM uIDEvent);
     
protected: // mouse events
    virtual void    onMouseEnter(int posX, int posY);
    virtual void    onMouseMove(int posX, int posY, WPARAM flags);
    virtual void    onMouseHover(int posX, int posY);
    virtual bool    onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags);
    virtual bool    onMouseCaptureChanged();
    virtual void    onMouseLeave();
    virtual bool    onMouseWheel(int wheelDelta, int posX, int posY, WPARAM flags);
    virtual bool    onSetCursor();

protected: // focus events
    virtual void    onSetFocus();
    virtual void    onKillFocus();

protected: // keyboard events (sent only to focused item)
    virtual bool    onCharEvent(WPARAM charCode, LPARAM flags);

protected: // context menu
    virtual void    onContextMenuItem(UINT itemId);
    virtual bool    onContextMenu(HWND hwndContext, int posX, int posY);
    virtual void    doShowContextMenu(XPopupMenu* contextMenu, int posX, int posY);

protected: // timers
    bool    startTimer(WPARAM uIDEvent, UINT uElapseMs);
    bool    stopTimer(WPARAM uIDEvent);

protected: // mark message as processed
    void    setMessageProcessed();
    bool    isMessageProcessed();

protected: // process messages
    virtual LRESULT processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
protected: // background
    virtual void    eraseBackground(HDC hdc, RECT& rect, COLORREF color);

private: // worker methods
    void    _closeResources();
    void    _updateSystemsMargins();
    void    _onWindowResized(int width, int height);
    void    _onMinMaxInfoRequested(WPARAM wParam, LPARAM lParam);
    void    _startMouseTracking();
    LRESULT _doProcessMessageHandlers(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private: // window class
    static bool     sRegisterXWindowClass();

private: // window procedure
    static LRESULT CALLBACK sXWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private: // protect from copy and assignment
    XWindow(const XWindow& ref)  {}
    const XWindow& operator=(const XWindow& ref) { return *this;}

private: // system margins
    int         m_nSysMarginX;
    int         m_nSysMarginY;

private: // controls
    XPopupMenu* m_contextMenu;
    XToolTip*   m_toolTip;
    bool        m_showContextMenu;
    bool        m_contextMenuActive;

private: // data
    IXLayout*   m_pLayout;
    COLORREF    m_bgColor;
    bool        m_bEraseBackground;
    bool        m_bMessageHandled;
    XWEventMap  m_xEventMap;
    bool        m_bMouseOver;
    bool        m_bMouseTracking;
    int         m_mouseHoverX;
    int         m_mouseHoverY;
};

// XWindow
/////////////////////////////////////////////////////////////////////

#endif // _XWINDOW_H_

