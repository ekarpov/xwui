// Content provider default implementation
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWCONTENTPROVIDER_H_
#define _XWCONTENTPROVIDER_H_

/////////////////////////////////////////////////////////////////////

// NOTE: content provider accepts callback pointer or window as an events callbacks.
//       If window is used it should monitor following events:
//        - WM_XWUI_URL_CONTENT_LOADED
//        - WM_XWUI_URL_CONTENT_LOAD_FAILED

/////////////////////////////////////////////////////////////////////
// IXWContentProviderCallback - content provider callback interface

interface IXWContentProviderCallback : public IUnknown
{
    virtual void    onUrlContentLoaded(DWORD id, const WCHAR* path) = 0;
    virtual void    onUrlContentLoadFailed(DWORD id, DWORD reason) = 0;
};

// IXWContentProviderCallback
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// IXWContentProvider - content provider interface

interface IXWContentProvider : public IUnknown
{
    virtual bool    isUrlContentLoaded(const WCHAR* url, XMediaSource& srcOut) = 0;
    virtual bool    loadUrlContent(const WCHAR* url, IXWContentProviderCallback* callback, DWORD& idOut) = 0;
    virtual bool    loadUrlContent(const WCHAR* url, HWND callbackWindow, DWORD& idOut) = 0;
    virtual void    cancelContentLoad(DWORD id) = 0;
};

// IXWContentProvider
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// XWContentProvider - content provider instance

namespace XWContentProvider
{
    // global provider instance
    IXWContentProvider* instance();
    void        setInstance(IXWContentProvider* provider);
    void        releaseInstance();
};

// XWContentProvider
/////////////////////////////////////////////////////////////////////

#endif // _XWCONTENTPROVIDER_H_

