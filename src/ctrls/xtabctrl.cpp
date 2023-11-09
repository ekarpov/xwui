// Tab control
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "../layout/xlayoutitem.h"

#include "../xwindow/xhwnd.h"

#include "xtabctrl.h"

/////////////////////////////////////////////////////////////////////
// XTabCtrl - tab control

XTabCtrl::XTabCtrl(HWND hWndParent, XWObject* parent, DWORD dwStyle, DWORD dwExStyle) :
    XWHWND(0, parent)
{
    XWASSERT(hWndParent);

    // create tab control
    m_hWnd = CreateWindowExW( 
        dwExStyle,
        WC_TABCONTROLW,   
        0,       
        WS_VISIBLE | WS_CHILD | dwStyle,
        0, 0,               // starting position 
        100, 14,            // default size
        hWndParent,         // parent window 
        NULL,               // No menu 
        ::GetModuleHandle(NULL), 
        NULL);      

    // check result
    if(m_hWnd == NULL)
    {
        // fatal error
        XWTRACE_WERR_LAST("XTabCtrl: Failed to create window");
        return;
    }

    // set default font 
    setDefaultFont();    
}

XTabCtrl::~XTabCtrl()
{
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
int XTabCtrl::getSelectedTab()
{
    return (int) ::SendMessageW(hwnd(), TCM_GETCURSEL, 0, 0);
}

void XTabCtrl::selectTab(unsigned int tabIndex)
{
    if(::SendMessageW(hwnd(), TCM_SETCURSEL, tabIndex, 0) == -1)
    {
        XWTRACE_WERR_LAST("XTabCtrl: Failed to select tab");
    }
}

void XTabCtrl::getDisplayRect(RECT& rectOut)
{
    // get window rect for control
    rectOut.left = 0;
    rectOut.top = 0;
    rectOut.right = width();
    rectOut.bottom = height();

    // get display area from control
    ::SendMessageW(hwnd(), TCM_ADJUSTRECT, FALSE, (LPARAM)&rectOut);
}

/////////////////////////////////////////////////////////////////////
// manage tabs
/////////////////////////////////////////////////////////////////////
bool XTabCtrl::insertTab(unsigned int tabIndex, WCHAR* text)
{
    // fill tab
    TCITEMW tiew;
    tiew.mask = TCIF_TEXT; 
    tiew.iImage = -1; 
    tiew.pszText = text; 
    
    // insert
    if(::SendMessageW(hwnd(), TCM_INSERTITEMW, tabIndex, (LPARAM)&tiew) == -1)
    {
        // fatal error
        XWTRACE_WERR_LAST("XTabCtrl: Failed to insert tab");
        return false;
    }

    return true;
}

bool XTabCtrl::setTabText(unsigned int tabIndex, WCHAR* text)
{
    // fill tab
    TCITEMW tiew;
    tiew.mask = TCIF_TEXT; 
    tiew.iImage = -1; 
    tiew.pszText = text; 
    
    // insert
    if(::SendMessageW(hwnd(), TCM_SETITEMW, tabIndex, (LPARAM)&tiew) != TRUE)
    {
        // fatal error
        XWTRACE_WERR_LAST("XTabCtrl: Failed to set tab");
        return false;
    }

    return true;
}

bool XTabCtrl::removeTab(unsigned int tabIndex)
{
    // delete
    if(::SendMessageW(hwnd(), TCM_DELETEITEM, tabIndex, 0) != TRUE)
    {
        // fatal error
        XWTRACE_WERR_LAST("XTabCtrl: Failed to remove tab");
        return false;
    }

    return true;
}

// XTabCtrl
/////////////////////////////////////////////////////////////////////

