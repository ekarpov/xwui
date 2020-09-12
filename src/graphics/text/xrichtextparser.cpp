// Text script parsing and generating
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"

#include "xtextinlineobject.h"
#include "xrichtext.h"
#include "xrichtextparser.h"

/////////////////////////////////////////////////////////////////////
// constants

// tag names (NOTE: order must match)
static const wchar_t* _XRichTextParserTagNames[] = 
{
    L"",            // eParseTagUnknown
    L"p",           // eParseTagParagraph
    L"b",           // eParseTagBold
    L"i",           // eParseTagItalic
    L"u",           // eParseTagUnderline
    L"font",        // eParseTagFont
    L"img",         // eParseTagImage
    L"a",           // eParseTagLink
    L"br",          // eParseTagLineBreak
};

// attribute names (NOTE: order must match)
static const wchar_t* _XRichTextParserAttrNames[] = 
{
    L"",            // eParseAttributeUnknown
    L"size",        // eParseAttributeSize
    L"color",       // eParseAttributeColor
    L"face",        // eParseAttributeFace
    L"src",         // eParseAttributeSource
    L"height",      // eParseAttributeHeight
    L"width",       // eParseAttributeWidth
    L"href",        // eParseAttributeHref
};

// entity names (NOTE: order must match)
static const wchar_t* _XRichTextParserEntityNames[] = 
{
    L"",            // eParseEntityUnknown
    L"&lt;",        // eParseEntityLessThan
    L"&gt;",        // eParseEntityGreaterThan
    L"&amp;",       // eParseEntityAmpersand
    L"&quot;",      // eParseEntityQuotation
    L"&apos;",      // eParseEntityApostrophe
};

// entity values (NOTE: order must match)
static const wchar_t* _XRichTextParserEntityValues[] = 
{
    L"",            // eParseEntityUnknown
    L"<",           // eParseEntityLessThan
    L">",           // eParseEntityGreaterThan
    L"&",           // eParseEntityAmpersand
    L"\"",          // eParseEntityQuotation
    L"\'",          // eParseEntityApostrophe
};

#define XRICHTEXT_TAG_NAME_COUNT        sizeof(_XRichTextParserTagNames) / sizeof(_XRichTextParserTagNames[0])
#define XRICHTEXT_ATTR_NAME_COUNT       sizeof(_XRichTextParserAttrNames) / sizeof(_XRichTextParserAttrNames[0])
#define XRICHTEXT_ENTITY_NAME_COUNT     sizeof(_XRichTextParserEntityNames) / sizeof(_XRichTextParserEntityNames[0])
#define XRICHTEXT_ENTITY_VALUE_COUNT    sizeof(_XRichTextParserEntityValues) / sizeof(_XRichTextParserEntityValues[0])

/////////////////////////////////////////////////////////////////////
// color names

struct _XRichTextParseColorIndex
{
    const wchar_t*  name;
    DWORD           color;
};

// supported names
static const _XRichTextParseColorIndex _XRichTextParseColorNames[] =
{
    {L"AliceBlue", 0xF0F8FF},
    {L"AntiqueWhite", 0xFAEBD7},
    {L"Aqua", 0x00FFFF},
    {L"Aquamarine", 0x7FFFD4},
    {L"Azure", 0xF0FFFF},
    {L"Beige", 0xF5F5DC},
    {L"Bisque", 0xFFE4C4},
    {L"Black", 0x000000},
    {L"BlanchedAlmond", 0xFFEBCD},
    {L"Blue", 0x0000FF},
    {L"BlueViolet", 0x8A2BE2},
    {L"Brown", 0xA52A2A},
    {L"BurlyWood", 0xDEB887},
    {L"CadetBlue", 0x5F9EA0},
    {L"Chartreuse", 0x7FFF00},
    {L"Chocolate", 0xD2691E},
    {L"Coral", 0xFF7F50},
    {L"CornflowerBlue", 0x6495ED},
    {L"Cornsilk", 0xFFF8DC},
    {L"Crimson", 0xDC143C},
    {L"Cyan", 0x00FFFF},
    {L"DarkBlue", 0x00008B},
    {L"DarkCyan", 0x008B8B},
    {L"DarkGoldenRod", 0xB8860B},
    {L"DarkGray", 0xA9A9A9},
    {L"DarkGrey", 0xA9A9A9},
    {L"DarkGreen", 0x006400},
    {L"DarkKhaki", 0xBDB76B},
    {L"DarkMagenta", 0x8B008B},
    {L"DarkOliveGreen", 0x556B2F},
    {L"DarkOrange", 0xFF8C00},
    {L"DarkOrchid", 0x9932CC},
    {L"DarkRed", 0x8B0000},
    {L"DarkSalmon", 0xE9967A},
    {L"DarkSeaGreen", 0x8FBC8F},
    {L"DarkSlateBlue", 0x483D8B},
    {L"DarkSlateGray", 0x2F4F4F},
    {L"DarkSlateGrey", 0x2F4F4F},
    {L"DarkTurquoise", 0x00CED1},
    {L"DarkViolet", 0x9400D3},
    {L"DeepPink", 0xFF1493},
    {L"DeepSkyBlue", 0x00BFFF},
    {L"DimGray", 0x696969},
    {L"DimGrey", 0x696969},
    {L"DodgerBlue", 0x1E90FF},
    {L"FireBrick", 0xB22222},
    {L"FloralWhite", 0xFFFAF0},
    {L"ForestGreen", 0x228B22},
    {L"Fuchsia", 0xFF00FF},
    {L"Gainsboro", 0xDCDCDC},
    {L"GhostWhite", 0xF8F8FF},
    {L"Gold", 0xFFD700},
    {L"GoldenRod", 0xDAA520},
    {L"Gray", 0x808080},
    {L"Grey", 0x808080},
    {L"Green", 0x008000},
    {L"GreenYellow", 0xADFF2F},
    {L"HoneyDew", 0xF0FFF0},
    {L"HotPink", 0xFF69B4},
    {L"IndianRed ", 0xCD5C5C},
    {L"Indigo ", 0x4B0082},
    {L"Ivory", 0xFFFFF0},
    {L"Khaki", 0xF0E68C},
    {L"Lavender", 0xE6E6FA},
    {L"LavenderBlush", 0xFFF0F5},
    {L"LawnGreen", 0x7CFC00},
    {L"LemonChiffon", 0xFFFACD},
    {L"LightBlue", 0xADD8E6},
    {L"LightCoral", 0xF08080},
    {L"LightCyan", 0xE0FFFF},
    {L"LightGoldenRodYellow", 0xFAFAD2},
    {L"LightGray", 0xD3D3D3},
    {L"LightGrey", 0xD3D3D3},
    {L"LightGreen", 0x90EE90},
    {L"LightPink", 0xFFB6C1},
    {L"LightSalmon", 0xFFA07A},
    {L"LightSeaGreen", 0x20B2AA},
    {L"LightSkyBlue", 0x87CEFA},
    {L"LightSlateGray", 0x778899},
    {L"LightSlateGrey", 0x778899},
    {L"LightSteelBlue", 0xB0C4DE},
    {L"LightYellow", 0xFFFFE0},
    {L"Lime", 0x00FF00},
    {L"LimeGreen", 0x32CD32},
    {L"Linen", 0xFAF0E6},
    {L"Magenta", 0xFF00FF},
    {L"Maroon", 0x800000},
    {L"MediumAquaMarine", 0x66CDAA},
    {L"MediumBlue", 0x0000CD},
    {L"MediumOrchid", 0xBA55D3},
    {L"MediumPurple", 0x9370DB},
    {L"MediumSeaGreen", 0x3CB371},
    {L"MediumSlateBlue", 0x7B68EE},
    {L"MediumSpringGreen", 0x00FA9A},
    {L"MediumTurquoise", 0x48D1CC},
    {L"MediumVioletRed", 0xC71585},
    {L"MidnightBlue", 0x191970},
    {L"MintCream", 0xF5FFFA},
    {L"MistyRose", 0xFFE4E1},
    {L"Moccasin", 0xFFE4B5},
    {L"NavajoWhite", 0xFFDEAD},
    {L"Navy", 0x000080},
    {L"OldLace", 0xFDF5E6},
    {L"Olive", 0x808000},
    {L"OliveDrab", 0x6B8E23},
    {L"Orange", 0xFFA500},
    {L"OrangeRed", 0xFF4500},
    {L"Orchid", 0xDA70D6},
    {L"PaleGoldenRod", 0xEEE8AA},
    {L"PaleGreen", 0x98FB98},
    {L"PaleTurquoise", 0xAFEEEE},
    {L"PaleVioletRed", 0xDB7093},
    {L"PapayaWhip", 0xFFEFD5},
    {L"PeachPuff", 0xFFDAB9},
    {L"Peru", 0xCD853F},
    {L"Pink", 0xFFC0CB},
    {L"Plum", 0xDDA0DD},
    {L"PowderBlue", 0xB0E0E6},
    {L"Purple", 0x800080},
    {L"RebeccaPurple", 0x663399},
    {L"Red", 0xFF0000},
    {L"RosyBrown", 0xBC8F8F},
    {L"RoyalBlue", 0x4169E1},
    {L"SaddleBrown", 0x8B4513},
    {L"Salmon", 0xFA8072},
    {L"SandyBrown", 0xF4A460},
    {L"SeaGreen", 0x2E8B57},
    {L"SeaShell", 0xFFF5EE},
    {L"Sienna", 0xA0522D},
    {L"Silver", 0xC0C0C0},
    {L"SkyBlue", 0x87CEEB},
    {L"SlateBlue", 0x6A5ACD},
    {L"SlateGray", 0x708090},
    {L"SlateGrey", 0x708090},
    {L"Snow", 0xFFFAFA},
    {L"SpringGreen", 0x00FF7F},
    {L"SteelBlue", 0x4682B4},
    {L"Tan", 0xD2B48C},
    {L"Teal", 0x008080},
    {L"Thistle", 0xD8BFD8},
    {L"Tomato", 0xFF6347},
    {L"Turquoise", 0x40E0D0},
    {L"Violet", 0xEE82EE},
    {L"Wheat", 0xF5DEB3},
    {L"White", 0xFFFFFF},
    {L"WhiteSmoke", 0xF5F5F5},
    {L"Yellow", 0xFFFF00},
    {L"YellowGreen", 0x9ACD32}
};

#define XRICHTEXT_COLOR_NAME_COUNT  sizeof(_XRichTextParseColorNames) / sizeof(_XRichTextParseColorNames[0])

/////////////////////////////////////////////////////////////////////
// XRichTextParser - formatted text parser

XRichTextParser::XRichTextParser() :
    m_parserObserver(0),
    m_parserState(eParseStateInit)
{
}

XRichTextParser::~XRichTextParser()
{
}

/////////////////////////////////////////////////////////////////////
// keywords
/////////////////////////////////////////////////////////////////////
void XRichTextParser::setKeyword(const wchar_t* text, unsigned long id)
{
    XWASSERT(text);
    if(text == 0) return;

    // insert keyword
    m_parseKeywords.insert(_ParserKeywords::value_type(text, id));
}

void XRichTextParser::removeKeyword(const wchar_t* text)
{
    XWASSERT(text);
    if(text == 0) return;

    // find keyword
    _ParserKeywords::iterator it = m_parseKeywords.find(text);
    if(it != m_parseKeywords.end())
    {
        m_parseKeywords.erase(it);
    }
}

void XRichTextParser::removeKeyword(unsigned long id)
{
    // find keyword by id
    for(_ParserKeywords::iterator it = m_parseKeywords.begin();
        it != m_parseKeywords.end(); ++it)
    {
        // check id
        if(it->second == id)
        {
            // remove keyword
            it = m_parseKeywords.erase(it);
        }
    }
}

/////////////////////////////////////////////////////////////////////
// parse 
/////////////////////////////////////////////////////////////////////
void XRichTextParser::parseBegin(const XTextStyle& defaulStyle, IXRichTextParserObserver* observer)
{
    // check input
    XWASSERT(observer);
    if(observer == 0) return;

    // reset state
    _resetState();

    // jump to text state
    m_parserState = eParseStateText;

    // observer
    m_parserObserver = observer;

    // style
    m_activeStyle = defaulStyle;
    m_defaultStyle = defaulStyle;
}

void XRichTextParser::parse(const wchar_t* text, size_t length)
{
    // check state
    if(!_isParsingState()) return;

    // check input
    if(text == 0 || length == 0) return;

    // process text
    for(size_t pos = 0; pos < length; )
    {
        // state specific processing
        switch(m_parserState)
        {
        case eParseStateText:               _onParseStateText(text, pos, length); break;
        case eParseStateEntity:             _onParseStateEntity(text, pos, length); break;
        case eParseStateTagClose:           _onParseStateTagClose(text, pos, length); break;
        case eParseStateTagSelfClose:       _onParseStateTagSelfClose(text, pos, length); break;
        case eParseStateTagName:            _onParseStateTagName(text, pos, length); break;
        case eParseStateTagAttrName:        _onParseStateTagAttrName(text, pos, length); break;
        case eParseStateTagAttrValue:       _onParseStateTagAttrValue(text, pos, length); break;
        case eParseStateTagAttrValueText:   _onParseStateTagAttrValueText(text, pos, length); break;
        case eParseStateTagIgnore:          _onParseStateTagIgnore(text, pos, length); break;

        default:
            XWASSERT1(0, "XRichTextParser: parser state is not valid");
            return;
            break;
        }
    }
}

void XRichTextParser::parseEnd()
{
    // check state
    if(!_isParsingState()) return;

    // report remaining text
    if(m_parserState == eParseStateText)
    {
        _reportText();

    } else
    {
        XWTRACE("XRichTextParser: incomplete input given, parser state is not valid");
    }

    // reset state
    _resetState();
}

/////////////////////////////////////////////////////////////////////
// generate
/////////////////////////////////////////////////////////////////////
void XRichTextParser::formatText(const XRichText* richText, IXRichTextParserObserver* observer)
{
    // TODO:
}

void XRichTextParser::formatClipboardText(const XRichText* richText, IXRichTextParserObserver* observer)
{
    // TODO:
}

/////////////////////////////////////////////////////////////////////
// state parsers
/////////////////////////////////////////////////////////////////////
void XRichTextParser::_onParseStateText(const wchar_t* text, size_t& pos, size_t length)
{
    ParserChar pchar;
    while(m_parserState == eParseStateText && _nextChar(text, pos, length, pchar))
    {
        if(pchar == eParseCharTagOpen)
        {
            // report text
            _reportText();

            // reset active tag if any
            _resetTag();

            // jump to next state
            m_parserState = eParseStateTagName;

        } else if(pchar == eParseCharAmpersand)
        {
            // report text
            _reportText();

            // copy current character
            m_parseBuffer.push_back(text[pos - 1]);

            // jump to next state
            m_parserState = eParseStateEntity;

        } else
        {
            // consume all other characters
            m_parseBuffer.push_back(text[pos-1]);
        }
    }
}

void XRichTextParser::_onParseStateEntity(const wchar_t* text, size_t& pos, size_t length)
{
    ParserChar pchar;
    while(m_parserState == eParseStateEntity && _nextChar(text, pos, length, pchar))
    {
        // NOTE: consume all characters, in case we cannot parse entity we will report it as such
        m_parseBuffer.push_back(text[pos-1]);

        if(pchar == eParseCharSemicolon)
        {
            // convert entity to buffer
            _convertEntity();

        } else if(pchar != eParseCharSpace)
        {
            XWTRACE("XRichTextParser: invalid character in entity name");
        }

        // jump to text state
        m_parserState = eParseStateText;
    }
}

void XRichTextParser::_onParseStateTagClose(const wchar_t* text, size_t& pos, size_t length)
{
    ParserChar pchar;
    while(m_parserState == eParseStateTagClose && _nextChar(text, pos, length, pchar))
    {
        if(pchar == eParseCharTagClose)
        {
            if(m_parseBuffer.size() != 0)
            {
                // parse tag
                _parseTagName(m_parserTag);

                if(m_parserTag != eParseTagUnknown)
                {
                    // tag ready, process it
                    _processTagClose();
                }
            }

            // jump to text state 
            m_parserState = eParseStateText;
        }
    }
}

void XRichTextParser::_onParseStateTagSelfClose(const wchar_t* text, size_t& pos, size_t length)
{
    ParserChar pchar;
    if(_nextChar(text, pos, length, pchar))
    {
        if(pchar == eParseCharTagClose)
        {
            if(m_parseBuffer.size() != 0)
            {
                // parse tag
                _parseTagName(m_parserTag);

                if(m_parserTag != eParseTagUnknown)
                {
                    // copy active tag
                    ParserTag tag = m_parserTag;

                    // first process tag opening
                    _processTag();

                    // restore active tag
                    m_parserTag = tag;

                    // process tag close
                    _processTagClose();
                }
            }

            // jump to text state 
            m_parserState = eParseStateText;
        }
    }
}

void XRichTextParser::_onParseStateTagName(const wchar_t* text, size_t& pos, size_t length)
{
    // process characters
    ParserChar pchar;
    if(_nextChar(text, pos, length, pchar))
    {
        if(pchar == eParseCharSlash && m_parseBuffer.size() == 0)
        {
            // tag is closing (e.g. </tag>)
            m_parserState = eParseStateTagClose;

        } else
        { 
            // parse tag
            _parseTagName(m_parserTag);

            if(m_parserTag != eParseTagUnknown)
            {
                // return character "back"
                pos--;

                // jump to next state 
                m_parserState = eParseStateTagAttrName;

            } else
            {
                // ignore tag
                m_parserState = eParseStateTagIgnore;
            }
        }
    }
}

void XRichTextParser::_onParseStateTagAttrName(const wchar_t* text, size_t& pos, size_t length)
{
    ParserChar pchar;
    if(_nextChar(text, pos, length, pchar))
    {
        if(pchar == eParseCharSlash && m_parseBuffer.size() == 0)
        {
            // tag is self-closing (e.g. <tag/>)
            m_parserState = eParseStateTagSelfClose;

        } else if(pchar == eParseCharTagClose)
        {
            // tag ready, process it
            _processTag();

            // back to text state
            m_parserState = eParseStateText;

        } else if(pchar == eParseCharEquals)
        {
            // parse attribute 
            _parseAttributeName(m_parserAttribute);

            // switch to attribute value
            m_parserState = eParseStateTagAttrValue;

        } else
        {
            // parse attribute 
            _parseAttributeName(m_parserAttribute);

            // process attribute without value
            _processAttribute();
        }
    }
}

void XRichTextParser::_onParseStateTagAttrValue(const wchar_t* text, size_t& pos, size_t length)
{
    ParserChar pchar;
    while(m_parserState == eParseStateTagAttrValue && _nextChar(text, pos, length, pchar))
    {
        if(pchar == eParseCharQuotation && m_parseBuffer.size() == 0)
        {
            // jump to text value state
            m_parserState = eParseStateTagAttrValueText;

        } else if(pchar == eParseCharTagClose || pchar == eParseCharSpace)
        {
            // process attribute
            _processAttribute();

            // report character back
            pos--;

            // switch to attribute name
            m_parserState = eParseStateTagAttrName;

        } else
        {
            // threat all other characters as value
            m_parseBuffer.push_back(text[pos - 1]);
        }
    }
}

void XRichTextParser::_onParseStateTagAttrValueText(const wchar_t* text, size_t& pos, size_t length)
{
    ParserChar pchar;

    // read text value
    while(m_parserState == eParseStateTagAttrValueText && _nextChar(text, pos, length, pchar))
    {
        if(pchar == eParseCharQuotation)
        {
            // process attribute
            _processAttribute();

            // switch to attribute name
            m_parserState = eParseStateTagAttrName;

        } else
        {
            // threat all other characters as value
            m_parseBuffer.push_back(text[pos - 1]);
        }
    }
}

void XRichTextParser::_onParseStateTagIgnore(const wchar_t* text, size_t& pos, size_t length)
{
    ParserChar pchar;

    // ignore unknown tag name with all its attributes
    while(m_parserState == eParseStateTagIgnore && _nextChar(text, pos, length, pchar))
    {
        if(pchar == eParseCharTagClose)
        {
            // reset buffer
            m_parseBuffer.clear();

            // jump to next state
            m_parserState = eParseStateText;
        }
    }
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XRichTextParser::_resetTag()
{
    // reset active tag
    m_parserTag = eParseTagUnknown;
    m_parserAttribute = eParseAttributeUnknown;
    m_attributeValue.clear();
    m_tagAttributes.clear();
}

void XRichTextParser::_resetState()
{
    // reset parser state
    m_parserState = eParseStateInit;
    m_parseBuffer.clear();
    m_textReported = false;
    m_linkRange.length = 0;
    m_linkRange.pos = 0;
    m_linkUrl.clear();

    // reset tag
    _resetTag();

    // reset parser stack
    m_fontStack.clear();
    m_boldCount = 0;
    m_italicCount = 0;
    m_underlineCount = 0;
    m_linkCount = 0;
    m_reportedCount = 0;
    m_hasTextColor = false;
}

bool XRichTextParser::_isParsingState()
{
    // check state
    XWASSERT(m_parserState != eParseStateInit);
    XWASSERT(m_parserObserver);
    if(m_parserState == eParseStateInit || m_parserObserver == 0) 
    {
        XWTRACE("XRichTextParser: parser is not initialized");
        return false;
    }

    return true;
}

void XRichTextParser::_doReportText(const wchar_t* text)
{
    // report text
    if(m_hasTextColor)
        m_parserObserver->onRichTextParserColoredText(text, m_activeStyle, m_textColor);
    else
        m_parserObserver->onRichTextParserText(text, m_activeStyle);

    // update counter
    m_reportedCount += (unsigned int)::wcslen(text);
}

void XRichTextParser::_reportText()
{
    // check state
    XWASSERT(m_parserState == eParseStateText);
    if(m_parserState != eParseStateText) return;

    // ignore if nothing to report
    if(m_parseBuffer.size() == 0) return;

    // update counter
    m_reportedCount += (unsigned int)m_parseBuffer.size();

    // end of line
    m_parseBuffer.push_back(0);

    // report text
    if(m_hasTextColor)
        m_parserObserver->onRichTextParserColoredText(m_parseBuffer.data(), m_activeStyle, m_textColor);
    else
        m_parserObserver->onRichTextParserText(m_parseBuffer.data(), m_activeStyle);

    // reset buffer
    m_parseBuffer.clear();

    // mark flag
    m_textReported = true;
}

bool XRichTextParser::_nextChar(const wchar_t* text, size_t& pos, size_t length, ParserChar& charOut)
{
    // process characters
    for(; pos < length; ++pos)
    {
        // process char
        charOut = ::iswspace(text[pos]) ? eParseCharSpace : eParseCharText;
        switch(text[pos])
        {
        case L'<':  charOut = eParseCharTagOpen; break;
        case L'>':  charOut = eParseCharTagClose; break;
        case L'=':  charOut = eParseCharEquals; break;
        case L'/':  charOut = eParseCharSlash; break;
        case L'\"': charOut = eParseCharQuotation; break;
        case L'&':  charOut = eParseCharAmpersand; break;
        case L';':  charOut = eParseCharSemicolon; break;
        }

        // process spaces
        if(charOut == eParseCharSpace)
        {
            // skip spaces in front
            if(m_parseBuffer.size() == 0 && !m_textReported) continue;

            // skip multiple spaces
            if(m_parseBuffer.size() > 0 && ::iswspace(m_parseBuffer.back())) continue;
        }

        // skip line breaks
        if(text[pos] == L'\n' || text[pos] == L'\r') continue;

        // stop if not text
        if(charOut != eParseCharText) 
        {
            pos++;
            return true;
        }

        // copy to buffer
        if(m_parserState != eParseStateTagIgnore)
        {
            m_parseBuffer.push_back(text[pos]);
        }
    }

    return false;
}

bool XRichTextParser::_matchTextToBuffer(const wchar_t* text)
{
    // match text
    for(size_t idx = 0; idx < m_parseBuffer.size(); ++idx)
    {
        // check if text ends
        if(text[idx] == 0) return false;

        // case not sensitive compare
        if(towlower(text[idx]) != towlower(m_parseBuffer.at(idx))) return false;
    }

    // check if text also ends
    return (text[m_parseBuffer.size()] == 0);
}

bool XRichTextParser::_matchArrayToBuffer(const wchar_t** values, size_t count, size_t& idxOut)
{
    // loop over array (NOTE: ignore first value)
    for(idxOut = 1; idxOut < count; ++idxOut)
    {
        // match value
        if(_matchTextToBuffer(values[idxOut])) return true;
    }

    // not found
    return false;
}

void XRichTextParser::_parseTagName(ParserTag& tagOut)
{
    // reset output
    tagOut = eParseTagUnknown;

    // check if there is any name
    if(m_parseBuffer.size() == 0)
    {
        XWTRACE("XRichTextParser: empty tag name ignored");
        return;
    }

    // match any known tags 
    size_t idx;
    if(_matchArrayToBuffer(_XRichTextParserTagNames, XRICHTEXT_TAG_NAME_COUNT, idx))
    {
        // tag found
        tagOut = (ParserTag)idx;
    }

    // reset buffer
    m_parseBuffer.clear();
}

void XRichTextParser::_processTag()
{
    // process known tags
    switch(m_parserTag)
    {
    case eParseTagParagraph:
        // report end of line if there was some text
        if(m_textReported)
            _doReportText(L"\n");

        // reset flag
        m_textReported = false;
        break;

    case eParseTagBold:
        // set bold style
        m_activeStyle.bBold = true;

        // stack counter
        m_boldCount++;
        break;

    case eParseTagItalic:
        // set italic style
        m_activeStyle.bItalic = true;

        // stack counter
        m_italicCount++;
        break;

    case eParseTagUnderline:
        // set underline style
        m_activeStyle.bUnderline = true;

        // stack counter
        m_underlineCount++;
        break;

    case eParseTagFont:
        _pushFontTag();
        break;

    case eParseTagImage:
        _processImageTag();

        // mark flag
        m_textReported = true;
        break;

    case eParseTagLink:
        if(m_linkCount == 0)
        {
            // init link range
            m_linkRange.pos = m_reportedCount;
            m_linkRange.length = 0; 

            // set link url
            _ParserTagAttributes::const_iterator it = m_tagAttributes.find(eParseAttributeHref);
            if(it != m_tagAttributes.end())
            {
                m_linkUrl = it->second;

            } else
            {
                m_linkUrl.clear();
            }            

        } else
        {
            XWTRACE("XRichTextParser: embedded links are not supported");
        }

        // update counter
        m_linkCount++;
        break;

    case eParseTagLineBreak:
        // report end of line
        _doReportText(L"\n");
        break;
    }

    // reset active tag
    _resetTag();

    // reset buffer
    m_parseBuffer.clear();
}

void XRichTextParser::_processTagClose()
{
    // reset buffer
    m_parseBuffer.clear();

    // process known tags
    switch(m_parserTag)
    {
    case eParseTagParagraph:
        // report end of line (twice) if there was some text
        if(m_textReported)
            _doReportText(L"\n\n");

        // reset flag
        m_textReported = false;
        break;

    case eParseTagBold:
        // stack counter
        m_boldCount--;

        // set bold style
        if(m_boldCount == 0)
            m_activeStyle.bBold = m_defaultStyle.bBold;
        break;

    case eParseTagItalic:
        // stack counter
        m_italicCount--;

        // set italic style
        if(m_italicCount == 0)
            m_activeStyle.bItalic = m_defaultStyle.bItalic;
        break;

    case eParseTagUnderline:
        // stack counter
        m_underlineCount--;

        // set underline style
        if(m_underlineCount == 0)
            m_activeStyle.bUnderline = m_defaultStyle.bUnderline;
        break;

    case eParseTagFont:
        _popFontTag();
        break;

    case eParseTagImage:
        // NOTE: image close tag is senseless, just ignore
        break;

    case eParseTagLink:
        if(m_linkCount == 1)
        {
            // update range
            m_linkRange.length = m_reportedCount - m_linkRange.pos;  // TODO: check if +1 is needed

            // report link to observer
            m_parserObserver->onRichTextParserLink(m_linkRange, m_linkUrl.c_str());
        }

        // update counter
        m_linkCount--;
        break;

    case eParseTagLineBreak:
        // ignore
        break;
    }

    // reset active tag
    _resetTag();
}

void XRichTextParser::_parseEntityName(ParserEntity& entityOut)
{
    // reset output
    entityOut = eParseEntityUnknown;

    // match any known entities 
    size_t idx;
    if(_matchArrayToBuffer(_XRichTextParserEntityNames, XRICHTEXT_ENTITY_NAME_COUNT, idx))
    {
        // found
        entityOut = (ParserEntity)idx;
    }
}

void XRichTextParser::_convertEntity()
{
    // ignore if there is any data
    if(m_parseBuffer.size() == 0) return;

    ParserEntity entity;

    // try to parse by name
    _parseEntityName(entity);
    if(entity != eParseEntityUnknown && entity < XRICHTEXT_ENTITY_VALUE_COUNT)
    {
        // convert entity
        const wchar_t* entityText = _XRichTextParserEntityValues[entity];
        XWASSERT(entityText);

        // copy converted text to buffer
        m_parseBuffer.assign(entityText, entityText + ::wcslen(entityText));

    } else
    {
        // check if enity is number
        if(m_parseBuffer.size() >= 4 && 
           m_parseBuffer.size() <= 7 &&
           m_parseBuffer[0] == L'&' && 
           m_parseBuffer[1] == L'#' && 
           m_parseBuffer[m_parseBuffer.size() - 1] == L';')
        {
            // validate numbers
            bool isnumber = true;
            for(size_t idx = 2; idx < m_parseBuffer.size() - 1; ++idx)
            {
                if(m_parseBuffer[idx] < L'0' || 
                   m_parseBuffer[idx] > L'9')
                {
                    isnumber = false;
                    break;
                }
            }

            // convert
            if(isnumber)
            {
                // replace column with end of line
                m_parseBuffer[m_parseBuffer.size() - 1] = 0;

                // convert
                wchar_t wch = (wchar_t)_wtoi(m_parseBuffer.data() + 2);

                // copy converted character to buffer
                if(wch != 0)
                {
                    m_parseBuffer.resize(1);
                    m_parseBuffer[0] = wch;
                }
            }
        }
    }

    // NOTE: in case conversion failed just ignore and entity will be reported as such
}

void XRichTextParser::_parseAttributeName(ParserAttribute& attributeOut)
{
    // reset output
    attributeOut = eParseAttributeUnknown;

    // match any known entities 
    size_t idx;
    if(_matchArrayToBuffer(_XRichTextParserAttrNames, XRICHTEXT_ATTR_NAME_COUNT, idx))
    {
        // found
        attributeOut = (ParserAttribute)idx;
    }

    // reset buffer
    m_parseBuffer.clear();
}

void XRichTextParser::_processAttribute()
{
    // add attribute to tag
    if(m_parserAttribute != eParseAttributeUnknown)
    {
        // copy value to string
        m_attributeValue.assign(m_parseBuffer.data(), m_parseBuffer.data() + m_parseBuffer.size());

        // add
        m_tagAttributes.insert(_ParserTagAttributes::value_type(m_parserAttribute, m_attributeValue));
    }

    // reset attribute
    m_parserAttribute = eParseAttributeUnknown;
    m_attributeValue.clear();

    // reset buffer
    m_parseBuffer.clear();
}

void XRichTextParser::_setFontColor(const wchar_t* color)
{
    COLORREF parsedColor;
    bool parsed = false;

    // check if this is name or number
    if(color[0] == L'#')
    {
        // convert
        unsigned long num = wcstoul(color + 1, 0, 16);
        if(num != 0 && num != ULONG_MAX)
        {
            parsed = true;
            parsedColor = num;
        }

    } else
    {
        // find name
        for(int idx = 0; idx < XRICHTEXT_COLOR_NAME_COUNT; ++idx)
        {
            // match name
            if(::_wcsicmp(color, _XRichTextParseColorNames[idx].name) == 0)
            {
                // found
                parsed = true;
                parsedColor = _XRichTextParseColorNames[idx].color;

                // stop
                break;
            }
        }
    }

    // set color if parsed
    if(parsed)
    {
        m_textColor = parsedColor;
        m_hasTextColor = true;

    } else
    {
        XWTRACE1("XRichTextParser: unknown color %S ignored", color);
    }
}

void XRichTextParser::_pushFontTag()
{
    // copy current font state
    ParserFontInfo finfo;
    finfo.face = m_activeStyle.strFontName;
    finfo.size = m_activeStyle.nFontSize;
    finfo.textColor = m_textColor;
    finfo.hasColor = m_hasTextColor;

    // add to stack
    m_fontStack.push_back(finfo);

    // process font attribtues
    for(_ParserTagAttributes::const_iterator it = m_tagAttributes.begin();
        it != m_tagAttributes.end(); ++it)
    {
        // check attribute
        switch(it->first)
        {
        case eParseAttributeFace:
            // set new font face
            if(it->second.length())
                m_activeStyle.strFontName = it->second;
            break;

        case eParseAttributeSize:
            // set new font size
            if(it->second.length())
            {
                int size = _wtoi(it->second.c_str());
                if(size > 0)
                    m_activeStyle.nFontSize = size;
            }
            break;

        case eParseAttributeColor:
            // set color
            if(it->second.length())
                _setFontColor(it->second.c_str());
            break;
        }
    }
}

void XRichTextParser::_popFontTag()
{
    // ignore if stack is empty
    if(m_fontStack.size() == 0) return;

    // restore state
    m_activeStyle.strFontName = m_fontStack.back().face;
    m_activeStyle.nFontSize = m_fontStack.back().size;
    m_textColor = m_fontStack.back().textColor;
    m_hasTextColor = m_fontStack.back().hasColor;

    // pop stack item
    m_fontStack.pop_back();
}

void XRichTextParser::_processImageTag()
{
    const wchar_t* imageUri;
    int width = 0;
    int height = 0;

    // process attribtues
    for(_ParserTagAttributes::const_iterator it = m_tagAttributes.begin();
        it != m_tagAttributes.end(); ++it)
    {
        // check attribute
        switch(it->first)
        {
        case eParseAttributeSource:
            // image source
            if(it->second.length())
                imageUri = it->second.c_str();
            break;

        case eParseAttributeWidth:
            // width
            if(it->second.length())
                width = _wtoi(it->second.c_str());
            break;

        case eParseAttributeHeight:
            // height
            if(it->second.length())
                height = _wtoi(it->second.c_str());
            break;
        }
    }

    // report
    if(imageUri)
        m_parserObserver->onRichTextParserImage(imageUri, width, height);
}

// XRichTextParser
/////////////////////////////////////////////////////////////////////
