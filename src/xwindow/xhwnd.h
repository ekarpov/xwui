// Window handle wrapper with layout support
//
/////////////////////////////////////////////////////////////////////

#ifndef _XHWNDLAYOUTITEM_H_
#define _XHWNDLAYOUTITEM_H_

/////////////////////////////////////////////////////////////////////
// XWHWND - layout item for Windows handle (HWND)

class XWHWND : public IXLayoutItem,
               public IXWScrollable,
               public XWObject
{
public: // construction/destruction
    XWHWND(HWND hWnd = 0, XWObject* parent = 0);
    virtual ~XWHWND();

public: // parent window
    void    setParent(HWND hWnd);

public: // visibility
    virtual void    show(int nCmdShow = SW_SHOW);
    virtual void    hide();
    bool            isVisible() const;

public: // focus
    virtual void    setFocusable(bool bFocusable);
    virtual bool    isFocusable() const;
    virtual void    setFocus();
    bool            hasFocus() const;

public: // window text
    void    setText(const wchar_t* text);
    int     getText(wchar_t* text, int maxSize) const;
    int     getTextLength() const;

public: // fonts
    void    setDefaultFont();
    void    setFont(HFONT hFont, BOOL bRepaint = FALSE);
    HFONT   getFont();

public: // style
    LONG    style() const;
    bool    setStyle(LONG lStyle);

public: // WM_COMMAND based notifications
    XWEventMask     mkCommandEvent(WORD wNotifyCode) const;
    XWEventMask     mkMenuEvent(UINT itemId) const;
    void            notifyParentWindow(WORD wNotifyCode, WORD wControlId = 0);
    void            notifyParentMenuItem(UINT itemId);

public: // set size constraints
    void    setMinWidth(int width);
    void    setMinHeight(int height);
    void    setMaxWidth(int width);
    void    setMaxHeight(int height);

public: // fix size
    void    setFixedWidth(int width);
    void    setFixedHeight(int height);
    void    setFixedSize(int width, int height);

public: // reset size policy
    void    resetSizePolicy();

public: // current size 
    void    getSize(int& width, int& height);
    int     width();
    int     height();

public: // window position (in screen coordinates)
    void    getRect(RECT& rect);
    void    getPos(LONG& posX, LONG& posY);

public: // window position relative to other window
    void    getRelativeRect(HWND hwnd, RECT& rect);
    void    getRelativePos(HWND hwnd, LONG& posX, LONG& posY);

public: // client size 
    void    getClientSize(int& width, int& height);
    int     clientWidth();
    int     clientHeight();

public: // state
    virtual void    enable(BOOL bEnable);
    virtual bool    isEnabled();

public: // window methods
    void    resize(int width, int height);
    void    move(int posX, int posY);
    void    repaint(BOOL paintNow = FALSE);

public: // scrolling (from IXWScrollable)
    virtual int     contentWidth();
    virtual int     contentHeight();
    virtual void    setScrollOffsetX(int scrollOffsetX);
    virtual void    setScrollOffsetY(int scrollOffsetY);

public: // properties (from IXLayoutItem)
    unsigned long   layoutItemId() const    { return xwoid(); }

public: // manipulations (from IXLayoutItem)
    void    update(int posX, int posY, int width, int height);

public: // resize policy (from IXLayoutItem)
    TResizePolicy   horizontalPolicy()  const;
    TResizePolicy   verticalPolicy()  const;

public: // size constraints (from IXLayoutItem)
    int     minWidth()   const;
    int     minHeight()  const;
    int     maxWidth()   const;
    int     maxHeight()  const;

public: // window handle
    HWND    hwnd()  const { return m_hWnd; }
    operator HWND() const { return m_hWnd; }

protected: // subclassing
    bool    isSubclassed() const;
    bool    subclassWindow();
    bool    removeSubclass();

protected: // process messages (in case window has been subclassed)
    virtual LRESULT processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private: // subclass window procedure
    static LRESULT CALLBACK sXSubclassWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

private: // content callbacks
    static BOOL CALLBACK _contentWidthCallbackProc(HWND hwnd, LPARAM lParam);
    static BOOL CALLBACK _contentHeightCallbackProc(HWND hwnd, LPARAM lParam);

protected: // window handle
    HWND        m_hWnd;

private: // policy
    TResizePolicy   m_rpHorizontal;
    TResizePolicy   m_rpVertical;

private: // data
    bool    m_isSubclassed;
    int     m_nMinWidth;
    int     m_nMinHeight;
    int     m_nMaxWidth;
    int     m_nMaxHeight;
    bool    m_bFocusable;
    int     m_contentStart;
    int     m_contentEnd;
};

// XWHWND
/////////////////////////////////////////////////////////////////////

#endif // _XHWNDLAYOUTITEM_H_

