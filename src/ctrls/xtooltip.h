// ToolTip control
//
/////////////////////////////////////////////////////////////////////

#ifndef _XTOOLTIP_H_
#define _XTOOLTIP_H_

/////////////////////////////////////////////////////////////////////

// NOTE: appart from standard method to work with tooltips using tools
//       this control also support "toolless" mode. It can be shown and
//       hidden without adding tool. Tooltip is shown near mouse pointer.

/////////////////////////////////////////////////////////////////////
// XToolTip - tooltip control

class XToolTip : public XWHWND
{
public: // construction/destruction
    XToolTip(HWND hWndParent, XWObject* parent = 0, DWORD dwStyle = 0, DWORD dwExStyle = 0);
    ~XToolTip();

public: // manage tools
    void    addToolHwnd(HWND hwndTool, const wchar_t* text);
    void    addToolRect(HWND hwndTool, const RECT& rect, const wchar_t* text);

public: // show and hide tool tip without tools
    void    enableToolLessMode();
    void    showToolLess(const wchar_t* text);
    void    hideToolLess();
    bool    isShownToolLess() const;

public: // properties
    void    setMaxWidth(int width);

private: // data
    bool    m_isShownToolLess;
};

// XToolTip
/////////////////////////////////////////////////////////////////////

#endif // _XTOOLTIP_H_


