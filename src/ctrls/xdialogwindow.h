// Dialog (modal) window functionality
//
/////////////////////////////////////////////////////////////////////

#ifndef _XDIALOGWINDOW_H_
#define _XDIALOGWINDOW_H_

/////////////////////////////////////////////////////////////////////
// XDialogWindow - base class for modal dialogs

class XDialogWindow : public XWindow
{
public: // construction/destruction
    XDialogWindow(DWORD dwStyle, XWObject* parent = 0, HWND hWndParent = 0, DWORD dwExStyle = 0);
    ~XDialogWindow();

public: // show modal dialod
    bool    showModal();

protected: // close dialog
    void    closeDialog(bool isAccepted = true);

protected: // events
    virtual void    onDestroy();
    virtual bool    onClose();

protected: // process messages
    virtual LRESULT processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private: // data
    bool    m_isModal;
    bool    m_isAccepted;
    HWND    m_hwndOwner;
};

// XDialogWindow
/////////////////////////////////////////////////////////////////////

#endif // _XDIALOGWINDOW_H_

