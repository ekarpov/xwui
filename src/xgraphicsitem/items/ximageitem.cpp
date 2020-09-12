// Image graphics item
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"
#include "../../graphics/xwgraphics.h"

#include "../xgraphicsitem.h"
#include "ximageitem.h"

/////////////////////////////////////////////////////////////////////
// XImageItem - image item

XImageItem::XImageItem(XGraphicsItem* parent) :
    m_bFitToSize(false),
    m_cutToFit(false),
    m_bTransparency(255),
    m_alignVertical(eAlignTop),
    m_alignHorizontal(eAlignLeft),
    m_frameDelay(0),
    m_animationId(0)
{
    // add to parent
    if(parent)
        parent->addChildItem(this);
}

XImageItem::~XImageItem()
{
    // release resources
    resetImage();
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
bool XImageItem::setImage(IWICBitmap* wicBitmap)
{
    // pass to image
    return m_itemImage.setImage(wicBitmap);
}

bool XImageItem::setImage(HBITMAP hBitmap)
{
    // pass to image
    return m_itemImage.setImage(hBitmap);
}

bool XImageItem::setImagePath(const WCHAR* filePath)
{
    // pass to image
    return m_itemImage.setImagePath(filePath);
}

bool XImageItem::setImageResourcePath(HMODULE hModule, const WCHAR* szResName, const WCHAR* szResType)
{
    // pass to image
    return m_itemImage.setImageResourcePath(hModule, szResName, szResType);
}

bool XImageItem::setImageStylePath(const WCHAR* stylePath)
{
    // pass to image
    return m_itemImage.setImageStylePath(stylePath);
}

bool XImageItem::setImageSource(const XMediaSource& source)
{
    // pass to image
    return m_itemImage.setImageSource(source);
}

void XImageItem::resetImage()
{
    // pass to image
    m_itemImage.resetImage();
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
void XImageItem::setFitToSize(bool fitToSize, bool cutToFit)
{
    m_bFitToSize = fitToSize;
    m_cutToFit = cutToFit;
}

void XImageItem::setTransparency(BYTE transparency)
{
    // copy transparency value
    m_bTransparency = transparency;
}

void XImageItem::setImageSize(int width, int height, bool keepAspectRatio)
{
    // pass to image
    m_itemImage.setImageSize(width, height, keepAspectRatio);
}

/////////////////////////////////////////////////////////////////////
// image properties
/////////////////////////////////////////////////////////////////////
int XImageItem::imageWidth()
{
    // pass to image
    return m_itemImage.width();
}

int XImageItem::imageHeight()
{
    // pass to image
    return m_itemImage.height();
}

/////////////////////////////////////////////////////////////////////
// alignment
/////////////////////////////////////////////////////////////////////
void XImageItem::setVerticalAlignment(TAlignment alignment)
{
    m_alignVertical = alignment;
}

void XImageItem::setHorizontalAlignment(TAlignment alignment)
{
    m_alignHorizontal = alignment;
}

/////////////////////////////////////////////////////////////////////
// animation
/////////////////////////////////////////////////////////////////////
bool XImageItem::hasFrameAnimation() const
{
    // check from image
    return m_itemImage.hasAnimation();
}

void XImageItem::setDefaultFrameDelay(int delay)
{
    // pass to image
    m_itemImage.setDefaultDelay(delay);
}

bool XImageItem::startFrameAnimation()
{
    // ignore if already ongoing
    if(m_itemImage.isAnimationActive()) return true;

    // start animation
    if(!m_itemImage.beginAnimation(XWUtils::GetWindowDC(parentWindow()))) return false;

    // get first frame delay
    m_itemImage.getFrameDelay(m_frameDelay);

    // start animation
    return startTimerAnimation(m_frameDelay, m_animationId);
}

void XImageItem::stopFrameAnimation()
{
    // stop animation if any
    if(m_animationId) stopAnimation(m_animationId);
    m_animationId = 0;

    // stop animation
    m_itemImage.endAnimation();
}

bool XImageItem::isFrameAnimationActive() const
{
    return m_itemImage.isAnimationActive();
}

/////////////////////////////////////////////////////////////////////
// scrolling (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
int XImageItem::contentWidth()
{
    return m_bFitToSize ? width() : m_itemImage.width();
}

int XImageItem::contentHeight()
{
    return m_bFitToSize ? height() : m_itemImage.height();
}

/////////////////////////////////////////////////////////////////////
// GDI resource caching (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XImageItem::onInitGDIResources(HDC hdc)
{
    // pass to parent
    XGraphicsItem::onInitGDIResources(hdc);

    // pass to image
    m_itemImage.onInitGDIResources(hdc);
}

void XImageItem::onResetGDIResources()
{
    // pass to image
    m_itemImage.onResetGDIResources();

    // pass to parent
    XGraphicsItem::onResetGDIResources();
}

void XImageItem::setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache)
{
    // pass to parent
    XGraphicsItem::setGDIResourcesCache(pXGdiResourcesCache);

    // pass to image
    m_itemImage.setGDIResourcesCache(pXGdiResourcesCache);
}

/////////////////////////////////////////////////////////////////////
// GDI painting (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XImageItem::onPaintGDI(HDC hdc, const RECT& rcPaint)
{
    XWASSERT(hdc);
    if(hdc == 0) return;

    // fill background first if needed
    if(m_fillBackground)
    {
        XGdiHelpers::fillRect(hdc, rcPaint, m_bgColor);
    }

    // ignore if image not set
    if(!m_itemImage.isImageSet()) return;

    // get bitmap from image
    HBITMAP hGdiPaintBitmap = m_itemImage.getGDIPaintBitmap(hdc);

    // must be set
    XWASSERT(hGdiPaintBitmap);
    if(hGdiPaintBitmap == 0) return;

    // draw bitmap
    HDC hdcMem = m_pXGdiResourcesCache->getCompatibleDC(hdc);
    if (hdcMem)
    {
        // select bitmap into the memory DC
        HGDIOBJ hbmOld = ::SelectObject(hdcMem, hGdiPaintBitmap);

        // NOTE: use AlphaBlend to preserve image alpha channel (we expect image to be in pre-multiplied format)
        BLENDFUNCTION bf;
        bf.BlendOp = AC_SRC_OVER;
        bf.BlendFlags = 0;
        bf.SourceConstantAlpha = m_bTransparency; // transparency value between 0-255
        bf.AlphaFormat = AC_SRC_ALPHA;  

        // check if we need to fit image into rect
        if(m_bFitToSize)
        {
            if(m_cutToFit)
            {
                int offsetX, offsetY;
                int imgWidth = m_itemImage.width();
                int imgHeight = m_itemImage.height();

                // compute scaling parameters
                XImageFileHelpers::fitToSizeAspectRatio(width(), height(), offsetX, offsetY, imgWidth, imgHeight);

                ::AlphaBlend(hdc, 
                    rect().left, rect().top,
                    width(), height(), 
                    hdcMem,
                    offsetX, offsetY, 
                    imgWidth, imgHeight, 
                    bf);

            } else
            {
                ::AlphaBlend(hdc, 
                    rect().left, rect().top,
                    width(), height(), 
                    hdcMem,
                    0, 0, 
                    m_itemImage.width(), m_itemImage.height(), 
                    bf);
            }

        } else
        {
            // make sure we have enough pixels in image
            int paintWidth = width();
            int paintHeight = height();
            if(m_itemImage.width() - scrollOffsetX() < paintWidth) paintWidth = m_itemImage.width() - scrollOffsetX();
            if(m_itemImage.height() - scrollOffsetY() < paintHeight) paintHeight = m_itemImage.height() - scrollOffsetY();

            // align image if needed
            int alignOffsetY = 0;
            if(m_alignVertical != eAlignTop && paintHeight < height())
            {
                if(m_alignHorizontal == eAlignCenter)
                    alignOffsetY = (height() - paintHeight) / 2;
                else if(m_alignHorizontal == eAlignBottom)
                    alignOffsetY = (height() - paintHeight);
            }

            int alignOffsetX = 0;
            if(m_alignHorizontal != eAlignLeft && paintWidth < width())
            {
                if(m_alignHorizontal == eAlignCenter)
                    alignOffsetX = (width() - paintWidth) / 2;
                else if(m_alignHorizontal == eAlignRight)
                    alignOffsetX = (width() - paintWidth);
            }

            ::AlphaBlend(hdc, 
                rect().left + alignOffsetX, rect().top + alignOffsetY,
                paintWidth, paintHeight, 
                hdcMem,
                scrollOffsetX(), scrollOffsetY(), 
                paintWidth, paintHeight, 
                bf);
        }
            
        // Restore the memory DC
        if(hbmOld)
            ::SelectObject(hdcMem, hbmOld);
    }
}

/////////////////////////////////////////////////////////////////////
// Direct2D resource caching (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XImageItem::onInitD2DTarget(ID2D1RenderTarget* pTarget)
{
    // pass to parent first
    XGraphicsItem::onInitD2DTarget(pTarget);

    // pass to image
    m_itemImage.onInitD2DTarget(pTarget);
}

void XImageItem::onResetD2DTarget()
{
    // pass to image
    m_itemImage.onResetD2DTarget();

    // pass to parent
    XGraphicsItem::onResetD2DTarget();
}

void XImageItem::setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache)
{
    // pass to parent first
    XGraphicsItem::setD2DResourcesCache(pXD2DResourcesCache);

    // pass to image
    m_itemImage.setD2DResourcesCache(pXD2DResourcesCache);
}

/////////////////////////////////////////////////////////////////////
// Direct2D painting (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XImageItem::onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint)
{
    XWASSERT(pTarget);
    if(pTarget == 0) return;

    // convert paint rectangle to DIPs
    D2D1_RECT_F paintRectDip;
    XD2DHelpers::gdiRectToD2dRect(rcPaint, paintRectDip);

    // fill background first if needed
    if(m_fillBackground)
    {
        // convert color
        D2D1_COLOR_F d2dBackgroundFillColor;
        XD2DHelpers::colorrefToD2dColor(m_bgColor, d2dBackgroundFillColor);

        // get brush from cache and fill
        ID2D1Brush* backgroundFillBrush = createD2DBrush(pTarget, d2dBackgroundFillColor);
        if(backgroundFillBrush)
        {
            pTarget->FillRectangle(paintRectDip, backgroundFillBrush);
            backgroundFillBrush->Release();
        }
    }

    // ignore if image not set
    if(!m_itemImage.isImageSet()) return;

    // get bitmap from image
    ID2D1Bitmap* d2dBitmap = m_itemImage.getD2DPaintBitmap(pTarget);

    // must be set
    XWASSERT(d2dBitmap);
    if(d2dBitmap == 0) return;

    // clip to required paint area
    pTarget->PushAxisAlignedClip(paintRectDip, D2D1_ANTIALIAS_MODE_ALIASED);

    // convert transparency
    FLOAT fTrans = (FLOAT)m_bTransparency / 255.0f;

    // draw image
    if(m_bFitToSize)
    {
        // paint rect
        D2D1_RECT_F drawRect;
        drawRect.left = XD2DHelpers::pixelsToDipsX(rect().left);
        drawRect.top = XD2DHelpers::pixelsToDipsY(rect().top);
        drawRect.right = drawRect.left + XD2DHelpers::pixelsToDipsX(width());
        drawRect.bottom = drawRect.top + XD2DHelpers::pixelsToDipsY(height());

        if(m_cutToFit)
        {
            int offsetX, offsetY;
            int imgWidth = m_itemImage.width();
            int imgHeight = m_itemImage.height();

            // compute scaling parameters
            XImageFileHelpers::fitToSizeAspectRatio(width(), height(), offsetX, offsetY, imgWidth, imgHeight);

            // image rect
            D2D1_RECT_F imageRect;
            imageRect.left = XD2DHelpers::pixelsToDipsX(offsetX);
            imageRect.top = XD2DHelpers::pixelsToDipsY(offsetY);
            imageRect.right = imageRect.left + XD2DHelpers::pixelsToDipsX(imgWidth);
            imageRect.bottom = imageRect.top + XD2DHelpers::pixelsToDipsY(imgHeight);

            // paint bitmap
            pTarget->DrawBitmap(d2dBitmap, drawRect, fTrans, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, imageRect);

        } else
        {
            // paint bitmap
            pTarget->DrawBitmap(d2dBitmap, drawRect, fTrans);
        }

    }
    else
    {
        // make sure we have enough pixels in image
        int paintWidth = width();
        int paintHeight = height();
        if(m_itemImage.width() - scrollOffsetX() < paintWidth) paintWidth = m_itemImage.width() - scrollOffsetX();
        if(m_itemImage.height() - scrollOffsetY() < paintHeight) paintHeight = m_itemImage.height() - scrollOffsetY();

        // align image if needed
        int alignOffsetY = 0;
        if(m_alignVertical != eAlignTop && paintHeight < height())
        {
            if(m_alignHorizontal == eAlignCenter)
                alignOffsetY = (height() - paintHeight) / 2;
            else if(m_alignHorizontal == eAlignBottom)
                alignOffsetY = (height() - paintHeight);
        }

        int alignOffsetX = 0;
        if(m_alignHorizontal != eAlignLeft && paintWidth < width())
        {
            if(m_alignHorizontal == eAlignCenter)
                alignOffsetX = (width() - paintWidth) / 2;
            else if(m_alignHorizontal == eAlignRight)
                alignOffsetX = (width() - paintWidth);
        }

        // paint rect
        D2D1_RECT_F drawRect;
        drawRect.left = XD2DHelpers::pixelsToDipsX(rect().left + alignOffsetX);
        drawRect.top = XD2DHelpers::pixelsToDipsY(rect().top + alignOffsetY);
        drawRect.right = drawRect.left + XD2DHelpers::pixelsToDipsX(paintWidth);
        drawRect.bottom = drawRect.top + XD2DHelpers::pixelsToDipsY(paintHeight);

        // image rect
        D2D1_RECT_F imageRect;
        imageRect.left = XD2DHelpers::pixelsToDipsX(scrollOffsetX());
        imageRect.top = XD2DHelpers::pixelsToDipsY(scrollOffsetY());
        imageRect.right = imageRect.left + XD2DHelpers::pixelsToDipsX(paintWidth);
        imageRect.bottom = imageRect.top + XD2DHelpers::pixelsToDipsY(paintHeight);

        pTarget->DrawBitmap(d2dBitmap, drawRect, fTrans, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, imageRect);
    }

    // remove clipping
    pTarget->PopAxisAlignedClip();
}

/////////////////////////////////////////////////////////////////////
// animation events (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XImageItem::onAnimationTimer(DWORD id)
{
    // must be item animation
    XWASSERT(m_animationId == id);
    if(m_animationId != id) return;

    // check if this is animation timer
    if(m_itemImage.isAnimationActive())
    {
        // select next frame
        if(m_itemImage.loadNextFrame(XWUtils::GetWindowDC(parentWindow())))
        {
            // start timer
            _startNextFrameTimer();
            
            // update image
            repaint();

        } else
        {
            // failed to load frame, stop animation
            if(m_animationId) stopAnimation(m_animationId);
            m_animationId = 0;
        }

    } else
    {
        // stop animation
        if(m_animationId) stopAnimation(m_animationId);
        m_animationId = 0;
    }
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
bool XImageItem::_startNextFrameTimer()
{
    int frameDelay = 0;

    // get frame delay
    m_itemImage.getFrameDelay(frameDelay);

    // check if we need to restart timer
    if(m_frameDelay != frameDelay)
    {
        // copy delay
        m_frameDelay = frameDelay;

        // stop animation
        if(m_animationId) stopAnimation(m_animationId);
        m_animationId = 0;

        // start animation again
        return startTimerAnimation(m_frameDelay, m_animationId);
    }

    return true;
}

// XImageItem
/////////////////////////////////////////////////////////////////////
