// XWObject event handling (object messages processing)
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWOBJECTEVENTMAP_H_
#define _XWOBJECTEVENTMAP_H_

// NOTE: using similar idea as in XWEventMap, but object event map is
//       supposed to handle messages between objects, while XWEventMap
//       is meant for processing window messages. Only XWObject based 
//       classes can be an event listener.

/////////////////////////////////////////////////////////////////////
// forward declarations
class XWObject;

/////////////////////////////////////////////////////////////////////
// convenience macro
#define XWOBJECT_EVENT_HANDLER(_ClassName, _MehtodName, _ObjPtr) \
    XWObjectEventDelegate::createDelegate<_ClassName, &##_ClassName##::_MehtodName>(_ObjPtr)

/////////////////////////////////////////////////////////////////////
// XWObjectEventDelegate - event handling delegate 

class XWObjectEventDelegate
{
public: // construction/destruction
    XWObjectEventDelegate() : m_pObjPtr(0), m_pXWObjPtr(0), m_pDelegate(0) {}
    ~XWObjectEventDelegate(){}

public: // create delegate
    template <typename _T, bool (_T::*_eventHandler)(unsigned long, XWObject*)>
    static XWObjectEventDelegate createDelegate(_T* objPtr)
    {
        XWASSERT(objPtr);

        XWObjectEventDelegate xwDelegate;
        xwDelegate.m_pObjPtr = objPtr;
        xwDelegate.m_pDelegate = &handlerStub<_T, _eventHandler>;

        // NOTE: if delegate is not derivied form XWObject this will
        //       not compile
        xwDelegate.m_pXWObjPtr = objPtr;

        return xwDelegate;
    }

public: // convenience operator
    bool operator()(unsigned long eventId, XWObject* sender) const
    {
        return (*m_pDelegate)(m_pObjPtr, eventId, sender);
    }

public: // XWObject pointer
    XWObject*   xwobject() const { return m_pXWObjPtr; }

private: // stub method
    template <typename _T, bool (_T::*_eventHandler)(unsigned long, XWObject*)>
    static bool handlerStub(void* objPtr, unsigned long eventId, XWObject* sender)
    {
        _T* obj = static_cast<_T*>(objPtr);
        return (obj->*_eventHandler)(eventId, sender); 

    }

private: // types
    typedef bool (*delegatePtr)(void*, unsigned long, XWObject*);

private: // data
    void*           m_pObjPtr;      // NOTE: this has to be void pointer, otherwise memory 
                                    //       will be corrupted while calling delegate method
    XWObject*       m_pXWObjPtr;
    delegatePtr     m_pDelegate;
};

// XWObjectEventDelegate
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// XWObjectEventMap - object event handling

class XWObjectEventMap
{
public: // construction/destruction
    XWObjectEventMap();
    ~XWObjectEventMap();

public: // interface
    void    clear();

public: // event handlers
    unsigned long   addEventHandler(unsigned long eventId, const XWObjectEventDelegate& handler);
    void            removeEventHandler(unsigned long handlerId);

public: // send event
    void    sendEvent(unsigned long eventId, XWObject* sender);

public: // remove object event handlers from map
    void    removeObjectEventHandler(unsigned long eventId, XWObject* obj);
    void    removeAllObjectHandlers(XWObject* obj);

public: // get unused event id
    unsigned long   unusedEventId() const;

private: // handler data
    struct XWObjectHandlerRef
    {
        unsigned long           handlerId;
        XWObjectEventDelegate   eventHandler;
    };

    // types
    typedef std::vector<XWObjectHandlerRef>     XWObjectHandlerRefVect;
    typedef std::map<unsigned long, XWObjectHandlerRefVect> XWObjectHandlersMap;

private: // data
    XWObjectHandlersMap     m_vEventHandlers;
    unsigned long           m_lNextHandlerId;
};

// XWObjectEventMap
/////////////////////////////////////////////////////////////////////

#endif // _XWOBJECTEVENTMAP_H_

