// Localized actions
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"
#include "xwaction.h"

/////////////////////////////////////////////////////////////////////
// functions

const WCHAR* sXWUIGetActionName(XWUI_ACTION action)
{
    switch(action)
    {
    case XWUI_ACTION_ADD:           return _LTEXT("Add", "Add UI action");
    case XWUI_ACTION_DELETE:        return _LTEXT("Delete", "Delete UI action");
    case XWUI_ACTION_BACK:          return _LTEXT("Back", "Back UI action");
    case XWUI_ACTION_FORWARD:       return _LTEXT("Forward", "Forward UI action");
    case XWUI_ACTION_CUT:           return _LTEXT("Cut", "Cut UI action");
    case XWUI_ACTION_COPY:          return _LTEXT("Copy", "Copy UI action");
    case XWUI_ACTION_PASTE:         return _LTEXT("Paste", "Paste UI action");
    case XWUI_ACTION_VIEW:          return _LTEXT("View", "View UI action");
    case XWUI_ACTION_JOIN:          return _LTEXT("Join", "Join UI action");
    }

    XWASSERT1(0, "XWUI: unknown action name requested");
    return L"Uknown";
}

/////////////////////////////////////////////////////////////////////
