// ActiveX control internals
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWACTIVEIMPL_H_
#define _XWACTIVEIMPL_H_

/////////////////////////////////////////////////////////////////////
// XWActiveImpl - ActiveX host internal implementation


class XWActiveImpl : public IDispatch,
                     public IOleClientSite,
                     public IOleInPlaceSite,
                     public IOleInPlaceFrame
{
public: // construction/destruction
    XWActiveImpl(HWND hwnd);
    virtual ~XWActiveImpl();

public: // load object
    bool    loadActiveObject(const wchar_t* clsid);
    bool    loadActiveObject(const CLSID& clsid);    
    void    releaseObject();
    bool    isActiveObjectLoaded();

public: // interface
    bool    activateInPlace();
    bool    resize(const RECT& sizeRect);

public: // get interface from loaded object
    bool    queryObjectInterface(REFIID riid, void** ppvObject);

public: // translate message 
    bool    translateAccelerator(MSG* pmsg);

public: // customizations
    void    setCustomizations(IUnknown* pCustomInterfaces);

public: // IUnknown 
    STDMETHODIMP            QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

public: // IDispatch
    STDMETHODIMP			GetTypeInfoCount(UINT* pctinfo);
    STDMETHODIMP            GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo);
    STDMETHODIMP            GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId);
    STDMETHODIMP            Invoke(DISPID dispIdMember,
                                   REFIID riid,
                                   LCID lcid,
                                   WORD wFlags,
                                   DISPPARAMS* pDispParams,
                                   VARIANT* pVarResult,
                                   EXCEPINFO* pExcepInfo,
                                   UINT* puArgErr);

public: // IOleClientSite
    STDMETHODIMP            SaveObject();
    STDMETHODIMP            GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker** ppmk);
    STDMETHODIMP            GetContainer(IOleContainer** ppContainer);
    STDMETHODIMP            ShowObject();
    STDMETHODIMP            OnShowWindow(BOOL fShow);
    STDMETHODIMP            RequestNewObjectLayout();

public: // IOleWindow
    STDMETHODIMP            GetWindow(HWND *phwnd);
    STDMETHODIMP            ContextSensitiveHelp(BOOL fEnterMode);

public: // IOleInPlaceSite
    STDMETHODIMP            CanInPlaceActivate();
    STDMETHODIMP            OnInPlaceActivate();
    STDMETHODIMP            OnUIActivate();
    STDMETHODIMP            GetWindowContext(IOleInPlaceFrame** ppFrame, 
                                             IOleInPlaceUIWindow** ppDoc, 
                                             LPRECT lprcPosRect, 
                                             LPRECT lprcClipRect, 
                                             LPOLEINPLACEFRAMEINFO lpFrameInfo);
    STDMETHODIMP            Scroll(SIZE scrollExtant);
    STDMETHODIMP            OnUIDeactivate(BOOL fUndoable);
    STDMETHODIMP            OnInPlaceDeactivate();
    STDMETHODIMP            DiscardUndoState();
    STDMETHODIMP            DeactivateAndUndo();
    STDMETHODIMP            OnPosRectChange(LPCRECT lprcPosRect);

public: // IOleInPlaceUIWindow
    STDMETHODIMP            GetBorder(LPRECT lprectBorder);
    STDMETHODIMP            RequestBorderSpace(LPCBORDERWIDTHS pborderwidths);
    STDMETHODIMP            SetBorderSpace(LPCBORDERWIDTHS pborderwidths);
    STDMETHODIMP            SetActiveObject(IOleInPlaceActiveObject *pActiveObject, LPCOLESTR pszObjName);

public: // IOleInPlaceFrame
    STDMETHODIMP            InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths);
    STDMETHODIMP            SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
    STDMETHODIMP            RemoveMenus(HMENU hmenuShared);
    STDMETHODIMP            SetStatusText(LPCOLESTR pszStatusText);
    STDMETHODIMP            EnableModeless(BOOL fEnable);
    STDMETHODIMP            TranslateAccelerator(LPMSG lpMsg, WORD grfModifiers);

private: // worker methods
    bool    _createStorage(const wchar_t* name);
    void    _closeStorage();

private: // data
    HWND                    m_hwndContainer;
    unsigned long           m_ulRef;
    bool                    m_bIOleInPlaceActiveObjectSupported;

private: // references
    IOleObject*                 m_pOleObject;
    IOleInPlaceObject*          m_pOleInPlaceObject;
    IOleInPlaceActiveObject*    m_pIOleInPlaceActiveObject;
    IStorage*                   m_pStorage;
    IUnknown*                   m_pCustomInterfaces;
};

// XWActiveImpl
/////////////////////////////////////////////////////////////////////

#endif // _XWACTIVEIMPL_H_

