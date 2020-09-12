// Application main window functionality
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "../layout/xlayoutitem.h"
#include "../layout/xlayout.h"

#include "../xwindow/xhwnd.h"
#include "../xwindow/xwindow.h"

#include "xmainwindow.h"

/////////////////////////////////////////////////////////////////////
// XMainWindow - main window functionality

XMainWindow::XMainWindow(DWORD dwStyle, XWObject* parent, HWND hWndParent, DWORD dwExStyle) :
    XWindow(dwStyle, parent, hWndParent, dwExStyle)
{
}

XMainWindow::~XMainWindow()
{
}

/////////////////////////////////////////////////////////////////////
// message handlers
/////////////////////////////////////////////////////////////////////
void XMainWindow::onDestroy()
{
    // pass quit message to meessage loop
    ::PostQuitMessage(0);
}

bool XMainWindow::onClose()
{
    // close by default
    return true;
}

/////////////////////////////////////////////////////////////////////
// process messages
/////////////////////////////////////////////////////////////////////
LRESULT XMainWindow::processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // process known messages
    switch(uMsg)
    {
    case WM_CLOSE:
        // check if we need to close window
        if(!onClose()) return 0;
        break;
    }

    return XWindow::processMessage(hwnd, uMsg, wParam, lParam);
}

// XMainWindow
/////////////////////////////////////////////////////////////////////
