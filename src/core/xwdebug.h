// debug helpers
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWDEBUG_H_
#define _XWDEBUG_H_

// tracing
#ifdef _XW_ENABLE_TRACES

    #define XWTRACE(msg)                     _xwtrace(msg)
    #define XWTRACE1(msg, arg)               _xwtrace(msg, arg)
    #define XWTRACE2(msg, arg1, arg2)        _xwtrace(msg, arg1, arg2)
    #define XWTRACE3(msg, arg1, arg2, arg3)  _xwtrace(msg, arg1, arg2, arg3)
    #define XWTRACE4(msg, arg1, arg2, arg3, arg4)  _xwtrace(msg, arg1, arg2, arg3, arg4)

    void _xwtrace(const char* format_str, ...);

    // trace windows errors
    #define XWTRACE_WERR(msg, err)          _xwtrace_werr(msg, err)
    #define XWTRACE_WERR_LAST(msg)          _xwtrace_werr(msg, ::GetLastError())

    void _xwtrace_werr(const char* text, unsigned long err);

    // trace HRESULT
    #define XWTRACE_HRES(msg, res)          _xwtrace_hres(msg, res)

    void _xwtrace_hres(const char* text, HRESULT res);

    // trace callback
    typedef void (*_xwtrace_callback_t)(const char* msg, void* callback_param);

    // trace options
    void    sXWInitTraceOptions(_xwtrace_callback_t callback_ptr, void* callback_param);

#else

    #define XWTRACE(msg)
    #define XWTRACE1(msg, arg)
    #define XWTRACE2(msg, arg1, arg2)
    #define XWTRACE3(msg, arg1, arg2, arg3)
    #define XWTRACE4(msg, arg1, arg2, arg3, arg4)
    #define XWTRACE_WERR(msg, err)
    #define XWTRACE_WERR_LAST(msg)
    #define XWTRACE_HRES(msg, res)
    #define sXWInitTraceOptions(callback_ptr, callback_param)

#endif // _XW_ENABLE_TRACES

// assert
#ifdef _DEBUG

    void _xwassert(const char* file, int line, const char* msg);

    #define XWASSERT(expr)        { if (!(expr)) {_xwassert(__FILE__, __LINE__, 0); __debugbreak(); } }
    #define XWASSERT1(expr,msg)   { if (!(expr)) {_xwassert(__FILE__, __LINE__, (msg)); __debugbreak(); } }

#else

    #define XWASSERT(expr)
    #define XWASSERT1(expr,msg)

#endif // _DEBUG

#endif // _XWDEBUG_H_

    
