// Windows Imaging Component helper functions
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWICHELPERS_H_
#define _XWICHELPERS_H_

/////////////////////////////////////////////////////////////////////
// WIC headers
#include <wincodec.h>
#include <wincodecsdk.h>

/////////////////////////////////////////////////////////////////////
// XWicHelpers - WIC helper functions

namespace XWicHelpers
{
    // WIC support
    bool                isSupported();

    // WIC factory
    IWICImagingFactory* getImagingFactory();
    void                releaseImagingFactory();

    // load images
    IWICBitmapDecoder*  decoderFromFile(const wchar_t* szFileName, IWICImagingFactory* pImagingFactory = 0);
    IWICBitmapDecoder*  decoderFromResource(HMODULE hModule, const wchar_t* szResName, const wchar_t* szResType,
                                            IWICImagingFactory* pImagingFactory = 0);
    IWICBitmapDecoder*  decoderFromBuffer(const unsigned char* memoryBuffer, ULONG bufferSize, 
                                          IWICImagingFactory* pImagingFactory = 0);

    // bitmaps
    IWICBitmap*         createBitmap(IWICBitmapSource* pBitmapSource, WICBitmapCreateCacheOption options,
                                     IWICImagingFactory* pImagingFactory = 0);

    IWICBitmap*         createBitmap(HBITMAP hBitmap, HPALETTE hPalette = 0, WICBitmapAlphaChannelOption options = WICBitmapUseAlpha,
                                     IWICImagingFactory* pImagingFactory = 0);

    // image scaling
    IWICBitmapSource*   scaleImageSource(IWICBitmapSource* pBitmapOriginal, int width, int height,
                                         WICBitmapInterpolationMode mode = WICBitmapInterpolationModeFant);

    // image clipping
    IWICBitmapSource*   clipImageSource(IWICBitmapSource* pBitmapOriginal, int posX, int posY, 
                                        int width, int height);

    // image flip and rotation
    IWICBitmapSource*   flipRotateImageSource(IWICBitmapSource* pBitmapOriginal, WICBitmapTransformOptions options);

    // image properties
    bool                getImageSize(IWICBitmap* wicBitmap, int& width, int& height);
};

// XWicHelpers
/////////////////////////////////////////////////////////////////////

#endif // _XWICHELPERS_H_


