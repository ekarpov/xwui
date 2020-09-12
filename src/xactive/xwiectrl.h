// Internet Explorer ActiveX control 
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWIECTRL_H_
#define _XWIECTRL_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
class XWActiveImpl;
struct IWebBrowser2;

/////////////////////////////////////////////////////////////////////
// XWIECtrl - Internet Explorer ActiveX control host

class XWIECtrl : public XWindow,
                 public IXWMessageHook
{
public: // construction/destruction
    XWIECtrl(DWORD dwStyle, XWObject* parent = 0, HWND hWndParent = 0, DWORD dwExStyle = 0);
    ~XWIECtrl();

public: // interface
    bool    init();
    void    close();

public: // settings
    void    setShowScrollBars(bool bShow);

public: // allowed keys
    enum TAllowedKey
    {
        eAllowedKeyNone         = 0x0000,
        eAllowedKeyTab          = 0x0001,
        eAllowedKeyCopyPaste    = 0x0002,
        eAllowedKeyDelete       = 0x0004,
        eAllowedKeyAll          = 0xFFFF,
    };

    void    setAllowedKeys(unsigned short keys);

public: // scrolling (from XWHWND)
    int     contentWidth();
    int     contentHeight();

public: // properties
    void    getContentSize(long& clientHeight, long& clientWidth);

public: // customizations
    void    setCustomizations(IUnknown* pCustomInterfaces);

public: // state
    bool    isBrowserLoaded() const { return m_pWebBrowser2 != 0; };

public: // browser interface
    IWebBrowser2*   webBrowser();

public: // message hooks (from IXWMessageHook)
    bool            processHookMessage(MSG* pmsg);
    unsigned long   messageHookId() const;

private: // events
    void    onResize(int type, int width, int height);

private: // data
    XWActiveImpl*   m_pImpl;
    IWebBrowser2*   m_pWebBrowser2;
    unsigned short  m_allowedKeys;
};

// XWIECtrl
/////////////////////////////////////////////////////////////////////

#endif // _XWIECTRL_H_

