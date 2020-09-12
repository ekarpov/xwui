// Core funtionality to manage parent and child objects
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xwobject.h"

/////////////////////////////////////////////////////////////////////
// unique id counter
static unsigned long    g_ulXWObjectUniqueId = 0;

/////////////////////////////////////////////////////////////////////
// XWObject - object interface

XWObject::XWObject(XWObject* parent) :
    m_parentObject(0),
    m_proxyObject(0),
    m_oid(0)
{
    // init id
    m_oid = ::InterlockedIncrement(&g_ulXWObjectUniqueId);

    // set parent object
    setParentObject(parent);
}

XWObject::~XWObject()
{
    // inform connected object about removal
    for(XWConnectionMap::iterator it = m_connectedObjects.begin(); it != m_connectedObjects.end(); ++it)
    {
        it->second->onConnectedObjectRemoved(this);
    }

    // clear connection list
    m_connectedObjects.clear();

    // remove itself from parent
    if(m_parentObject) m_parentObject->_removeChildObject(this);

    // remove child items
    for(std::list<XWObject*>::iterator listIt = m_childObjects.begin();
        listIt != m_childObjects.end(); ++listIt)
    {
        // reset parent to avoid it calling _removeChildObject
        (*listIt)->m_parentObject = 0;

        // delete item
        delete (*listIt);
    }
}

/////////////////////////////////////////////////////////////////////
// event listeners
/////////////////////////////////////////////////////////////////////
unsigned long XWObject::addEventHandler(unsigned long eventId, const XWObjectEventDelegate& handler)
{
    //// validate delegate
    XWASSERT(handler.xwobject());
    if(handler.xwobject() == 0) return 0;

    // check if object is the same
    if(xwoid() != handler.xwobject()->xwoid())
    {
        // add each other to connected object list
        addConnectedObject(handler.xwobject());
        handler.xwobject()->addConnectedObject(this);
    }

    // pass to event map
    return m_eventHandlers.addEventHandler(eventId, handler);
}

void XWObject::removeEventHandler(unsigned long handlerId)
{
    // pass to event map
    m_eventHandlers.removeEventHandler(handlerId);
}

/////////////////////////////////////////////////////////////////////
// event proxy
/////////////////////////////////////////////////////////////////////
void XWObject::addEventProxy(XWObject* proxyObject)
{
    // set proxy object
    m_proxyObject = proxyObject;
}

void XWObject::removeEventProxy()
{
    // reset proxy object
    m_proxyObject = 0;
}

/////////////////////////////////////////////////////////////////////
// add connected object (to be informed when object is deleted)
/////////////////////////////////////////////////////////////////////
void XWObject::addConnectedObject(XWObject* obj)
{
    XWASSERT(obj);
    if(obj == 0) return;

    // add connected object
    _addConnectedObject(obj);

    // add itself to connected object as well
    obj->_addConnectedObject(this);
}

void XWObject::removeConnectedObject(XWObject* obj)
{
    XWASSERT(obj);
    if(obj == 0) return;

    // remove connected object
    onConnectedObjectRemoved(obj);

    // remove itself from connected object as well
    obj->onConnectedObjectRemoved(this);
}

/////////////////////////////////////////////////////////////////////
// remove other object handlers from this object event map
/////////////////////////////////////////////////////////////////////
void XWObject::removeObjectEventHandler(unsigned long eventId, XWObject* obj)
{
    XWASSERT(obj);
    if(obj == 0) return;

    // remove object event handlers
    m_eventHandlers.removeObjectEventHandler(eventId, obj);
}

void XWObject::removeAllObjectHandlers(XWObject* obj)
{
    XWASSERT(obj);
    if(obj == 0) return;

    // remove all object event handlers
    m_eventHandlers.removeAllObjectHandlers(obj);
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XWObject::setParentObject(XWObject* parent, bool transferChildren)
{
    // ignore if parent is already same
    if((m_parentObject ? m_parentObject->xwoid() : 0) == (parent ? parent->xwoid() : 0) ) return;

    // remove itself from old parent
    if(m_parentObject) 
    {
        // remove from parent
        m_parentObject->_removeChildObject(this);
    }

    // set parent reference
    m_parentObject = parent;

    // add itself to parent
    if(parent) parent->_addChildObject(this);

    // transfer all children to parent if needed
    if(transferChildren && parent)
    {
        // replace parent
        for(std::list<XWObject*>::iterator listIt = m_childObjects.begin();
            listIt != m_childObjects.end(); ++listIt)
        {
            // replace parent object
            (*listIt)->m_parentObject = parent;

            // append object to parents list
            (*listIt)->m_childObjects.push_back(*listIt);
        }

        // clear children references
        m_childObjects.clear();
    }
}

/////////////////////////////////////////////////////////////////////
// get unused event id
/////////////////////////////////////////////////////////////////////
unsigned long XWObject::unusedEventId() const
{
    // pass to event map
    return m_eventHandlers.unusedEventId();
}

/////////////////////////////////////////////////////////////////////
// send event to listeners
/////////////////////////////////////////////////////////////////////
void XWObject::sendEvent(unsigned long eventId)
{
    // check if we have event proxy
    if(m_proxyObject)
    {
        // pass to proxy
        m_proxyObject->onProxyObjectEvent(eventId, this);

    } else
    {
        // pass to event map
        m_eventHandlers.sendEvent(eventId, this);
    }
}

/////////////////////////////////////////////////////////////////////
// events
/////////////////////////////////////////////////////////////////////
void XWObject::onChildObjectRemoved(XWObject* child)
{
    // do nothing in default implementation
}

void XWObject::onProxyObjectEvent(unsigned long eventId, XWObject* sender)
{
    // forward event
    sendEvent(eventId);
}

/////////////////////////////////////////////////////////////////////
// connection events
/////////////////////////////////////////////////////////////////////
void XWObject::onConnectedObjectRemoved(XWObject* obj)
{
    XWASSERT(obj);
    if(obj == 0) return;

    // remove object from event listeners
    m_eventHandlers.removeAllObjectHandlers(obj);

    // remove object from map
    XWConnectionMap::iterator it = m_connectedObjects.find(obj->xwoid());
    if(it != m_connectedObjects.end())
    {
        m_connectedObjects.erase(it);
    }
}

/////////////////////////////////////////////////////////////////////
// helper methods
/////////////////////////////////////////////////////////////////////
void XWObject::_addChildObject(XWObject* child)
{
    XWASSERT(child);
    if(child == 0) return;

    // append reference
    m_childObjects.push_back(child);
}

void XWObject::_removeChildObject(XWObject* child)
{
    XWASSERT(child);
    if(child == 0) return;

    // find child
    for(std::list<XWObject*>::iterator listIt = m_childObjects.begin();
        listIt != m_childObjects.end(); ++listIt)
    {
        // check if we found this object
        if((*listIt)->xwoid() == child->xwoid())
        {
            // remove reference from list
            m_childObjects.erase(listIt);

            // handle child item removed in derived classes
            onChildObjectRemoved(child);

            // stop search
            break;
        }
    }
}

void XWObject::_addConnectedObject(XWObject* obj)
{
    // add object to connections map if not there yet
    if(m_connectedObjects.count(obj->xwoid()) == 0)
    {
        // insert
        m_connectedObjects.insert(XWConnectionMap::value_type(obj->xwoid(), obj));
    }
}

// XWObject
/////////////////////////////////////////////////////////////////////

