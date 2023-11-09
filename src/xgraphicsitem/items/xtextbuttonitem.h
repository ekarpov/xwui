// Text button grapchics item implementation
//
/////////////////////////////////////////////////////////////////////

#ifndef _XTEXTBUTTONITEM_H_
#define _XTEXTBUTTONITEM_H_

/////////////////////////////////////////////////////////////////////
// XTextButtonItem - text button graphics item

class XTextButtonItem : public XGraphicsItem
{
public: // construction/destruction
    XTextButtonItem(XGraphicsItem* parent = 0);
    XTextButtonItem(const XWUIStyle::XTextButtonStyle& style, XGraphicsItem* parent = 0);
    ~XTextButtonItem();

public: // interface
    void    setText(const wchar_t* text);

public: // style
    void    setStyle(const XWUIStyle::XTextButtonStyle& style);
    const XWUIStyle::XTextButtonStyle& getStyle() const; 

public: // manipulations (from XGraphicsItem)
    void    update(int posX, int posY, int width, int height);

public: // mouse events
    bool    onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags);

public: // keyboard events (from XGraphicsItem)
    bool    onCharEvent(WPARAM charCode, LPARAM flags);

public: // GDI painting
    void    onPaintGDI(HDC hdc, const RECT& rcPaint);   

public: // Direct2D painting
    void    onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint); 

public: // style state
    void    onStateFlagChanged(TStateFlag flag, bool value);

private: // worker method
    void    _init(const XWUIStyle::XTextButtonStyle& style);
    void    _handleContentChanged();

private: // data
    XTextItem*                  m_textItem;
    XWUIStyle::XTextButtonStyle m_btnStyle;
};

// XTextButtonItem
/////////////////////////////////////////////////////////////////////

#endif // _XTEXTBUTTONITEM_H_

