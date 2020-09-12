// Text label
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "../layout/xlayoutitem.h"

#include "../xwindow/xhwnd.h"

#include "xtextlabel.h"

/////////////////////////////////////////////////////////////////////
// XTextLabel - text label window

XTextLabel::XTextLabel(const wchar_t* text, HWND hWndParent, XWObject* parent, DWORD dwStyle, DWORD dwExStyle) :
    XWHWND(0, parent)
{
    XWASSERT(hWndParent);

    // create static control
    m_hWnd = CreateWindowExW( 
        dwExStyle,
        WC_STATICW,
        text,       
        WS_VISIBLE | WS_CHILD | SS_NOTIFY | dwStyle,
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
        XWTRACE_WERR_LAST("XTextLabel: Failed to create window");
        return;
    }

    // set default font 
    setDefaultFont();

    // NOTE: for default sizes refer to MSDN:
    //       http://msdn.microsoft.com/en-us/library/ms997619.aspx

    // NOTE: font has to be set before
    // get default button dimensions
    LONG units = XWUtils::sGetDialogBaseUnits(m_hWnd);
    int width = MulDiv(LOWORD(units), 100, 4);   // NOTE: use slightly bigger (50 is default)
    int height = MulDiv(HIWORD(units), 12, 8);   // NOTE: use slightly bigger (10 is default)

    // fix default size
    XWHWND::setFixedSize(width, height);
}

XTextLabel::~XTextLabel()
{
}

// XTextLabel
/////////////////////////////////////////////////////////////////////

