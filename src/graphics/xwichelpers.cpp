// Windows Imaging Component helper functions
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"
#include "../xactive/xolestream.h"

#include "xwichelpers.h"

/////////////////////////////////////////////////////////////////////
// NOTE: Information on MSDN
//
// http://msdn.microsoft.com/en-us/library/ee719902%28v=VS.85%29.aspx
// http://msdn.microsoft.com/en-us/magazine/cc500647.aspx
//
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// static data

// imaging factory
bool    g_bIsWicSupported = true;

// imaging factory
IWICImagingFactory* g_pIWICImagingFactory = 0;

// global stream for resource loading
IWICStream*         g_pIWICStream = 0;

/////////////////////////////////////////////////////////////////////
// XWicHelpers - WIC helper functions

/////////////////////////////////////////////////////////////////////
// WIC support
bool XWicHelpers::isSupported()
{
    // try to get factory
    return (getImagingFactory() != 0);
}

/////////////////////////////////////////////////////////////////////
// WIC factory
IWICImagingFactory* XWicHelpers::getImagingFactory()
{
    // check if we have created factory already
    if(g_pIWICImagingFactory) return g_pIWICImagingFactory;

    // try to load factory
    if(g_bIsWicSupported)
    {
        // try to load factory
        HRESULT res = ::CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_ALL, IID_IWICImagingFactory, (LPVOID*)&g_pIWICImagingFactory);

        // check results
        if(FAILED(res))
        {
            // trace error
            XWTRACE_HRES("XWicHelpers: Failed to load WIC imaging factory, Windows Imaging Component is not supported", res);

            // WIC is not supported
            g_bIsWicSupported = false;
        }
    }

    return g_pIWICImagingFactory;
}

void XWicHelpers::releaseImagingFactory()
{
    // release stream first if any
    if(g_pIWICStream)
    {
        g_pIWICStream->Release();
        g_pIWICStream = 0;
    }

    if(g_pIWICImagingFactory)
    {
        g_pIWICImagingFactory->Release();
        g_pIWICImagingFactory = 0;
    }
}

/////////////////////////////////////////////////////////////////////
// load images
IWICBitmapDecoder* XWicHelpers::decoderFromFile(const wchar_t* szFileName, IWICImagingFactory* pImagingFactory)
{
    // set factory if not provided
    if(pImagingFactory == 0) 
    {
        pImagingFactory = getImagingFactory();
        if(pImagingFactory == 0) return 0;
    }

    IWICBitmapDecoder* pRetDecoder = 0;

    // load image from file
    HRESULT res = pImagingFactory->CreateDecoderFromFilename(szFileName, NULL, 
        GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pRetDecoder);

    // check results
    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XWicHelpers: Failed to load image from file", res);
    }

    return pRetDecoder;
}

/////////////////////////////////////////////////////////////////////
// NOTE: this code is inspired by MSDN example
//       http://msdn.microsoft.com/en-us/library/windows/desktop/dd756685(v=vs.85).aspx
/////////////////////////////////////////////////////////////////////
IWICBitmapDecoder* XWicHelpers::decoderFromResource(HMODULE hModule, const wchar_t* szResName, const wchar_t* szResType, 
                                                    IWICImagingFactory* pImagingFactory)
{
    // release previous stream first if any
    if(g_pIWICStream)
    {
        g_pIWICStream->Release();
        g_pIWICStream = 0;
    }

    // set factory if not provided
    if(pImagingFactory == 0) 
    {
        pImagingFactory = getImagingFactory();
        if(pImagingFactory == 0) return 0;
    }

    // locate resource first
    HRSRC imageResHandle = ::FindResourceW(hModule, szResName, szResType);
    if(imageResHandle == NULL)
    {
        XWTRACE("XWicHelpers: Failed to locate image resource");
        return 0;
    }

    // calculate resource size
    int nImageDataSize = ::SizeofResource(hModule, imageResHandle);
    if(nImageDataSize == 0)
    {
        XWTRACE("XWicHelpers: Failed to get image resource size");
        return 0;
    }

    // load resource
    HGLOBAL imageResDataHandle = ::LoadResource(hModule, imageResHandle);
    if(imageResDataHandle == NULL)
    {
        XWTRACE("XWicHelpers: Failed to load image resource");
        return 0;
    }

    // lock resource
    void* pImageData = ::LockResource(imageResDataHandle);
    if(pImageData == NULL)
    {
        XWTRACE("XWicHelpers: Failed to lock image resource");
        return 0;
    }

    // create WIC stream from resource
    HRESULT res = pImagingFactory->CreateStream(&g_pIWICStream);
    if(FAILED(res))
    {
        XWTRACE_HRES("XWicHelpers: Failed to create stream", res);
        return 0;
    }

    // init stream
    res = g_pIWICStream->InitializeFromMemory((BYTE*)pImageData, nImageDataSize);
    if(FAILED(res))
    {
        XWTRACE_HRES("XWicHelpers: Failed to init stream", res);
        return 0;
    }
    
    IWICBitmapDecoder* pRetDecoder = 0;

    // create a decoder from stream
    res = pImagingFactory->CreateDecoderFromStream(g_pIWICStream, NULL, WICDecodeMetadataCacheOnDemand, &pRetDecoder);
    if(FAILED(res))
    {
        XWTRACE_HRES("XWicHelpers: Failed to create decoder from stream", res);
        return 0;
    }

    return pRetDecoder;
}

IWICBitmapDecoder* XWicHelpers::decoderFromBuffer(const unsigned char* memoryBuffer, ULONG bufferSize, 
                                        IWICImagingFactory* pImagingFactory)
{
    // set factory if not provided
    if(pImagingFactory == 0) 
    {
        pImagingFactory = getImagingFactory();
        if(pImagingFactory == 0) return 0;
    }

    // create in memory IStream
    XOleMemoryStream* stream = new XOleMemoryStream;
    stream->AddRef();

    // init from buffer
    if(!stream->initReadOnly(memoryBuffer, bufferSize))
    {
        XWTRACE("XWicHelpers: failed to create IStream from memory buffer");

        stream->Release();
        return 0;
    }

    IWICBitmapDecoder* pRetDecoder = 0;

    // load image from stream
    HRESULT res = pImagingFactory->CreateDecoderFromStream(stream, NULL, 
        WICDecodeMetadataCacheOnDemand, &pRetDecoder);

    // release stream
    stream->Release();

    // check results
    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XWicHelpers: Failed to load image from memory buffer", res);
    }

    return pRetDecoder;
}

/////////////////////////////////////////////////////////////////////
// bitmaps
/////////////////////////////////////////////////////////////////////
IWICBitmap* XWicHelpers::createBitmap(IWICBitmapSource* pBitmapSource, WICBitmapCreateCacheOption options,
                                      IWICImagingFactory* pImagingFactory)
{
    // check input
    XWASSERT(pBitmapSource);
    if(pBitmapSource == 0) return 0;

    // set factory if not provided
    if(pImagingFactory == 0) 
    {
        pImagingFactory = getImagingFactory();
        if(pImagingFactory == 0) return 0;
    }

    IWICBitmap* pBitmap = 0;

    // create bitmap
    HRESULT res = pImagingFactory->CreateBitmapFromSource(pBitmapSource, options, &pBitmap);
    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XWicHelpers: Failed to create bitmap from IWICBitmapSource", res);
        return 0;
    }

    // return bitmap
    return pBitmap;
}

IWICBitmap* XWicHelpers::createBitmap(HBITMAP hBitmap, HPALETTE hPalette, WICBitmapAlphaChannelOption options,
                                     IWICImagingFactory* pImagingFactory)
{
    // check input
    XWASSERT(hBitmap);
    if(hBitmap == 0) return 0;

    // set factory if not provided
    if(pImagingFactory == 0) 
    {
        pImagingFactory = getImagingFactory();
        if(pImagingFactory == 0) return 0;
    }

    IWICBitmap* pBitmap = 0;

    // create bitmap
    HRESULT res = pImagingFactory->CreateBitmapFromHBITMAP(hBitmap, hPalette, options, &pBitmap);
    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XWicHelpers: Failed to create bitmap from HBITMAP", res);
        return 0;
    }

    // return bitmap
    return pBitmap;
}

/////////////////////////////////////////////////////////////////////
// image scaling
/////////////////////////////////////////////////////////////////////
IWICBitmapSource* XWicHelpers::scaleImageSource(IWICBitmapSource* pBitmapOriginal, int width, int height,
                                          WICBitmapInterpolationMode mode)
{
    IWICBitmapScaler* pScaler = 0;

    // create bitmap scaler
    HRESULT res = XWicHelpers::getImagingFactory()->CreateBitmapScaler(&pScaler);
    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XWicHelpers: Failed to create image scaler", res);
        return 0;
    }

    // initialize scaler
    res = pScaler->Initialize(pBitmapOriginal, width, height, mode);

    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XWicHelpers: Failed to initialize image scaler", res);
        pScaler->Release();
        return 0;
    }

    return pScaler;
}

/////////////////////////////////////////////////////////////////////
// image clipping
/////////////////////////////////////////////////////////////////////
IWICBitmapSource* XWicHelpers::clipImageSource(IWICBitmapSource* pBitmapOriginal, int posX, int posY, 
                                          int width, int height)
{
    IWICBitmapClipper* pCliper = 0;

    // create bitmap cliper
    HRESULT res = XWicHelpers::getImagingFactory()->CreateBitmapClipper(&pCliper);
    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XWicHelpers: Failed to create image cliper", res);
        return 0;
    }

    // init rect
    WICRect rect;
    rect.X = posX;
    rect.Y = posY;
    rect.Width = width;
    rect.Height = height;

    // initialize cliper
    res = pCliper->Initialize(pBitmapOriginal, &rect);

    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XWicHelpers: Failed to initialize image cliper", res);
        pCliper->Release();
        return 0;
    }

    return pCliper;
}

/////////////////////////////////////////////////////////////////////
// image flip and rotation
/////////////////////////////////////////////////////////////////////
IWICBitmapSource* XWicHelpers::flipRotateImageSource(IWICBitmapSource* pBitmapOriginal, WICBitmapTransformOptions options)
{
    IWICBitmapFlipRotator* pFlipRotator = 0;

    // create bitmap "fliprotator"
    HRESULT res = XWicHelpers::getImagingFactory()->CreateBitmapFlipRotator(&pFlipRotator);
    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XWicHelpers: Failed to create image fliprotator", res);
        return 0;
    }

    // initialize "fliprotator"
    res = pFlipRotator->Initialize(pBitmapOriginal, options);

    if(FAILED(res))
    {
        // trace error
        XWTRACE_HRES("XWicHelpers: Failed to initialize image fliprotator", res);
        pFlipRotator->Release();
        return 0;
    }

    return pFlipRotator;
}

/////////////////////////////////////////////////////////////////////
// image properties
/////////////////////////////////////////////////////////////////////
bool XWicHelpers::getImageSize(IWICBitmap* wicBitmap, int& width, int& height)
{
    XWASSERT(wicBitmap);
    if(wicBitmap == 0) return false;

    UINT uWidth, uHeight;

    // get size
    HRESULT hr = wicBitmap->GetSize(&uWidth, &uHeight);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XWicHelpers::getImageSize failed", hr);
        return false;
    }

    // copy size
    width = uWidth;
    height = uHeight;

    return true;
}

// XWicHelpers
/////////////////////////////////////////////////////////////////////
