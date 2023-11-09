// Direct2D helper functions
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xwichelpers.h"
#include "xd2dhelpres.h"
#include "ximagefile.h"

/////////////////////////////////////////////////////////////////////

// NOTE: to get debug messages from Direct2D a debug layer has to be installed
//       http://msdn.microsoft.com/en-us/library/windows/desktop/ee794287(v=vs.85).aspx

// NOTE: Direct2D and High-DPI
//       https://msdn.microsoft.com/en-us/library/windows/desktop/dd756649(v=vs.85).aspx#what_is_a_dip_

/////////////////////////////////////////////////////////////////////
// static data

// DLL instance
HINSTANCE g_hDirect2DLibrary = 0;

// Direct2D factory
ID2D1Factory* g_pID2D1Factory = 0;

// export function
typedef HRESULT (WINAPI *D2D1CreateFactoryPtr)(D2D1_FACTORY_TYPE, REFIID,
    const D2D1_FACTORY_OPTIONS *,void **);

D2D1CreateFactoryPtr g_pD2D1CreateFactoryFunc;

// DPI scaling
FLOAT   g_d2dDpiScaleFactorX = 1.0f;
FLOAT   g_d2dDpiScaleFactorY = 1.0f;

/////////////////////////////////////////////////////////////////////
// XD2DHelpers - helper functions

/////////////////////////////////////////////////////////////////////
// dynamic loading
bool XD2DHelpers::isDirect2DLoaded()
{
    return (g_hDirect2DLibrary != 0 && g_pD2D1CreateFactoryFunc != 0 && g_pID2D1Factory != 0);
}

bool XD2DHelpers::loadDirect2DLibrary()
{
    // check if already loaded
    if(XD2DHelpers::isDirect2DLoaded()) return true;

    // load Direct2D DLL
    g_hDirect2DLibrary = ::LoadLibraryW(L"d2d1.dll");
    if(g_hDirect2DLibrary == 0)
    {
        XWTRACE_WERR_LAST("XD2DHelpers: Failed to load Direct2D library");
        return false;
    }

    // locate factory function
    g_pD2D1CreateFactoryFunc = (D2D1CreateFactoryPtr)::GetProcAddress(g_hDirect2DLibrary, "D2D1CreateFactory");
    if(g_pD2D1CreateFactoryFunc == 0)
    {
        XWTRACE_WERR_LAST("XD2DHelpers: Failed to get factory function from Direct2D library");
        ::FreeLibrary(g_hDirect2DLibrary);
        g_hDirect2DLibrary = 0;
        return false;
    }

    // factory options
    D2D1_FACTORY_OPTIONS fopt;
#ifdef _DEBUG
    fopt.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#else
    fopt.debugLevel = D2D1_DEBUG_LEVEL_NONE;
#endif // _DEBUG

    // create global factory
    HRESULT hr = g_pD2D1CreateFactoryFunc(D2D1_FACTORY_TYPE_SINGLE_THREADED,
        __uuidof(g_pID2D1Factory), &fopt, (void**)&g_pID2D1Factory);

    if(FAILED(hr))
    {
        XWTRACE_HRES("XD2DHelpers: Failed to create Direct2D factory", hr);
        XD2DHelpers::freeDirect2DLibrary();
        return false;
    }

    // init global DPI scale factors
    getDpiScaleFactors(g_d2dDpiScaleFactorX, g_d2dDpiScaleFactorY);

    return true;
}

void XD2DHelpers::freeDirect2DLibrary()
{
    // check if it has been loaded
    if(!XD2DHelpers::isDirect2DLoaded()) return;

    // release factory if any
    if(g_pID2D1Factory)
    {
        g_pID2D1Factory->Release();
        g_pID2D1Factory = 0;
    }

    // free library
    if(!::FreeLibrary(g_hDirect2DLibrary))
    {
        XWTRACE_WERR_LAST("XD2DHelpers: Failed to release Direct2D library");
    }

    // reset pointers
    g_hDirect2DLibrary = 0;
    g_pD2D1CreateFactoryFunc = 0;
}

/////////////////////////////////////////////////////////////////////
// global factory
ID2D1Factory* XD2DHelpers::getDirect2DFactory()
{
    return g_pID2D1Factory;
}

/////////////////////////////////////////////////////////////////////
// bitmaps
/////////////////////////////////////////////////////////////////////
ID2D1Bitmap* XD2DHelpers::createBitmap(IWICBitmapSource* pBitmapSource, ID2D1RenderTarget* pRenderTarget)
{
    // check input
    XWASSERT(pBitmapSource);
    XWASSERT(pRenderTarget);
    if(pBitmapSource == 0 || pRenderTarget == 0) return 0;

    IWICFormatConverter *pConverter = 0;

    // as MSDN says before Direct2D can use the image, it must be converted to the 32bppPBGRA pixel format
    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd756685(v=vs.85).aspx
    HRESULT res = XWicHelpers::getImagingFactory()->CreateFormatConverter(&pConverter);
    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XD2DHelpers: Failed to create format converter", res);
        return 0;
    }

    // initialize converter
    res = pConverter->Initialize(pBitmapSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone,
                                 NULL, 0.f, WICBitmapPaletteTypeMedianCut);
    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XD2DHelpers: Failed to convert image format", res);
        pConverter->Release();
        return 0;
    }

    ID2D1Bitmap* pD2DBitmap = 0;

    // create D2D bitmap from WIC bitmap
    res = pRenderTarget->CreateBitmapFromWicBitmap(pConverter, &pD2DBitmap);
    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XD2DHelpers: Failed to convert WIC bitmap to D2D bitmap", res);
    }

    // release resources
    pConverter->Release();

    return pD2DBitmap;
}

ID2D1Bitmap* XD2DHelpers::createSharedBitmap(ID2D1Bitmap* pOriginalImage, ID2D1RenderTarget* pRenderTarget)
{
    // check input
    XWASSERT(pOriginalImage);
    XWASSERT(pRenderTarget);
    if(pOriginalImage == 0 || pRenderTarget == 0) return 0;

    ID2D1Bitmap* pD2DBitmap = 0;

    // create shared bitmap
    HRESULT res = pRenderTarget->CreateSharedBitmap(__uuidof(ID2D1Bitmap), pOriginalImage, 0, &pD2DBitmap);
    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XD2DHelpers: Failed to create shared D2D bitmap", res);
    }

    return pD2DBitmap;
}

ID2D1Bitmap* XD2DHelpers::createBitmap(XImageFile* pImageFile, ID2D1RenderTarget* pRenderTarget)
{
    // check input
    XWASSERT(pImageFile);
    XWASSERT(pRenderTarget);
    if(pImageFile == 0 || pRenderTarget == 0) return 0;

    IWICBitmap* wicBitmap = pImageFile->createWicBitmap();
    if(wicBitmap == 0)
    {
        XWTRACE("XD2DHelpers: failed to create WIC bitmap from image file");
        return 0;
    }

    // create bitmap
    ID2D1Bitmap* bitmap = XD2DHelpers::createBitmap(wicBitmap, pRenderTarget);

    // release WIC bitmap
    wicBitmap->Release();

    return bitmap;
}

/////////////////////////////////////////////////////////////////////
// colors
/////////////////////////////////////////////////////////////////////
void XD2DHelpers::d2dColorToColorref(const D2D1_COLOR_F& d2dColor, COLORREF& color)
{
    color = RGB(d2dColor.r * 255, d2dColor.g * 255, d2dColor.b * 255);
}

void XD2DHelpers::colorrefToD2dColor(const COLORREF& color, D2D1_COLOR_F& d2dColor)
{
    d2dColor.r = GetRValue(color) / 255.0f;
    d2dColor.g = GetGValue(color) / 255.0f;
    d2dColor.b = GetBValue(color) / 255.0f;
    d2dColor.a = 1.0f;
}

/////////////////////////////////////////////////////////////////////
// coordinates
/////////////////////////////////////////////////////////////////////
void XD2DHelpers::getDpiScaleFactors(FLOAT& dpiScaleX, FLOAT& dpiScaleY)
{
    // reset scale in any case
    dpiScaleX = 1.0f;
    dpiScaleY = 1.0f;

    XWASSERT(g_pID2D1Factory);
    if(g_pID2D1Factory == 0) return;

    // get DPI settings first
    FLOAT dpiX, dpiY;
    g_pID2D1Factory->GetDesktopDpi(&dpiX, &dpiY);

    // scale factors
    dpiScaleX = dpiX/96.0f;
    dpiScaleY = dpiY/96.0f;
}

void XD2DHelpers::gdiRectToD2dRect(const RECT& rect, D2D1_RECT_F& d2dRect)
{
    // use cached values
    gdiRectToD2dRect(rect, d2dRect, g_d2dDpiScaleFactorX, g_d2dDpiScaleFactorY);
}

void XD2DHelpers::gdiRectToD2dRect(const RECT& rect, D2D1_RECT_F& d2dRect, FLOAT dpiScaleX, FLOAT dpiScaleY)
{
    // copy values
    d2dRect.left = pixelsToDips(rect.left, dpiScaleX);
    d2dRect.right = pixelsToDips(rect.right, dpiScaleX);
    d2dRect.top = pixelsToDips(rect.top, dpiScaleY);
    d2dRect.bottom = pixelsToDips(rect.bottom, dpiScaleY);
}

/////////////////////////////////////////////////////////////////////
// dpi scale factors
/////////////////////////////////////////////////////////////////////
FLOAT XD2DHelpers::getDpiScaleX()
{
    return g_d2dDpiScaleFactorX;
}

FLOAT XD2DHelpers::getDpiScaleY()
{
    return g_d2dDpiScaleFactorY;
}

// device update (update cache)
void XD2DHelpers::deviceCapsUpdated()
{
    // update global DPI scale factors
    getDpiScaleFactors(g_d2dDpiScaleFactorX, g_d2dDpiScaleFactorY);
}

/////////////////////////////////////////////////////////////////////
// brush cache helpers
/////////////////////////////////////////////////////////////////////
void XD2DHelpers::resetColorBrushCache(XD2DColorBrushCache& brushCache)
{
    // loop over all cache entries
    for(XD2DColorBrushCache::iterator it = brushCache.begin(); it != brushCache.end(); ++it)
    {
        // release brush
        it->brush->Release();
    }

    // reset cache
    brushCache.clear();
}

ID2D1Brush* XD2DHelpers::findColorBrush(const XD2DColorBrushCache& brushCache, const D2D1_COLOR_F& color)
{
    // loop over all cache entries
    for(XD2DColorBrushCache::const_iterator it = brushCache.begin(); it != brushCache.end(); ++it)
    {
        // check if color matches
        if(it->color.r == color.r && 
           it->color.g == color.g &&
           it->color.b == color.b &&
           it->color.a == color.a)
        {
            // brush found
            return it->brush;
        }
    }

    // not found
    return 0;
}

void XD2DHelpers::appendColorBrush(XD2DColorBrushCache& brushCache, ID2D1Brush* brush, const D2D1_COLOR_F& color)
{
    // check in debug if color is unique
    XWASSERT(findColorBrush(brushCache, color) == 0);

    // append entry
    XD2DColorBrushRef cacheEntry;
    cacheEntry.color = color;
    cacheEntry.brush = brush;

    // append
    brushCache.push_back(cacheEntry);
}

// XD2DHelpers
/////////////////////////////////////////////////////////////////////
