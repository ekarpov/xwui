// Internet Explorer ActiveX control helpers
// 
/////////////////////////////////////////////////////////////////////

#ifndef _XWBROWSERHELPERS_H_
#define _XWBROWSERHELPERS_H_

/////////////////////////////////////////////////////////////////////
// NOTE: good example https://github.com/nasa/World-Wind-Java/blob/master/WorldWind/lib-external/webview/windows/WebViewWindow.cpp

/////////////////////////////////////////////////////////////////////
// forward declarations
struct IWebBrowser2;
struct IHTMLDocument2;
struct IHTMLDocument3;
struct IHTMLElement;
struct IHTMLElement2;
struct IHTMLStyle;

/////////////////////////////////////////////////////////////////////
// XWBrowserHelpers - browser helper methods

namespace XWBrowserHelpers
{
    // browser document helpers
    HRESULT     getHTMLDocument2(IWebBrowser2* webBrowser, IHTMLDocument2** docOut);
    HRESULT     getHTMLDocument3(IWebBrowser2* webBrowser, IHTMLDocument3** docOut);
    HRESULT     getDocumentElement(IWebBrowser2* webBrowser, IHTMLElement** elOut);
    HRESULT     getDocumentElement2(IWebBrowser2* webBrowser, IHTMLElement2** elOut);
    HRESULT     getDocumentStyle(IWebBrowser2* webBrowser, IHTMLStyle** styleOut);
    
    // document options
    HRESULT     setShowScrollBars(IWebBrowser2* webBrowser, bool bShow);
    HRESULT     getContentSize(IWebBrowser2* webBrowser, long& clientHeight, long& clientWidth);
    HRESULT     hideAllScrollBars(IWebBrowser2* webBrowser);

    // write in-memory HTML
    HRESULT     writeHTML(IWebBrowser2* webBrowser, const WCHAR* html);

    // IE version
    LONG        getInstalledVersion(int& majorOut, int& minorOut);

    // features (HKEY_CURRENT_USER\Software\Microsoft\Internet Explorer\Main\FeatureControl)
    // https://msdn.microsoft.com/en-us/library/ee330720(v=vs.85).aspx
    LONG        setBinaryFeature(const WCHAR* binaryName, const WCHAR* feature, DWORD value);
    LONG        removeBinaryFeature(const WCHAR* binaryName, const WCHAR* feature);

    // known features 
    LONG        setEmulationVersion(const WCHAR* binaryName, int version);
    LONG        setMaxEmulationVersion(const WCHAR* binaryName);
    LONG        enableGPURendering(const WCHAR* binaryName, BOOL enable);

    // language settings
    LONG        readLanguage(std::wstring& value);
    LONG        setLanguage(const WCHAR* value);
    LONG        setLanguage(const WCHAR* subkey, const WCHAR* value);
};

// XWBrowserHelpers
/////////////////////////////////////////////////////////////////////

#endif // _XWBROWSERHELPERS_H_

