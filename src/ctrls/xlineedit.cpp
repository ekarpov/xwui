// Line edit control
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "../layout/xlayoutitem.h"

#include "../xwindow/xhwnd.h"

#include "xlineedit.h"

/////////////////////////////////////////////////////////////////////
// XLineEdit - line edit control

XLineEdit::XLineEdit(HWND hWndParent, XWObject* parent, DWORD dwStyle, DWORD dwExStyle) :
    XWHWND(0, parent)
{
    XWASSERT(hWndParent);

    // create editor
    m_hWnd = CreateWindowExW( 
        dwExStyle,
        WC_EDITW,   
        0,       
        WS_VISIBLE | WS_CHILD | dwStyle,
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
        XWTRACE_WERR_LAST("XLineEdit: Failed to create window");
        return;
    }

    // set default font 
    setDefaultFont();

    // NOTE: for default sizes refer to MSDN:
    //       http://msdn.microsoft.com/en-us/library/ms997619.aspx

    // NOTE: font has to be set before
    // get default text dimensions
    LONG units = XWUtils::sGetDialogBaseUnits(m_hWnd);
    int width = MulDiv(LOWORD(units), 100, 4);   // NOTE: use slightly bigger (50 is default)
    int height = MulDiv(HIWORD(units), 12, 8);   // NOTE: use slightly bigger (10 is default)

    // fix default size
    XWHWND::setFixedSize(width, height);
}

XLineEdit::~XLineEdit()
{
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
void XLineEdit::setReadOnly(bool bReadOnly)
{
    // set read only status
    if(::SendMessageW(hwnd(), EM_SETREADONLY, bReadOnly ? TRUE : FALSE, 0) == 0)
    {
        // fatal error
        XWTRACE_WERR_LAST("XLineEdit: Failed to change read only mode");
    }
}

bool XLineEdit::isReadOnly() const
{
    // check if style is set
    return (style() & ES_READONLY) != 0;
}

bool XLineEdit::isPasswordMode() const
{
    // check if style is set
    return (style() & ES_PASSWORD) != 0;
}

/////////////////////////////////////////////////////////////////////
// text
/////////////////////////////////////////////////////////////////////
void XLineEdit::setTextLimit(int nLimit)
{
    // set limit
    if(::SendMessageW(hwnd(), EM_SETLIMITTEXT, nLimit, 0) == 0)
    {
        // fatal error
        XWTRACE_WERR_LAST("XLineEdit: Failed to set text limit");
    }
}

int XLineEdit::textLimit() const
{
    return (int) ::SendMessageW(hwnd(), EM_GETLIMITTEXT, 0, 0);
}

/////////////////////////////////////////////////////////////////////
// cursor
/////////////////////////////////////////////////////////////////////
void XLineEdit::setCursorPos(int pos)
{
    // set position
    if(::SendMessageW(hwnd(), EM_SETSEL, pos, pos) == 0)
    {
        // fatal error
        XWTRACE_WERR_LAST("XLineEdit: Failed to set cusror position");
    }
}

/////////////////////////////////////////////////////////////////////
// read text 
/////////////////////////////////////////////////////////////////////
const WCHAR* XLineEdit::readText()
{
    int textLength = getTextLength();
    if(textLength)
    {
        // reserve space (for end of line as well)
        m_textBuffer.resize(textLength + 1);

        // get text
        getText(m_textBuffer.data(), textLength + 1);

    } else
    {
        // format empty string
        m_textBuffer.resize(1);
        m_textBuffer[0] = 0;
    }

    // return string
    return m_textBuffer.data();
}

// XLineEdit
/////////////////////////////////////////////////////////////////////

