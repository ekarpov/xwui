// XWUI custom messages
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWMESSAGES_H_
#define _XWMESSAGES_H_

/////////////////////////////////////////////////////////////////////
// custom messages

enum XWUI_MESSAGES
{
    WM_XWUI_MESSAGES_OFFSET =        WM_USER + 100,      

    ////////////////////////////
    // Window messages:

    // Description: Child window content changed, parent must update layout and repaint
    // Agruments:   none
    WM_XWUI_CHILD_CONTENT_CHANGED,      

    // Description: Window should send XWObject event after receiving this message
    // Agruments:   event id is sent as WPARAM
    WM_XWUI_CALLBACK_EVENT_REQUEST,

    // Description: Inform animation timer callback that timer has expired
    // Agruments:   animation id is sent as WPARAM
    WM_XWUI_ANIMATION_TIMER_EVENT,

    // Description: Inform animation value callback that value has changed
    // Agruments:   animation id is sent as WPARAM
    WM_XWUI_ANIMATION_VALUE_EVENT,

    // Description: Inform animation callback that animation has completed
    // Agruments:   animation id is sent as WPARAM
    WM_XWUI_ANIMATION_COMPLETED,

    // Description: Inform content loading callback that download has completed
    // Agruments:   content id is sent as WPARAM, path to file is sent as LPARAM
    WM_XWUI_URL_CONTENT_LOADED,

    // Description: Inform content loading callback that download has failed
    // Agruments:   content id is sent as WPARAM, error reason is sent as LPARAM
    WM_XWUI_URL_CONTENT_LOAD_FAILED,

    ////////////////////////////
    // Graphics items messages:

    // Description: send event to graphics item
    // Agruments:   graphics item id as WPARAM, event argument as LPARAM
    // Returns: TRUE if event delivered or FALSE if item not found
    WM_XWUI_GITEM_EVENT,

    // Description: inform parent window to start mouse capture and route all mouse 
    //              traffic to graphics item
    // Arguments: graphics item id is sent as WPARAM
    WM_XWUI_GITEM_SET_MOUSE_CAPTURE,

    // Description: inform parent window to stop mouse capture
    // Agruments:   none
    WM_XWUI_GITEM_RESET_MOUSE_CAPTURE,

    // Description: graphics item content has changed, full UI update is needed
    // Agruments:   none
    WM_XWUI_GITEM_CONTENT_CHANGED,

    // Description: send shared GDI cache to be used by XGraphicsItemWindow
    // Arguments: GDI cache pointer is sent as LPARAM
    WM_XWUI_GITEM_SET_SHARED_GDI_CACHE,

    // Description: read shared GDI cache from window if set
    // Arguments: pointer to GDI cache pointer is sent as LPARAM, caller is responsible 
    //            to Release returned reference
    // Returns: NOERROR in case of success or error code otherwise
    WM_XWUI_GITEM_GET_SHARED_GDI_CACHE,

    // Description: show tooltip window
    // Arguments: tooltip text is sent as LPARAM
    WM_XWUI_GITEM_SHOW_TOOLTIP,

    // Description: show graphics item context menu
    // Arguments: graphics item id is sent as WPARAM, menu position is sent as LPARAM
    WM_XWUI_GITEM_SHOW_CONTEXT_MENU,

    WM_XWUI_MESSAGES_LAST      // must be the last
};

/////////////////////////////////////////////////////////////////////

#endif // _XWMESSAGES_H_

