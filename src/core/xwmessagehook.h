// Message hooks
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWMESSAGEHOOK_H_
#define _XWMESSAGEHOOK_H_

/////////////////////////////////////////////////////////////////////
// IXWMessageHook - message hook interface

interface IXWMessageHook
{
public: // interface
    virtual bool            processHookMessage(MSG* pmsg) = 0;
    virtual unsigned long   messageHookId() const = 0;
};

// IXWMessageHook
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// message hooks
void    sXWUIAddMessageHook(IXWMessageHook* messageHook);
void    sXWUIRemoveMessageHook(unsigned long hookId);
void    sXWUIClearMessageHooks();
bool    sXWUIProcessMessageHooks(MSG* pmsg);

/////////////////////////////////////////////////////////////////////

#endif // _XWMESSAGEHOOK_H_

