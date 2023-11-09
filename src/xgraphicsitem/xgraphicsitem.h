// Graphics item base functionality
//
/////////////////////////////////////////////////////////////////////

#ifndef _XGRAPHICSITEM_H_
#define _XGRAPHICSITEM_H_

/////////////////////////////////////////////////////////////////////
// common includes 
#include "../layout/xlayoutitem.h"
#include "../layout/xlayout.h"

/////////////////////////////////////////////////////////////////////
// forward declarations
struct ID2D1Factory;
struct ID2D1HwndRenderTarget;
struct ID2D1GdiInteropRenderTarget;
class XGraphicsItemLayout;
class XD2DResourcesCache;
class XPopupMenu;

/////////////////////////////////////////////////////////////////////
// graphics item events
enum TXGraphicsItemEvent
{
    XGITEM_EVENT_CLICKED        = 0x0001,
    XGITEM_EVENT_MOUSE_HOVER,
    
    XGITEM_EVENT_COUNT      // must be the last
};

/////////////////////////////////////////////////////////////////////
// XGraphicsItem - graphics item

class XGraphicsItem : public IXLayoutItem,
                      public IXWScrollable,
                      public XWObject
{
public: // construction/destruction
    XGraphicsItem(XGraphicsItem* parent = 0);
    virtual ~XGraphicsItem();

public: // parent window
    virtual void    setParentWindow(HWND hwndParent);
    HWND    parentWindow() const { return m_hwndParent; }

public: // message processing
    bool    processingMessages() const { return m_messageProcessing; }
    virtual LRESULT processWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& messageProcessed);

public: // standard event handlers
    unsigned long   addClickedHandler(const XWObjectEventDelegate& handler);

public: // layout engine (item takes ownership)
    void    setLayout(IXLayout* layout);
    IXLayout*   layout() const { return m_pLayout; }

public: // child items (item takes ownership)
    void    addChildItem(XGraphicsItem* childItem);
    void    deleteChildItem(unsigned long itemId);
    void    deleteAllChildItems();
    XGraphicsItem*  findChildItem(unsigned long itemId);
    XGraphicsItem*  findAnimationItem(DWORD animationId);
    XGraphicsItem*  findContentItem(DWORD contentId);

public: // Z-ordering (item must be child item)
    void    moveItemOnTop(XGraphicsItem* childItem);
    void    moveItemUp(XGraphicsItem* childItem);
    void    moveItemDown(XGraphicsItem* childItem);
    bool    isTopItem(XGraphicsItem* childItem) const;
    bool    isBottomItem(XGraphicsItem* childItem) const;

public: // position
    virtual void    move(int posX, int posY);

public: // manipulations (from IXLayoutItem)
    void    update(int posX, int posY, int width, int height);

public: // mouse cursors
    void    setMouseCursor(HCURSOR cursor);
    void    resetMouseCursor();

public: // mouse capture
    void    setMouseCapture();
    void    resetMouseCapture();

public: // hint text
    void    setHintText(const wchar_t* text);

public: // context menu
    void    setContextMenu(XPopupMenu* contextMenu);
    void    enableContextMenu(bool enable);
    XPopupMenu* contextMenu();

public: // content 
    virtual void    handleContentChanged();
    virtual int     contentWidthForHeight(int height);
    virtual int     contentHeightForWidth(int width);

public: // scrolling interface (from IXWScrollable)
    virtual bool    canScrollContent();
    virtual int     contentWidth();
    virtual int     contentHeight();
    virtual void    setScrollOffsetX(int scrollOffsetX);
    virtual void    setScrollOffsetY(int scrollOffsetY);

public: // properties
    virtual void    setEnabled(bool bEnabled);
    virtual bool    isEnabled() const;
    virtual void    setVisible(bool bVisible);
    virtual void    setObscured(bool bObscured);
    virtual bool    isObscured() const;
    virtual void    setClickable(bool clickable);
    virtual bool    isClickable() const;

public: // properties (from IXLayoutItem)
    bool            isVisible() const       { return m_visible; }
    unsigned long   layoutItemId() const    { return xwoid(); }

public: // painter type
    virtual void        setPainterType(XWUIGraphicsPainter type);
    XWUIGraphicsPainter painterType() const { return m_graphicsPainter; }
    bool                isGDIPainterMode() const { return (m_graphicsPainter == XWUI_PAINTER_GDI); }
    bool                isD2DPainterMode() const { return (m_graphicsPainter == XWUI_PAINTER_D2D); }

public: // focus
    virtual bool    isFocusable() const;
    virtual void    setFocus(bool bFocus);
    virtual bool    hasFocus() const;

public: // mouse events
    virtual void    onMouseEnter(int posX, int posY);
    virtual void    onMouseMove(int posX, int posY, WPARAM flags);
    virtual bool    onMouseHover(int posX, int posY);
    virtual bool    onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags);
    virtual void    onMouseLeave();
    virtual bool    onMouseWheel(int wheelDelta, int posX, int posY, WPARAM flags);
    virtual void    onMouseCaptureReset();
    virtual bool    onSetCursor();

public: // context menu
    virtual bool    onContextMenu(int posX, int posY);
    virtual void    onContextMenuItem(UINT itemId);

public: // keyboard events (sent only to focused item)
    virtual bool    onCharEvent(WPARAM charCode, LPARAM flags);

public: // timer events
    virtual bool    onTimerEvent();

public: // graphics item events
    virtual void    onItemEvent(LPARAM param);
    virtual void    onBroadcastEvent(LPARAM param);

public: // animation events
    virtual void    onAnimationTimer(DWORD id);
    virtual void    onAnimationValue(DWORD id, float value);
    virtual void    onAnimationCompleted(DWORD id);

public: // content loading events
    virtual void    onUrlContentLoaded(DWORD id, const WCHAR* path);
    virtual void    onUrlContentLoadFailed(DWORD id, DWORD reason);
    virtual void    onUrlContentCompleted(DWORD id);

public: // hit testing
    virtual bool    isInside(int posX, int posY);
    virtual bool    rectOverlap(const RECT& rect);

public: // GDI painting
    virtual void    onPaintGDI(HDC hdc, const RECT& rcPaint);   

public: // GDI resource caching
    virtual void    onInitGDIResources(HDC hdc);
    virtual void    onResetGDIResources();
    virtual void    setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache);

public: // Direct2D painting
    virtual void    onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint); 

public: // Direct2D resource caching
    virtual void    onInitD2DTarget(ID2D1RenderTarget* pTarget);
    virtual void    onResetD2DTarget();
    virtual void    setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache);

public: // item state

    // state flags
    enum TStateFlag
    {
        STATE_FLAG_MOUSEOVER        = 0x00000001,
        STATE_FLAG_PRESSED          = 0x00000002,
        STATE_FLAG_DISABLED         = 0x00000004,
        STATE_FLAG_FOCUSED          = 0x00000008,
        STATE_FLAG_MOUSECAPTURE     = 0x00000010
    };

    unsigned long stateFlags() const { return m_stateFlags; }
    XWUIStyle::XStyleState  styleState() const;

public: // update ui
    void    updateUI();

public: // paint helpers
    void    repaint(bool paintNow = false);
    void    repaint(const RECT& rcPaint, bool paintNow = false);
    void    repaint(HRGN hrgn);

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

public: // resize policy (from IXLayoutItem)
    TResizePolicy   horizontalPolicy() const;
    TResizePolicy   verticalPolicy() const;
    void            updateResizePolicies();

public: // size constraints (from IXLayoutItem)
    int     minWidth()  const;
    int     minHeight() const;
    int     maxWidth()  const;
    int     maxHeight() const;

public: // size
    RECT    rect() const    { return m_itemRect; }
    int     width() const   { return m_itemRect.right - m_itemRect.left; } 
    int     height() const  { return m_itemRect.bottom - m_itemRect.top; } 

public: // background fill (NOTE: in GDI mode use only if double buffer is enabled to avoid flickering)
    void    setBackgroundFill(const COLORREF& fillColor);
    void    clearBackgroundFill();

protected: // enable default content scrolling
    void    enableContentScrolling(bool bEnable);

protected: // timer methods
    bool    startTimer(UINT uElapseMs);
    bool    stopTimer();

protected: // animation methods
    bool    startTimerAnimation(UINT intervalMs, DWORD& idOut);
    bool    startValueAnimation(UINT intervalMs, const XWAnimationTimer::ValueAnimation& animation, DWORD& idOut);
    void    stopAnimation(DWORD id);
    void    validateAnimations();
    void    pauseAnimations();
    void    resumeAnimations();
    void    stopAllAnimations();

protected: // content loading
    bool    isUrlContentLoaded(const WCHAR* url, XMediaSource& srcOut);
    bool    loadUrlContent(const WCHAR* url, DWORD& idOut);
    void    cancelContentLoad(DWORD id);
    void    cancelAllContent();

protected: // helper methods
    void    onPaintD2DFromGDI(ID2D1RenderTarget* pTarget, const RECT& rcPaint);

protected: // protected properties
    void    setFocusable(bool focusable);
    void    setMessageProcessing(bool processing);
    void    setFocusProperty(bool hasFocus);

protected: // GDI resource cashing
    XGdiResourcesCache*     getGdiResourcesCache(HDC hdc);

protected: // Direct2D resource cashing
    XD2DResourcesCache*     getD2DResourcesCache(ID2D1RenderTarget* pTarget);
    ID2D1Brush*             createD2DBrush(ID2D1RenderTarget* pTarget, const D2D1_COLOR_F& color);
    ID2D1Brush*             createD2DBrush(ID2D1RenderTarget* pTarget, const COLORREF& color);

protected: // resource management
    bool    hasResourceCache();
    void    reloadResources();

protected: // load images to cache
    bool    loadBitmap(const XMediaSource& source, std::wstring& hashOut);  
    bool    loadBitmap(const XMediaSource& source, int width, int height, std::wstring& hashOut);  
    bool    releaseBitmap(std::wstring& bitmapHash);

protected: // item state
    void    setStateFlag(TStateFlag flag, bool value);
    bool    getStateFlag(TStateFlag flag) const;
    virtual void    onStateFlagChanged(TStateFlag flag, bool value);

protected: // events
    void    onChildObjectRemoved(XWObject* child);

protected: // worker methods
    void    _setVisibleImpl(bool bVisible);
    void    _checkGdiCacheReady(HDC hdc);
    void    _checkD2DCacheReady(ID2D1RenderTarget* pTarget);
    void    _focusNextItem();
    void    _focusItem(XGraphicsItem* pItem);
    XGraphicsItem*  _findItem(int posX, int posY);
    XGraphicsItem*  _findItem(unsigned long itemId);
    std::list<XGraphicsItem*>::iterator _findItemIt(unsigned long itemId);

protected: // item data
    RECT            m_itemRect;
    unsigned long   m_stateFlags;

protected: // background fill
    COLORREF        m_bgColor;
    bool            m_fillBackground;

protected: // caching
    XGdiResourcesCache* m_pXGdiResourcesCache;
    XD2DResourcesCache* m_pXD2DResourcesCache;

protected: // child items
    std::list<XGraphicsItem*>   m_childItems;

private: // animations
    std::vector<DWORD>          m_itemAnimations;
    std::vector<DWORD>          m_itemContentIds;

private: // data
    HWND            m_hwndParent;
    IXLayout*       m_pLayout;
    std::wstring    m_hintText;
    XPopupMenu*     m_contextMenu;
    bool            m_contextMenuEnabled;

private: // properties
    bool            m_visible;
    bool            m_obscured;
    bool            m_clickable;
    bool            m_focusable;
    bool            m_scrollingEnabled;
    bool            m_messageProcessing;
    XWUIGraphicsPainter m_graphicsPainter;

private: // properties
    XGraphicsItem*  m_mouseItem;
    XGraphicsItem*  m_focusItem;

private: // layout item properties
    TResizePolicy   m_rpHorizontal;
    TResizePolicy   m_rpVertical;
    int             m_minWidth;
    int             m_minHeight;
    int             m_maxWidth;
    int             m_maxHeight;

private: // cursors
    HCURSOR         m_itemCursor;
    HCURSOR         m_originalCursor;

private: // protect from copy and assignment
    XGraphicsItem(const XGraphicsItem& ref)  {}
    const XGraphicsItem& operator=(const XGraphicsItem& ref) { return *this;}

private: // Direct2D interoperability 
    ID2D1GdiInteropRenderTarget*    m_pGDIRenderTarget;
};

// XGraphicsItem
/////////////////////////////////////////////////////////////////////

#endif // _XGRAPHICSITEM_H_

