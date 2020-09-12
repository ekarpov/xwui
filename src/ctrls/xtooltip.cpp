// ToolTip control
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"
#include "../layout/xlayoutitem.h"
#include "../xwindow/xhwnd.h"

#include "xtooltip.h"

/////////////////////////////////////////////////////////////////////
// XToolTip - tooltip control

/////////////////////////////////////////////////////////////////////
// construction/destruction
/////////////////////////////////////////////////////////////////////
XToolTip::XToolTip(HWND hWndParent, XWObject* parent, DWORD dwStyle, DWORD dwExStyle) :
    XWHWND(0, parent),
    m_isShownToolLess(false)
{
    XWASSERT(hWndParent);

    // create control
    m_hWnd = CreateWindowExW( 
        dwExStyle | WS_EX_TOPMOST,
        TOOLTIPS_CLASS,   
        0,       
        dwStyle | WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hWndParent,         // parent window 
        NULL,               // No menu 
        ::GetModuleHandle(NULL), 
        NULL);      

    // check result
    if(m_hWnd == NULL)
    {
        // fatal error
        XWTRACE_WERR_LAST("XToolTip: Failed to create window");
        return;
    }
    
    // mark as topmost
    ::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, 
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);    
    
    // set default font 
    setDefaultFont();    
}

XToolTip::~XToolTip()
{
}

/////////////////////////////////////////////////////////////////////
// manage tools
/////////////////////////////////////////////////////////////////////
void XToolTip::addToolHwnd(HWND hwndTool, const wchar_t* text)
{
    XWASSERT(hwndTool);
    XWASSERT(text);
    if(hwndTool == 0 || text == 0) return;

    // format tool info
    TOOLINFOW tiw = { 0 };
    tiw.cbSize = sizeof(TOOLINFOW);
    tiw.uFlags = TTF_SUBCLASS | TTF_IDISHWND | TTF_PARSELINKS;
    tiw.hwnd = hwndTool;
    tiw.uId = (UINT_PTR)hwndTool;
    tiw.hinst = 0;
    tiw.lpszText = (LPWSTR) text;

    // add tool
    ::SendMessageW(hwnd(), TTM_ADDTOOLW, 0, (LPARAM)&tiw);
}

void XToolTip::addToolRect(HWND hwndTool, const RECT& rect, const wchar_t* text)
{
    XWASSERT(hwndTool);
    XWASSERT(text);
    if(hwndTool == 0 || text == 0) return;

    // format tool info
    TOOLINFOW tiw = { 0 };
    tiw.cbSize = sizeof(TOOLINFOW);
    tiw.uFlags = TTF_SUBCLASS | TTF_PARSELINKS;
    tiw.hwnd = hwndTool;
    tiw.rect = rect;
    tiw.hinst = 0;
    tiw.lpszText = (LPWSTR) text;

    // add tool
    ::SendMessageW(hwnd(), TTM_ADDTOOLW, 0, (LPARAM)&tiw);
}

/////////////////////////////////////////////////////////////////////
// show and hide tool tip without tools
/////////////////////////////////////////////////////////////////////
void XToolTip::enableToolLessMode()
{
    // add dummy tool
    TOOLINFOW tiw = { 0 };
    tiw.cbSize = sizeof(TOOLINFOW);
    tiw.uFlags = TTF_IDISHWND | TTF_PARSELINKS;
    tiw.hwnd = hwnd();
    tiw.uId = (UINT_PTR)hwnd();
    tiw.lpszText = L"";

    // add tool
    ::SendMessageW(hwnd(), TTM_ADDTOOLW, 0, (LPARAM)&tiw);
}

void XToolTip::showToolLess(const wchar_t* text)
{
    XWASSERT(text);
    if(text == 0) return;

    // format tool
    TOOLINFOW tiw = { 0 };
    tiw.cbSize = sizeof(TOOLINFOW);
    tiw.hwnd = hwnd();
    tiw.uId = (UINT_PTR)hwnd();
    tiw.lpszText = (LPWSTR) text;

    // update text 
    ::SendMessageW(hwnd(), TTM_UPDATETIPTEXTW, 0, (LPARAM)&tiw);

    // show tooltip
    ::SendMessageW(hwnd(), TTM_TRACKACTIVATE, TRUE, (LPARAM)&tiw);

    // mark flag
    m_isShownToolLess = true;
}

void XToolTip::hideToolLess()
{
    // format tool
    TOOLINFOW tiw = { 0 };
    tiw.cbSize = sizeof(TOOLINFOW);
    tiw.hwnd = hwnd();
    tiw.uId = (UINT_PTR)hwnd();

    // hide tooltip
    ::SendMessageW(hwnd(), TTM_TRACKACTIVATE, FALSE, (LPARAM)&tiw);

    // mark flag
    m_isShownToolLess = false;
}

bool XToolTip::isShownToolLess() const
{
    return m_isShownToolLess;
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
void XToolTip::setMaxWidth(int width)
{
    // update
    ::SendMessageW(hwnd(), TTM_SETMAXTIPWIDTH, 0, (LPARAM)width);
}

// XToolTip
/////////////////////////////////////////////////////////////////////
