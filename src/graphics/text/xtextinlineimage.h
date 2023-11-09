// Text inline image 
//
/////////////////////////////////////////////////////////////////////

#ifndef _XTEXTINLINEIMAGE_H_
#define _XTEXTINLINEIMAGE_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
struct ID2D1RenderTarget;
struct ID2D1Bitmap;
struct IWICBitmap;
class XGdiResourcesCache;
class XD2DResourcesCache;

/////////////////////////////////////////////////////////////////////
// XTextInlineImage - text inline image 

class XTextInlineImage : public XTextInlineObject,
                         public IXWAnimationTimerCallback
{
public: // construction/destruction
    XTextInlineImage();
    ~XTextInlineImage();

public: // set image 
    bool    setImage(IWICBitmap* wicBitmap);
    bool    setImage(HBITMAP hBitmap);              // NOTE: image doesn't take HBITMAP ownership
    bool    setImagePath(const WCHAR* filePath);
    bool    setImageResourcePath(HMODULE hModule, const WCHAR* szResName, const WCHAR* szResType);
    bool    setImageUri(const wchar_t* imageUri, int width, int height);
    void    resetImage();

public: // properties
    void    setFitToText(bool fitToText);
    void    setFixedSize(int width, int height, bool keepAspectRatio = false);

public: // properties (from XTextInlineObject)
    int     width() const;
    int     height() const;
    int     baseline() const;

public: // shape object based on text style (from XTextInlineObject)
    void    shapeContentGDI(HDC hdc, int fontHeight, int fontAscent);
    void    shapeContentD2D(FLOAT fontHeight, FLOAT fontAscent);

public: // position object (from XTextInlineObject)
    void    setOrigin(int originX, int originY);
    void    moveOrigin(int offsetX, int offsetY);

public: // animation (from XTextInlineObject)
    void    enableAnimation(HWND parentWindow, bool enable);
    void    pauseAnimation();
    void    resumeAnimation();

public: // content loading (from IXWContentProviderCallback)
    void    onUrlContentLoaded(DWORD id, const WCHAR* path);
    void    onUrlContentLoadFailed(DWORD id, DWORD reason);

public: // GDI resource caching 
    void    onInitGDIResources(HDC hdc);
    void    onResetGDIResources();
    void    setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache);

public: // GDI painting (from XTextInlineObject)
    void    onPaintGDI(int posX, int posY, HDC hdc);

public: // Direct2D resource caching (from XTextInlineObject)
    void    onInitD2DTarget(ID2D1RenderTarget* pTarget);
    void    onResetD2DTarget();
    void    setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache);

public: // Direct2D painting (from XTextInlineObject)
    void    onPaintD2D(FLOAT posX, FLOAT posY, ID2D1RenderTarget* pTarget); 

public: // convert to formatted text 
    void    toFormattedText(std::wstring& text);

public: // create OLE object 
    HRESULT createOleObject(IOleClientSite* oleClientSite, IStorage* storage, IOleObject** objectOut);

private: // events
    void    onAnimationTimer(DWORD id);

private: // worker methods
    void    _startAnimation();
    void    _stopAnimation();
    bool    _startNextFrameTimer();
    void    _cancelContentLoad();

private: // bitmap data
    XImage          m_itemImage;
    bool            m_fitToText;
    int             m_baseline;

private: // animation data
    bool            m_runAnimation;
    HWND            m_hwndParent;
    DWORD           m_timerId;
    DWORD           m_contentId;
    int             m_frameDelay;
    int             m_originX;
    int             m_originY;
};

// XTextInlineImage
/////////////////////////////////////////////////////////////////////

#endif // _XTEXTINLINEIMAGE_H_

