// Image functionality
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xwichelpers.h"
#include "xd2dhelpres.h"
#include "xgdihelpres.h"
#include "ximagefilehelpers.h"
#include "xgdiresourcescache.h"
#include "xd2dresourcescache.h"
#include "ximagefile.h"
#include "ximage.h"

/////////////////////////////////////////////////////////////////////
// constants

// default image animation delay
#define XIMAGE_DEFAULT_ANIMATION_DELAY      90

/////////////////////////////////////////////////////////////////////
// XImage - basic image functionality

XImage::XImage() :
    m_bTransparency(255),
    m_bKeepAspectRatio(false),
    m_pWicBitmap(0),
    m_hGdiBitmap(0),
    m_hGdiLoadedBitmap(0),
    m_pXGdiResourcesCache(0),
    m_pXD2DResourcesCache(0),
    m_hasAnimation(false),
    m_hasBackground(false),
    m_animationActive(false),
    m_frameBackground(0),
    m_frameAlpha(0),
    m_gdiFrameBitmap(0),
    m_pD2DFrameBitmap(0),
    m_framePixels(0),
    m_defaultDelay(XIMAGE_DEFAULT_ANIMATION_DELAY),
    m_useFrameDelay(false),
    m_hGdiPaintBitmap(0),
    m_pD2DBitmap(0),
    m_width(0),
    m_height(0),
    m_originalWidth(0),
    m_originalHeight(0),
    m_fixedWidth(0),
    m_fixedHeight(0)
{
}

XImage::~XImage()
{
    // release image
    resetImage();

    // release caches
    if(m_pXGdiResourcesCache) m_pXGdiResourcesCache->Release();
    m_pXGdiResourcesCache = 0;
    if(m_pXD2DResourcesCache) m_pXD2DResourcesCache->Release();
    m_pXD2DResourcesCache = 0;
}
        
/////////////////////////////////////////////////////////////////////
// set image
/////////////////////////////////////////////////////////////////////
bool XImage::setImage(IWICBitmap* wicBitmap)
{
    XWASSERT(wicBitmap);
    if(wicBitmap == 0) return false;

    // release previous images if any
    resetImage();

    // copy WIC image
    m_pWicBitmap = wicBitmap;
    m_pWicBitmap->AddRef();

    // load image size
    if(!_loadImageSize())
    {
        XWTRACE("XImage::setImage(IWICBitmap) failed, image is not valid");
        
        // reset image data
        m_pWicBitmap->Release();
        m_pWicBitmap = 0;
    }

    return (m_pWicBitmap != 0);
}

bool XImage::setImage(HBITMAP hBitmap)
{
    XWASSERT(hBitmap);
    if(hBitmap == 0) return false;

    // release previous image if any
    resetImage();

    // copy image
    m_hGdiBitmap = hBitmap;

    // load image size
    if(!_loadImageSize())
    {
        XWTRACE("XImage::setImage(HBITMAP) failed, image is not valid");
        
        // reset image data
        m_hGdiBitmap = 0;
    }

    return (m_hGdiBitmap != 0);
}

bool XImage::setImagePath(const WCHAR* filePath)
{
    XWASSERT(filePath);
    if(filePath == 0) return false;

    // release previous image if any
    resetImage();

    // copy path
    m_imageSource.setFilePath(filePath);

    // load image size
    if(!_loadImageSize())
    {
        XWTRACE("XImage::setImage(FILENAME) failed, image is not valid");
        
        // reset image path
        m_imageSource.reset();
    }

    return m_imageSource.isSet();
}

bool XImage::setImageResourcePath(HMODULE hModule, const WCHAR* szResName, const WCHAR* szResType)
{
    XWASSERT(szResName);
    if(szResName == 0) return false;

    // release previous image if any
    resetImage();

    // copy path
    m_imageSource.setResource(hModule, szResName, szResType);

    // load image size
    if(!_loadImageSize())
    {
        XWTRACE("XImage::setImage(RESOURCE) failed, image is not valid");
        
        // reset image path
        m_imageSource.reset();
    }

    return m_imageSource.isSet();
}

bool XImage::setImageStylePath(const WCHAR* stylePath)
{
    XWASSERT(stylePath);
    if(stylePath == 0) return false;

    // release previous image if any
    resetImage();

    // copy path
    m_imageSource.setStylePath(stylePath);

    // load image size
    if(!_loadImageSize())
    {
        XWTRACE("XImage::setImageStyle(STYLE) failed, image is not valid");
        
        // reset image path
        m_imageSource.reset();
    }

    return m_imageSource.isSet();
}

bool XImage::setImageSource(const XMediaSource& source)
{
    // release previous image if any
    resetImage();

    // copy source
    m_imageSource = source;

    // load image size
    if(!_loadImageSize())
    {
        XWTRACE("XImage::setImageSource failed, image is not valid");
        
        // reset image path
        m_imageSource.reset();
    }

    return m_imageSource.isSet();
}

void XImage::resetImage()
{
    // release images
    _resetPaintBitmaps();

    // reset input bitmap if any
    m_hGdiBitmap = 0;

    // release WIC bitmap
    if(m_pWicBitmap) m_pWicBitmap->Release();
    m_pWicBitmap = 0;

    // reset path
    m_imageSource.reset();

    // reset animation
    _resetAnimation();

    // reset state
    m_hasAnimation = false;
    m_width = 0;
    m_height = 0;
    m_originalWidth = 0;
    m_originalHeight = 0;
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
bool XImage::isImageSet() const
{
    return (m_pWicBitmap != 0) ||
           (m_hGdiBitmap != 0) ||
           m_imageSource.isSet();
}

void XImage::setTransparency(BYTE transparency)
{
    // copy transparency value
    m_bTransparency = transparency;
}

void XImage::setImageSize(int width, int height, bool keepAspectRatio)
{
    // check input
    XWASSERT(width >= 0);
    XWASSERT(height >= 0);
    if(width < 0 || height < 0) return;

    // release images if any
    _resetPaintBitmaps();

    // stop animation if any
    endAnimation();

    // copy size
    m_fixedWidth = width;
    m_fixedHeight = height;

    // copy flag
    m_bKeepAspectRatio = keepAspectRatio;

    // update size if set
    if(isImageSet())
    {
        // copy fixed size
        m_width = m_fixedWidth;
        m_height = m_fixedHeight;

        // preserve aspect ratio if required
        if(m_bKeepAspectRatio)
            XImageFileHelpers::preserveAspectRatio(m_originalWidth, m_originalHeight, m_width, m_height);
    }
}

/////////////////////////////////////////////////////////////////////
// animation
/////////////////////////////////////////////////////////////////////
void XImage::setDefaultDelay(int delay)
{
    // copy default delay
    m_defaultDelay = delay;
}

bool XImage::beginAnimation(HDC hdc)
{
    // ignore if no animation
    if(!m_hasAnimation || !m_imageSource.isSet()) return false;

    // ignore if already active
    if(m_animationActive) return true;

    // open image animation file
    if(!m_inputFile.open(m_imageSource))
    {
        XWTRACE("XImage: failed to start animation, cannot open image file");
        return false;
    }

    // read color information
    m_hasBackground = m_inputFile.getBackgroundColor(m_frameBackground, m_frameAlpha);

    // set state
    m_animationActive = true;
    m_useFrameDelay = false;

    // release paint images
    _resetPaintBitmaps();

    // create frame bitmap if needed
    if(m_gdiFrameBitmap == 0)
    {
        // create 
        if(!_createFrameBitmap(hdc, m_gdiFrameBitmap, &m_framePixels)) return false;
    }

    // load frame pixels
    return _copyFramePixels();
}

void XImage::endAnimation()
{
    // ignore if no animation
    if(!m_hasAnimation) return;

    // reset animation state
    _resetAnimation();
}

bool XImage::getFrameDelay(int& delayOut)
{
    // get from file
    if(!m_inputFile.getFrameDelay(delayOut))
    {
        // use default
        delayOut = m_defaultDelay;
    }

    // NOTE: sometimes frame delay is missing from file, in this case
    //       we use default animation delay (same as most of the browsers 
    //       do). But in the case when frame delay is set for some frame 
    //       and not for the others we use zero delay anyway.
    if(delayOut != 0)
        m_useFrameDelay = false;

    if(delayOut == 0 && !m_useFrameDelay)
        delayOut = m_defaultDelay;

    return true;
}

bool XImage::loadNextFrame(HDC hdc)
{
    // ignore if no animation
    if(!m_animationActive) return false;

    // release paint images
    _resetPaintBitmaps();

    // get frame count
    if(m_inputFile.activeFrameIndex() + 1 < m_inputFile.frameCount() && m_gdiFrameBitmap != 0)
    {
        // next frame
        if(!m_inputFile.setActiveFrame(m_inputFile.activeFrameIndex() + 1)) return false;

    } else
    {
        // load first frame
        if(!m_inputFile.setActiveFrame(0)) return false;
    }

    // create frame bitmap if needed
    if(m_gdiFrameBitmap == 0)
    {
        // create 
        if(!_createFrameBitmap(hdc, m_gdiFrameBitmap, &m_framePixels)) return false;
    }

    // copy pixels
    return _copyFramePixels();
}

/////////////////////////////////////////////////////////////////////
// GDI painting 
/////////////////////////////////////////////////////////////////////
void XImage::onInitGDIResources(HDC hdc)
{
    // check if animation is ongoing
    if(m_animationActive)
        // load frame
        _loadGDIFrame(hdc);
    else
        // load image 
        _loadGdiImage(hdc);
}

void XImage::onResetGDIResources()
{
    // release image
    _releaseGdiImage();
}

void XImage::onPaintGDI(int originX, int originY, HDC hdc)
{
    XWASSERT(hdc);
    if(hdc == 0) return;

    // ignore if no bitmap to paint
    if(!isImageSet()) return;

    // check if image is ready
    if(m_hGdiPaintBitmap == 0) 
    {
        // try to init 
        onInitGDIResources(hdc);

        // double check
        XWASSERT(m_hGdiPaintBitmap);
        if(m_hGdiPaintBitmap == 0) return;
    }

    // create device context
    HDC hdcMem = 0;
    if(m_pXGdiResourcesCache)
        hdcMem = m_pXGdiResourcesCache->getCompatibleDC(hdc);
    else
        hdcMem = ::CreateCompatibleDC(hdc);

    if (hdcMem)
    {
        // select bitmap into the memory DC
        HGDIOBJ hbmOld = ::SelectObject(hdcMem, m_hGdiPaintBitmap);

        // NOTE: use AlphaBlend to preserve image alpha channel (we expect image to be in pre-multiplied format)
        BLENDFUNCTION bf;
        bf.BlendOp = AC_SRC_OVER;
        bf.BlendFlags = 0;
        bf.SourceConstantAlpha = m_bTransparency; // transparency value between 0-255
        bf.AlphaFormat = AC_SRC_ALPHA;  

        ::AlphaBlend(hdc, 
            originX, originY,
            m_width, m_height, 
            hdcMem,
            0, 0,
            m_width, m_height, 
            bf);
            
        // Restore the memory DC
        if(hbmOld)
            ::SelectObject(hdcMem, hbmOld);
    }

    // release DC
    if(m_pXGdiResourcesCache == 0)
        ::DeleteDC(hdcMem);
}

void XImage::setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache)
{
    // release image
    _releaseGdiImage();

    // release previous cache if any
    if(m_pXGdiResourcesCache) m_pXGdiResourcesCache->Release();

    // copy reference
    m_pXGdiResourcesCache = pXGdiResourcesCache;
    if(m_pXGdiResourcesCache) 
        m_pXGdiResourcesCache->AddRef();
}

/////////////////////////////////////////////////////////////////////
// Direct2D painting 
/////////////////////////////////////////////////////////////////////
void XImage::onInitD2DTarget(ID2D1RenderTarget* pTarget)
{
    // check if animation is ongoing
    if(m_animationActive)
        // load frame
        _loadD2DFrame(pTarget);
    else
        // load image
        _loadD2DImage(pTarget);
}

void XImage::onResetD2DTarget()
{
    // release image
    _releaseD2DImage();
}

void XImage::onPaintD2D(FLOAT originX, FLOAT originY, ID2D1RenderTarget* pTarget)
{
    XWASSERT(pTarget);
    if(pTarget == 0) return;

    // ignore if no bitmap to paint
    if(!isImageSet()) return;

    // check if image is ready
    if(m_pD2DBitmap == 0) 
    {
        // try to init 
        onInitD2DTarget(pTarget);

        // double check
        XWASSERT(m_pD2DBitmap);
        if(m_pD2DBitmap == 0) return;
    }

    // convert transparency
    FLOAT fTrans = (FLOAT)m_bTransparency / 255.0f;

    // paint rect
    D2D1_RECT_F drawRect;
    drawRect.left = originX;
    drawRect.top = originY;
    drawRect.right = drawRect.left + XD2DHelpers::pixelsToDipsX(m_width);
    drawRect.bottom = drawRect.top + XD2DHelpers::pixelsToDipsY(m_height);

    // no need to scale bitmap, D2D can handle this
    pTarget->DrawBitmap(m_pD2DBitmap, drawRect, fTrans);
}

void XImage::setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache)
{
    // release image
    _releaseD2DImage();

    // release previous cache if any
    if(m_pXD2DResourcesCache) m_pXD2DResourcesCache->Release();

    // copy reference
    m_pXD2DResourcesCache = pXD2DResourcesCache;
    if(m_pXD2DResourcesCache) 
        m_pXD2DResourcesCache->AddRef();
}

/////////////////////////////////////////////////////////////////////
// image data 
/////////////////////////////////////////////////////////////////////
HBITMAP XImage::getGDIPaintBitmap(HDC hdc)
{
    // try to load image first
    if(m_hGdiPaintBitmap == 0)
        onInitGDIResources(hdc);

    return m_hGdiPaintBitmap;
}

ID2D1Bitmap* XImage::getD2DPaintBitmap(ID2D1RenderTarget* pTarget)
{
    // try to load image first
    if(m_pD2DBitmap == 0)
        onInitD2DTarget(pTarget);

    return m_pD2DBitmap;
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
bool XImage::_isFixedSize()
{
    return (m_fixedWidth != 0 && m_fixedHeight != 0);
}

void XImage::_releaseGdiImage()
{
    // release cached image if any
    if(m_pXGdiResourcesCache)
        m_pXGdiResourcesCache->releaseBitmap(m_strImageHashGDI);

    // release loaded bitmap if any
    if(m_hGdiLoadedBitmap)
        ::DeleteObject(m_hGdiLoadedBitmap);

    // reset bitmaps
    m_hGdiLoadedBitmap = 0;
    m_hGdiPaintBitmap = 0;
}

void XImage::_loadGdiImage(HDC hdc)
{
    // release old image if any
    _releaseGdiImage();

    // ignore if no image set
    if(!isImageSet()) return;

    // check input
    XWASSERT(hdc);
    if(hdc == 0) return;

    // check if bitmap is set directly 
    if(m_hGdiBitmap) 
    {
        // get image size
        int imageWidth, imageHeight;
        if(!XGdiHelpers::getBitmapSize(m_hGdiBitmap, imageWidth, imageHeight))
        {
            imageWidth = 0;
            imageHeight = 0;
        }

        // check if scaling is needed
        if(_isFixedSize() && imageWidth != m_width && imageHeight != m_height)
        {
            // scale bitmap
            m_hGdiLoadedBitmap = XGdiHelpers::scaleBitmap(hdc, m_hGdiBitmap, m_width, m_height, true);
            if(m_hGdiLoadedBitmap == 0)
            {
                XWTRACE("XImage::_loadGdiImage failed to scale GDI bitmap");
                return;
            }

            // set reference
            m_hGdiPaintBitmap = m_hGdiLoadedBitmap;

        } else
        {
            // just set reference
            m_hGdiPaintBitmap = m_hGdiBitmap;
        }
    }

    // if WIC bitmap is set create bitmap from it
    if(m_pWicBitmap)
    {
        // check if scaling is needed
        if(_isFixedSize())
        {
            m_hGdiLoadedBitmap = XGdiHelpers::createBitmap(hdc, m_pWicBitmap, m_width, m_height, true);

        } else
        {
            m_hGdiLoadedBitmap = XGdiHelpers::createBitmap(hdc, m_pWicBitmap, true);
        }

        // check result
        if(m_hGdiLoadedBitmap == 0)
        {
            XWTRACE("XImage::_loadGdiImage failed to create HBITMAP from IWICBitmap");
            return;
        }

        // set reference
        m_hGdiPaintBitmap = m_hGdiLoadedBitmap;
    }

    // load image if path is set    
    if(m_imageSource.isSet())
    {
        // check if cache is set
        if(m_pXGdiResourcesCache)
        {
            // load from cache
            m_hGdiPaintBitmap = _loadCachedImageGDI();

        } else
        {
            // load from file
            m_hGdiPaintBitmap = _loadImageFromFileGDI(hdc);
        }

        if(m_hGdiPaintBitmap == 0)
        {
            XWTRACE("XImage::_loadGdiImage failed to load HBITMAP from file/resource");
            return;
        }
    }

    // update loaded image size just in case
    if(m_hGdiPaintBitmap)
    {
        if(!XGdiHelpers::getBitmapSize(m_hGdiPaintBitmap, m_width, m_height))
        {
            XWTRACE("XImage::_loadGdiImage failed to read size from loaded image");
        }
    }
}

void XImage::_releaseD2DImage()
{
    // release bitmap
    if(m_pD2DBitmap) m_pD2DBitmap->Release();
    m_pD2DBitmap = 0;

    // release cache if any
    if(m_pXD2DResourcesCache)
        m_pXD2DResourcesCache->releaseBitmap(m_strImageHashD2D);
}

bool XImage::_d2dBitmapFromWicBitmap(IWICBitmap* wicBitmap, ID2D1RenderTarget* pTarget)
{
    XWASSERT(wicBitmap);
    if(wicBitmap == 0) return false;

    // check if scaling is needed
    if(_isFixedSize())
    {
        // scale
        IWICBitmapSource* imgSource = XWicHelpers::scaleImageSource(wicBitmap, m_width, m_height);
        if(imgSource == 0) return false;

        // create D2D image from scaled WIC image
        m_pD2DBitmap = XD2DHelpers::createBitmap(imgSource, pTarget);

        // release scaled image
        imgSource->Release();

    } else
    {
        // create D2D image from WIC image
        m_pD2DBitmap = XD2DHelpers::createBitmap(wicBitmap, pTarget);
    }

    return (m_pD2DBitmap != 0);
}

void XImage::_loadD2DImage(ID2D1RenderTarget* pTarget)
{
    // release old if any
    _releaseD2DImage();

    // ignore if no image set
    if(!isImageSet()) return;

    // check input
    XWASSERT(pTarget);
    if(pTarget == 0) return;

    // check if we have WIC bitmap set
    if(m_pWicBitmap)
    {
        // create D2D bitmap
        if(!_d2dBitmapFromWicBitmap(m_pWicBitmap, pTarget))
        {
            XWTRACE("XImage::_loadD2DImage failed to create D2D bitmap from WIC bitmap");
            return;
        }

    } else if(m_hGdiBitmap)
    {
        // create temporary WIC bitmap first from GDI image
        IWICBitmap* wicBitmap = XWicHelpers::createBitmap(m_hGdiBitmap);
        if(!wicBitmap)
        {
            XWTRACE("XImage::_loadD2DImage failed to create WIC bitmap from GDI bitmap");
            return;
        }

        // create D2D bitmap
        if(!_d2dBitmapFromWicBitmap(wicBitmap, pTarget))
        {
            XWTRACE("XImage::_loadD2DImage failed to create D2D bitmap from GDI bitmap");
        }

        // release temporary bitmap
        wicBitmap->Release();
    }

    // load image if path is set
    if(m_imageSource.isSet())
    {
        // check if cache is set
        if(m_pXD2DResourcesCache)
        {
            // load from cache
            m_pD2DBitmap = _loadCachedImageD2D();

        } else
        {
            // load from file
            m_pD2DBitmap = _loadImageFromFileD2D(pTarget);
        }

        if(m_pD2DBitmap == 0)
        {
            XWTRACE("XImage::_loadD2DImage failed to load HBITMAP from file/resource");
            return;
        }
    }

    // update loaded image size just in case
    if(m_pD2DBitmap)
    {
        // get image size (might be different from fixed size in case of scaling)
        D2D1_SIZE_F imageSize = m_pD2DBitmap->GetSize();

        // convert to pixels
        m_width = XD2DHelpers::dipsToPixelsX(imageSize.width);
        m_height = XD2DHelpers::dipsToPixelsY(imageSize.height);
    }
}

bool XImage::_loadImageSize()
{
    // reset size
    m_originalWidth = 0;
    m_originalHeight = 0;
    m_width = 0;
    m_height = 0;

    // reset animation
    m_hasAnimation = false;

    // check image source
    if(m_pWicBitmap)
    {
        if(!XWicHelpers::getImageSize(m_pWicBitmap, m_originalWidth, m_originalHeight)) return false;

    } else if(m_hGdiBitmap)
    {
        if(!XGdiHelpers::getBitmapSize(m_hGdiBitmap, m_originalWidth, m_originalHeight)) return false;

    } else if(m_imageSource.isSet())
    {
        // open image file
        XImageFile file;
        if(!file.open(m_imageSource)) return false;

        // read size
        m_originalWidth = file.width();
        m_originalHeight = file.height();

        // animation
        m_hasAnimation = file.hasAnimation();

        // close file
        file.close();

    } else
    {
        // image not set
        return false;
    }

    // make sure size has been loaded
    if(m_originalWidth == 0 || m_originalHeight == 0) return false;

    // check if size is fixed
    if(_isFixedSize())
    {
        // copy fixed size
        m_width = m_fixedWidth;
        m_height = m_fixedHeight;

        // preserve aspect ratio if required
        if(m_bKeepAspectRatio)
            XImageFileHelpers::preserveAspectRatio(m_originalWidth, m_originalHeight, m_width, m_height);

    } else
    {
        // use image size
        m_width = m_originalWidth;
        m_height = m_originalHeight;
    }

    return true;
}

bool XImage::_openImageFromFile(XImageFile& file)
{
    // load from file
    if(!file.open(m_imageSource)) return false;

    // scale image if needed
    if(_isFixedSize())
    {
        if(!file.scale(m_width, m_height))
        {
            XWTRACE("XImage: failed to scale image");
        }
    }

    return true;
}

HBITMAP XImage::_loadImageFromFileGDI(HDC hdc)
{
    // open image file
    XImageFile file;
    if(!_openImageFromFile(file)) return 0;

    // create bitmap
    m_hGdiLoadedBitmap = file.createGdiBitmap(hdc, true);

    // close file
    file.close();

    return m_hGdiLoadedBitmap;
}

HBITMAP XImage::_loadCachedImageGDI()
{
    // cache must be set
    XWASSERT(m_pXGdiResourcesCache);
    if(m_pXGdiResourcesCache == 0) return 0;

    int width = _isFixedSize() ? m_width : 0;
    int height = _isFixedSize() ? m_height : 0;

    // load image
    if(!m_pXGdiResourcesCache->loadBitmap(m_imageSource, 
        width, height, m_strImageHashGDI)) return false;

    // get bitmap from cache
    return m_pXGdiResourcesCache->getBitmap(m_strImageHashGDI);
}

ID2D1Bitmap* XImage::_loadImageFromFileD2D(ID2D1RenderTarget* pTarget)
{
    // open image file
    XImageFile file;
    if(!_openImageFromFile(file)) return 0;

    // create bitmap
    m_pD2DBitmap = XD2DHelpers::createBitmap(&file, pTarget);

    // close file
    file.close();

    return m_pD2DBitmap;
}

ID2D1Bitmap* XImage::_loadCachedImageD2D()
{
    // cache must be set
    XWASSERT(m_pXD2DResourcesCache);
    if(m_pXD2DResourcesCache == 0) return 0;

    int width = _isFixedSize() ? m_width : 0;
    int height = _isFixedSize() ? m_height : 0;

    // load 
    if(!m_pXD2DResourcesCache->loadBitmap(m_imageSource, 
        width, height, m_strImageHashD2D)) return false;

    // get image from cache
    return m_pXD2DResourcesCache->getBitmap(m_strImageHashD2D);
}

bool XImage::_createFrameBitmap(HDC hdc, HBITMAP& bitmapOut, DWORD** pixelsOut)
{
    bitmapOut = XGdiHelpers::createBitmap(hdc, m_width, m_height, (void**)pixelsOut);
    if(bitmapOut == 0 || *pixelsOut == 0)
    {
        XWTRACE("XImage: failed to create frame bitmap");

        // delete bitmap if any
        if(bitmapOut) ::DeleteObject(bitmapOut);
        
        // stop animation
        _resetAnimation();

        return false;
    }

    return true;
}

void XImage::_resetPaintBitmaps()
{
    // release bitmaps if any
    _releaseGdiImage();
    _releaseD2DImage();
}

void XImage::_resetAnimationBitmaps()
{
    // reset paint bitmap
    if(m_hGdiPaintBitmap)
    {
        XWASSERT(m_gdiFrameBitmap == m_hGdiPaintBitmap);
        m_hGdiPaintBitmap = 0;
    }

    // GDI frame bitmap
    if(m_gdiFrameBitmap) ::DeleteObject(m_gdiFrameBitmap);
    m_gdiFrameBitmap = 0;
    m_framePixels = 0;

    // release D2D paint bitmap
    if(m_pD2DBitmap) m_pD2DBitmap->Release();
    m_pD2DBitmap = 0;

    // D2D frame bitmap
    if(m_pD2DFrameBitmap) m_pD2DFrameBitmap->Release();
    m_pD2DFrameBitmap = 0;
}

void XImage::_resetAnimation()
{
    // release bitmap
    _resetAnimationBitmaps();

    // reset state
    m_animationActive = false;
    m_hasBackground = false;
    m_frameBackground = 0;
    m_frameAlpha = 0;

    // close file
    m_inputFile.close();
}

void XImage::_getFramePlacement(IWICBitmapSource* frameBitmapSource, int& frameLeft, int& frameTop, int& frameWidth, int& frameHeight)
{
    if(m_inputFile.getFramePlacement(frameLeft, frameTop, frameWidth, frameHeight))
    {
        // scale frame placement if needed
        if(m_originalWidth != m_width && m_width != 0)
        {
            frameLeft = (int)(frameLeft * ((double)m_width/m_originalWidth));
            frameWidth = (int)(frameWidth * ((double)m_width/m_originalWidth));
        }

        if(m_originalHeight != m_height && m_height != 0)
        {
            frameTop = (int)(frameTop * ((double)m_height/m_originalHeight));
            frameHeight = (int)(frameHeight * ((double)m_height/m_originalHeight));
        }

    } else
    {
        // use default
        frameLeft = 0;
        frameTop = 0;
        frameWidth = m_width;
        frameHeight = m_height;

        // get frame size
        UINT uwidth, uheight;
        HRESULT hr = frameBitmapSource->GetSize(&uwidth, &uheight);
        if(SUCCEEDED(hr))
        {
            // update size
            frameWidth = uwidth;
            frameHeight = uheight;
        }
    }

    // size must not exceed (check just in case)
    if(frameWidth > m_width) frameWidth = m_width;
    if(frameLeft > m_width) frameLeft = m_width;
    if(frameHeight > m_height) frameHeight = m_height;
    if(frameTop > m_height) frameTop = m_height;
}

bool XImage::_copyFramePixels()
{
    // pixel buffer must be ready
    XWASSERT(m_framePixels);
    if(m_framePixels == 0) return false;

    // get dispose method
    XImageFile::TDisposeMethod method;
    if(!m_inputFile.getFrameDisposeMethod(method) || method == XImageFile::eDisposeUnknown)
    {
        // use default
        method = XImageFile::eDisposeNone;
    }

    // fill background if needed
    if(method == XImageFile::eDisposeBackground || (m_hasBackground && m_inputFile.activeFrameIndex() == 0))
    {
        if(m_hasBackground)
            // fill with frame colors
            XGdiHelpers::fillBitmapPixels(m_framePixels, (m_width * m_height), m_frameBackground, m_frameAlpha);
        else
            // fill with transparent white color
            XGdiHelpers::fillBitmapPixels(m_framePixels, (m_width * m_height), RGB(255, 255, 255), 0);
    } 

    // frame data
    IWICBitmapSource* frameBitmapSource = m_inputFile.frameBitmapSource();
    if(frameBitmapSource == 0)
    {
        XWTRACE("XImage: image file active frame not set");

        // stop animation
        _resetAnimation();
        return false;
    }

    // get frame placements
    int frameLeft, frameTop, frameWidth, frameHeight;
    _getFramePlacement(frameBitmapSource, frameLeft, frameTop, frameWidth, frameHeight);

    // check if we can simply read frame to buffer
    if(frameWidth == m_width && frameHeight == m_height && method == XImageFile::eDisposeNone && !m_hasBackground)
    {
        // copy pixels from frame directly
        if(!XGdiHelpers::copyPixelsFromBitmap(m_framePixels, m_width, m_height, frameBitmapSource, true))
        {
            XWTRACE("XImage: failed to read image frame");

            // stop animation
            _resetAnimation();
            return false;
        }

    } else
    {
        // reserve space
        m_pixelBuffer.resize(frameWidth*frameHeight);

        // copy pixels to buffer first
        if(!XGdiHelpers::copyPixelsFromBitmap(m_pixelBuffer.data(), frameWidth, frameHeight, frameBitmapSource, true))
        {
            XWTRACE("XImage: failed to read image frame");

            // stop animation
            _resetAnimation();
            return false;
        }
        
        // check if blending is needed
        if(method == XImageFile::eDisposeNone && !m_hasBackground)
        {
            // copy over
            for(int posY = 0; posY < frameHeight; ++posY)
            {
                // copy
                ::CopyMemory(m_framePixels + (posY + frameTop) * m_width + frameLeft, 
                             m_pixelBuffer.data() + posY * frameWidth, 
                             sizeof(DWORD) * frameWidth);
            }

        } else
        {
            DWORD pixelValue, bgPixelValue, blendedValue;
            BYTE pixelAlpha;
            float alphaFactor;

            // alpha blend
            for(int posY = 0; posY < frameHeight; ++posY)
            for(int posX = 0; posX < frameWidth; ++posX)
            {
                // source pixel
                pixelValue = m_pixelBuffer.at(posX + posY * frameWidth);

                // source pixel alpha
                pixelAlpha = (BYTE)(pixelValue >> 24);

                // background pixel value
                bgPixelValue = m_framePixels[(posX + frameLeft) + (posY + frameTop) * m_width];

                // alpha factor
                alphaFactor = (float)(255 - pixelAlpha) / 255.f; 

                // // alpha blend
                blendedValue = 
                    ((BYTE)(((bgPixelValue & 0xFF000000) >> 24) * alphaFactor) + ((pixelValue & 0xFF000000) >> 24)) << 24 |
                    ((BYTE)(((bgPixelValue & 0x00FF0000) >> 16) * alphaFactor) + ((pixelValue & 0x00FF0000) >> 16)) << 16 |
                    ((BYTE)(((bgPixelValue & 0x0000FF00) >> 8) * alphaFactor) + ((pixelValue & 0x0000FF00) >> 8)) << 8 |
                    ((BYTE)(((bgPixelValue & 0x000000FF)) * alphaFactor) + (pixelValue & 0x000000FF));

                // alpha blend
                m_framePixels[(posX + frameLeft) + (posY + frameTop) * m_width] = blendedValue;
            }
        }
    }

    return true;
}

void XImage::_loadGDIFrame(HDC hdc)
{
    // frame bitmap must be ready
    XWASSERT(m_gdiFrameBitmap);
    if(!m_gdiFrameBitmap) return;

    // set paint bitmap
    m_hGdiPaintBitmap = m_gdiFrameBitmap;
}

void XImage::_loadD2DFrame(ID2D1RenderTarget* pTarget)
{
    // frame bitmap buffer must be ready
    XWASSERT(m_framePixels);
    if(!m_framePixels) return;
    
    //if(m_pD2DBitmap) m_pD2DBitmap->Release();
    //m_pD2DBitmap = XD2DHelpers::createBitmap(&m_inputFile, pTarget);
    //return;

    // create bitmap if needed
    if(m_pD2DFrameBitmap == 0)
    {
        // size
        D2D1_SIZE_U usize;
        usize.width = m_width;
        usize.height = m_height;

        // properties
        D2D1_BITMAP_PROPERTIES bprops;
        bprops.dpiX = 96;
        bprops.dpiY = 96;
        bprops.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
        bprops.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

        // create frame bitmap
        HRESULT hr = pTarget->CreateBitmap(usize, bprops, &m_pD2DFrameBitmap);
        if(FAILED(hr))
        {
            XWTRACE_HRES("XImage: failed to create D2D frame bitmap", hr);

            // stop animation
            _resetAnimation();
            return;
        }
    }

    D2D1_RECT_U urect;
    urect.left = 0;
    urect.top = 0;
    urect.right = m_width;
    urect.bottom = m_height;

    // copy pixels
    HRESULT hr = m_pD2DFrameBitmap->CopyFromMemory(&urect, m_framePixels, 4 * m_width);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XImage: failed to copy pixels to D2D frame bitmap", hr);

        // stop animation
        _resetAnimation();
        return;
    }

    // release old paint bitmap if any
    if(m_pD2DBitmap) m_pD2DBitmap->Release();

    // set new bitamp
    m_pD2DBitmap = m_pD2DFrameBitmap;
    m_pD2DBitmap->AddRef();
}

// XImage
/////////////////////////////////////////////////////////////////////
