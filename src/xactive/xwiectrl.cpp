// Internet Explorer ActiveX control 
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

// ActiveX support
#include <exdisp.h> 

#include "../layout/xlayoutitem.h"
#include "../layout/xlayout.h"

#include "../xwindow/xhwnd.h"
#include "../xwindow/xwindow.h"

#include "xwactiveimpl.h"
#include "xwbrowserhelpers.h"
#include "xwiectrl.h"

/////////////////////////////////////////////////////////////////////
// XWIECtrl - Internet Explorer ActiveX control host

XWIECtrl::XWIECtrl(DWORD dwStyle, XWObject* parent, HWND hWndParent, DWORD dwExStyle) :
    XWindow(dwStyle, parent, hWndParent, dwExStyle),
    m_pWebBrowser2(0),
    m_allowedKeys(eAllowedKeyAll)
{
    // create implementation object
    m_pImpl = new XWActiveImpl(hwnd());
    m_pImpl->AddRef();

    // set focusable
    setFocusable(true);
}

XWIECtrl::~XWIECtrl()
{
    // close resources
    close();

    // delete implementation
    m_pImpl->Release();

    // remove itself from message hooks
    sXWUIRemoveMessageHook(messageHookId());
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
bool XWIECtrl::init()
{
    XWASSERT(m_pImpl);

    // close previous instance if any
    close();

    // load IE active object
    if(!m_pImpl->loadActiveObject(CLSID_WebBrowser))
    {
        XWTRACE("XWIECtrl: failed to load Internet Explorer ActiveX");
        return false;
    }

    // activate in place
    if(!m_pImpl->activateInPlace())
    {
        XWTRACE("XWIECtrl: failed to activate Internet Explorer ActiveX");
        close();
        return false;
    }

    // get web browser interface
    if(!m_pImpl->queryObjectInterface(IID_IWebBrowser2, (LPVOID*)&m_pWebBrowser2))
    {
        XWTRACE("XWIECtrl: failed to get IWebBrowser2 interface from Internet Explorer ActiveX");
        close();
        return false;
    }

    // loaded
    return true;
}

void XWIECtrl::close()
{
    XWASSERT(m_pImpl);
    if(m_pImpl == 0) return;

    // release web browser interface first
    if(m_pWebBrowser2)
    {
        m_pWebBrowser2->Release();
        m_pWebBrowser2 = 0;
    }

    // release control
    m_pImpl->releaseObject();
}

/////////////////////////////////////////////////////////////////////
// settings
/////////////////////////////////////////////////////////////////////
void XWIECtrl::setShowScrollBars(bool bShow)
{
    if(m_pWebBrowser2)
        XWBrowserHelpers::setShowScrollBars(m_pWebBrowser2, bShow);
}

/////////////////////////////////////////////////////////////////////
// allowed keys
/////////////////////////////////////////////////////////////////////
void XWIECtrl::setAllowedKeys(unsigned short keys)
{
    // copy flag
    m_allowedKeys = keys;

    if(m_allowedKeys != eAllowedKeyNone)
    {
        // add itself as message hook
        sXWUIAddMessageHook(this);

    } else
    {
        // remove itself from message hooks
        sXWUIRemoveMessageHook(messageHookId());
    }
}

/////////////////////////////////////////////////////////////////////
// scrolling (from XWHWND)
/////////////////////////////////////////////////////////////////////
int XWIECtrl::contentWidth()
{
    long clientHeight, clientWidth;

    // get size
    getContentSize(clientHeight, clientWidth);

    return clientWidth;
}

int XWIECtrl::contentHeight()
{
    long clientHeight, clientWidth;

    // get size
    getContentSize(clientHeight, clientWidth);

    return clientHeight;
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
void XWIECtrl::getContentSize(long& clientHeight, long& clientWidth)
{
    // reset output
    clientHeight = 0;
    clientWidth = 0;

    // get size
    if(m_pWebBrowser2)
        XWBrowserHelpers::getContentSize(m_pWebBrowser2, clientHeight, clientWidth);
}

/////////////////////////////////////////////////////////////////////
// customizations
/////////////////////////////////////////////////////////////////////
void XWIECtrl::setCustomizations(IUnknown* pCustomInterfaces)
{
    XWASSERT(m_pImpl);
    if(m_pImpl) m_pImpl->setCustomizations(pCustomInterfaces);
}

/////////////////////////////////////////////////////////////////////
// browser interface
/////////////////////////////////////////////////////////////////////
IWebBrowser2* XWIECtrl::webBrowser()
{
    XWASSERT(m_pWebBrowser2);
    return m_pWebBrowser2;
}

/////////////////////////////////////////////////////////////////////
// message hooks (from IXWMessageHook)
/////////////////////////////////////////////////////////////////////
bool XWIECtrl::processHookMessage(MSG* pmsg)
{
    XWASSERT(m_pImpl);
    XWASSERT(pmsg);
    if(m_pImpl == 0 || pmsg == 0) return false;

    // ignore if not visible
    if(!isVisible()) return false;

    // ignore if keys are not allowed
    if(m_allowedKeys == eAllowedKeyNone) return false;

    // check if we need to process all keys
    if(m_allowedKeys == eAllowedKeyAll) 
    {
        // pass to implementation object
        return m_pImpl->translateAccelerator(pmsg);
    }

    // copy and paste
    if(m_allowedKeys & eAllowedKeyCopyPaste)
    {
        // translate Ctrl+C, Ctrl+V, Ctrl+X, Ctrl+Z
        if (pmsg->message == WM_KEYDOWN && (::GetKeyState(VK_CONTROL) & 0x8000) &&
            (pmsg->wParam == XW_VK_C || pmsg->wParam == XW_VK_V || pmsg->wParam == XW_VK_X || pmsg->wParam == XW_VK_Z))
        {
            // pass to implementation object
            return m_pImpl->translateAccelerator(pmsg);
        }
    }

    // delete
    if((m_allowedKeys & eAllowedKeyDelete) && pmsg->wParam == VK_DELETE)
    {
        // pass to implementation object
        return m_pImpl->translateAccelerator(pmsg);
    }

    // translate TAB if needed
    if((m_allowedKeys & eAllowedKeyTab) && pmsg->wParam == VK_TAB)
    {
        // pass to implementation object
        return m_pImpl->translateAccelerator(pmsg);
    }

    // ignore other keys
    return false;
}

unsigned long XWIECtrl::messageHookId() const
{
    // use object id as message hook id
    return xwoid();
}

/////////////////////////////////////////////////////////////////////
// events
/////////////////////////////////////////////////////////////////////
void XWIECtrl::onResize(int type, int width, int height)
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

// XWIECtrl
/////////////////////////////////////////////////////////////////////
