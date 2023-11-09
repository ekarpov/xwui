// Internet Explorer ActiveX control events (DWebBrowserEvents2)
// 
/////////////////////////////////////////////////////////////////////

#ifndef _XWBROWSEREVENTS_H_
#define _XWBROWSEREVENTS_H_

/////////////////////////////////////////////////////////////////////
// XWBrowserEvents - helper class to convert IDispatch events

class XWBrowserEvents
{
public: // construction/destruction
    XWBrowserEvents();
    ~XWBrowserEvents();

public: // convert IDispatch Invoke to DWebBrowserEvents2 events
    STDMETHODIMP    handleInvoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, 
                                 VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);    

public: // DWebBrowserEvents2 events
    virtual void    BeforeNavigate2(IDispatch* pDisp, BSTR url, LONG flags, BSTR targetFrameName, 
                                    LPVOID postData, LONG postDataSize, BSTR headers, VARIANT_BOOL* cancelFlag); 
    virtual void    ClientToHostWindow();
    virtual void    CommandStateChange();
    virtual void    DocumentComplete();
    virtual void    DownloadBegin();
    virtual void    DownloadComplete();
    virtual void    NavigateComplete2(IDispatch * pDisp, BSTR url);
    virtual void    NavigateError(IDispatch * pDisp, BSTR* url, BSTR* targetFrameName, LONG* statusCode, VARIANT_BOOL* cancelFlag);    
    virtual void    NewProcess();
    virtual void    NewWindow2();
    virtual void    NewWindow3(IDispatch * pDisp, VARIANT_BOOL* cancelFlag, LONG dwFlags, BSTR bstrUrlContext, BSTR bstrUrl);
    virtual void    OnFullScreen();
    virtual void    OnMenuBar();
    virtual void    OnQuit();
    virtual void    OnStatusBar();
    virtual void    OnTheaterMode();
    virtual void    OnToolBar();
    virtual void    OnVisible();
    virtual void    PrintTemplateInstantiation();
    virtual void    PrintTemplateTeardown();
    virtual void    ProgressChange();
    virtual void    PropertyChange();
    virtual void    StatusTextChange();
    virtual void    TitleChange();
    virtual void    UpdatePageStatus();
    virtual void    WindowClosing(BOOL bIsChildWindow, VARIANT_BOOL* cancelFlag);
    virtual void    WindowSetHeight();
    virtual void    WindowSetLeft();
    virtual void    WindowSetResizable();
    virtual void    WindowSetTop();
    virtual void    WindowStateChanged();

private: // helper methods
    bool    _beginSafeArrayRead(VARIANT* sfArray, void** pData, LONG* dataSize);
    bool    _endSafeArrayRead(VARIANT* sfArray);
};

// XWBrowserEvents
/////////////////////////////////////////////////////////////////////

#endif // _XWBROWSEREVENTS_H_

