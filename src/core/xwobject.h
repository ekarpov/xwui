// Core funtionality to manage parent and child objects
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWOBJECT_H_
#define _XWOBJECT_H_

/////////////////////////////////////////////////////////////////////
// XWObject - object interface

class XWObject
{
public: // construction/destruction
    XWObject(XWObject* parent = 0);
    virtual ~XWObject();

public: // unique object id
    unsigned long   xwoid() const { return m_oid; }

public: // event listeners
    unsigned long addEventHandler(unsigned long eventId, const XWObjectEventDelegate& handler);
    void    removeEventHandler(unsigned long handlerId);

public: // event proxy
    void    addEventProxy(XWObject* proxyObject);
    void    removeEventProxy();

public: // add connected object (to be informed when object is deleted)
    void    addConnectedObject(XWObject* obj);
    void    removeConnectedObject(XWObject* obj);

public: // remove other object handlers from this object event map
    void    removeObjectEventHandler(unsigned long eventId, XWObject* obj);
    void    removeAllObjectHandlers(XWObject* obj);

public: // parent object
    virtual void    setParentObject(XWObject* parent, bool transferChildren = false);
    XWObject*       parentObject() const { return m_parentObject; }

public: // get unused event id
    unsigned long   unusedEventId() const;

protected: // send event to listeners
    void    sendEvent(unsigned long eventId);

protected: // events
    virtual void    onChildObjectRemoved(XWObject* child);
    virtual void    onProxyObjectEvent(unsigned long eventId, XWObject* sender);

protected: // connection events
    virtual void    onConnectedObjectRemoved(XWObject* obj);

private: // helper methods
    void    _addChildObject(XWObject* child);
    void    _removeChildObject(XWObject* child);
    void    _addConnectedObject(XWObject* obj);

protected: // connections map
    typedef std::map<unsigned long, XWObject*>  XWConnectionMap;

protected: // data
    XWObject*               m_parentObject;
    XWObject*               m_proxyObject;
    std::list<XWObject*>    m_childObjects;
    XWObjectEventMap        m_eventHandlers;
    unsigned long           m_oid;
    XWConnectionMap         m_connectedObjects;
};

// XWObject
/////////////////////////////////////////////////////////////////////

#endif // _XWOBJECT_H_

