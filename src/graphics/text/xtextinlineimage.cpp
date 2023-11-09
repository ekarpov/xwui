// Text inline image 
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"

#include "../../xactive/xoledataobject.h"
#include "../xgdihelpres.h"
#include "../xd2dhelpres.h"
#include "../xwichelpers.h"
#include "../ximagefile.h"
#include "../ximage.h"
#include "xtextinlineobject.h"
#include "xtextinlineimage.h"

/////////////////////////////////////////////////////////////////////
// XTextInlineImage - text inline image 

XTextInlineImage::XTextInlineImage() :
    m_fitToText(false),
    m_baseline(0),
    m_runAnimation(false),
    m_hwndParent(0),
    m_timerId(0),
    m_contentId(0),
    m_frameDelay(0),
    m_originX(0),
    m_originY(0)
{
}

XTextInlineImage::~XTextInlineImage()
{
    // release resources
    resetImage();
}

/////////////////////////////////////////////////////////////////////
// set image 
/////////////////////////////////////////////////////////////////////
bool XTextInlineImage::setImage(IWICBitmap* wicBitmap)
{
    // release previous image if any
    resetImage();

    // pass to image
    return m_itemImage.setImage(wicBitmap);
}

bool XTextInlineImage::setImage(HBITMAP hBitmap)
{
    // release previous image if any
    resetImage();

    // pass to image
    return m_itemImage.setImage(hBitmap);
}

bool XTextInlineImage::setImagePath(const WCHAR* filePath)
{
    // release previous image if any
    resetImage();

    // pass to image
    return m_itemImage.setImagePath(filePath);
}

bool XTextInlineImage::setImageResourcePath(HMODULE hModule, const WCHAR* szResName, const WCHAR* szResType)
{
    // release previous image if any
    resetImage();

    // pass to image
    return m_itemImage.setImageResourcePath(hModule, szResName, szResType);
}

bool XTextInlineImage::setImageUri(const wchar_t* imageUri, int width, int height)
{
    XWASSERT(imageUri);
    if(imageUri == 0) return false;

    // release previous image if any
    resetImage();

    // set size
    if(width != 0 && height != 0)
        setFixedSize(width, height);

    XMediaSource imgSource;

    // check if image is already loaded
    if(XWContentProvider::instance()->isUrlContentLoaded(imageUri, imgSource))
    {
        // set source
        return m_itemImage.setImageSource(imgSource);

    } else
    {
        // try to load image
        if(!XWContentProvider::instance()->loadUrlContent(imageUri, this, m_contentId))
        {
            XWTRACE("XTextInlineImage: failed to start image loading");
            return false;
        }
    }

    return true;
}

void XTextInlineImage::resetImage()
{
    // stop animation if any
    _stopAnimation();

    // cancel content loading if any
    _cancelContentLoad();

    // pass to image
    m_itemImage.resetImage();
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
void XTextInlineImage::setFitToText(bool fitToText)
{
    // copy flag
    m_fitToText = fitToText;
}

void XTextInlineImage::setFixedSize(int width, int height, bool keepAspectRatio)
{
    // pass to image
    m_itemImage.setImageSize(width, height, keepAspectRatio);
}

/////////////////////////////////////////////////////////////////////
// item properties (from XTextInlineObject)
/////////////////////////////////////////////////////////////////////
int XTextInlineImage::width() const
{
    // pass to image
    return m_itemImage.width();
}

int XTextInlineImage::height() const
{
    // pass to image
    return m_itemImage.height();
}

int XTextInlineImage::baseline() const
{
    return m_baseline;
}

/////////////////////////////////////////////////////////////////////
// shape object based on text style (from XTextInlineObject)
/////////////////////////////////////////////////////////////////////
void XTextInlineImage::shapeContentGDI(HDC hdc, int fontHeight, int fontAscent)
{
    // ignore if image not set
    if(!m_itemImage.isImageSet()) return;

    if(m_fitToText)
    {
        // resize image to fit font height and position in line with characters
        m_itemImage.setImageSize(m_itemImage.width(), fontHeight, true);

        // TODO: update m_baseline ?
    }
}

void XTextInlineImage::shapeContentD2D(FLOAT fontHeight, FLOAT fontAscent)
{
    // ignore if image not set
    if(!m_itemImage.isImageSet()) return;

    if(m_fitToText)
    {
        // resize image to fit font height and position in line with characters
        m_itemImage.setImageSize(m_itemImage.width(), XD2DHelpers::dipsToPixelsY(fontHeight), true);

        // TODO: update m_baseline ?
    }
}

/////////////////////////////////////////////////////////////////////
// position object (from XTextInlineObject)
/////////////////////////////////////////////////////////////////////
void XTextInlineImage::setOrigin(int originX, int originY)
{
    // save origin 
    m_originX = originX;
    m_originY = originY;
}

void XTextInlineImage::moveOrigin(int offsetX, int offsetY)
{
    // move origin 
    m_originX += offsetX;
    m_originY += offsetY;
}

/////////////////////////////////////////////////////////////////////
// animation (from XTextInlineObject)
/////////////////////////////////////////////////////////////////////
void XTextInlineImage::enableAnimation(HWND parentWindow, bool enable)
{
    // ignore if the same
    if(enable == m_runAnimation) return;

    // validate input    
    if(enable && parentWindow == 0)
    {
        XWASSERT1(0, "XTextInlineImage: parent window must be set if animation is enabled");
        return;
    }

    // copy 
    m_runAnimation = enable;
    m_hwndParent = parentWindow;

    // stop animation if needed
    if(!m_runAnimation) 
        _stopAnimation();
}

void XTextInlineImage::pauseAnimation()
{
    // ignore if not enabled
    if(!m_runAnimation) return;

    // check if timer is active
    if(m_timerId != 0)
        XWAnimationTimer::instance()->pauseAnimation(m_timerId);
}

void XTextInlineImage::resumeAnimation()
{
    // ignore if not enabled
    if(!m_runAnimation) return;

    // check if timer is active
    if(m_timerId != 0)
        XWAnimationTimer::instance()->resumeAnimation(m_timerId);
}

/////////////////////////////////////////////////////////////////////
// content loading (from IXWContentProviderCallback)
/////////////////////////////////////////////////////////////////////
void XTextInlineImage::onUrlContentLoaded(DWORD id, const WCHAR* path)
{
    XWASSERT(path);
    if(path == 0) return;

    // load image from path
    if(!setImagePath(path))
    {
        XWTRACE("XTextInlineImage: failed to load url content");
    }

    // TODO: inform parent layout that content has been updated
}

void XTextInlineImage::onUrlContentLoadFailed(DWORD id, DWORD reason)
{
    // TODO: inform parent layout that content has failed to load
}

/////////////////////////////////////////////////////////////////////
// GDI resource caching 
/////////////////////////////////////////////////////////////////////
void XTextInlineImage::onInitGDIResources(HDC hdc)
{
    // pass to parent
    XTextInlineObject::onInitGDIResources(hdc);

    // pass to image
    m_itemImage.onInitGDIResources(hdc);
}

void XTextInlineImage::onResetGDIResources()
{
    // pass to image
    m_itemImage.onResetGDIResources();

    // pass to parent
    XTextInlineObject::onResetGDIResources();
}

void XTextInlineImage::setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache)
{
    // pass to parent
    XTextInlineObject::setGDIResourcesCache(pXGdiResourcesCache);

    // pass to image
    m_itemImage.setGDIResourcesCache(pXGdiResourcesCache);
}

/////////////////////////////////////////////////////////////////////
// GDI painting (from XTextInlineObject)
/////////////////////////////////////////////////////////////////////
void XTextInlineImage::onPaintGDI(int posX, int posY, HDC hdc)
{
    // check if animation is enabled
    if(m_runAnimation && m_itemImage.hasAnimation())
    {
        // start animation if not running already
        _startAnimation();
    }

    // pass to image
    m_itemImage.onPaintGDI(posX, posY, hdc);
}

/////////////////////////////////////////////////////////////////////
// Direct2D resource caching (from XTextInlineObject)
/////////////////////////////////////////////////////////////////////
void XTextInlineImage::onInitD2DTarget(ID2D1RenderTarget* pTarget)
{
    // pass to parent first
    XTextInlineObject::onInitD2DTarget(pTarget);

    // pass to image
    m_itemImage.onInitD2DTarget(pTarget);
}

void XTextInlineImage::onResetD2DTarget()
{
    // pass to image
    m_itemImage.onResetD2DTarget();

    // pass to parent
    XTextInlineObject::onResetD2DTarget();
}

void XTextInlineImage::setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache)
{
    // pass to parent first
    XTextInlineObject::setD2DResourcesCache(pXD2DResourcesCache);

    // pass to image
    m_itemImage.setD2DResourcesCache(pXD2DResourcesCache);
}

/////////////////////////////////////////////////////////////////////
// Direct2D painting (from XTextInlineObject)
/////////////////////////////////////////////////////////////////////
void XTextInlineImage::onPaintD2D(FLOAT posX, FLOAT posY, ID2D1RenderTarget* pTarget)
{
    // check if animation is enabled
    if(m_runAnimation && m_itemImage.hasAnimation())
    {
        // start animation if not running already
        _startAnimation();
    }

    // pass to image
    m_itemImage.onPaintD2D(posX, posY, pTarget);
}

/////////////////////////////////////////////////////////////////////
// convert to formatted text 
/////////////////////////////////////////////////////////////////////
void XTextInlineImage::toFormattedText(std::wstring& text)
{
    // TODO: format <img> tag
}

/////////////////////////////////////////////////////////////////////
// create OLE object 
/////////////////////////////////////////////////////////////////////
HRESULT XTextInlineImage::createOleObject(IOleClientSite* oleClientSite, IStorage* storage, IOleObject** objectOut)
{
    // ignore if image not set
    if(!m_itemImage.isImageSet()) return E_NOT_VALID_STATE;

    // get global HDC
    HDC hdc = ::GetDC(::GetDesktopWindow());
    if(hdc == 0) return E_UNEXPECTED;

    // create OLE data object
    XOleDataObject* oleDataObject = new XOleDataObject();
    oleDataObject->AddRef();

    // set bitmap
    HRESULT hr = oleDataObject->setBitmapData(m_itemImage.getGDIPaintBitmap(hdc), true);
    if(SUCCEEDED(hr))
    {
        // convert
        hr = oleDataObject->createStaticOleObject(oleClientSite, storage, objectOut);
    }

    // release DC
    ::ReleaseDC(::GetDesktopWindow(), hdc);

    // release object
    oleDataObject->Release();

    return hr;
}

/////////////////////////////////////////////////////////////////////
// events
/////////////////////////////////////////////////////////////////////
void XTextInlineImage::onAnimationTimer(DWORD id)
{
    XWASSERT(id == m_timerId);
    if(m_timerId != id) return;

    // select next frame
    if(m_itemImage.loadNextFrame(XWUtils::GetWindowDC(m_hwndParent)))
    {
        // start timer
        if(_startNextFrameTimer())
        {
            // update region 
            RECT rcPaint;
            rcPaint.left = m_originX;
            rcPaint.top = m_originY;
            rcPaint.right = m_originX + m_itemImage.width();
            rcPaint.bottom = m_originY + m_itemImage.height();

            // request update
            ::InvalidateRect(m_hwndParent, &rcPaint, FALSE);

            return;
        }
    }

    // failed to load frame, stop animation
    _stopAnimation();
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XTextInlineImage::_startAnimation()
{
    // ignore if already running
    if(m_itemImage.isAnimationActive()) return;

    // ignore if not enabled
    if(!m_runAnimation) return;

    // ignore if image doesn't have animation
    if(!m_itemImage.hasAnimation()) return;

    // start image animation
    if(!m_itemImage.beginAnimation(XWUtils::GetWindowDC(m_hwndParent))) return;

    // get first frame delay
    m_itemImage.getFrameDelay(m_frameDelay);

    // start timer
    XWAnimationTimer::instance()->startTimerAnimation(m_frameDelay, this, m_timerId);
}

void XTextInlineImage::_stopAnimation()
{
    // stop timer if any
    if(m_timerId != 0)
        XWAnimationTimer::instance()->stopAnimation(m_timerId);
    
    // reset timer
    m_timerId = 0;

    // end image animation
    if(m_itemImage.isAnimationActive())
        m_itemImage.endAnimation();
}

bool XTextInlineImage::_startNextFrameTimer()
{
    int frameDelay = 0;

    // get frame delay
    m_itemImage.getFrameDelay(frameDelay);

    // check if we need to restart timer
    if(m_frameDelay != frameDelay)
    {
        // copy delay
        m_frameDelay = frameDelay;

        // stop timer if any
        if(m_timerId != 0)
            XWAnimationTimer::instance()->stopAnimation(m_timerId);

        // start timer
        return XWAnimationTimer::instance()->startTimerAnimation(m_frameDelay, this, m_timerId);
    }

    return true;
}

void XTextInlineImage::_cancelContentLoad()
{
    // ignore if nothing loading
    if(m_contentId == 0) return;

    // cancel load
    XWContentProvider::instance()->cancelContentLoad(m_contentId);

    // reset id
    m_contentId = 0;
}

// XTextInlineImage
/////////////////////////////////////////////////////////////////////


