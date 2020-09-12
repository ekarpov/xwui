// xWUI - eXtended Windows UI Library
// Copyright (c) 2020 Evgeny Karpov
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWUI_H_
#define _XWUI_H_

/////////////////////////////////////////////////////////////////////
// configuration
#include "xwui_config.h"

/////////////////////////////////////////////////////////////////////
// painter types
enum XWUIGraphicsPainter
{
    XWUI_PAINTER_AUTOMATIC = 0,     // select best available painter automatically
    XWUI_PAINTER_GDI,               // use GDI painter
    XWUI_PAINTER_D2D                // use Direct2D painter
};

XWUIGraphicsPainter sXWUIDefaultPainter();

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

