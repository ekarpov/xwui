// Extended controls common messages
//
/////////////////////////////////////////////////////////////////////

#ifndef _XCTRLMSG_H_
#define _XCTRLMSG_H_

/////////////////////////////////////////////////////////////////////
// custom control messages

// custom message offset
#define WM_XCTRL_MESSAGE_OFFSET             WM_USER + 1000

// layout
#define WM_XCTRL_GET_MINWIDTH_HINT          WM_XCTRL_MESSAGE_OFFSET + 101
#define WM_XCTRL_GET_MINHEIGHT_HINT         WM_XCTRL_MESSAGE_OFFSET + 102
#define WM_XCTRL_GET_WIDTH_HINT             WM_XCTRL_MESSAGE_OFFSET + 103
#define WM_XCTRL_GET_HEIGHT_HINT            WM_XCTRL_MESSAGE_OFFSET + 104

// colors
#define WM_XCTRL_SET_TEXTCOLOR              WM_XCTRL_MESSAGE_OFFSET + 201
#define WM_XCTRL_GET_TEXTCOLOR              WM_XCTRL_MESSAGE_OFFSET + 202
#define WM_XCTRL_SET_BKCOLOR                WM_XCTRL_MESSAGE_OFFSET + 203
#define WM_XCTRL_GET_BKCOLOR                WM_XCTRL_MESSAGE_OFFSET + 204

/////////////////////////////////////////////////////////////////////

#endif // _XCTRLMSG_H_

