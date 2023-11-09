// Image functionality
//
/////////////////////////////////////////////////////////////////////

#ifndef _XIMAGE_H_
#define _XIMAGE_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
struct ID2D1RenderTarget;
struct ID2D1Bitmap;
struct IWICBitmap;
class XGdiResourcesCache;
class XD2DResourcesCache;

/////////////////////////////////////////////////////////////////////
// XImage - basic image functionality

class XImage
{
public: // construction/destruction
    XImage();
    ~XImage();
        
public: // set image
    bool    setImage(IWICBitmap* wicBitmap);
    bool    setImage(HBITMAP hBitmap);              // NOTE: image doesn't take HBITMAP ownership
    bool    setImagePath(const WCHAR* filePath);
    bool    setImageResourcePath(HMODULE hModule, const WCHAR* szResName, const WCHAR* szResType);
    bool    setImageStylePath(const WCHAR* stylePath);
    bool    setImageSource(const XMediaSource& source);
    void    resetImage();

public: // properties
    bool    isImageSet() const;
    void    setTransparency(BYTE transparency);
    void    setImageSize(int width, int height, bool keepAspectRatio = false);

public: // size
    int     width() const { return m_width; }
    int     height() const { return m_height; }

public: // animation
    bool    hasAnimation() const { return m_hasAnimation; }
    bool    isAnimationActive() const { return m_animationActive; }
    void    setDefaultDelay(int delay);
    bool    beginAnimation(HDC hdc);
    void    endAnimation();
    bool    getFrameDelay(int& delayOut);
    bool    loadNextFrame(HDC hdc);

public: // GDI painting 
    void    onInitGDIResources(HDC hdc);
    void    onResetGDIResources();
    void    onPaintGDI(int originX, int originY, HDC hdc);
    void    setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache);

public: // Direct2D painting 
    void    onInitD2DTarget(ID2D1RenderTarget* pTarget);
    void    onResetD2DTarget();
    void    onPaintD2D(FLOAT originX, FLOAT originY, ID2D1RenderTarget* pTarget); 
    void    setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache);

public: // image data 
    HBITMAP         getGDIPaintBitmap(HDC hdc);
    ID2D1Bitmap*    getD2DPaintBitmap(ID2D1RenderTarget* pTarget);

private: // protect from copy and assignment
    XImage(const XImage& ref)  {}
    const XImage& operator=(const XImage& ref) { return *this;}

private: // worker methods
    bool    _isFixedSize();
    void    _releaseGdiImage();
    void    _loadGdiImage(HDC hdc);
    void    _releaseD2DImage();
    bool    _d2dBitmapFromWicBitmap(IWICBitmap* wicBitmap, ID2D1RenderTarget* pTarget);
    void    _loadD2DImage(ID2D1RenderTarget* pTarget);
    bool    _loadImageSize();
    bool    _openImageFromFile(XImageFile& file);
    HBITMAP _loadImageFromFileGDI(HDC hdc);
    HBITMAP _loadCachedImageGDI();
    ID2D1Bitmap* _loadImageFromFileD2D(ID2D1RenderTarget* pTarget);
    ID2D1Bitmap* _loadCachedImageD2D();
    bool    _createFrameBitmap(HDC hdc, HBITMAP& bitmapOut, DWORD** pixelsOut);
    void    _resetPaintBitmaps();
    void    _resetAnimationBitmaps();
    void    _resetAnimation();
    void    _getFramePlacement(IWICBitmapSource* frameBitmapSource, int& frameLeft, int& frameTop, int& frameWidth, int& frameHeight);
    bool    _copyFramePixels();
    void    _loadGDIFrame(HDC hdc);
    void    _loadD2DFrame(ID2D1RenderTarget* pTarget);

private: // image data
    BYTE                m_bTransparency;
    bool                m_bKeepAspectRatio;
    IWICBitmap*         m_pWicBitmap;
    HBITMAP             m_hGdiBitmap;
    HBITMAP             m_hGdiLoadedBitmap;
    XMediaSource        m_imageSource;

protected: // caching
    XGdiResourcesCache* m_pXGdiResourcesCache;
    XD2DResourcesCache* m_pXD2DResourcesCache;

private: // animation data
    bool                m_hasAnimation;
    bool                m_animationActive;
    bool                m_hasBackground;
    COLORREF            m_frameBackground;
    BYTE                m_frameAlpha;
    XImageFile          m_inputFile;
    HBITMAP             m_gdiFrameBitmap;
    ID2D1Bitmap*        m_pD2DFrameBitmap;
    DWORD*              m_framePixels;
    std::vector<DWORD>  m_pixelBuffer;
    int                 m_defaultDelay;
    bool                m_useFrameDelay;

private: // data
    HBITMAP             m_hGdiPaintBitmap;
    ID2D1Bitmap*        m_pD2DBitmap;
    std::wstring        m_strImageHashGDI;
    std::wstring        m_strImageHashD2D;
    int                 m_width;
    int                 m_height;
    int                 m_originalWidth;
    int                 m_originalHeight;
    int                 m_fixedWidth;
    int                 m_fixedHeight;
};

// XImage
/////////////////////////////////////////////////////////////////////

#endif // _XIMAGE_H_

