// Content provider default implementation
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xwcontentprovider.h"
#include "xwcontentproviderimpl.h"

/////////////////////////////////////////////////////////////////////
// global data

// global instance
static IXWContentProvider*    g_IXWContentProviderInstance = 0;

/////////////////////////////////////////////////////////////////////
// XWContentProvider - content provider instance

/////////////////////////////////////////////////////////////////////
// global provider instance
/////////////////////////////////////////////////////////////////////
IXWContentProvider* XWContentProvider::instance()
{
    // create base implementation if not set
    if(g_IXWContentProviderInstance == 0)
    {
        g_IXWContentProviderInstance = new XWContentProviderImpl();
        g_IXWContentProviderInstance->AddRef();
    }

    return g_IXWContentProviderInstance;
}

void XWContentProvider::setInstance(IXWContentProvider* provider)
{
    // release previous instance if any
    releaseInstance();

    // copy
    if(provider)
    {
        g_IXWContentProviderInstance = provider;
        g_IXWContentProviderInstance->AddRef();
    }
}

void XWContentProvider::releaseInstance()
{
    if(g_IXWContentProviderInstance) g_IXWContentProviderInstance->Release();
    g_IXWContentProviderInstance = 0;
}

// XWContentProvider
/////////////////////////////////////////////////////////////////////
