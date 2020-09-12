// Internet Explorer ActiveX customizations
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xwiecustomization.h"

/////////////////////////////////////////////////////////////////////
// XWIECustomization - empty customization implementation

XWIECustomization::XWIECustomization()
{
}

XWIECustomization::~XWIECustomization()
{
}

/////////////////////////////////////////////////////////////////////
// IDocHostUIHandler 
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XWIECustomization::ShowContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved)
{
    // If text select or control menu, then return S_FALSE to show menu.
    if(dwID == CONTEXT_MENU_TEXTSELECT || dwID == CONTEXT_MENU_CONTROL)
    {
        return S_FALSE;
    }

    // prevent from displaying any context menu
    return S_OK;
}

STDMETHODIMP XWIECustomization::GetHostInfo(DOCHOSTUIINFO *pInfo)
{
    // init flags
    if(pInfo)
    {
        pInfo->cbSize = sizeof(DOCHOSTUIINFO);
        pInfo->dwFlags = DOCHOSTUIFLAG_DIALOG |
                         DOCHOSTUIFLAG_DISABLE_HELP_MENU | 
                         DOCHOSTUIFLAG_NO3DBORDER | 
                         DOCHOSTUIFLAG_SCROLL_NO |
                         DOCHOSTUIFLAG_ENABLE_REDIRECT_NOTIFICATION; // enable redirects in BeforeNavigate2
        pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;
    }

    return S_OK;
}

STDMETHODIMP XWIECustomization::ShowUI(DWORD dwID, IOleInPlaceActiveObject *pActiveObject, IOleCommandTarget *pCommandTarget,
                           IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc)
{
    return E_NOTIMPL;
}

STDMETHODIMP XWIECustomization::HideUI()
{
    return E_NOTIMPL;
}

STDMETHODIMP XWIECustomization::UpdateUI()
{
    return E_NOTIMPL;
}

STDMETHODIMP XWIECustomization::EnableModeless(BOOL fEnable)
{
    return E_NOTIMPL;
}

STDMETHODIMP XWIECustomization::OnDocWindowActivate(BOOL fActivate)
{
    return E_NOTIMPL;
}

STDMETHODIMP XWIECustomization::OnFrameWindowActivate(BOOL fActivate)
{
    return E_NOTIMPL;
}

STDMETHODIMP XWIECustomization::ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow)
{
    return E_NOTIMPL;
}

STDMETHODIMP XWIECustomization::TranslateAccelerator(LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID)
{
    // block all other keys by default
    //return S_OK;

    // translate all by default, let parent control handle what is allowed
    return S_FALSE;
}

STDMETHODIMP XWIECustomization::GetOptionKeyPath(LPOLESTR *pchKey, DWORD dw)
{
    return E_NOTIMPL;
}

STDMETHODIMP XWIECustomization::GetDropTarget(IDropTarget *pDropTarget, IDropTarget **ppDropTarget)
{
    return E_NOTIMPL;
}

STDMETHODIMP XWIECustomization::GetExternal(IDispatch **ppDispatch)
{
    return E_NOTIMPL;
}

STDMETHODIMP XWIECustomization::TranslateUrl(DWORD dwTranslate, OLECHAR *pchURLIn,OLECHAR **ppchURLOut)
{
    return E_NOTIMPL;
}

STDMETHODIMP XWIECustomization::FilterDataObject(IDataObject *pDO, IDataObject **ppDORet)
{
    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////
// IDocHostUIHandler2 (webbrowser registry changes)
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XWIECustomization::GetOverrideKeyPath(LPOLESTR *pchKey, DWORD dw)
{
    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////
// IDocHostShowUI
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XWIECustomization::ShowMessage(HWND hwnd, LPOLESTR lpstrText, LPOLESTR lpstrCaption, 
                                                  DWORD dwType, LPOLESTR lpstrHelpFile, DWORD dwHelpContext, LRESULT *plResult)
{
    // Return S_OK to disable WebBrowser Control messages. 
    // Any other return value, like S_FALSE or E_NOTIMPL, 
    // allows the WebBrowser Control to display with its message box.
    return S_OK;
}

STDMETHODIMP XWIECustomization::ShowHelp(HWND hwnd, LPOLESTR pszHelpFile, UINT uCommand, DWORD dwData, 
                                               POINT ptMouse, IDispatch *pDispatchObjectHit)
{
    // Return S_OK to override Internet Explorer Help, or another HRESULT 
    // value to let Internet Explorer proceed with its Help.
    return S_OK;
}

// XWIECustomization
/////////////////////////////////////////////////////////////////////
