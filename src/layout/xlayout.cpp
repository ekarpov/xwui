// Base functionality for window layout engine
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xlayoutitem.h"
#include "xlayout.h"

/////////////////////////////////////////////////////////////////////
// IXLayout - layout manager interface
IXLayout::IXLayout() :
    m_nMarginLeft(0),
    m_nMarginTop(0),
    m_nMarginRight(0),
    m_nMarginBottom(0)
{
}

IXLayout::~IXLayout()
{
}

/////////////////////////////////////////////////////////////////////
// content margins
/////////////////////////////////////////////////////////////////////
void IXLayout::setContentMargins(int left, int top, int right, int bottom)
{
    // set margins
    m_nMarginLeft = left;
    m_nMarginTop = top;
    m_nMarginRight = right;
    m_nMarginBottom = bottom;
}

// IXLayout
/////////////////////////////////////////////////////////////////////
