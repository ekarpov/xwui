// Dialog (modal) window functionality
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "../layout/xlayoutitem.h"
#include "../layout/xlayout.h"

#include "../xwindow/xhwnd.h"
#include "../xwindow/xwindow.h"

#include "xdialogwindow.h"

/////////////////////////////////////////////////////////////////////
// XDialogWindow - base class for modal dialogs

XDialogWindow::XDialogWindow(DWORD dwStyle, XWObject* parent, HWND hWndParent, DWORD dwExStyle) :
    XWindow(dwStyle, parent, hWndParent, dwExStyle),
    m_isModal(false),
    m_isAccepted(false)
{
}

XDialogWindow::~XDialogWindow()
{
}

/////////////////////////////////////////////////////////////////////
// show modal dialog
/////////////////////////////////////////////////////////////////////
bool XDialogWindow::showModal()
{
    // reset return value
    m_isAccepted = false;

    // ignore if no window created
    if(hwnd() == NULL) 
    {
        XWTRACE("XDialogWindow: dialog window has not been created");
        return false;
    }

    // get dialog owner window if set
    m_hwndOwner = ::GetWindow(hwnd(), GW_OWNER);

    // disable owner if set
    if(m_hwndOwner)
        ::EnableWindow(m_hwndOwner, FALSE);

    // center dialog on owner window
    if(m_hwndOwner)
        XWUtils::centerWindowOnWindow(hwnd(), m_hwndOwner);

    // set flag
    m_isModal = true;

    // show
    show(SW_SHOWNORMAL);

    // run message loop
    MSG Msg;
    while(m_isModal && ::GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        ::TranslateMessage(&Msg);
        ::DispatchMessage(&Msg);
    }

    return m_isAccepted;
}

/////////////////////////////////////////////////////////////////////
// close dialog
/////////////////////////////////////////////////////////////////////
void XDialogWindow::closeDialog(bool isAccepted)
{
    // copy flag
    m_isAccepted = isAccepted;

    // enable owner back
    if(m_hwndOwner)
    {
        ::EnableWindow(m_hwndOwner, TRUE);
        ::SetForegroundWindow(m_hwndOwner);
    }

    // close window
    destroy();
}

/////////////////////////////////////////////////////////////////////
// events
/////////////////////////////////////////////////////////////////////
void XDialogWindow::onDestroy()
{
    // reset modal flag if any so that message processing will stop
    m_isModal = false;
}

bool XDialogWindow::onClose()
{
    // close by default
    return true;
}

/////////////////////////////////////////////////////////////////////
// process messages
/////////////////////////////////////////////////////////////////////
LRESULT XDialogWindow::processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // process known messages
    switch(uMsg)
    {
    case WM_CLOSE:
        // check if we need to close window
        if(onClose()) 
        {
            // NOTE: we need to enable owner here so that Windows will not hide it
            //       while trying to bring to foreground next window

            // enable owner back
            if(m_hwndOwner)
            {
                ::EnableWindow(m_hwndOwner, TRUE);
                ::SetForegroundWindow(m_hwndOwner);
            }

        } else
        {
            // do not close window
            return 0;
        }
        break;
    }

    return XWindow::processMessage(hwnd, uMsg, wParam, lParam);
}

// XDialogWindow
/////////////////////////////////////////////////////////////////////


