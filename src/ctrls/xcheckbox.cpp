// CheckBox control
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "../layout/xlayoutitem.h"

#include "../xwindow/xhwnd.h"

#include "xcheckbox.h"

/////////////////////////////////////////////////////////////////////
// XCheckBox - checkbox control

XCheckBox::XCheckBox(const wchar_t* text, HWND hWndParent, XWObject* parent, DWORD dwStyle, DWORD dwExStyle) :
    XWHWND(0, parent)
{    
    XWASSERT(hWndParent);

    // create editor
    m_hWnd = CreateWindowExW( 
        dwExStyle,
        WC_BUTTONW,   
        text,       
        WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | dwStyle,
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
        XWTRACE_WERR_LAST("XCheckBox: Failed to create window");
        return;
    }

    // set default font 
    setDefaultFont();

    // NOTE: for default sizes refer to MSDN:
    //       http://msdn.microsoft.com/en-us/library/ms997619.aspx

    // NOTE: font has to be set before
    // get default text dimensions
    LONG units = XWUtils::sGetDialogBaseUnits(m_hWnd);
    int width = MulDiv(LOWORD(units), 100, 4);   
    int height = MulDiv(HIWORD(units), 12, 8);   // NOTE: use slightly bigger (10 is default)

    // fix default size
    XWHWND::setFixedHeight(height);
    XWHWND::setMinWidth(width);
}

XCheckBox::~XCheckBox()
{
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
bool XCheckBox::isChecked()
{
    // get state from control
    return (::SendMessageW(hwnd(), BM_GETCHECK, 0, 0) == BST_CHECKED);
}

void XCheckBox::setChecked(bool checked)
{
    // set state (this message always returns zero)
    ::SendMessageW(hwnd(), BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
}

// XCheckBox
/////////////////////////////////////////////////////////////////////

