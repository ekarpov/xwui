// Spinner progress bar
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"
#include "../../graphics/xwgraphics.h"

#include "../xgraphicsitem.h"
#include "ximageitem.h"
#include "xanibitmapitem.h"
#include "xspinneritem.h"

/////////////////////////////////////////////////////////////////////
// XSpinnerItem - spinner progress item

/////////////////////////////////////////////////////////////////////
// construction/destruction
/////////////////////////////////////////////////////////////////////
XSpinnerItem::XSpinnerItem(XWUIStyle::XSpinnerSize size, XGraphicsItem* parent)
{
    // progress bitmap
    m_progressBitmap = new XAniBitmapItem(this);

    // bitmaps
    std::vector<XMediaSource> bitmaps;
    if(size == XWUIStyle::eSpinnerSize26)
    {
        bitmaps.push_back(XMediaSource(L"spinner_frame_1_26.png", eXMediaSourceStyle));
        bitmaps.push_back(XMediaSource(L"spinner_frame_2_26.png", eXMediaSourceStyle));
        bitmaps.push_back(XMediaSource(L"spinner_frame_3_26.png", eXMediaSourceStyle));
        bitmaps.push_back(XMediaSource(L"spinner_frame_4_26.png", eXMediaSourceStyle));
        bitmaps.push_back(XMediaSource(L"spinner_frame_5_26.png", eXMediaSourceStyle));
        bitmaps.push_back(XMediaSource(L"spinner_frame_6_26.png", eXMediaSourceStyle));
        bitmaps.push_back(XMediaSource(L"spinner_frame_7_26.png", eXMediaSourceStyle));
        bitmaps.push_back(XMediaSource(L"spinner_frame_8_26.png", eXMediaSourceStyle));
        m_progressBitmap->setBitmaps(26, 26, bitmaps);

    } else
    {
        bitmaps.push_back(XMediaSource(L"spinner_frame_1_32.png", eXMediaSourceStyle));
        bitmaps.push_back(XMediaSource(L"spinner_frame_2_32.png", eXMediaSourceStyle));
        bitmaps.push_back(XMediaSource(L"spinner_frame_3_32.png", eXMediaSourceStyle));
        bitmaps.push_back(XMediaSource(L"spinner_frame_4_32.png", eXMediaSourceStyle));
        bitmaps.push_back(XMediaSource(L"spinner_frame_5_32.png", eXMediaSourceStyle));
        bitmaps.push_back(XMediaSource(L"spinner_frame_6_32.png", eXMediaSourceStyle));
        bitmaps.push_back(XMediaSource(L"spinner_frame_7_32.png", eXMediaSourceStyle));
        bitmaps.push_back(XMediaSource(L"spinner_frame_8_32.png", eXMediaSourceStyle));
        m_progressBitmap->setBitmaps(32, 32, bitmaps);
    }

    // add to parent
    if(parent)
        parent->addChildItem(this);
}

XSpinnerItem::~XSpinnerItem()
{
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XSpinnerItem::startProgress()
{
    // pass to progress bitmap
    m_progressBitmap->startAnimation();
}

void XSpinnerItem::stopProgress()
{
    // pass to progress bitmap
    m_progressBitmap->stopAnimation();
}

/////////////////////////////////////////////////////////////////////
// manipulations (from IXLayoutItem)
/////////////////////////////////////////////////////////////////////
void XSpinnerItem::update(int posX, int posY, int width, int height)
{
    // pass to parent first
    XGraphicsItem::update(posX, posY, width, height);

    // center progress bitmap
    int startX = (width - m_progressBitmap->contentWidth()) / 2;
    int startY = (height - m_progressBitmap->contentHeight()) / 2;

    // position progress bitmap
    m_progressBitmap->update(startX, startY, 
        m_progressBitmap->contentWidth(), m_progressBitmap->contentHeight());
}

/////////////////////////////////////////////////////////////////////
// content size
/////////////////////////////////////////////////////////////////////
int XSpinnerItem::contentHeight()
{
    return m_progressBitmap->contentHeight();
}

// XSpinnerItem
/////////////////////////////////////////////////////////////////////
