// Uniscribe helper functions
//
/////////////////////////////////////////////////////////////////////

#ifndef _XUNISCRIBEHELPERS_H_
#define _XUNISCRIBEHELPERS_H_

/////////////////////////////////////////////////////////////////////
// Uniscribe headers
#include <Usp10.h>

/////////////////////////////////////////////////////////////////////
// XUniscribeHelpers

namespace XUniscribeHelpers
{
    ///// text run
    struct XUniTextRun
    {
        XTextRange                  range;
        XTextStyle                  style;
        bool                        isInlineObject;
        bool                        isComplex;
        bool                        isRTL;
        SCRIPT_ANALYSIS             scriptProps;
    };

    ///// font data and cache
    struct XUniFontData
    {
        HFONT                       hFont;      
        int                         fontAscent;     
        int                         fontHeight;     
        SCRIPT_CACHE                scriptCache;
    };

    ///// script shape data
    struct XUniScriptShape
    {
        std::vector<WORD>           glyphs;         // needed by ScriptTextOut
        std::vector<WORD>           logCluster;     // needed to map glyphs to characters
        std::vector<SCRIPT_VISATTR> scriptAttrs;    // needed by ScriptJustify
    };

    ///// script place data
    struct XUniScriptPlace
    {
        std::vector<int>            advances;       // needed by ScriptTextOut
        std::vector<GOFFSET>        offsets;        // needed by ScriptTextOut
        ABC                         abcOffsets;
        int                         width;
    };

    ///// script data cache for text run
    struct XUniTextRunCache
    {
        XUniscribeHelpers::XUniScriptShape  shape;
        XUniscribeHelpers::XUniScriptPlace  place;
        std::vector<unsigned int>           glyphToChar;
        std::vector<SCRIPT_LOGATTR>         logAttrs;
    };

    // split text into runs based on text style and script
    void    analyseRichText(const XRichText* richText, const XTextRange& range, std::vector<XUniTextRun>& runsOut,
                            const SCRIPT_CONTROL* scriptCtrl = 0, const SCRIPT_STATE* scriptState = 0);
    
    // re-order text runs in visual order
    void    layoutTextRuns(std::vector<XUniTextRun>& uniTextRuns);

    // generate shape data for text run
    bool    shapeText(HDC hdc, const XRichText* richText, XUniTextRun& textRun, 
                      XUniFontData& fontCache, XUniScriptShape& scriptShape, bool forceShape);

    // generate position data for text run
    void    placeText(HDC hdc, const XUniScriptShape& scriptShape, XUniTextRun& textRun, 
                      XUniFontData& fontCache, XUniScriptPlace& scriptPlace);

    // NOTE: glyph mapping is done so that each cluster corresponds to cluster start character
    //       use logCluster index returned by shapeText for fine-grained mapping

    // get mapping from glyphs to characters for complex scripts
    void    mapGlyphsToChars(const XUniTextRun& textRun, const XUniScriptShape& scriptShape, std::vector<unsigned int>& glyphToChar);

    // get logical line attributes
    void    getRunLogicalAttrs(const XRichText* richText, const XUniTextRun& textRun, std::vector<SCRIPT_LOGATTR>& logAttrs);

    // release cache
    void    releaseCache(XUniFontData& fontCache);
};

// XUniscribeHelpers
/////////////////////////////////////////////////////////////////////

#endif // _XUNISCRIBEHELPERS_H_

