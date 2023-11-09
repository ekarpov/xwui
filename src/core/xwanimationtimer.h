// Animation timer implementation
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWANIMATIONTIMER_H_
#define _XWANIMATIONTIMER_H_

/////////////////////////////////////////////////////////////////////

// NOTE: animation timer accepts callback pointer or window as an events callbacks.
//       If window is used it should monitor following events:
//        - WM_XWUI_ANIMATION_TIMER_EVENT
//        - WM_XWUI_ANIMATION_VALUE_EVENT
//        - WM_XWUI_ANIMATION_COMPLETED

// NOTE: callback methods will be called from UI thread (thread that creates animation
//       timer). A message queue must be running in this thread.

/////////////////////////////////////////////////////////////////////
// IXWAnimationTimerCallback - animation timer callback

class IXWAnimationTimerCallback
{
public: // construction/destruction
    IXWAnimationTimerCallback();
    virtual ~IXWAnimationTimerCallback();

public: // events
    virtual void    onAnimationTimer(DWORD id);
    virtual void    onAnimationValue(DWORD id, float value);
    virtual void    onAnimationCompleted(DWORD id);
};

// IXWAnimationTimerCallback
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// XWAnimationTimer - animation timer

class XWAnimationTimer
{
public: // destruction
    ~XWAnimationTimer();

public: // single instance
    static XWAnimationTimer* instance();
    static void closeIstance();

public: // timers
    bool    startTimerAnimation(unsigned int intervalMs, IXWAnimationTimerCallback* callback, DWORD& idOut);
    bool    startTimerAnimation(unsigned int intervalMs, HWND callback, DWORD& idOut);
    bool    startSingleAnimation(unsigned int intervalMs, IXWAnimationTimerCallback* callback, DWORD& idOut);
    bool    startSingleAnimation(unsigned int intervalMs, HWND callback, DWORD& idOut);

public: // values

    // value animation data
    struct ValueAnimation
    {
        float   fromValue;
        float   toValue;
        float   step;
        bool    periodic;

        // constructors
        ValueAnimation() : 
                fromValue(0.0f), toValue(0.0f), step(0.0f), periodic(false) {}
        ValueAnimation(float from, float to, float step, bool repeat) : 
                fromValue(from), toValue(to), step(step), periodic(repeat) {}
    };

    bool    startValueAnimation(unsigned int intervalMs, const ValueAnimation& animation, IXWAnimationTimerCallback* callback, DWORD& idOut);
    bool    startValueAnimation(unsigned int intervalMs, const ValueAnimation& animation, HWND callback, DWORD& idOut);
    bool    getAnimationValue(DWORD id, float& valueOut);

public: // manage animations
    bool    hasAnimation(DWORD id);
    void    pauseAnimation(DWORD id);
    void    resumeAnimation(DWORD id);
    void    stopAnimation(DWORD id);

private: // types
    enum _AnimationType
    {
        eTimerAnimation,
        eValueAnimation
    };

    struct _AnimationData
    {
        _AnimationType  type;

        ValueAnimation  valueAnimation;
        float           value;

        unsigned int    interval;
        unsigned int    intervalLeft;
        bool            singleTime;
        bool            completed;
        bool            paused;

        IXWAnimationTimerCallback*      callback;
        HWND                            hwndCallback;

        // constructors
        _AnimationData() : 
                type(eTimerAnimation), value(0.0f), interval(0), intervalLeft(0), 
                singleTime(false), completed(false), paused(false), callback(0), hwndCallback(0) {}
    };

    enum _ThreadTask
    {
        eThreadTaskNone,
        eThreadItemsChanged,
        eThreadExit
    };

    typedef std::map<DWORD, _AnimationData>     _AnimationDataT;

private: // hide constructor (only single instance allowed)
    XWAnimationTimer();

private: // window methods
    static LRESULT CALLBACK _windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static bool             _registerWindowClass();
    void                    _processAnimationTimer(DWORD id);
    void                    _processAnimationValue(DWORD id);
    void                    _processAnimationCompleted(DWORD id);

private: // thread methods
    static DWORD WINAPI     _threadProcFunc(LPVOID threadParameter);
    DWORD                   _threadProc();
    bool                    _processThreadTask();
    void                    _processItemTimeouts(DWORD timeout);
    void                    _sendItemEvent(DWORD id, _AnimationData& item);

private: // worker methods
    void        _init();
    void        _close();
    void        _sendThreadTask(_ThreadTask task);
    void        _addAnimationData(_AnimationType type, unsigned int intervalMs, const ValueAnimation* value, bool singleTime,
                                  IXWAnimationTimerCallback* callbackPtr, HWND callbackHwnd, DWORD& idOut);
    DWORD       _getNextItemId();
    LONGLONG    _getTimeMs();
    DWORD       _roundInterval(DWORD interval);

private: // data
    _AnimationDataT     m_animationData;
    DWORD               m_nextId;
    DWORD               m_waitInterval;
    _ThreadTask         m_threadTask;
    HWND                m_eventWindow;
    HANDLE              m_syncEvent;
    HANDLE              m_timerThread;
    CRITICAL_SECTION    m_criticalSection;
};

// XWAnimationTimer
/////////////////////////////////////////////////////////////////////

#endif // _XWANIMATIONTIMER_H_

