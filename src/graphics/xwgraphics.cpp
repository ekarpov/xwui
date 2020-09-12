// Common graphics functions 
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xwgraphics.h"

/////////////////////////////////////////////////////////////////////
// Uniscribe library
#pragma comment(lib, "Usp10.lib")

/////////////////////////////////////////////////////////////////////
// IIDs for WIC library
#pragma comment(lib, "WindowsCodecs.lib")

/////////////////////////////////////////////////////////////////////
// init and close resources
bool sInitXWUIGraphics(bool loadDirect2D, bool loadDirectWrite)
{
    bool bRetVal = true;

    // try to load Direct2D
    if(loadDirect2D && !XD2DHelpers::loadDirect2DLibrary())
    {
        XWTRACE("Failed to load Direct2D, GDI will be used instead for rendering");
    }

    // try to load DirectWrite
    if(loadDirectWrite && !XDWriteHelpers::loadDirectWriteLibrary())
    {
        XWTRACE("Failed to load DirectWrite, GDI will be used instead for text rendering");
    }

    return bRetVal;
}

void sCloseXWUIGraphics()
{
    // release font caches if any
    XGdiFonts::releaseFontCache();
    XDWFonts::releaseFontCache();

    // release DirectWrite if any
    XDWriteHelpers::freeDirectWriteLibrary();

    // release Direct2D if any
    XD2DHelpers::freeDirect2DLibrary();

    // release WIC factory if any
    XWicHelpers::releaseImagingFactory();
}

/////////////////////////////////////////////////////////////////////


