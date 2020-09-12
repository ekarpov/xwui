// xWUI - eXtended Windows UI Library
// Copyright (c) 2020 Evgeny Karpov
//
// Common configuration and include files
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWUI_CONFIG_H_
#define _XWUI_CONFIG_H_

/////////////////////////////////////////////////////////////////////
// Windows
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <commctrl.h>
#include <limits.h>
#include <crtdbg.h>
#include <comdef.h>

/////////////////////////////////////////////////////////////////////
// Direct2D 
#include <d2d1.h>
#include <d2d1helper.h>

/////////////////////////////////////////////////////////////////////
// standard library
#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <set>

/////////////////////////////////////////////////////////////////////
// core
#include "core/xwdebug.h"
#include "core/xweventmap.h"
#include "core/xwobjecteventmap.h"
#include "core/xwobject.h"
#include "core/xwkeys.h"
#include "core/xwscrollable.h"
#include "core/xwscrollviewlogic.h"
#include "core/xwmessagehook.h"
#include "core/xwmessages.h"
#include "core/xtextstyle.h"
#include "core/xmediasource.h"
#include "core/xwanimationtimer.h"
#include "core/xwcontentprovider.h"
#include "core/xwcontentproviderimpl.h"
#include "core/xwutils.h"

/////////////////////////////////////////////////////////////////////
// style
//#include "style/xwuistyle.h"

/////////////////////////////////////////////////////////////////////
// localization
//#include "locale/xwlocale.h"

/////////////////////////////////////////////////////////////////////

#endif // _XWUI_CONFIG_H_

