// Formatted text storage
//
/////////////////////////////////////////////////////////////////////

#ifndef _XRICHTEXT_H_
#define _XRICHTEXT_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
class XTextInlineObject;

/////////////////////////////////////////////////////////////////////
// includes
#include "xtextstyleindex.h"

/////////////////////////////////////////////////////////////////////
// IXRichTextObserver - observer interface for text changes

class IXRichTextObserver
{
public: // construction/destruction
    IXRichTextObserver() {}
    virtual ~IXRichTextObserver() {}

public: // interface
    virtual void    onRichTextModified() {}
    virtual void    onRichTextAdded(const XTextRange& range) {}
    virtual void    onRichTextRemoved(const XTextRange& range) {}
    virtual void    onRichTextStyleChanged(const XTextRange& range) {}
    virtual void    onRichTextColorChanged(const XTextRange& range) {}
};

// IXRichTextObserver
/////////////////////////////////////////////////////////////////////

// TODO: add support for inline objects over text ranges:
//       - use XTEXTSTYLE_NOT_AN_INDEX_MASK to mark that text is replaced with 
//         inline object
//       - for each character store style anyway (in a separate buffer)
//       - possibility to switch off inline object and display corresponding 
//         text instead
//       - layout code will have to be updated as well to support inline
//         objects for text ranges (e.g. update "advances" appropriately)
//       - update inline object interface so it can get style and text that
//         is associated with it (before paint event, in case e.g. inline object
//         is just a special effect for text - or separate methods for text effects???)
//
// TODO: is it better to leave inline objects as they are now and add special text
//       drawing effects for cases outlined above??? (e.g. smily detection, tabs, lists...)
//       or combine those two??? what is inline object anyway???
//
// TODO: extra style may help to deal with inline objects, drawing effects and so on,
//       but this will add extra memory overhead. Perhaps removing reference to richtext
//       in layouts may help. It may work like: script -> rich text -> layout (free richtext 
//       object)

/////////////////////////////////////////////////////////////////////
// XRichText - simple formatted text storage implementation

class XRichText
{
public: // construction/destruction
    XRichText(IXRichTextObserver* observerRef = 0);
    ~XRichText();

public: // text changes observer
    void            addObserver(IXRichTextObserver* observerRef);
    void            removeObserver(IXRichTextObserver* observerRef);

public: // reset
    void            reset();

public: // text 
    void            setText(const wchar_t* text);
    void            getText(std::wstring& text, const XTextRange& range) const;

public: // internal text data
    const wchar_t*  data() const;
    wchar_t*        data();

public: // edit text
    void            insertText(int textPos, const wchar_t* text, const XTextStyle* style = 0, const COLORREF* textColor = 0);
    void            appendText(const wchar_t* text, const XTextStyle* style = 0, const COLORREF* textColor = 0);
    void            deleteText(const XTextRange& range);
    wchar_t         charAt(int textPos) const;

public: // text runs (parts of text with the same style)
    int             getTextRun(int textPos, XTextStyle& textStyle, bool& hasInlineObject, int maxLength = 0) const;

public: // color runs (parts of text with the same colors)
    int             getColorRun(int textPos, int maxLength = 0) const;
    bool            sameColors(int textPos1, int textPos2) const;

public: // text properties
    XTextRange      totalRange() const;
    unsigned int    textLength() const;
    void            splitToLines(std::vector<XTextRange>& textLines);

public: // inline objects
    bool            hasInlineObjects() const;
    void            insertInlineObject(int textPos, XTextInlineObject* inlineObject, const XTextStyle* style = 0);
    void            appendInlineObject(XTextInlineObject* inlineObject, const XTextStyle* style = 0);
    bool            isInlineObjectAt(int textPos) const;
    XTextInlineObject*  inlineObjectAt(int textPos) const;

public: // enum inline objects
    void            getInlineObjects(std::vector<XTextInlineObject*>& objectsOut);
    size_t          inlineObjectCount() const;
    XTextInlineObject*  getInlineObject(size_t index) const;

public: // style ranges
    void            getTextStyles(const XTextRange& range, std::vector<xstyle_index_t>& styles) const;
    void            applyTextStyles(const XTextRange& range, const std::vector<xstyle_index_t>& styles);
    void            applyTextStyleMask(const XTextRange& range, xstyle_index_t style, xstyle_index_t styleMask);
    xstyle_index_t  styleIndexAt(int textPos) const;

public: // set text style properties
    void            setTextStyle(const XTextStyle& style, const XTextRange& range);
    void            setFont(const std::wstring& fontName, const XTextRange& range);
    void            setFontSize(int fontSize, const XTextRange& range);
    void            setBold(bool bold, const XTextRange& range);
    void            setItalic(bool italic, const XTextRange& range);
    void            setUnderline(bool underline, const XTextRange& range);
    void            setStrike(bool strike, const XTextRange& range);
    void            setRTL(bool rtldir, const XTextRange& range);
    void            setTextColor(COLORREF textColor, const XTextRange& range);
    void            setBackgroundColor(COLORREF backgroundColor, const XTextRange& range);
    void            clearTextColor(const XTextRange& range);
    void            clearBackgroundColor(const XTextRange& range);

public: // set text style properties (for whole range)
    void            setTextStyle(const XTextStyle& style);
    void            setFont(const std::wstring& fontName);
    void            setTextColor(COLORREF textColor);

public: // get text style properties
    XTextStyle      textStyle(int textPos, XTextRange* range = 0) const;
    std::wstring    fontName(int textPos, XTextRange* range = 0) const;
    int             fontSize(int textPos, XTextRange* range = 0) const;
    bool            isBold(int textPos, XTextRange* range = 0) const;
    bool            isItalic(int textPos, XTextRange* range = 0) const;
    bool            isUnderline(int textPos, XTextRange* range = 0) const;
    bool            isStrike(int textPos, XTextRange* range = 0) const;
    bool            isRTL(int textPos, XTextRange* range = 0) const;
    bool            textColor(int textPos, COLORREF& textColor, XTextRange* range = 0) const;
    bool            backgroundColor(int textPos, COLORREF& backgroundColor, XTextRange* range = 0) const;

public: // text style hashing (support for style caching)
    xstyle_index_t  hashFromTextStyle(const XTextStyle& style);
    XTextStyle      textStyleFromHash(xstyle_index_t styleHash) const;

private: // protect from copy and assignment
    XRichText(const XRichText& ref)  {}
    const XRichText& operator=(const XRichText& ref) { return *this;}

private: // helper methods
    bool    _validateRange(const XTextRange& range) const;
    bool    _validateTextPos(int textPos) const;

private: // inline object styles
    xstyle_index_t  _inlineObjectStyle(unsigned int pos) const;
    void            _setInlineObjectStyle(unsigned int pos, xstyle_index_t style);

private: // observer
    IXRichTextObserver* m_observerRef;

private: // style index
    XTextStyleIndex     m_styleIndex;

private: // inline objects
    struct InlineObject
    {
        XTextInlineObject*      object;
        xstyle_index_t          style;
    };

private: // data
    std::vector<wchar_t>            m_text;
    std::vector<xstyle_index_t>     m_styles;
    std::vector<InlineObject>       m_inlineObjects;
};

// XRichText
/////////////////////////////////////////////////////////////////////

#endif // _XRICHTEXT_H_

