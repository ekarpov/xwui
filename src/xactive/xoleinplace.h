// Windowless OLE in-place activation helper
//
/////////////////////////////////////////////////////////////////////

#ifndef _XOLEINPLACE_H_
#define _XOLEINPLACE_H_

/////////////////////////////////////////////////////////////////////
// NOTE: interface inheritance:
// IOleInPlaceSiteWindowless -> IOleInPlaceSiteEx -> IOleInPlaceSite -> IOleWindow -> IUnknown
// IOleInPlaceFrame -> IOleInPlaceUIWindow -> IOleWindow -> IUnknown
// IOleClientSite -> IUnknown

/////////////////////////////////////////////////////////////////////
// XOleInplaceImpl - windowless OLE in-place implementation

class XOleInplaceImpl : public IOleInPlaceSiteWindowless,
                        public IOleClientSite,
                        public IOleInPlaceFrame
                        
{
public: // construction/destruction
    XOleInplaceImpl();
    ~XOleInplaceImpl();

public: // initialization
    bool    loadOleObject(const wchar_t* clsid);
    bool    loadOleObject(const CLSID& clsid);
    void    releaseObject();
    bool    isOleObjectLoaded() const;

public: // interface
    bool    activateInPlace(const RECT& rcClient, bool windowless = false);
    bool    update(const RECT& rcClient);

public: // get interface from loaded object
    bool    queryObjectInterface(REFIID riid, void** ppvObject);

public: // message processing
    LRESULT processWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& messageProcessed);

public: // properties
    void    setEnabled(bool bEnabled);
    bool    isEnabled() const;

public: // focus
    void    setFocus(bool bFocus);

public: // GDI painting
    void    onPaintGDI(HDC hdc, const RECT& rcPaint);   

public: // IUnknown 
    STDMETHODIMP            QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

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

public: // IOleInPlaceSiteEx
    STDMETHODIMP            OnInPlaceActivateEx(BOOL *pfNoRedraw, DWORD dwFlags);
    STDMETHODIMP            OnInPlaceDeactivateEx(BOOL fNoRedraw);
    STDMETHODIMP            RequestUIActivate();

public: // IOleInPlaceSiteWindowless
    STDMETHODIMP            CanWindowlessActivate();
    STDMETHODIMP            GetCapture();
    STDMETHODIMP            SetCapture(BOOL fCapture);
    STDMETHODIMP            GetFocus();
    STDMETHODIMP            SetFocus(BOOL fFocus);
    STDMETHODIMP            GetDC(LPCRECT pRect, DWORD grfFlags, HDC *phDC);
    STDMETHODIMP            ReleaseDC(HDC hDC);
    STDMETHODIMP            InvalidateRect(LPCRECT pRect, BOOL fErase);
    STDMETHODIMP            InvalidateRgn(HRGN hRGN, BOOL fErase);
    STDMETHODIMP            ScrollRect(INT dx, INT dy, LPCRECT pRectScroll, LPCRECT pRectClip);
    STDMETHODIMP            AdjustRect(LPRECT prc);
    STDMETHODIMP            OnDefWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);

public: // IOleClientSite
    STDMETHODIMP            SaveObject();
    STDMETHODIMP            GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker** ppmk);
    STDMETHODIMP            GetContainer(IOleContainer** ppContainer);
    STDMETHODIMP            ShowObject();
    STDMETHODIMP            OnShowWindow(BOOL fShow);
    STDMETHODIMP            RequestNewObjectLayout();

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

private: // helper methods
    bool    _createStorage(const wchar_t* name);
    void    _closeStorage();

private: // data
    HWND                    m_hwndContainer;
    unsigned long           m_ulRef;
    bool                    m_bWindowless;

private: // references
    IOleObject*                     m_pOleObject;
    IOleInPlaceObject*              m_pOleInPlaceObject;
    IOleInPlaceObjectWindowless*    m_pOleInPlaceObjectWindowless;
    IStorage*                       m_pStorage;
};

// XOleInplaceImpl
/////////////////////////////////////////////////////////////////////

#endif // _XOLEINPLACE_H_

