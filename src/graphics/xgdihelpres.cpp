// GDI helper functions
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xwichelpers.h"
#include "xgdihelpres.h"

/////////////////////////////////////////////////////////////////////

// NOTE: Relationship Between Inches, Picas, Points, Pitch, and Twips
//       https://support.microsoft.com/en-us/help/76388/relationship-between-inches,-picas,-points,-pitch,-and-twips

// NOTE: Calculating The Logical Height and Point Size of a Font
//       https://support.microsoft.com/en-us/help/74299/info-calculating-the-logical-height-and-point-size-of-a-font

/////////////////////////////////////////////////////////////////////
// global constants

// pixels per inch
int     g_gdiPixelsPerInchX = 0;
int     g_gdiPixelsPerInchY = 0;

/////////////////////////////////////////////////////////////////////
// XGdiHelpers - GDI helper functions

/////////////////////////////////////////////////////////////////////
// bitmaps
/////////////////////////////////////////////////////////////////////
HBITMAP XGdiHelpers::createBitmap(HDC hdc, int width, int height, void** imageBitsOut)
{
    // fill BITMAPINFO
    BITMAPINFO bminfo;
    ZeroMemory(&bminfo, sizeof(bminfo));
    bminfo.bmiHeader.biSize         = sizeof(BITMAPINFOHEADER);
    bminfo.bmiHeader.biWidth        = width;
    bminfo.bmiHeader.biHeight       = -(LONG)height;
    bminfo.bmiHeader.biPlanes       = 1;
    bminfo.bmiHeader.biBitCount     = 32;
    bminfo.bmiHeader.biCompression  = BI_RGB;

    // use screen DC if not provided
    HDC hdcScreen = 0;
    if(hdc == 0)
    {
        hdcScreen = ::GetDC(NULL);
        if(hdcScreen == 0)
        {
            XWTRACE_WERR_LAST("XGdiHelpers: failed to get screen DC");
            return 0;
        }
        hdc = hdcScreen;
    }

    // create bitmap
    HBITMAP hRetBitmap = ::CreateDIBSection(hdc, &bminfo, DIB_RGB_COLORS, imageBitsOut, NULL, 0);
    if(hRetBitmap == 0)
    {
        XWTRACE_WERR_LAST("XGdiHelpers: failed to create DIB section");
    }

    // release screen DC
    if(hdcScreen)
    {
        ::ReleaseDC(NULL, hdcScreen);
    }

    return hRetBitmap;
}

HBITMAP XGdiHelpers::scaleBitmap(HDC hdc, HBITMAP hBitmap, int width, int height, bool premultiply)
{
    // check input
    XWASSERT(hBitmap);
    if(hBitmap == 0) return 0;

    HBITMAP hScaledBitmap = 0;

    // use WIC if supproted
    if(XWicHelpers::isSupported())
    {
        // create WIC bitmap first
        IWICBitmap* pWicBitmap = XWicHelpers::createBitmap(hBitmap);
        if(pWicBitmap)
        {
            // create bitmap
            hScaledBitmap = XGdiHelpers::createBitmap(hdc, pWicBitmap, width, height, premultiply);

            // release temporary WIC bitmap
            pWicBitmap->Release();
        }
    }

    // use GDI if WIC didn't help
    if(hScaledBitmap == 0)
    {
        // get image size
        int nImageWidth = 0;
        int nImageHeight = 0;

        if(!XGdiHelpers::getBitmapSize(hBitmap, nImageWidth, nImageHeight)) return 0;

        // create source DC
        HDC hSrcDC = ::CreateCompatibleDC(NULL);
        ::SelectObject(hSrcDC, hBitmap);

        // create destination DC
        HDC hDestDc = ::CreateCompatibleDC(hSrcDC);
        hScaledBitmap = ::CreateCompatibleBitmap(hSrcDC, width, height);
        ::SelectObject(hDestDc, hScaledBitmap);

        // scale
        ::SetStretchBltMode(hDestDc, HALFTONE);
        if(::StretchBlt(hDestDc, 0, 0, width, height, hSrcDC, 0, 0, nImageWidth, nImageHeight, SRCCOPY) == 0)
        {
            // failed to scale, delete image
            ::DeleteObject(hScaledBitmap);
            hScaledBitmap = 0;
        }

        // release temporary DC
        ::DeleteDC(hSrcDC);
        ::DeleteDC(hDestDc);
    }

    return hScaledBitmap;
}

/////////////////////////////////////////////////////////////////////
// WIC bitmaps
/////////////////////////////////////////////////////////////////////
HBITMAP XGdiHelpers::createBitmap(HDC hdc, IWICBitmapSource* pBitmapSource, bool premultiply)
{
    // image size
    UINT width = 0;
    UINT height = 0;

    // get size
    HRESULT res = pBitmapSource->GetSize(&width, &height);
    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XGdiHelpers: failed to get size from bitmap source", res);
        return 0;
    }

    // create bitmap
    void* pImageBits = 0;
    HBITMAP hRetBitmap = createBitmap(hdc, width, height, &pImageBits);
    
    // fill bits
    if(hRetBitmap)
    {
        // extract pixels
        if(!copyPixelsFromBitmap((DWORD*)pImageBits, width, height, pBitmapSource, premultiply))
        {
            // trace error
            XWTRACE_HRES("XGdiHelpers: failed to copy pixels from bitmap source", res);

            // release bitmap
            ::DeleteObject(hRetBitmap);
            hRetBitmap = 0;
        }

    } else
    {
        XWTRACE_WERR_LAST("XGdiHelpers: failed to create DIB section");
    }

    return hRetBitmap;
}

HBITMAP XGdiHelpers::createBitmap(HDC hdc, IWICBitmapSource* pBitmapSource, int width, int height, bool premultiply)
{
    HBITMAP hRetBitmap = 0;

    // scale bitmap first
    IWICBitmapSource* pScaledBitmap = XWicHelpers::scaleImageSource(pBitmapSource, width, height);
    if(pScaledBitmap)
    {
        // convert
        hRetBitmap = XGdiHelpers::createBitmap(hdc, pScaledBitmap, premultiply);

        // release scaled bitmap
        pScaledBitmap->Release();
    }

    return hRetBitmap;
}

/////////////////////////////////////////////////////////////////////
// fill HBITMAP with WIC bitmap
/////////////////////////////////////////////////////////////////////
bool XGdiHelpers::copyPixelsFromBitmap(DWORD* bitmapPixels, int width, int height, IWICBitmapSource* pOriginalBitmapSource, bool premultiply)
{
    // check input
    XWASSERT(bitmapPixels);
    XWASSERT(width);
    XWASSERT(height);
    XWASSERT(pOriginalBitmapSource);
    if(bitmapPixels == 0 || width == 0 || height == 0 || pOriginalBitmapSource == 0) return 0;

    // image size
    UINT bitmapWidth = 0;
    UINT bitmapHeight = 0;

    // get size
    HRESULT res = pOriginalBitmapSource->GetSize(&bitmapWidth, &bitmapHeight);
    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XGdiHelpers: failed to get size from bitmap source", res);
        return 0;
    }

    // check if size matches
    IWICBitmapSource* pScaledBitmap = 0;
    if(bitmapWidth != width || bitmapHeight != height)
    {
        // scale bitmap
        pScaledBitmap = XWicHelpers::scaleImageSource(pOriginalBitmapSource, width, height);
        if(pScaledBitmap)
        {
            // switch
            pOriginalBitmapSource = pScaledBitmap;
        }
    }

    // convert image to the 32bppBGR (or pre-multiplied if needed) pixel format for GDI rendering 
    IWICFormatConverter* pBitmapSource = 0;
    res = XWicHelpers::getImagingFactory()->CreateFormatConverter(&pBitmapSource);
    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XGdiHelpers: failed to create format converter", res);
        if(pScaledBitmap) pScaledBitmap->Release();
        return 0;
    }

    // initialize converter
    if(premultiply)
    {
        // load pre-multiplied bitmap (need for AlphaBlend to work with transparency)
        res = pBitmapSource->Initialize(pOriginalBitmapSource, GUID_WICPixelFormat32bppPBGRA, 
            WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeCustom);
    } else
    {
        // load normal bitmap
        res = pBitmapSource->Initialize(pOriginalBitmapSource, GUID_WICPixelFormat32bppBGR, 
            WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeCustom);
    }

    // release scaling converter
    if(pScaledBitmap) pScaledBitmap->Release();

    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XGdiHelpers: failed to initialize format converter", res);
        pBitmapSource->Release();
        return 0;
    }

    // size of a scan line represented in bytes: 4 bytes each pixel
    UINT cbStride = 4 * width;

    // size of the image, represented in bytes
    UINT cbBufferSize = cbStride * height;

    // extract pixels
    res = pBitmapSource->CopyPixels(0, cbStride, cbBufferSize, (BYTE*)bitmapPixels);
    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XGdiHelpers: failed to copy pixels from bitmap source", res);
    }

    // release converter
    pBitmapSource->Release();

    // return result
    return (SUCCEEDED(res)) ? true : false;
}

/////////////////////////////////////////////////////////////////////
// bitmap properties
/////////////////////////////////////////////////////////////////////
bool XGdiHelpers::getBitmapSize(HBITMAP hBitmap, int& width, int& height)
{
    XWASSERT(hBitmap);

    BITMAP bm;
    if(::GetObjectW(hBitmap, sizeof(bm), &bm))
    {
        // copy size
        width = bm.bmWidth;
        height = bm.bmHeight;
        return true;
    }

    // failed to get size
    XWTRACE_WERR_LAST("XGdiHelpers::getBitmapSize failed to get image size");
    return false;
}

/////////////////////////////////////////////////////////////////////
// device capabilities
/////////////////////////////////////////////////////////////////////
int XGdiHelpers::getGlobalDeviceCaps(int index)
{
    int retCaps = 0;

    // get global DC
    HDC hdc = ::GetDC(::GetDesktopWindow());
    if(hdc)
    {
        // get caps
        retCaps = ::GetDeviceCaps(hdc, index);

        // release DC
        ::ReleaseDC(::GetDesktopWindow(), hdc);

    } else
    {
        XWTRACE_WERR_LAST("XWUtils::sGetGlobalDeviceCaps failed");

        // use some predefined values
        switch(index)
        {
        case LOGPIXELSX: return 96; 
        case LOGPIXELSY: return 96; 
        }
    }

    return retCaps;
}

void XGdiHelpers::deviceCapsUpdated()
{
    // reset cached values
    g_gdiPixelsPerInchX = 0;
    g_gdiPixelsPerInchY = 0;
}

/////////////////////////////////////////////////////////////////////
// pixels per inch
/////////////////////////////////////////////////////////////////////
int XGdiHelpers::getPixelsPerInchX()
{
    // check if we need to read this first
    if(g_gdiPixelsPerInchX == 0)
    {
        g_gdiPixelsPerInchX = getGlobalDeviceCaps(LOGPIXELSX);
    }
    
    return g_gdiPixelsPerInchX;
}

int XGdiHelpers::getPixelsPerInchY()
{
    // check if we need to read this first
    if(g_gdiPixelsPerInchY == 0)
    {
        g_gdiPixelsPerInchY = getGlobalDeviceCaps(LOGPIXELSY);
    }
    
    return g_gdiPixelsPerInchY;
}

/////////////////////////////////////////////////////////////////////
// convert dips to pixels
/////////////////////////////////////////////////////////////////////
int XGdiHelpers::dpiScalePixelsX(int pix)
{
    return (int)((float)(pix * getPixelsPerInchX()) / 96.f);
}

int XGdiHelpers::dpiScalePixelsY(int pix)
{
    return (int)((float)(pix * getPixelsPerInchY()) / 96.f);
}

int XGdiHelpers::dpiDipsToPixelsX(int dips)
{
    return (int)((float)(dips * 96.f) / getPixelsPerInchX());
}

int XGdiHelpers::dpiDipsToPixelsY(int dips)
{
    return (int)((float)(dips * 96.f) / getPixelsPerInchY());
}

/////////////////////////////////////////////////////////////////////
// twips
/////////////////////////////////////////////////////////////////////
LONG XGdiHelpers::pixelsToTwipsX(int pix)
{
    return (LONG)::MulDiv(pix, 1440, getPixelsPerInchX());
}

LONG XGdiHelpers::pixelsToTwipsY(int pix)
{
    return (LONG)::MulDiv(pix, 1440, getPixelsPerInchY());
}

int XGdiHelpers::twipsToPixelsX(LONG twips)
{
    return (int)(((double)twips) * (1.0 / 1440.0) * getPixelsPerInchX());
}

int XGdiHelpers::twipsToPixelsY(LONG twips)
{
    return (int)(((double)twips) * (1.0 / 1440.0) * getPixelsPerInchY());
}

/////////////////////////////////////////////////////////////////////
// points (1/72 of an inch)
/////////////////////////////////////////////////////////////////////
float XGdiHelpers::pixelsToPointsX(int pix)
{
    return ((float)pix * 72.0f) / getPixelsPerInchX();
}

float XGdiHelpers::pixelsToPointsY(int pix)
{
    return ((float)pix * 72.0f) / getPixelsPerInchY();
}

int XGdiHelpers::pointsToPixelsX(float points)
{
    return (int)((points * getPixelsPerInchX()) / 72.0f);
}

int XGdiHelpers::pointsToPixelsY(float points)
{
    return (int)((points * getPixelsPerInchY()) / 72.0f);
}

/////////////////////////////////////////////////////////////////////
// points to twips
/////////////////////////////////////////////////////////////////////
LONG XGdiHelpers::pointsToTwips(float points)
{
    return (LONG)(points * 20.0f);
}

float XGdiHelpers::twipsToPoints(LONG twips)
{
    return (float)twips / 20.0f;
}

/////////////////////////////////////////////////////////////////////
// HIMETRIC conversion
/////////////////////////////////////////////////////////////////////

// NOTE: 2540 is HIMETRIC units per inch 

LONG XGdiHelpers::pixToHimetricX(LONG x)
{
    // convert
    return (LONG)::MulDiv(x, 2540, XGdiHelpers::getPixelsPerInchX());
}

LONG XGdiHelpers::pixToHimetricY(LONG y)
{
    // convert
    return (LONG)::MulDiv(y, 2540, XGdiHelpers::getPixelsPerInchY());
}

/////////////////////////////////////////////////////////////////////
// drawing
/////////////////////////////////////////////////////////////////////
void XGdiHelpers::fillRect(HDC hdc, const RECT& rcPaint, COLORREF fillColor)
{
    XWASSERT(hdc);

    // NOTE: when window is first time painted trick with ExtTextOut to fill rectangle 
    //       doesn't always work for some reason, so using PatBlt here instead
    //
    //       COLORREF oldcr = ::SetBkColor(hdc, fillColor);
    //       ::ExtTextOutW(hdc, rcPaint.left, rcPaint.top, ETO_OPAQUE | ETO_CLIPPED, &rcPaint, L"", 0, 0);
    //       ::SetBkColor(hdc, oldcr);

    // init bursh
    HBRUSH fillBrush = ::CreateSolidBrush(fillColor);
    HGDIOBJ oldBrush = ::SelectObject(hdc, fillBrush);

    // fill rectangle
    ::PatBlt(hdc, rcPaint.left, rcPaint.top, (rcPaint.right - rcPaint.left), (rcPaint.bottom - rcPaint.top), PATCOPY);

    // set brush back
    ::SelectObject(hdc, oldBrush);
    ::DeleteObject(fillBrush);
}

void XGdiHelpers::fillRect(HDC hdc, const RECT& rcPaint, COLORREF fillColor, BYTE alpha)
{
    // check if transparency is needed
    if(alpha == 255)
    {
        // just fill using normal function
        XGdiHelpers::fillRect(hdc, rcPaint, fillColor);
        return;
    }

    // NOTE: we need to fill bitmap first and then AlphaBlend it to hdc

    // rect size
    int width = (rcPaint.right - rcPaint.left);
    int height = (rcPaint.bottom - rcPaint.top);

    DWORD* bitmapPixels = 0;
    size_t pixelCount = width * height; // returned image has 32bits color depth

    // create bitmap
    HBITMAP fillBitmap = createBitmap(hdc, width, height, (void**)&bitmapPixels);
    if(fillBitmap == 0 || bitmapPixels == 0)
    {
        XWTRACE_WERR_LAST("XWUtils::fillRect failed to create fill bitmap");

        if(fillBitmap)::DeleteObject(fillBitmap);

        // fill using normal function
        XGdiHelpers::fillRect(hdc, rcPaint, fillColor);
        return;
    }

    // NOTE: AlphaBlend assumes premultiplied colors
    fillBitmapPixels(bitmapPixels, pixelCount, fillColor, alpha);

    // get dc
    HDC hdcMem = ::CreateCompatibleDC(hdc);
    if(hdcMem)
    {
        // select bitmap into the memory DC
        HGDIOBJ hbmOld = ::SelectObject(hdcMem, fillBitmap);

        BLENDFUNCTION bf;
        bf.BlendOp = AC_SRC_OVER;
        bf.BlendFlags = 0;
        bf.SourceConstantAlpha = 255; 
        bf.AlphaFormat = AC_SRC_ALPHA;  

        // copy filled bitmap into destination DC
        ::AlphaBlend(hdc, 
            rcPaint.left, rcPaint.top,
            width, height,
            hdcMem,
            0, 0,
            width, height, 
            bf);

        // restore the memory DC
        if(hbmOld)
            ::SelectObject(hdcMem, hbmOld);

        // delete memory DC
        ::DeleteDC(hdcMem);

    } else
    {
        // fill using normal function
        XGdiHelpers::fillRect(hdc, rcPaint, fillColor);
    }

    // delete bitmap buffer
    ::DeleteObject(fillBitmap);
}

void XGdiHelpers::fillBitmapPixels(DWORD* bitmapPixels, size_t pixelCount, COLORREF fillColor, BYTE alpha)
{
    // alpha factor
    float alphaFactor = (float)alpha / (float)0xff; 

    // NOTE: fill premultiplied colors
    for(size_t idx = 0; idx < pixelCount; ++idx)
    {
        // pixel value
        bitmapPixels[idx] = (alpha << 24) |                                 // 0xaa000000 
                    ((UCHAR)(GetRValue(fillColor) * alphaFactor) << 16) |   // 0x00rr0000 
                    ((UCHAR)(GetGValue(fillColor) * alphaFactor) << 8)  |   // 0x0000gg00 
                    ((UCHAR)(GetBValue(fillColor) * alphaFactor));          // 0x000000bb 
    }
}

void XGdiHelpers::roundRectGradient(HDC hdc, const RECT& rect, int radius, 
                                  COLORREF beginColor, COLORREF endColor)
{
    // TODO: this code doesn't work properly, bottom right corner is not rounded

    // draw round rect as a path first
    ::BeginPath(hdc);
    ::RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);
    ::EndPath(hdc);
    ::SelectClipPath(hdc, RGN_COPY);

    // prepare gradient
    TRIVERTEX vertices[2];

    vertices[0].x = rect.left;
    vertices[0].y = rect.top;
    vertices[0].Red = GetRValue(beginColor) << 8;
    vertices[0].Green = GetGValue(beginColor) << 8;
    vertices[0].Blue = GetBValue(beginColor) << 8;
    vertices[0].Alpha = 0xffff;

    vertices[1].x = rect.right;
    vertices[1].y = rect.bottom;
    vertices[1].Red = GetRValue(endColor) << 8;
    vertices[1].Green = GetGValue(endColor) << 8;
    vertices[1].Blue = GetBValue(endColor) << 8;
    vertices[1].Alpha = 0xffff;

    GRADIENT_RECT grRect;
    grRect.UpperLeft = 0;
    grRect.LowerRight = 1;

    // fill
    ::GdiGradientFill(hdc, vertices, 2, &grRect, 1, GRADIENT_FILL_RECT_V);
}

/////////////////////////////////////////////////////////////////////
// text sizes
/////////////////////////////////////////////////////////////////////
void XGdiHelpers::getTextWidth(HDC hdc, HFONT font, const wchar_t * text, LONG& width)
{
    LONG height;

    // just ignore height
    getTextSize(hdc, font, text, width, height);
}

void XGdiHelpers::getTextSize(HDC hdc, HFONT font, const wchar_t * text, LONG& width, LONG& height)
{
    // reset output
    width = 0;
    height = 0;

    // check input
    XWASSERT(text);
    XWASSERT(hdc);
    if(text == 0 || hdc == 0) return;

    SIZE size;

    // select font
    HGDIOBJ oldFont = ::SelectObject(hdc, font);

    // compute size
    if(!::GetTextExtentPoint32W(hdc, text, (int)_tcslen(text), &size) != 0)
    {
        XWTRACE_WERR_LAST("XWUtils::getTextSize failed to compute text extent");
        return;
    }

    // restore the original font
    ::SelectObject(hdc, oldFont);

    // convert size
    width = dpiScalePixelsX(size.cx);
    height = dpiScalePixelsY(size.cy);
}

void XGdiHelpers::getWindowTextWidth(HWND hwnd, HFONT font, const wchar_t * text, LONG& width)
{
    LONG height;

    // just ignore height
    getWindowTextSize(hwnd, font, text, width, height);
}

void XGdiHelpers::getWindowTextSize(HWND hwnd, HFONT font, const wchar_t * text, LONG& width, LONG& height)
{
    // get window DC
    HDC hdc = ::GetDC(hwnd);

    // get size
    getTextSize(hdc, font, text, width, height);

    // release device context
    ::ReleaseDC(hwnd, hdc); 
}

// XGdiHelpers
/////////////////////////////////////////////////////////////////////
