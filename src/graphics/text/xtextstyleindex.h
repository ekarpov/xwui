// Text style indexing helper class
//
/////////////////////////////////////////////////////////////////////

#ifndef _XTEXTSTYLEINDEX_H_
#define _XTEXTSTYLEINDEX_H_

/////////////////////////////////////////////////////////////////////
// Style index format:
//  1 bit: marks if index represents something else
//  7 bits: font index in fonts array (128 max)
//  1 byte: font size (256 max)
//  1 byte: text and background colors (128 for text and 128 for background)
//  2 bits: color flags
//  1 bit: 
//  1 bit: text direction (RTL if set, LTR otherwise)
//  4 bits: bold, underline, strike and italic flags 

// masks
#define XTEXTSTYLE_FONT_MASK                0x7F000000
#define XTEXTSTYLE_FONTSIZE_MASK            0x00FF0000
#define XTEXTSTYLE_TEXT_COLOR_MASK          0x00000F00
#define XTEXTSTYLE_BG_COLOR_MASK            0x0000F000
#define XTEXTSTYLE_FLAGS_MASK               0x000000FF

// masks
#define XTEXTSTYLE_STYLEONLY_MASK           0xFFFF003F
#define XTEXTSTYLE_COLORSONLY_MASK          0x0000FFC0

// flags
#define XTEXTSTYLE_BOLDFACE_FLAG            0x01
#define XTEXTSTYLE_ITALICFACE_FLAG          0x02
#define XTEXTSTYLE_UNDERLINE_FLAG           0x04
#define XTEXTSTYLE_STRIKE_FLAG              0x08
#define XTEXTSTYLE_RTL_DIRECTION_FLAG       0x10
//#define XTEXTSTYLE__FLAG            0x20
#define XTEXTSTYLE_TEXT_COLOR_FLAG          0x40
#define XTEXTSTYLE_BG_COLOR_FLAG            0x80

// max sizes
#define XTEXTSTYLE_FONT_MAXINDEX            128
#define XTEXTSTYLE_FONT_MAXSIZE             256
#define XTEXTSTYLE_TEXT_COLOR_MAXINDEX      128
#define XTEXTSTYLE_BG_COLOR_MAXINDEX        128

// not valid index
#define XTEXTSTYLE_NOT_AN_INDEX_MASK        0x80000000

// default style
#define XTEXTSTYLE_DEFAULT_INDEX            0x00000000

// style index
typedef unsigned long   xstyle_index_t;

/////////////////////////////////////////////////////////////////////
// style index comparison (excluding colors and inline object flag)
inline bool XTEXTSTYLE_SAME_STYLE(xstyle_index_t styleLeft, xstyle_index_t styleRight)
{
    return (styleLeft & XTEXTSTYLE_STYLEONLY_MASK) == (styleRight & XTEXTSTYLE_STYLEONLY_MASK);
}

/////////////////////////////////////////////////////////////////////
// XTextStyleIndex - text style information storage

class XTextStyleIndex
{
public: // construction/destruction
    XTextStyleIndex();
    ~XTextStyleIndex();

public: // text style
    xstyle_index_t  indexFromStyle(const XTextStyle& style);
    XTextStyle      styleFromIndex(xstyle_index_t styleIndex) const;

public: // reset index data
    void            reset();

public: // style index fields
    std::wstring    getFont(xstyle_index_t styleIndex) const;
    int             getFontSize(xstyle_index_t styleIndex) const;
    bool            isBold(xstyle_index_t styleIndex) const;
    bool            isItalic(xstyle_index_t styleIndex) const;
    bool            isUnderline(xstyle_index_t styleIndex) const;
    bool            isStrike(xstyle_index_t styleIndex) const;
    bool            isRTL(xstyle_index_t styleIndex) const;
    bool            isTextColorSet(xstyle_index_t styleIndex) const;
    bool            isTextBackgroundSet(xstyle_index_t styleIndex) const;
    COLORREF        getTextColor(xstyle_index_t styleIndex) const;
    COLORREF        getTextBackground(xstyle_index_t styleIndex) const;

public: // modify style index 
    xstyle_index_t  setFont(xstyle_index_t styleIndex, const std::wstring& strFontName);
    xstyle_index_t  setFontSize(xstyle_index_t styleIndex, int nFontSize);
    xstyle_index_t  setBold(xstyle_index_t styleIndex, bool bBold);
    xstyle_index_t  setItalic(xstyle_index_t styleIndex, bool bItalic);
    xstyle_index_t  setUnderline(xstyle_index_t styleIndex, bool bUnderline);
    xstyle_index_t  setStrike(xstyle_index_t styleIndex, bool bStrike);
    xstyle_index_t  setRTL(xstyle_index_t styleIndex, bool bRTL);
    xstyle_index_t  setTextColor(xstyle_index_t styleIndex, COLORREF clColor);
    xstyle_index_t  setTextBackground(xstyle_index_t styleIndex, COLORREF clColor);
    xstyle_index_t  clearTextColor(xstyle_index_t styleIndex);
    xstyle_index_t  clearTextBackground(xstyle_index_t styleIndex);

private: // index data
    std::vector<std::wstring>   m_vFontIndex;
    std::vector<COLORREF>       m_vTextColors;
    std::vector<COLORREF>       m_vBackgroundColors;
};

// XTextStyleIndex
/////////////////////////////////////////////////////////////////////

#endif // _XTEXTSTYLEINDEX_H_

