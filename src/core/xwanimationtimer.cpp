// Animation timer implementation
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xwanimationtimer.h"

/////////////////////////////////////////////////////////////////////
// constants 

// class name
#define XWUI_ANIMATION_TIMER_WINDOW_CLASS_NAME  L"XWUI_ANIMATION_TIMER_WINDOW_CLASS"

// global instance
static XWAnimationTimer*    g_XWAnimationTimerInstance = 0;

/////////////////////////////////////////////////////////////////////
// IXWAnimationTimerCallback - animation timer callback

/////////////////////////////////////////////////////////////////////
// construction/destruction
/////////////////////////////////////////////////////////////////////
IXWAnimationTimerCallback::IXWAnimationTimerCallback()
{
}

IXWAnimationTimerCallback::~IXWAnimationTimerCallback()
{
}

/////////////////////////////////////////////////////////////////////
// events
/////////////////////////////////////////////////////////////////////
void IXWAnimationTimerCallback::onAnimationTimer(DWORD id)
{
}

void IXWAnimationTimerCallback::onAnimationValue(DWORD id, float value)
{
}

void IXWAnimationTimerCallback::onAnimationCompleted(DWORD id)
{
}

// IXWAnimationTimerCallback
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// XWAnimationTimer - animation timer

/////////////////////////////////////////////////////////////////////
// destruction
/////////////////////////////////////////////////////////////////////
XWAnimationTimer::~XWAnimationTimer()
{
    // close
    _close();
}

/////////////////////////////////////////////////////////////////////
// single instance
/////////////////////////////////////////////////////////////////////
XWAnimationTimer* XWAnimationTimer::instance()
{
    if(g_XWAnimationTimerInstance == 0)
    {
        g_XWAnimationTimerInstance = new XWAnimationTimer();
    }

    return g_XWAnimationTimerInstance;
}

void XWAnimationTimer::closeIstance()
{
    if(g_XWAnimationTimerInstance)
    {
        delete g_XWAnimationTimerInstance;
        g_XWAnimationTimerInstance = 0;
    }
}

/////////////////////////////////////////////////////////////////////
// timers
/////////////////////////////////////////////////////////////////////
bool XWAnimationTimer::startTimerAnimation(unsigned int intervalMs, IXWAnimationTimerCallback* callback, DWORD& idOut)
{
    // check input
    XWASSERT(intervalMs >= 10);
    XWASSERT(callback);
    if(intervalMs < 10 || callback == 0) return false;

    // add data
    _addAnimationData(eTimerAnimation, intervalMs, 0, false, callback, 0, idOut);

    return true;
}

bool XWAnimationTimer::startTimerAnimation(unsigned int intervalMs, HWND callback, DWORD& idOut)
{
    // check input
    XWASSERT(intervalMs >= 10);
    XWASSERT(callback);
    if(intervalMs < 10 || callback == 0) return false;

    // add data
    _addAnimationData(eTimerAnimation, intervalMs, 0, false, 0, callback, idOut);

    return true;
}

bool XWAnimationTimer::startSingleAnimation(unsigned int intervalMs, IXWAnimationTimerCallback* callback, DWORD& idOut)
{
    // check input
    XWASSERT(intervalMs >= 10);
    XWASSERT(callback);
    if(intervalMs < 10 || callback == 0) return false;

    // add data
    _addAnimationData(eTimerAnimation, intervalMs, 0, true, callback, 0, idOut);

    return true;
}

bool XWAnimationTimer::startSingleAnimation(unsigned int intervalMs, HWND callback, DWORD& idOut)
{
    // check input
    XWASSERT(intervalMs >= 10);
    XWASSERT(callback);
    if(intervalMs < 10 || callback == 0) return false;

    // add data
    _addAnimationData(eTimerAnimation, intervalMs, 0, true, 0, callback, idOut);

    return true;
}

/////////////////////////////////////////////////////////////////////
// values
/////////////////////////////////////////////////////////////////////
bool XWAnimationTimer::startValueAnimation(unsigned int intervalMs, const ValueAnimation& animation, IXWAnimationTimerCallback* callback, DWORD& idOut)
{
    // check input
    XWASSERT(intervalMs >= 10);
    XWASSERT(callback);
    if(intervalMs < 10 || callback == 0) return false;

    // add data
    _addAnimationData(eValueAnimation, intervalMs, &animation, false, callback, 0, idOut);

    return true;
}

bool XWAnimationTimer::startValueAnimation(unsigned int intervalMs, const ValueAnimation& animation, HWND callback, DWORD& idOut)
{
    // check input
    XWASSERT(intervalMs >= 10);
    XWASSERT(callback);
    if(intervalMs < 10 || callback == 0) return false;

    // add data
    _addAnimationData(eValueAnimation, intervalMs, &animation, false, 0, callback, idOut);

    return true;
}

bool XWAnimationTimer::getAnimationValue(DWORD id, float& valueOut)
{
    bool ret = false;

    // reset output
    valueOut = 0.0f;

    // enter data protection
    ::EnterCriticalSection(&m_criticalSection);

    // check if item exsists
    _AnimationDataT::const_iterator fit = m_animationData.find(id);
    if(fit != m_animationData.end())
    {
        valueOut = fit->second.value;
        ret = true;
    }

    // leave data protection
    ::LeaveCriticalSection(&m_criticalSection);

    return ret;
}

/////////////////////////////////////////////////////////////////////
// stop animation
/////////////////////////////////////////////////////////////////////
bool XWAnimationTimer::hasAnimation(DWORD id)
{
    // enter data protection
    ::EnterCriticalSection(&m_criticalSection);

    bool ret = (m_animationData.count(id) != 0);

    // leave data protection
    ::LeaveCriticalSection(&m_criticalSection);

    return ret;
}

void XWAnimationTimer::pauseAnimation(DWORD id)
{
    // enter data protection
    ::EnterCriticalSection(&m_criticalSection);

    // check if item exsists
    _AnimationDataT::iterator fit = m_animationData.find(id);
    if(fit != m_animationData.end())
    {
        // mark as paused
        fit->second.paused = true;

        // signal thread
        _sendThreadTask(eThreadItemsChanged);
    }

    // leave data protection
    ::LeaveCriticalSection(&m_criticalSection);
}

void XWAnimationTimer::resumeAnimation(DWORD id)
{
    // enter data protection
    ::EnterCriticalSection(&m_criticalSection);

    // check if item exsists
    _AnimationDataT::iterator fit = m_animationData.find(id);
    if(fit != m_animationData.end())
    {
        // reset flag
        fit->second.paused = false;

        // signal thread
        _sendThreadTask(eThreadItemsChanged);
    }

    // leave data protection
    ::LeaveCriticalSection(&m_criticalSection);
}

void XWAnimationTimer::stopAnimation(DWORD id)
{
    // enter data protection
    ::EnterCriticalSection(&m_criticalSection);

    // check if item exsists
    _AnimationDataT::iterator fit = m_animationData.find(id);
    if(fit != m_animationData.end())
    {
        // remove animation
        m_animationData.erase(fit);

        // signal thread
        _sendThreadTask(eThreadItemsChanged);
    }

    // leave data protection
    ::LeaveCriticalSection(&m_criticalSection);
}

/////////////////////////////////////////////////////////////////////
// hide constructor (only single instance allowed)
/////////////////////////////////////////////////////////////////////
XWAnimationTimer::XWAnimationTimer() :
    m_nextId(1),
    m_waitInterval(INFINITE),
    m_eventWindow(0),
    m_syncEvent(0),
    m_timerThread(0)
{
    // init
    _init();
}

/////////////////////////////////////////////////////////////////////
// window methods
/////////////////////////////////////////////////////////////////////
LRESULT CALLBACK XWAnimationTimer::_windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // check message
    if(uMsg == WM_NCCREATE)
    {
        // store pointer in window structure
        ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);

        // message processed
        return TRUE;
    }

    // get window pointer
    XWAnimationTimer* animationTimer = (XWAnimationTimer*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

    // check if we have window handle
    if(animationTimer == 0 || animationTimer->m_eventWindow == 0)
    {
        // ignore any messages prior WM_NCCREATE
        return ::DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }

    // process messages
    if(uMsg == WM_XWUI_ANIMATION_TIMER_EVENT)
    {
        // pass to event listener
        animationTimer->_processAnimationTimer((DWORD)wParam);
        return TRUE;

    } else if(uMsg == WM_XWUI_ANIMATION_VALUE_EVENT)
    {
        // pass to event listener
        animationTimer->_processAnimationValue((DWORD)wParam);
        return TRUE;

    } else if(uMsg == WM_XWUI_ANIMATION_COMPLETED)
    {
        // pass to event listener
        animationTimer->_processAnimationCompleted((DWORD)wParam);
        return TRUE;
    }

    // default processing
    return ::DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

bool XWAnimationTimer::_registerWindowClass()
{
    static bool IsRegistered = false;

    // check if class already registered
    if(IsRegistered) return true;

    WNDCLASSEX wc;

    // class information
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = _windowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = ::GetModuleHandle(NULL);
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = 0;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = XWUI_ANIMATION_TIMER_WINDOW_CLASS_NAME;
    wc.hIconSm       = NULL;

    // register class
    if(::RegisterClassExW(&wc)) IsRegistered = true;

    // trace error
    if(!IsRegistered)
    {
        XWTRACE_WERR_LAST("XWAnimationTimer: failed to register window class");
    }

    return IsRegistered;
}

void XWAnimationTimer::_processAnimationTimer(DWORD id)
{
    // enter data protection
    ::EnterCriticalSection(&m_criticalSection);

    // check if item exsists
    _AnimationDataT::iterator fit = m_animationData.find(id);
    if(fit != m_animationData.end())
    {
        // pass to callback
        XWASSERT(fit->second.callback);
        if(fit->second.callback)
            fit->second.callback->onAnimationTimer(id);
    }

    // leave data protection
    ::LeaveCriticalSection(&m_criticalSection);
}

void XWAnimationTimer::_processAnimationValue(DWORD id)
{
    // enter data protection
    ::EnterCriticalSection(&m_criticalSection);

    // check if item exsists
    _AnimationDataT::iterator fit = m_animationData.find(id);
    if(fit != m_animationData.end())
    {
        // pass to callback
        XWASSERT(fit->second.callback);
        if(fit->second.callback)
            fit->second.callback->onAnimationValue(id, fit->second.value);
    }

    // leave data protection
    ::LeaveCriticalSection(&m_criticalSection);
}

void XWAnimationTimer::_processAnimationCompleted(DWORD id)
{
    // enter data protection
    ::EnterCriticalSection(&m_criticalSection);

    // check if item exsists
    _AnimationDataT::iterator fit = m_animationData.find(id);
    if(fit != m_animationData.end())
    {
        // pass to callback
        if(fit->second.callback)
            fit->second.callback->onAnimationCompleted(id);

        // remove animation
        XWASSERT(fit->second.completed);
        m_animationData.erase(fit);
    }

    // leave data protection
    ::LeaveCriticalSection(&m_criticalSection);
}

/////////////////////////////////////////////////////////////////////
// thread methods
/////////////////////////////////////////////////////////////////////
DWORD WINAPI XWAnimationTimer::_threadProcFunc(LPVOID threadParameter)
{
    // check input
    XWASSERT(threadParameter);
    if(threadParameter == 0) return 0;

    return ((XWAnimationTimer*)threadParameter)->_threadProc();
}

DWORD XWAnimationTimer::_threadProc()
{
    // process tasks
    while(m_threadTask != eThreadExit)
    {
        // copy timestamp
        LONGLONG waitstart = _getTimeMs();

        // wait for timeout or task
        DWORD ret = ::WaitForSingleObject(m_syncEvent, m_waitInterval);
        if(ret == WAIT_OBJECT_0)
        {
            // process task
            if(!_processThreadTask()) break;

            // process timeouts
            _processItemTimeouts(_roundInterval((DWORD)(_getTimeMs() - waitstart)));

        } else if(ret == WAIT_TIMEOUT)
        {
            // process timeouts
            _processItemTimeouts(m_waitInterval);

        } else
        {
            // wait failed
            XWTRACE_WERR_LAST("XWAnimationTimer: WaitForSingleObject failed");

            // small delay to avoid eating all CPU time
            ::Sleep(100);
        }
    }

    return 0;
}

bool XWAnimationTimer::_processThreadTask()
{
    bool ret = true;

    // enter data protection
    ::EnterCriticalSection(&m_criticalSection);

    // check task
    if(m_threadTask == eThreadExit)
    {
        // return false to stop thread
        ret = false;
    }

    // leave data protection
    ::LeaveCriticalSection(&m_criticalSection);

    return ret;
}

void XWAnimationTimer::_processItemTimeouts(DWORD timeout)
{
    // enter data protection
    ::EnterCriticalSection(&m_criticalSection);

    // loop over all items
    for(_AnimationDataT::iterator it = m_animationData.begin(); 
        it != m_animationData.end(); ++it)
    {
        // ignore completed and paused
        if(it->second.completed || it->second.paused) continue;

        // check if item has expired
        if(it->second.intervalLeft <= timeout)
        {
            // fire item event
            _sendItemEvent(it->first, it->second);

            // remove item if single instance or last event sent
            if(it->second.singleTime || it->second.completed)
            {
                // inform that animation has completed
                if(it->second.hwndCallback)
                    ::PostMessageW(it->second.hwndCallback, WM_XWUI_ANIMATION_COMPLETED, it->first, 0);

                // NOTE: even if callback is window handle we still send event so that any previous
                //       message (e.g. value update) will be delivered properly and client can read final
                //       value while processing them. Message handle in event window will remove animation 
                //       from list.
                ::PostMessageW(m_eventWindow, WM_XWUI_ANIMATION_COMPLETED, it->first, 0);

                // mark as completed
                it->second.completed = true;

            } else
            {
                // reset wait interval
                it->second.intervalLeft = it->second.interval;
            }

        } else
        {
            // substract timeout
            it->second.intervalLeft -= timeout;
        }
    }

    m_waitInterval = 0;

    // select minimum time to wait
    for(_AnimationDataT::iterator it = m_animationData.begin(); 
        it != m_animationData.end(); ++it)
    {
        // ignore completed and paused
        if(it->second.completed || it->second.paused) continue;

        // update minimum
        if(m_waitInterval > it->second.intervalLeft || m_waitInterval == 0)
            m_waitInterval = it->second.intervalLeft;
    }

    // check if there is any interval
    if(m_waitInterval == 0)
    {
        // ignore timeout
        m_waitInterval = INFINITE;
    }

    // leave data protection
    ::LeaveCriticalSection(&m_criticalSection);
}

void XWAnimationTimer::_sendItemEvent(DWORD id, _AnimationData& item)
{
    // check type
    if(item.type == eTimerAnimation)
    {
        // send message
        if(item.hwndCallback)
            ::PostMessageW(item.hwndCallback, WM_XWUI_ANIMATION_TIMER_EVENT, id, 0);
        else
            ::PostMessageW(m_eventWindow, WM_XWUI_ANIMATION_TIMER_EVENT, id, 0);

    } else if(item.type == eValueAnimation)
    {
        // update value
        item.value += item.valueAnimation.step;

        // check value limits
        if(item.value > item.valueAnimation.toValue)
        {
            // check if value is periodic
            if(item.valueAnimation.periodic)
            {
                // jump to interval start
                item.value = item.valueAnimation.fromValue;

            } else
            {
                // jump to interval end
                item.value = item.valueAnimation.toValue;

                // mark item as completed
                item.completed = true;
            }
        }

        // report value
        if(item.hwndCallback)
            ::PostMessageW(item.hwndCallback, WM_XWUI_ANIMATION_VALUE_EVENT, id, 0);
        else
            ::PostMessageW(m_eventWindow, WM_XWUI_ANIMATION_VALUE_EVENT, id, 0);
    }
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XWAnimationTimer::_init()
{
    // init critical section
    ::InitializeCriticalSection(&m_criticalSection);

    // create event
    m_syncEvent = ::CreateEvent(0, FALSE, FALSE, 0);
    if(m_syncEvent == 0)
    {
        XWTRACE_WERR_LAST("XWAnimationTimer: failed to create sync event, animation will not work");
        return;
    }

    // register class
    if(_registerWindowClass()) 
    {
        // NOTE: If you fail to specify HWND_MESSAGE unexpected behaviours may occur. I have used NULL which is NOT correct. 
        //       In my case it caused that MS Excel took 10s or more to load an xls file, while it normally takes less then 
        //       a second when my app was not running!
        // http://msdn.microsoft.com/en-us/library/ms632599(VS.85).aspx#message_only

        // create window
        m_eventWindow = CreateWindowExW(
            0,
            XWUI_ANIMATION_TIMER_WINDOW_CLASS_NAME,
            L"",
            0,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            HWND_MESSAGE, 
            NULL, 
            ::GetModuleHandle(NULL), 
            this);

        if(m_eventWindow == NULL)
        {
            // fatal error
            XWTRACE_WERR_LAST("XWAnimationTimer: failed to create window, interface callbacks will not work");
        }
    }

    // start thread
    m_timerThread = ::CreateThread(0, 0, _threadProcFunc, this, 0, 0);
    if(m_timerThread == 0)
    {
        XWTRACE_WERR_LAST("XWAnimationTimer: failed to start timer thread, animation will not work");
    }
}

void XWAnimationTimer::_close()
{
    // stop thread
    _sendThreadTask(eThreadExit);

    // destroy window if any
    if(m_eventWindow)
    {
        // reset class instance pointer
        ::SetWindowLongPtrW(m_eventWindow, GWLP_USERDATA, 0);

        // destroy window
        ::DestroyWindow(m_eventWindow);

        // reset handle
        m_eventWindow = 0;
    }

    // wait for thread to stop
    DWORD result = ::WaitForSingleObject(m_timerThread, 500);
    if(result != WAIT_OBJECT_0)
    {
        XWTRACE("XWAnimationTimer: timer thread is still running, closing it");

        ::TerminateThread(m_timerThread, 0);
    }

    // destroy thread handle
    ::CloseHandle(m_timerThread);

    // destroy sync event
    ::CloseHandle(m_syncEvent);

    // destroy critical section
    ::DeleteCriticalSection(&m_criticalSection);
}

void XWAnimationTimer::_sendThreadTask(_ThreadTask task)
{
    // set thread task
    m_threadTask = task;

    // singal event
    ::SetEvent(m_syncEvent);
}

void XWAnimationTimer::_addAnimationData(_AnimationType type, unsigned int intervalMs, const ValueAnimation* value, bool singleTime, 
                                  IXWAnimationTimerCallback* callbackPtr, HWND callbackHwnd, DWORD& idOut)
{
    // enter data protection
    ::EnterCriticalSection(&m_criticalSection);

    _AnimationData animationData;

    // fill entry
    animationData.type = type;
    animationData.interval = _roundInterval(intervalMs);
    animationData.intervalLeft = animationData.interval;
    animationData.callback = callbackPtr;
    animationData.hwndCallback = callbackHwnd;
    animationData.singleTime = singleTime;

    // value
    if(value)
    {
        animationData.valueAnimation = *value;
        animationData.value = value->fromValue;
    }

    // new item id
    idOut = _getNextItemId();

    // insert
    m_animationData.insert(_AnimationDataT::value_type(idOut, animationData));

    // signal thread
    _sendThreadTask(eThreadItemsChanged);

    // leave data protection
    ::LeaveCriticalSection(&m_criticalSection);
}

DWORD XWAnimationTimer::_getNextItemId()
{
    // select not used id
    while(m_nextId == 0 || m_animationData.count(m_nextId) != 0)
    {
        ++m_nextId;
    }

    return m_nextId;
}

LONGLONG XWAnimationTimer::_getTimeMs()
{
    FILETIME ft;

    // get file time
    ::GetSystemTimeAsFileTime(&ft);

    // convert to milliseconds
    return (((ULONGLONG) ft.dwHighDateTime) << 32) + ft.dwLowDateTime;
}

DWORD XWAnimationTimer::_roundInterval(DWORD interval)
{
    // round to tens of milliseconds
    return interval - (interval % 10);
}

// XWAnimationTimer
/////////////////////////////////////////////////////////////////////
