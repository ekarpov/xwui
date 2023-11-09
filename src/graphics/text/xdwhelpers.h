// Direct Write helper functions
//
/////////////////////////////////////////////////////////////////////

#ifndef _XDWHELPERS_H_
#define _XDWHELPERS_H_

/////////////////////////////////////////////////////////////////////
// Uniscribe headers
#include <Usp10.h>

/////////////////////////////////////////////////////////////////////
// text analysis
#include "xdwanalyse.h"

/////////////////////////////////////////////////////////////////////
// XDWriteHelpers - DirectWrite helper functions

namespace XDWriteHelpers
{
    // dynamic loading
    bool    isDirectWriteLoaded();
    bool    loadDirectWriteLibrary();
    void    freeDirectWriteLibrary();

    // global factory
    IDWriteFactory*   getDirectWriteFactory();

    // global text analyzer
    IDWriteTextAnalyzer* getDirectWriteTextAnalyzer();

    ///// text run
    struct XDwTextRun
    {
        XTextRange                  range;
        XTextStyle                  style;
        bool                        isInlineObject;
        bool                        isRTL;
        bool                        isComplex;
        DWRITE_SCRIPT_ANALYSIS      scriptProps;
        UINT8                       bidiLevel;
    };

    ///// font data
    struct XDwFontData
    {
        IDWriteFontFace*            fontFace;      
        FLOAT                       fontEmSize;     
        FLOAT                       fontAscent;     
        FLOAT                       fontHeight;     

        FLOAT                       underlinePosition;     
        FLOAT                       underlineThickness;     
        FLOAT                       strikethroughPosition;     
        FLOAT                       strikethroughThickness;     
    };

    ///// script shape data
    struct XDwScriptShape
    {
        std::vector<UINT16>         glyphs;
        std::vector<UINT16>         clusterMap;
        std::vector<DWRITE_SHAPING_TEXT_PROPERTIES>     textProps;
        std::vector<DWRITE_SHAPING_GLYPH_PROPERTIES>    scriptAttrs;
    };

    ///// script place data
    struct XDwScriptPlace
    {
        std::vector<FLOAT>                  advances;
        std::vector<DWRITE_GLYPH_OFFSET>    offsets;
        FLOAT                               width;
    };

    ///// script logical attributes
    struct XDwScriptLogAttr
    {
        BYTE    fSoftBreak      :1;     // Potential linebreak point
        BYTE    fWhiteSpace     :1;     // A unicode whitespace character, except NBSP, ZWNBSP
        BYTE    fReserved       :6;
    };

    ///// script data cache for text run
    struct XDwTextRunCache
    {
        XDWriteHelpers::XDwScriptShape      shape;
        XDWriteHelpers::XDwScriptPlace      place;
        std::vector<UINT16>                 glyphToChar;
        std::vector<XDwScriptLogAttr>       logAttrs;
    };

    // split text into runs based on text style and script
    void    analyseRichText(const XRichText* richText, const XTextRange& range, std::vector<XDwTextRun>& runsOut,
                            const wchar_t* localeName = 0, bool isRTL = false);

    // re-order text runs in visual order
    void    layoutTextRuns(std::vector<XDwTextRun>& dwTextRuns);

    // generate shape data for text run
    bool    shapeText(const XRichText* richText, XDwTextRun& textRun, XDwFontData& fontCache, 
                      XDwScriptShape& scriptShape, bool forceShape, const wchar_t* localeName = 0);

    // generate position data for text run
    void    placeText(const XRichText* richText, XDwTextRun& textRun, XDwFontData& fontCache,
                      XDwScriptShape& scriptShape, XDwScriptPlace& scriptPlace, const wchar_t* localeName = 0);

    // get logical line attributes
    void    getRunLogicalAttrs(const XRichText* richText, const XDwTextRun& textRun, 
                               std::vector<XDwScriptLogAttr>& logAttrs, const wchar_t* localeName = 0);

    // get mapping from glyphs to characters for complex scripts
    void    mapGlyphsToChars(const XDwTextRun& textRun, XDwScriptShape& scriptShape, std::vector<UINT16>& glyphToChar);

    // release font data
    void    releaseFontData(XDwFontData& fontData);
};

// XDWriteHelpers
/////////////////////////////////////////////////////////////////////

#endif // _XDWHELPERS_H_

