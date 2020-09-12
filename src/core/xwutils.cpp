// Windows utility functions
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xwutils.h"

/////////////////////////////////////////////////////////////////////
// global variables
static HFONT g_sXWDefaultFont = 0;
static DWORD g_sWindowsMajorVersion = 0;
static DWORD g_sWindowsMinorVersion = 0;

/////////////////////////////////////////////////////////////////////
// default locale reference
static wchar_t*     s_pDefaultLocaleName = 0;
static wchar_t      s_pDefaultLocaleBuffer[LOCALE_NAME_MAX_LENGTH];

/////////////////////////////////////////////////////////////////////
// cursors

// system cursors will be loaded here
static HCURSOR      g_hSystemCursors[XWUtils::eSystemCursorCount] = {0, 0, 0, 0, 0};

// currsor mapping
static LPCWSTR      g_hSystemCursorNames[XWUtils::eSystemCursorCount] = 
{
    IDC_ARROW,      // eCursorArrow
    IDC_IBEAM,      // eCursorIBeam
    IDC_WAIT,       // eCursorWait
    IDC_CROSS,      // eCursorCross
    IDC_HAND,       // eCursorHand
    IDC_SIZENS,     // eCursorResizeV
    IDC_SIZEWE      // eCursorResizeH
};

/////////////////////////////////////////////////////////////////////
// export functions for dynamic loading
typedef int (WINAPI *GetUserDefaultLocaleNamePtr)(LPWSTR, int);
typedef int (WINAPI *GetTimeFormatExPtr)(LPCWSTR, DWORD, const SYSTEMTIME*, LPCWSTR, LPWSTR, int); 
typedef int (WINAPI *GetDateFormatExPtr)(LPCWSTR, DWORD, const SYSTEMTIME*, LPCWSTR, LPWSTR, int, LPCWSTR); 

/////////////////////////////////////////////////////////////////////
// dynamic functions
static HINSTANCE g_hKernel32Library = 0;
static GetUserDefaultLocaleNamePtr  g_pGetUserDefaultLocaleNameFunc = 0;
static GetTimeFormatExPtr           g_pGetTimeFormatExFunc = 0;
static GetDateFormatExPtr           g_pGetDateFormatExFunc = 0;

/////////////////////////////////////////////////////////////////////
// helper functions
static void _xwutilsReadWindowsVersion()
{
    // ignore if already loaded
    if(g_sWindowsMajorVersion != 0) return;

    OSVERSIONINFO osvi;
    ::ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    // NOTE: for OS version refer: 
    //       http://msdn.microsoft.com/en-us/library/ms724832(v=VS.85).aspx

    // get version
    if(::GetVersionExW(&osvi) == 0)
    {
        // failed
        XWTRACE_WERR_LAST("XWUtils: failed to get system version info");
        return;
    }

    // copy result
    g_sWindowsMajorVersion = osvi.dwMajorVersion;
    g_sWindowsMinorVersion = osvi.dwMinorVersion;
}

HINSTANCE _xwuiGetKernel32Library()
{
    // load kernel32 DLL
    if(g_hKernel32Library == 0)
    {
        g_hKernel32Library = ::LoadLibraryW(L"kernel32.dll");
        if(g_hKernel32Library == 0)
        {
            XWTRACE_WERR_LAST("XWUtils: failed to load kernel32.dll");
            return 0;
        }
    }

    return g_hKernel32Library;
}

GetUserDefaultLocaleNamePtr _xwuiGetUserDefaultLocaleNamePtr()
{
    // locate function
    if(g_pGetUserDefaultLocaleNameFunc == 0)
    {
        g_pGetUserDefaultLocaleNameFunc = (GetUserDefaultLocaleNamePtr)::GetProcAddress(_xwuiGetKernel32Library(), "GetUserDefaultLocaleName");
        if(g_pGetUserDefaultLocaleNameFunc == 0)
        {
            XWTRACE_WERR_LAST("XWUtils: failed to locate GetUserDefaultLocaleName in kernel32.dll");
            return 0;
        }
    }

    // function pointer
    return g_pGetUserDefaultLocaleNameFunc;
}

GetTimeFormatExPtr _xwuiGetTimeFormatExPtr()
{
    // locate function
    if(g_pGetTimeFormatExFunc == 0)
    {
        g_pGetTimeFormatExFunc = (GetTimeFormatExPtr)::GetProcAddress(_xwuiGetKernel32Library(), "GetTimeFormatEx");
        if(g_pGetTimeFormatExFunc == 0)
        {
            XWTRACE_WERR_LAST("XWUtils: failed to locate GetTimeFormatEx in kernel32.dll");
            return 0;
        }
    }

    // function pointer
    return g_pGetTimeFormatExFunc;
}

GetDateFormatExPtr _xwuiGetDateFormatExPtr()
{
    // locate function
    if(g_pGetDateFormatExFunc == 0)
    {
        g_pGetDateFormatExFunc = (GetDateFormatExPtr)::GetProcAddress(_xwuiGetKernel32Library(), "GetDateFormatEx");
        if(g_pGetDateFormatExFunc == 0)
        {
            XWTRACE_WERR_LAST("XWUtils: failed to locate GetDateFormatEx in kernel32.dll");
            return 0;
        }
    }

    // function pointer
    return g_pGetDateFormatExFunc;
}

/////////////////////////////////////////////////////////////////////
// XWUtils

/////////////////////////////////////////////////////////////////////
// Windows OS version
/////////////////////////////////////////////////////////////////////
int XWUtils::sGetWindowsMajorVersion()
{
    // load version if needed
    _xwutilsReadWindowsVersion();

    return g_sWindowsMajorVersion;
}

int XWUtils::sGetWindowsMinorVersion()
{
    // load version if needed
    _xwutilsReadWindowsVersion();

    return g_sWindowsMinorVersion;
}

bool XWUtils::sIsWindowsVistaOrAbove()
{
    return (sGetWindowsMajorVersion() >= 6);
}

bool XWUtils::sIsWindowsSevenOrAbove()
{
    // NOTE: http://msdn.microsoft.com/en-us/library/ms724833(v=VS.85).aspx

    return (sGetWindowsMajorVersion() >= 6) && (sGetWindowsMinorVersion() >= 1);
}

/////////////////////////////////////////////////////////////////////
// get module name from handle
/////////////////////////////////////////////////////////////////////
bool XWUtils::sGetModuleName(HMODULE hModule, std::vector<wchar_t>& moduleName)
{
    moduleName.resize(MAX_PATH);

    // read module name
    DWORD dwLength = ::GetModuleFileNameW(hModule, moduleName.data(), (DWORD)moduleName.size());
    while(dwLength != 0 && dwLength == moduleName.size() && moduleName.size() < 32768U)
    {
        // double buffer size and read again
        moduleName.reserve(2 * moduleName.size());
        dwLength = ::GetModuleFileNameW(hModule, moduleName.data(), (DWORD)moduleName.size());
    }

    // mark actual data size (zero in case of error)
    moduleName.resize(dwLength);

    // check if failed
    if(dwLength == 0)
    {
        XWTRACE_WERR_LAST("XWUtils::sGetModuleName failed to read module name");
        return false;
    }

    // find first slash
    for(size_t idx = moduleName.size() - 1; idx != 0; --idx)
    {
        if(moduleName[idx] == L'/' || moduleName[idx] == L'\\')
        {
            // check if path is valid
            if(idx == moduleName.size() - 1)
            {
                XWTRACE("XWUtils::sGetModuleName fail name is not valid");
                moduleName.resize(0);
                return false;
            }

            // remove beginning
            moduleName.erase(moduleName.begin(), moduleName.begin() + idx + 1);

            // stop
            break;
        }
    }

    // append end if string
    moduleName.push_back(0);

    return true;
}

/////////////////////////////////////////////////////////////////////
// default fonts
/////////////////////////////////////////////////////////////////////
bool XWUtils::sGetDefaultFontProps(LOGFONT& fnt)
{
    // metrics structure
    NONCLIENTMETRICS ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICS);

    // NOTE: in XP and below we need to extract sizeof iPaddedBorderWidth
    //       from structure size or SystemParametersInfo will fail
    //       see Remarks section in http://msdn.microsoft.com/en-us/library/ms724506(VS.85).aspx

    // check version
    if(!XWUtils::sIsWindowsVistaOrAbove())
    {
        ncm.cbSize -= sizeof(ncm.iPaddedBorderWidth);
    }

    // get metrics
    if(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0) == 0)
    {
        // failed
        XWTRACE_WERR_LAST("sGetDefaultFontProps: Failed to get system parameters");
        return false;
    }

    // copy font
    fnt = ncm.lfMessageFont;

    // enable antialiasing by default ?
    //fnt.lfQuality = ANTIALIASED_QUALITY; // CLEARTYPE_QUALITY ???

    return true;
}

HFONT XWUtils::sCreateDefaultFont()
{
    // get font details
    LOGFONT fnt;
    if(!XWUtils::sGetDefaultFontProps(fnt)) return false;

    // create font
    HFONT hFont = ::CreateFontIndirect(&fnt);

    // check result
    if(!hFont)
    {
        // failed
        XWTRACE_WERR_LAST("sCreateDefaultFont: Failed to create font");
        return 0;
    }

    return hFont;
}

HFONT XWUtils::sGetDefaultFont()
{
    // create default font if needed
    if(g_sXWDefaultFont == 0)
    {
        g_sXWDefaultFont = sCreateDefaultFont();
    }

    return g_sXWDefaultFont;
}

/////////////////////////////////////////////////////////////////////
// default colors
/////////////////////////////////////////////////////////////////////
COLORREF XWUtils::sGetDefaultTextColor()
{
    // TODO:
    return RGB(0, 0, 0);
}

COLORREF XWUtils::sGetDefaultHoverTextColor()
{
    // TODO:
    return RGB(128, 128, 128);
}

COLORREF XWUtils::sGetDefaultBackgroundColor()
{
    return RGB(236, 233, 216);
}

/////////////////////////////////////////////////////////////////////
// locale functions
/////////////////////////////////////////////////////////////////////
const wchar_t* XWUtils::sGetDefaultLocaleName()
{
    // read locale first if needed
    if(s_pDefaultLocaleName == 0)
    {
        // NOTE: GetUserDefaultLocaleName is only available on Windows Vista and above
        if(sIsWindowsVistaOrAbove())
        {
            // get function
            GetUserDefaultLocaleNamePtr GetUserDefaultLocaleNameFunc = _xwuiGetUserDefaultLocaleNamePtr();
            if(GetUserDefaultLocaleNameFunc)
            {
                // read locale
                if(GetUserDefaultLocaleNameFunc((LPWSTR)s_pDefaultLocaleBuffer, LOCALE_NAME_MAX_LENGTH) != 0)
                {
                    // set reference
                    s_pDefaultLocaleName = s_pDefaultLocaleBuffer;
                }

            } else
            {
                XWTRACE("XWUtils: failed to locate GetUserDefaultLocaleName in kernel32.dll");
            }
        }

        // check if we are running XP or locale load failed
        if(s_pDefaultLocaleName == 0)
        {
            // get system LANGID 
            LANGID langid = ::GetSystemDefaultLangID();

            // convert to string
            DWORD dwCount = ::GetLocaleInfoW(langid, LOCALE_SNAME, s_pDefaultLocaleBuffer, sizeof(s_pDefaultLocaleBuffer) / sizeof(WCHAR));
            if(dwCount != 0)
            {
                // set reference
                s_pDefaultLocaleName = s_pDefaultLocaleBuffer;

            } else
            {
                XWTRACE("XWUtils: failed to convert LANGID to locale name");
            }
        }

        // check if set
        if(s_pDefaultLocaleName == 0)
        {
            XWTRACE("XWUtils: failed to load default locale from system, setting default as en-US");

            // use some default
            s_pDefaultLocaleName = L"en-US";
        }
    }

    return s_pDefaultLocaleName;
}

int XWUtils::sFormatTimeString(const wchar_t* locale, const SYSTEMTIME& st, WCHAR* timeOut, int bufferSize)
{
    // validate input
    if(st.wHour > 23 || st.wMinute > 59)
    {
        XWASSERT1(0, "XWUtils: invalid time format");
        return 0;
    }

    // validate buffer
    if(bufferSize > 0 && timeOut == 0)
    {
        XWASSERT1(0, "XWUtils: buffer not set");
        return 0;
    }

    // use default locale if not set
    if(locale == 0)
        locale = sGetDefaultLocaleName();

    // format flags
    DWORD dwFlags = TIME_NOSECONDS;

    // NOTE: GetTimeFormatEx is only available on Windows Vista and above
    if(sIsWindowsVistaOrAbove())
    {
        // check if loaded
        GetTimeFormatExPtr GetTimeFormatExFunc = _xwuiGetTimeFormatExPtr();
        if(GetTimeFormatExFunc)
        {
            // format
            int ret = GetTimeFormatExFunc(locale, dwFlags, &st, 0, timeOut, bufferSize);
            if(ret > 0) return ret;
        }
    }

    // convert locale name to id
    LCID lcid = ::LocaleNameToLCID(locale, 0);
    if(lcid == 0)
    {
        XWTRACE_WERR_LAST("XWUtils: failed to convert locale name to locale id, will be using system default");
        lcid = ::GetSystemDefaultLCID();
    }

    // use Windows XP style GetTimeFormatW
    int ret = ::GetTimeFormatW(lcid, dwFlags, &st, 0, 0, 0);
    if(ret > 0) return ret;

    // if all failed use default format
    int requiredSize = (st.wHour > 9) ? 2 : 1
                     + (st.wMinute > 9) ? 2 : 1
                     + 2;

    // check if we need only required size
    if(bufferSize == 0) return requiredSize;

    // check if there is enough size
    if(bufferSize < requiredSize)
    {
        XWTRACE("XWUtils: not enough buffer to format time string");
        return 0;
    }

    // format
    return ::swprintf(timeOut, bufferSize, L"%d:%d", st.wHour, st.wMinute);
}

int XWUtils::sFormatTimeString(const wchar_t* locale, const SYSTEMTIME& st, std::vector<WCHAR>& timeOut)
{
    // reset output
    timeOut.clear();

    // validate input
    if(st.wHour > 23 || st.wMinute > 59)
    {
        XWASSERT1(0, "XWUtils: invalid time format");
        return 0;
    }

    // count space required first
    int sizeNeeded = sFormatTimeString(locale, st, 0, 0);
    if(sizeNeeded > 0)
    {
        // reserve enough space
        timeOut.resize(sizeNeeded);

        // format
        sFormatTimeString(locale, st, timeOut.data(), (int)timeOut.size());

    } else
    {
        // reserve enough space
        timeOut.resize(16);

        // basic format 
        int size = ::swprintf(timeOut.data(), timeOut.size(), L"%d:%d", st.wHour, st.wMinute);

        // resize output
        timeOut.resize(size);
    }

    return (int)timeOut.size();
}

int XWUtils::sFormatDateString(const wchar_t* locale, const SYSTEMTIME& st, WCHAR* timeOut, int bufferSize)
{
    // validate input
    if(st.wDay > 31 || st.wMonth > 12 || st.wYear > 9999)
    {
        XWASSERT1(0, "XWUtils: invalid time format");
        return 0;
    }

    // validate buffer
    if(bufferSize > 0 && timeOut == 0)
    {
        XWASSERT1(0, "XWUtils: buffer not set");
        return 0;
    }

    // use default locale if not set
    if(locale == 0)
        locale = sGetDefaultLocaleName();

    // format flags
    DWORD dwFlags = DATE_SHORTDATE;

    // NOTE: GetDateFormatEx is only available on Windows Vista and above
    if(sIsWindowsVistaOrAbove())
    {
        // check if loaded
        GetDateFormatExPtr GetDateFormatExFunc = _xwuiGetDateFormatExPtr();
        if(GetDateFormatExFunc)
        {
            // format
            int ret = GetDateFormatExFunc(locale, dwFlags, &st, 0, timeOut, bufferSize, 0);
            if(ret > 0) return ret;
        }
    }

    // convert locale name to id
    LCID lcid = ::LocaleNameToLCID(locale, 0);
    if(lcid == 0)
    {
        XWTRACE_WERR_LAST("XWUtils: failed to convert locale name to locale id, will be using system default");
        lcid = ::GetSystemDefaultLCID();
    }

    // use Windows XP style GetDateFormatW
    int ret = ::GetDateFormatW(lcid, dwFlags, &st, 0, 0, 0);
    if(ret > 0) return ret;

    // if all failed use default format (dd:mm:yyyy)
    int requiredSize = (st.wDay > 9) ? 2 : 1
                     + (st.wMonth > 9) ? 2 : 1
                     + 4
                     + 3;

    // check if we need only required size
    if(bufferSize == 0) return requiredSize;

    // check if there is enough size
    if(bufferSize < requiredSize)
    {
        XWTRACE("XWUtils: not enough buffer to format date string");
        return 0;
    }

    // format
    return ::swprintf(timeOut, bufferSize, L"%d:%d:%d", st.wDay, st.wMonth, st.wYear);
}

int XWUtils::sFormatDateString(const wchar_t* locale, const SYSTEMTIME& st, std::vector<WCHAR>& dateOut)
{
    // reset output
    dateOut.clear();

    // validate input
    if(st.wDay > 31 || st.wMonth > 12 || st.wYear > 9999)
    {
        XWASSERT1(0, "XWUtils: invalid time format");
        return 0;
    }

    // count space required first
    int sizeNeeded = sFormatDateString(locale, st, 0, 0);
    if(sizeNeeded > 0)
    {
        // reserve enough space
        dateOut.resize(sizeNeeded);

        // format
        sFormatDateString(locale, st, dateOut.data(), (int)dateOut.size());

    } else
    {
        // reserve enough space
        dateOut.resize(16);

        // basic format 
        int size = ::swprintf(dateOut.data(), dateOut.size(), L"%d:%d:%d", st.wDay, st.wMonth, st.wYear);

        // resize output
        dateOut.resize(size);
    }

    return (int)dateOut.size();
}

int XWUtils::sFormatDateTimeString(const wchar_t* locale, const SYSTEMTIME& st, WCHAR* dateTimeOut, int bufferSize)
{
    // check if we need to compute size
    if(bufferSize == 0)
    {
        return sFormatTimeString(locale, st, 0, 0) + 
               sFormatDateString(locale, st, 0, 0) + 2;
    }

    // validate buffer
    XWASSERT1(dateTimeOut, "XWUtils: buffer not set");
    if(dateTimeOut == 0) return 0;

    // format date string first
    int dlen = sFormatDateString(locale, st, dateTimeOut, bufferSize);
    if(dlen == 0) return dlen;

    // add space
    if(dlen + 1 < bufferSize)
    {
        dateTimeOut[dlen] = ' ';
        dlen += 1;
    }

    // append time string 
    int tlen = sFormatTimeString(locale, st, dateTimeOut + dlen, bufferSize - dlen);
    if(tlen == 0) return tlen;

    return dlen + tlen;
}

int XWUtils::sFormatDateTimeString(const wchar_t* locale, const SYSTEMTIME& st, std::vector<WCHAR>& dateTimeOut)
{
    // reset output
    dateTimeOut.clear();

    // count space required first
    int sizeNeeded = sFormatDateTimeString(locale, st, 0, 0);
    if(sizeNeeded > 0)
    {
        // reserve enough space
        dateTimeOut.resize(sizeNeeded);

        // format
        sizeNeeded = sFormatDateTimeString(locale, st, dateTimeOut.data(), (int)dateTimeOut.size());
    }

    return sizeNeeded;
}

/////////////////////////////////////////////////////////////////////
// dialog base units
/////////////////////////////////////////////////////////////////////
LONG XWUtils::sGetDialogBaseUnits(HWND hwnd)
{
    // get window dc
    HDC hdc = ::GetDC(hwnd);

    // get font used by controls
    HFONT hFont = (HFONT) ::SendMessageW(hwnd, WM_GETFONT, 0, 0);

    // select font if any
    if(hFont)
    {
        ::SelectObject(hdc, hFont);
    }

    // get text metrics
    TEXTMETRIC tm;
    ::ZeroMemory( &tm, sizeof( TEXTMETRIC ) );
    ::GetTextMetrics( hdc, &tm );    // compute font size

    // release device context
    ::ReleaseDC(hwnd, hdc); 

    // use default font settings
    return MAKELONG(tm.tmAveCharWidth, tm.tmHeight);
}

/////////////////////////////////////////////////////////////////////
// convert font logical units to twips
/////////////////////////////////////////////////////////////////////
LONG XWUtils::sLogicalUnitsToTwips(HWND hwnd, LONG lunits)
{
    LONG twips;

    // get window dc
    HDC hdc = ::GetDC(hwnd);

    // NOTE: 1440 comes from MSDN documents about CHARFORMAT structure 
    //       yHeight - Character height, in twips (1/1440 of an inch or 1/20 of a printer's point).
    // convert
    twips = ::MulDiv(lunits, 1440, ::GetDeviceCaps(hdc, LOGPIXELSY));

    // release device context
    ::ReleaseDC(hwnd, hdc); 

    return twips;
}

/////////////////////////////////////////////////////////////////////
// windows classes
/////////////////////////////////////////////////////////////////////
bool XWUtils::isClassRegistered(const wchar_t* szClassName)
{
    WNDCLASSEX wc;
    wc.cbSize = sizeof(WNDCLASSEX);

    // try to get class information
    return (::GetClassInfoEx(0, szClassName, &wc) != 0);
}

/////////////////////////////////////////////////////////////////////
// RTL
/////////////////////////////////////////////////////////////////////
bool XWUtils::isSystemDefaultLayoutRTL()
{
    DWORD dwLayout = 0;

    // get system layout
    if(::GetProcessDefaultLayout(&dwLayout))
    {
        // check if RTL
        return (dwLayout == LAYOUT_RTL);
    }

    return false;
}

// rectangles
bool XWUtils::rectIsInside(const RECT& rect, int posX, int posY)
{
    // check if point is inside rect
    return (posX >= rect.left && posX <= rect.right) &&
           (posY >= rect.top && posY <= rect.bottom);
}

bool XWUtils::rectOverlap(const RECT& rect1, const RECT& rect2)
{
    // check if rectangles overlap
    return (rect1.left <= rect2.right && rect1.right >= rect2.left &&
            rect1.top <= rect2.bottom && rect1.bottom >= rect2.top);
}

bool XWUtils::rectIntersect(const RECT& rect1, const RECT& rect2, RECT& rectOut)
{
    // intersect rectangles
    return (::IntersectRect(&rectOut, &rect1, &rect2) != 0);
}

// convert screen to window coordinates
bool XWUtils::screenToClient(HWND hwnd, LONG& posX, LONG& posY)
{
    // check input
    XWASSERT(hwnd);
    if(hwnd == 0) return false;

    POINT pt;
    pt.x = posX;
    pt.y = posY;

    // convert position to window coordinates
    BOOL retVal = ::ScreenToClient(hwnd, &pt);
    if(retVal)
    {
        // copy back
        posX = pt.x;
        posY = pt.y;

    } else
    {
        XWTRACE_WERR_LAST("XWUtils: failed to convert screen coordinates to client");
    }

    return (retVal == TRUE);
}

// position window
void XWUtils::centerWindowOnRect(HWND hwnd, const RECT& rect)
{
    // check input
    XWASSERT(hwnd);
    if(hwnd == 0) return;

    RECT windowRect;

    // get window rect
    if(::GetWindowRect(hwnd, &windowRect))
    {
        int posX = rect.left + (rect.right - rect.left - windowRect.right + windowRect.left) / 2;
        int posY = rect.top + (rect.bottom - rect.top - windowRect.bottom + windowRect.top) / 2;

        // move window
        if(!::SetWindowPos(hwnd, 0, posX, posY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOREDRAW))
        {
            XWTRACE("XWUtils::centerWindowOnWindow failed to move window");
        }

    } else
    {
        XWTRACE("XWUtils::centerWindowOnWindow get window rect failed");
    }
}

void XWUtils::centerWindowOnScreen(HWND hwnd)
{
    // get screen window
    HWND hdesktop = ::GetDesktopWindow();
    if(hdesktop)
    {
        // position window
        centerWindowOnWindow(hwnd, hdesktop);

    } else
    {
        XWTRACE("XWUtils::centerWindowOnScreen get desktop window failed");
    }
}

void XWUtils::centerWindowOnWindow(HWND hwnd, HWND hwndRelative)
{
    // check input
    XWASSERT(hwndRelative);
    if(hwndRelative == 0) return;

    RECT rect;

    // get window rect
    if(::GetWindowRect(hwndRelative, &rect))
    {
        // position window
        centerWindowOnRect(hwnd, rect);

    } else
    {
        XWTRACE("XWUtils::centerWindowOnWindow get window rect failed");
    }
}

HCURSOR XWUtils::getSystemCursor(TXWSystemCursor cursor)
{
    // double check that index is not out of range
    XWASSERT(cursor < sizeof(g_hSystemCursors) / sizeof(g_hSystemCursors[0]));
    if(cursor >= sizeof(g_hSystemCursors) / sizeof(g_hSystemCursors[0])) return 0;

    // NOTE: system cursors do not have to be freed

    // MSDN: The LoadCursor function loads the cursor resource only if it has not been loaded; 
    //       otherwise, it retrieves the handle to the existing resource. 
    //       http://msdn.microsoft.com/en-us/library/windows/desktop/ms648391(v=vs.85).aspx

    // check if cursor is already loaded
    if(g_hSystemCursors[cursor] == 0)
        g_hSystemCursors[cursor] = ::LoadCursorW(NULL, g_hSystemCursorNames[cursor]);

    // return pre-loaded cursor
    return g_hSystemCursors[cursor];
}

// string conversion
void XWUtils::utf8ToUtf16(const char* utf8, size_t length, std::vector<wchar_t>& utf16Out)
{
    // reset output
    utf16Out.clear();

    // check input
    if(utf8 == 0 || length == 0) return;

    // compute required size
    int outSize = ::MultiByteToWideChar(CP_UTF8, 0, utf8, (int)length, 0, 0);
    if(outSize > 0)
    {
        // reserve space
        utf16Out.resize(outSize);

        // convert
        ::MultiByteToWideChar(CP_UTF8, 0, utf8, (int)length, utf16Out.data(), outSize);
    }
}

void XWUtils::asciiToUnicode(const char* stringIn, size_t length, std::wstring& stringOut)
{
    // reset output
    stringOut.clear();

    // check input
    if(stringIn == 0) return;

    // compute length if needed
    if(length == 0)
        length = ::strlen(stringIn);

    // convert
    for(size_t idx = 0; idx < length; ++idx)
    {
        stringOut.push_back(stringIn[idx]);
    }
}

void XWUtils::unicodeToAscii(const wchar_t* stringIn, size_t length, std::string& stringOut)
{
    // reset output
    stringOut.clear();

    // check input
    if(stringIn == 0) return;

    // compute length if needed
    if(length == 0)
        length = ::wcslen(stringIn);

    // convert
    for(size_t idx = 0; idx < length; ++idx)
    {
        stringOut.push_back((char)stringIn[idx]);
    }
}

/////////////////////////////////////////////////////////////////////
// text helpers
/////////////////////////////////////////////////////////////////////
void XWUtils::cutFormattedText(const std::wstring& text, int maxSize, std::wstring& textOut)
{
    // find text end including tags
    bool insideTag = false;
    int textLength = 0;
    for(std::wstring::const_iterator it = text.begin(); it != text.end() && textLength < maxSize; ++it)
    {
        // check if we are inside tag
        if(!insideTag)
        {
            // check for tags
            insideTag = (*it == L'<');

            // update length
            if(!insideTag)
                textLength++;

        } else
        {
            // wait for tag end
            insideTag = (*it != L'>');
        }

        // copy text
        textOut.push_back(*it);
    }
}

void XWUtils::stripFormatTags(const std::wstring& text, std::wstring& textOut)
{
    bool insideTag = false;
    for(std::wstring::const_iterator it = text.begin(); it != text.end(); ++it)
    {
        // check if we are inside tag
        if(!insideTag)
        {
            // check for tags
            insideTag = (*it == L'<');

            // copy text
            if(!insideTag)
                textOut.push_back(*it);

        } else
        {
            // wait for tag end
            insideTag = (*it != L'>');
        }
    }
}

/////////////////////////////////////////////////////////////////////
// release global resources
/////////////////////////////////////////////////////////////////////
void XWUtils::releaseResources()
{
    // destroy default font
    if(g_sXWDefaultFont)
    {
        ::DeleteObject(g_sXWDefaultFont);
        g_sXWDefaultFont = 0;
    }

    // release libraries
    if(g_hKernel32Library)
    {
        ::FreeLibrary(g_hKernel32Library);
        g_pGetUserDefaultLocaleNameFunc = 0;
        g_pGetTimeFormatExFunc = 0;
        g_hKernel32Library = 0;
    }
}

// XWUtils
/////////////////////////////////////////////////////////////////////
