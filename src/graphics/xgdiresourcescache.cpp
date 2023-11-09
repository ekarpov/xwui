// GDI resource caching
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "ximagefile.h"
#include "ximagefilehelpers.h"
#include "xgdiresourcescache.h"

/////////////////////////////////////////////////////////////////////
// XGdiResourcesCache - GDI cache

XGdiResourcesCache::XGdiResourcesCache() :
    m_ulRef(0),
    m_hCompatibleDC(0),
    m_hDoubleBufferDC(0),
    m_hDoubleBufferBitmap(0),
    m_hDoubleBufferOldBitmap(0),
    m_nBufferWidth(0),
    m_nBufferHeight(0)
{
}

XGdiResourcesCache::~XGdiResourcesCache()
{
    // close cache
    close();
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XGdiResourcesCache::init(HDC hDC)
{
    XWASSERT(hDC);
    if(hDC == 0) return;

    // close previous data if any
    close();

    // create compatible DC
    m_hCompatibleDC = ::CreateCompatibleDC(hDC);
}

void XGdiResourcesCache::close()
{
    // check if there are some bitmaps left
    if(m_gdiBitmapCache.size() > 0)
    {
        XWASSERT1(0, "XGdiResourcesCache: trying to close cache while some bitmaps are not released");

        // loop over all items
        for(XGdiBitmapChache::iterator it = m_gdiBitmapCache.begin(); it != m_gdiBitmapCache.end(); ++it)
        {
            ::DeleteObject(it->second.bitmap);
        }
        m_gdiBitmapCache.clear();
    }

    // close double buffer if any
    closeDoubleBufferDC();

    // release DC
    if(m_hCompatibleDC)
    {
        ::DeleteDC(m_hCompatibleDC);
        m_hCompatibleDC = 0;
    }
}

/////////////////////////////////////////////////////////////////////
// compatible DC for bitmap paint
/////////////////////////////////////////////////////////////////////
HDC XGdiResourcesCache::getCompatibleDC(HDC hDC)
{
    // create compatible DC if not in cache already
    if(m_hCompatibleDC == 0 && hDC)
    {
        m_hCompatibleDC = ::CreateCompatibleDC(hDC);
    }

    return m_hCompatibleDC;
}

/////////////////////////////////////////////////////////////////////
// window double buffering
/////////////////////////////////////////////////////////////////////
HDC XGdiResourcesCache::getDoubleBufferDC(HDC hdc, int width, int height)
{
    // check if size is different
    if(m_nBufferWidth != width || m_nBufferHeight != height)
    {
        // reset buffer
        closeDoubleBufferDC();
    }

    // create DC if needed
    if(m_hDoubleBufferDC == 0)
    {
        // create in-memory DC
        m_hDoubleBufferDC = ::CreateCompatibleDC(hdc);
        if(m_hDoubleBufferDC == 0) 
        {
            XWTRACE_WERR_LAST("XGdiResourcesCache: failed to create compatible DC for double buffering");
            return 0;
        }

        // create bitmap buffer
        m_hDoubleBufferBitmap = ::CreateCompatibleBitmap(hdc, width, height);
        if(m_hDoubleBufferDC == 0) 
        {
            XWTRACE_WERR_LAST("XGdiResourcesCache: failed to create compatible bitmap for double buffering");
            return 0;
        }

        // activate 
        m_hDoubleBufferOldBitmap = ::SelectObject(m_hDoubleBufferDC, m_hDoubleBufferBitmap);

        // copy size
        m_nBufferWidth = width;
        m_nBufferHeight = height;
    }

    return m_hDoubleBufferDC;
}

void XGdiResourcesCache::closeDoubleBufferDC()
{
    // release resources
    if(m_hDoubleBufferDC)
    {
        ::SelectObject(m_hDoubleBufferDC, m_hDoubleBufferOldBitmap);
        ::DeleteDC(m_hDoubleBufferDC);    

        m_hDoubleBufferDC = 0;
        m_hDoubleBufferOldBitmap = 0;
    }

    if(m_hDoubleBufferBitmap)
    {
        ::DeleteObject(m_hDoubleBufferBitmap);
        m_hDoubleBufferBitmap = 0;
    }
}

/////////////////////////////////////////////////////////////////////
// get cached bitmap
/////////////////////////////////////////////////////////////////////
HBITMAP XGdiResourcesCache::getBitmap(const std::wstring& bitmapHash)
{
    // check if we have this bitmap
    XGdiBitmapChache::iterator it = m_gdiBitmapCache.find(bitmapHash);
    if(it != m_gdiBitmapCache.end())
    {
        // return cached bitmap
        return it->second.bitmap;
    }

    // not found
    return 0;
}

/////////////////////////////////////////////////////////////////////
// loading bitmaps
/////////////////////////////////////////////////////////////////////
bool XGdiResourcesCache::loadBitmap(const XMediaSource& source, std::wstring& hashOut)
{
    // use scaling version
    return loadBitmap(source, 0, 0, hashOut);
}

bool XGdiResourcesCache::loadBitmap(const XMediaSource& source, int width, int height, std::wstring& hashOut)
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
    XGdiBitmapChache::iterator it = m_gdiBitmapCache.find(hashOut);
    if(it != m_gdiBitmapCache.end()) 
    {
        // increase count
        it->second.refCount++;

        return true;
    }

    // open image
    XImageFile image;
    if(!image.open(source))
    {
        XWTRACE("XGdiResourcesCache: failed to load bitmap");
        return false;
    }

    // scale if needed
    if(bNeedsScaling)
    {
        if(!image.scale(width, height))
        {
            XWTRACE("XGdiResourcesCache: failed to scale bitmap");
            image.close();
            return false;
        }
    }

    // create bitmap (NOTE: always use pre-multiplied images)
    HBITMAP bitmap = image.createGdiBitmap(m_hCompatibleDC, true);

    // close file
    image.close();

    // ignore if failed
    if(bitmap == 0)
    {
        XWTRACE("XGdiResourcesCache: failed to create bitmap");
        return false;
    }

    // add to cache
    setBitmap(hashOut, bitmap);

    return true;
}

/////////////////////////////////////////////////////////////////////
// add/replace bitmaps in cache (cache takes ownership, reference count is increased by one)
/////////////////////////////////////////////////////////////////////
void XGdiResourcesCache::setBitmap(const std::wstring& bitmapHash, HBITMAP bitmap)
{
    XWASSERT(bitmapHash.length());
    XWASSERT(bitmap);
    if(bitmapHash.length() == 0 || bitmap == 0) return;

    // check if we have this bitmap already
    XGdiBitmapChache::iterator it = m_gdiBitmapCache.find(bitmapHash);
    if(it == m_gdiBitmapCache.end())
    {
        // init reference count as one to keep one reference
        XGdiBitmapCacheItem item(bitmap);
        item.refCount = 1;

        // add bitmap to cache
        m_gdiBitmapCache.insert(XGdiBitmapChache::value_type(bitmapHash, item));
    
    } else 
    {
        // replace bitmap 
        ::DeleteObject(it->second.bitmap);
        it->second.bitmap = bitmap;

        // increase reference count
        it->second.refCount++;
    }
}

/////////////////////////////////////////////////////////////////////
// reserve cached bitmap (increase reference count)
/////////////////////////////////////////////////////////////////////
bool XGdiResourcesCache::reserveBitmap(const std::wstring& bitmapHash)
{
    // ignore if hash is empty
    if(bitmapHash.length() == 0) return false;

    // check if we have this bitmap
    XGdiBitmapChache::iterator it = m_gdiBitmapCache.find(bitmapHash);
    if(it != m_gdiBitmapCache.end())
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
// release bitmap (reference counted)
/////////////////////////////////////////////////////////////////////
void XGdiResourcesCache::releaseBitmap(std::wstring& bitmapHash)
{
    // ignore if hash is empty
    if(bitmapHash.length() == 0) return;

    // check if we have this bitmap
    XGdiBitmapChache::iterator it = m_gdiBitmapCache.find(bitmapHash);
    if(it != m_gdiBitmapCache.end())
    {
        // reference count
        it->second.refCount--;

        // remove item if last reference removed
        if(it->second.refCount <= 0)
        {
            ::DeleteObject(it->second.bitmap);
            m_gdiBitmapCache.erase(it);
        }
    }

    // reset hash to avoid releasing twice
    bitmapHash.clear();
}

/////////////////////////////////////////////////////////////////////
// IUnknown 
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XGdiResourcesCache::QueryInterface(REFIID riid, void** ppvObject)
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

STDMETHODIMP_(ULONG) XGdiResourcesCache::AddRef()
{
    return ::InterlockedIncrement(&m_ulRef);
}

STDMETHODIMP_(ULONG) XGdiResourcesCache::Release()
{
    if(::InterlockedDecrement(&m_ulRef) == 0)
    {
        delete this;
        return 0;
    }

    return m_ulRef;
}

// XGdiResourcesCache
/////////////////////////////////////////////////////////////////////
