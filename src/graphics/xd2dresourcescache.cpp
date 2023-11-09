// Direct2D resource caching
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "ximagefile.h"
#include "ximagefilehelpers.h"
#include "xwichelpers.h"
#include "xd2dhelpres.h"
#include "xd2dresourcescache.h"

/////////////////////////////////////////////////////////////////////
// XD2DResourcesCache - Direct2D cache

XD2DResourcesCache::XD2DResourcesCache() :
    m_ulRef(0),
    m_pRenderTarget(0)
{
}

XD2DResourcesCache::~XD2DResourcesCache()
{
    // reset cache
    reset();
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XD2DResourcesCache::init(ID2D1RenderTarget* pD2DRenderTarget)
{
    XWASSERT(pD2DRenderTarget);
    if(pD2DRenderTarget == 0) return;

    // reset previous data if any
    reset();

    // copy target reference
    m_pRenderTarget = pD2DRenderTarget;
    m_pRenderTarget->AddRef();
}

void XD2DResourcesCache::reset()
{    
    XWASSERT1(m_d2dBitmapCache.size() == 0, "XD2DResourcesCache: trying to close cache while some bitmaps are not released");

    // free resources
    for(XD2DBitmapChache::iterator it = m_d2dBitmapCache.begin(); it != m_d2dBitmapCache.end(); ++it)
    {
        // release bitmap
        it->second.bitmap->Release();
    }
    m_d2dBitmapCache.clear();

    // free resources
    for(XD2DBrushChache::iterator it = m_d2dBrushCache.begin(); it != m_d2dBrushCache.end(); ++it)
    {
        // release brush
        it->brush->Release();
    }
    m_d2dBrushCache.clear();

    // release render target if any
    if(m_pRenderTarget)
    {
        m_pRenderTarget->Release();
        m_pRenderTarget = 0;
    }
}

/////////////////////////////////////////////////////////////////////
// use cached bitmaps 
/////////////////////////////////////////////////////////////////////
ID2D1Bitmap* XD2DResourcesCache::getBitmap(const std::wstring& bitmapHash)
{
    // check if we have this bitmap
    XD2DBitmapChache::iterator it = m_d2dBitmapCache.find(bitmapHash);
    if(it != m_d2dBitmapCache.end())
    {
        // add extra reference to bitmap
        it->second.bitmap->AddRef();

        // return cached bitmap
        return it->second.bitmap;
    }

    // not found
    return 0;
}

/////////////////////////////////////////////////////////////////////
// loading bitmaps (use getBitmap to get actual bitmap)
/////////////////////////////////////////////////////////////////////
bool XD2DResourcesCache::loadBitmap(const XMediaSource& source, std::wstring& hashOut)
{
    // use scaling version
    return loadBitmap(source, 0, 0, hashOut);
}

bool XD2DResourcesCache::loadBitmap(const XMediaSource& source, int width, int height, std::wstring& hashOut)
{
    // check if scaling is needed
    bool bNeedsScaling = (width > 0 && height > 0);

    // format hash first
    if(bNeedsScaling)
    {
        XImageFileHelpers::formatImageBitmapHash(source, width, height, hashOut);

    } else
    {
        XImageFileHelpers::formatImageBitmapHash(source, hashOut);
    }

    // check if we have this in cashe already
    XD2DBitmapChache::iterator it = m_d2dBitmapCache.find(hashOut);
    if(it != m_d2dBitmapCache.end()) 
    {
        // increase count
        it->second.refCount++;

        return true;
    }

    // open image
    XImageFile image;
    if(!image.open(source))
    {
        XWTRACE("XD2DResourcesCache: failed to load bitmap");
        return false;
    }

    // scale if needed
    if(bNeedsScaling)
    {
        if(!image.scale(width, height))
        {
            XWTRACE("XD2DResourcesCache: failed to scale bitmap");
            image.close();
            return false;
        }
    }

    // create bitmap
    ID2D1Bitmap* bitmap = XD2DHelpers::createBitmap(&image, m_pRenderTarget);

    // close file
    image.close();

    // ignore if failed
    if(bitmap == 0)
    {
        XWTRACE("XD2DResourcesCache: failed to create bitmap");
        return false;
    }

    // add to cache
    setBitmap(hashOut, bitmap);

    // remove extra reference
    bitmap->Release();

    return true;
}

/////////////////////////////////////////////////////////////////////
// add/replace bitmaps in cache 
/////////////////////////////////////////////////////////////////////
void XD2DResourcesCache::setBitmap(const std::wstring& bitmapHash, ID2D1Bitmap* pD2DBitmap)
{
    XWASSERT(bitmapHash.length());
    XWASSERT(pD2DBitmap);
    if(bitmapHash.length() == 0 || pD2DBitmap == 0) return;

    // add extra reference (for cache itself)
    pD2DBitmap->AddRef();

    // check if we have this bitmap already
    XD2DBitmapChache::iterator it = m_d2dBitmapCache.find(bitmapHash);
    if(it == m_d2dBitmapCache.end())
    {
        // init reference count as one to keep one reference
        XD2DBitmapCacheItem item(pD2DBitmap);
        item.refCount = 1;

        // add bitmap to cache
        m_d2dBitmapCache.insert(XD2DBitmapChache::value_type(bitmapHash, item));

    } else 
    {
        // replace bitmap 
        it->second.bitmap->Release();
        it->second.bitmap = pD2DBitmap;

        // increase reference count
        it->second.refCount++;
    }
}

/////////////////////////////////////////////////////////////////////
// reserve cached bitmap (increase reference count)
/////////////////////////////////////////////////////////////////////
bool XD2DResourcesCache::reserveBitmap(const std::wstring& bitmapHash)
{
    // ignore if hash is empty
    if(bitmapHash.length() == 0) return false;

    // check if we have this bitmap
    XD2DBitmapChache::iterator it = m_d2dBitmapCache.find(bitmapHash);
    if(it != m_d2dBitmapCache.end())
    {
        // increase reference count
        it->second.refCount++;

        // found
        return true;
    }

    // not found
    return false;
}

/////////////////////////////////////////////////////////////////////
// release bitmap (reference count decreased)
/////////////////////////////////////////////////////////////////////
void XD2DResourcesCache::releaseBitmap(std::wstring& bitmapHash)
{
    // ignore if hash is empty
    if(bitmapHash.length() == 0) return;

    // check if we have this bitmap
    XD2DBitmapChache::iterator it = m_d2dBitmapCache.find(bitmapHash);
    if(it != m_d2dBitmapCache.end())
    {
        // reference count
        it->second.refCount--;

        // remove item if last reference removed
        if(it->second.refCount <= 0)
        {
            it->second.bitmap->Release();
            m_d2dBitmapCache.erase(it);
        }
    }

    // reset hash to avoid releasing twice
    bitmapHash.clear();
}

/////////////////////////////////////////////////////////////////////
// cache brushes
/////////////////////////////////////////////////////////////////////
ID2D1Brush* XD2DResourcesCache::getBrush(const D2D1_COLOR_F& color)
{
    // ignore if no render target set
    if(m_pRenderTarget == 0) return 0;

    // loop over all cache entries
    for(XD2DBrushChache::iterator it = m_d2dBrushCache.begin(); it != m_d2dBrushCache.end(); ++it)
    {
        // check if color matches
        if(it->color.r == color.r && 
           it->color.g == color.g &&
           it->color.b == color.b &&
           it->color.a == color.a)
        {
            // add reference to bursh itself
            it->brush->AddRef();

            // cached brush
            return it->brush;
        }
    }

    ID2D1SolidColorBrush* brush = 0;

    // create brush
    HRESULT hr = m_pRenderTarget->CreateSolidColorBrush(color, &brush);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XD2DResourcesCache::getBrush failed to create brush", hr);
        return 0;
    }

    // add extra reference (for cache itself)
    brush->AddRef();

    // append entry
    XD2DColorBrushRef cacheEntry;
    cacheEntry.color = color;
    cacheEntry.brush = brush;

    // append
    m_d2dBrushCache.push_back(cacheEntry);

    return brush;
}

/////////////////////////////////////////////////////////////////////
// IUnknown 
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XD2DResourcesCache::QueryInterface(REFIID riid, void** ppvObject)
{
    // reset pointer first
    *ppvObject = 0;

    ///// check required interface

    // IUnknown
    if(riid == IID_IUnknown)
        *ppvObject = (IUnknown*)this;


    // check if interface is not supported
    if (!*ppvObject)
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) XD2DResourcesCache::AddRef()
{
    return ::InterlockedIncrement(&m_ulRef);
}

STDMETHODIMP_(ULONG) XD2DResourcesCache::Release()
{
    if(::InterlockedDecrement(&m_ulRef) == 0)
    {
        delete this;
        return 0;
    }

    return m_ulRef;
}

// XD2DResourcesCache
/////////////////////////////////////////////////////////////////////
