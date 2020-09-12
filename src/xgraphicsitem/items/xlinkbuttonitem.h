// Text link button grapchics item implementation
//
/////////////////////////////////////////////////////////////////////

#ifndef _XLINKBUTTONITEM_H_
#define _XLINKBUTTONITEM_H_

/////////////////////////////////////////////////////////////////////
// XLinkButtonItem - link button graphics item

class XLinkButtonItem : public XGraphicsItem
{
public: // construction/destruction
    XLinkButtonItem(XGraphicsItem* parent = 0);
    XLinkButtonItem(const XWUIStyle::XLinkButtonStyle& style, XGraphicsItem* parent = 0);
    ~XLinkButtonItem();

public: // interface
    void    setText(const wchar_t* text);

public: // style
    void    setStyle(const XWUIStyle::XLinkButtonStyle& style);
    const XWUIStyle::XLinkButtonStyle& getStyle() const; 

public: // manipulations (from XGraphicsItem)
    void    update(int posX, int posY, int width, int height);

public: // mouse events
    bool    onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags);

public: // style state
    void    onStateFlagChanged(TStateFlag flag, bool value);

private: // worker method
    void    _init(const XWUIStyle::XLinkButtonStyle& style);
    void    _handleContentChanged();
    void    _handleStateChanged();

private: // data
    XTextItem*                  m_textItem;
    XWUIStyle::XLinkButtonStyle m_btnStyle;
};

// XLinkButtonItem
/////////////////////////////////////////////////////////////////////

#endif // _XLINKBUTTONITEM_H_


