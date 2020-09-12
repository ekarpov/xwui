// Localized actions
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWACTION_H_
#define _XWACTION_H_

/////////////////////////////////////////////////////////////////////
// types

enum XWUI_ACTION
{
    XWUI_ACTION_UNKNOWN = 0,

    // common UI actions
    XWUI_ACTION_ADD,
    XWUI_ACTION_DELETE,
    XWUI_ACTION_BACK,
    XWUI_ACTION_FORWARD,
    XWUI_ACTION_CUT,
    XWUI_ACTION_COPY,
    XWUI_ACTION_PASTE,
    XWUI_ACTION_VIEW,
    XWUI_ACTION_JOIN,

    XWUI_ACTION_COUNT           // must be the last
};

/////////////////////////////////////////////////////////////////////
// functions

// get localized action name
const WCHAR*    sXWUIGetActionName(XWUI_ACTION action);

/////////////////////////////////////////////////////////////////////

#endif // _XWACTION_H_

