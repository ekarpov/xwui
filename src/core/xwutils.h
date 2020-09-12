// Windows utility functions
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWUTILS_H_
#define _XWUTILS_H_

/////////////////////////////////////////////////////////////////////
// XWUtils

namespace XWUtils
{
    // Windows OS version
    int     sGetWindowsMajorVersion();
    int     sGetWindowsMinorVersion();
    bool    sIsWindowsVistaOrAbove();
    bool    sIsWindowsSevenOrAbove();

    // get module name from handle
    bool    sGetModuleName(HMODULE hModule, std::vector<wchar_t>& moduleName);

    // default fonts
    bool    sGetDefaultFontProps(LOGFONT& fnt);
    HFONT   sCreateDefaultFont();
    HFONT   sGetDefaultFont();

    // default colors
    COLORREF    sGetDefaultTextColor();
    COLORREF    sGetDefaultHoverTextColor();
    COLORREF    sGetDefaultBackgroundColor();

    // locale functions
    const wchar_t* sGetDefaultLocaleName();
    int     sFormatTimeString(const wchar_t* locale, const SYSTEMTIME& st, WCHAR* timeOut, int bufferSize);
    int     sFormatTimeString(const wchar_t* locale, const SYSTEMTIME& st, std::vector<WCHAR>& timeOut);
    int     sFormatDateString(const wchar_t* locale, const SYSTEMTIME& st, WCHAR* timeOut, int bufferSize);
    int     sFormatDateString(const wchar_t* locale, const SYSTEMTIME& st, std::vector<WCHAR>& dateOut);
    int     sFormatDateTimeString(const wchar_t* locale, const SYSTEMTIME& st, WCHAR* dateTimeOut, int bufferSize);
    int     sFormatDateTimeString(const wchar_t* locale, const SYSTEMTIME& st, std::vector<WCHAR>& dateTimeOut);

    // dialog base units
    LONG    sGetDialogBaseUnits(HWND hwnd);

    // convert font logical units to twips
    LONG    sLogicalUnitsToTwips(HWND hwnd, LONG lunits);

    // windows classes
    bool    isClassRegistered(const wchar_t* szClassName);

    // RTL
    bool    isSystemDefaultLayoutRTL();

    // rectangles
    bool        rectIsInside(const RECT& rect, int posX, int posY);
    bool        rectOverlap(const RECT& rect1, const RECT& rect2);
    bool        rectIntersect(const RECT& rect1, const RECT& rect2, RECT& rectOut);

    // convert screen to window coordinates
    bool        screenToClient(HWND hwnd, LONG& posX, LONG& posY);

    // position window
    void        centerWindowOnRect(HWND hwnd, const RECT& rect);
    void        centerWindowOnScreen(HWND hwnd);
    void        centerWindowOnWindow(HWND hwnd, HWND hwndRelative);

    // cursors
    enum TXWSystemCursor
    {
        eCursorArrow,
        eCursorIBeam,
        eCursorWait,
        eCursorCross,
        eCursorHand,
        eCursorResizeV,
        eCursorResizeH,

        eSystemCursorCount // must be the last
    };

    HCURSOR     getSystemCursor(TXWSystemCursor cursor);

    // string conversion
    void        utf8ToUtf16(const char* utf8, size_t length, std::vector<wchar_t>& utf16Out);
    void        asciiToUnicode(const char* stringIn, size_t length, std::wstring& stringOut);
    void        unicodeToAscii(const wchar_t* stringIn, size_t length, std::string& stringOut);

    // text helpers
    void        cutFormattedText(const std::wstring& text, int maxSize, std::wstring& textOut);
    void        stripFormatTags(const std::wstring& text, std::wstring& textOut);

    // helper to get window DC handle and release it
    class GetWindowDC
    {
        HWND    m_hwnd;
        HDC     m_hdc;
    public:
        GetWindowDC(HWND hwnd) : m_hwnd(hwnd)  {  m_hdc = ::GetDC(hwnd); }
        ~GetWindowDC() { ::ReleaseDC(m_hwnd, m_hdc);  }

        operator HDC() const { return m_hdc; }
    };

    // release global resources
    void    releaseResources();
};

// XWUtils
/////////////////////////////////////////////////////////////////////

#endif // _XWUTILS_H_

