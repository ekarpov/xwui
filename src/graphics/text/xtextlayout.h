// Text layout
//
/////////////////////////////////////////////////////////////////////

#ifndef _XTEXTLAYOUT_H_
#define _XTEXTLAYOUT_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
class XGdiResourcesCache;
class XD2DResourcesCache;
class XGdiTextLayout;
class XD2DTextLayout;

/////////////////////////////////////////////////////////////////////

// NOTE: Text layout can work in two modes: GDI and Direct2D. Mode can be changed
//       with setPainterType function. If GDI is selected a device context (HDC)
//       must be provided when needed, while in Direct2D mode HDC will be ignored 
//       (e.g. set it to zero)

/////////////////////////////////////////////////////////////////////
// XTextLayout - text layout funnctionality

class XTextLayout : public IXRichTextObserver,
                    public IXRichTextParserObserver
{
public: // construction/destruction
    XTextLayout();
    ~XTextLayout();

public: // interface
    void    setPainterType(XWUIGraphicsPainter painterType);

public: // size
    void    resize(int width);
    int     width() const { return m_width; }

public: // content size
    int     contentWidth(HDC hdc);
    int     contentHeight(HDC hdc);

public: // pain text
    void    setTextStyle(const XTextStyle& style);
    void    setTextColor(COLORREF textColor);
    void    setText(const wchar_t* text);

public: // formatted text 
    void    setFormattedText(const wchar_t* text);

public: // inline object animation 
    void    enableAnimation(HWND parentWindow, bool enable);
    void    pauseAnimation();
    void    resumeAnimation();

public: // rich text
    XRichText*  richText() { return &m_richText; }

public: // word wrap
    void    setWordWrap(bool bWordWrap);
    bool    wordWrap() const;

public: // single line mode
    void    setSingleLineMode(bool singleLine);
    bool    singleLineMode() const;

public: // alignment
    void    setAlignment(TTextAlignment textAlignment);
    TTextAlignment  alignment() const;

public: // line spacing 
    void    setLinePadding(int beforeLine, int afterLine);
    void    getLinePadding(int& beforeLine, int& afterLine) const;

public: // size calculations
    int     getHeightForWidth(HDC hdc, int width);

public: // line properties
    int     getLineCount(HDC hdc);
    bool    getLineMetrics(HDC hdc, int lineIdx, int& textBegin, int& textEnd, int& lineHeight);
    bool    getLineFitPos(HDC hdc, int lineIdx, int maxLineWidth, int& textPos);

public: // hit testing
    bool    isInsideText(HDC hdc, int originX, int originY, int posX, int posY);
    bool    isInsideSelection(HDC hdc, int originX, int originY, int posX, int posY);
    bool    getTextFromPos(HDC hdc, int originX, int originY, int posX, int posY, unsigned int& textPos);

public: // regions
    XRectRegion getTextRegion(HDC hdc, int originX, int originY, int textPos, int textLength);

public: // selection
    bool    selectionActive() const;
    void    selectionBegin();
    void    selectionEnd();
    bool    selectTo(HDC hdc, int originX, int originY, int selectFomX, int selectFromY, int selectToX, int selectToY);
    void    clearSelection();
    void    getSelectedText(XTextRange& selectedText);

public: // selection colors
    void    setSelectionColor(COLORREF clFillColor);
    void    setSelectionTextColor(COLORREF clTextColor);
    void    resetSelectionTextColor();

public: // background  
    void    setBackgroundFill(COLORREF clFillColor);
    void    clearBackgroundFill();
    bool    backgroundFillEnabled() const;
    COLORREF backgroundFillColor() const;

public: // GDI properties
    void    enableGDIDoubleBuffering(bool enable);

public: // GDI painting 
    void    onInitGDIResources(HDC hdc);
    void    onResetGDIResources();
    void    onPaintGDI(HDC hdc, int originX, int originY, const RECT& rcPaint);
    void    setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache);

public: // Direct2D painting 
    void    onInitD2DTarget(ID2D1RenderTarget* pTarget);
    void    onResetD2DTarget();
    void    onPaintD2D(int originX, int originY, ID2D1RenderTarget* pTarget, const RECT& rcPaint); 
    void    setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache);

public: // rich text changes (from IXRichTextObserver)
    void    onRichTextModified();
    void    onRichTextStyleChanged(const XTextRange& range);
    void    onRichTextColorChanged(const XTextRange& range);

public: // rich text parsing (from IXRichTextParserObserver)
    void    onRichTextParserText(const wchar_t* text, const XTextStyle& style);
    void    onRichTextParserColoredText(const wchar_t* text, const XTextStyle& style, const COLORREF& color);
    void    onRichTextParserImage(const wchar_t* imageUri, int width, int height);

private: // protect from copy and assignment
    XTextLayout(const XTextLayout& ref)  {}
    const XTextLayout& operator=(const XTextLayout& ref) { return *this;}

private: // worker methods
    bool    _validateState() const;
    bool    _validateInput(HDC hdc) const;

private: // text
    XRichText           m_richText;
    XTextStyle          m_defaultStyle;
    COLORREF            m_defaultTextColor;
    XRichTextParser     m_textParser;

private: // layout data
    XGdiTextLayout*     m_gdiTextLayout;
    XD2DTextLayout*     m_d2dTextLayout; 

private: // data
    int                 m_width;
    bool                m_runAnimation;
    HWND                m_hwndParent;
};

// XTextLayout
/////////////////////////////////////////////////////////////////////

#endif // _XTEXTLAYOUT_H_

