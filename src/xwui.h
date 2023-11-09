// xWUI - eXtended Windows UI Library
// Copyright (c) 2020 Evgeny Karpov
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWUI_H_
#define _XWUI_H_

/////////////////////////////////////////////////////////////////////

// configuration
#include "xwui_config.h"

// localization
#include "locale/xwaction.h"

// layouts
#include "layout/xwlayouts.h"

// xwindow
#include "xwindow/xwindow.h"

// controls
#include "ctrls/xwcontrols.h"

// graphics 
#include "graphics/xwgraphics.h"

// graphics items
#include "xgraphicsitem/xwgraphicsitems.h"

// ActiveX controls
#include "xactive/xwactivecontrols.h"

// extended controls
#include "xctrls/xwextcontrols.h"

/////////////////////////////////////////////////////////////////////

// init library
bool    sInitXWUILibrary(XWUIGraphicsPainter painter = XWUI_PAINTER_AUTOMATIC);
void    sCloseXWUILibrary();

// register extended controls
bool    sRegisterXWUIControls();

// message loop
WPARAM  sXWUIRunMessageLoop();

/////////////////////////////////////////////////////////////////////

#endif // _XWUI_H_

