// Text inline object
//
/////////////////////////////////////////////////////////////////////

#ifndef _XTEXTINLINEOBJECT_H_
#define _XTEXTINLINEOBJECT_H_

/////////////////////////////////////////////////////////////////////
// direct write headers
#include <dwrite.h>

/////////////////////////////////////////////////////////////////////
// forward declarations
struct ID2D1RenderTarget;
struct ID2D1GdiInteropRenderTarget;
class XGdiResourcesCache;
class XD2DResourcesCache;

/////////////////////////////////////////////////////////////////////
// XTextInlineObject - basic inline object functionality and interface

class XTextInlineObject : public IDWriteInlineObject,
                          public IXWContentProviderCallback
{
public: // construction/destruction
    XTextInlineObject();
    virtual ~XTextInlineObject();

public: // properties
    virtual int     width() const = 0;
    virtual int     height() const = 0;
    virtual int     baseline() const;

public: // shape object based on text style
    virtual void    shapeContentGDI(HDC hdc, int fontHeight, int fontAscent);
    virtual void    shapeContentD2D(FLOAT fontHeight, FLOAT fontAscent);

public: // position object
    virtual void    setOrigin(int originX, int originY);
    virtual void    moveOrigin(int offsetX, int offsetY);

public: // animation
    virtual void    enableAnimation(HWND parentWindow, bool enable);
    virtual void    pauseAnimation();
    virtual void    resumeAnimation();

public: // GDI resource caching 
    virtual void    onInitGDIResources(HDC hdc);
    virtual void    onResetGDIResources();
    virtual void    setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache);

public: // GDI painting
    virtual void    onPaintGDI(int posX, int posY, HDC hdc);

public: // Direct2D resource caching
    virtual void    onInitD2DTarget(ID2D1RenderTarget* pTarget);
    virtual void    onResetD2DTarget();
    virtual void    setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache);

public: // Direct2D painting
    virtual void    onPaintD2D(FLOAT posX, FLOAT posY, ID2D1RenderTarget* pTarget); 

public: // convert to formatted text (if possible)
    virtual void    toFormattedText(std::wstring& text);

public: // create OLE object 
    virtual HRESULT createOleObject(IOleClientSite* oleClientSite, IStorage* storage, IOleObject** objectOut);

public: // IUnknown 
    STDMETHODIMP            QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

public: // IDWriteInlineObject 
    STDMETHODIMP Draw(void* clientDrawingContext,
                      IDWriteTextRenderer* renderer,
                      FLOAT originX,
                      FLOAT originY,
                      BOOL isSideways,
                      BOOL isRightToLeft,
                      IUnknown* clientDrawingEffect);

    STDMETHODIMP GetMetrics(DWRITE_INLINE_OBJECT_METRICS* metrics);
    STDMETHODIMP GetOverhangMetrics(DWRITE_OVERHANG_METRICS* overhangs);
    STDMETHODIMP GetBreakConditions(DWRITE_BREAK_CONDITION* breakConditionBefore, DWRITE_BREAK_CONDITION* breakConditionAfter);

public: // IXWContentProviderCallback
    virtual void    onUrlContentLoaded(DWORD id, const WCHAR* path);
    virtual void    onUrlContentLoadFailed(DWORD id, DWORD reason);

protected: // helper methods
    void    onPaintD2DFromGDI(FLOAT posX, FLOAT posY, ID2D1RenderTarget* pTarget);

protected: // caching
    XGdiResourcesCache* m_pXGdiResourcesCache;
    XD2DResourcesCache* m_pXD2DResourcesCache;

private: // data
    unsigned long                   m_ulRef;
    ID2D1GdiInteropRenderTarget*    m_pGDIRenderTarget;
};

// XTextInlineObject
/////////////////////////////////////////////////////////////////////

#endif // _XTEXTINLINEOBJECT_H_

