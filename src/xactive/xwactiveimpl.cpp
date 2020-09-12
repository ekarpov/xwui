// ActiveX control internals
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include <DocObj.h>
#include "xwactiveimpl.h"

/////////////////////////////////////////////////////////////////////
// XWActiveImpl - ActiveX host internal implementation


XWActiveImpl::XWActiveImpl(HWND hwnd) :
    m_hwndContainer(hwnd),
    m_ulRef(0),
    m_bIOleInPlaceActiveObjectSupported(true),
    m_pOleObject(0),
    m_pOleInPlaceObject(0),
    m_pIOleInPlaceActiveObject(0),
    m_pStorage(0),
    m_pCustomInterfaces(0)
{
    XWASSERT(m_hwndContainer);
}

XWActiveImpl::~XWActiveImpl()
{
}

/////////////////////////////////////////////////////////////////////
// load object
/////////////////////////////////////////////////////////////////////
bool XWActiveImpl::loadActiveObject(const wchar_t* clsid)
{
    CLSID retClsid;

    // convert to CLSID
    HRESULT res = CLSIDFromString(clsid, &retClsid); 
    if(SUCCEEDED(res))
    {
        return loadActiveObject(retClsid);
    }

    XWTRACE2("Failed to convert CLSID %ws, HRESULT %X", clsid, res);
    return false;
}

bool XWActiveImpl::loadActiveObject(const CLSID& clsid)
{
    // release previous object if any
    releaseObject();

    // create storage first
    if(!_createStorage(0)) return false;

    // load ActiveX
    HRESULT res = ::OleCreate(clsid, IID_IOleObject, OLERENDER_DRAW, 0 ,(IOleClientSite*)this, m_pStorage, (LPVOID*)&m_pOleObject);
    if(res != S_OK || m_pOleObject == 0)
    {
        XWTRACE_HRES("Failed to create ActiveX control", res);
        return false;
    }

    // set as contained
    res = ::OleSetContainedObject(m_pOleObject, TRUE);
    if(FAILED(res))
    {
        XWTRACE_HRES("Failed to set ActiveX control as contained", res);
        return false;
    }

    return true;
}

void XWActiveImpl::releaseObject()
{
    // release interfaces
    if(m_pCustomInterfaces) m_pCustomInterfaces->Release();
    if(m_pIOleInPlaceActiveObject) m_pIOleInPlaceActiveObject->Release();
    if(m_pOleInPlaceObject) m_pOleInPlaceObject->Release();
    if(m_pOleObject) 
    {
        // close object
        HRESULT hr = m_pOleObject->Close(OLECLOSE_NOSAVE);
        if(FAILED(hr))
        {
            XWTRACE_HRES("XWActiveImpl: failed to close Ole Object", hr);
        }

        m_pOleObject->Release();
    }

    // reset pointers
    m_pIOleInPlaceActiveObject = 0;
    m_pOleInPlaceObject = 0;
    m_pOleObject = 0;

    // close storage
    _closeStorage();

    // reset state
    m_bIOleInPlaceActiveObjectSupported = true;
}

bool XWActiveImpl::isActiveObjectLoaded()
{
    return (m_pOleObject != 0);
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
bool XWActiveImpl::activateInPlace()
{
    XWASSERT(m_pOleObject);
    if(!m_pOleObject) return false;

    // get host window size
    RECT rect;
    ::GetClientRect(m_hwndContainer, &rect);

    // activate control
    HRESULT res = m_pOleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, 0, (IOleClientSite*)this, 0, m_hwndContainer, &rect);
    if(FAILED(res))
    {
        XWTRACE_HRES("Failed to in-place activate ActiveX control", res);
        return false;
    }
    
    return true;
}

bool XWActiveImpl::resize(const RECT& sizeRect)
{
    XWASSERT(m_pOleObject);
    if(!m_pOleObject) return false;
    
    HRESULT res;

    // check if we have interface already
    if(!m_pOleInPlaceObject)
    {
        // query interface
        res = m_pOleObject->QueryInterface(IID_IOleInPlaceObject, (LPVOID*)&m_pOleInPlaceObject);
        if(FAILED(res))
        {
            XWTRACE_HRES("Failed to query IID_IOleInPlaceActiveObject from ActiveX control", res);
            return false;
        }
    }

    // update size
    res = m_pOleInPlaceObject->SetObjectRects(&sizeRect, &sizeRect);
    if(FAILED(res))
    {
        XWTRACE_HRES("Failed to SetObjectRects for ActiveX control", res);
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////
// get interface from loaded object
/////////////////////////////////////////////////////////////////////
bool XWActiveImpl::queryObjectInterface(REFIID riid, void** ppvObject)
{
    // reset pointer first
    *ppvObject = 0;

    // check if loaded
    if(!m_pOleObject) return false;

    // query interface
    HRESULT res = m_pOleObject->QueryInterface(riid, ppvObject);
    if(FAILED(res))
    {
        XWTRACE_HRES("Failed to query inteface from ActiveX control", res);
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////
// translate message 
/////////////////////////////////////////////////////////////////////
bool XWActiveImpl::translateAccelerator(MSG* pmsg)
{
    // ignore if not supported
    if(!m_bIOleInPlaceActiveObjectSupported) return false;

    XWASSERT(m_pOleObject);
    if(!m_pOleObject) return false;
    
    HRESULT res;

    // check if we have IOleInPlaceActiveObject
    if(m_pIOleInPlaceActiveObject == 0)
    {
        // try to load interface from object
        res = m_pOleObject->QueryInterface(IID_IOleInPlaceActiveObject, (LPVOID*)&m_pIOleInPlaceActiveObject);
        if(FAILED(res) || m_pIOleInPlaceActiveObject == 0)
        {
            XWTRACE_HRES("Failed to query IOleInPlaceActiveObject from ActiveX control", res);
            m_bIOleInPlaceActiveObjectSupported = false;
            return false;
        }
    }

    // translate message
    res = m_pIOleInPlaceActiveObject->TranslateAccelerator(pmsg);

    // S_OK - translated
    // S_FALSE - not translated
    return (res == S_OK);
}

/////////////////////////////////////////////////////////////////////
// customizations
/////////////////////////////////////////////////////////////////////
void XWActiveImpl::setCustomizations(IUnknown* pCustomInterfaces)
{
    // release previous if any
    if(m_pCustomInterfaces) m_pCustomInterfaces->Release();

    // set
    m_pCustomInterfaces = pCustomInterfaces;

    // add ref
    if(m_pCustomInterfaces) m_pCustomInterfaces->AddRef();
}

/////////////////////////////////////////////////////////////////////
// IUnknown 
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XWActiveImpl::QueryInterface(REFIID riid, void** ppvObject)
{
    if(ppvObject == 0) return E_INVALIDARG;

    // reset pointer first
    *ppvObject = 0;

    ///// check required interface

    // IUnknown
    if(riid == IID_IUnknown)
        *ppvObject = (IUnknown*)(IDispatch*)this;

    // IDispatch
    else if(riid == IID_IDispatch)
        *ppvObject = (IDispatch*)this;

    // IOleClientSite
    else if(riid == IID_IOleClientSite)
        *ppvObject = (IOleClientSite*)this;

    // IOleWindow
    else if(riid == IID_IOleWindow)
        *ppvObject = (IOleWindow*)(IOleInPlaceSite*)this;

    // IOleInPlaceSite
    else if(riid == IID_IOleInPlaceSite)
        *ppvObject = (IOleInPlaceSite*)this;

    // IOleInPlaceUIWindow
    else if(riid == IID_IOleInPlaceUIWindow)
        *ppvObject = (IOleInPlaceUIWindow*)(IOleInPlaceFrame*)this;

    // IOleInPlaceFrame
    else if(riid == IID_IOleInPlaceFrame)
        *ppvObject = (IOleInPlaceFrame*)this;

    if (!*ppvObject)
    {
        // pass to customization interface if set
        if(m_pCustomInterfaces)
            return m_pCustomInterfaces->QueryInterface(riid, ppvObject);
        else
            return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) XWActiveImpl::AddRef()
{
    return ++m_ulRef;
}

STDMETHODIMP_(ULONG) XWActiveImpl::Release()
{
    if (!--m_ulRef) 
    {
        delete this;
        return 0;
    }
    return m_ulRef;
}

/////////////////////////////////////////////////////////////////////
// IDispatch
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XWActiveImpl::GetTypeInfoCount(UINT* /*pctinfo*/)
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XWActiveImpl::GetTypeInfo(UINT /*iTInfo*/, LCID /*lcid*/, ITypeInfo** /*ppTInfo*/)
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XWActiveImpl::GetIDsOfNames(REFIID /*riid*/, OLECHAR** /*rgszNames*/, UINT /*cNames*/, LCID /*lcid*/, DISPID* /*rgDispId*/)
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XWActiveImpl::Invoke(DISPID dispIdMember,
                                   REFIID riid,
                                   LCID lcid,
                                   WORD wFlags,
                                   DISPPARAMS* pDispParams,
                                   VARIANT* pVarResult,
                                   EXCEPINFO* pExcepInfo,
                                   UINT* puArgErr)
{
    // not needed
    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////
// IOleClientSite
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XWActiveImpl::SaveObject()
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XWActiveImpl::GetMoniker(DWORD /*dwAssign*/, DWORD /*dwWhichMoniker*/, IMoniker** ppmk)
{
    if (!ppmk)
        return E_POINTER;

    *ppmk = 0;
    return E_NOTIMPL;
}

STDMETHODIMP XWActiveImpl::GetContainer(IOleContainer** ppContainer)
{
    if (!ppContainer)
        return E_POINTER;

    *ppContainer = 0;
    return E_NOINTERFACE;
}

STDMETHODIMP XWActiveImpl::ShowObject()
{
    return S_OK;
}

STDMETHODIMP XWActiveImpl::OnShowWindow(BOOL /*fShow*/)
{
    InvalidateRect(m_hwndContainer, 0, TRUE);
    InvalidateRect(::GetParent(m_hwndContainer), 0, TRUE);
    return S_OK;
}

STDMETHODIMP XWActiveImpl::RequestNewObjectLayout()
{
    // not needed
    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////
// IOleWindow
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XWActiveImpl::GetWindow(HWND *phwnd)
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

STDMETHODIMP XWActiveImpl::ContextSensitiveHelp(BOOL fEnterMode)
{
    // not needed
    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////
// IOleInPlaceSite
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XWActiveImpl::CanInPlaceActivate()
{
    return S_OK;
}

STDMETHODIMP XWActiveImpl::OnInPlaceActivate()
{
    return S_OK;
}

STDMETHODIMP XWActiveImpl::OnUIActivate()
{
    return S_OK;
}

STDMETHODIMP XWActiveImpl::GetWindowContext(IOleInPlaceFrame** ppFrame, 
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

STDMETHODIMP XWActiveImpl::Scroll(SIZE /*scrollExtant*/)
{
    return S_FALSE;
}

STDMETHODIMP XWActiveImpl::OnUIDeactivate(BOOL fUndoable)
{
    return S_OK;
}

STDMETHODIMP XWActiveImpl::OnInPlaceDeactivate()
{
    return S_OK;
}

STDMETHODIMP XWActiveImpl::DiscardUndoState()
{
    return S_OK;
}

STDMETHODIMP XWActiveImpl::DeactivateAndUndo()
{
    return S_OK;
}

STDMETHODIMP XWActiveImpl::OnPosRectChange(LPCRECT lprcPosRect)
{
    return S_OK;
}

/////////////////////////////////////////////////////////////////////
// IOleInPlaceUIWindow
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XWActiveImpl::GetBorder(LPRECT lprectBorder)
{
    if (!lprectBorder) 
        return E_POINTER;

    ::GetClientRect(m_hwndContainer, lprectBorder);
    return S_OK;
}

STDMETHODIMP XWActiveImpl::RequestBorderSpace(LPCBORDERWIDTHS /*pborderwidths*/)
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XWActiveImpl::SetBorderSpace(LPCBORDERWIDTHS /*pborderwidths*/)
{
    return S_OK;
}

STDMETHODIMP XWActiveImpl::SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR /*pszObjName*/)
{
    return S_OK;
}

/////////////////////////////////////////////////////////////////////
// IOleInPlaceFrame
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XWActiveImpl::InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XWActiveImpl::SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject)
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XWActiveImpl::RemoveMenus(HMENU hmenuShared)
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XWActiveImpl::SetStatusText(LPCOLESTR pszStatusText)
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XWActiveImpl::EnableModeless(BOOL fEnable)
{
    // not needed
    return E_NOTIMPL;
}

STDMETHODIMP XWActiveImpl::TranslateAccelerator(LPMSG lpMsg, WORD grfModifiers)
{
    // not needed
    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
bool XWActiveImpl::_createStorage(const wchar_t* name)
{
    // close previous storage if any
    _closeStorage();

    // create storage
    HRESULT res = ::StgCreateStorageEx(name, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_CREATE, 
        STGFMT_STORAGE, 0, 0, 0, IID_IStorage, (LPVOID*)&m_pStorage);

    if(res != S_OK || m_pStorage == 0)
    {
        XWTRACE_HRES("Failed to create storage", res);
        return false;
    }

    return true;
}

void XWActiveImpl::_closeStorage()
{
    if(m_pStorage) m_pStorage->Release();
    m_pStorage = 0;
}

// XWActiveImpl
/////////////////////////////////////////////////////////////////////
