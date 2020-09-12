// Tab control
//
/////////////////////////////////////////////////////////////////////

#ifndef _XTABCTRL_H_
#define _XTABCTRL_H_

/////////////////////////////////////////////////////////////////////
// XTabCtrl - tab control

class XTabCtrl : public XWHWND
{
public: // construction/destruction
    XTabCtrl(HWND hWndParent, XWObject* parent = 0, DWORD dwStyle = 0, DWORD dwExStyle = 0);
    ~XTabCtrl();

public: // properties
    int     getSelectedTab();
    void    selectTab(unsigned int tabIndex);
    void    getDisplayRect(RECT& rectOut);

public: // manage tabs
    bool    insertTab(unsigned int tabIndex, WCHAR* text);
    bool    setTabText(unsigned int tabIndex, WCHAR* text);
    bool    removeTab(unsigned int tabIndex);
};

// XTabCtrl
/////////////////////////////////////////////////////////////////////

#endif // _XTABCTRL_H_

