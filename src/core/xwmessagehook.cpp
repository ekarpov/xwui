// Message hooks
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xwmessagehook.h"

/////////////////////////////////////////////////////////////////////
// message hooks
static std::list<IXWMessageHook*>*      s_pXWMessageHooks = 0;

/////////////////////////////////////////////////////////////////////
// message hooks
void sXWUIAddMessageHook(IXWMessageHook* messageHook)
{
    XWASSERT(messageHook);
    if(messageHook == 0) return;

    // remove hook if already added
    sXWUIRemoveMessageHook(messageHook->messageHookId());

    // allocate hooks if needed
    if(s_pXWMessageHooks == 0)
    {
        s_pXWMessageHooks = new std::list<IXWMessageHook*>();

        XWASSERT(s_pXWMessageHooks);
        if(s_pXWMessageHooks == 0) return;
    }

    // insert hook in front
    s_pXWMessageHooks->push_front(messageHook);
}

void sXWUIRemoveMessageHook(unsigned long hookId)
{
    // remove hook by id
    if(s_pXWMessageHooks)
    {
        std::list<IXWMessageHook*>::iterator it = s_pXWMessageHooks->begin();

        // remove all hooks with the same id
        while(it != s_pXWMessageHooks->end())
        {
            // check id
            if((*it)->messageHookId() == hookId)
            {
                // remove 
                it = s_pXWMessageHooks->erase(it);

            } else
            {
                // next
                ++it;
            }
        }

        // delete hooks if empty
        if(s_pXWMessageHooks->size() == 0)
        {
            delete s_pXWMessageHooks;
            s_pXWMessageHooks = 0;
        }
    }
}

void sXWUIClearMessageHooks()
{
    if(s_pXWMessageHooks)
    {
        delete s_pXWMessageHooks;
        s_pXWMessageHooks = 0;
    }
}

bool sXWUIProcessMessageHooks(MSG* pmsg)
{
    XWASSERT(pmsg);
    if(pmsg == 0) return false;

    // check if there are any hooks
    if(s_pXWMessageHooks)
    {
        for(std::list<IXWMessageHook*>::iterator it = s_pXWMessageHooks->begin(); 
                                                 it != s_pXWMessageHooks->end(); ++it)
        {
            // process message
            if((*it)->processHookMessage(pmsg))
            {
                // message has been consumed, stop other processing
                return true;
            }
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////
