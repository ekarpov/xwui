// Internet Explorer ActiveX control helpers
// 
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

// windows headers
#include <DocObj.h>
#include <exdisp.h> 
#include <Mshtml.h>

#include "xwbrowserhelpers.h"

/////////////////////////////////////////////////////////////////////
// constants 

#define XWBROWSER_FEATURE_EMULATION             L"FEATURE_BROWSER_EMULATION"
#define XWBROWSER_FEATURE_GPU_RENDERING         L"FEATURE_GPU_RENDERING"

/////////////////////////////////////////////////////////////////////
// XWBrowserHelpers - browser helper methods

// browser document helpers
HRESULT XWBrowserHelpers::getHTMLDocument2(IWebBrowser2* webBrowser, IHTMLDocument2** docOut)
{
    XWASSERT(webBrowser);
    XWASSERT(docOut);
    if(webBrowser == 0 || docOut == 0) return E_INVALIDARG;

    // get document
    IDispatch* browserDoc = 0;
    HRESULT hr = webBrowser->get_Document(&browserDoc);
    if(FAILED(hr) || browserDoc == 0) return hr;

    // get document interface
    hr = browserDoc->QueryInterface(__uuidof(IHTMLDocument2), (void**)docOut);
    browserDoc->Release();

    return hr;
}

HRESULT XWBrowserHelpers::getHTMLDocument3(IWebBrowser2* webBrowser, IHTMLDocument3** docOut)
{
    XWASSERT(webBrowser);
    XWASSERT(docOut);
    if(webBrowser == 0 || docOut == 0) return E_INVALIDARG;

    XWASSERT(webBrowser);
    XWASSERT(docOut);
    if(webBrowser == 0 || docOut == 0) return E_INVALIDARG;

    // get document
    IDispatch* browserDoc = 0;
    HRESULT hr = webBrowser->get_Document(&browserDoc);
    if(FAILED(hr) || browserDoc == 0) return hr;

    // get document interface
    hr = browserDoc->QueryInterface(__uuidof(IHTMLDocument3), (void**)docOut);
    browserDoc->Release();

    return hr;
}

HRESULT XWBrowserHelpers::getDocumentElement(IWebBrowser2* webBrowser, IHTMLElement** elOut)
{
    XWASSERT(webBrowser);
    XWASSERT(elOut);
    if(webBrowser == 0 || elOut == 0) return E_INVALIDARG;

    // get document interface
    IHTMLDocument3* htmlDoc = 0;
    HRESULT hr = getHTMLDocument3(webBrowser, &htmlDoc);
    if(FAILED(hr) || htmlDoc == 0) return hr;

    // html element
    hr = htmlDoc->get_documentElement(elOut);
    htmlDoc->Release();

    return hr;
}

HRESULT XWBrowserHelpers::getDocumentElement2(IWebBrowser2* webBrowser, IHTMLElement2** elOut)
{
    XWASSERT(webBrowser);
    XWASSERT(elOut);
    if(webBrowser == 0 || elOut == 0) return E_INVALIDARG;

    // html element
    IHTMLElement* htmlElement = 0;
    HRESULT hr = getDocumentElement(webBrowser, &htmlElement);
    if(FAILED(hr) || htmlElement == 0) return hr;

    // get IHTMLElement2 interface
    hr = htmlElement->QueryInterface(__uuidof(IHTMLElement2), (void**)elOut);
    htmlElement->Release();

    return hr;
}

HRESULT XWBrowserHelpers::getDocumentStyle(IWebBrowser2* webBrowser, IHTMLStyle** styleOut)
{
    XWASSERT(webBrowser);
    XWASSERT(styleOut);
    if(webBrowser == 0 || styleOut == 0) return E_INVALIDARG;

    // html element
    IHTMLElement* htmlElement = 0;
    HRESULT hr = getDocumentElement(webBrowser, &htmlElement);
    if(FAILED(hr) || htmlElement == 0) return hr;

    // get style
    hr = htmlElement->get_style(styleOut);
    htmlElement->Release();

    return hr;
}

// document options
HRESULT XWBrowserHelpers::setShowScrollBars(IWebBrowser2* webBrowser, bool bShow)
{
    XWASSERT(webBrowser);
    if(webBrowser == 0) return E_INVALIDARG;
    
    // get style
    IHTMLStyle* htmlStyle = 0;
    HRESULT hr = getDocumentStyle(webBrowser, &htmlStyle);
    if(FAILED(hr) || htmlStyle == 0) return hr;
    
    // hide scrollbars
    if(bShow)
        hr = htmlStyle->put_overflow(L"auto");
    else
        hr = htmlStyle->put_overflow(L"hidden");
    htmlStyle->Release();

    return hr;
}

HRESULT XWBrowserHelpers::getContentSize(IWebBrowser2* webBrowser, long& clientHeight, long& clientWidth)
{
    XWASSERT(webBrowser);
    if(webBrowser == 0) return E_INVALIDARG;
    
    // html element
    IHTMLElement2* htmlElement = 0;
    HRESULT hr = getDocumentElement2(webBrowser, &htmlElement);
    if(FAILED(hr) || htmlElement == 0) return hr;

    if(SUCCEEDED(hr))
        hr = htmlElement->get_scrollWidth(&clientWidth);
    if(SUCCEEDED(hr))
        hr = htmlElement->get_scrollHeight(&clientHeight);

    htmlElement->Release();

    return hr;
}

HRESULT XWBrowserHelpers::hideAllScrollBars(IWebBrowser2* webBrowser)
{
    XWASSERT(webBrowser);
    if(webBrowser == 0) return E_INVALIDARG;
    
    // get document interface
    IHTMLDocument2* htmlDoc = 0;
    HRESULT hr = getHTMLDocument2(webBrowser, &htmlDoc);
    if(FAILED(hr) || htmlDoc == 0) return hr;

    // get all elements
    IHTMLElementCollection* docElements = 0;
    hr = htmlDoc->get_all(&docElements);
    htmlDoc->Release();
    if(FAILED(hr) || docElements == 0) return hr;

    // loop over all elements
    LONG count = 0;
    hr = docElements->get_length(&count);
    for(LONG ii = 0; SUCCEEDED(hr) && ii < count; ++ii)
    {
        VARIANT varIndex;
        varIndex.vt = VT_UINT;
        varIndex.lVal = ii;
        VARIANT var2;
        VariantInit(&var2);

        // get element
        IDispatch* pDisp; 
        hr = docElements->item(varIndex, var2, &pDisp );        
        if(SUCCEEDED(hr) && pDisp != 0)
        {
            // get element interface
            IHTMLElement* pElem = 0;
            hr = pDisp->QueryInterface(IID_IHTMLElement, (void **)&pElem);
            if(SUCCEEDED(hr) && pElem != 0)
            {
                // get element style
                IHTMLStyle* htmlStyle = 0;
                hr = pElem->get_style(&htmlStyle);
                if(SUCCEEDED(hr) && htmlStyle != 0)
                {
                    // hide scrollbars
                    hr = htmlStyle->put_overflow(L"hidden");

                    htmlStyle->Release();
                }

                pElem->Release();
            }

            pDisp->Release();
        }
    }

    docElements->Release();

    return hr;
}

// write in-memory HTML
HRESULT XWBrowserHelpers::writeHTML(IWebBrowser2* webBrowser, const WCHAR* html)
{
    // check input
    XWASSERT(webBrowser);
    XWASSERT(html);
    if(webBrowser == 0 || html == 0) return E_INVALIDARG;

    VARIANT vUrl;
    vUrl.vt = VT_BSTR;
    vUrl.bstrVal = L"about:blank";

    // NOTE: this is required for in-memory HTML to work
    HRESULT hr = webBrowser->Navigate2(&vUrl, 0, 0, 0, 0);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XWBrowserHelpers: failed to pre-load about:blank, in memory HTML will not work", hr);
        return hr;
    }

    // get html document from browser
    IDispatch* htmlDoc;
    hr = webBrowser->get_Document(&htmlDoc);

    // check errors
    if(FAILED(hr)) return hr;
    if(htmlDoc == 0) return E_FAIL;

    // get interface
    IHTMLDocument2* htmlDoc2 = 0;
    hr = htmlDoc->QueryInterface(__uuidof(htmlDoc2), (void**)&htmlDoc2);

    // release initial doc
    htmlDoc->Release();

    // check errors
    if(FAILED(hr)) return hr;
    if(htmlDoc2 == 0) return E_FAIL;
    
    hr = S_OK;

    // Creates a new one-dimensional array
    SAFEARRAY* psaStrings = ::SafeArrayCreateVector(VT_VARIANT, 0, 1);
    if(psaStrings)
    {
	BSTR bstr = ::SysAllocString(html);
	if (bstr)
	{
            VARIANT* param = 0;
            hr = ::SafeArrayAccessData(psaStrings, (LPVOID*)&param);
            if (SUCCEEDED(hr))
            {
                param->vt = VT_BSTR;
                param->bstrVal = bstr;
                hr = ::SafeArrayUnaccessData(psaStrings);
                if (SUCCEEDED(hr))			
                {
                    // write content to the browser
                    htmlDoc2->write(psaStrings);
                    htmlDoc2->close();
                }
            }
        } else
        {
            // failed to allocate string
            hr = E_OUTOFMEMORY;
        }

	// NOTE: SafeArrayDestroy calls SysFreeString for each BSTR
        ::SafeArrayDestroy(psaStrings);    

    } else
    {
        // safe array failed for some reason
        hr = E_FAIL;
    }
    
    // release document
    htmlDoc2->Release();

    return hr;
}

/////////////////////////////////////////////////////////////////////
// IE version
/////////////////////////////////////////////////////////////////////
LONG _readVersionValue(const WCHAR* valueName, int& majorOut, int& minorOut)
{
    HKEY hKey;
    LONG lRes = NOERROR;
    std::wstring value;

    // open key
    lRes = ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Internet Explorer", 0, KEY_READ, &hKey);
    if(lRes == ERROR_SUCCESS)
    {
        // read key value size
        DWORD dwValueSize = 0;
        lRes = ::RegQueryValueEx(hKey, valueName, 0, 0, 0, &dwValueSize);
        if(lRes == ERROR_SUCCESS)
        {
            // allocate space
            value.resize(dwValueSize + 1, 0);

            // read value
            lRes = ::RegQueryValueExW(hKey, valueName, 0, 0, (LPBYTE)&value[0], &dwValueSize);
        }

        // close key
        ::RegCloseKey(hKey);
    }

    // convert value
    if(lRes == ERROR_SUCCESS)
    {
        // parse
        swscanf_s(value.c_str(), L"%d.%d", &majorOut, &minorOut);
    }

    return lRes;
}

LONG XWBrowserHelpers::getInstalledVersion(int& majorOut, int& minorOut)
{
    // read frist value
    LONG lRes = _readVersionValue(L"svcVersion", majorOut, minorOut);
    if(lRes == ERROR_FILE_NOT_FOUND)
    {
        // fallback to old version name
        lRes = _readVersionValue(L"Version", majorOut, minorOut);
    }

    if(lRes != ERROR_SUCCESS)
    {
        XWTRACE_WERR("XWBrowserHelpers: failed to read installed version from registry", lRes);
    }

    return lRes;
}

/////////////////////////////////////////////////////////////////////
// features (HKEY_CURRENT_USER\Software\Microsoft\Internet Explorer\Main\FeatureControl)
/////////////////////////////////////////////////////////////////////
LONG XWBrowserHelpers::setBinaryFeature(const WCHAR* binaryName, const WCHAR* feature, DWORD value)
{
    // get current binary name if needed
    std::vector<wchar_t> moduleName;
    if(binaryName == 0)
    {
        if(XWUtils::sGetModuleName(NULL, moduleName))
        {
            binaryName = moduleName.data();
        }
    }

    XWASSERT(binaryName);
    XWASSERT(feature);
    if(binaryName == 0 || feature == 0) return E_INVALIDARG;

    HKEY hKey;
    LONG lRes = NOERROR;

    // format subkey
    std::wstring subkey = L"Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\";
    subkey += feature;

    // open key
    lRes = ::RegOpenKeyExW(HKEY_CURRENT_USER, subkey.c_str(), 0, KEY_WRITE, &hKey);

    // create key if it doesn't exist
    if(lRes == ERROR_FILE_NOT_FOUND)
    {
        lRes = ::RegCreateKeyExW(HKEY_CURRENT_USER, subkey.c_str(), 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &hKey, 0);
    }

    if(lRes == ERROR_SUCCESS)
    {
        // set value
        lRes = ::RegSetValueExW(hKey, binaryName, 0, REG_DWORD, (const BYTE*)&value, sizeof(value));

        // close key
        ::RegCloseKey(hKey);
    }

    if(lRes != ERROR_SUCCESS)
    {
        XWTRACE_WERR("XWBrowserHelpers: failed to set binary feature", lRes);
    }

    return lRes;
}

LONG XWBrowserHelpers::removeBinaryFeature(const WCHAR* binaryName, const WCHAR* feature)
{
    XWASSERT(binaryName);
    XWASSERT(feature);
    if(binaryName == 0 || feature == 0) return E_INVALIDARG;

    HKEY hKey;
    LONG lRes = NOERROR;

    // format subkey
    std::wstring subkey = L"Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\";
    subkey += feature;

    // open key
    lRes = ::RegOpenKeyExW(HKEY_CURRENT_USER, subkey.c_str(), 0, KEY_WRITE, &hKey);

    if(lRes == ERROR_SUCCESS)
    {
        // delete value
        lRes = ::RegDeleteValueW(hKey, binaryName);

        // close key
        ::RegCloseKey(hKey);
    }

    if(lRes != ERROR_SUCCESS && lRes != ERROR_FILE_NOT_FOUND)
    {
        XWTRACE_WERR("XWBrowserHelpers: failed to set binary feature", lRes);
    }

    return lRes;
}

/////////////////////////////////////////////////////////////////////
// known features 
/////////////////////////////////////////////////////////////////////
LONG XWBrowserHelpers::setEmulationVersion(const WCHAR* binaryName, int version)
{
    // NOTE: https://msdn.microsoft.com/en-us/library/ee330730(v=vs.85).aspx#browser_emulation

    // set version
    switch(version)
    {
    case 11:    return setBinaryFeature(binaryName, XWBROWSER_FEATURE_EMULATION, 0x02AF8);
    case 10:    return setBinaryFeature(binaryName, XWBROWSER_FEATURE_EMULATION, 0x02710);
    case 9:     return setBinaryFeature(binaryName, XWBROWSER_FEATURE_EMULATION, 0x02328);
    case 8:     return setBinaryFeature(binaryName, XWBROWSER_FEATURE_EMULATION, 0x01F40);
    case 7:     return setBinaryFeature(binaryName, XWBROWSER_FEATURE_EMULATION, 0x01B58);

    default:
        XWTRACE("XWBrowserHelpers: unsupported version requested");
        return E_INVALIDARG;
    }
}

LONG XWBrowserHelpers::setMaxEmulationVersion(const WCHAR* binaryName)
{
    int major, minor;

    // get installed version
    LONG lRes = getInstalledVersion(major, minor);
    if(lRes == ERROR_SUCCESS)
    {
        // set version
        return setEmulationVersion(binaryName, major);
    }

    return lRes;
}

LONG XWBrowserHelpers::enableGPURendering(const WCHAR* binaryName, BOOL enable)
{
    if(enable)
        return setBinaryFeature(binaryName, XWBROWSER_FEATURE_GPU_RENDERING, 1);
    else
        return removeBinaryFeature(binaryName, XWBROWSER_FEATURE_GPU_RENDERING);
}

/////////////////////////////////////////////////////////////////////
// language settings
/////////////////////////////////////////////////////////////////////
LONG XWBrowserHelpers::readLanguage(std::wstring& value)
{
    HKEY hKey;
    LONG lRes = NOERROR;

    // reset return value
    value.clear();

    // open key
    lRes = ::RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Internet Explorer\\International", 0, KEY_READ, &hKey);
    if(lRes == ERROR_SUCCESS)
    {
        // read key value size
        DWORD dwValueSize = 0;
        lRes = ::RegQueryValueEx(hKey, L"AcceptLanguage", 0, 0, 0, &dwValueSize);
        if(lRes == ERROR_SUCCESS)
        {
            // allocate space
            value.resize(dwValueSize + 1, 0);

            // read value
            lRes = ::RegQueryValueExW(hKey, L"AcceptLanguage", 0, 0, (LPBYTE)&value[0], &dwValueSize);

            // resize
            value.resize(::wcslen(value.c_str()));
        }

        // close key
        ::RegCloseKey(hKey);
    }

    return lRes;
}

LONG XWBrowserHelpers::setLanguage(const WCHAR* value)
{
    // use default key
    return setLanguage(L"Software\\Microsoft\\Internet Explorer", value);
}

LONG XWBrowserHelpers::setLanguage(const WCHAR* subkey, const WCHAR* value)
{
    XWASSERT(subkey);
    XWASSERT(value);
    if(subkey == 0 || value == 0) return E_INVALIDARG;

    HKEY hKey;
    LONG lRes = NOERROR;

    // open key
    lRes = ::RegOpenKeyExW(HKEY_CURRENT_USER, subkey, 0, KEY_WRITE, &hKey);

    // create key if it doesn't exist
    if(lRes == ERROR_FILE_NOT_FOUND)
    {
        lRes = ::RegCreateKeyExW(HKEY_CURRENT_USER, subkey, 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &hKey, 0);
    }

    if(lRes == ERROR_SUCCESS)
    {
        HKEY hSubKey;

        // open key
        lRes = ::RegOpenKeyExW(hKey, L"International", 0, KEY_WRITE, &hSubKey);

        // create key if it doesn't exist
        if(lRes == ERROR_FILE_NOT_FOUND)
        {
            lRes = ::RegCreateKeyExW(hKey, L"International", 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &hSubKey, 0);
        }

        if(lRes == ERROR_SUCCESS)
        {
            // set value
            lRes = ::RegSetValueExW(hSubKey, L"AcceptLanguage", 0, REG_SZ, (BYTE*)value, (DWORD)(sizeof(WCHAR) * (::wcslen(value) + 1)));

            // close key
            ::RegCloseKey(hSubKey);
        }

        // close key
        ::RegCloseKey(hKey);
    }

    return lRes;
}

// XWBrowserHelpers
/////////////////////////////////////////////////////////////////////
