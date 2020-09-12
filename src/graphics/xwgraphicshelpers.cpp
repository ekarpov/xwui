// Graphics helpers
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xwgraphicshelpers.h"

/////////////////////////////////////////////////////////////////////
// XWUIGraphics

// regions
void XWUIGraphics::addRectRegion(XRectRegion& region, const RECT& rect)
{
    // just append rect
    region.push_back(rect);
}

bool XWUIGraphics::isInsideRectRegion(XRectRegion& region, int posX, int posY)
{
    // loop over all rectangles
    for(XRectRegion::iterator it = region.begin(); it != region.end(); ++it)
    {
        // check if point is inside
        if(XWUtils::rectIsInside((*it), posX, posY)) return true;
    }

    return false;
}

void XWUIGraphics::appendRectRegion(XRectRegion& region, const XRectRegion& toAppend)
{
    // append
    region.insert(region.end(), toAppend.begin(), toAppend.end());
}

XRectRegion XWUIGraphics::createRectRegionFromPoints(int x1, int y1, int x2, int y2)
{
    // create rect
    RECT rect;

    if(x1 < x2)
    {
        rect.left = x1;
        rect.right = x2;

    } else
    {
        rect.left = x2;
        rect.right = x1;
    }

    if(y1 < y2)
    {
        rect.top = y1;
        rect.bottom = y2;

    } else
    {
        rect.top = y2;
        rect.bottom = y1;
    }

    // create region
    XRectRegion region;
    region.push_back(rect);

    return region;
}

// XWUIGraphics
/////////////////////////////////////////////////////////////////////

