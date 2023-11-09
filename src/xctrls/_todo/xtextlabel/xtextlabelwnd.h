// Extended text label implementation
//
/////////////////////////////////////////////////////////////////////

#ifndef _XTEXTLABELWND_H_
#define _XTEXTLABELWND_H_

/////////////////////////////////////////////////////////////////////
// standard library
#include <string>

/////////////////////////////////////////////////////////////////////
// class window helpers
#include "../xclasswindow.h"
#include "../xctrlwndbase.h"

/////////////////////////////////////////////////////////////////////
// XTextLabelWindow - text label window implementation

class XTextLabelWindow : public XClassWindowT<XTextLabelWindow>,
                         public XCtrlWndBase
{
    friend class XClassWindowT<XTextLabelWindow>;

private: // construction/destruction
    XTextLabelWindow();
    ~XTextLabelWindow();

public: // window class
    static const wchar_t*  windowClass();

public: // window procedure
    LRESULT windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private: // layout helpers
    int getMinWidthHint(HWND hwnd) const;
    int getMinHeightHint(HWND hwnd) const;

private: // implementation
    void    _onPaint(HWND hwnd);
};

// XTextLabelWindow
/////////////////////////////////////////////////////////////////////

#endif // _XTEXTLABELWND_H_

