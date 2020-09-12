// Rich text parsing and generating
//
/////////////////////////////////////////////////////////////////////

#ifndef _XRICHTEXTPARSER_H_
#define _XRICHTEXTPARSER_H_

/////////////////////////////////////////////////////////////////////
// notes

// NOTE: text parser is used to generate rich text object from formatted text
//       and vice versa, generate formatted text from rich text object

// NOTE:  Parser supports small subset of HTML tags and attributes:
//        <p>                       - paragraph
//        <b>                       - bold 
//        <i>                       - italic 
//        <u>                       - underline 
//        <br>                      - line break
//        <font size|color|face>    - font properties
//        <img src|height|width>    - inline image
//        <a href>                  - link 
//
//        All other tags will be removed from output text

// NOTE: parser keywords are words with special meaning that may be replaced
//       by parser user to e.g. inline objects (e.g. emoticons may be replaced with
//       corresponding images)

/////////////////////////////////////////////////////////////////////
// IXRichTextParserObserver - observer interface for text parser

class IXRichTextParserObserver
{
public: // construction/destruction
    IXRichTextParserObserver() {}
    virtual ~IXRichTextParserObserver() {}

public: // interface
    virtual void    onRichTextParserText(const wchar_t* text, const XTextStyle& style) {}
    virtual void    onRichTextParserColoredText(const wchar_t* text, const XTextStyle& style, const COLORREF& color) {}
    virtual void    onRichTextParserLink(const XTextRange& range, const wchar_t* url) {}
    virtual void    onRichTextParserImage(const wchar_t* imageUri, int width, int height) {}
    virtual void    onRichTextParserKeyword(const wchar_t* text, unsigned long id) {}
};

// IXRichTextParserObserver
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// XRichTextParser - formatted text parser

class XRichTextParser
{
public: // construction/destruction
    XRichTextParser();
    ~XRichTextParser();

public: // keywords
    void    setKeyword(const wchar_t* text, unsigned long id);
    void    removeKeyword(const wchar_t* text);
    void    removeKeyword(unsigned long id);

public: // parse 
    void    parseBegin(const XTextStyle& defaulStyle, IXRichTextParserObserver* observer);
    void    parse(const wchar_t* text, size_t length);
    void    parseEnd();

public: // generate
    void    formatText(const XRichText* richText, IXRichTextParserObserver* observer);
    void    formatClipboardText(const XRichText* richText, IXRichTextParserObserver* observer);

private: // types

    // characters
    enum ParserChar
    {
        eParseCharText,
        eParseCharSpace,
        eParseCharTagOpen,
        eParseCharTagClose,
        eParseCharEquals,
        eParseCharSlash,
        eParseCharQuotation,
        eParseCharAmpersand,
        eParseCharSemicolon
    };

    // entities
    enum ParserEntity
    {
        eParseEntityUnknown,
        eParseEntityLessThan,
        eParseEntityGreaterThan,
        eParseEntityAmpersand,
        eParseEntityQuotation,
        eParseEntityApostrophe
    };

    // states
    enum ParserState
    {
        eParseStateInit,
        eParseStateText,
        eParseStateEntity,
        eParseStateTagClose,
        eParseStateTagSelfClose,
        eParseStateTagName,
        eParseStateTagAttrName,
        eParseStateTagAttrValue,
        eParseStateTagAttrValueText,
        eParseStateTagIgnore
    };

    // supported tags
    enum ParserTag
    {
        eParseTagUnknown = 0,
        eParseTagParagraph,
        eParseTagBold,
        eParseTagItalic,
        eParseTagUnderline,
        eParseTagFont,
        eParseTagImage,
        eParseTagLink,
        eParseTagLineBreak
    };

    // supported attributes
    enum ParserAttribute
    {
        eParseAttributeUnknown = 0,
        eParseAttributeSize,
        eParseAttributeColor,
        eParseAttributeFace,
        eParseAttributeSource,
        eParseAttributeHeight,
        eParseAttributeWidth,
        eParseAttributeHref
    };

    // font item
    struct ParserFontInfo
    {
        std::wstring    face;
        int             size;
        COLORREF        textColor;
        bool            hasColor;
    };

    typedef std::map<std::wstring, unsigned long>   _ParserKeywords;
    typedef std::map<ParserAttribute, std::wstring> _ParserTagAttributes;
    typedef std::vector<ParserFontInfo>             _ParserFontStack;

private: // state parsers
    void    _onParseStateText(const wchar_t* text, size_t& pos, size_t length);
    void    _onParseStateEntity(const wchar_t* text, size_t& pos, size_t length);
    void    _onParseStateTagClose(const wchar_t* text, size_t& pos, size_t length);
    void    _onParseStateTagSelfClose(const wchar_t* text, size_t& pos, size_t length);
    void    _onParseStateTagName(const wchar_t* text, size_t& pos, size_t length);
    void    _onParseStateTagAttrName(const wchar_t* text, size_t& pos, size_t length);
    void    _onParseStateTagAttrValue(const wchar_t* text, size_t& pos, size_t length);
    void    _onParseStateTagAttrValueText(const wchar_t* text, size_t& pos, size_t length);
    void    _onParseStateTagIgnore(const wchar_t* text, size_t& pos, size_t length);

private: // worker methods
    void    _resetTag();
    void    _resetState();
    bool    _isParsingState();
    void    _doReportText(const wchar_t* text);
    void    _reportText();
    bool    _nextChar(const wchar_t* text, size_t& pos, size_t length, ParserChar& charOut);
    bool    _matchTextToBuffer(const wchar_t* text);
    bool    _matchArrayToBuffer(const wchar_t** values, size_t count, size_t& idxOut);
    void    _parseTagName(ParserTag& tagOut);
    void    _processTag();
    void    _processTagClose();
    void    _parseEntityName(ParserEntity& entityOut);
    void    _convertEntity();
    void    _parseAttributeName(ParserAttribute& attributeOut);
    void    _processAttribute();
    void    _setFontColor(const wchar_t* color);
    void    _pushFontTag();
    void    _popFontTag();
    void    _processImageTag();

private: // parser state
    XTextStyle                  m_activeStyle;
    XTextStyle                  m_defaultStyle;
    ParserState                 m_parserState;
    ParserTag                   m_parserTag;
    ParserAttribute             m_parserAttribute;
    std::wstring                m_attributeValue;
    _ParserTagAttributes        m_tagAttributes;
    std::vector<wchar_t>        m_parseBuffer;
    std::wstring                m_linkUrl;

private: // parser stack
    _ParserFontStack            m_fontStack;
    unsigned int                m_boldCount;
    unsigned int                m_italicCount;
    unsigned int                m_underlineCount;
    unsigned int                m_linkCount;
    unsigned int                m_reportedCount;
    bool                        m_textReported;
    XTextRange                  m_linkRange;
    COLORREF                    m_textColor;
    bool                        m_hasTextColor;

private: // data
    _ParserKeywords             m_parseKeywords;
    IXRichTextParserObserver*   m_parserObserver;
};

// XRichTextParser
/////////////////////////////////////////////////////////////////////

#endif // _XRICHTEXTPARSER_H_

