// Push button
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "../layout/xlayoutitem.h"

#include "../xwindow/xhwnd.h"

#include "xpushbutton.h"

/////////////////////////////////////////////////////////////////////
// XPushButton - push button window
XPushButton::XPushButton(const wchar_t* text, HWND hWndParent, XWObject* parent, DWORD dwStyle, DWORD dwExStyle) :
    XWHWND(0, parent)
{
    XWASSERT(hWndParent);

    // create button
    m_hWnd = CreateWindowExW( 
        dwExStyle,
        WC_BUTTONW,   
        text,       
        WS_VISIBLE | WS_CHILD | dwStyle,
        0,0,            // starting position 
        50,14,          // default size
        hWndParent,     // parent window 
        NULL,           // No menu 
        ::GetModuleHandle(NULL), 
        NULL);      

    // check result
    if(m_hWnd == NULL)
    {
        // fatal error
        XWTRACE_WERR_LAST("XPushButton: Failed to create window");
        return;
    }

    // set default font 
    setDefaultFont();

    // NOTE: for default sizes refer to MSDN:
    //       http://msdn.microsoft.com/en-us/library/ms997619.aspx

    // NOTE: font has to be set before
    // get default button dimensions
    LONG units = XWUtils::sGetDialogBaseUnits(m_hWnd);
    int width = MulDiv(LOWORD(units), 60, 4);   // NOTE: use slightly bigger (50 is default)
    int height = MulDiv(HIWORD(units), 14, 8);

    // fix default button size
    XWHWND::setFixedSize(width, height);
}

XPushButton::~XPushButton()
{
}

// XPushButton
/////////////////////////////////////////////////////////////////////

