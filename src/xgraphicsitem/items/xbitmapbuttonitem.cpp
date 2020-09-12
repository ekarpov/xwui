// Bitmap button grapchics item 
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"
#include "../../graphics/xwgraphics.h"

#include "../xgraphicsitem.h"
#include "../items/ximageitem.h"

#include "xbitmapbuttonitem.h"

/////////////////////////////////////////////////////////////////////
// XBitmapButtonItem - bitmap button graphics item

/////////////////////////////////////////////////////////////////////
// construction/destruction
/////////////////////////////////////////////////////////////////////
XBitmapButtonItem::XBitmapButtonItem(XGraphicsItem* parent) :
    m_bitmapWidth(0),
    m_bitmapHeight(0),
    m_isChecked(false)
{
    // add to parent
    if(parent)
        parent->addChildItem(this);
}

XBitmapButtonItem::XBitmapButtonItem(XWUIStyle::XStyleBitmapButtons button, XGraphicsItem* parent) :
    m_bitmapWidth(0),
    m_bitmapHeight(0),
    m_isChecked(false)
{
    // add to parent
    if(parent)
        parent->addChildItem(this);

    // set bitmaps
    setBitmaps(button);
}

XBitmapButtonItem::~XBitmapButtonItem()
{
    // release resources
    _releaseBitmaps();
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XBitmapButtonItem::setBitmaps(int width, int height, const XWUIStyle::XStyleBitmaps& bitmaps)
{
    // validate size
    XWASSERT(width);
    XWASSERT(height);
    if(width == 0 || height == 0) return;

    // validate input
    XWASSERT1(bitmaps.normal.isSet(), "XBitmapButtonItem: default state bitmap is missing");
    if(!bitmaps.normal.isSet()) return;

    // release previous bitmaps if any
    _releaseBitmaps();

    // copy input
    m_buttonBitmaps = bitmaps;
    m_bitmapWidth = width;
    m_bitmapHeight = height;

    // load bitmaps
    _loadBitmaps();

    // resize
    setFixedSize(width, height);

    // properties
    setClickable(true);

    // set pointer cursor
    XGraphicsItem::setMouseCursor(XWUtils::getSystemCursor(XWUtils::eCursorHand));
}

void XBitmapButtonItem::setBitmaps(XWUIStyle::XStyleBitmapButtons button)
{
    XWUIStyle::XStyleBitmaps bitmaps;
    XWUIStyle::XStyleSize size;

    // get bitmaps from style
    XWUIStyle::getButtonBitmaps(button, bitmaps, size);

    // set bitmaps
    setBitmaps(size.width, size.height, bitmaps);
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
void XBitmapButtonItem::setChecked(bool checked)
{
    // ignore if flag is the same
    if(m_isChecked == checked) return;

    // copy flag
    m_isChecked = checked;

    // select active bitmap
    _selectActiveBitmap();

    // update UI
    repaint();
}

/////////////////////////////////////////////////////////////////////
// scrolling interface (from IXWScrollable)
/////////////////////////////////////////////////////////////////////
int XBitmapButtonItem::contentWidth()
{
    return (m_bitmapWidth != 0) ? m_bitmapWidth : XImageItem::contentWidth();
}

int XBitmapButtonItem::contentHeight()
{
    return (m_bitmapHeight != 0) ? m_bitmapHeight : XImageItem::contentHeight();
}

/////////////////////////////////////////////////////////////////////
// GDI resource caching (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XBitmapButtonItem::onInitGDIResources(HDC hdc)
{
    // pass to parent
    XImageItem::onInitGDIResources(hdc);

    // preload style bitmaps
    _loadBitmaps();
}

void XBitmapButtonItem::onResetGDIResources()
{
    // release all preloaded images
    _releaseBitmaps();

    // pass to parent
    XImageItem::onResetGDIResources();
}

/////////////////////////////////////////////////////////////////////
// Direct2D resource caching (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XBitmapButtonItem::onInitD2DTarget(ID2D1RenderTarget* pTarget)
{
    // pass to parent
    XImageItem::onInitD2DTarget(pTarget);

    // preload style bitmaps
    _loadBitmaps();
}

void XBitmapButtonItem::onResetD2DTarget()
{
    // release all preloaded images
    _releaseBitmaps();

    // pass to parent
    XImageItem::onResetD2DTarget();
}

/////////////////////////////////////////////////////////////////////
// item state
/////////////////////////////////////////////////////////////////////
void XBitmapButtonItem::onStateFlagChanged(TStateFlag flag, bool value)
{
    // select active bitmap
    _selectActiveBitmap();

    // update UI
    repaint();
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XBitmapButtonItem::_loadBitmap(XWUIStyle::XStyleState state)
{
    // get image
    XMediaSource imgsrc = XWUIStyle::getStateBitmap(state, m_buttonBitmaps);

    // ignore if not set
    if(!imgsrc.isSet()) return;

    std::wstring imghash;

    // load
    if(loadBitmap(XWUIStyle::getStateBitmap(state, m_buttonBitmaps), imghash))
    {
        // add hash to map
        m_bitmapHashes.insert(_BitmapHashesT::value_type(state, imghash));

    } else
    {
        XWTRACE("XBitmapButtonItem: failed to load state bitmap");
    }
}

void XBitmapButtonItem::_loadBitmaps()
{
    // release previous bitmaps if any
    _releaseBitmaps();

    // pre-load all bitmaps 
    for(int state = XWUIStyle::eStyleStateDefault; state != XWUIStyle::eStyleStateCount; ++state)
    {
        _loadBitmap((XWUIStyle::XStyleState)state);
    }

    // select active bitmap
    _selectActiveBitmap();
}

void XBitmapButtonItem::_releaseBitmaps()
{
    // loop over all hashes
    for(_BitmapHashesT::iterator it = m_bitmapHashes.begin(); 
        it != m_bitmapHashes.end(); ++it)
    {
        // release bitmap
        releaseBitmap(it->second);
    }

    // reset hashes
    m_bitmapHashes.clear();
}

void XBitmapButtonItem::_selectActiveBitmap()
{
    // get image from style state
    XMediaSource imgsrc = XWUIStyle::getStateBitmap(styleState(), m_buttonBitmaps);

    // if item is checked use pressed bitmap
    if(m_isChecked && styleState() == XWUIStyle::eStyleStateDefault)
    {
        // set pressed bitmap instead
        if(m_buttonBitmaps.pressed.isSet())
            imgsrc = m_buttonBitmaps.pressed;
    }

    // check if set
    if(imgsrc.isSet())
    {
        // set image
        if(!setImageSource(imgsrc))
        {
            XWTRACE("XBitmapButtonItem: failed to set state image");
        }

    } else if(m_buttonBitmaps.normal.isSet())
    {
        // set default
        if(!setImageSource(m_buttonBitmaps.normal))
        {
            XWTRACE("XBitmapButtonItem: failed to set default image");
        }
    }
}

// XBitmapButtonItem
/////////////////////////////////////////////////////////////////////

