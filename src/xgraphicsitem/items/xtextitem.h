// Text graphics item
//
/////////////////////////////////////////////////////////////////////

#ifndef _XTEXTITEM_H_
#define _XTEXTITEM_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
class XRichText;
class XGdiTextLayout;
class XTextInlineImage;

/////////////////////////////////////////////////////////////////////
// IXTextItemObserver - text item events observer

class IXTextItemObserver
{
public: // construction/destruction
    IXTextItemObserver() {}
    virtual ~IXTextItemObserver() {}

public: // interface
    virtual void    onTextRangeMouseEnter(const XTextRange& range, const std::wstring& command) {}
    virtual void    onTextRangeMouseLeave(const XTextRange& range, const std::wstring& command) {}
    virtual void    onTextRangeClicked(const XTextRange& range, const std::wstring& command) {}
};

// IXTextItemObserver
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// XTextItem - text item

class XTextItem : public XGraphicsItem,
                  public IXRichTextParserObserver
{
public: // construction/destruction
    XTextItem(XGraphicsItem* parent = 0);
    ~XTextItem();

public: // interface
    void    layoutText(int width);

public: // text item events observer
    void    addObserver(IXTextItemObserver* observerRef);
    void    removeObserver(IXTextItemObserver* observerRef);

public: // default settings
    void    initDefault();

public: // properties
    void    enableSelection(bool enable);
    void    enableDragAndDrop(bool enable);

public: // text
    void    setTextStyle(const XTextStyle& style);
    void    setText(const wchar_t* text);

public: // formatted text 
    void    setFormattedText(const wchar_t* text);

public: // inline object animation 
    void    enableAnimation(bool enable);

public: // rich text
    XRichText*      richText();

public: // word wrap
    void    setWordWrap(bool bWordWrap);
    bool    wordWrap() const;

public: // alignment
    void    setAlignment(TTextAlignment textAlignment);
    TTextAlignment  alignment() const;

public: // line spacing 
    void    setLinePadding(int beforeLine, int afterLine);
    void    getLinePadding(int& beforeLine, int& afterLine) const;

public: // size
    int     getHeightForWidth(int width);

public: // line properties
    int     getLineCount();
    bool    getLineMetrics(int lineIdx, int& textBegin, int& textEnd, int& lineHeight);

public: // hit testing
    bool    isInsideText(int posX, int posY);
    bool    isInsideSelection(int posX, int posY);
    bool    getTextFromPos(int posX, int posY, unsigned int& textPos);

public: // clickable text regions
    void    addClickableText(const wchar_t* command, const XTextRange& range, const XTextStyle* hoverStyle = 0, xstyle_index_t styleMask = 0);

public: // selection
    void    clearSelection();
    void    getSelectedText(XTextRange& selectedText);

public: // selection colors
    void    setSelectionColor(COLORREF clFillColor);
    void    setSelectionTextColor(COLORREF clTextColor);
    void    resetSelectionTextColor();

public: // link color (clickable area)
    void    setLinkColor(COLORREF clLinkColor);

public: // background  
    void        setBackgroundFill(COLORREF clFillColor);
    void        clearBackgroundFill();
    bool        backgroundFillEnabled() const;
    COLORREF    backgroundFillColor() const;

public: // scrolling
    int     contentWidth();
    int     contentHeight();

public: // properties (from XGraphicsItem)
    void    setObscured(bool bObscured);

public: // parent window (from XGraphicsItem)
    void    setParentWindow(HWND hwndParent);

public: // painter type (from XGraphicsItem)
    void    setPainterType(XWUIGraphicsPainter type);

public: // position (from XGraphicsItem)
    void    move(int posX, int posY);

public: // manipulations (from XGraphicsItem)
    void    update(int posX, int posY, int width, int height);

public: // properties (from XGraphicsItem)
    void    setEnabled(bool bEnabled);

public: // mouse events (from XGraphicsItem)
    void    onMouseEnter(int posX, int posY);
    void    onMouseMove(int posX, int posY, WPARAM flags);
    bool    onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags);
    void    onMouseLeave();
    void    onMouseCaptureReset();
    bool    onSetCursor();

public: // keyboard events (from XGraphicsItem)
    bool    onCharEvent(WPARAM charCode, LPARAM flags);

public: // GDI painting (from XGraphicsItem)
    void    enableGDIDoubleBuffering(bool enable);
    void    onPaintGDI(HDC hdc, const RECT& rcPaint);   

public: // GDI resource caching (from XGraphicsItem)
    void    onInitGDIResources(HDC hdc);
    void    onResetGDIResources();
    void    setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache);

public: // Direct2D painting (from XGraphicsItem)
    void    onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint); 

public: // Direct2D resource caching (from XGraphicsItem)
    void    onInitD2DTarget(ID2D1RenderTarget* pTarget);
    void    onResetD2DTarget();
    void    setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache);

private: // rich text parsing (from IXRichTextParserObserver)
    void    onRichTextParserText(const wchar_t* text, const XTextStyle& style);
    void    onRichTextParserColoredText(const wchar_t* text, const XTextStyle& style, const COLORREF& color);
    void    onRichTextParserLink(const XTextRange& range, const wchar_t* url);
    void    onRichTextParserImage(const wchar_t* imageUri, int width, int height);

private: // protect from copy and assignment
    XTextItem(const XTextItem& ref)  {}
    const XTextItem& operator=(const XTextItem& ref) { return *this;}

private: // helper types
    struct XClickableTextRegion
    {
        XTextRange          range;
        XRectRegion         region;
        xstyle_index_t      hoverStyle;
        xstyle_index_t      styleMask;
        std::wstring        command;

        std::vector<xstyle_index_t> originalStyles;
    };

    enum TActiveCursor
    {
        eCursorDefault,
        eCursorSelection,
        eCursorPointer
    };

    struct InlineImage
    {
        XTextInlineImage*   image;
        std::wstring        path;
        std::wstring        hash;
    };

private: // helper methods
    void    _deleteClickableRegions();
    void    _updateClickableRegions();
    void    _updateMouseCursor(int posX, int posY);
    void    _onClickableRangeMouseEnter(XClickableTextRegion* region);
    void    _onClickableRangeMouseLeave(XClickableTextRegion* region);

private: // selection helpers
    void    _selectionBegin(int posX, int posY);
    void    _selectTo(int posX, int posY);
    void    _selectionEnd(int posX, int posY);

private: // cursor helpers
    void    _setActiveCursor(TActiveCursor cursor);

private: // text layout 
    XTextLayout     m_textLayout;
    XTextStyle      m_defaultStyle;
    XRichTextParser m_textParser;

private: // cursor
    TActiveCursor   m_activeCursor;

private: // data 
    int             m_originX;
    int             m_originY;
    POINT           m_clickPoint;
    bool            m_runAnimation;

private: // properties
    bool            m_selectionEnabled;
    bool            m_dragAndDropEnabled;
    bool            m_dragAndDropActive;
    COLORREF        m_linkColor;

private: // clickable regions
    std::vector<XClickableTextRegion>   m_clickableRegions;
    XClickableTextRegion*               m_activeRegion;

private: // observer
    IXTextItemObserver*                 m_observerRef;
};

// XTextItem
/////////////////////////////////////////////////////////////////////

#endif // _XTEXTITEM_H_

