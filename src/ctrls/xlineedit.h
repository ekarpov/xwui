// Line edit control
//
/////////////////////////////////////////////////////////////////////

#ifndef _XLINEEDIT_H_
#define _XLINEEDIT_H_

/////////////////////////////////////////////////////////////////////
// XLineEdit - line edit control

class XLineEdit : public XWHWND
{
public: // construction/destruction
    XLineEdit(HWND hWndParent, XWObject* parent = 0, DWORD dwStyle = ES_AUTOHSCROLL | WS_BORDER, DWORD dwExStyle = 0);
    ~XLineEdit();

public: // properties
    void    setReadOnly(bool bReadOnly);
    bool    isReadOnly() const;
    bool    isPasswordMode() const;

public: // text
    void    setTextLimit(int nLimit);
    int     textLimit() const;

public: // cursor
    void    setCursorPos(int pos);

public: // read text 
    const WCHAR*    readText();

public: // editor events
    XWEventMask textChanged() const
    {
        return mkCommandEvent(EN_CHANGE);
    }
    XWEventMask textUpdated() const
    {
        return mkCommandEvent(EN_UPDATE);
    }
    XWEventMask focused() const
    {
        return mkCommandEvent(EN_SETFOCUS);
    }

private: // buffers
    std::vector<WCHAR>  m_textBuffer;
};

// XLineEdit
/////////////////////////////////////////////////////////////////////

#endif // _XLINEEDIT_H_

