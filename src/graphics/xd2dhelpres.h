// Direct2D helper functions
//
/////////////////////////////////////////////////////////////////////

#ifndef _XD2DHELPERS_H_
#define _XD2DHELPERS_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
struct IWICBitmapSource;
class XImageFile;

/////////////////////////////////////////////////////////////////////
// XD2DHelpers - helper functions

namespace XD2DHelpers
{
    // dynamic loading
    bool    isDirect2DLoaded();
    bool    loadDirect2DLibrary();
    void    freeDirect2DLibrary();

    // global factory
    ID2D1Factory*   getDirect2DFactory();

    // bitmaps
    ID2D1Bitmap*    createBitmap(IWICBitmapSource* pBitmapSource, ID2D1RenderTarget* pRenderTarget);
    ID2D1Bitmap*    createSharedBitmap(ID2D1Bitmap* pOriginalImage, ID2D1RenderTarget* pRenderTarget);
    ID2D1Bitmap*    createBitmap(XImageFile* pImageFile, ID2D1RenderTarget* pRenderTarget);

    // colors
    void            d2dColorToColorref(const D2D1_COLOR_F& d2dColor, COLORREF& color);
    void            colorrefToD2dColor(const COLORREF& color, D2D1_COLOR_F& d2dColor);

    // coordinates
    void            getDpiScaleFactors(FLOAT& dpiScaleX, FLOAT& dpiScaleY);
    void            gdiRectToD2dRect(const RECT& rect, D2D1_RECT_F& d2dRect);
    void            gdiRectToD2dRect(const RECT& rect, D2D1_RECT_F& d2dRect, FLOAT dpiScaleX, FLOAT dpiScaleY);

    // dpi scale factors (cached after loading)
    FLOAT           getDpiScaleX();
    FLOAT           getDpiScaleY();

    // device update (update cache)
    void            deviceCapsUpdated();

    // conversion between pixels and dips
    inline FLOAT    pixelsToDips(int pix, FLOAT dpiScale) { return ((FLOAT)pix) / dpiScale; }
    inline int      dipsToPixels(FLOAT dip, FLOAT dpiScale) { return (int)::ceilf(dip * dpiScale + 0.5f); }
    
    // convenience wrappers for DIP conversion
    inline FLOAT    pixelsToDipsX(int pix) { return pixelsToDips(pix, getDpiScaleX()); }
    inline FLOAT    pixelsToDipsY(int pix) { return pixelsToDips(pix, getDpiScaleY()); }
    inline int      dipsToPixelsX(FLOAT dip) { return dipsToPixels(dip, getDpiScaleX()); }
    inline int      dipsToPixelsY(FLOAT dip) { return dipsToPixels(dip, getDpiScaleY()); }

    // color brush cache
    struct XD2DColorBrushRef
    {
        ID2D1Brush*     brush;
        D2D1_COLOR_F    color;
    };

    // cache type
    typedef std::vector<XD2DColorBrushRef>      XD2DColorBrushCache;

    // brush cache helpers
    void            resetColorBrushCache(XD2DColorBrushCache& brushCache);
    ID2D1Brush*     findColorBrush(const XD2DColorBrushCache& brushCache, const D2D1_COLOR_F& color);
    void            appendColorBrush(XD2DColorBrushCache& brushCache, ID2D1Brush* brush, const D2D1_COLOR_F& color);
};

// XD2DHelpers
/////////////////////////////////////////////////////////////////////

#endif // _XD2DHELPERS_H_

