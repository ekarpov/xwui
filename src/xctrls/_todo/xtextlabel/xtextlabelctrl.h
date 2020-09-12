// Extended text label control
//
/////////////////////////////////////////////////////////////////////

#ifndef _XTEXTLABELCTRL_H_
#define _XTEXTLABELCTRL_H_

/////////////////////////////////////////////////////////////////////
// control implementation helpers
#include "../xctrlimplbase.h"

/////////////////////////////////////////////////////////////////////
// XTextLabel - extended text label

class XTextLabel : public XCtrImplBase
{
public: // construction/destruction
    XTextLabel(const wchar_t* text, HWND hWndParent, XWObject* parent = 0, DWORD dwStyle = 0, DWORD dwExStyle = 0);
    ~XTextLabel();

public: // events
};

// XTextLabel
/////////////////////////////////////////////////////////////////////

#endif // _XTEXTLABELCTRL_H_

