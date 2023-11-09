// ITextServices interface helpers
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"

// text services 
#include <richedit.h>
#include <richole.h>
#include <textserv.h>

#include "xtextservices.h"

/////////////////////////////////////////////////////////////////////
// text services interfaces

EXTERN_C const IID IID_ITextServices = { // 8d33f740-cf58-11ce-a89d-00aa006cadc5
    0x8d33f740,
    0xcf58,
    0x11ce,
    {0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
  };

EXTERN_C const IID IID_ITextHost = { /* c5bdd8d0-d26e-11ce-a89e-00aa006cadc5 */
    0xc5bdd8d0,
    0xd26e,
    0x11ce,
    {0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
  };

/////////////////////////////////////////////////////////////////////
// text document interfaces

// NOTE: Text Object Model (TOM) GUIDs are given in Tom.h inside the MIDL_INTERFACE statements. 
//       To use the associated interfaces, you must first declare the interface by using the GUID.
//       https://msdn.microsoft.com/en-us/library/windows/desktop/hh270409(v=vs.85).aspx

EXTERN_C const IID IID_ITextDocument = { // 8CC497C0-A1DF-11ce-8098-00AA0047BE5D
    0x8CC497C0,
    0xA1DF,
    0x11CE,
    {0x80, 0x98, 0x00, 0xAA, 0x00, 0x47, 0xBE, 0x5D}
};

EXTERN_C const IID IID_ITextSelection = { // 8CC497C1-A1DF-11ce-8098-00AA0047BE5D
    0x8CC497C1,
    0xA1DF,
    0x11CE,
    {0x80, 0x98, 0x00, 0xAA, 0x00, 0x47, 0xBE, 0x5D}
};

EXTERN_C const IID IID_ITextRange = { // 8CC497C2-A1DF-11ce-8098-00AA0047BE5D
    0x8CC497C2,
    0xA1DF,
    0x11CE,
    {0x80, 0x98, 0x00, 0xAA, 0x00, 0x47, 0xBE, 0x5D}
};

EXTERN_C const IID IID_ITextFont = { // 8CC497C3-A1DF-11ce-8098-00AA0047BE5D
    0x8CC497C3,
    0xA1DF,
    0x11CE,
    {0x80, 0x98, 0x00, 0xAA, 0x00, 0x47, 0xBE, 0x5D}
};

EXTERN_C const IID IID_ITextPara = { // 8CC497C4-A1DF-11ce-8098-00AA0047BE5D
    0x8CC497C4,
    0xA1DF,
    0x11CE,
    {0x80, 0x98, 0x00, 0xAA, 0x00, 0x47, 0xBE, 0x5D}
};

EXTERN_C const IID IID_ITextStoryRanges = { // 8CC497C5-A1DF-11ce-8098-00AA0047BE5D
    0x8CC497C5,
    0xA1DF,
    0x11CE,
    {0x80, 0x98, 0x00, 0xAA, 0x00, 0x47, 0xBE, 0x5D}
};

EXTERN_C const IID IID_ITextDocument2 = { // 01c25500-4268-11d1-883a-3c8b00c10000
    0x01c25500,
    0x4268,
    0x11d1,
    {0x88, 0x3a, 0x3c, 0x8b, 0x00, 0xc1, 0x00, 0x00}
};

/////////////////////////////////////////////////////////////////////
// global constants

// export functions
typedef HRESULT (WINAPI *CreateTextServicesPtr)(IUnknown*, ITextHost*, IUnknown**);

// DLL instance
HINSTANCE g_hRichEditLibrary = 0;

// text services function pointer
CreateTextServicesPtr g_pCreateTextServicesFunc = 0;

// text services interface IID
const IID*  g_pTextServicesInterfaceID = &IID_ITextServices;
const IID*  g_pTextHostInterfaceID = &IID_ITextHost;

/////////////////////////////////////////////////////////////////////
// XTextServices - ITextServices helper functions

/////////////////////////////////////////////////////////////////////
// init services
/////////////////////////////////////////////////////////////////////
bool XTextServices::initTextServices()
{
    // ignore if already loaded
    if(g_pCreateTextServicesFunc != 0) return true;

    // try to find library first
    HMODULE richeditlib = ::GetModuleHandleW(L"Msftedit.dll");

    // if library is not loaded try to load it
    if(richeditlib == 0)
    {
        // try to load
        g_hRichEditLibrary = ::LoadLibraryW(L"Msftedit.dll");
        if(g_hRichEditLibrary == 0)
        {
            XWTRACE_WERR_LAST("XTextServices: failed to load RichEdit DLL");
            return false;
        }

        richeditlib = g_hRichEditLibrary;
    }

    // get function pointer
    g_pCreateTextServicesFunc = (CreateTextServicesPtr)::GetProcAddress(richeditlib, "CreateTextServices");
    if(g_pCreateTextServicesFunc == 0)
    {
        XWTRACE_WERR_LAST("XTextServices: failed to locate CreateTextServices from RichEdit DLL");
        return false;
    }

    // update interface IIDs

    // NOTE: this code is based on MSDN article:
    //       https://msdn.microsoft.com/en-us/library/windows/desktop/bb787613(v=vs.85).aspx

    // get interface from library
    g_pTextServicesInterfaceID = (IID*) (VOID*) GetProcAddress(richeditlib, "IID_ITextServices");

    // set to default if not found
    if(g_pTextServicesInterfaceID == 0)
        g_pTextServicesInterfaceID = &IID_ITextServices;

    // get interface from library
    g_pTextHostInterfaceID = (IID*) (VOID*) GetProcAddress(richeditlib, "IID_ITextHost");

    // set to default if not found
    if(g_pTextHostInterfaceID == 0)
        g_pTextHostInterfaceID = &IID_ITextHost;

    return true;
}

void XTextServices::releaseTextServices()
{
    // free library
    if(!::FreeLibrary(g_hRichEditLibrary))
    {
        XWTRACE_WERR_LAST("XTextServices: failed to release RichEdit library");
    }

    // reset pointers
    g_hRichEditLibrary = 0;
    g_pCreateTextServicesFunc = 0;
}

/////////////////////////////////////////////////////////////////////
// service IID's (IID_ITextServices, IID_ITextHost)
/////////////////////////////////////////////////////////////////////
const IID* XTextServices::textServicesIID()
{
    // init text services just in case
    if(g_pCreateTextServicesFunc == 0)
        initTextServices();

    return g_pTextServicesInterfaceID;
}

const IID* XTextServices::textHostIID()
{
    // init text services just in case
    if(g_pCreateTextServicesFunc == 0)
        initTextServices();

    return g_pTextHostInterfaceID;
}

/////////////////////////////////////////////////////////////////////
// create text services
/////////////////////////////////////////////////////////////////////
bool XTextServices::createTextServices(HWND hwnd, const RECT& rcClient, ITextHost* textHost, ITextServices** textServicesOut)
{
    // check input
    XWASSERT(hwnd);
    XWASSERT(textHost);
    XWASSERT(textServicesOut);
    if(hwnd == 0 || textHost == 0 || textServicesOut == 0) return false;

    // init richedit library first
    if(!initTextServices()) return false;

    // create text services interface
    IUnknown* tmpTextServices = 0;
    HRESULT hr = g_pCreateTextServicesFunc(0, textHost, &tmpTextServices);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XTextServices: failed to create text services", hr);
        return false;
    }

    // get text services interface
    hr = tmpTextServices->QueryInterface(*g_pTextServicesInterfaceID, (void**)textServicesOut);

    // release temp interface
    tmpTextServices->Release();
    tmpTextServices = 0;

    // check result
    if(FAILED(hr))
    {
        XWTRACE_HRES("XTextServices: failed to create text services interface", hr);
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////
// text object model
/////////////////////////////////////////////////////////////////////
HRESULT XTextServices::queryTextDocument(IRichEditOle* richEditOle, ITextDocument** textDocumentOut)
{
    // check input
    XWASSERT(richEditOle);
    XWASSERT(textDocumentOut);
    if(richEditOle == 0 || textDocumentOut == 0) return E_INVALIDARG;

    // query interface
    return richEditOle->QueryInterface(IID_ITextDocument, (LPVOID*)textDocumentOut);
}

HRESULT XTextServices::queryTextRange(IRichEditOle* richEditOle, LONG textPos, LONG textLength, ITextRange** textRangeOut)
{
    // check input
    XWASSERT(richEditOle);
    XWASSERT(textRangeOut);
    if(richEditOle == 0 || textRangeOut == 0) return E_INVALIDARG;

    // query document first
    ITextDocument* textDocument = 0;
    HRESULT hr = queryTextDocument(richEditOle, &textDocument);
    if(FAILED(hr)) return hr;

    // query document range
    *textRangeOut = 0;
    hr = textDocument->Range(textPos, textPos + textLength, textRangeOut);

    // release document
    textDocument->Release();
    textDocument = 0;

    // result
    return hr;
}

HRESULT XTextServices::queryTextRangeFont(IRichEditOle* richEditOle, LONG textPos, LONG textLength, ITextFont** textFontOut)
{
    // check input
    XWASSERT(richEditOle);
    XWASSERT(textFontOut);
    if(richEditOle == 0 || textFontOut == 0) return E_INVALIDARG;

    // query range first
    ITextRange* textRange = 0;
    HRESULT hr = queryTextRange(richEditOle, textPos, textLength, &textRange);
    if(FAILED(hr)) return hr;

    // query range font
    *textFontOut = 0;
    hr = textRange->GetFont(textFontOut);

    // release range
    textRange->Release();
    textRange = 0;

    // result
    return hr;
}

HRESULT XTextServices::queryTextSelection(IRichEditOle* richEditOle, ITextSelection** textSelectionOut)
{
    // check input
    XWASSERT(richEditOle);
    XWASSERT(textSelectionOut);
    if(richEditOle == 0 || textSelectionOut == 0) return E_INVALIDARG;

    // query document first
    ITextDocument* textDocument = 0;
    HRESULT hr = queryTextDocument(richEditOle, &textDocument);
    if(FAILED(hr)) return hr;

    // query document selection
    *textSelectionOut = 0;
    hr = textDocument->GetSelection(textSelectionOut);

    // release document
    textDocument->Release();
    textDocument = 0;

    // result
    return hr;
}

// XTextServices
/////////////////////////////////////////////////////////////////////
