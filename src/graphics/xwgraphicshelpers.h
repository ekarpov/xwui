// Graphics helpers
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWGRAPHICSHELPERS_H_
#define _XWGRAPHICSHELPERS_H_

/////////////////////////////////////////////////////////////////////
// types

// region as a set of rectangles
typedef std::vector<RECT>   XRectRegion;

/////////////////////////////////////////////////////////////////////
// XWUIGraphics

namespace XWUIGraphics
{
    // regions
    void        addRectRegion(XRectRegion& region, const RECT& rect);
    bool        isInsideRectRegion(XRectRegion& region, int posX, int posY);
    void        appendRectRegion(XRectRegion& region, const XRectRegion& toAppend);

    // create regions
    XRectRegion     createRectRegionFromPoints(int x1, int y1, int x2, int y2);
}

// XWUIGraphics
/////////////////////////////////////////////////////////////////////

#endif // _XWGRAPHICSHELPERS_H_

