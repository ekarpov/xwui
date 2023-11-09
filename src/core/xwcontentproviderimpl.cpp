// Content provider default implementation
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xwcontentprovider.h"
#include "xwcontentproviderimpl.h"

/////////////////////////////////////////////////////////////////////
// XWContentProviderImpl - content provider 

/////////////////////////////////////////////////////////////////////
// construction/destruction
/////////////////////////////////////////////////////////////////////
XWContentProviderImpl::XWContentProviderImpl() :
    m_ulRef(0)
{
}

XWContentProviderImpl::~XWContentProviderImpl()
{
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
bool XWContentProviderImpl::isUrlContentLoaded(const WCHAR* url, XMediaSource& srcOut)
{
    XWASSERT(url);
    if(url == 0) return false;

    const WCHAR* schema;
    size_t len;

    // parse schema
    if(_parseUrlSchema(url, &schema, &len))
    {
        // checm supported schemas
        if(::wcsncmp(schema, L"file://", len) == 0)
        {
            // file
            srcOut.setFilePath(url + len);

        } else if(::wcsncmp(schema, L"style://", len) == 0)
        {
            // style
            srcOut.setStylePath(url + len);

        } else if(::wcsncmp(schema, L"res://", len) == 0)
        {
            // resource
            srcOut.setResource(::GetModuleHandle(0), url + len, RT_RCDATA);

        } else
        {
            // unknown schema
            return false;
        }

    } else
    {
        // if schema is not set assume url is simple file name
        srcOut.setFilePath(url);
    }

    // url points to local resource
    return true;
}

bool XWContentProviderImpl::loadUrlContent(const WCHAR* url, IXWContentProviderCallback* callback, DWORD& idOut)
{
    // NOTE: not supported in default implementation
    return false;
}

bool XWContentProviderImpl::loadUrlContent(const WCHAR* url, HWND callbackWindow, DWORD& idOut)
{
    // NOTE: not supported in default implementation
    return false;
}

void XWContentProviderImpl::cancelContentLoad(DWORD id)
{
    // NOTE: not supported in default implementation
}

/////////////////////////////////////////////////////////////////////
// IUnknown 
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XWContentProviderImpl::QueryInterface(REFIID riid, void** ppvObject)
{
    // reset pointer first
    *ppvObject = 0;

    ///// check required interface

    // IUnknown
    if(riid == IID_IUnknown)
        *ppvObject = (IUnknown*)this;

    // check if interface is not supported
    if (!*ppvObject)
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) XWContentProviderImpl::AddRef()
{
    return ::InterlockedIncrement(&m_ulRef);
}

STDMETHODIMP_(ULONG) XWContentProviderImpl::Release()
{
    if(::InterlockedDecrement(&m_ulRef) == 0)
    {
        delete this;
        return 0;
    }

    return m_ulRef;
}


/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
bool XWContentProviderImpl::_parseUrlSchema(const WCHAR* url, const WCHAR** schemaOut, size_t* lengthOut)
{
    XWASSERT(url);
    if(url == 0) return false;

    // reset output
    *schemaOut = 0;
    *lengthOut = 0;

    // skip spaces
    while(*url != 0 && ::iswspace(*url)) ++url;

    // find schema 
    size_t pos = 0;
    while(url[pos] != 0)
    {
        // check for schema separator
        if(url[pos] == L':' && url[pos+1] == L'/' && url[pos+2] == L'/')
        {
            // set output
            *schemaOut = url;
            *lengthOut = pos + 3;

            // schema found
            return true;
        }

        // next
        pos++;
    }

    // not found
    return false;
}

// XWContentProviderImpl
/////////////////////////////////////////////////////////////////////

