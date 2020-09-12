// Text label
//
/////////////////////////////////////////////////////////////////////

#ifndef _XTEXTLABEL_H_
#define _XTEXTLABEL_H_

/////////////////////////////////////////////////////////////////////
// XTextLabel - text label window

class XTextLabel : public XWHWND
{
public: // construction/destruction
    XTextLabel(const wchar_t* text, HWND hWndParent, XWObject* parent = 0, DWORD dwStyle = 0, DWORD dwExStyle = 0);
    ~XTextLabel();

public: // events
    XWEventMask clicked() const
    {
        return mkCommandEvent(STN_CLICKED);
    }

    XWEventMask doubleClicked() const
    {
        return mkCommandEvent(STN_DBLCLK);
    }
};

// XTextLabel
/////////////////////////////////////////////////////////////////////

#endif // _XTEXTLABEL_H_

