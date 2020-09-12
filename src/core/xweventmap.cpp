// Window events map
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xweventmap.h"

/////////////////////////////////////////////////////////////////////
// XWEventMask - window event mask
XWEventMask::XWEventMask()
{
    // reset flags
    m_uFlags = 0;
}

XWEventMask::~XWEventMask()
{
}

/////////////////////////////////////////////////////////////////////
// convenience mask constructors
/////////////////////////////////////////////////////////////////////
XWEventMask::XWEventMask(const HWND hWnd)
{
    // reset flags
    m_uFlags = 0;

    // set handle
    setWindowHandle(hWnd);
}

XWEventMask::XWEventMask(const UINT uMsg)
{
    // reset flags
    m_uFlags = 0;

    // set handle
    setWindowMessage(uMsg);
}

XWEventMask::XWEventMask(const HWND hWnd, const UINT uMsg)
{
    // reset flags
    m_uFlags = 0;

    // set 
    setWindowHandle(hWnd);
    setWindowMessage(uMsg);
}

/////////////////////////////////////////////////////////////////////
// set mask values
/////////////////////////////////////////////////////////////////////
void XWEventMask::setWindowHandle(const HWND hWnd)
{
    // add value
    m_uFlags |= eWindowHandle;
    m_hWnd = hWnd;
}

void XWEventMask::setWindowMessage(const UINT uMsg)
{
    // add value
    m_uFlags |= eWindowMessage;
    m_uMsg = uMsg;
}

void XWEventMask::setWParam(const WPARAM wParam)
{
    // set value
    m_uFlags |= (eLoWordWParam | eHiWordWParam);
    m_wParam = wParam;
}

void XWEventMask::setWParamLoWord(const WORD word)
{
    // add flag
    m_uFlags |= eLoWordWParam;

    // set new value
    WORD hWord = HIWORD(m_wParam);
    m_wParam = MAKEWPARAM(word, hWord);
}

void XWEventMask::setWParamHiWord(const WORD word)
{
    // add flag
    m_uFlags |= eHiWordWParam;

    // set new value
    WORD lWord = LOWORD(m_wParam);
    m_wParam = MAKEWPARAM(lWord, word);
}

void XWEventMask::setLParam(const LPARAM lParam)
{
    // set value
    m_uFlags |= (eLoWordLParam | eHiWordLParam);
    m_lParam = lParam;
}

void XWEventMask::setLParamLoWord(const WORD word)
{
    // add flag
    m_uFlags |= eLoWordLParam;

    // set new value
    WORD hWord = HIWORD(m_lParam);
    m_lParam = MAKELPARAM(word, hWord);
}

void XWEventMask::setLParamHiWord(const WORD word)
{
    // add flag
    m_uFlags |= eHiWordLParam;

    // set new value
    WORD lWord = LOWORD(m_lParam);
    m_lParam = MAKELPARAM(lWord, word);
}

/////////////////////////////////////////////////////////////////////
// reset mask
/////////////////////////////////////////////////////////////////////
void XWEventMask::reset()
{
    // reset flags
    m_uFlags = 0;
}

/////////////////////////////////////////////////////////////////////
// match events
/////////////////////////////////////////////////////////////////////
bool XWEventMask::matchEvent(const XWEvent& xwEvent)
{
    return sMatchEvent(xwEvent, *this);
}

bool XWEventMask::sMatchEvent(const XWEvent& xwEvent, const XWEventMask& mask)
{
    // check if there are any mask flags
    if(mask.m_uFlags == 0) return false;

    // check masked values
    if((mask.m_uFlags & eWindowHandle) && mask.m_hWnd != xwEvent.hWnd) return false;
    if((mask.m_uFlags & eWindowMessage) && mask.m_uMsg != xwEvent.uMsg) return false;

    // WPARAM
    if((mask.m_uFlags & eLoWordWParam) && 
        LOWORD(mask.m_wParam) != LOWORD(xwEvent.wParam)) return false;
    if((mask.m_uFlags & eHiWordWParam) && 
        HIWORD(mask.m_wParam) != HIWORD(xwEvent.wParam)) return false;

    // LPARAM
    if((mask.m_uFlags & eLoWordLParam) && 
        LOWORD(mask.m_lParam) != LOWORD(xwEvent.lParam)) return false;
    if((mask.m_uFlags & eHiWordLParam) && 
        HIWORD(mask.m_lParam) != HIWORD(xwEvent.lParam)) return false;

    return true;
}

// XWEventMask
/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
// XWEventMap - event map
XWEventMap::XWEventMap()
{
}

XWEventMap::~XWEventMap()
{
}

/////////////////////////////////////////////////////////////////////
// add handlers
/////////////////////////////////////////////////////////////////////
void XWEventMap::addEventHandler(const XWEventMask& mask, const XWEventDelegate& handler)
{
    // handler reference
    _HandlerRef ref;
    ref.xwEventMask = mask;
    ref.xwEventHandler = handler;

    // check if we have this handler already
    std::map<UINT, _HandlerRefVect>::iterator it = m_vEventHandlers.find(mask.m_uMsg);
    if(it != m_vEventHandlers.end())
    {
        // add one more handler
        it->second.push_back(ref);

    } else
    {
        // add new vector
        _HandlerRefVect vect;
        vect.push_back(ref);

        // add to map
        m_vEventHandlers.insert(std::map<UINT, _HandlerRefVect>::value_type(mask.m_uMsg, vect));
    }
}

void XWEventMap::removeEventHandlers(const XWEventMask& mask)
{
    // check if we have this handler at all
    std::map<UINT, _HandlerRefVect>::iterator it = m_vEventHandlers.find(mask.m_uMsg);
    if(it != m_vEventHandlers.end())
    {
        // loop over all handlers
        _HandlerRefVect::iterator href = it->second.begin();
        while(href != it->second.end())
        {
            // compare masks
            if(href->xwEventMask == mask)
            {
                // remove handler
                href = it->second.erase(href);

            } else
            {
                ++href;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////
// find handlers for events
/////////////////////////////////////////////////////////////////////
std::vector<XWEventDelegate> XWEventMap::findHandlers(const XWEvent& xwEvent)
{
    std::vector<XWEventDelegate> result;

    // check if we have this handler at all
    std::map<UINT, _HandlerRefVect>::iterator it = m_vEventHandlers.find(xwEvent.uMsg);
    if(it != m_vEventHandlers.end())
    {
        // get handlers
        _HandlerRefVect& handlers = it->second;

        // loop over all handlers
        for(unsigned int idx = 0; idx < handlers.size(); ++idx)
        {
            // math event with mask
            if(handlers.at(idx).xwEventMask.matchEvent(xwEvent))
            {
                // add handler
                result.push_back(handlers.at(idx).xwEventHandler);
            }
        }
    }

    return result;
}

// XWEventMap
/////////////////////////////////////////////////////////////////////
