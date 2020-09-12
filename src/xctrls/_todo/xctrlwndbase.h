// Extended controls common functionality
//
/////////////////////////////////////////////////////////////////////

#ifndef _XCTRLWNDBASE_H_
#define _XCTRLWNDBASE_H_

/////////////////////////////////////////////////////////////////////
// standard library
#include <string>

/////////////////////////////////////////////////////////////////////
// XCtrlWndBase - base functionality for xcontrol window

class XCtrlWndBase
{
protected: // construction/destruction
    XCtrlWndBase();
    ~XCtrlWndBase();

protected: // window procedure
    LRESULT baseWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected: // layout helpers
    virtual int getWidthHint(HWND hwnd) const;
    virtual int getHeightHint(HWND hwnd) const;
    virtual int getMinWidthHint(HWND hwnd) const;
    virtual int getMinHeightHint(HWND hwnd) const;

protected: // data
    std::wstring    xctrl_strText;
    COLORREF        xctrl_textColor;
    COLORREF        xctrl_bkColor;
    HFONT           xctrl_hFont;

private: // message handlers
    LRESULT _onCreate(const CREATESTRUCT* lpCreate);
    LRESULT _onSetText(const wchar_t* text);
    LRESULT _onGetText();
    LRESULT _onGetTextLength();
};

// XCtrlWndBase
/////////////////////////////////////////////////////////////////////

#endif // _XCTRLWNDBASE_H_

