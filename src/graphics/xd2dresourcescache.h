// Direct2D resource caching
//
/////////////////////////////////////////////////////////////////////

#ifndef _XD2DRESOURCESCACHE_H_
#define _XD2DRESOURCESCACHE_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
class XMediaSource;

/////////////////////////////////////////////////////////////////////

// NOTE: How to use Direct2D cache with images
//          - "load" or "set" bitmap to cache
//          - if item already exists it will not be loaded (only ref count increased)
//          - in paint method use getBitmap to get actual bitmap
//          - when image is no longer needed "release" it (it will be freed if ref count is zero)

// NOTE: No need to release or add reference to ID2D1Bitmap returned by getBitmap

/////////////////////////////////////////////////////////////////////
// XD2DResourcesCache - Direct2D cache

class XD2DResourcesCache : public IUnknown
{
public: // construction/destruction
    XD2DResourcesCache();
    ~XD2DResourcesCache();

public: // interface
    void    init(ID2D1RenderTarget* pD2DRenderTarget);
    void    reset();

public: // render target
    ID2D1RenderTarget*  renderTarget() const { return m_pRenderTarget;}

public: // use cached bitmaps, does not increase reference count (NOTE: call ID2D1Bitmap::Release after using)
    ID2D1Bitmap*    getBitmap(const std::wstring& bitmapHash);

public: // loading bitmaps (use getBitmap to get actual bitmap)
    bool    loadBitmap(const XMediaSource& source, std::wstring& hashOut);
    bool    loadBitmap(const XMediaSource& source, int width, int height, std::wstring& hashOut);

public: // add/replace bitmaps in cache 
    void    setBitmap(const std::wstring& bitmapHash, ID2D1Bitmap* pD2DBitmap);

public: // reserve cached bitmap (increase reference count)
    bool    reserveBitmap(const std::wstring& bitmapHash);

public: // release bitmap from cache (reference count decreased)
    void    releaseBitmap(std::wstring& bitmapHash);

public: // cache brushes (NOTE: use ID2D1Brush::Release to release cached brush)
    ID2D1Brush*     getBrush(const D2D1_COLOR_F& color);

public: // IUnknown 
    STDMETHODIMP            QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

private: // protect from copy and assignment
    XD2DResourcesCache(const XD2DResourcesCache& ref)  {}
    const XD2DResourcesCache& operator=(const XD2DResourcesCache& ref) { return *this;}

private: // types 
    struct XD2DBitmapCacheItem
    {
        ID2D1Bitmap*    bitmap;
        long            refCount;

        XD2DBitmapCacheItem() : bitmap(0), refCount(0) {}
        XD2DBitmapCacheItem(ID2D1Bitmap* bt) : bitmap(bt), refCount(0) {}
    };

    // bitmap cache
    typedef std::map<std::wstring, XD2DBitmapCacheItem>     XD2DBitmapChache;
    
    // color brush cache
    struct XD2DColorBrushRef
    {
        ID2D1Brush*     brush;
        D2D1_COLOR_F    color;
    };

    // brush cache
    typedef std::vector<XD2DColorBrushRef>          XD2DBrushChache;

private: // data
    unsigned long       m_ulRef;
    ID2D1RenderTarget*  m_pRenderTarget;
    XD2DBitmapChache    m_d2dBitmapCache;
    XD2DBrushChache     m_d2dBrushCache;
};

// XD2DResourcesCache
/////////////////////////////////////////////////////////////////////

#endif // _XD2DRESOURCESCACHE_H_

