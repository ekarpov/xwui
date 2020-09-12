// Text style indexing helper class
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"

#include "xtextstyleindex.h"

/////////////////////////////////////////////////////////////////////
// style index helpers
inline void _xtextstyle_flag_set(xstyle_index_t& styleIndex, unsigned long flag)
{
    // set flag
    styleIndex |= (XTEXTSTYLE_FLAGS_MASK & flag);
}

inline void _xtextstyle_flag_clear(xstyle_index_t& styleIndex, unsigned long flag)
{
    // clear flag
    styleIndex &= ~(XTEXTSTYLE_FLAGS_MASK & flag);
}

inline bool _xtextstyle_flag_isset(xstyle_index_t styleIndex, unsigned long flag)
{
    // check flag
    return ((styleIndex & (XTEXTSTYLE_FLAGS_MASK & flag)) != 0);
}

inline void _xtextstyle_flag_enable(xstyle_index_t& styleIndex, unsigned long flag, bool enable)
{
    if(enable)
        _xtextstyle_flag_set(styleIndex, flag);
    else
        _xtextstyle_flag_clear(styleIndex, flag);
}

///// index
inline void _xtextstyle_index_set(xstyle_index_t& styleIndex, unsigned long value, 
                                   unsigned long mask, unsigned long offset)
{
    styleIndex = ((~mask) & styleIndex) + (mask & (value << offset));
}

inline unsigned char _xtextstyle_index_get(xstyle_index_t styleIndex, unsigned long mask, 
                                            unsigned long offset)
{
    return (unsigned char)((styleIndex & mask) >> offset);
}

/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// XTextStyleIndex - text style information storage

XTextStyleIndex::XTextStyleIndex()
{
    // reserve zero indexes for "default" values
    m_vFontIndex.push_back(L"");
    m_vTextColors.push_back(RGB(0, 0, 0));
    m_vBackgroundColors.push_back(RGB(255, 255, 255));
}

XTextStyleIndex::~XTextStyleIndex()
{
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
xstyle_index_t XTextStyleIndex::indexFromStyle(const XTextStyle& style)
{
    xstyle_index_t styleIndex = 0;

    ///// format index
    styleIndex = setFont(styleIndex, style.strFontName);
    styleIndex = setFontSize(styleIndex, style.nFontSize);
    styleIndex = setBold(styleIndex, style.bBold);
    styleIndex = setItalic(styleIndex, style.bItalic);
    styleIndex = setUnderline(styleIndex, style.bUnderline);
    styleIndex = setStrike(styleIndex, style.bStrike);
    styleIndex = setRTL(styleIndex, style.isRTL);
    
    ///// colors
    //if(style.bTextColorSet)
    //    styleIndex = setTextColor(styleIndex, style.clTextColor);
    //else
    //    styleIndex = clearTextColor(styleIndex);

    //if(style.bTextBackgroundSet)
    //    styleIndex = setTextBackground(styleIndex, style.clTextBackground);
    //else
    //    styleIndex = clearTextBackground(styleIndex);

    // return index
    return styleIndex;
}

XTextStyle XTextStyleIndex::styleFromIndex(xstyle_index_t styleIndex) const
{
    XTextStyle style;

    // parse index
    style.strFontName = getFont(styleIndex);
    style.nFontSize = getFontSize(styleIndex);
    style.bBold = isBold(styleIndex);
    style.bItalic = isItalic(styleIndex);
    style.bUnderline = isUnderline(styleIndex);
    style.bStrike = isStrike(styleIndex);
    style.isRTL = isRTL(styleIndex);
    //style.bTextColorSet = isTextColorSet(styleIndex);
    //style.bTextBackgroundSet = isTextBackgroundSet(styleIndex);
    //style.clTextColor = getTextColor(styleIndex);
    //style.clTextBackground = getTextBackground(styleIndex);

    // return style
    return style;
}

/////////////////////////////////////////////////////////////////////
// reset index data
/////////////////////////////////////////////////////////////////////
void XTextStyleIndex::reset()
{
    // create default index
    XTextStyleIndex defaultIndex;

    // replace all properties with default
    *this = defaultIndex;
}

/////////////////////////////////////////////////////////////////////
// style index fields
/////////////////////////////////////////////////////////////////////
std::wstring XTextStyleIndex::getFont(xstyle_index_t styleIndex) const
{
    // get index
    unsigned long fontIndex = _xtextstyle_index_get(styleIndex, XTEXTSTYLE_FONT_MASK, 24);

    // validate
    if(fontIndex >= m_vFontIndex.size()) fontIndex = 0;

    // return font name
    return m_vFontIndex.at(fontIndex);
}

int XTextStyleIndex::getFontSize(xstyle_index_t styleIndex) const
{
    // get index
    unsigned long fontSize = _xtextstyle_index_get(styleIndex, XTEXTSTYLE_FONTSIZE_MASK, 16);

    // validate
    if(fontSize > XTEXTSTYLE_FONT_MAXSIZE) fontSize = 0;

    // return size
    return fontSize;
}

bool XTextStyleIndex::isBold(xstyle_index_t styleIndex) const
{
    return _xtextstyle_flag_isset(styleIndex, XTEXTSTYLE_BOLDFACE_FLAG);
}

bool XTextStyleIndex::isItalic(xstyle_index_t styleIndex) const
{
    return _xtextstyle_flag_isset(styleIndex, XTEXTSTYLE_ITALICFACE_FLAG);
}

bool XTextStyleIndex::isUnderline(xstyle_index_t styleIndex) const
{
    return _xtextstyle_flag_isset(styleIndex, XTEXTSTYLE_UNDERLINE_FLAG);
}

bool XTextStyleIndex::isStrike(xstyle_index_t styleIndex) const
{
    return _xtextstyle_flag_isset(styleIndex, XTEXTSTYLE_STRIKE_FLAG);
}

bool XTextStyleIndex::isRTL(xstyle_index_t styleIndex) const
{
    return _xtextstyle_flag_isset(styleIndex, XTEXTSTYLE_RTL_DIRECTION_FLAG);
}

bool XTextStyleIndex::isTextColorSet(xstyle_index_t styleIndex) const
{
    return _xtextstyle_flag_isset(styleIndex, XTEXTSTYLE_TEXT_COLOR_FLAG);
}

bool XTextStyleIndex::isTextBackgroundSet(xstyle_index_t styleIndex) const
{
    return _xtextstyle_flag_isset(styleIndex, XTEXTSTYLE_BG_COLOR_FLAG);
}

COLORREF XTextStyleIndex::getTextColor(xstyle_index_t styleIndex) const
{
    // check if color set
    if(!isTextColorSet(styleIndex))
    {
        // return default color
        return m_vTextColors.at(0);
    }

    // get index
    unsigned long colorIndex = _xtextstyle_index_get(styleIndex, XTEXTSTYLE_TEXT_COLOR_MASK, 8);

    // validate
    if(colorIndex >= m_vTextColors.size()) colorIndex = 0;
    
    // return color
    return m_vTextColors.at(colorIndex);
}

COLORREF XTextStyleIndex::getTextBackground(xstyle_index_t styleIndex) const
{
    // check if color set
    if(!isTextBackgroundSet(styleIndex))
    {
        // return default color
        return m_vBackgroundColors.at(0);
    }

    // get index
    unsigned long colorIndex = _xtextstyle_index_get(styleIndex, XTEXTSTYLE_BG_COLOR_MASK, 12);

    // validate
    if(colorIndex >= m_vBackgroundColors.size()) colorIndex = 0;
    
    // return color
    return m_vBackgroundColors.at(colorIndex);
}

/////////////////////////////////////////////////////////////////////
// modify style index 
/////////////////////////////////////////////////////////////////////
xstyle_index_t XTextStyleIndex::setFont(xstyle_index_t styleIndex, const std::wstring& strFontName)
{
    unsigned long fontIndex = 0;

    // check if we have this font already
    for(fontIndex = 0; fontIndex < m_vFontIndex.size(); ++fontIndex)
    {
        if(m_vFontIndex.at(fontIndex) == strFontName) break;
    }

    // add new font if not found
    if(fontIndex == m_vFontIndex.size())
    {
        // check max size first
        if(fontIndex < XTEXTSTYLE_FONT_MAXINDEX)
        {
            // add new font (NOTE: fontIndex already points to correct position)
            m_vFontIndex.push_back(strFontName);

        } else
        {
            // trace error
            XWTRACE1("XTextStyleIndex: failed to add new font, maximum number %d has been reached", XTEXTSTYLE_FONT_MAXINDEX);

            // use default font
            fontIndex = 0;
        }
    }

    // set font index
    _xtextstyle_index_set(styleIndex, fontIndex, XTEXTSTYLE_FONT_MASK, 24);

    // return new style
    return styleIndex;
}

xstyle_index_t XTextStyleIndex::setFontSize(xstyle_index_t styleIndex, int nFontSize)
{
    unsigned long fontSize = 0;
    
    // check font size
    if(nFontSize < XTEXTSTYLE_FONT_MAXSIZE && nFontSize >= 0)
    {
        // copy size
        fontSize = nFontSize;

    } else
    {
        // trace error
        XWTRACE1("XTextStyleIndex: font size %d is too big", nFontSize);

        // use default font size
        fontSize = 0;
    }

    // set font size
    _xtextstyle_index_set(styleIndex, fontSize, XTEXTSTYLE_FONTSIZE_MASK, 16);

    // return new style
    return styleIndex;
}

xstyle_index_t XTextStyleIndex::setBold(xstyle_index_t styleIndex, bool bBold)
{
    // set flag
    _xtextstyle_flag_enable(styleIndex, XTEXTSTYLE_BOLDFACE_FLAG, bBold);

    // return new value
    return styleIndex;
}

xstyle_index_t XTextStyleIndex::setItalic(xstyle_index_t styleIndex, bool bItalic)
{
    // set flag
    _xtextstyle_flag_enable(styleIndex, XTEXTSTYLE_ITALICFACE_FLAG, bItalic);

    // return new value
    return styleIndex;
}

xstyle_index_t XTextStyleIndex::setUnderline(xstyle_index_t styleIndex, bool bUnderline)
{
    // set flag
    _xtextstyle_flag_enable(styleIndex, XTEXTSTYLE_UNDERLINE_FLAG, bUnderline);

    // return new value
    return styleIndex;
}

xstyle_index_t XTextStyleIndex::setStrike(xstyle_index_t styleIndex, bool bStrike)
{
    // set flag
    _xtextstyle_flag_enable(styleIndex, XTEXTSTYLE_STRIKE_FLAG, bStrike);

    // return new value
    return styleIndex;
}

xstyle_index_t XTextStyleIndex::setRTL(xstyle_index_t styleIndex, bool bRTL)
{
    // set flag
    _xtextstyle_flag_enable(styleIndex, XTEXTSTYLE_RTL_DIRECTION_FLAG, bRTL);

    // return new value
    return styleIndex;
}

xstyle_index_t XTextStyleIndex::setTextColor(xstyle_index_t styleIndex, COLORREF clColor)
{
    unsigned long colorIndex = 0;

    // check if we have this color already
    for(colorIndex = 0; colorIndex < m_vTextColors.size(); ++colorIndex)
    {
        if(m_vTextColors.at(colorIndex) == clColor) break;
    }

    // add new color if not found
    if(colorIndex == m_vTextColors.size())
    {
        // check max size first
        if(colorIndex < XTEXTSTYLE_TEXT_COLOR_MAXINDEX)
        {
            // add new color (NOTE: colorIndex already points to correct position)
            m_vTextColors.push_back(clColor);

        } else
        {
            // trace error
            XWTRACE1("XTextStyleIndex: failed to add new text color, maximum number %d has been reached", XTEXTSTYLE_TEXT_COLOR_MAXINDEX);

            // use default color
            colorIndex = 0;
        }
    }

    // set color
    _xtextstyle_index_set(styleIndex, colorIndex, XTEXTSTYLE_TEXT_COLOR_MASK, 8);

    // set flag
    _xtextstyle_flag_enable(styleIndex, XTEXTSTYLE_TEXT_COLOR_FLAG, true);

    // return new style
    return styleIndex;
}

xstyle_index_t XTextStyleIndex::setTextBackground(xstyle_index_t styleIndex, COLORREF clColor)
{
    unsigned long colorIndex = 0;

    // check if we have this color already
    for(colorIndex = 0; colorIndex < m_vBackgroundColors.size(); ++colorIndex)
    {
        if(m_vBackgroundColors.at(colorIndex) == clColor) break;
    }

    // add new color if not found
    if(colorIndex == m_vBackgroundColors.size())
    {
        // check max size first
        if(colorIndex < XTEXTSTYLE_BG_COLOR_MAXINDEX)
        {
            // add new color (NOTE: colorIndex already points to correct position)
            m_vBackgroundColors.push_back(clColor);

        } else
        {
            // trace error
            XWTRACE1("XTextStyleIndex: failed to add new background color, maximum number %d has been reached", XTEXTSTYLE_BG_COLOR_MAXINDEX);

            // use default color
            colorIndex = 0;
        }
    }

    // set color
    _xtextstyle_index_set(styleIndex, colorIndex, XTEXTSTYLE_BG_COLOR_MASK, 12);

    // set flag
    _xtextstyle_flag_enable(styleIndex, XTEXTSTYLE_BG_COLOR_FLAG, true);

    // return new style
    return styleIndex;
}

xstyle_index_t XTextStyleIndex::clearTextColor(xstyle_index_t styleIndex)
{
    // set default color
    _xtextstyle_index_set(styleIndex, 0, XTEXTSTYLE_TEXT_COLOR_MASK, 8);

    // reset flag
    _xtextstyle_flag_enable(styleIndex, XTEXTSTYLE_TEXT_COLOR_FLAG, false);

    // return new value
    return styleIndex;
}

xstyle_index_t XTextStyleIndex::clearTextBackground(xstyle_index_t styleIndex)
{
    // set default color
    _xtextstyle_index_set(styleIndex, 0, XTEXTSTYLE_BG_COLOR_MASK, 12);

    // reset flag
    _xtextstyle_flag_enable(styleIndex, XTEXTSTYLE_BG_COLOR_FLAG, false);

    // return new value
    return styleIndex;
}

// XTextStyleIndex
/////////////////////////////////////////////////////////////////////


