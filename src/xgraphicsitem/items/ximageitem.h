// Image graphics item
//
/////////////////////////////////////////////////////////////////////

#ifndef _XIMAGEITEM_H_
#define _XIMAGEITEM_H_

/////////////////////////////////////////////////////////////////////

// NOTE: XImageItem assumes that image path that it gets is always valid,
//       in case image validation is needed it should be done before setting image

/////////////////////////////////////////////////////////////////////
// XImageItem - image item

class XImageItem : public XGraphicsItem
{
public: // construction/destruction
    XImageItem(XGraphicsItem* parent = 0);
    ~XImageItem();
        
public: // set image
    bool    setImage(IWICBitmap* wicBitmap);
    bool    setImage(HBITMAP hBitmap);              // NOTE: item doesn't take HBITMAP ownership
    bool    setImagePath(const WCHAR* filePath);
    bool    setImageResourcePath(HMODULE hModule, const WCHAR* szResName, const WCHAR* szResType);
    bool    setImageStylePath(const WCHAR* stylePath);
    bool    setImageSource(const XMediaSource& source);
    void    resetImage();

public: // properties
    void    setFitToSize(bool fitToSize, bool cutToFit = false);
    void    setTransparency(BYTE transparency);
    void    setImageSize(int width, int height, bool keepAspectRatio = false);

public: // image properties
    int     imageWidth();
    int     imageHeight();

public: // alignment
    enum TAlignment
    {
        eAlignLeft,
        eAlignRight,
        eAlignTop,
        eAlignBottom,
        eAlignCenter
    };

    void    setVerticalAlignment(TAlignment alignment);
    void    setHorizontalAlignment(TAlignment alignment);

public: // frame animation
    bool    hasFrameAnimation() const;
    void    setDefaultFrameDelay(int delay);
    bool    startFrameAnimation();
    void    stopFrameAnimation();
    bool    isFrameAnimationActive() const;

public: // scrolling (from XGraphicsItem)
    int     contentWidth();
    int     contentHeight();

public: // GDI resource caching (from XGraphicsItem)
    void    onInitGDIResources(HDC hdc);
    void    onResetGDIResources();
    void    setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache);

public: // GDI painting (from XGraphicsItem)
    void    onPaintGDI(HDC hdc, const RECT& rcPaint);   

public: // Direct2D resource caching (from XGraphicsItem)
    void    onInitD2DTarget(ID2D1RenderTarget* pTarget);
    void    onResetD2DTarget();
    void    setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache);

public: // Direct2D painting (from XGraphicsItem)
    void    onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint); 

public: // animation events (from XGraphicsItem)
    void    onAnimationTimer(DWORD id);

private: // worker methods
    bool    _startNextFrameTimer();

private: // protect from copy and assignment
    XImageItem(const XImageItem& ref)  {}
    const XImageItem& operator=(const XImageItem& ref) { return *this;}

private: // data
    bool            m_bFitToSize;
    bool            m_cutToFit;
    BYTE            m_bTransparency;
    TAlignment      m_alignVertical;
    TAlignment      m_alignHorizontal;
    XImage          m_itemImage;
    int             m_frameDelay;
    DWORD           m_animationId;
};

// XImageItem
/////////////////////////////////////////////////////////////////////

#endif // _XIMAGEITEM_H_

