// Window events map (window messages processing)
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWEVENTMAP_H_
#define _XWEVENTMAP_H_

/////////////////////////////////////////////////////////////////////
// convenience macro
#define XEVENT_HANDLER(_ClassName, _MehtodName, _ObjPtr) \
    XWEventDelegate::createDelegate<_ClassName, &##_ClassName##::_MehtodName>(_ObjPtr)

/////////////////////////////////////////////////////////////////////
// XWEvent - window event
struct XWEvent
{
    HWND        hWnd;
    UINT        uMsg;
    WPARAM      wParam;
    LPARAM      lParam;

    // default values
    XWEvent() : hWnd(0), uMsg(0), wParam(0), lParam(0){}
    XWEvent(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) : 
                 hWnd(hwnd), uMsg(umsg), wParam(wparam), lParam(lparam){}
};

// XWEvent
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// XWEventMask - window event mask

class XWEventMask
{
public: // construction/destruction
    XWEventMask();
    ~XWEventMask();

public: // convenience mask constructors
    XWEventMask(const HWND hWnd);
    XWEventMask(const UINT uMsg);
    XWEventMask(const HWND hWnd, const UINT uMsg);

public: // set mask values
    void    setWindowHandle(const HWND hWnd);
    void    setWindowMessage(const UINT uMsg);
    void    setWParam(const WPARAM wParam);
    void    setWParamLoWord(const WORD word);
    void    setWParamHiWord(const WORD word);
    void    setLParam(const LPARAM wParam);
    void    setLParamLoWord(const WORD word);
    void    setLParamHiWord(const WORD word);

public: // reset mask
    void    reset();

public: // match events
    bool matchEvent(const XWEvent& xwEvent);
    static bool sMatchEvent(const XWEvent& xwEvent, const XWEventMask& mask);

public: // compare masks
    bool operator==(const XWEventMask& mask) const
    {
        return (m_hWnd == mask.m_hWnd) &&
            (m_uMsg == mask.m_uMsg) &&
            (m_wParam == mask.m_wParam) &&
            (m_lParam == mask.m_lParam) &&
            (m_uFlags == mask.m_uFlags);
    }

public: // mask flags
    enum TFlags
    {
        eWindowHandle   = 0x0001,
        eWindowMessage  = 0x0002,
        eLoWordWParam   = 0x0004,
        eHiWordWParam   = 0x0008,
        eLoWordLParam   = 0x0010,
        eHiWordLParam   = 0x0020
    };

public: // data
    HWND            m_hWnd;
    UINT            m_uMsg;
    WPARAM          m_wParam;
    LPARAM          m_lParam;
    unsigned short  m_uFlags;
};

// XWEventMask
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// NOTE: this code is based on idea from: 
// http://www.codeproject.com/KB/cpp/ImpossiblyFastCppDelegate.aspx

/////////////////////////////////////////////////////////////////////
// XWEventDelegate - event handling delegate 

class XWEventDelegate
{
public: // construction/destruction
    XWEventDelegate() : m_pObjPtr(0), m_pDelegate(0) {}
    ~XWEventDelegate(){}

public: // create delegate
    template <typename _T, LRESULT (_T::*_eventHandler)(const XWEvent&)>
    static XWEventDelegate createDelegate(_T* objPtr)
    {
        XWEventDelegate xwDelegate;
        xwDelegate.m_pObjPtr = objPtr;
        xwDelegate.m_pDelegate = &handlerStub<_T, _eventHandler>;

        return xwDelegate;
    }

public: // convenience operator
    LRESULT operator()(const XWEvent& xwEvent) const
    {
        return (*m_pDelegate)(m_pObjPtr, xwEvent);
    }

private: // stub method
    template <typename _T, LRESULT (_T::*_eventHandler)(const XWEvent&)>
    static LRESULT handlerStub(void* objPtr, const XWEvent& xwEvent)
    {
        _T* obj = static_cast<_T*>(objPtr);
        return (obj->*_eventHandler)(xwEvent); 

    }

private: // types
    typedef LRESULT (*delegatePtr)(void*, const XWEvent&);

private: // data
    void*       m_pObjPtr;
    delegatePtr m_pDelegate;
};

// XWEventDelegate
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// XWEventMap - event map

class XWEventMap
{
public: // construction/destruction
    XWEventMap();
    ~XWEventMap();

public: // add handlers
    void    addEventHandler(const XWEventMask& mask, const XWEventDelegate& handler);
    void    removeEventHandlers(const XWEventMask& mask);

public: // find handlers for events
    std::vector<XWEventDelegate>    findHandlers(const XWEvent& xwEvent);

private: // handler data
    struct _HandlerRef
    {
        XWEventMask     xwEventMask;
        XWEventDelegate xwEventHandler;
    };

private: // types
    typedef std::vector<_HandlerRef>            _HandlerRefVect;

private: // data
    std::map<UINT, _HandlerRefVect>              m_vEventHandlers;
};

// XWEventMap
/////////////////////////////////////////////////////////////////////

#endif // _XWEVENTMAP_H_

