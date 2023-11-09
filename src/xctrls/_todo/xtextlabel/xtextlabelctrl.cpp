// Extended text label control
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"

#include "../../layout/xlayoutitem.h"

#include "../../xwindow/xhwndlayoutitem.h"

#include "xtextlabelwnd.h"
#include "xtextlabelctrl.h"

/////////////////////////////////////////////////////////////////////
// XTextLabel - extended text label

XTextLabel::XTextLabel(const wchar_t* text, HWND hWndParent, XWObject* parent, DWORD dwStyle, DWORD dwExStyle) :
    XCtrImplBase(0, parent)
{
    XWASSERT(hWndParent);

    // register class if needed
    if(!XTextLabelWindow::sRegisterWindowClass())
    {
        // error has been already reported
        return;
    }

    // create text label control
    m_hWnd = CreateWindowExW( 
        dwExStyle,
        XTextLabelWindow::windowClass(),
        text,       
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | dwStyle,
        0,0,            // starting position 
        100,14,          // default size
        hWndParent,     // parent window 
        NULL,           // No menu 
        ::GetModuleHandle(NULL), 
        NULL);      

    // check result
    if(m_hWnd == NULL)
    {
        // fatal error
        XWTRACE_WERR("XTextLabel: Failed to create window", ::GetLastError());
        return;
    }

    // set default font 
    setDefaultFont();
}

XTextLabel::~XTextLabel()
{
}

// XTextLabel
/////////////////////////////////////////////////////////////////////
