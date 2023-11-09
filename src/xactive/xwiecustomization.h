// Internet Explorer ActiveX customizations
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWIECUSTOMIZATION_H_
#define _XWIECUSTOMIZATION_H_

/////////////////////////////////////////////////////////////////////
// IE ActiveX support
#include <Mshtmhst.h>

/////////////////////////////////////////////////////////////////////
// WebBrowser Customization:
// https://msdn.microsoft.com/en-us/library/aa770041(v=vs.85).aspx

/////////////////////////////////////////////////////////////////////
// XWIECustomization - empty customization implementation

class XWIECustomization : public IDocHostShowUI,
                          public IDocHostUIHandler2
{
public: // construction/destruction
    XWIECustomization();
    virtual ~XWIECustomization();

public: // IDocHostUIHandler (webbrowser control properties)
    STDMETHODIMP    ShowContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved);        
    STDMETHODIMP    GetHostInfo(DOCHOSTUIINFO *pInfo);        
    STDMETHODIMP    ShowUI(DWORD dwID, IOleInPlaceActiveObject *pActiveObject, IOleCommandTarget *pCommandTarget,
                                   IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc);        
    STDMETHODIMP    HideUI();        
    STDMETHODIMP    UpdateUI();        
    STDMETHODIMP    EnableModeless(BOOL fEnable);        
    STDMETHODIMP    OnDocWindowActivate(BOOL fActivate);        
    STDMETHODIMP    OnFrameWindowActivate(BOOL fActivate);        
    STDMETHODIMP    ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow);        
    STDMETHODIMP    TranslateAccelerator(LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID);        
    STDMETHODIMP    GetOptionKeyPath(LPOLESTR *pchKey, DWORD dw);        
    STDMETHODIMP    GetDropTarget(IDropTarget *pDropTarget, IDropTarget **ppDropTarget);        
    STDMETHODIMP    GetExternal(IDispatch **ppDispatch);        
    STDMETHODIMP    TranslateUrl(DWORD dwTranslate, OLECHAR *pchURLIn,OLECHAR **ppchURLOut);        
    STDMETHODIMP    FilterDataObject(IDataObject *pDO, IDataObject **ppDORet);

public: // IDocHostUIHandler2 (webbrowser registry changes)
    STDMETHODIMP    GetOverrideKeyPath(LPOLESTR *pchKey, DWORD dw);        

public: // IDocHostShowUI
    STDMETHODIMP    ShowMessage(HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, DWORD dwType, 
                                LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult);        
    STDMETHODIMP    ShowHelp(HWND hwnd, LPOLESTR pszHelpFile, UINT uCommand, DWORD dwData, 
                             POINT ptMouse, IDispatch *pDispatchObjectHit);
};

// XWIECustomization
/////////////////////////////////////////////////////////////////////

#endif // _XWIECUSTOMIZATION_H_

