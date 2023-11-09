// DirectWrite font creation and caching
//
/////////////////////////////////////////////////////////////////////

#ifndef _XDWFONTS_H_
#define _XDWFONTS_H_

/////////////////////////////////////////////////////////////////////
// direct write headers
#include <dwrite.h>

/////////////////////////////////////////////////////////////////////
// XDWFonts - DirectWrite fonts helpers

namespace XDWFonts
{
    // get font based on its properties 
    IDWriteFontFace*    getDirectWriteFont(const XTextStyle& style);

    // create front from GDI LOGFONT
    IDWriteFontFace*    getFontFromLOGFONT(const LOGFONT& fnt);

    // default system font
    IDWriteFontFace*    getDefaultFont();

    // release font cache
    void        releaseFontCache();
};

// XDWFonts
/////////////////////////////////////////////////////////////////////

#endif // _XDWFONTS_H_

