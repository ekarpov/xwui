// GDI font creation and caching
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"
#include "../xgdihelpres.h"

#include "xgdifonts.h"

/////////////////////////////////////////////////////////////////////

// NOTE: LOGFONT lfHeight (The height, in logical units, of the font's character cell or character)
//       https://msdn.microsoft.com/en-us/library/windows/desktop/dd145037(v=vs.85).aspx

/////////////////////////////////////////////////////////////////////
// font cache entry
struct XGdiFontsCacheEntry
{
    XTextStyle  style;
    HFONT       hFont;
};

// cache key
bool sXGdiFontsCompareStyles(const XTextStyle& styleLeft, const XTextStyle& styleRight)
{
    return styleLeft.strFontName == styleRight.strFontName &&
           styleLeft.nFontSize == styleRight.nFontSize &&
           styleLeft.bBold == styleRight.bBold &&
           styleLeft.bItalic == styleRight.bItalic &&
           styleLeft.bUnderline == styleRight.bUnderline &&
           styleLeft.bStrike == styleRight.bStrike;
}

// font cache
static std::vector<XGdiFontsCacheEntry>* s_pXGdiFontsCache = 0;

/////////////////////////////////////////////////////////////////////
// XGdiFonts - GDI fonts helpers

/////////////////////////////////////////////////////////////////////
// get font based on its properties
/////////////////////////////////////////////////////////////////////
HFONT XGdiFonts::getGDIFont(const XTextStyle& style, bool bUseCache)
{
    // create cache if not set already
    if(s_pXGdiFontsCache == 0 && bUseCache)
    {
        s_pXGdiFontsCache = new std::vector<XGdiFontsCacheEntry>;
    }

    // check from cache fist
    if(s_pXGdiFontsCache && bUseCache)
    {
        // loop over all items
        for(unsigned int idx = 0; idx < s_pXGdiFontsCache->size(); ++idx)
        {
            // compare styles
            if(sXGdiFontsCompareStyles(s_pXGdiFontsCache->at(idx).style, style))
            {
                // use cached font
                return s_pXGdiFontsCache->at(idx).hFont;
            }
        }
    }

    LOGFONTW lf;

    // init with default settings
    XWUtils::sGetDefaultFontProps(lf);

    // format style settings
    lf.lfHeight = - XGdiHelpers::dpiScalePixelsY(style.nFontSize);
    lf.lfWeight = style.bBold ? FW_BOLD : FW_NORMAL;
    lf.lfItalic = style.bItalic ? TRUE : FALSE;
    lf.lfUnderline = style.bUnderline ? TRUE : FALSE;
    lf.lfStrikeOut = style.bStrike ? TRUE : FALSE;

    size_t fontNameLen = style.strFontName.length();

    // copy face name if set
    if(fontNameLen > 0)
    {
        // check if font name length is smaller than maximum size
        if(fontNameLen < LF_FACESIZE + 1)
        {
            // copy font name
            ::wcsncpy_s(lf.lfFaceName, LF_FACESIZE, style.strFontName.c_str(), fontNameLen);

        } else
        {
            XWTRACE1("XGdiFonts::getGDIFont font name ignored as it exceeds %d", LF_FACESIZE);
        }
    }

    // create font
    HFONT hFont = ::CreateFontIndirect(&lf);

    // check result
    if(hFont == 0)
    {
        XWTRACE_WERR_LAST("XGdiFonts::getGDIFont failed to create font, backing up to default");
        return XWUtils::sGetDefaultFont();
    }

    // add to cache if needed
    if(s_pXGdiFontsCache && bUseCache)
    {
        // format cache entry
        XGdiFontsCacheEntry entry;
        entry.style = style;
        entry.hFont = hFont;

        // append
        s_pXGdiFontsCache->push_back(entry);
    }

    return hFont;
}

/////////////////////////////////////////////////////////////////////
// font properties
/////////////////////////////////////////////////////////////////////
void XGdiFonts::getGDIFontHeight(HWND hwnd, HFONT hFont, int& fontHeight)
{
    // get window dc
    HDC hdc = ::GetDC(hwnd);

    // select font
    ::SelectObject(hdc, hFont);

    // get text metrics
    TEXTMETRIC tm;
    ::ZeroMemory(&tm, sizeof( TEXTMETRIC ));    
    ::GetTextMetrics(hdc, &tm);

    // NOTE: The TEXTMETRIC structure contains basic information about a physical font. 
    //       All sizes are specified in logical units; that is, they depend on the current 
    //       mapping mode of the display context.

    // font height in pixels
//    fontHeight = XGdiHelpers::dpiScalePixelsY(tm.tmHeight);

    // TODO: font height for some reason is already in pixels
    //       check mapping mode!
    fontHeight = tm.tmHeight;

    // release device context
    ::ReleaseDC(hwnd, hdc); 
}

/////////////////////////////////////////////////////////////////////
// release font cache if any
/////////////////////////////////////////////////////////////////////
void XGdiFonts::releaseFontCache()
{
    // ignore if not set
    if(s_pXGdiFontsCache == 0) return;

    // loop over all entries
    for(unsigned int idx = 0; idx < s_pXGdiFontsCache->size(); ++idx)
    {
        // release font
        ::DeleteObject(s_pXGdiFontsCache->at(idx).hFont);
    }

    // delete cache
    delete s_pXGdiFontsCache;
    s_pXGdiFontsCache = 0;
}

// XGdiFonts
/////////////////////////////////////////////////////////////////////

