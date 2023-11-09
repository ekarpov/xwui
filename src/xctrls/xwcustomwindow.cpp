// Custom window base implementation
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

// dependencies
#include "../layout/xwlayouts.h"
#include "../xwindow/xwindow.h"

#include "xwcustomwindow.h"

/////////////////////////////////////////////////////////////////////
// notes

// WM_NCHITTEST
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms645618.aspx

// WM_NCCALCSIZE
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms632634(v=vs.85).aspx

// WinAPI C++: Reprogramming Window Resize
// http://stackoverflow.com/questions/19106047/winapi-c-reprogramming-window-resize

/////////////////////////////////////////////////////////////////////
// XWCustomWindow - custom window helpers

XWCustomWindow::XWCustomWindow(XWObject* parent):
    XWindow(parent),
    m_bMovingEnabled(0),
    m_bResizeEnabled(0),
    m_nBorderSize(0)
{
    // reset areas
    ::ZeroMemory(&m_rcMovingHitArea, sizeof(m_rcMovingHitArea));
}

XWCustomWindow::XWCustomWindow(DWORD dwStyle, XWObject* parent, HWND hWndParent, DWORD dwExStyle):
    XWindow(dwStyle, parent, hWndParent, dwExStyle),
    m_bMovingEnabled(0),
    m_bResizeEnabled(0),
    m_nBorderSize(0)
{
    // reset areas
    ::ZeroMemory(&m_rcMovingHitArea, sizeof(m_rcMovingHitArea));
}

XWCustomWindow::~XWCustomWindow()
{
}

/////////////////////////////////////////////////////////////////////
// moving (window will respond to WM_NCHITTEST)
/////////////////////////////////////////////////////////////////////
void XWCustomWindow::enableMoving(bool enable)
{
    // copy flag
    m_bMovingEnabled = enable;
}

void XWCustomWindow::setMovingHitArea(const RECT& area)
{
    // copy area
    m_rcMovingHitArea = area;
}

/////////////////////////////////////////////////////////////////////
// resize (window will respond to WM_NCHITTEST)
/////////////////////////////////////////////////////////////////////
void XWCustomWindow::enableResize(bool enable)
{
    // copy flag
    m_bResizeEnabled = enable;
}

void XWCustomWindow::setBorderSize(int size)
{
    // copy size
    m_nBorderSize = size;
}

/////////////////////////////////////////////////////////////////////
// process messages
/////////////////////////////////////////////////////////////////////
LRESULT XWCustomWindow::processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // ignore if no custom processing is needed
    if(!m_bMovingEnabled && !m_bResizeEnabled)
    {
        // pass to parent
        return XWindow::processMessage(hwnd, uMsg, wParam, lParam);
    }

    // process messages
    if(uMsg == WM_NCCALCSIZE)
    {
        // NOTE: If wParam is TRUE, it specifies that the application should indicate which 
        //       part of the client area contains valid information. The system copies the 
        //       valid information to the specified area within the new client area.
        // https://msdn.microsoft.com/en-us/library/windows/desktop/ms632634(v=vs.85).aspx

        if(m_bResizeEnabled && m_nBorderSize > 0 && wParam && lParam)
        {
            // NOTE: If wParam is TRUE, lParam points to an NCCALCSIZE_PARAMS structure that 
            //       contains information an application can use to calculate the new size 
            //       and position of the client rectangle.

            // size of the client area
            NCCALCSIZE_PARAMS* ncsizeParams = (NCCALCSIZE_PARAMS*)(lParam);
            ncsizeParams->rgrc[0].bottom += m_nBorderSize; 
            ncsizeParams->rgrc[0].right += m_nBorderSize;
            ncsizeParams->rgrc[1].bottom += m_nBorderSize;
            ncsizeParams->rgrc[1].right += m_nBorderSize;
            ncsizeParams->rgrc[2].bottom += m_nBorderSize;
            ncsizeParams->rgrc[2].right += m_nBorderSize;
            return 0;
        }

        // pass to parent
        return XWindow::processMessage(hwnd, uMsg, wParam, lParam);
    }
    else if(uMsg == WM_NCHITTEST)
    {
        // window rect
        RECT winRect;
        ::GetWindowRect(hwnd, &winRect);

        // get points (in window coordinates)
        int posX = GET_X_LPARAM(lParam) - winRect.left;
        int posY = GET_Y_LPARAM(lParam) - winRect.top;

        // check if point hits border
        bool borderPoint = _isBorderPoint(posX, posY, winRect);

        // check moving
        if(m_bMovingEnabled && !borderPoint && XWUtils::rectIsInside(m_rcMovingHitArea, posX, posY)) return HTCAPTION;

        // check resize
        if(m_bResizeEnabled && borderPoint)
        {
            // check what border type we are in
            if (posX < m_nBorderSize && posY < m_nBorderSize)
                return HTTOPLEFT;
            else if (posX > winRect.right - winRect.left - m_nBorderSize && posY < m_nBorderSize)
                return HTTOPRIGHT;
            else if (posX > winRect.right - winRect.left - m_nBorderSize && posY > winRect.bottom - winRect.top - m_nBorderSize)
                return HTBOTTOMRIGHT;
            else if (posX < m_nBorderSize && posY > winRect.bottom - winRect.top - m_nBorderSize)
                return HTBOTTOMLEFT;
            else if (posX < m_nBorderSize)
                return HTLEFT;
            else if (posY < m_nBorderSize)
                return HTTOP;
            else if (posX > winRect.right - winRect.left - m_nBorderSize)
                return HTRIGHT;
            else if (posY > winRect.bottom - winRect.top - m_nBorderSize)
                return HTBOTTOM;
            else
                return HTCLIENT;        
        }
    }

    // pass to parent
    return XWindow::processMessage(hwnd, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
bool XWCustomWindow::_isBorderPoint(int posX, int posY, const RECT& windowRect)
{
    // ignore if border size not set
    if(!m_bResizeEnabled || m_nBorderSize == 0) return false;

    // check if point hits border
    bool hitX = ((posX >= 0 && posX <= m_nBorderSize) || 
       (posX >= windowRect.right - m_nBorderSize && posX <= windowRect.right));

    bool hitY = ((posY >= 0 && posY <= m_nBorderSize) || 
       (posY >= windowRect.bottom - m_nBorderSize && posY <= windowRect.bottom));

    return (hitX && hitY);
}

// XWCustomWindow
/////////////////////////////////////////////////////////////////////
