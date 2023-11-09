// GDI resource caching
//
/////////////////////////////////////////////////////////////////////

#ifndef _XGDIRESOURCESCACHE_H_
#define _XGDIRESOURCESCACHE_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
class XMediaSource;

/////////////////////////////////////////////////////////////////////

// NOTE: How to use GDI cache with images
//          - "load" or "set" bitmap to cache
//          - if item already exists it will not be loaded (only ref count increased)
//          - in paint method use getBitmap to get actual bitmap
//          - when image is no longer needed "release" it (it will be freed if ref count is zero)

// NOTE: Do not store bitmaps returned by cache, they may get released

/////////////////////////////////////////////////////////////////////
// XGdiResourcesCache - GDI cache

class XGdiResourcesCache : public IUnknown
{
public: // construction/destruction
    XGdiResourcesCache();
    ~XGdiResourcesCache();

public: // interface
    void    init(HDC hDC);
    void    close();

public: // compatible DC for bitmap paint
    HDC     getCompatibleDC(HDC hDC);

public: // window double buffering
    HDC     getDoubleBufferDC(HDC hdc, int width, int height);
    void    closeDoubleBufferDC();

public: // get cached bitmap (does not increase reference count)
    HBITMAP getBitmap(const std::wstring& bitmapHash);

public: // loading bitmaps (use getBitmap and returned hash to get actual bitmap)
    bool    loadBitmap(const XMediaSource& source, std::wstring& hashOut);
    bool    loadBitmap(const XMediaSource& source, int width, int height, std::wstring& hashOut);

public: // add/replace bitmaps in cache (cache takes ownership, reference count is increased)
    void    setBitmap(const std::wstring& bitmapHash, HBITMAP bitmap);

public: // reserve cached bitmap (increase reference count)
    bool    reserveBitmap(const std::wstring& bitmapHash);

public: // release bitmap (reference count decreased)
    void    releaseBitmap(std::wstring& bitmapHash);

public: // IUnknown 
    STDMETHODIMP            QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

private: // protect from copy and assignment
    XGdiResourcesCache(const XGdiResourcesCache& ref)  {}
    const XGdiResourcesCache& operator=(const XGdiResourcesCache& ref) { return *this;}

private: // bitmap cache
    struct XGdiBitmapCacheItem
    {
        HBITMAP     bitmap;
        long        refCount;

        XGdiBitmapCacheItem() : bitmap(0), refCount(0) {}
        XGdiBitmapCacheItem(HBITMAP bt) : bitmap(bt), refCount(0) {}
    };

    typedef std::map<std::wstring, XGdiBitmapCacheItem> XGdiBitmapChache;

private: // data
    unsigned long       m_ulRef;
    HDC                 m_hCompatibleDC;
    XGdiBitmapChache    m_gdiBitmapCache;

private: // double buffering
    HDC                 m_hDoubleBufferDC;
    HBITMAP             m_hDoubleBufferBitmap;
    HGDIOBJ             m_hDoubleBufferOldBitmap;
    int                 m_nBufferWidth;
    int                 m_nBufferHeight;
};

// XGdiResourcesCache
/////////////////////////////////////////////////////////////////////

#endif // _XGDIRESOURCESCACHE_H_

