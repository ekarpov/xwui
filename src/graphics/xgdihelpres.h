// GDI helper functions
//
/////////////////////////////////////////////////////////////////////

#ifndef _XGDIHELPERS_H_
#define _XGDIHELPERS_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
struct IWICBitmapSource;

/////////////////////////////////////////////////////////////////////
// XGdiHelpers - GDI helper functions

namespace XGdiHelpers
{
    // NOTE: created bitmaps are 32bbp RGB

    // bitmaps
    HBITMAP     createBitmap(HDC hdc, int width, int height, void** imageBitsOut);
    HBITMAP     scaleBitmap(HDC hdc, HBITMAP hBitmap, int width, int height, bool premultiply);

    // WIC bitmaps
    HBITMAP     createBitmap(HDC hdc, IWICBitmapSource* pBitmapSource, bool premultiply);
    HBITMAP     createBitmap(HDC hdc, IWICBitmapSource* pBitmapSource, int width, int height, bool premultiply);

    // copy pixels from WIC bitmap (buffer must fit at least width * height pixels)
    bool        copyPixelsFromBitmap(DWORD* bitmapPixels, int width, int height, IWICBitmapSource* pBitmapSource, bool premultiply);

    // bitmap properties
    bool        getBitmapSize(HBITMAP hBitmap, int& width, int& height);

    // device capabilities
    int         getGlobalDeviceCaps(int index);
    void        deviceCapsUpdated();

    // pixels per inch
    int         getPixelsPerInchX();
    int         getPixelsPerInchY();

    // convert dips to pixels
    int         dpiScalePixelsX(int pix);
    int         dpiScalePixelsY(int pix);
    int         dpiDipsToPixelsX(int dips);
    int         dpiDipsToPixelsY(int dips);

    // twips (1/1440 of an inch)
    LONG        pixelsToTwipsX(int pix);
    LONG        pixelsToTwipsY(int pix);
    int         twipsToPixelsX(LONG twips);
    int         twipsToPixelsY(LONG twips);

    // points (1/72 of an inch)
    float       pixelsToPointsX(int pix);
    float       pixelsToPointsY(int pix);
    int         pointsToPixelsX(float points);
    int         pointsToPixelsY(float points);

    // points to twips
    LONG        pointsToTwips(float points);
    float       twipsToPoints(LONG twips);

    // HIMETRIC conversion
    LONG        pixToHimetricX(LONG x);
    LONG        pixToHimetricY(LONG y);

    // drawing
    void        fillRect(HDC hdc, const RECT& rcPaint, COLORREF fillColor);
    void        fillRect(HDC hdc, const RECT& rcPaint, COLORREF fillColor, BYTE alpha);
    void        fillBitmapPixels(DWORD* bitmapPixels, size_t pixelCount, COLORREF fillColor, BYTE alpha);
    void        roundRectGradient(HDC hdc, const RECT& rect, int radius, 
                                  COLORREF beginColor, COLORREF endColor);

    // text sizes
    void        getTextWidth(HDC hdc, HFONT font, const wchar_t * text, LONG& width);
    void        getTextSize(HDC hdc, HFONT font, const wchar_t * text, LONG& width, LONG& height);
    void        getWindowTextWidth(HWND hwnd, HFONT font, const wchar_t * text, LONG& width);
    void        getWindowTextSize(HWND hwnd, HFONT font, const wchar_t * text, LONG& width, LONG& height);
};

// XGdiHelpers
/////////////////////////////////////////////////////////////////////

#endif // _XGDIHELPERS_H_

