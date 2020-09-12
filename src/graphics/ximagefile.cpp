// Image file functionality
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xwichelpers.h"
#include "xgdihelpres.h"

#include "ximagefile.h"

/////////////////////////////////////////////////////////////////////
// NOTE: Native Image Format Metadata Queries:
//       https://msdn.microsoft.com/en-us/library/windows/desktop/ee719904(v=vs.85).aspx

// NOTE: WIC GUIDs and CLSIDs
//       https://msdn.microsoft.com/en-us/library/windows/desktop/ee719882(v=vs.85).aspx

/////////////////////////////////////////////////////////////////////
// constants

// metadata names
#define XIMAGEFILE_META_COLOR_TABLE_FLAG    L"/logscrdesc/GlobalColorTableFlag"
#define XIMAGEFILE_META_BGCOLOR_INDEX       L"/logscrdesc/BackgroundColorIndex"

// frame metadata names
#define XIMAGEFILE_META_FRAME_LEFT          L"/imgdesc/Left"
#define XIMAGEFILE_META_FRAME_TOP           L"/imgdesc/Top"
#define XIMAGEFILE_META_FRAME_WIDTH         L"/imgdesc/Width"
#define XIMAGEFILE_META_FRAME_HEIGHT        L"/imgdesc/Height"
#define XIMAGEFILE_META_FRAME_DELAY         L"/grctlext/Delay"
#define XIMAGEFILE_META_FRAME_DISPOSAL      L"/grctlext/Disposal"

/////////////////////////////////////////////////////////////////////
// XImageFile - image file functionality

XImageFile::XImageFile() :
    m_imageDecoder(0),
    m_imageFrame(0),
    m_frameDecode(0),
    m_frameIndex(0)
{
}

XImageFile::XImageFile(const wchar_t* szImageFilePath) :
    m_imageDecoder(0),
    m_imageFrame(0),
    m_frameDecode(0),
    m_frameIndex(0)
{
    // open file
    openFile(szImageFilePath);
}

XImageFile::~XImageFile()
{
    // close resources if any
    close();
}

/////////////////////////////////////////////////////////////////////
// read 
/////////////////////////////////////////////////////////////////////
bool XImageFile::openFile(const wchar_t* szImageFilePath)
{
    XWASSERT(szImageFilePath);
    if(szImageFilePath == 0) return false;

    // close resources if any
    close();

    // check if WIC is supported
    if(!XWicHelpers::isSupported())
    {
        XWTRACE("XImageFile: failed to open image file as WIC is not supported");
        return false;
    }

    // copy path
    m_strPath = szImageFilePath;

    // create decoder 
    m_imageDecoder = XWicHelpers::decoderFromFile(szImageFilePath);
    
    // select first frame by default
    setActiveFrame(0);

    // check result
    return (m_imageDecoder != 0);
}

bool XImageFile::openResource(HMODULE hModule, const wchar_t* szResName, const wchar_t* szResType)
{
    XWASSERT(szResName);
    if(szResName == 0) return false;

    // close resources if any
    close();

    // check if WIC is supported
    if(!XWicHelpers::isSupported())
    {
        XWTRACE("XImageFile: failed to open image file as WIC is not supported");
        return false;
    }

    // use current module if not set
    if(hModule == 0)
    {
        hModule = ::GetModuleHandleW(0);
    }

    // create decoder 
    m_imageDecoder = XWicHelpers::decoderFromResource(hModule, szResName, szResType);

    // select first frame by default
    setActiveFrame(0);

    // check result
    return (m_imageDecoder != 0);
}

bool XImageFile::openStyle(const wchar_t* szImageStylePath)
{
    XWASSERT(szImageStylePath);
    if(szImageStylePath == 0) return false;

    // close resources if any
    close();

    // check if WIC is supported
    if(!XWicHelpers::isSupported())
    {
        XWTRACE("XImageFile: failed to open image file as WIC is not supported");
        return false;
    }

    const unsigned char* bitmapData;
    size_t  dataSize;

    // get bitmap buffer from style
    if(!XWUIStyle::getStyleBitmap(szImageStylePath, bitmapData, dataSize))
    {
        XWTRACE("XImageFile: style image not found");
        return false;
    }

    // create decoder 
    m_imageDecoder = XWicHelpers::decoderFromBuffer(bitmapData, (ULONG)dataSize);
    
    // select first frame by default
    setActiveFrame(0);

    // check result
    return (m_imageDecoder != 0);
}

bool XImageFile::open(const XMediaSource& source)
{
    // check type
    if(source.type() == eXMediaSourceResource)
        return openResource(source.resModule(), source.resName(), source.resType());
    else if(source.type() == eXMediaSourceStyle)
        return openStyle(source.path());
    else
        return openFile(source.path());
}

void XImageFile::close()
{
    // active frame
    if(m_imageFrame) m_imageFrame->Release();
    m_imageFrame = 0;

    // frame decoder
    if(m_frameDecode) m_frameDecode->Release();
    m_frameDecode = 0;

    // decoder
    if(m_imageDecoder) m_imageDecoder->Release();
    m_imageDecoder = 0;

    // reset path
    m_strPath.clear();
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
int XImageFile::width() const
{
    // check if there is active frame
    if(m_imageFrame)
    {
        UINT uWidth = 0;
        UINT uHeight = 0;

        // get size from image frame
        HRESULT res = m_imageFrame->GetSize(&uWidth, &uHeight);
        if(FAILED(res))
        {
            XWTRACE_HRES("XImageFile: Failed to get size from image frame", res);
            return 0;
        }

        return uWidth;
    }

    return 0;
}

int XImageFile::height() const
{
    // check if there is active frame
    if(m_imageFrame)
    {
        UINT uWidth = 0;
        UINT uHeight = 0;

        // get size from image frame
        HRESULT res = m_imageFrame->GetSize(&uWidth, &uHeight);
        if(FAILED(res))
        {
            XWTRACE_HRES("XImageFile: Failed to get size from image frame", res);
            return 0;
        }

        return uHeight;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////
// frames
/////////////////////////////////////////////////////////////////////
bool XImageFile::setActiveFrame(int nFrameIndex)
{
    // ignore if no decode active
    if(m_imageDecoder == 0) return false;

    // check if there are enough frames
    if(nFrameIndex < frameCount())
    {
        // release frame decoder if any
        if(m_frameDecode) m_frameDecode->Release();
        m_frameDecode = 0;

        // get frame decoder
        HRESULT res = m_imageDecoder->GetFrame(nFrameIndex, &m_frameDecode);
        if(FAILED(res))
        {
            // trace error
            XWTRACE_HRES("XImageFile: Failed to get frame decoder from image file", res);
            return false;
        }

        // release old frame
        if(m_imageFrame) m_imageFrame->Release();
        m_imageFrame = 0;

        // set active frame
        m_imageFrame = m_frameDecode;
        m_imageFrame->AddRef();

        // copy index
        m_frameIndex = nFrameIndex;

        // new frame active
        return true;
    }

    return false;
}

int XImageFile::activeFrameIndex() const
{
    return m_frameIndex;
}

int XImageFile::frameCount() const
{
    if(m_imageDecoder)
    {
        UINT nFrameCount = 0;

        // get number of frames from decoder
        HRESULT res = m_imageDecoder->GetFrameCount(&nFrameCount);
        if(FAILED(res))
        {
            XWTRACE_HRES("XImageFile: Failed to get frame count from image file", res);
            return 0;
        }

        return nFrameCount;
    }

    // image is not loaded, no frames
    return 0;
}

/////////////////////////////////////////////////////////////////////
// animation
/////////////////////////////////////////////////////////////////////
bool XImageFile::hasAnimation()
{
    bool result = false;

    if(m_imageDecoder)
    {
        IWICBitmapDecoderInfo* decoderInfo = 0;

        // query decoder info
        HRESULT hr = m_imageDecoder->GetDecoderInfo(&decoderInfo);
        if(SUCCEEDED(hr) && decoderInfo)
        {
            BOOL isSupported = FALSE;

            // check if animation supported
            hr = decoderInfo->DoesSupportAnimation(&isSupported);
            if(SUCCEEDED(hr))
            {
                // get result
                result = (isSupported == TRUE);

            } else
            {
                XWTRACE_HRES("XImageFile: Failed to get animation support from decoder", hr);
            }

            // release
            decoderInfo->Release();

        } else
        {
            XWTRACE_HRES("XImageFile: Failed to get decode information", hr);
        }
    }

    return result;
}

bool XImageFile::getFramePlacement(int& leftOut, int& topOut, int& widthOut, int& heightOut)
{
    bool result = false;

    // reset output
    leftOut = 0;
    topOut = 0;
    widthOut = 0;
    heightOut = 0;

    if(m_frameDecode)
    {
        IWICMetadataQueryReader* metaReader = 0;

        // get metadata reader
        HRESULT hr = m_frameDecode->GetMetadataQueryReader(&metaReader);
        if(SUCCEEDED(hr))
        {
            // read values
            result = _queryMetadataValueInt(metaReader, XIMAGEFILE_META_FRAME_LEFT, leftOut);
            result |= _queryMetadataValueInt(metaReader, XIMAGEFILE_META_FRAME_TOP, topOut);
            result |= _queryMetadataValueInt(metaReader, XIMAGEFILE_META_FRAME_WIDTH, widthOut);
            result |= _queryMetadataValueInt(metaReader, XIMAGEFILE_META_FRAME_HEIGHT, heightOut);

            // release reader
            metaReader->Release();

        } else
        {
            XWTRACE_HRES("XImageFile: Failed to query frame metadata reader", hr);
        }
    }

    return result;
}

bool XImageFile::getFrameDelay(int& delayOut)
{
    bool result = false;

    // reset output
    delayOut = 0;

    if(m_frameDecode)
    {
        IWICMetadataQueryReader* metaReader = 0;

        // get metadata reader
        HRESULT hr = m_frameDecode->GetMetadataQueryReader(&metaReader);
        if(SUCCEEDED(hr))
        {
            // read delay
            result = _queryMetadataValueInt(metaReader, XIMAGEFILE_META_FRAME_DELAY, delayOut);
            if(result)
            {
                // convert from 10ms unit to 1ms
                delayOut *= 10;
            }

            // release reader
            metaReader->Release();

        } else
        {
            XWTRACE_HRES("XImageFile: Failed to query frame metadata reader", hr);
        }
    }

    return result;
}

bool XImageFile::getFrameDisposeMethod(TDisposeMethod& methodOut)
{
    bool result = false;

    // reset output
    methodOut = eDisposeUnknown;

    if(m_frameDecode)
    {
        IWICMetadataQueryReader* metaReader = 0;

        // get metadata reader
        HRESULT hr = m_frameDecode->GetMetadataQueryReader(&metaReader);
        if(SUCCEEDED(hr))
        {
            // read method
            int intValue = 0;
            if(_queryMetadataValueInt(metaReader, XIMAGEFILE_META_FRAME_DISPOSAL, intValue))
            {
                methodOut = (TDisposeMethod)intValue;
                result = true;
            }

            // release reader
            metaReader->Release();

        } else
        {
            XWTRACE_HRES("XImageFile: Failed to query frame metadata reader", hr);
        }
    }

    return result;
}

bool XImageFile::getBackgroundColor(COLORREF& colorOut, BYTE& alphaOut)
{
    bool result = false;

    // reset output
    colorOut = RGB(0, 0, 0);
    alphaOut = 255;

    if(m_imageDecoder)
    {
        IWICMetadataQueryReader* metaReader = 0;

        // get metadata reader
        HRESULT hr = m_imageDecoder->GetMetadataQueryReader(&metaReader);
        if(SUCCEEDED(hr))
        {
            int tableFlag = 0;
            int colorIndex = 0;
            IWICPalette* wicPalette = 0;
            std::vector<WICColor> wicColors;
            UINT colorsCount = 0;
            UINT colorsCopied = 0;

            // global color table flag
            result = _queryMetadataValueInt(metaReader, XIMAGEFILE_META_COLOR_TABLE_FLAG, tableFlag);

            // flag must be set
            if(tableFlag == 0) result = false;

            // color index
            if(result)
                result = _queryMetadataValueInt(metaReader, XIMAGEFILE_META_BGCOLOR_INDEX, colorIndex);

            // create palette
            if(result)
                hr = XWicHelpers::getImagingFactory()->CreatePalette(&wicPalette);

            if(result && FAILED(hr))
            {
                XWTRACE_HRES("XImageFile: Failed to create palette", hr);
                result = false;
            }

            // copy from file
            if(result)
                hr = m_imageDecoder->CopyPalette(wicPalette);

            if(result && FAILED(hr))
            {
                XWTRACE_HRES("XImageFile: Failed to copy palette", hr);
                result = false;
            }

            // copy colors
            if(result)
            {
                hr = wicPalette->GetColorCount(&colorsCount);
                if(SUCCEEDED(hr))
                {
                    wicColors.resize(colorsCount);
                    hr = wicPalette->GetColors(colorsCount, wicColors.data(), &colorsCopied);
                    if(SUCCEEDED(hr))
                    {
                        wicColors.resize(colorsCopied);
                    }
                }
            }

            if(result && FAILED(hr))
            {
                XWTRACE_HRES("XImageFile: Failed to copy colors from palette", hr);
                result = false;
            }

            // get color
            if(result)
            {
                // check if index is in range
                if(colorIndex >= 0 && colorIndex < (int)wicColors.size())
                {
                    // get color in ARGB format
                    WICColor wicColor = wicColors.at(colorIndex);

                    // get alpha
                    alphaOut = (BYTE)(wicColor >> 24);

                    // convert to COLORREF
                    colorOut = RGB(((wicColor & 0x00FF0000) >> 16), ((wicColor & 0x0000FF00) >> 8), (wicColor & 0x000000FF));

                } else
                {
                    XWTRACE("XImageFile: color index is out of color table range");
                    result = false;
                }
            }

            // release 
            if(wicPalette) wicPalette->Release();
            metaReader->Release();

        } else
        {
            XWTRACE_HRES("XImageFile: Failed to query frame metadata reader", hr);
        }
    }

    return result;
}

/////////////////////////////////////////////////////////////////////
// save
/////////////////////////////////////////////////////////////////////
bool XImageFile::create(int width, int height)
{
    // TODO:
    return false;
}

bool XImageFile::addFrame(HBITMAP hBitmap, int nFrameIndex)
{
    // TODO:
    return false;
}

bool XImageFile::addFrame(ID2DBitmap* pBitmap, int nFrameIndex)
{
    // TODO:
    return false;
}

bool XImageFile::save(const wchar_t* szImageFilePath, const char* format)
{
    // TODO:
    return false;
}

/////////////////////////////////////////////////////////////////////
// basic image manipulations
/////////////////////////////////////////////////////////////////////
bool XImageFile::scale(int width, int height)
{
    // ignore if active frame is not set
    if(m_imageFrame == 0) return false;

    // get size from image frame
    UINT uWidth = 0;
    UINT uHeight = 0;
    HRESULT res = m_imageFrame->GetSize(&uWidth, &uHeight);
    if(SUCCEEDED(res))
    {
        // ignore if already the same size
        if(uWidth == width && uHeight == height) return true;
    }

    // create scaler
    IWICBitmapSource* pScaledBitmap = XWicHelpers::scaleImageSource(m_imageFrame, width, height);

    // check result
    if(pScaledBitmap)
    {
        // release active source
        m_imageFrame->Release();
        m_imageFrame = pScaledBitmap;

        return true;
    }

    return false;
}

bool XImageFile::clip(int posX, int posY, int width, int height)
{
    // ignore if active frame is not set
    if(m_imageFrame == 0) return false;

    // create scaler
    IWICBitmapSource* pClippedBitmap = XWicHelpers::clipImageSource(m_imageFrame, posX, posY, width, height);

    // check result
    if(pClippedBitmap)
    {
        // release active source
        m_imageFrame->Release();
        m_imageFrame = pClippedBitmap;

        return true;
    }

    return false;
}

bool XImageFile::rotate(int angle)
{
    // ignore if active frame is not set
    if(m_imageFrame == 0) return false;

    WICBitmapTransformOptions opt;

    // set options
    switch(angle)
    {
    case 0:
        opt = WICBitmapTransformRotate0;
        break;
    case 90:
        opt = WICBitmapTransformRotate90;
        break;
    case 180:
        opt = WICBitmapTransformRotate180;
        break;
    case 270:
        opt = WICBitmapTransformRotate270;
        break;

    default:
        // not supported angle
        XWTRACE1("XImageFile: %d is not sipported angle, only 0, 90, 180 and 270 are supproted", angle);
        return false;
        break;
    }

    // create rotator
    IWICBitmapSource* pRotatedBitmap = XWicHelpers::flipRotateImageSource(m_imageFrame, opt);

    // check result
    if(pRotatedBitmap)
    {
        // release active source
        m_imageFrame->Release();
        m_imageFrame = pRotatedBitmap;

        return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////
// image data
/////////////////////////////////////////////////////////////////////
IWICBitmap* XImageFile::createWicBitmap() const
{
    // ignore if active frame is not set
    if(m_imageFrame == 0) return false;

    // create bitmap and load its content to memory
    return XWicHelpers::createBitmap(m_imageFrame, WICBitmapCacheOnLoad);
}

HBITMAP XImageFile::createGdiBitmap(HDC hdc, bool premultiply) const
{
    // ignore if active frame is not set
    if(m_imageFrame == 0) return false;

    // create HBITMAP
    return XGdiHelpers::createBitmap(hdc, m_imageFrame, premultiply);
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
bool XImageFile::_queryMetadataValueInt(IWICMetadataQueryReader* reader, const WCHAR* name, int& valueOut)
{
    bool result = false;

    PROPVARIANT propValue;
    ::PropVariantInit(&propValue);
    
    // reset output
    valueOut = 0;

    // read value
    HRESULT hr = reader->GetMetadataByName(name,  &propValue);
    if(SUCCEEDED(hr))
    {
        result = true;

        // convert to int
        switch(propValue.vt)
        {
        case VT_I1:
            valueOut = propValue.cVal;
            break;
        case VT_I2:
            valueOut = propValue.iVal;
            break;
        case VT_I4:
            valueOut = propValue.lVal;
            break;
        case VT_I8:
            valueOut = (int)propValue.hVal.QuadPart;
            break;
        case VT_UI1:
            valueOut = propValue.bVal;
            break;
        case VT_UI2:
            valueOut = propValue.uiVal;
            break;
        case VT_UI4:
            valueOut = propValue.ulVal;
            break;
        case VT_UI8:
            valueOut = (int)propValue.uhVal.QuadPart;
            break;
        case VT_INT:
            valueOut = propValue.intVal;
            break;
        case VT_UINT:
            valueOut = propValue.uintVal;
            break;
        case VT_BOOL:
            valueOut = (propValue.boolVal != VARIANT_FALSE) ? 1 : 0; 
            break;

        default:
            XWTRACE("XImageFile: unknown variant type while reading metadata integer value");
            result = false;
            break;
        }
    }

    ::PropVariantClear(&propValue);

    return result;
}

// XImageFile
/////////////////////////////////////////////////////////////////////
