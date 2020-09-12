// DirectWrite font creation and caching
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"

#include <vector>

#include "xrichtext.h"
#include "xdwhelpers.h"
#include "xdwfonts.h"

/////////////////////////////////////////////////////////////////////
// font cache entry
struct XDWFontsCacheEntry
{
    XTextStyle          style;
    IDWriteFontFace*    fontFace;
};

// cache key
bool sXDWFontsCompareStyles(const XTextStyle& styleLeft, const XTextStyle& styleRight)
{
    return styleLeft.strFontName == styleRight.strFontName &&
           styleLeft.bBold == styleRight.bBold &&
           styleLeft.bItalic == styleRight.bItalic;

           // NOTE: these attributes are not relevant to DirectWrite font, 
           //       they must be rendered manually

           //styleLeft.nFontSize == styleRight.nFontSize && 
           //styleLeft.bUnderline == styleRight.bUnderline &&
           //styleLeft.bStrike == styleRight.bStrike;
}

// font cache
static std::vector<XDWFontsCacheEntry>* s_pXDWFontsCache = 0;

// global default font
static IDWriteFontFace*     s_pDefaultSystemFont = 0;

/////////////////////////////////////////////////////////////////////
// XDWFonts - DirectWrite fonts helpers

/////////////////////////////////////////////////////////////////////
// get font based on its properties 
/////////////////////////////////////////////////////////////////////
IDWriteFontFace* XDWFonts::getDirectWriteFont(const XTextStyle& style)
{
    // ignore if not loaded
    if(!XDWriteHelpers::isDirectWriteLoaded())
    {
        XWTRACE("XDWFonts::getDirectWriteFont cannot load font as DirectWrite is not loaded");
        return 0;
    }

    // create cache if not set already
    if(s_pXDWFontsCache == 0)
    {
        s_pXDWFontsCache = new std::vector<XDWFontsCacheEntry>;
    }

    // check from cache fist
    if(s_pXDWFontsCache)
    {
        // loop over all items
        for(unsigned int idx = 0; idx < s_pXDWFontsCache->size(); ++idx)
        {
            // compare styles
            if(sXDWFontsCompareStyles(s_pXDWFontsCache->at(idx).style, style))
            {
                // add reference
                s_pXDWFontsCache->at(idx).fontFace->AddRef();

                // use cached font
                return s_pXDWFontsCache->at(idx).fontFace;
            }
        }
    }

    IDWriteFontFace* fontFace = 0;

    // get system font collection
    IDWriteFontCollection* fontCollection = 0;
    HRESULT hr = XDWriteHelpers::getDirectWriteFactory()->GetSystemFontCollection(&fontCollection);

    // search by name
    if(SUCCEEDED(hr) && fontCollection)
    {
        UINT32 fontIndex = 0; // zero by default
        BOOL bExists = FALSE;

        // style font name
        std::wstring fontName = style.strFontName;

        // check if font name is set
        if(fontName.length() == 0)
        {
            // get default font properties
            LOGFONT fnt;
            XWUtils::sGetDefaultFontProps(fnt);

            // copy name
            fontName = fnt.lfFaceName;
        }

        // search
        hr = fontCollection->FindFamilyName(fontName.c_str(), &fontIndex, &bExists);

        // use default system font if not found (and default has not been used already)
        if((FAILED(hr) || !bExists) && style.strFontName.length() != 0)
        {
            // get default font properties
            LOGFONT fnt;
            XWUtils::sGetDefaultFontProps(fnt);

            // search
            hr = fontCollection->FindFamilyName(fnt.lfFaceName, &fontIndex, &bExists);
        }

        // use some default family if required doesn't exist
        if(FAILED(hr) || !bExists)
        {
            XWTRACE("XDWFonts::getDirectWriteFont failed to find required font name");

            // use first font from collection as default
            fontIndex = 0; 
        }

        IDWriteFontFamily* fontFamily = 0;

        // get font family
        if(SUCCEEDED(hr))
        {
            hr = fontCollection->GetFontFamily(fontIndex, &fontFamily);
            if(FAILED(hr))
            {
                XWTRACE_HRES("XDWFonts::getDirectWriteFont failed to get font family", hr);
            }
        }

        IDWriteFont* dwFont = 0;

        // get font 
        if(SUCCEEDED(hr) && fontFamily)
        {
            hr = fontFamily->GetFirstMatchingFont(
                style.bBold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                style.bItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
                &dwFont);

            // report errors
            if(FAILED(hr))
            {
                XWTRACE_HRES("XDWFonts::getDirectWriteFont failed to get matching font", hr);
            }
        }

        // get font face
        if(SUCCEEDED(hr) && dwFont)
        {
            // get font face
            hr = dwFont->CreateFontFace(&fontFace);

            // report errors
            if(FAILED(hr))
            {
                XWTRACE_HRES("XDWFonts::getDirectWriteFont failed to create font face", hr);
            }
        }

        // release font 
        if(dwFont) dwFont->Release();
        dwFont = 0;

        // release font family
        if(fontFamily) fontFamily->Release();
        fontFamily = 0;
    }

    // release font colleciton
    if(fontCollection) fontCollection->Release();
    fontCollection = 0;

    // check result
    if(FAILED(hr) || fontFace == 0)
    {
        XWTRACE_HRES("XDWFonts::getDirectWriteFont: failed to get font, will skip style and load default font", hr);

        // get default font
        fontFace = getDefaultFont();
    }

    // add to cache
    if(fontFace)
    {
        // NOTE: we keep single reference for cache (will be released in releaseFontCache)
        fontFace->AddRef();

        // add reference
        fontFace->AddRef();

        // format cache entry
        XDWFontsCacheEntry entry;
        entry.style = style;
        entry.fontFace = fontFace;

        // append
        s_pXDWFontsCache->push_back(entry);
    }

    return fontFace;
}

/////////////////////////////////////////////////////////////////////
// default system font
/////////////////////////////////////////////////////////////////////
IDWriteFontFace* XDWFonts::getFontFromLOGFONT(const LOGFONT& fnt)
{
    // ignore if not loaded
    if(!XDWriteHelpers::isDirectWriteLoaded())
    {
        XWTRACE("XDWFonts::getFontFromLOGFONT cannot load font as DirectWrite is not loaded");
        return 0;
    }

    // get GDI interop interface
    IDWriteGdiInterop* gdiInterop = 0;
    HRESULT hr = XDWriteHelpers::getDirectWriteFactory()->GetGdiInterop(&gdiInterop);

    // check result
    if(FAILED(hr))
    {
        XWTRACE_HRES("XDWFonts::getFontFromLOGFONT failed to create GDI interoperability interface", hr);
        return 0;
    }

    // find font
    IDWriteFont* dwFont = 0;
    hr = gdiInterop->CreateFontFromLOGFONT(&fnt, &dwFont);
    
    // release GDI interop
    gdiInterop->Release();
    gdiInterop = 0;

    // check result
    if(FAILED(hr))
    {
        XWTRACE_HRES("XDWFonts::getFontFromLOGFONT failed to create DirectWrite front from LOGFONT", hr);
        return 0;
    }
    
    // get font face
    IDWriteFontFace* fontFace = 0;
    hr = dwFont->CreateFontFace(&fontFace);

    // release font
    dwFont->Release();
    dwFont = 0;

    // check result
    if(FAILED(hr))
    {
        XWTRACE_HRES("XDWFonts::getFontFromLOGFONT failed to create front face from font", hr);
        return 0;
    }
    
    return fontFace;
}

/////////////////////////////////////////////////////////////////////
// default system font
/////////////////////////////////////////////////////////////////////
IDWriteFontFace* XDWFonts::getDefaultFont()
{
    // get default font properties
    LOGFONT fnt;
    XWUtils::sGetDefaultFontProps(fnt);

    // create font from LOGFONT
    return getFontFromLOGFONT(fnt);
}

/////////////////////////////////////////////////////////////////////
// release font cache
/////////////////////////////////////////////////////////////////////
void XDWFonts::releaseFontCache()
{
    // ignore if not set
    if(s_pXDWFontsCache == 0) return;

    // loop over all entries
    for(unsigned int idx = 0; idx < s_pXDWFontsCache->size(); ++idx)
    {
        // release font
        s_pXDWFontsCache->at(idx).fontFace->Release();
    }

    // delete cache
    delete s_pXDWFontsCache;
    s_pXDWFontsCache = 0;
}

// XDWFonts
/////////////////////////////////////////////////////////////////////


