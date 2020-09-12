// Windowless OLE in-place activation helper
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xoleinplace.h"

/////////////////////////////////////////////////////////////////////
// XOleInplaceImpl - windowless OLE in-place implementation

XOleInplaceImpl::XOleInplaceImpl() :
    m_hwndContainer(0),
    m_ulRef(0),
    m_bWindowless(false),
    m_pOleObject(0),
    m_pOleInPlaceObject(0),
    m_pOleInPlaceObjectWindowless(0),
    m_pStorage(0)
{
}

XOleInplaceImpl::~XOleInplaceImpl()
{
    // release object if any
    releaseObject();
}

/////////////////////////////////////////////////////////////////////
// initialization
/////////////////////////////////////////////////////////////////////
bool XOleInplaceImpl::loadOleObject(const wchar_t* clsid)
{
    CLSID retClsid;

    // convert to CLSID
    HRESULT res = CLSIDFromString(clsid, &retClsid); 
    if(SUCCEEDED(res))
    {
        return loadOleObject(retClsid);
    }

    XWTRACE2("XOleInplaceImpl: Failed to convert CLSID %ws, HRESULT %X", clsid, res);
    return false;
}

bool XOleInplaceImpl::loadOleObject(const CLSID& clsid)
{
    // release previous object if any
    releaseObject();

    // create storage first
    if(!_createStorage(0)) return false;

    // load ActiveX
    HRESULT res = ::OleCreate(clsid, IID_IOleObject, OLERENDER_DRAW, 0 ,(IOleClientSite*)this, m_pStorage, (LPVOID*)&m_pOleObject);
    if(res != S_OK || m_pOleObject == 0)
    {
        XWTRACE_HRES("XOleInplaceImpl: Failed to create OLE object", res);
        return false;
    }

    // set as contained
    res = ::OleSetContainedObject(m_pOleObject, TRUE);
    if(FAILED(res))
    {
        XWTRACE_HRES("XOleInplaceImpl: Failed to set OLE object as contained", res);
        return false;
    }

    return true;
}

void XOleInplaceImpl::releaseObject()
{
    // release interfaces
    if(m_pOleInPlaceObjectWindowless) m_pOleInPlaceObjectWindowless->Release();
    if(m_pOleInPlaceObject) m_pOleInPlaceObject->Release();
    if(m_pOleObject) m_pOleObject->Release();

    // reset pointers
    m_pOleInPlaceObjectWindowless = 0;
    m_pOleInPlaceObject = 0;
    m_pOleObject = 0;

    // close storage
    _closeStorage(); 
}

bool XOleInplaceImpl::isOleObjectLoaded() const
{
    return (m_pOleObject != 0);
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
bool XOleInplaceImpl::activateInPlace(const RECT& rcClient, bool windowless)
{
    XWASSERT(m_pOleObject);
    if(!m_pOleObject) return false;

    // copy flag
    m_bWindowless = windowless;
    
    // get interfaces
    HRESULT res = m_pOleObject->QueryInterface(IID_IOleInPlaceObject, (LPVOID*)&m_pOleInPlaceObject);
    if(FAILED(res))
    {
        XWTRACE_HRES("XOleInplaceImpl: Failed to query IID_IOleInPlaceObject from OLE object", res);
        return false;
    }

    // get windowless interface if needed
    if(m_bWindowless)
    {
        res = m_pOleObject->QueryInterface(IID_IOleInPlaceObjectWindowless, (LPVOID*)&m_pOleInPlaceObjectWindowless);
        if(FAILED(res))
        {
            XWTRACE_HRES("XOleInplaceImpl: Failed to query IID_IOleInPlaceObjectWindowless from OLE object", res);

            // reset flag
            m_bWindowless = false;
        }
    }

    // activate control
    res = m_pOleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, 0, (IOleClientSite*)this, 0, m_hwndContainer, &rcClient);
    if(FAILED(res))
    {
        XWTRACE_HRES("XOleInplaceImpl: Failed to in-place activate OLE object", res);
        return false;
    }

    return true;
}

bool XOleInplaceImpl::update(const RECT& rcClient)
{
    XWASSERT(m_pOleInPlaceObject);
    if(!m_pOleInPlaceObject) return false;

    // update size
    HRESULT res = m_pOleInPlaceObject->SetObjectRects(&rcClient, &rcClient);
    if(FAILED(res))
    {
        XWTRACE_HRES("XOleInplaceImpl: Failed to SetObjectRects for OLE object", res);
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////
// get interface from loaded object
/////////////////////////////////////////////////////////////////////
bool XOleInplaceImpl::queryObjectInterface(REFIID riid, void** ppvObject)
{
    // reset pointer first
    *ppvObject = 0;

    // check if loaded
    if(!m_pOleObject) return false;

    // query interface
    HRESULT res = m_pOleObject->QueryInterface(riid, ppvObject);
    if(FAILED(res))
    {
        XWTRACE_HRES("XOleInplaceImpl: Failed to query inteface from OLE object", res);
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////
// message processing
/////////////////////////////////////////////////////////////////////
LRESULT XOleInplaceImpl::processWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& messageProcessed)
{
    // TODO:
    return 0;
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
void XOleInplaceImpl::setEnabled(bool bEnabled)
{
    // TODO:
}

bool XOleInplaceImpl::isEnabled() const
{
    // TODO:
    return true;
}

/////////////////////////////////////////////////////////////////////
// focus
/////////////////////////////////////////////////////////////////////
void XOleInplaceImpl::setFocus(bool bFocus)
{
    // TODO:
}

/////////////////////////////////////////////////////////////////////
// GDI painting
/////////////////////////////////////////////////////////////////////
void XOleInplaceImpl::onPaintGDI(HDC hdc, const RECT& rcPaint)
{
    // TODO:
}

/////////////////////////////////////////////////////////////////////
// IUnknown 
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XOleInplaceImpl::QueryInterface(REFIID riid, void** ppvObject)
{
    // reset pointer first
    *ppvObject = 0;

    ///// check required interface

    // IUnknown
    if(riid == IID_IUnknown)
        *ppvObject = (IUnknown*)(IOleInPlaceSiteWindowless*)this;

    // IOleWindow
    else if(riid == IID_IOleWindow)
        *ppvObject = (IOleWindow*)(IOleInPlaceSiteWindowless*)this;

    // IOleInPlaceSite
    else if(riid == IID_IOleInPlaceSite)
        *ppvObject = (IOleInPlaceSite*)(IOleInPlaceSiteWindowless*)this;

    // IOleInPlaceSiteEx
    else if(riid == IID_IOleInPlaceSiteEx)
        *ppvObject = (IOleInPlaceSiteEx*)(IOleInPlaceSiteWindowless*)this;

    // IOleInPlaceSiteWindowless
    else if(riid == IID_IOleInPlaceSiteWindowless)
        *ppvObject = (IOleInPlaceSiteWindowless*)this;

    // IOleClientSite
    else if(riid == IID_IOleClientSite)
        *ppvObject = (IOleClientSite*)this;

    // IOleInPlaceUIWindow
    else if(riid == IID_IOleInPlaceUIWindow)
        *ppvObject = (IOleInPlaceUIWindow*)(IOleInPlaceFrame*)this;

    // IOleInPlaceFrame
    else if(riid == IID_IOleInPlaceFrame)
        *ppvObject = (IOleInPlaceFrame*)this;

    // check if interface is not supported
    if (!*ppvObject)
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) XOleInplaceImpl::AddRef()
{
    return ::InterlockedIncrement(&m_ulRef);
}

STDMETHODIMP_(ULONG) XOleInplaceImpl::Release()
{
    if(::InterlockedDecrement(&m_ulRef) == 0)
    {
        delete this;
        return 0;
    }

    return m_ulRef;
}

/////////////////////////////////////////////////////////////////////
// IOleWindow
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XOleInplaceImpl::GetWindow(HWND *phwnd)
{
    // set window handle
    if(phwnd) 
    {
        *phwnd = m_hwndContainer;
        return S_OK;

    } else
    {
        return E_POINTER;
    }    
}

STDMETHODIMP XOleInplaceImpl::ContextSensitiveHelp(BOOL fEnterMode)
{
    // not needed
    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////
// IOleInPlaceSite
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XOleInplaceImpl::CanInPlaceActivate()
{
    return S_OK;
}

STDMETHODIMP XOleInplaceImpl::OnInPlaceActivate()
{
    return S_OK;
}

STDMETHODIMP XOleInplaceImpl::OnUIActivate()
{
    return S_OK;
}

STDMETHODIMP XOleInplaceImpl::GetWindowContext(IOleInPlaceFrame** ppFrame, 
                                            IOleInPlaceUIWindow** ppDoc, 
                                            LPRECT lprcPosRect, 
                                            LPRECT lprcClipRect, 
                                            LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    if (!ppFrame || !ppDoc || !lprcPosRect || !lprcClipRect || !lpFrameInfo)
        return E_POINTER;

    QueryInterface(IID_IOleInPlaceFrame, (void**)ppFrame);
    QueryInterface(IID_IOleInPlaceUIWindow, (void**)ppDoc);

    ::GetClientRect(m_hwndContainer, lprcPosRect);
    ::GetClientRect(m_hwndContainer, lprcClipRect);

    lpFrameInfo->cb = sizeof(OLEINPLACEFRAMEINFO);
    lpFrameInfo->fMDIApp = false;
    lpFrameInfo->haccel = 0;
    lpFrameInfo->cAccelEntries = 0;
    lpFrameInfo->hwndFrame = ::GetParent(m_hwndContainer);

    return S_OK;
}

STDMETHODIMP XOleInplaceImpl::Scroll(SIZE scrollExtant)
{
    return S_FALSE;
}

STDMETHODIMP XOleInplaceImpl::OnUIDeactivate(BOOL fUndoable)
{
    return S_OK;
}

STDMETHODIMP XOleInplaceImpl::OnInPlaceDeactivate()
{
    return S_OK;
}

STDMETHODIMP XOleInplaceImpl::DiscardUndoState()
{
    return S_OK;
}

STDMETHODIMP XOleInplaceImpl::DeactivateAndUndo()
{
    return S_OK;
}

STDMETHODIMP XOleInplaceImpl::OnPosRectChange(LPCRECT lprcPosRect)
{
    return S_OK;
}

/////////////////////////////////////////////////////////////////////
// IOleInPlaceSiteEx
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XOleInplaceImpl::OnInPlaceActivateEx(BOOL *pfNoRedraw, DWORD dwFlags)
{
    // TODO:
    return S_OK;
}

STDMETHODIMP XOleInplaceImpl::OnInPlaceDeactivateEx(BOOL fNoRedraw)
{
    // TODO:
    return S_OK;
}

STDMETHODIMP XOleInplaceImpl::RequestUIActivate()
{
    // TODO:
    return S_OK;
}

/////////////////////////////////////////////////////////////////////
// IOleInPlaceSiteWindowless
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XOleInplaceImpl::CanWindowlessActivate()
{
    // check if windowless mode is required
    if(m_bWindowless)
        return S_OK;
    else
        return S_FALSE;
}

STDMETHODIMP XOleInplaceImpl::GetCapture()
{
    // TODO:
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::SetCapture(BOOL fCapture)
{
    // TODO:
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::GetFocus()
{
    // TODO:
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::SetFocus(BOOL fFocus)
{
    // TODO:
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::GetDC(LPCRECT pRect, DWORD grfFlags, HDC *phDC)
{
    // TODO:
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::ReleaseDC(HDC hDC)
{
    // TODO:
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::InvalidateRect(LPCRECT pRect, BOOL fErase)
{
    // TODO:
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::InvalidateRgn(HRGN hRGN, BOOL fErase)
{
    // TODO:
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::ScrollRect(INT dx, INT dy, LPCRECT pRectScroll, LPCRECT pRectClip)
{
    // TODO:
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::AdjustRect(LPRECT prc)
{
    // TODO:
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::OnDefWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
{
    // TODO:
    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////
// IOleClientSite
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XOleInplaceImpl::SaveObject()
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::GetMoniker(DWORD /*dwAssign*/, DWORD /*dwWhichMoniker*/, IMoniker** ppmk)
{
    if (!ppmk)
        return E_POINTER;

    *ppmk = 0;
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::GetContainer(IOleContainer** ppContainer)
{
    if (!ppContainer)
        return E_POINTER;

    *ppContainer = 0;
    return E_NOINTERFACE;
}

STDMETHODIMP XOleInplaceImpl::ShowObject()
{
    return S_OK;
}

STDMETHODIMP XOleInplaceImpl::OnShowWindow(BOOL /*fShow*/)
{
    ::InvalidateRect(m_hwndContainer, 0, TRUE);
    ::InvalidateRect(::GetParent(m_hwndContainer), 0, TRUE);
    return S_OK;
}

STDMETHODIMP XOleInplaceImpl::RequestNewObjectLayout()
{
    // not needed
    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////
// IOleInPlaceUIWindow
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XOleInplaceImpl::GetBorder(LPRECT lprectBorder)
{
    if (!lprectBorder) 
        return E_POINTER;

    ::GetClientRect(m_hwndContainer, lprectBorder);
    return S_OK;
}

STDMETHODIMP XOleInplaceImpl::RequestBorderSpace(LPCBORDERWIDTHS /*pborderwidths*/)
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::SetBorderSpace(LPCBORDERWIDTHS /*pborderwidths*/)
{
    return S_OK;
}

STDMETHODIMP XOleInplaceImpl::SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR /*pszObjName*/)
{
    return S_OK;
}

/////////////////////////////////////////////////////////////////////
// IOleInPlaceFrame
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XOleInplaceImpl::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::RemoveMenus(HMENU hmenuShared)
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::SetStatusText(LPCOLESTR pszStatusText)
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::EnableModeless(BOOL fEnable)
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XOleInplaceImpl::TranslateAccelerator(LPMSG lpMsg, WORD grfModifiers)
{
    // not needed
    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////
// helper methods
/////////////////////////////////////////////////////////////////////
bool XOleInplaceImpl::_createStorage(const wchar_t* name)
{
    // close previous storage if any
    _closeStorage();

    // create storage
    HRESULT res = ::StgCreateStorageEx(name, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_CREATE, 
        STGFMT_STORAGE, 0, 0, 0, IID_IStorage, (LPVOID*)&m_pStorage);

    if(res != S_OK || m_pStorage == 0)
    {
        XWTRACE_HRES("XOleInplaceImpl: Failed to create storage", res);
        return false;
    }

    return true;
}

void XOleInplaceImpl::_closeStorage()
{
    if(m_pStorage) m_pStorage->Release();
    m_pStorage = 0;
}

// XOleInplaceImpl
/////////////////////////////////////////////////////////////////////

