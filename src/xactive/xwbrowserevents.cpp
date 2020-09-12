// Internet Explorer ActiveX control events (DWebBrowserEvents2)
//
/////////////////////////////////////////////////////////////////////

// NOTE: http://msdn.microsoft.com/en-us/library/aa768283(v=vs.85).aspx

#include "../xwui_config.h"

// windows includes
#include <OCIdl.h>
#include <exdisp.h> 
#include <exdispid.h>

#include "xwbrowserevents.h"

/////////////////////////////////////////////////////////////////////
// XWBrowserEvents - helper class to convert IDispatch events

XWBrowserEvents::XWBrowserEvents()
{
}

XWBrowserEvents::~XWBrowserEvents()
{
}

/////////////////////////////////////////////////////////////////////
// convert IDispatch Invoke to DWebBrowserEvents2 events
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XWBrowserEvents::handleInvoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, 
                                 VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
    // check input
    XWASSERT(pDispParams);
    if(pDispParams == 0) return E_INVALIDARG;

    PVOID           ppvData = 0;
    LONG            dataSize = 0;
    VARIANT_BOOL    boolValue;

    // riid should always be IID_NULL
    if(!IsEqualIID(riid, IID_NULL)) return DISP_E_UNKNOWNINTERFACE; 

    switch(dispIdMember)
    {

    // BeforeNavigate2
    case DISPID_BEFORENAVIGATE2:

        // validate input
        if(pDispParams->cArgs < 7) return E_INVALIDARG;

        // postData
        if(!_beginSafeArrayRead(pDispParams->rgvarg[2].pvarVal->pvarVal, &ppvData, &dataSize)) return E_INVALIDARG;

        // init value for return flag if set
        boolValue = VARIANT_FALSE;
        if(pDispParams->rgvarg[0].pvarVal->vt == VT_BOOL)
            boolValue = pDispParams->rgvarg[0].pvarVal->boolVal;

        // pass to event handler
        BeforeNavigate2(pDispParams->rgvarg[6].pdispVal, 
                        pDispParams->rgvarg[5].pvarVal->bstrVal, 
                        pDispParams->rgvarg[4].pvarVal->lVal, 
                        pDispParams->rgvarg[3].pvarVal->bstrVal, 
                        ppvData, dataSize, 
                        pDispParams->rgvarg[1].pvarVal->bstrVal, 
                        &boolValue);

        // update return value if changed
        if(boolValue != VARIANT_FALSE)
        {
            pDispParams->rgvarg[0].pvarVal->vt = VT_BOOL;
            pDispParams->rgvarg[0].pvarVal->boolVal = boolValue;
        }

        // release array
        if(!_endSafeArrayRead(pDispParams->rgvarg[2].pvarVal->pvarVal)) return E_INVALIDARG;
        break;


    // NavigateComplete2
    case DISPID_NAVIGATECOMPLETE2:

        // validate input
        if(pDispParams->cArgs < 2) return E_INVALIDARG;

        // pass to event handler
        NavigateComplete2(pDispParams->rgvarg[1].pdispVal, 
                          pDispParams->rgvarg[0].pvarVal->bstrVal);
        break;

    // DownloadBegin
    case DISPID_DOWNLOADBEGIN:
        // pass to event handler
        DownloadBegin();
        break;

    // DownloadComplete
    case DISPID_DOWNLOADCOMPLETE:
        // pass to event handler
        DownloadComplete();
        break;

    // WindowClosing
    case DISPID_WINDOWCLOSING:
        // validate input
        if(pDispParams->cArgs < 2) return E_INVALIDARG;

        // pass to event handler
        WindowClosing(pDispParams->rgvarg[1].boolVal, pDispParams->rgvarg[0].pboolVal);
        break;

    case DISPID_NAVIGATEERROR:
        // validate input
        if(pDispParams->cArgs < 5) return E_INVALIDARG;

        // pass to event handler
        NavigateError(pDispParams->rgvarg[4].pdispVal, pDispParams->rgvarg[3].pbstrVal, pDispParams->rgvarg[2].pbstrVal, 
            pDispParams->rgvarg[1].plVal, pDispParams->rgvarg[0].pboolVal);
        break;

    case DISPID_NEWWINDOW3:
        // validate input
        if(pDispParams->cArgs < 5) return E_INVALIDARG;

        // pass to event handler
        NewWindow3(pDispParams->rgvarg[4].pdispVal, pDispParams->rgvarg[3].pboolVal, pDispParams->rgvarg[2].lVal, 
            pDispParams->rgvarg[1].bstrVal, pDispParams->rgvarg[0].bstrVal);
        break;

    // DocumentComplete
    case DISPID_DOCUMENTCOMPLETE:
        // pass to event handler
        DocumentComplete();
        break;

    }

    return S_OK;
}

/////////////////////////////////////////////////////////////////////
// DWebBrowserEvents2 events (empty default implementations)
/////////////////////////////////////////////////////////////////////
void XWBrowserEvents::BeforeNavigate2(IDispatch * pDisp, BSTR url, LONG flags, BSTR targetFrameName, 
    LPVOID postData, LONG postDataSize, BSTR headers, VARIANT_BOOL* cancelFlag) {}

void XWBrowserEvents::ClientToHostWindow() {}
void XWBrowserEvents::CommandStateChange() {}
void XWBrowserEvents::DocumentComplete() {}
void XWBrowserEvents::DownloadBegin() {}
void XWBrowserEvents::DownloadComplete() {}
void XWBrowserEvents::NavigateComplete2(IDispatch * pDisp, BSTR url) {}
void XWBrowserEvents::NavigateError(IDispatch * pDisp, BSTR* url, BSTR* targetFrameName, LONG* statusCode, VARIANT_BOOL* cancelFlag) {}
void XWBrowserEvents::NewProcess() {}
void XWBrowserEvents::NewWindow2() {}
void XWBrowserEvents::NewWindow3(IDispatch * pDisp, VARIANT_BOOL* cancelFlag, LONG dwFlags, BSTR bstrUrlContext, BSTR bstrUrl) {}
void XWBrowserEvents::OnFullScreen() {}
void XWBrowserEvents::OnMenuBar() {}
void XWBrowserEvents::OnQuit() {}
void XWBrowserEvents::OnStatusBar() {}
void XWBrowserEvents::OnTheaterMode() {}
void XWBrowserEvents::OnToolBar() {}
void XWBrowserEvents::OnVisible() {}
void XWBrowserEvents::PrintTemplateInstantiation() {}
void XWBrowserEvents::PrintTemplateTeardown() {}
void XWBrowserEvents::ProgressChange() {}
void XWBrowserEvents::PropertyChange() {}
void XWBrowserEvents::StatusTextChange() {}
void XWBrowserEvents::TitleChange() {}
void XWBrowserEvents::UpdatePageStatus() {}
void XWBrowserEvents::WindowClosing(BOOL bIsChildWindow, VARIANT_BOOL* cancelFlag) {}
void XWBrowserEvents::WindowSetHeight() {}
void XWBrowserEvents::WindowSetLeft() {}
void XWBrowserEvents::WindowSetResizable() {}
void XWBrowserEvents::WindowSetTop() {}
void XWBrowserEvents::WindowStateChanged() {}

/////////////////////////////////////////////////////////////////////
// helper methods
/////////////////////////////////////////////////////////////////////
bool XWBrowserEvents::_beginSafeArrayRead(VARIANT* sfArray, void** pData, LONG* dataSize)
{
    LONG lbound, ubound;

    // check if array is empty
    if(sfArray->vt == VT_EMPTY)
    {
        *pData = 0;
        *dataSize = 0;
        return true;
    }

    // check type
    if(!(sfArray->vt & VT_ARRAY)) return false;

    // init size
    if(FAILED(::SafeArrayGetLBound(sfArray->parray, 1, &lbound))) return false;
    if(FAILED(::SafeArrayGetUBound(sfArray->parray, 1, &ubound))) return false;

    // start reading
    if(FAILED(::SafeArrayAccessData(sfArray->parray, pData))) return false;
    
    // init size
    *dataSize = ubound-lbound+1;

    return true;
}

bool XWBrowserEvents::_endSafeArrayRead(VARIANT* sfArray)
{
    // ignore if array is empty
    if(sfArray->vt = VT_EMPTY) return true;

    // check type
    if(!(sfArray->vt & VT_ARRAY)) return false;

    // release data
    if(FAILED(::SafeArrayUnaccessData(sfArray->parray))) return false;

    return true;
}

// XWBrowserEvents
/////////////////////////////////////////////////////////////////////
