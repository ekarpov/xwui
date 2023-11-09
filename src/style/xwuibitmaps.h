// XWUI style bitmaps
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWUI_BITMAPS_H_
#define _XWUI_BITMAPS_H_

/////////////////////////////////////////////////////////////////////
// XWUIBitmaps - in memory bitmaps

namespace XWUIBitmaps
{
    // manage bitmaps
    bool    setBitmapData(const wchar_t* name, const unsigned char* data, size_t size);

    // bitmap data from name
    bool    getBitmapData(const wchar_t* name, const unsigned char*& dataOut, size_t& sizeOut);

    // release resources
    void    releaseBitmapData();
};

// XWUIBitmaps
/////////////////////////////////////////////////////////////////////

#endif // _XWUI_BITMAPS_H_

