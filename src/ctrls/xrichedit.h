// Rich edit control
//
/////////////////////////////////////////////////////////////////////

#ifndef _XRICHEDIT_H_
#define _XRICHEDIT_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
class XPopupMenu;

/////////////////////////////////////////////////////////////////////
// XRichEdit - richedit control

class XRichEdit : public XWHWND
{
public: // construction/destruction
    XRichEdit(HWND hWndParent, XWObject* parent = 0, DWORD dwStyle = WS_BORDER, DWORD dwExStyle = 0);
    ~XRichEdit();

public: // manage text 
    void    appendText(const wchar_t* text, bool keepSelection = false);
    void    insertText(int pos, const wchar_t* text);
    int     getCharacterCount();
    int     getLineCount();

public: // text selection
    bool    hasSelection();
    void    selectText(const XTextRange& range);
    void    getTextSelection(XTextRange& range);
    void    resetSelection();

public: // cursor position
    void    setCusrosPos(int pos);
    void    setCursorEnd();
    int     getCursorPos();

public: // text format flags
    enum TFormatFlags
    {
        eSetDefault,            // set default format
        eFormatCurrent,         // change format for new text at insertion point
        eFormatSelection,       // format selection     
        eFormatAll              // format all text
    };

public: // text style 
    void    setTextStyle(const XTextStyle& style, TFormatFlags flags = eSetDefault);
    void    getTextStyle(XTextStyle& style);
    void    setTextBold(bool bold, TFormatFlags flags = eSetDefault);
    void    setTextItalic(bool italic, TFormatFlags flags = eSetDefault);

public: // properties
    void    setReadOnly(bool bReadOnly);
    bool    isReadOnly() const;
    void    hideCaret(bool hide);
    bool    isCaretHidden() const;
    void    enableAutoFont(bool enable);

public: // text properties
    void    setTextLimit(int nLimit);
    int     textLimit() const;
    void    setPlainTextMode();
    void    setRichTextMode();

public: // context menu
    void    setContextMenu(XPopupMenu* contextMenu);
    void    enableContextMenu(bool enable);
    XPopupMenu* contextMenu();

public: // URL detection
    void    enableURLDetection(bool enable);

public: // editor events
    XWEventMask textChanged()   const { return mkCommandEvent(EN_CHANGE); }
    XWEventMask textUpdated()   const { return mkCommandEvent(EN_UPDATE); }
    XWEventMask focusSet()      const { return mkCommandEvent(EN_SETFOCUS); }
    XWEventMask focusLost()     const { return mkCommandEvent(EN_KILLFOCUS); }

protected: // events
    virtual void    onContextMenuItem(UINT itemId);
    virtual bool    onShowContextMenu(int posX, int posY);

protected: // process messages (in case window has been subclassed)
    LRESULT     processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private: // helper methods
    void        _pushSelectionState();
    void        _popSelectionState();
    WPARAM      _formatFlags(TFormatFlags flags);
    void        _applyFormat(TFormatFlags flags, LPARAM format);

private: // data
    bool            m_hideCaret;
    XTextRange      m_selectionRange;
    XPopupMenu*     m_contextMenu;
    bool            m_showContextMenu;
    bool            m_contextMenuActive;
};

// XRichEdit
/////////////////////////////////////////////////////////////////////

#endif // _XRICHEDIT_H_

