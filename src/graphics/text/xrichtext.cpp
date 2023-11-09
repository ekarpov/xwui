// Formatted text storage
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"

#include "xtextinlineobject.h"
#include "xrichtext.h"

/////////////////////////////////////////////////////////////////////
// XRichText - formatted text storage

XRichText::XRichText(IXRichTextObserver* observerRef) :
    m_observerRef(observerRef)
{
}

XRichText::~XRichText()
{
    // reset observer if any
    m_observerRef = 0;

    // reset content
    reset();
}

/////////////////////////////////////////////////////////////////////
// text changes observer
/////////////////////////////////////////////////////////////////////
void XRichText::addObserver(IXRichTextObserver* observerRef)
{
    // TODO: observer array
    m_observerRef = observerRef;
}

void XRichText::removeObserver(IXRichTextObserver* observerRef)
{
    // TODO: observer array
    if(m_observerRef == observerRef) 
    {
        m_observerRef = 0;
    }
}

/////////////////////////////////////////////////////////////////////
// reset
/////////////////////////////////////////////////////////////////////
void XRichText::reset()
{
    // reset text
    m_text.clear();
    m_styles.clear();
    m_styleIndex.reset();

    // reset inline objects
    for(unsigned int idx = 0; idx < m_inlineObjects.size(); ++idx)
    {
        // clear
        m_inlineObjects.at(idx).object->Release();
    }

    // reset array
    m_inlineObjects.clear();

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextModified();
}

/////////////////////////////////////////////////////////////////////
// text 
/////////////////////////////////////////////////////////////////////
void XRichText::setText(const wchar_t* text)
{
    // check
    XWASSERT(text);
    if(text == 0) return;

    // reset previous text
    reset();

    // append characters with default style
    while(*text)
    {
        // append char with default style
        m_text.push_back(*text);
        m_styles.push_back(XTEXTSTYLE_DEFAULT_INDEX);

        // next
        ++text;
    }

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextModified();
}

void XRichText::getText(std::wstring& text, const XTextRange& range) const
{
    XWASSERT(m_text.size() == m_styles.size());

    // validate text range
    if(!_validateRange(range)) return;

    // append text
    text.append(m_text.begin() + range.pos, m_text.begin() + range.pos + range.length);
}

/////////////////////////////////////////////////////////////////////
// internal text data
/////////////////////////////////////////////////////////////////////
const wchar_t* XRichText::data() const
{
    return m_text.data();
}

wchar_t* XRichText::data()
{
    return m_text.data();
}

/////////////////////////////////////////////////////////////////////
// edit text
/////////////////////////////////////////////////////////////////////
void XRichText::insertText(int textPos, const wchar_t* text, const XTextStyle* style, const COLORREF* textColor)
{
    XWASSERT(m_text.size() == m_styles.size());
    XWASSERT(textPos >= 0 && textPos <= (int)m_text.size());
    XWASSERT(text);

    // check input
    if(text == 0) return;

    // use last position if position is out of bounds
    if(textPos < 0 || textPos > (int)m_text.size()) 
    {
        textPos = (int) m_text.size();
    }

    xstyle_index_t styleIndex = XTEXTSTYLE_DEFAULT_INDEX;

    // use provided style if set
    if(style)
    {
        // add to index
        styleIndex = m_styleIndex.indexFromStyle(*style);
    }

    // set text color if needed
    if(textColor)
    {
        // add to style index
        styleIndex = m_styleIndex.setTextColor(styleIndex, *textColor);
    }

    // text size
    size_t textLen = ::wcslen(text);

    // fill styles
    std::vector<xstyle_index_t> insertStyles;
    insertStyles.resize(textLen);
    std::fill(insertStyles.begin(), insertStyles.end(), styleIndex);

    // insert data
    m_text.insert(m_text.begin() + textPos, text, text + textLen);
    m_styles.insert(m_styles.begin() + textPos, insertStyles.begin(), insertStyles.end());

    XWASSERT(m_text.size() == m_styles.size());

    // modified range
    XTextRange insertRange(textPos, (int)textLen);

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextAdded(insertRange);
}

void XRichText::appendText(const wchar_t* text, const XTextStyle* style, const COLORREF* textColor)
{
    // append text
    insertText((int)m_text.size(), text, style, textColor);
}

void XRichText::deleteText(const XTextRange& range)
{
    // validate text range
    if(!_validateRange(range)) return;

    // erase ranges
    m_text.erase(m_text.begin() + range.pos, m_text.begin() + range.pos + range.length);
    m_styles.erase(m_styles.begin() + range.pos, m_styles.begin() + range.pos + range.length);

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextRemoved(range);
}

wchar_t XRichText::charAt(int textPos) const
{
    // validate text position
    if(!_validateTextPos(textPos)) return 0; 

    // return character
    return m_text.at(textPos);
}

/////////////////////////////////////////////////////////////////////
// text runs (parts of text with the same style)
/////////////////////////////////////////////////////////////////////
int XRichText::getTextRun(int textPos, XTextStyle& textStyle, bool& hasInlineObject, int maxLength)  const
{
    // validate text position
    if(!_validateTextPos(textPos)) return (int) m_styles.size(); 

    // check for inline object
    hasInlineObject = ((m_styles.at(textPos) & XTEXTSTYLE_NOT_AN_INDEX_MASK) != 0);
    if(hasInlineObject)
    {
        // init style
        textStyle = m_styleIndex.styleFromIndex(_inlineObjectStyle(textPos));

        return textPos + 1;
    }

    // ignore maximum if not set
    if(maxLength <= 0) maxLength = (int) m_styles.size();

    // active style index
    xstyle_index_t styleIndex = m_styles.at(textPos);

    // reset colors
    styleIndex &= XTEXTSTYLE_STYLEONLY_MASK;

    // init style
    textStyle = m_styleIndex.styleFromIndex(styleIndex);

    // count characters with the same style (ignore colors)
    while(textPos < (int)m_styles.size() && styleIndex == (m_styles.at(textPos) & XTEXTSTYLE_STYLEONLY_MASK))
    {
        // next
        ++textPos;

        // ignore at maximum length
        if(--maxLength <= 0) break;
    }

    return textPos;
}

/////////////////////////////////////////////////////////////////////
// color runs (parts of text with the same colors)
/////////////////////////////////////////////////////////////////////
int XRichText::getColorRun(int textPos, int maxLength) const
{
    // validate text position
    if(!_validateTextPos(textPos)) return (int) m_styles.size(); 

    // ignore maximum if not set
    if(maxLength <= 0) maxLength = (int) m_styles.size();

    // active style index
    xstyle_index_t styleIndex = m_styles.at(textPos);

    // reset style
    styleIndex &= XTEXTSTYLE_COLORSONLY_MASK;

    // count characters with the same colors (ignore style)
    while(textPos < (int)m_styles.size() && styleIndex == (m_styles.at(textPos) & XTEXTSTYLE_COLORSONLY_MASK))
    {
        // next
        ++textPos;

        // ignore at maximum length
        if(--maxLength <= 0) break;
    }

    return textPos;
}

bool XRichText::sameColors(int textPos1, int textPos2) const
{
    // validate text position
    if(!_validateTextPos(textPos1) || !_validateTextPos(textPos2)) return false; 

    // check if colors are the same
    return ( (m_styles.at(textPos1) & XTEXTSTYLE_COLORSONLY_MASK) ==
            (m_styles.at(textPos2) & XTEXTSTYLE_COLORSONLY_MASK) );
}

/////////////////////////////////////////////////////////////////////
// text properties
/////////////////////////////////////////////////////////////////////
XTextRange XRichText::totalRange() const
{
    XWASSERT(m_text.size() == m_styles.size());

    return XTextRange(0, (int)m_text.size());
}

unsigned int XRichText::textLength() const
{
    XWASSERT(m_text.size() == m_styles.size());

    return (int)m_text.size();
}

void XRichText::splitToLines(std::vector<XTextRange>& textLines)
{
    XTextRange range(0, 0);

    // loop over all text
    for(unsigned int idx = 0; idx < m_text.size(); ++idx)
    {
        // current char 
        wchar_t ch = m_text.at(idx);

        // NOTE: https://developer.apple.com/library/mac/#documentation/Cocoa/Conceptual/Strings/Articles/stringsParagraphBreaks.html
        // paragraph separator U+2029
        // line separator U+2028

        range.length++;

        // check for end of line
        if(ch == L'\n' || ch == 0x2029 || ch == 0x2028)
        {
            // append range
            textLines.push_back(range);

            // next range
            range.pos = idx + 1;
            range.length = 0;
        }
    }

    // append last range
    if(range.length > 0)
    {
        textLines.push_back(range);
    }
}

/////////////////////////////////////////////////////////////////////
// inline objects
/////////////////////////////////////////////////////////////////////
bool XRichText::hasInlineObjects() const
{
    return (m_inlineObjects.size() > 0);
}

void XRichText::appendInlineObject(XTextInlineObject* inlineObject, const XTextStyle* style)
{
    // append inline object
    insertInlineObject((int)m_text.size(), inlineObject, style);
}

void XRichText::insertInlineObject(int textPos, XTextInlineObject* inlineObject, const XTextStyle* style)
{
    XWASSERT(textPos >= 0 && textPos <= (int)m_text.size());
    XWASSERT(inlineObject);

    // check input
    if(textPos < 0 || textPos > (int)m_text.size() || inlineObject == 0) return;

    // add extra reference
    inlineObject->AddRef();

    InlineObject objectRef;
    objectRef.object = inlineObject;
    if(style)
        objectRef.style = m_styleIndex.indexFromStyle(*style);
    else
        objectRef.style = XTEXTSTYLE_DEFAULT_INDEX;

    // add inline object
    m_inlineObjects.push_back(objectRef);

    // use space in place of inline object in text
    m_text.insert(m_text.begin() + textPos, L' ');

    // use style index as reference to inline object array
    xstyle_index_t index = (xstyle_index_t)m_inlineObjects.size() - 1;

    // mark index as not valid
    index |= XTEXTSTYLE_NOT_AN_INDEX_MASK;

    // append to styles
    m_styles.insert(m_styles.begin() + textPos, index);

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextAdded(XTextRange(textPos, 1));
}

bool XRichText::isInlineObjectAt(int textPos) const
{
    // validate text position
    if(!_validateTextPos(textPos)) return false; 

    // check style
    return ((m_styles.at(textPos) & XTEXTSTYLE_NOT_AN_INDEX_MASK) != 0);
}

XTextInlineObject* XRichText::inlineObjectAt(int textPos) const
{
    // validate text position
    if(!_validateTextPos(textPos)) return 0; 

    // get style index
    xstyle_index_t index = m_styles.at(textPos);

    // check style index
    if(!(index & XTEXTSTYLE_NOT_AN_INDEX_MASK)) 
    {
        XWASSERT(false);
        return 0;
    }
    
    // get offset from style index
    index &= ~(XTEXTSTYLE_NOT_AN_INDEX_MASK);

    // get inline object
    if(index < m_inlineObjects.size())
        return m_inlineObjects.at(index).object;

    XWASSERT(false);
    return 0;
}

/////////////////////////////////////////////////////////////////////
// enum inline objects
/////////////////////////////////////////////////////////////////////
void XRichText::getInlineObjects(std::vector<XTextInlineObject*>& objectsOut)
{
    // copy all objects
    for(std::vector<InlineObject>::const_iterator it = m_inlineObjects.begin();
        it != m_inlineObjects.end(); ++it)
    {
        objectsOut.push_back(it->object);
    }
}

size_t XRichText::inlineObjectCount() const
{
    return m_inlineObjects.size();
}

XTextInlineObject* XRichText::getInlineObject(size_t index) const
{
    if(index < m_inlineObjects.size())
        return m_inlineObjects.at(index).object;

    XWASSERT(false);
    return 0;
}

/////////////////////////////////////////////////////////////////////
// style ranges
/////////////////////////////////////////////////////////////////////
void XRichText::getTextStyles(const XTextRange& range, std::vector<xstyle_index_t>& styles) const
{
    // validate text range
    if(!_validateRange(range)) return;

    // loop over text range
    for(unsigned int idx = range.pos; idx < m_styles.size() && idx < range.pos + range.length; ++idx)
    {
        // check for inline object
        if(!(m_styles.at(idx) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
        {
            // copy style
            styles.push_back(m_styles.at(idx));

        } else
        {
            // copy style
            styles.push_back(_inlineObjectStyle(idx));
        }
    }

}

void XRichText::applyTextStyles(const XTextRange& range, const std::vector<xstyle_index_t>& styles)
{
    // validate text range
    if(!_validateRange(range) || styles.size() < range.length) return;

    // loop over text range
    for(unsigned int idx = range.pos; idx < m_styles.size() && idx < range.pos + range.length; ++idx)
    {
        // check for inline object
        if(!(m_styles.at(idx) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
        {
            // copy style
            m_styles.at(idx) = styles.at(idx - range.pos);

        } else
        {
            _setInlineObjectStyle(idx, styles.at(idx - range.pos));
        }
    }

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextStyleChanged(range);
}

void XRichText::applyTextStyleMask(const XTextRange& range, xstyle_index_t style, xstyle_index_t styleMask)
{
    // validate text range
    if(!_validateRange(range)) return;

    // loop over text range
    for(unsigned int idx = range.pos; idx < m_styles.size() && idx < range.pos + range.length; ++idx)
    {
        // check for inline object
        if(!(m_styles.at(idx) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
        {
            // update only style bits marked by mask
            m_styles.at(idx) = (m_styles.at(idx) & (~styleMask)) | (style & styleMask);

        } else
        {
            // current style
            xstyle_index_t styleIndex = _inlineObjectStyle(idx);

            // update only style bits marked by mask
            _setInlineObjectStyle(idx, (styleIndex & (~styleMask)) | (style & styleMask));
        }
    }

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextStyleChanged(range);
}

xstyle_index_t XRichText::styleIndexAt(int textPos) const
{
    // validate text position
    if(!_validateTextPos(textPos)) return XTEXTSTYLE_DEFAULT_INDEX;

    // check for inline object
    if(!(m_styles.at(textPos) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
    {
        // return style only index
        return (m_styles.at(textPos) & XTEXTSTYLE_STYLEONLY_MASK);

    } else
    {
        // return style only index
        return _inlineObjectStyle(textPos) & XTEXTSTYLE_STYLEONLY_MASK;
    }
}

/////////////////////////////////////////////////////////////////////
// set text style properties
/////////////////////////////////////////////////////////////////////
void XRichText::setTextStyle(const XTextStyle& style, const XTextRange& range)
{
    // validate text range
    if(!_validateRange(range)) return;

    // add style to index
    xstyle_index_t styleIndex = m_styleIndex.indexFromStyle(style);

    // loop over text range
    for(unsigned int idx = range.pos; idx < m_styles.size() && idx < range.pos + range.length; ++idx)
    {
        // check for inline object
        if(!(m_styles.at(idx) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
        {
            // set new style
            m_styles.at(idx) = styleIndex;

        } else
        {
            _setInlineObjectStyle(idx, styleIndex);
        }
    }

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextStyleChanged(range);
}

void XRichText::setFont(const std::wstring& fontName, const XTextRange& range)
{
    // validate text range
    if(!_validateRange(range)) return;

    // loop over text range
    for(unsigned int idx = range.pos; idx < m_styles.size() && idx < range.pos + range.length; ++idx)
    {
        // check for inline object
        if(!(m_styles.at(idx) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
        {
            // update style
            m_styles.at(idx) = m_styleIndex.setFont(m_styles.at(idx), fontName);

        } else
        {
            // update style
            _setInlineObjectStyle(idx, m_styleIndex.setFont(_inlineObjectStyle(idx), fontName));
        }
    }

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextStyleChanged(range);
}

void XRichText::setFontSize(int fontSize, const XTextRange& range)
{
    // validate text range
    if(!_validateRange(range)) return;

    // loop over text range
    for(unsigned int idx = range.pos; idx < m_styles.size() && idx < range.pos + range.length; ++idx)
    {
        // check for inline object
        if(!(m_styles.at(idx) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
        {
            // update style
            m_styles.at(idx) = m_styleIndex.setFontSize(m_styles.at(idx), fontSize);

        } else
        {
            // update style
            _setInlineObjectStyle(idx, m_styleIndex.setFontSize(_inlineObjectStyle(idx), fontSize));
        }
    }

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextStyleChanged(range);
}

void XRichText::setBold(bool bold, const XTextRange& range)
{
    // validate text range
    if(!_validateRange(range)) return;

    // loop over text range
    for(unsigned int idx = range.pos; idx < m_styles.size() && idx < range.pos + range.length; ++idx)
    {
        // check for inline object
        if(!(m_styles.at(idx) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
        {
            // update style
            m_styles.at(idx) = m_styleIndex.setBold(m_styles.at(idx), bold);

        } else
        {
            // update style
            _setInlineObjectStyle(idx, m_styleIndex.setBold(_inlineObjectStyle(idx), bold));
        }
    }

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextStyleChanged(range);
}

void XRichText::setItalic(bool italic, const XTextRange& range)
{
    // validate text range
    if(!_validateRange(range)) return;

    // loop over text range
    for(unsigned int idx = range.pos; idx < m_styles.size() && idx < range.pos + range.length; ++idx)
    {
        // check for inline object
        if(!(m_styles.at(idx) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
        {
            // update style
            m_styles.at(idx) = m_styleIndex.setItalic(m_styles.at(idx), italic);

        } else
        {
            // update style
            _setInlineObjectStyle(idx, m_styleIndex.setItalic(_inlineObjectStyle(idx), italic));
        }
    }

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextStyleChanged(range);
}

void XRichText::setUnderline(bool underline, const XTextRange& range)
{
    // validate text range
    if(!_validateRange(range)) return;

    // loop over text range
    for(unsigned int idx = range.pos; idx < m_styles.size() && idx < range.pos + range.length; ++idx)
    {
        // check for inline object
        if(!(m_styles.at(idx) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
        {
            // update style
            m_styles.at(idx) = m_styleIndex.setUnderline(m_styles.at(idx), underline);

        } else
        {
            // update style
            _setInlineObjectStyle(idx, m_styleIndex.setUnderline(_inlineObjectStyle(idx), underline));
        }
    }

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextStyleChanged(range);
}

void XRichText::setStrike(bool strike, const XTextRange& range)
{
    // validate text range
    if(!_validateRange(range)) return;

    // loop over text range
    for(unsigned int idx = range.pos; idx < m_styles.size() && idx < range.pos + range.length; ++idx)
    {
        // check for inline object
        if(!(m_styles.at(idx) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
        {
            // update style
            m_styles.at(idx) = m_styleIndex.setStrike(m_styles.at(idx), strike);

        } else
        {
            // update style
            _setInlineObjectStyle(idx, m_styleIndex.setStrike(_inlineObjectStyle(idx), strike));
        }
    }

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextStyleChanged(range);
}

void XRichText::setRTL(bool rtldir, const XTextRange& range)
{
    // validate text range
    if(!_validateRange(range)) return;

    // loop over text range
    for(unsigned int idx = range.pos; idx < m_styles.size() && idx < range.pos + range.length; ++idx)
    {
        // check for inline object
        if(!(m_styles.at(idx) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
        {
            // update style
            m_styles.at(idx) = m_styleIndex.setRTL(m_styles.at(idx), rtldir);

        } else
        {
            // update style
            _setInlineObjectStyle(idx, m_styleIndex.setRTL(_inlineObjectStyle(idx), rtldir));
        }
    }

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextStyleChanged(range);
}

void XRichText::setTextColor(COLORREF textColor, const XTextRange& range)
{
    // validate text range
    if(!_validateRange(range)) return;

    // loop over text range
    for(unsigned int idx = range.pos; idx < m_styles.size() && idx < range.pos + range.length; ++idx)
    {
        // check for inline object
        if(!(m_styles.at(idx) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
        {
            // update style
            m_styles.at(idx) = m_styleIndex.setTextColor(m_styles.at(idx), textColor);

        } else
        {
            // update style
            _setInlineObjectStyle(idx, m_styleIndex.setTextColor(_inlineObjectStyle(idx), textColor));
        }
    }

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextColorChanged(range);
}

void XRichText::setBackgroundColor(COLORREF backgroundColor, const XTextRange& range)
{
    // validate text range
    if(!_validateRange(range)) return;

    // loop over text range
    for(unsigned int idx = range.pos; idx < m_styles.size() && idx < range.pos + range.length; ++idx)
    {
        // check for inline object
        if(!(m_styles.at(idx) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
        {
            // update style
            m_styles.at(idx) = m_styleIndex.setTextBackground(m_styles.at(idx), backgroundColor);

        } else
        {
            // update style
            _setInlineObjectStyle(idx, m_styleIndex.setTextBackground(_inlineObjectStyle(idx), backgroundColor));
        }
    }

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextColorChanged(range);
}

void XRichText::clearTextColor(const XTextRange& range)
{
    // validate text range
    if(!_validateRange(range)) return;

    // loop over text range
    for(unsigned int idx = range.pos; idx < m_styles.size() && idx < range.pos + range.length; ++idx)
    {
        // check for inline object
        if(!(m_styles.at(idx) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
        {
            // update style
            m_styles.at(idx) = m_styleIndex.clearTextColor(m_styles.at(idx));

        } else
        {
            // update style
            _setInlineObjectStyle(idx, m_styleIndex.clearTextColor(_inlineObjectStyle(idx)));
        }
    }

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextColorChanged(range);
}

void XRichText::clearBackgroundColor(const XTextRange& range)
{
    // validate text range
    if(!_validateRange(range)) return;

    // loop over text range
    for(unsigned int idx = range.pos; idx < m_styles.size() && idx < range.pos + range.length; ++idx)
    {
        // check for inline object
        if(!(m_styles.at(idx) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
        {
            // update style
            m_styles.at(idx) = m_styleIndex.clearTextBackground(m_styles.at(idx));

        } else
        {
            // update style
            _setInlineObjectStyle(idx, m_styleIndex.clearTextBackground(_inlineObjectStyle(idx)));
        }
    }

    // inform observer
    if(m_observerRef) m_observerRef->onRichTextColorChanged(range);
}

/////////////////////////////////////////////////////////////////////
// set text style properties (for whole range)
/////////////////////////////////////////////////////////////////////
void XRichText::setTextStyle(const XTextStyle& style)
{
    // set style for whole text
    setTextStyle(style, totalRange());
}

void XRichText::setFont(const std::wstring& fontName)
{
    // set for whole text
    setFont(fontName, totalRange());
}

void XRichText::setTextColor(COLORREF textColor)
{
    // set for whole text
    setTextColor(textColor, totalRange());
}

/////////////////////////////////////////////////////////////////////
// get text style properties
/////////////////////////////////////////////////////////////////////
XTextStyle XRichText::textStyle(int textPos, XTextRange* range) const
{
    // validate text position
    if(!_validateTextPos(textPos)) return XTextStyle();

    // check for inline object
    if(m_styles.at(textPos) & XTEXTSTYLE_NOT_AN_INDEX_MASK) 
    {
        // init range
        if(range)
        {
            range->pos = textPos;
            range->length = 1;
        }

        // return style
        return m_styleIndex.styleFromIndex(_inlineObjectStyle(textPos));
    }

    // get style from index
    XTextStyle style = m_styleIndex.styleFromIndex(m_styles.at(textPos));

    // fill range if needed
    if(range)
    {
        // range start
        range->pos = textPos;

        // active style index
        xstyle_index_t styleIndex = m_styles.at(textPos);

        // reset colors
        styleIndex &= XTEXTSTYLE_STYLEONLY_MASK;

        // range length
        for(range->length = 1; range->pos + range->length < m_styles.size(); range->length++)
        {
            // check style (ignore colors)
            if(styleIndex != (m_styles.at(range->pos + range->length) & XTEXTSTYLE_STYLEONLY_MASK))
            {
                // stop
                break;
            }
        }
    }

    return style;
}

std::wstring XRichText::fontName(int textPos, XTextRange* range) const
{
    // validate text position
    if(!_validateTextPos(textPos)) return L"";

    // check for inline object
    if(m_styles.at(textPos) & XTEXTSTYLE_NOT_AN_INDEX_MASK) 
    {
        // init range
        if(range)
        {
            range->pos = textPos;
            range->length = 1;
        }

        return m_styleIndex.getFont(_inlineObjectStyle(textPos));
    }

    // get font from index
    std::wstring fontName = m_styleIndex.getFont(m_styles.at(textPos));

    // fill range if needed
    if(range)
    {
        // range start
        range->pos = textPos;

        // range length
        for(range->length = 1; range->pos + range->length < m_styles.size(); range->length++)
        {
            // check style 
            if(fontName != m_styleIndex.getFont(m_styles.at(range->pos + range->length)))
            {
                // stop
                break;
            }
        }
    }

    return fontName;
}

int XRichText::fontSize(int textPos, XTextRange* range) const
{
    // validate text position
    if(!_validateTextPos(textPos)) return 0; 

    // check for inline object
    if(m_styles.at(textPos) & XTEXTSTYLE_NOT_AN_INDEX_MASK)
    {
        // init range
        if(range)
        {
            range->pos = textPos;
            range->length = 1;
        }

        return m_styleIndex.getFontSize(_inlineObjectStyle(textPos));
    }

    // get size from index
    int fontSize = m_styleIndex.getFontSize(m_styles.at(textPos));

    // fill range if needed
    if(range)
    {
        // range start
        range->pos = textPos;

        // range length
        for(range->length = 1; range->pos + range->length < m_styles.size(); range->length++)
        {
            // check style 
            if(fontSize != m_styleIndex.getFontSize(m_styles.at(range->pos + range->length)))
            {
                // stop
                break;
            }
        }
    }

    return fontSize;
}

bool XRichText::isBold(int textPos, XTextRange* range) const
{
    // validate text position
    if(!_validateTextPos(textPos)) return false; 

    // check for inline object
    if(m_styles.at(textPos) & XTEXTSTYLE_NOT_AN_INDEX_MASK)
    {
        // init range
        if(range)
        {
            range->pos = textPos;
            range->length = 1;
        }

        return m_styleIndex.isBold(_inlineObjectStyle(textPos));
    }

    // get flag from index
    bool result = m_styleIndex.isBold(m_styles.at(textPos));

    // fill range if needed
    if(range)
    {
        // range start
        range->pos = textPos;

        // range length
        for(range->length = 1; range->pos + range->length < m_styles.size(); range->length++)
        {
            // check style 
            if(result != m_styleIndex.isBold(m_styles.at(range->pos + range->length)))
            {
                // stop
                break;
            }
        }
    }

    return result;
}

bool XRichText::isItalic(int textPos, XTextRange* range) const
{
    // validate text position
    if(!_validateTextPos(textPos)) return false; 

    // check for inline object
    if(m_styles.at(textPos) & XTEXTSTYLE_NOT_AN_INDEX_MASK)
    {
        // init range
        if(range)
        {
            range->pos = textPos;
            range->length = 1;
        }

        return m_styleIndex.isItalic(_inlineObjectStyle(textPos));
    }

    // get flag from index
    bool result = m_styleIndex.isItalic(m_styles.at(textPos));

    // fill range if needed
    if(range)
    {
        // range start
        range->pos = textPos;

        // range length
        for(range->length = 1; range->pos + range->length < m_styles.size(); range->length++)
        {
            // check style 
            if(result != m_styleIndex.isItalic(m_styles.at(range->pos + range->length)))
            {
                // stop
                break;
            }
        }
    }

    return result;
}

bool XRichText::isUnderline(int textPos, XTextRange* range) const
{
    // validate text position
    if(!_validateTextPos(textPos)) return false; 

    // check for inline object
    if(m_styles.at(textPos) & XTEXTSTYLE_NOT_AN_INDEX_MASK)
    {
        // init range
        if(range)
        {
            range->pos = textPos;
            range->length = 1;
        }

        return m_styleIndex.isUnderline(_inlineObjectStyle(textPos));
    }

    // get flag from index
    bool result = m_styleIndex.isUnderline(m_styles.at(textPos));

    // fill range if needed
    if(range)
    {
        // range start
        range->pos = textPos;

        // range length
        for(range->length = 1; range->pos + range->length < m_styles.size(); range->length++)
        {
            // check style 
            if(result != m_styleIndex.isUnderline(m_styles.at(range->pos + range->length)))
            {
                // stop
                break;
            }
        }
    }

    return result;
}

bool XRichText::isStrike(int textPos, XTextRange* range) const
{
    // validate text position
    if(!_validateTextPos(textPos)) return false; 

    // check for inline object
    if(m_styles.at(textPos) & XTEXTSTYLE_NOT_AN_INDEX_MASK)
    {
        // init range
        if(range)
        {
            range->pos = textPos;
            range->length = 1;
        }

        return m_styleIndex.isStrike(_inlineObjectStyle(textPos));
    }

    // get flag from index
    bool result = m_styleIndex.isStrike(m_styles.at(textPos));

    // fill range if needed
    if(range)
    {
        // range start
        range->pos = textPos;

        // range length
        for(range->length = 1; range->pos + range->length < m_styles.size(); range->length++)
        {
            // check style 
            if(result != m_styleIndex.isStrike(m_styles.at(range->pos + range->length)))
            {
                // stop
                break;
            }
        }
    }

    return result;
}

bool XRichText::isRTL(int textPos, XTextRange* range) const
{
    // validate text position
    if(!_validateTextPos(textPos)) return false; 

    // check for inline object
    if(m_styles.at(textPos) & XTEXTSTYLE_NOT_AN_INDEX_MASK)
    {
        // init range
        if(range)
        {
            range->pos = textPos;
            range->length = 1;
        }

        return m_styleIndex.isRTL(_inlineObjectStyle(textPos));
    }

    // get flag from index
    bool result = m_styleIndex.isRTL(m_styles.at(textPos));

    // fill range if needed
    if(range)
    {
        // range start
        range->pos = textPos;

        // range length
        for(range->length = 1; range->pos + range->length < m_styles.size(); range->length++)
        {
            // check style 
            if(result != m_styleIndex.isRTL(m_styles.at(range->pos + range->length)))
            {
                // stop
                break;
            }
        }
    }

    return result;
}

bool XRichText::textColor(int textPos, COLORREF& textColor, XTextRange* range) const
{
    // validate text position
    if(!_validateTextPos(textPos)) return false; 

    // check for inline object
    if(m_styles.at(textPos) & XTEXTSTYLE_NOT_AN_INDEX_MASK)
    {
        // init range
        if(range)
        {
            range->pos = textPos;
            range->length = 1;
        }

        if(m_styleIndex.isTextColorSet(_inlineObjectStyle(textPos)))
        {
            textColor = m_styleIndex.getTextColor(_inlineObjectStyle(textPos));
            return true;

        } else
        {
            return false;
        }
    }

    // check if color has been set
    bool colorSet = m_styleIndex.isTextColorSet(m_styles.at(textPos));

    // get color from index
    if(colorSet)
    {
        textColor = m_styleIndex.getTextColor(m_styles.at(textPos));
    }

    // fill range if needed
    if(range)
    {
        // range start
        range->pos = textPos;

        // range length
        for(range->length = 1; range->pos + range->length < m_styles.size(); range->length++)
        {
            // color set flag
            if(colorSet  != m_styleIndex.isTextColorSet(m_styles.at(range->pos + range->length)))
            {
                // stop
                break;
            }

            // check color
            if(colorSet && textColor != m_styleIndex.getTextColor(m_styles.at(range->pos + range->length)))
            {
                // stop
                break;
            }
        }
    }

    return colorSet;
}

bool XRichText::backgroundColor(int textPos, COLORREF& backgroundColor, XTextRange* range) const
{
    // validate text position
    if(!_validateTextPos(textPos)) return false; 

    // check for inline object
    if(m_styles.at(textPos) & XTEXTSTYLE_NOT_AN_INDEX_MASK)
    {
        // init range
        if(range)
        {
            range->pos = textPos;
            range->length = 1;
        }

        if(m_styleIndex.isTextBackgroundSet(_inlineObjectStyle(textPos)))
        {
            backgroundColor = m_styleIndex.getTextBackground(_inlineObjectStyle(textPos));
            return true;

        } else
        {
            return false;
        }
    }

    // check if color has been set
    bool colorSet = m_styleIndex.isTextBackgroundSet(m_styles.at(textPos));

    // get color from index
    if(colorSet)
    {
        backgroundColor = m_styleIndex.getTextBackground(m_styles.at(textPos));
    }

    // fill range if needed
    if(range)
    {
        // range start
        range->pos = textPos;

        // range length
        for(range->length = 1; range->pos + range->length < m_styles.size(); range->length++)
        {
            // color set flag
            if(colorSet  != m_styleIndex.isTextBackgroundSet(m_styles.at(range->pos + range->length)))
            {
                // stop
                break;
            }

            // check color
            if(colorSet && backgroundColor != m_styleIndex.getTextBackground(m_styles.at(range->pos + range->length)))
            {
                // stop
                break;
            }
        }
    }

    return colorSet;
}

/////////////////////////////////////////////////////////////////////
// text style hashing (support for style caching)
/////////////////////////////////////////////////////////////////////
xstyle_index_t XRichText::hashFromTextStyle(const XTextStyle& style)
{
    return m_styleIndex.indexFromStyle(style);
}

XTextStyle XRichText::textStyleFromHash(xstyle_index_t styleHash) const
{
    return m_styleIndex.styleFromIndex(styleHash);
}

/////////////////////////////////////////////////////////////////////
// helper methods
/////////////////////////////////////////////////////////////////////
bool XRichText::_validateRange(const XTextRange& range) const
{
    XWASSERT(m_text.size() == m_styles.size());

    // check if range is valid
    if(range.pos < 0 || range.pos + range.length > m_styles.size() || range.length < 0) 
    {
        XWASSERT1(false, "Text range is out of bounds");
        return false;
    }

    // sanity check
    if(m_text.size() != m_styles.size())
    {
        XWASSERT1(false, "Inernal RichText buffer mismatch");
        return false;
    }

    return true;
}

bool XRichText::_validateTextPos(int textPos) const
{
    XWASSERT(m_text.size() == m_styles.size());

    if(textPos < 0 || textPos >= (int)m_styles.size())
    {
        XWASSERT1(false, "Text position is out of bounds");
        return false;
    }

    // sanity check
    if(m_text.size() != m_styles.size())
    {
        XWASSERT1(false, "Inernal RichText buffer mismatch");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////
// inline object styles
/////////////////////////////////////////////////////////////////////
xstyle_index_t XRichText::_inlineObjectStyle(unsigned int pos) const
{
    // validate postion
    if(pos < m_styles.size() && (m_styles.at(pos) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
    {
        // get offset from style index
        xstyle_index_t objectIndex = m_styles.at(pos) & ~(XTEXTSTYLE_NOT_AN_INDEX_MASK);

        // get style
        if(objectIndex < m_inlineObjects.size())
            return m_inlineObjects.at(objectIndex).style;
    }

    XWASSERT(false);
    return XTEXTSTYLE_DEFAULT_INDEX;
}

void XRichText::_setInlineObjectStyle(unsigned int pos, xstyle_index_t style)
{
    // validate postion
    if(pos < m_styles.size() && (m_styles.at(pos) & XTEXTSTYLE_NOT_AN_INDEX_MASK))
    {
        // get offset from style index
        xstyle_index_t objectIndex = m_styles.at(pos) & ~(XTEXTSTYLE_NOT_AN_INDEX_MASK);

        // set style
        if(objectIndex < m_inlineObjects.size())
            m_inlineObjects.at(objectIndex).style = style;
    }
}

// XRichText
/////////////////////////////////////////////////////////////////////

