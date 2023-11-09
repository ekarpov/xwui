// Text inline object
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"

#include "../xwgraphicshelpers.h"
#include "../xd2dhelpres.h"
#include "../xd2dresourcescache.h"
#include "../xgdiresourcescache.h"

#include "xtextinlineobject.h"

/////////////////////////////////////////////////////////////////////
// XTextInlineObject - basic inline object functionality and interface

XTextInlineObject::XTextInlineObject() :
    m_pXGdiResourcesCache(0),
    m_pXD2DResourcesCache(0),
    m_ulRef(0),
    m_pGDIRenderTarget(0)
{
}

XTextInlineObject::~XTextInlineObject()
{
    // reset resources
    onResetGDIResources();
    onResetD2DTarget();

    // release caches if any
    if(m_pXGdiResourcesCache) 
        m_pXGdiResourcesCache->Release();
    if(m_pXD2DResourcesCache)
        m_pXD2DResourcesCache->Release();
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
int XTextInlineObject::baseline() const
{
    // use height by default
    return height();
}

/////////////////////////////////////////////////////////////////////
// shape object based on text style
/////////////////////////////////////////////////////////////////////
void XTextInlineObject::shapeContentGDI(HDC hdc, int fontHeight, int fontAscent)
{
    // do nothing in default implementation
}

void XTextInlineObject::shapeContentD2D(FLOAT fontHeight, FLOAT fontAscent)
{
    // do nothing in default implementation
}

/////////////////////////////////////////////////////////////////////
// position object 
/////////////////////////////////////////////////////////////////////
void XTextInlineObject::setOrigin(int originX, int originY)
{
    // do nothing in default implementation
}

void XTextInlineObject::moveOrigin(int offsetX, int offsetY)
{
    // do nothing in default implementation
}

/////////////////////////////////////////////////////////////////////
// animation
/////////////////////////////////////////////////////////////////////
void XTextInlineObject::enableAnimation(HWND parentWindow, bool enable)
{
    // do nothing in default implementation
}

void XTextInlineObject::pauseAnimation()
{
    // do nothing in default implementation
}

void XTextInlineObject::resumeAnimation()
{
    // do nothing in default implementation
}

/////////////////////////////////////////////////////////////////////
// GDI resource caching 
/////////////////////////////////////////////////////////////////////
void XTextInlineObject::onInitGDIResources(HDC hdc)
{
    // do nothing in default implementation
}

void XTextInlineObject::onResetGDIResources()
{
    // do nothing in default implementation
}

void XTextInlineObject::setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache)
{
    // release previous instance if any
    if(m_pXGdiResourcesCache)
    {
        // reset resources as they might be using cache
        onResetGDIResources();

        // release
        m_pXGdiResourcesCache->Release();
    }

    // copy reference
    m_pXGdiResourcesCache = pXGdiResourcesCache;
    if(m_pXGdiResourcesCache) 
        m_pXGdiResourcesCache->AddRef();
}

/////////////////////////////////////////////////////////////////////
// GDI painting
/////////////////////////////////////////////////////////////////////
void XTextInlineObject::onPaintGDI(int posX, int posY, HDC hdc)
{
    // do nothing in default implementation
}

/////////////////////////////////////////////////////////////////////
// Direct2D resource caching
/////////////////////////////////////////////////////////////////////
void XTextInlineObject::onInitD2DTarget(ID2D1RenderTarget* pTarget)
{
    // do nothing in default implementation
}

void XTextInlineObject::onResetD2DTarget()
{
    // release GDI render target if any
    if(m_pGDIRenderTarget) 
    { 
        m_pGDIRenderTarget->Release();
        m_pGDIRenderTarget = 0;
    }
}

void XTextInlineObject::setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache)
{
    // release previous instance if any
    if(m_pXD2DResourcesCache)
    {
        // reset previous resources
        onResetD2DTarget();

        // release cache
        m_pXD2DResourcesCache->Release();
    }

    // copy reference
    m_pXD2DResourcesCache = pXD2DResourcesCache;
    if(m_pXD2DResourcesCache) 
        m_pXD2DResourcesCache->AddRef();
}

/////////////////////////////////////////////////////////////////////
// Direct2D painting
/////////////////////////////////////////////////////////////////////
void XTextInlineObject::onPaintD2D(FLOAT posX, FLOAT posY, ID2D1RenderTarget* pTarget)
{
    // paint using GDI by default
    onPaintD2DFromGDI(posX, posY, pTarget);
}

/////////////////////////////////////////////////////////////////////
// convert to formatted text (if possible)
/////////////////////////////////////////////////////////////////////
void XTextInlineObject::toFormattedText(std::wstring& text)
{
    // do nothing in default implementation
}

/////////////////////////////////////////////////////////////////////
// create OLE object 
/////////////////////////////////////////////////////////////////////
HRESULT XTextInlineObject::createOleObject(IOleClientSite* oleClientSite, IStorage* storage, IOleObject** objectOut)
{
    // not implemented by default
    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////
// IUnknown 
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XTextInlineObject::QueryInterface(REFIID riid, void** ppvObject)
{
    // reset pointer first
    *ppvObject = 0;

    ///// check required interface

    // IUnknown
    if(riid == IID_IUnknown)
        *ppvObject = (IUnknown*)(IDWriteInlineObject*)this;

    // IDataObject
    else if(riid == IID_IDataObject)
        *ppvObject = (IDataObject*)this;

    // check if interface is not supported
    if (!*ppvObject)
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) XTextInlineObject::AddRef()
{
    return ::InterlockedIncrement(&m_ulRef);
}

STDMETHODIMP_(ULONG) XTextInlineObject::Release()
{
    if(::InterlockedDecrement(&m_ulRef) == 0)
    {
        delete this;
        return 0;
    }

    return m_ulRef;
}

/////////////////////////////////////////////////////////////////////
// IDWriteInlineObject 
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XTextInlineObject::Draw(void* clientDrawingContext,
                      IDWriteTextRenderer* renderer,
                      FLOAT originX,
                      FLOAT originY,
                      BOOL isSideways,
                      BOOL isRightToLeft,
                      IUnknown* clientDrawingEffect)
{
    // TODO: check if we need to store a copy of render target (copy from onInitD2DTarget)

    // get render target
    ID2D1RenderTarget* renderTarget = 0;
    HRESULT hr = renderer->QueryInterface(__uuidof(ID2D1RenderTarget), (void**)&renderTarget);
    
    // ignore if failed
    if (FAILED(hr)) return hr;

    // paint as usual
    onPaintD2D(originX, originY, renderTarget);

    // release render target
    renderTarget->Release();
    renderTarget = 0;

    return S_OK;
}

STDMETHODIMP XTextInlineObject::GetMetrics(DWRITE_INLINE_OBJECT_METRICS* metrics)
{
    // default implementation
    metrics->width = XD2DHelpers::pixelsToDipsX(width());
    metrics->height = XD2DHelpers::pixelsToDipsY(height());
    metrics->baseline = XD2DHelpers::pixelsToDipsY(baseline());
    metrics->supportsSideways = FALSE;

    return S_OK;
}

STDMETHODIMP XTextInlineObject::GetOverhangMetrics(DWRITE_OVERHANG_METRICS* overhangs)
{
    // default implementation
    overhangs->left      = 0;
    overhangs->top       = 0;
    overhangs->right     = 0;
    overhangs->bottom    = 0;

    return S_OK;
}

STDMETHODIMP XTextInlineObject::GetBreakConditions(DWRITE_BREAK_CONDITION* breakConditionBefore, DWRITE_BREAK_CONDITION* breakConditionAfter)
{
    // default implementation
    *breakConditionBefore = DWRITE_BREAK_CONDITION_NEUTRAL;
    *breakConditionAfter  = DWRITE_BREAK_CONDITION_NEUTRAL;

    return S_OK;
}

/////////////////////////////////////////////////////////////////////
// IXWContentProviderCallback
/////////////////////////////////////////////////////////////////////
void XTextInlineObject::onUrlContentLoaded(DWORD id, const WCHAR* path)
{
    // do nothing in default implementation
}

void XTextInlineObject::onUrlContentLoadFailed(DWORD id, DWORD reason)
{
    // do nothing in default implementation
}

/////////////////////////////////////////////////////////////////////
// helper methods
/////////////////////////////////////////////////////////////////////
void XTextInlineObject::onPaintD2DFromGDI(FLOAT posX, FLOAT posY, ID2D1RenderTarget* pTarget)
{
    // NOTE: create GDI target only if this method is called
    if(m_pGDIRenderTarget == 0)
    {
        // get GDI render target (NOTE: ignore return code)
        pTarget->QueryInterface(__uuidof(ID2D1GdiInteropRenderTarget), (void**)&m_pGDIRenderTarget);
    }

    // render using GDI
    if(m_pGDIRenderTarget) 
    { 
        // init render target
        HDC hdc = 0;
        HRESULT hr = m_pGDIRenderTarget->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hdc);

        // render item using GDI method
        if(SUCCEEDED(hr))
        {
            // render item 
            onPaintGDI(XD2DHelpers::dipsToPixelsX(posX), XD2DHelpers::dipsToPixelsY(posY), hdc);

            // reset render target
            m_pGDIRenderTarget->ReleaseDC(NULL);

        } else
        {
            XWTRACE_HRES("XTextInlineObject: failed to get DC for GDI compatible paint", hr);
        }
    }
}

// XTextInlineObject
/////////////////////////////////////////////////////////////////////

