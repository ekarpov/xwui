// ActiveX control 
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWACTIVE_H_
#define _XWACTIVE_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
class XWActiveImpl;

/////////////////////////////////////////////////////////////////////
// XWActive - ActiveX control host

class XWActive : public XWindow
{
public: // construction/destruction
    XWActive(HWND hWndParent, XWObject* parent = 0, DWORD dwStyle = 0, DWORD dwExStyle = 0);
    ~XWActive();

public: // load ActiveX
    bool    load(const wchar_t* clsid);
    void    release();
    bool    isLoaded();

public: // interface
    bool    queryObjectInterface(REFIID riid, void** ppvObject);

private: // events
    void    onResize(int type, int width, int height);

private: // data
    XWActiveImpl*   m_pImpl;
};

// XWActive
/////////////////////////////////////////////////////////////////////

#endif // _XWACTIVE_H_

