// ActiveX control 
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "../layout/xlayoutitem.h"
#include "../layout/xlayout.h"

#include "../xwindow/xhwnd.h"
#include "../xwindow/xwindow.h"

#include "xwactiveimpl.h"
#include "xwactive.h"

/////////////////////////////////////////////////////////////////////
// XWActive - text label window

XWActive::XWActive(HWND hWndParent, XWObject* parent, DWORD dwStyle, DWORD dwExStyle) :
    XWindow(dwStyle, parent, hWndParent, dwExStyle)
{
    // create implementation object
    m_pImpl = new XWActiveImpl(hwnd());
    m_pImpl->AddRef();
}

XWActive::~XWActive()
{
    // release control if any
    release();

    // delete implementation
    m_pImpl->Release();
}

/////////////////////////////////////////////////////////////////////
// interface
bool XWActive::load(const wchar_t* clsid)
{
    XWASSERT(m_pImpl);

    // load active object
    if(!m_pImpl->loadActiveObject(clsid))
    {
        return false;
    }

    // activate in place
    return m_pImpl->activateInPlace();
}

void XWActive::release()
{
    XWASSERT(m_pImpl);

    // release 
    m_pImpl->releaseObject();
}

bool XWActive::isLoaded()
{
    XWASSERT(m_pImpl);

    // pass to implementation
    return m_pImpl->isActiveObjectLoaded();
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
bool XWActive::queryObjectInterface(REFIID riid, void** ppvObject)
{
    XWASSERT(m_pImpl);

    // pass to implementation
    return m_pImpl->queryObjectInterface(riid, ppvObject);
}

/////////////////////////////////////////////////////////////////////
// events
/////////////////////////////////////////////////////////////////////
void XWActive::onResize(int type, int width, int height)
{
    XWASSERT(m_pImpl);

    // ignore if not loaded
    if(!m_pImpl->isActiveObjectLoaded()) return;

    RECT rec;
    rec.left = 0;
    rec.top = 0;
    rec.right = width;
    rec.bottom = height;

    // pass new size
    m_pImpl->resize(rec);
}

// XWActive
/////////////////////////////////////////////////////////////////////
