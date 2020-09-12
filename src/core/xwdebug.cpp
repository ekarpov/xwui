// debug helpers
//
/////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <strsafe.h>

#include "xwdebug.h"

#define _XWMAX_DBG_MSG       1024

///// only if traceces defined
#ifdef _XW_ENABLE_TRACES

// callback settings if any
static _xwtrace_callback_t  _xwtrace_callback_ptr = 0;
static void*                _xwtrace_callback_param = 0;

// trace options
void    sXWInitTraceOptions(_xwtrace_callback_t callback_ptr, void* callback_param)
{
    // set pointers
    _xwtrace_callback_ptr = callback_ptr;
    _xwtrace_callback_param = callback_param;
}

// tracing
void _xwtrace(const char* format_str, ...)
{
    char buff[_XWMAX_DBG_MSG];

    va_list args;
    va_start(args, format_str);

    /* format message */
    StringCbVPrintfA(buff, _XWMAX_DBG_MSG, format_str, args);

    va_end(args);

    /* output to debugger  */
    OutputDebugStringA(buff);
    OutputDebugStringA("\n");

    // output also to callback if set
    if(_xwtrace_callback_ptr)
    {
        _xwtrace_callback_ptr(buff, _xwtrace_callback_param);
    }
}


// text error for windows error code
const char* _xwerror_text(unsigned long err_code)
{
    static char lpMsgBuf[_XWMAX_DBG_MSG];

    DWORD retVal = ::FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_MAX_WIDTH_MASK, // ignore line breaks
        NULL,
        err_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&lpMsgBuf,
        sizeof(lpMsgBuf), NULL );

    /* check size */
    if(retVal >= sizeof(lpMsgBuf)) retVal = sizeof(lpMsgBuf) - 1;

    /* make string */
    lpMsgBuf[retVal] = 0;

    return lpMsgBuf;
}

void _xwtrace_werr(const char* text, unsigned long err)
{
    // trace error
    _xwtrace("%s: (Windows error: %d - \"%s\")", text, err, _xwerror_text(err));
}

void _xwtrace_hres(const char* text, HRESULT res)
{
    // trace error
    _xwtrace("%s: (HRESULT: %X - \"%s\")", text, res, _xwerror_text(res));
}

#endif // _XW_ENABLE_TRACES

///// only in debug builds
#ifdef _DEBUG

void _xwassert(const char* file, int line, const char* msg)
{
    char buff[_XWMAX_DBG_MSG];

    /* format assertion message */
    if(msg)
        StringCbPrintfA(buff, _XWMAX_DBG_MSG, "Assertion failed in %s at line %d: %s", file, line, msg);
    else
        StringCbPrintfA(buff, _XWMAX_DBG_MSG, "Assertion failed in %s at line %d", file, line);

    /* just trace message */
    XWTRACE(buff);
}

#endif // _DEBUG

