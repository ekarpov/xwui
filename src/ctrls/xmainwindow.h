// Application main window functionality
//
/////////////////////////////////////////////////////////////////////

#ifndef _XMAINWINDOW_H_
#define _XMAINWINDOW_H_

/////////////////////////////////////////////////////////////////////
// XMainWindow - main window functionality

class XMainWindow : public XWindow
{
public: // construction/destruction
    XMainWindow(DWORD dwStyle, XWObject* parent = 0, HWND hWndParent = 0, DWORD dwExStyle = 0);
    ~XMainWindow();

public: // TODO:
    // setCentralWindow

protected: // events
    virtual void    onDestroy();
    virtual bool    onClose();

protected: // process messages
    virtual LRESULT processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

// XMainWindow
/////////////////////////////////////////////////////////////////////

#endif // _XMAINWINDOW_H_

