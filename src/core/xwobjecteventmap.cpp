// XWObject event handling
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xwobjecteventmap.h"

/////////////////////////////////////////////////////////////////////
// XWObjectEventMap - object event handling

XWObjectEventMap::XWObjectEventMap() :
    m_lNextHandlerId(0)
{
}

XWObjectEventMap::~XWObjectEventMap()
{
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XWObjectEventMap::clear()
{
    // clear all listeners
    m_vEventHandlers.clear();
}

/////////////////////////////////////////////////////////////////////
// add handlers
/////////////////////////////////////////////////////////////////////
unsigned long XWObjectEventMap::addEventHandler(unsigned long eventId, const XWObjectEventDelegate& handler)
{
    // listener reference
    XWObjectHandlerRef ref;
    ref.eventHandler = handler;
    ref.handlerId = ++m_lNextHandlerId;

    // check if we have this handler already
    XWObjectHandlersMap::iterator it = m_vEventHandlers.find(eventId);
    if(it != m_vEventHandlers.end())
    {
        // add one more handler
        it->second.push_back(ref);

    } else
    {
        // add new vector
        XWObjectHandlerRefVect vect;
        vect.push_back(ref);

        // add to map
        m_vEventHandlers.insert(XWObjectHandlersMap::value_type(eventId, vect));
    }

    // return handler id
    return ref.handlerId;
}

void XWObjectEventMap::removeEventHandler(unsigned long handlerId)
{
    // loop over all items
    for(XWObjectHandlersMap::iterator it = m_vEventHandlers.begin(); it != m_vEventHandlers.end(); ++it)
    {
        XWObjectHandlerRefVect::iterator href = it->second.begin();
        while(href != it->second.end())
        {
            // check if handler id matches
            if(href->handlerId == handlerId)
            {
                // remove handler
                href = it->second.erase(href);

            } else
            {
                ++href;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////
// send event
/////////////////////////////////////////////////////////////////////
void XWObjectEventMap::sendEvent(unsigned long eventId, XWObject* sender)
{
    // check if we have this handler
    XWObjectHandlersMap::iterator it = m_vEventHandlers.find(eventId);
    if(it != m_vEventHandlers.end())
    {
        XWObjectHandlerRefVect& handlers = it->second;

        // loop over all handlers
        for(XWObjectHandlerRefVect::iterator href = handlers.begin(); href != handlers.end(); ++href)
        {
            // pass to handler
            if(href->eventHandler(eventId, sender)) 
            {
                // stop other processing if handler returns true
                return;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////
// remove object event handlers from map
/////////////////////////////////////////////////////////////////////
void XWObjectEventMap::removeObjectEventHandler(unsigned long eventId, XWObject* obj)
{
    XWASSERT(obj);
    if(obj == 0) return;

    // check if we have this event
    XWObjectHandlersMap::iterator it = m_vEventHandlers.find(eventId);
    if(it != m_vEventHandlers.end())
    {
        XWObjectHandlerRefVect& handlers = it->second;

        // loop over all handlers
        for(XWObjectHandlerRefVect::iterator href = handlers.begin(); href != handlers.end(); )
        {
            // check if handler is from requested object
            if(href->eventHandler.xwobject()->xwoid() == obj->xwoid())
            {
                // remove handler
                href = handlers.erase(href);

            } else
            {
                // next
                ++href;
            }
        }
    }
}

void XWObjectEventMap::removeAllObjectHandlers(XWObject* obj)
{
    XWASSERT(obj);
    if(obj == 0) return;

    // loop over all items
    for(XWObjectHandlersMap::iterator it = m_vEventHandlers.begin(); it != m_vEventHandlers.end(); ++it)
    {
        // loop over all handlers
        for(XWObjectHandlerRefVect::iterator href = it->second.begin(); href != it->second.end(); )
        {
            // check if handler is from removed object
            if(href->eventHandler.xwobject()->xwoid() == obj->xwoid())
            {
                // remove handler
                href = it->second.erase(href);

            } else
            {
                // next
                ++href;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////
// get unused event id
/////////////////////////////////////////////////////////////////////
unsigned long XWObjectEventMap::unusedEventId() const
{
    unsigned long eventId = 1;

    // find unused event id from map
    while(m_vEventHandlers.count(eventId) != 0)
    {
        // try next
        ++eventId;
    }

    return eventId;
}

// XWObjectEventMap
/////////////////////////////////////////////////////////////////////
