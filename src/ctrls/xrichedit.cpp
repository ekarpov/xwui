// Rich edit control
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include <richedit.h>

#include "../layout/xlayoutitem.h"

#include "../xwindow/xhwnd.h"

#include "xpopupmenu.h"
#include "xrichedit.h"

/////////////////////////////////////////////////////////////////////
// NOTE:
// RichEdit on MSDN:    https://msdn.microsoft.com/en-us/library/windows/desktop/bb787605(v=vs.85).aspx
// Rich Edit Messages:  https://msdn.microsoft.com/en-us/library/windows/desktop/ff486015(v=vs.85).aspx
// EM_SETCHARFORMAT:    https://msdn.microsoft.com/en-us/library/windows/desktop/bb774230(v=vs.85).aspx
// CHARFORMAT2:         https://msdn.microsoft.com/en-us/library/windows/desktop/bb787883(v=vs.85).aspx

/////////////////////////////////////////////////////////////////////
// XRichEdit - richedit control

XRichEdit::XRichEdit(HWND hWndParent, XWObject* parent, DWORD dwStyle, DWORD dwExStyle) :
    XWHWND(0, parent),
    m_hideCaret(false),
    m_contextMenu(0),
    m_showContextMenu(true),
    m_contextMenuActive(false)
{
    XWASSERT(hWndParent);

    wchar_t* szClassName = MSFTEDIT_CLASS;

    // check if rich edit class is registered
    if(!XWUtils::isClassRegistered(szClassName))
    {
        // use older version
        szClassName = RICHEDIT_CLASSW;
    }

    // create editor
    m_hWnd = CreateWindowExW( 
        dwExStyle,
        szClassName,   
        0,       
        WS_VISIBLE | WS_CHILD | ES_MULTILINE | dwStyle,
        0,0,            // starting position 
        100,14,          // default size
        hWndParent,     // parent window 
        NULL,           // No menu 
        ::GetModuleHandle(NULL), 
        NULL);      

    // check result
    if(m_hWnd == NULL)
    {
        // fatal error
        XWTRACE_WERR_LAST("XRichEdit: Failed to create window");
        return;
    }

    // set default font 
    setDefaultFont();

    // set notification mask
    ::SendMessageW(m_hWnd, EM_SETEVENTMASK, 0, ENM_CHANGE | ENM_UPDATE);
}

XRichEdit::~XRichEdit()
{
}

/////////////////////////////////////////////////////////////////////
// text 
/////////////////////////////////////////////////////////////////////
void XRichEdit::appendText(const wchar_t* text, bool keepSelection)
{
    // save selection
    if(keepSelection)
    {
        _pushSelectionState();
    }

    CHARRANGE crange;
    crange.cpMin = -1;
    crange.cpMax = -1;

    // replace end of text with new text
    ::SendMessageW(hwnd(), EM_EXSETSEL, 0, (LPARAM)&crange);
    ::SendMessageW(hwnd(), EM_REPLACESEL, 0, (LPARAM)text);

    // restore selection
    if(keepSelection)
    {
        _popSelectionState();
    }
}

void XRichEdit::insertText(int pos, const wchar_t* text)
{
    // save selection
    _pushSelectionState();

    CHARRANGE crange;
    crange.cpMin = pos;
    crange.cpMax = pos;

    // replace end of text with new text
    ::SendMessageW(hwnd(), EM_EXSETSEL, 0, (LPARAM)&crange);
    ::SendMessageW(hwnd(), EM_REPLACESEL, 0, (LPARAM)text);

    // restore selection
    _popSelectionState();
}

int XRichEdit::getCharacterCount()
{
    GETTEXTLENGTHEX gtlex;
    gtlex.flags = GTL_NUMCHARS;
    gtlex.codepage = 1200; // Unicode characters

    LRESULT count = ::SendMessageW(hwnd(), EM_GETTEXTLENGTHEX, (WPARAM)&gtlex, 0);
    if(count == E_INVALIDARG)
    {
        XWTRACE_WERR_LAST("XRichEdit failed to read number of characters");

        // fall back to default 
        return getTextLength();
    }

    return (int)count;
}

int XRichEdit::getLineCount()
{
    // get number of lines
    return (int)::SendMessageW(hwnd(), EM_GETLINECOUNT, 0, 0);
}

/////////////////////////////////////////////////////////////////////
// text selection
/////////////////////////////////////////////////////////////////////
bool XRichEdit::hasSelection()
{
    XTextRange range;

    // get selection
    getTextSelection(range);

    // check if there is any selection
    return (range.length != 0);
}

void XRichEdit::selectText(const XTextRange& range)
{
    CHARRANGE crange;
    crange.cpMin = range.pos;
    crange.cpMax = range.pos + range.length;

    // select text
    ::SendMessageW(hwnd(), EM_EXSETSEL, 0, (LPARAM)&crange);
}

void XRichEdit::getTextSelection(XTextRange& range)
{
    CHARRANGE crange;

    // get selection
    ::SendMessageW(hwnd(), EM_EXGETSEL, 0, (LPARAM)&crange);

    // NOTE: If the cpMin and cpMax members are equal, the range is empty. 
    //       The range includes everything if cpMin is 0 and cpMax is –1.
    //       cpMin - Character position index immediately preceding the first character in the range.
    //       cpMax - Character position immediately following the last character in the range.

    // convert to text range
    range.pos = crange.cpMin;
    if(crange.cpMax >= crange.cpMin)
        range.length = crange.cpMax - crange.cpMin;
    else 
        range.length = getTextLength();
}

void XRichEdit::resetSelection()
{
    XTextRange range;

    // get current selection
    getTextSelection(range);

    // reset selection
    range.pos += range.length;
    range.length = 0;

    // update selection
    selectText(range);
}

/////////////////////////////////////////////////////////////////////
// cursor position
/////////////////////////////////////////////////////////////////////
void XRichEdit::setCusrosPos(int pos)
{
    CHARRANGE crange;
    crange.cpMin = pos;
    crange.cpMax = pos;

    // select text
    ::SendMessageW(hwnd(), EM_EXSETSEL, 0, (LPARAM)&crange);
}

void XRichEdit::setCursorEnd()
{
    CHARRANGE crange;
    crange.cpMin = -1;
    crange.cpMax = -1;

    // move cursor at the end
    ::SendMessageW(hwnd(), EM_EXSETSEL, 0, (LPARAM)&crange);
}

int XRichEdit::getCursorPos()
{
    XTextRange range;

    // get current selection
    getTextSelection(range);

    // cursor postion
    return range.pos + range.length;
}

/////////////////////////////////////////////////////////////////////
// text style 
/////////////////////////////////////////////////////////////////////
void XRichEdit::setTextStyle(const XTextStyle& style, TFormatFlags flags)
{
    CHARFORMAT2W chf2;
    ::ZeroMemory(&chf2, sizeof(CHARFORMAT2W));
    chf2.cbSize = sizeof(CHARFORMAT2W);

    // font name
    if(style.strFontName.length())
    {
        chf2.dwMask |= CFM_FACE;

        // copy name
        ::wcscpy_s(chf2.szFaceName, sizeof(chf2.szFaceName) / sizeof(WCHAR), style.strFontName.c_str());
    }

    // font size
    if(style.nFontSize)
    {
        chf2.dwMask |= CFM_SIZE;
        
        // convert units to twips
        chf2.yHeight = XWUtils::sLogicalUnitsToTwips(hwnd(), style.nFontSize);
    }

    // styles
    if(style.bBold) chf2.dwEffects |= CFE_BOLD;
    if(style.bItalic) chf2.dwEffects |= CFE_ITALIC;
    if(style.bStrike) chf2.dwEffects |= CFE_STRIKEOUT;
    if(style.bUnderline) chf2.dwEffects |= CFE_UNDERLINE;
    chf2.dwMask |= CFM_BOLD | CFM_ITALIC | CFM_STRIKEOUT | CFM_UNDERLINE; 

    // set style
    _applyFormat(flags, (LPARAM)&chf2);
}

void XRichEdit::getTextStyle(XTextStyle& style)
{
    // TODO:
    // https://msdn.microsoft.com/en-us/library/windows/desktop/bb788026(v=vs.85).aspx
}

void XRichEdit::setTextBold(bool bold, TFormatFlags flags)
{
    CHARFORMAT2W chf2;
    ::ZeroMemory(&chf2, sizeof(CHARFORMAT2W));
    chf2.cbSize = sizeof(CHARFORMAT2W);

    // styles
    chf2.dwMask |= CFM_BOLD;
    if(bold) chf2.dwEffects |= CFE_BOLD;

    // set style
    _applyFormat(flags, (LPARAM)&chf2);
}

void XRichEdit::setTextItalic(bool italic, TFormatFlags flags)
{
    CHARFORMAT2W chf2;
    ::ZeroMemory(&chf2, sizeof(CHARFORMAT2W));
    chf2.cbSize = sizeof(CHARFORMAT2W);

    // styles
    chf2.dwMask |= CFM_ITALIC;
    if(italic) chf2.dwEffects |= CFE_ITALIC;

    // set style
    _applyFormat(flags, (LPARAM)&chf2);
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
void XRichEdit::setReadOnly(bool bReadOnly)
{
    // set read only status
    if(::SendMessageW(hwnd(), EM_SETREADONLY, bReadOnly ? TRUE : FALSE, 0) == 0)
    {
        // fatal error
        XWTRACE_WERR_LAST("XRichEdit: Failed to change read only mode");
    }
}

bool XRichEdit::isReadOnly() const
{
    // check if style is set
    return (style() & ES_READONLY) != 0;
}

void XRichEdit::hideCaret(bool hide)
{
    // we need to listen for set focus notification
    if(hide)
    {
        // subclass window to listen for messages
        subclassWindow();
    }

    // copy flag
    m_hideCaret = hide;
}

bool XRichEdit::isCaretHidden() const
{
    return m_hideCaret;
}

void XRichEdit::enableAutoFont(bool enable)
{
    // NOTE: switch off/on automatic font change
    // https://msdn.microsoft.com/ru-ru/library/windows/desktop/bb774250(v=vs.85).aspx

    // read current value
    LRESULT ops = ::SendMessageW(hwnd(), EM_GETLANGOPTIONS, 0, 0);

    if(enable)
        ops |= ~IMF_AUTOFONT;
    else
        ops &= ~IMF_AUTOFONT;

    // update
    ::SendMessageW(hwnd(), EM_SETLANGOPTIONS, 0, ops);
}

/////////////////////////////////////////////////////////////////////
// text properties
/////////////////////////////////////////////////////////////////////
void XRichEdit::setTextLimit(int nLimit)
{
    // set limit
    if(::SendMessageW(hwnd(), EM_SETLIMITTEXT, nLimit, 0) == 0)
    {
        // fatal error
        XWTRACE_WERR_LAST("XRichEdit: Failed to set text limit");
    }
}

int XRichEdit::textLimit() const
{
    return (int)::SendMessageW(hwnd(), EM_GETLIMITTEXT, 0, 0);
}

void XRichEdit::setPlainTextMode()
{
    // we must reset text first
    setText(L"");

    // set mode
    if(::SendMessageW(hwnd(), EM_SETTEXTMODE, TM_PLAINTEXT, 0) != 0)
    {
        XWTRACE_WERR_LAST("XRichEdit: Failed to set plain text mode");
    }
}

void XRichEdit::setRichTextMode()
{
    // we must reset text first
    setText(L"");

    // set mode
    if(::SendMessageW(hwnd(), EM_SETTEXTMODE, TM_RICHTEXT, 0) != 0)
    {
        XWTRACE_WERR_LAST("XRichEdit: Failed to set rich text mode");
    }
}

/////////////////////////////////////////////////////////////////////
// context menu
/////////////////////////////////////////////////////////////////////
void XRichEdit::setContextMenu(XPopupMenu* contextMenu)
{
    // delete old menu if any
    if(m_contextMenu) delete m_contextMenu;

    // copy menu (may be null)
    m_contextMenu = contextMenu;

    // init menu
    if(m_contextMenu)
    {
        // copy parent
        m_contextMenu->setParentObject(this);

        // subclass window to listen for messages
        subclassWindow();
    }
}

void XRichEdit::enableContextMenu(bool enable)
{
    // set flag
    m_showContextMenu = enable;
}

XPopupMenu* XRichEdit::contextMenu()
{
    return m_contextMenu;
}

/////////////////////////////////////////////////////////////////////
// URL detection
/////////////////////////////////////////////////////////////////////
void XRichEdit::enableURLDetection(bool enable)
{
    // send message
    if(::SendMessageW(hwnd(), EM_AUTOURLDETECT, enable ? TRUE : FALSE, 0) != 0)
    {
        XWTRACE_WERR_LAST("XRichEdit: Failed to change url detection mode");
    }
}

/////////////////////////////////////////////////////////////////////
// events
/////////////////////////////////////////////////////////////////////
void XRichEdit::onContextMenuItem(UINT itemId)
{
    // notify parent by default
    notifyParentMenuItem(itemId);
}

bool XRichEdit::onShowContextMenu(int posX, int posY)
{
    // ignore if menu not set
    if(m_contextMenu == 0 || !m_showContextMenu) return false;

    // change cursor
    HCURSOR originalCursor = ::SetCursor(XWUtils::getSystemCursor(XWUtils::eCursorArrow));

    // set flag
    m_contextMenuActive = true;

    // track menu
    UINT itemId = 0;
    if(m_contextMenu->track(hwnd(), posX, posY, itemId))
    {
        // process selected item
        onContextMenuItem(itemId);
    }

    // set original cursor back
    if(originalCursor)
        ::SetCursor(originalCursor);

    // reset flag
    m_contextMenuActive = false;

    // consume event
    return true;
}

/////////////////////////////////////////////////////////////////////
// process messages (in case window has been subclassed)
/////////////////////////////////////////////////////////////////////
LRESULT XRichEdit::processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // process known messages
    switch(uMsg)
    {
    case WM_SETFOCUS:
        // hide caret if needed
        if(m_hideCaret)
        {
            ::HideCaret(hwnd);
            return 0;
        }
        break;

    case WM_CONTEXTMENU:
        // process 
        if(onShowContextMenu(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))) return 0;
        break;

    case WM_SETCURSOR:
        // block cursor from changing while content menu is active
        if(m_contextMenuActive) return TRUE;
        break;
    }

    // pass to parent
    return XWHWND::processMessage(hwnd, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////
// helper methods
/////////////////////////////////////////////////////////////////////
void XRichEdit::_pushSelectionState()
{
    // read current selection
    getTextSelection(m_selectionRange);
}

void XRichEdit::_popSelectionState()
{
    // select text
    selectText(m_selectionRange);

    // reset selection
    m_selectionRange.length = 0;
    m_selectionRange.pos = 0;
}

WPARAM XRichEdit::_formatFlags(TFormatFlags flags)
{
    switch(flags)
    {
    case eSetDefault:       return SCF_DEFAULT;
    case eFormatCurrent:    return SCF_SELECTION;
    case eFormatSelection:  return SCF_SELECTION;
    case eFormatAll:        return SCF_ALL;
    }

    XWASSERT1(0, "XRichEdit: unknown format flag");
    return SCF_DEFAULT;
}

void XRichEdit::_applyFormat(TFormatFlags flags, LPARAM format)
{
    if(flags == eFormatCurrent)
    {
        // save selection
        _pushSelectionState();

        // reset selection
        resetSelection();
    }

    // set style
    if(::SendMessageW(hwnd(), EM_SETCHARFORMAT, _formatFlags(flags), format) == 0)
    {
        // fatal error
        XWTRACE_WERR_LAST("XRichEdit: failed to set character format");
    }

    if(flags == eFormatCurrent)
    {
        // restore selection
        _popSelectionState();
    }
}

// XRichEdit
/////////////////////////////////////////////////////////////////////


