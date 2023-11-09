// Image file helpers
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "ximagefilehelpers.h"

/////////////////////////////////////////////////////////////////////
// XImageFileHelpers - image file functionality

/////////////////////////////////////////////////////////////////////
// image caching
/////////////////////////////////////////////////////////////////////
void XImageFileHelpers::formatFileBitmapHash(const wchar_t* filePath, std::wstring& hashOut)
{
    // reset hash
    hashOut.clear();

    XWASSERT(filePath);
    if(filePath == 0) return;

    // use path as hash
    hashOut = filePath;
}

void XImageFileHelpers::formatFileBitmapHash(const wchar_t* filePath, int width, int height, std::wstring& hashOut)
{
    // format normal hash first
    formatFileBitmapHash(filePath, hashOut);

    // append scaling
    hashOut += L":" + std::to_wstring((ULONGLONG)width) + 
               L":" + std::to_wstring((ULONGLONG)height);
}

void XImageFileHelpers::formatResourceBitmapHash(HMODULE hModule, const wchar_t* szResName, const wchar_t* szResType, std::wstring& hashOut)
{
    // reset hash
    hashOut.clear();

    // validate input
    XWASSERT(szResName);
    if(szResName == 0) return;

    std::wstring resName, resType;

    // check if resources are used as integer offsets
    if(IS_INTRESOURCE(szResName))
    {
        // we need to convert integer from pointer
        WORD wResId = (WORD)szResName;

        // format unique name from id
        resName = L"id:" + std::to_wstring((ULONGLONG)wResId);

    } else
    {
        // just copy name
        resName = szResName;
    }

    if(szResType && IS_INTRESOURCE(szResType))
    {
        // we need to convert integer from pointer
        WORD wResTypeId = (WORD)szResType;
        
        // format unique name from id
        resType = L"id:" + std::to_wstring((ULONGLONG)wResTypeId);

    } else if(szResType)
    {
        // just copy type
        resType = szResType;
    }

    // check if module is set and different from current
    if(hModule != 0 && hModule != ::GetModuleHandleW(0))
    {
        // get module name for path
        std::vector<wchar_t> moduleName;
        if(!XWUtils::sGetModuleName(hModule, moduleName))
        {
            XWTRACE("XImageFile: failed to get module name, image hash will be incorrect");
        }

        // format path
        hashOut = std::wstring(moduleName.begin(), moduleName.end()) + L":/" + resName;

    } else
    {
        // format path
        hashOut = std::wstring(L":/") + resName;
    }

    // append type if any
    if(szResType) hashOut += std::wstring(L"/") + resType;
}

void XImageFileHelpers::formatResourceBitmapHash(HMODULE hModule, const wchar_t* szResName, const wchar_t* szResType, int width, int height, std::wstring& hashOut)
{
    // format normal hash first
    formatResourceBitmapHash(hModule, szResName, szResType, hashOut);

    // append scaling
    hashOut += L":" + std::to_wstring((ULONGLONG)width) + 
               L":" + std::to_wstring((ULONGLONG)height);
}

void XImageFileHelpers::formatStyleBitmapHash(const wchar_t* stylePath, std::wstring& hashOut)
{
    // reset hash
    hashOut.clear();

    XWASSERT(stylePath);
    if(stylePath == 0) return;

    // format hash
    hashOut = std::wstring(L"style:/") + stylePath;
}

void XImageFileHelpers::formatStyleBitmapHash(const wchar_t* stylePath, int width, int height, std::wstring& hashOut)
{
    // format normal hash first
    formatStyleBitmapHash(stylePath, hashOut);

    // append scaling
    hashOut += L":" + std::to_wstring((ULONGLONG)width) + 
               L":" + std::to_wstring((ULONGLONG)height);
}

void XImageFileHelpers::formatImageBitmapHash(const XMediaSource& imageSource, std::wstring& hashOut)
{
    // check image source type
    if(imageSource.type() == eXMediaSourceResource)
        formatResourceBitmapHash(imageSource.resModule(), imageSource.resName(), imageSource.resType(), hashOut);
    else if(imageSource.type() == eXMediaSourceStyle)
        formatStyleBitmapHash(imageSource.path(), hashOut);
    else
        formatFileBitmapHash(imageSource.path(), hashOut);
}

void XImageFileHelpers::formatImageBitmapHash(const XMediaSource& imageSource, int width, int height, std::wstring& hashOut)
{
    // check image source type
    if(imageSource.type() == eXMediaSourceResource)
        formatResourceBitmapHash(imageSource.resModule(), imageSource.resName(), imageSource.resType(), width, height, hashOut);
    else if(imageSource.type() == eXMediaSourceStyle)
        formatStyleBitmapHash(imageSource.path(), width, height, hashOut);
    else
        formatFileBitmapHash(imageSource.path(), width, height, hashOut);
}

void XImageFileHelpers::formatCustomBitmapHash(const XMediaSource& imageSource, const std::wstring& keyword, std::wstring& hashOut)
{
    // format default hash first
    formatImageBitmapHash(imageSource, hashOut);

    // append keyword
    hashOut += L":";
    hashOut += keyword;
}

void XImageFileHelpers::formatCustomBitmapHash(const XMediaSource& imageSource, const std::wstring& keyword, int width, int height, std::wstring& hashOut)
{
    // format default hash first
    formatImageBitmapHash(imageSource, width, height, hashOut);

    // append keyword
    hashOut += L":";
    hashOut += keyword;
}

/////////////////////////////////////////////////////////////////////
// make sure scaled size will preserve aspect ratio
/////////////////////////////////////////////////////////////////////
void XImageFileHelpers::preserveAspectRatio(int originalWidth, int originalHeight, int& width, int& height)
{
    // validate input
    if(originalWidth <= 0 || originalHeight <= 0 || width <= 0 || height <= 0) return;

    // ignore if already scalled
    if(originalWidth == width && originalHeight == height) return;

    float widthScale = (float)width/(float)originalWidth;
    float heightScale = (float)height/(float)originalHeight;

    // use smallest ratio so that scalled image will fit to required size
    float scaleFactor = (widthScale <= heightScale) ? widthScale : heightScale;

    // update scale
    originalWidth = (int) (originalWidth * scaleFactor);
    originalHeight = (int) (originalHeight * scaleFactor);

    // make sure sizes will not get bigger due to some rounding error
    if(originalWidth > width) originalWidth = width;
    if(originalHeight > height) originalHeight = height;

    // set new size that respects aspect ratio
    width = originalWidth;
    height = originalHeight;
}

void XImageFileHelpers::fitToSizeAspectRatio(int width, int height, int& offsetX, int& offsetY, int& imgWidth, int& imgHeight)
{
    // reset offsets
    offsetX = 0;
    offsetY = 0;

    // validate input
    if(imgWidth <= 0 || imgHeight <= 0 || width <= 0 || height <= 0) return;

    // ignore if already scalled
    if(imgWidth == width && imgHeight == height) return;

    float widthScale = (float)width/(float)imgWidth;
    float heightScale = (float)height/(float)imgHeight;

    // check where we need to scale
    if(width < imgWidth * heightScale)
    {
        // find difference we need to cut
        float cutSize = ((imgWidth * heightScale) - width) / heightScale;

        // image offset
        offsetX = (int)(cutSize / 2);

        // image widht
        imgWidth -= 2 * offsetX;

    } else if(height < imgHeight * widthScale)
    {
        // find difference we need to cut
        float cutSize = ((imgHeight * widthScale) - height) / widthScale;

        // image offset
        offsetY = (int)(cutSize / 2);

        // image height
        imgHeight -= 2 * offsetY;
    }
}

// XImageFileHelpers
/////////////////////////////////////////////////////////////////////
