// RichTextEdit windowless RichText editor
//
/////////////////////////////////////////////////////////////////////

#ifndef _XRICHTEXTEDIT_H_
#define _XRICHTEXTEDIT_H_

/////////////////////////////////////////////////////////////////////
// IXRichEditObserver - rich edit observer

class IXRichEditObserver
{
public: // construction/destruction
    IXRichEditObserver() {}
    virtual ~IXRichEditObserver() {}

public: // scrollbars
    virtual void    onRichEditVerticalScrollPos(int pos) {}
    virtual void    onRichEditHorizontalScrollPos(int pos) {}

public: // content
    virtual void    onRichEditContentRectChanged(const RECT& rcContent) {}

public: // links
    virtual void    onTextLinkClicked(const XTextRange& range, const std::wstring& link) {}
};

// IXRichEditObserver
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// XRichTextEdit - windowless RichText editor

class XRichTextEdit : public ITextHost,
                      public IXRichTextParserObserver
{
public: // construction/destruction
    XRichTextEdit();
    virtual ~XRichTextEdit();

public: // initialization
    bool    init(HWND hwnd, const RECT& rcClient);
    void    close();
    bool    isReady() const;

public: // observer
    void    setObserver(IXRichEditObserver* observer);

public: // interfaces
    ITextServices*  textServices() const { return m_textServices; }
    IRichEditOle*   richEditOle() const  { return m_richEditOle; }

public: // text style 
    XTextStyle& textStyle() { return m_textStyle; };
    void    setTextStyle(const XTextStyle& style);

public: // paragraph properties 
    void    setAlignment(TTextAlignment alignment);
    void    setLineSpacing(TLineSpacing spacing);
    void    setParaSpacing(DWORD left, DWORD right, DWORD top, DWORD bottom);
    void    setTextBackgroundColor(COLORREF color);

    TTextAlignment  alignment() const;
    TLineSpacing    lineSpacing() const;
    void    getParaSpacing(DWORD& left, DWORD& right, DWORD& top, DWORD& bottom);
    void    getTextBackgroundColor(COLORREF& colorOut);

public: // editor properties
    void    setRichTextMode(bool richText)  { _setPropertyBit(TXTBIT_RICHTEXT, richText); }
    void    setMultiline(bool multiline)    { _setPropertyBit(TXTBIT_MULTILINE, multiline); }
    void    setReadOnly(bool readonly)      { _setPropertyBit(TXTBIT_READONLY, readonly); }
    void    setWordWrap(bool wrap)          { _setPropertyBit(TXTBIT_WORDWRAP, wrap); }
    void    setPasswordMode(bool enable)    { _setPropertyBit(TXTBIT_USEPASSWORD, enable); }
    void    setTextLimit(DWORD textLimit);

    bool    isRichTextMode() const          { return _getPropertyBit(TXTBIT_RICHTEXT); }
    bool    isMultiline() const             { return _getPropertyBit(TXTBIT_MULTILINE); }
    bool    isReadOnly() const              { return _getPropertyBit(TXTBIT_READONLY); }
    bool    hasWordWrap() const             { return _getPropertyBit(TXTBIT_WORDWRAP); }
    bool    isPasswordMode() const          { return _getPropertyBit(TXTBIT_USEPASSWORD); }
    DWORD   textLimit() const               { return m_maxLength; }

public: // editor colors 
    enum EditorColor
    {
        eColorText,
        eColorBackground,
        eColorSelectionText,
        eColorSelectionBackground
    };

    void    setColor(EditorColor color, COLORREF value);
    void    getColor(EditorColor color, COLORREF& valueOut);
    void    enableBackgroundFill(bool enable);

public: // edit text
    bool    insertText(int pos, const wchar_t* text, UINT length = 0);
    bool    insertRichText(int pos, const XRichText* richText);
    bool    insertFormatText(int pos, const wchar_t* text);
    bool    deleteText(int pos, int length);
    bool    clearText();

public: // read text
    bool    getText(int pos, int length, std::wstring& textOut);
    bool    getRichText(int pos, int length, XRichText* richTextOut);
    bool    getFormatText(int pos, int length, std::wstring& formatTextOut);
    int     getTextLength();
    int     getLineCount();

public: // text style
    bool    setTextStyle(int pos, int length, const XTextStyle& style);
    bool    getTextStyle(int pos, XTextStyle& style);

public: // text color
    bool    setTextColor(int pos, int length, COLORREF textColor);
    bool    resetTextColor(int pos, int length);
    bool    getTextColor(int pos, COLORREF& colorOut);
    bool    setBackgroundColor(int pos, int length, COLORREF backgroundColor);
    bool    resetBackgroundColor(int pos, int length);
    bool    getBackgroundColor(int pos, COLORREF& colorOut);

public: // inline objects
    bool    insertInlineObject(int textPos, XTextInlineObject* inlineObject, const XTextStyle* style = 0);
    int     getInlineObjectCount();
    bool    getInlineObject(int index, IDataObject** objectOut, int* textPosOut = 0);

public: // urls
    bool    setTextUrl(int pos, int length, const wchar_t* url);

public: // cursor 
    int     cursorPos();
    void    setCursorEnd();
    void    setCursorPos(int pos);

public: // selection 
    bool    hasSelection();
    bool    selectText(int pos, int length);
    bool    getTextSelection(int& pos, int& length);
    bool    resetSelection();

public: // content size
    int     contentWidth();
    int     contentHeight();

public: // scrolling info
    long    scrollOffsetX();
    long    scrollOffsetY();

public: // scrolling content
    void    setScrollOffsetX(int pos);
    void    setScrollOffsetY(int pos);

public: // message processing
    bool    sendWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* resultOut = 0);
    LRESULT processWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& messageProcessed);

public: // interface
    void    update(const RECT& rcClient);

public: // properties
    void    setEnabled(bool bEnabled);
    bool    isEnabled() const;

public: // focus
    void    setFocus(bool bFocus);

public: // cursor
    bool    onSetCursor(int posX, int posY);

public: // GDI painting
    void    onPaintGDI(HDC hdc, const RECT& rcPaint);   

public: // TODO: Direct2D painting (Windows 8 only)
    // ITextHost2: https://msdn.microsoft.com/en-us/library/windows/desktop/hh768552(v=vs.85).aspx
    // ITextServices2: https://msdn.microsoft.com/en-us/library/windows/desktop/hh768718(v=vs.85).aspx
    //void    onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint); 
    //void    onInitD2DTarget(ID2D1RenderTarget* pTarget);
    //void    onResetD2DTarget();

public: // IUnknown 
    STDMETHODIMP            QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

public: // ITextHost 
    HDC         TxGetDC();
    INT         TxReleaseDC(HDC hdc);
    BOOL        TxShowScrollBar(INT fnBar, BOOL fShow);
    BOOL        TxEnableScrollBar (INT fuSBFlags, INT fuArrowflags);
    BOOL        TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw);
    BOOL        TxSetScrollPos (INT fnBar, INT nPos, BOOL fRedraw);
    void        TxInvalidateRect(LPCRECT prc, BOOL fMode);
    void        TxViewChange(BOOL fUpdate);
    BOOL        TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight);
    BOOL        TxShowCaret(BOOL fShow);
    BOOL        TxSetCaretPos(INT x, INT y);
    BOOL        TxSetTimer(UINT idTimer, UINT uTimeout);
    void        TxKillTimer(UINT idTimer);
    void        TxScrollWindowEx (INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll);
    void        TxSetCapture(BOOL fCapture);
    void        TxSetFocus();
    void        TxSetCursor(HCURSOR hcur, BOOL fText);
    BOOL        TxScreenToClient (LPPOINT lppt);
    BOOL        TxClientToScreen (LPPOINT lppt);
    HRESULT     TxActivate( LONG * plOldState );
    HRESULT     TxDeactivate( LONG lNewState );
    HRESULT     TxGetClientRect(LPRECT prc);
    HRESULT     TxGetViewInset(LPRECT prc);
    HRESULT     TxGetCharFormat(const CHARFORMATW **ppCF );
    HRESULT     TxGetParaFormat(const PARAFORMAT **ppPF);
    COLORREF    TxGetSysColor(int nIndex);
    HRESULT     TxGetBackStyle(TXTBACKSTYLE *pstyle);
    HRESULT     TxGetMaxLength(DWORD *plength);
    HRESULT     TxGetScrollBars(DWORD *pdwScrollBar);
    HRESULT     TxGetPasswordChar(TCHAR *pch);
    HRESULT     TxGetAcceleratorPos(LONG *pcp);
    HRESULT     TxGetExtent(LPSIZEL lpExtent);
    HRESULT     OnTxCharFormatChange (const CHARFORMATW* pcf);
    HRESULT     OnTxParaFormatChange (const PARAFORMAT* ppf);
    HRESULT     TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits);
    HRESULT     TxNotify(DWORD iNotify, void *pv);

    // Far East Methods for getting the Input Context
    HIMC        TxImmGetContext();
    void        TxImmReleaseContext( HIMC himc );
    HRESULT     TxGetSelectionBarWidth (LONG *lSelBarWidth);

private: // rich text parsing (from IXRichTextParserObserver)
    void    onRichTextParserText(const wchar_t* text, const XTextStyle& style);
    void    onRichTextParserColoredText(const wchar_t* text, const XTextStyle& style, const COLORREF& color);
    void    onRichTextParserLink(const XTextRange& range, const wchar_t* url);
    void    onRichTextParserImage(const wchar_t* imageUri, int width, int height);

private: // helper methods
    void    _textStyleToCharFormat(const XTextStyle& style, CHARFORMAT2* pcf2);
    void    _notifyPropertyChanged(DWORD bit);
    void    _setPropertyBit(DWORD bit, bool value);
    bool    _getPropertyBit(DWORD bit) const;

private: // document methods
    IStorage*       _createStorage();
    IOleClientSite* _getOleClientSite();
    ITextDocument*  _getDocument();
    ITextRange*     _getDocumentRange(int pos, int length);
    ITextFont*      _getRangeFont(int pos, int length);
    ITextSelection* _getSelection();

protected: // properties
    XTextStyle      m_textStyle;
    DWORD           m_propertyBits;
    DWORD           m_maxLength;
    CHARFORMAT2     m_charFormat;
    PARAFORMAT2     m_paraFormat;

protected: // colors
    COLORREF        m_crBackground;
    COLORREF        m_crSelectionText;
    COLORREF        m_crSelectionBackground;
    bool            m_fillBackground;

private: // data
    HWND            m_hwndContainer;
    unsigned long   m_ulRef;
    ITextServices*  m_textServices;
    IRichEditOle*   m_richEditOle;
    RECT            m_rcClient;
    RECT            m_rcContent;
    IXRichEditObserver* m_richEditObserver;
    XRichTextParser m_textParser;
    int             m_parseInsertPos;
};

// XRichTextEdit
/////////////////////////////////////////////////////////////////////

#endif // _XRICHTEXTEDIT_H_


