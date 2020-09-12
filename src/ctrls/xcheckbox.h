// CheckBox control
//
/////////////////////////////////////////////////////////////////////

#ifndef _XCHECKBOX_H_
#define _XCHECKBOX_H_

/////////////////////////////////////////////////////////////////////
// XCheckBox - checkbox control

class XCheckBox : public XWHWND
{
public: // construction/destruction
    XCheckBox(const wchar_t* text, HWND hWndParent, XWObject* parent = 0, DWORD dwStyle = 0, DWORD dwExStyle = 0);
    ~XCheckBox();

public: // properties
    bool    isChecked();
    void    setChecked(bool checked);

public: // events
    XWEventMask clicked() const { return mkCommandEvent(BN_CLICKED); }
};

// XCheckBox
/////////////////////////////////////////////////////////////////////

#endif // _XCHECKBOX_H_

