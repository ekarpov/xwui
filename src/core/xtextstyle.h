// Text styles
//
/////////////////////////////////////////////////////////////////////

#ifndef _XTEXTSTYLE_H_
#define _XTEXTSTYLE_H_

/////////////////////////////////////////////////////////////////////
// text style information
struct XTextStyle
{
    // default values
    XTextStyle() : nFontSize(0), 
                    bBold(false),
                    bItalic(false),
                    bUnderline(false),
                    bStrike(false),
                    isRTL(false) {}

    std::wstring    strFontName;
    int             nFontSize;      // NOTE: size in logical units (or device independent pixels)
    bool            bBold;
    bool            bItalic;
    bool            bUnderline;
    bool            bStrike;
    bool            isRTL;
};

// text block with the same style
struct XTextRun
{
    std::wstring    text;
    XTextStyle      style;
};

// text range
struct XTextRange
{
    // constructors
    XTextRange() : pos(0), length(0) {}
    XTextRange(int _pos, int _length) : pos(_pos), length(_length) {}

    unsigned int    pos;
    unsigned int    length;
};

// horizontal text alignment
enum TTextAlignment
{
    eTextAlignLeft,
    eTextAlignCenter,
    eTextAlignRight,
    eTextAlignJustify
};

// line sapcing
enum TLineSpacing
{
    eLineSpacingNone,
    eLineSpacingSingle,
    eLineSpacingOneHalf,
    eLineSpacingDouble
};

/////////////////////////////////////////////////////////////////////

#endif // _XTEXTSTYLE_H_

