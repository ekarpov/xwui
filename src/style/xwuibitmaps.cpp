// XWUI style bitmaps
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"
#include "../resources/xwui_resources.h"

#include "xwuibitmaps.h"

/////////////////////////////////////////////////////////////////////
// bitmap data 

// reference
struct XWUIBitmapRef
{
    std::wstring            name;
    const unsigned char*    data;
    size_t                  size;

    XWUIBitmapRef() : data(0), size(0) {}
};

// bitmaps
typedef std::vector<XWUIBitmapRef>  XWUIBitmapDataT;

// NOTE: we need to allocate static data on the heap for memory
//       leak checking to work properly. Otherwise std::wstring in
//       static data causes false alarms
static XWUIBitmapDataT* g_pXWUIBitmapData = 0;

/////////////////////////////////////////////////////////////////////
// helper functions

void    _initXWUIBitmaps(XWUIBitmapDataT* xwuiBitmaps);
bool    _findXWUIBitmap(XWUIBitmapDataT* xwuiBitmaps, const wchar_t* name, size_t& idxOut);

/////////////////////////////////////////////////////////////////////
// style instance
inline XWUIBitmapDataT* _xwuiBitmapData()
{
    if(g_pXWUIBitmapData == 0)
    {
        g_pXWUIBitmapData = new XWUIBitmapDataT();

        // init predefined bitmaps
        _initXWUIBitmaps(g_pXWUIBitmapData);
    }

    XWASSERT(g_pXWUIBitmapData);
    return g_pXWUIBitmapData;
}

/////////////////////////////////////////////////////////////////////
// XWUIBitmaps - in memory bitmaps

// manage bitmaps
bool XWUIBitmaps::setBitmapData(const wchar_t* name, const unsigned char* data, size_t size)
{
    // check input
    XWASSERT(name);
    if(name == 0) return false;

    // get bitmaps
    XWUIBitmapDataT* xwuiBitmaps = _xwuiBitmapData();
    if(xwuiBitmaps == 0) return false;

    // find bitmap
    size_t bitmapIdx = 0;
    if(_findXWUIBitmap(xwuiBitmaps, name, bitmapIdx))
    {
        // replace data
        xwuiBitmaps->at(bitmapIdx).data = data;
        xwuiBitmaps->at(bitmapIdx).size = size;

    } else
    {
        XWUIBitmapRef ref;

        // add new bitmap
        ref.name = name;
        ref.data = data;
        ref.size = size;

        xwuiBitmaps->push_back(ref);
    }

    return true;
}

// bitmap data from name
bool XWUIBitmaps::getBitmapData(const wchar_t* name, const unsigned char*& dataOut, size_t& sizeOut)
{
    // check input
    XWASSERT(name);
    if(name == 0) return false;

    // get bitmaps
    XWUIBitmapDataT* xwuiBitmaps = _xwuiBitmapData();
    if(xwuiBitmaps == 0) return false;

    // reset output
    dataOut = 0;
    sizeOut = 0;

    // find bitmap
    size_t bitmapIdx = 0;
    if(_findXWUIBitmap(xwuiBitmaps, name, bitmapIdx))
    {
        // set data
        dataOut = xwuiBitmaps->at(bitmapIdx).data;
        sizeOut = xwuiBitmaps->at(bitmapIdx).size;

        return true;

    } else
    {
        XWTRACE1("XWUIBitmaps: requested bitmap \"%S\" not found", name);
        return false;
    }
}

/////////////////////////////////////////////////////////////////////
// release resources
/////////////////////////////////////////////////////////////////////
void XWUIBitmaps::releaseBitmapData()
{
    if(g_pXWUIBitmapData)
    {
        delete g_pXWUIBitmapData;
        g_pXWUIBitmapData = 0;
    }
}

// XWUIBitmaps
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// helper functions

// helper macro
#define XWUI_ADD_BITMAP_DATA(_name, _ext) \
    {\
        XWUIBitmapRef bitmapRef;\
        bitmapRef.name = L#_name;\
        bitmapRef.name += L".";\
        bitmapRef.name += L#_ext;\
        bitmapRef.data = ##_name##_##_ext##_##data;\
        bitmapRef.size = ##_name##_##_ext##_##data##_##size;\
        xwuiBitmaps->push_back(bitmapRef);\
    }

void _initXWUIBitmaps(XWUIBitmapDataT* xwuiBitmaps)
{
    // spinner frames 26px
    XWUI_ADD_BITMAP_DATA(spinner_frame_1_26, png);
    XWUI_ADD_BITMAP_DATA(spinner_frame_2_26, png);
    XWUI_ADD_BITMAP_DATA(spinner_frame_3_26, png);
    XWUI_ADD_BITMAP_DATA(spinner_frame_4_26, png);
    XWUI_ADD_BITMAP_DATA(spinner_frame_5_26, png);
    XWUI_ADD_BITMAP_DATA(spinner_frame_6_26, png);
    XWUI_ADD_BITMAP_DATA(spinner_frame_7_26, png);
    XWUI_ADD_BITMAP_DATA(spinner_frame_8_26, png);

    // spinner frames 32px
    XWUI_ADD_BITMAP_DATA(spinner_frame_1_32, png);
    XWUI_ADD_BITMAP_DATA(spinner_frame_2_32, png);
    XWUI_ADD_BITMAP_DATA(spinner_frame_3_32, png);
    XWUI_ADD_BITMAP_DATA(spinner_frame_4_32, png);
    XWUI_ADD_BITMAP_DATA(spinner_frame_5_32, png);
    XWUI_ADD_BITMAP_DATA(spinner_frame_6_32, png);
    XWUI_ADD_BITMAP_DATA(spinner_frame_7_32, png);
    XWUI_ADD_BITMAP_DATA(spinner_frame_8_32, png);

    // bitmap buttons 26px
    XWUI_ADD_BITMAP_DATA(add_icon_26, png);
    XWUI_ADD_BITMAP_DATA(add_selected_icon_26, png);
    XWUI_ADD_BITMAP_DATA(back_icon_26, png);
    XWUI_ADD_BITMAP_DATA(back_selected_icon_26, png);
    XWUI_ADD_BITMAP_DATA(refresh_icon_26, png);
    XWUI_ADD_BITMAP_DATA(refresh_selected_icon_26, png);
    XWUI_ADD_BITMAP_DATA(delete_icon_26, png);
    XWUI_ADD_BITMAP_DATA(delete_selected_icon_26, png);
    XWUI_ADD_BITMAP_DATA(create_new_icon_26, png);
    XWUI_ADD_BITMAP_DATA(create_new_selected_icon_26, png);

    // bitmap buttons 15px
    XWUI_ADD_BITMAP_DATA(close_15_filled, png);
    XWUI_ADD_BITMAP_DATA(close_15_filled_transparent, png);
    XWUI_ADD_BITMAP_DATA(delete_15, png);
    XWUI_ADD_BITMAP_DATA(delete_filled_15, png);
}

bool _findXWUIBitmap(XWUIBitmapDataT* xwuiBitmaps, const wchar_t* name, size_t& idxOut)
{
    XWASSERT(name);
    XWASSERT(xwuiBitmaps);

    // loop over bitmaps
    for(idxOut = 0; idxOut < xwuiBitmaps->size(); ++idxOut)
    {
        if(_wcsicmp(xwuiBitmaps->at(idxOut).name.c_str(), name) == 0) 
        {
            // item found
            return true;
        }
    }

    // not found
    return false;
}

/////////////////////////////////////////////////////////////////////
