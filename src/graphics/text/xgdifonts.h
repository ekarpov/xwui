// GDI font creation and caching
//
/////////////////////////////////////////////////////////////////////

#ifndef _XGDIFONTS_H_
#define _XGDIFONTS_H_

/////////////////////////////////////////////////////////////////////
// XGdiFonts - GDI fonts helpers

namespace XGdiFonts
{
    // NOTE: if bUseCache flag is set XGdiFonts keeps result in its 
    //       own cache, do not delete returned HFONT

    // get font based on its properties 
    HFONT       getGDIFont(const XTextStyle& style, bool bUseCache = true);

    // font properties
    void        getGDIFontHeight(HWND hwnd, HFONT hFont, int& fontHeight);

    // release font cache
    void        releaseFontCache();
};

// XGdiFonts
/////////////////////////////////////////////////////////////////////

#endif // _XGDIFONTS_H_

