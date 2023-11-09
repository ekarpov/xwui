// Text label (single line of text)
//
/////////////////////////////////////////////////////////////////////

#ifndef _XTEXTLABELITEM_H_
#define _XTEXTLABELITEM_H_

/////////////////////////////////////////////////////////////////////
// XTextLabelItem - single line text label

class XTextLabelItem : public XGraphicsItem
{
public: // construction/destruction
    XTextLabelItem(XGraphicsItem* parent = 0);
    ~XTextLabelItem();

public: // properties
    void    setContentMargins(int left, int top, int right, int bottom);
    void    enableCutToFit(bool enable);
    void    setClickable(bool clickable);

public: // text
    void    setTextStyle(const XTextStyle& style);
    void    setTextColor(COLORREF textColor);
    void    setText(const wchar_t* text);
    void    setFormattedText(const wchar_t* text);

public: // text alignment
    void    setAlignment(TTextAlignment textAlignment);
    TTextAlignment  alignment() const;

public: // background  
    void    setBackgroundFill(const COLORREF& fillColor);
    void    clearBackgroundFill();

public: // item size and position (from XGraphicsItem)
    void    update(int posX, int posY, int width, int height);
    int     contentWidth();
    int     contentHeight();

public: // painter type (from XGraphicsItem)
    void    setPainterType(XWUIGraphicsPainter type);

public: // mouse events (from XGraphicsItem)
    void    onMouseEnter(int posX, int posY);
    void    onMouseMove(int posX, int posY, WPARAM flags);
    bool    onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags);
    void    onMouseLeave();

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

private: // worker methods
    void    _setFullText();
    void    _fitContent(int width);
    void    _updateItemSize();
    void    _resetClickableState();
    void    _updateClickableState(int posX, int posY);

private: // data
    XTextLayout     m_textLayout;
    std::wstring    m_labelText;
    bool            m_cutToFit;
    bool            m_isTextCut;
    bool            m_isFormattedText;
    bool            m_isClickable;
    bool            m_isOverText;
    int             m_fitWidth;
    int             m_leftMargin;
    int             m_topMargin;
    int             m_rightMargin;
    int             m_bottomMargin;
};

// XTextLabelItem
/////////////////////////////////////////////////////////////////////

#endif // _XTEXTLABELITEM_H_

