// Image file helpers
//
/////////////////////////////////////////////////////////////////////

#ifndef _XIMAGEFILEHELPERS_H_
#define _XIMAGEFILEHELPERS_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
class XMediaSource;

/////////////////////////////////////////////////////////////////////
// XImageFileHelpers - image file functionality

namespace XImageFileHelpers
{
    // image caching
    void     formatFileBitmapHash(const wchar_t* filePath, std::wstring& hashOut);
    void     formatFileBitmapHash(const wchar_t* filePath, int width, int height, std::wstring& hashOut);
    void     formatResourceBitmapHash(HMODULE hModule, const wchar_t* szResName, const wchar_t* szResType, std::wstring& hashOut);
    void     formatResourceBitmapHash(HMODULE hModule, const wchar_t* szResName, const wchar_t* szResType, int width, int height, std::wstring& hashOut);
    void     formatStyleBitmapHash(const wchar_t* stylePath, std::wstring& hashOut);
    void     formatStyleBitmapHash(const wchar_t* stylePath, int width, int height, std::wstring& hashOut);
    void     formatImageBitmapHash(const XMediaSource& imageSource, std::wstring& hashOut);
    void     formatImageBitmapHash(const XMediaSource& imageSource, int width, int height, std::wstring& hashOut);
    void     formatCustomBitmapHash(const XMediaSource& imageSource, const std::wstring& keyword, std::wstring& hashOut);
    void     formatCustomBitmapHash(const XMediaSource& imageSource, const std::wstring& keyword, int width, int height, std::wstring& hashOut);

    // make sure scaled size will preserve aspect ratio
    void     preserveAspectRatio(int originalWidth, int originalHeight, int& width, int& height);
    void     fitToSizeAspectRatio(int width, int height, int& offsetX, int& offsetY, int& imgWidth, int& imgHeight);
};

// XImageFileHelpers
/////////////////////////////////////////////////////////////////////

#endif // _XIMAGEFILEHELPERS_H_

