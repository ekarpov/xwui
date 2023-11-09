// Common graphics functions 
//
/////////////////////////////////////////////////////////////////////

#ifndef _XGRAPHICS_H_
#define _XGRAPHICS_H_

/////////////////////////////////////////////////////////////////////
// graphics

// common
#include "xwgraphicshelpers.h"
#include "xwichelpers.h"
#include "xd2dhelpres.h"
#include "xgdihelpres.h"
#include "ximagefile.h"
#include "ximagefilehelpers.h"
#include "ximage.h"
#include "xgdiresourcescache.h"
#include "xd2dresourcescache.h"

// text
#include "text/xtextinlineobject.h"
#include "text/xtextinlineimage.h"
#include "text/xrichtext.h"
#include "text/xrichtextparser.h"
#include "text/xgdifonts.h"
#include "text/xdwfonts.h"
#include "text/xuniscribehelpers.h"
#include "text/xgditextlayout.h"
#include "text/xdwhelpers.h"
#include "text/xd2dtextlayout.h"
#include "text/xtextservices.h"
#include "text/xrichtextedit.h"
#include "text/xtextlayout.h"

/////////////////////////////////////////////////////////////////////
// init and close resources
bool sInitXWUIGraphics(bool loadDirect2D, bool loadDirectWrite);
void sCloseXWUIGraphics();

/////////////////////////////////////////////////////////////////////

#endif // _XGRAPHICS_H_

