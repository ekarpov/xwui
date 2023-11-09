// Scrollable interface
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xwscrollable.h"

/////////////////////////////////////////////////////////////////////
// IXWScrollable - scrollable interface

IXWScrollable::IXWScrollable() :
    m_scrollOffsetX(0),
    m_scrollOffsetY(0)
{
}

IXWScrollable::~IXWScrollable()
{
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
bool IXWScrollable::canScrollContent()
{
    // not scrollable by default
    return false;
}

/////////////////////////////////////////////////////////////////////
// offset
/////////////////////////////////////////////////////////////////////
int IXWScrollable::scrollOffsetX()  
{ 
    return m_scrollOffsetX; 
}

int IXWScrollable::scrollOffsetY()  
{ 
    return m_scrollOffsetY; 
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void IXWScrollable::setScrollOffsetX(int scrollOffsetX)
{
    // update scroll offset
    m_scrollOffsetX = scrollOffsetX;
}

void IXWScrollable::setScrollOffsetY(int scrollOffsetY)
{
    // update scroll offset
    m_scrollOffsetY = scrollOffsetY;
}

int IXWScrollable::scrollOffsetForWheel(int wheelDelta)
{
    // use half of delta by default
    return wheelDelta / 2;
}

// IXWScrollable
/////////////////////////////////////////////////////////////////////


