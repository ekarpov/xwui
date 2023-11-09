// Animation bitmap graphics item
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"
#include "../../graphics/xwgraphics.h"

#include "../xgraphicsitem.h"
#include "ximageitem.h"
#include "xanibitmapitem.h"


/////////////////////////////////////////////////////////////////////
// XAniBitmapItem - animation bitmap

/////////////////////////////////////////////////////////////////////
// construction/destruction
/////////////////////////////////////////////////////////////////////
XAniBitmapItem::XAniBitmapItem(XGraphicsItem* parent) :
    XImageItem(parent),
    m_activeBitmap(0),
    m_animationActive(false),
    m_bitmapWidth(0),
    m_bitmapHeight(0),
    m_animationInterval(100)
{
}

XAniBitmapItem::~XAniBitmapItem()
{
    // stop animation if any
    stopAnimation();

    // release bitmaps
    _releaseBitmaps();
}
        
/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XAniBitmapItem::setBitmaps(int width, int height, const std::vector<XMediaSource>& bitmaps)
{
    // check input
    XWASSERT(width > 0);
    XWASSERT(height > 0);
    XWASSERT(bitmaps.size() > 0);
    if(width <= 0 || height <= 0 || bitmaps.size() == 0) return;

    // release previous bitmaps if any
    _releaseBitmaps();

    // copy properties
    m_bitmapWidth = width;
    m_bitmapHeight = height;
    m_bitmaps = bitmaps;

    // fix bitmap size
    setFixedSize(m_bitmapWidth, m_bitmapHeight);
}

void XAniBitmapItem::setAnimationInterval(unsigned long intervalMs)
{
    // check input
    XWASSERT(intervalMs);
    if(intervalMs < 0) return;

    // copy interval
    m_animationInterval = intervalMs;
}

/////////////////////////////////////////////////////////////////////
// animation
/////////////////////////////////////////////////////////////////////
void XAniBitmapItem::startAnimation()
{
    // ignore if already active
    if(m_animationActive) return;

    // set flag
    m_animationActive = true;

    // start timer
    startTimer(m_animationInterval);
}

void XAniBitmapItem::stopAnimation()
{
    // ignore if not active
    if(!m_animationActive) return;

    // stop timer
    stopTimer();

    // reset flag
    m_animationActive = false;
}

/////////////////////////////////////////////////////////////////////
// timer
/////////////////////////////////////////////////////////////////////
bool XAniBitmapItem::onTimerEvent()
{
    // ignore if not active
    if(!m_animationActive) return false;

    // move next bitmap
    m_activeBitmap++;

    // update active bitmap
    _setActiveBitmap();

    // repaint
    repaint();

    return true;
}

/////////////////////////////////////////////////////////////////////
// scrolling interface (from IXWScrollable)
/////////////////////////////////////////////////////////////////////
int XAniBitmapItem::contentWidth()
{
    return m_bitmapWidth;
}

int XAniBitmapItem::contentHeight()
{
    return m_bitmapHeight;
}

/////////////////////////////////////////////////////////////////////
// GDI resource caching (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XAniBitmapItem::onInitGDIResources(HDC hdc)
{
    // pass to parent
    XImageItem::onInitGDIResources(hdc);

    // cache must be ready
    XWASSERT(m_pXGdiResourcesCache);
    if(m_pXGdiResourcesCache == 0) return;

    // preload style bitmaps
    _loadBitmaps();

    // update active bitmap
    _setActiveBitmap();
}

void XAniBitmapItem::onResetGDIResources()
{
    // release all preloaded style images
    if(m_pXGdiResourcesCache)
    {
        _releaseBitmaps();
    }

    // pass to parent
    XImageItem::onResetGDIResources();
}

/////////////////////////////////////////////////////////////////////
// Direct2D resource caching (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XAniBitmapItem::onInitD2DTarget(ID2D1RenderTarget* pTarget)
{
    // pass to parent
    XImageItem::onInitD2DTarget(pTarget);

    // cache must be ready
    XWASSERT(m_pXD2DResourcesCache);
    if(m_pXD2DResourcesCache == 0) return;

    // preload style bitmaps
    _loadBitmaps();

    // update active bitmap
    _setActiveBitmap();
}

void XAniBitmapItem::onResetD2DTarget()
{
    // release all preloaded style images
    if(m_pXD2DResourcesCache)
    {
        _releaseBitmaps();
    }

    // pass to parent
    XImageItem::onResetD2DTarget();
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XAniBitmapItem::_loadBitmaps()
{
    // reset hashes just in case
    XWASSERT(m_bitmapHashes.size() == 0);
    m_bitmapHashes.clear();

    // load bitmaps
    for(std::vector<XMediaSource>::iterator it = m_bitmaps.begin();
        it != m_bitmaps.end(); ++it)
    {
        // load bitmap
        std::wstring hash; 
        if(!loadBitmap(*it, hash))
        {
            XWTRACE("XAniBitmapItem: failed to load animation bitmap");
            continue;
        }

        // append hash
        m_bitmapHashes.push_back(hash);
    }

    // all items must be loaded
    XWASSERT(m_bitmapHashes.size() == m_bitmaps.size())
}

void XAniBitmapItem::_releaseBitmaps()
{
    // release bitmaps
    for(std::vector<std::wstring>::iterator it = m_bitmapHashes.begin();
        it != m_bitmapHashes.end(); ++it)
    {
        // release bitmap
        if(!releaseBitmap((*it)))
        {
            XWTRACE("XAniBitmapItem: failed to release bitmap");
            continue;
        }
    }

    // reset hashes
    m_bitmapHashes.clear();
}

void XAniBitmapItem::_setActiveBitmap()
{
    // check overflow
    if(m_activeBitmap >= m_bitmaps.size())
        m_activeBitmap = 0;

    // set active image
    setImageSource(m_bitmaps.at(m_activeBitmap));
}

// XAniBitmapItem
/////////////////////////////////////////////////////////////////////

