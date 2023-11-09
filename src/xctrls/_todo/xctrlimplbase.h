// Extended controls implementation helpers
//
/////////////////////////////////////////////////////////////////////

#ifndef _XCTRLIMPLBASE_H_
#define _XCTRLIMPLBASE_H_

/////////////////////////////////////////////////////////////////////
// XCtrImplBase - base functionality for extended control implemnetaiton

class XCtrImplBase : public XHWNDLayoutItem
{
public: // construction/destruction
    XCtrImplBase(HWND hWnd = 0, XWObject* parent = 0);
    ~XCtrImplBase();

public: // transparent background
    void        setTransparent(bool bTransparent);
    bool        isTransparent();

public: // layout helpers
    int         getWidthHint() const;
    int         getHeightHint() const;
    int         getMinWidthHint() const;
    int         getMinHeightHint() const;

public: // colors
    bool        setTextColor(COLORREF col, BOOL bRepaint = FALSE);
    COLORREF    getTextColor() const;
    bool        setBackgroundColor(COLORREF col, BOOL bRepaint = FALSE);
    COLORREF    getBackgroundColor() const;
};

// XCtrImplBase
/////////////////////////////////////////////////////////////////////

#endif // _XCTRLIMPLBASE_H_

