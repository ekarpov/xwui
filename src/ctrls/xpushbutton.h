// Push button
//
/////////////////////////////////////////////////////////////////////

#ifndef _XPUSHBUTTON_H_
#define _XPUSHBUTTON_H_

/////////////////////////////////////////////////////////////////////
// XPushButton - push button window

class XPushButton : public XWHWND
{
public: // construction/destruction
    XPushButton(const wchar_t* text, HWND hWndParent, XWObject* parent = 0, DWORD dwStyle = BS_PUSHBUTTON, DWORD dwExStyle = 0);
    ~XPushButton();

public: // button events
    XWEventMask clicked() const { return mkCommandEvent(BN_CLICKED); }
};

// XPushButton
/////////////////////////////////////////////////////////////////////

#endif // _XPUSHBUTTON_H_


