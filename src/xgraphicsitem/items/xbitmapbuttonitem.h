// Bitmap button grapchics item 
//
/////////////////////////////////////////////////////////////////////

#ifndef _XBITMAPBUTTONITEM_H_
#define _XBITMAPBUTTONITEM_H_

/////////////////////////////////////////////////////////////////////
// XBitmapButtonItem - bitmap button graphics item

class XBitmapButtonItem : public XImageItem
{
public: // construction/destruction
    XBitmapButtonItem(XGraphicsItem* parent = 0);
    XBitmapButtonItem(XWUIStyle::XStyleBitmapButtons button, XGraphicsItem* parent = 0);
    ~XBitmapButtonItem();

public: // interface
    void    setBitmaps(int width, int height, const XWUIStyle::XStyleBitmaps& bitmaps);
    void    setBitmaps(XWUIStyle::XStyleBitmapButtons button);

public: // properties
    void    setChecked(bool checked);
    bool    isChecked() const { return m_isChecked; }

public: // scrolling interface (from IXWScrollable)
    int     contentWidth();
    int     contentHeight();

protected: // GDI resource caching (from XGraphicsItem)
    void    onInitGDIResources(HDC hdc);
    void    onResetGDIResources();

protected: // Direct2D resource caching (from XGraphicsItem)
    void    onInitD2DTarget(ID2D1RenderTarget* pTarget);
    void    onResetD2DTarget();

protected: // item state
    void    onStateFlagChanged(TStateFlag flag, bool value);

private: // worker methods
    void    _loadBitmap(XWUIStyle::XStyleState state);
    void    _loadBitmaps();
    void    _releaseBitmaps();
    void    _selectActiveBitmap();

private: // types
    typedef std::map<XWUIStyle::XStyleState, std::wstring> _BitmapHashesT;

private: // bitmaps
    XWUIStyle::XStyleBitmaps    m_buttonBitmaps;
    _BitmapHashesT              m_bitmapHashes;

private: // data
    int     m_bitmapWidth;
    int     m_bitmapHeight;
    bool    m_isChecked;
};

// XBitmapButtonItem
/////////////////////////////////////////////////////////////////////

#endif // _XBITMAPBUTTONITEM_H_



