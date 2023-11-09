// Content provider default implementation
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWCONTENTPROVIDERIMPL_H_
#define _XWCONTENTPROVIDERIMPL_H_

/////////////////////////////////////////////////////////////////////
// XWContentProviderImpl - content provider base implementation

class XWContentProviderImpl : public IXWContentProvider
{
public: // construction/destruction
    XWContentProviderImpl();
    virtual ~XWContentProviderImpl();

public: // IXWContentProvider
    virtual bool    isUrlContentLoaded(const WCHAR* url, XMediaSource& srcOut);
    virtual bool    loadUrlContent(const WCHAR* url, IXWContentProviderCallback* callback, DWORD& idOut);
    virtual bool    loadUrlContent(const WCHAR* url, HWND callbackWindow, DWORD& idOut);
    virtual void    cancelContentLoad(DWORD id);

public: // IUnknown 
    STDMETHODIMP            QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

protected: // worker methods
    bool    _parseUrlSchema(const WCHAR* url, const WCHAR** schemaOut, size_t* lengthOut);

private: // data
    unsigned long           m_ulRef;
};

// XWContentProviderImpl
/////////////////////////////////////////////////////////////////////

#endif // _XWCONTENTPROVIDERIMPL_H_

