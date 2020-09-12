// Text edit graphics item
//
/////////////////////////////////////////////////////////////////////

#ifndef _XTEXTEDITITEM_H_
#define _XTEXTEDITITEM_H_

/////////////////////////////////////////////////////////////////////
// windowless richedit 
#include "../../graphics/text/xrichtextedit.h"

/////////////////////////////////////////////////////////////////////

// NOTE: use GDI double buffering in parent window with windowless rich 
//       edit, otherwise it may flicker

/////////////////////////////////////////////////////////////////////
// XTextEditItem - text edit item

class XTextEditItem : public XGraphicsItem,
                              public IXRichEditObserver
{
public: // construction/destruction
    XTextEditItem(XGraphicsItem* parent = 0);
    ~XTextEditItem();

public: // richtextedit interface
    XRichTextEdit*  richTextEdit() { return m_richTextEdit; }

public: // parent window
    void    setParentWindow(HWND hwndParent);

public: // message processing (from XGraphicsItem)
    LRESULT processWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& messageProcessed);

public: // scrolling (from XGraphicsItem)
    int     contentWidth();
    int     contentHeight();
    void    setScrollOffsetX(int scrollOffsetX);
    void    setScrollOffsetY(int scrollOffsetY);

public: // focus (from XGraphicsItem)
    bool    isFocusable() const;
    void    setFocus(bool bFocus);

public: // manipulations (from XGraphicsItem)
    void    update(int posX, int posY, int width, int height);

public: // properties (from XGraphicsItem)
    void    setEnabled(bool bEnabled);

public: // mouse events
    bool    onSetCursor();

public: // GDI painting (from XGraphicsItem)
    void    onPaintGDI(HDC hdc, const RECT& rcPaint);   

public: // Direct2D painting (from XGraphicsItem)
    void    onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint); 

private: // scrollbars (from IXRichEditHostObserver)
    void    onRichEditVerticalScrollPos(int pos);
    void    onRichEditHorizontalScrollPos(int pos);

private: // content (from IXRichEditHostObserver)
    void    onRichEditContentRectChanged(const RECT& rcContent);

private: // helper methods
    void    _initRichEdit(HWND hwndParent);

private: // data
    XRichTextEdit*  m_richTextEdit;
};

// XTextEditItem
/////////////////////////////////////////////////////////////////////

#endif // _XTEXTEDITITEM_H_

