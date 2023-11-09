// Splitter window
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

// dependencies
#include "../layout/xwlayouts.h"
#include "../xwindow/xwindow.h"
#include "../graphics/xgdihelpres.h"

#include "xwsplitterwindow.h"

/////////////////////////////////////////////////////////////////////
// XWSplitterWindow - splitter window

/////////////////////////////////////////////////////////////////////
// construction/destruction
/////////////////////////////////////////////////////////////////////
XWSplitterWindow::XWSplitterWindow(DWORD dwStyle, XWObject* parent, HWND hWndParent, DWORD dwExStyle) :
    XWindow(dwStyle | WS_CLIPCHILDREN, parent, hWndParent, dwExStyle),
    m_orientation(XWUI_SPLIT_VERTICALLY),
    m_fillColor(XWUIStyle::getColorBorder()),
    m_ltWindow(0),
    m_rbWindow(0),
    m_originalCursor(0),
    m_splitterRatio(0.0),
    m_splitterPos(0),
    m_splitterWidth(5), // default width
    m_mouseMoveActive(false),
    m_mousePosX(0),
    m_mousePosY(0)
{
}

XWSplitterWindow::~XWSplitterWindow()
{
}

/////////////////////////////////////////////////////////////////////
// windows to split (window will take ownership)
/////////////////////////////////////////////////////////////////////
void XWSplitterWindow::setSplitWindows(TXWSplitOrientation orientation, XWHWND* ltWindow, XWHWND* rbWindow)
{
    // check input
    XWASSERT(ltWindow);
    XWASSERT(rbWindow);
    if(ltWindow == 0 || rbWindow == 0) return;

    // delete previous windows if any
    if(m_ltWindow) delete m_ltWindow;
    if(m_rbWindow) delete m_rbWindow;

    // copy windows
    m_ltWindow = ltWindow;
    m_rbWindow = rbWindow;

    // set ownership
    m_ltWindow->setParentObject(this);
    rbWindow->setParentObject(this);

    // set parent window
    m_ltWindow->setParent(hwnd());
    rbWindow->setParent(hwnd());

    // copy orientation
    m_orientation = orientation;

    // if pos not set split in half
    if(m_splitterPos == 0)
    {
        if(m_orientation == XWUI_SPLIT_VERTICALLY)
            m_splitterPos = height() / 2;
        else if(m_orientation == XWUI_SPLIT_HORIZONTALLY)
            m_splitterPos = width() / 2;
    }

    // set position
    _setSplitterPos(m_splitterPos);
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XWSplitterWindow::setSplitterWidth(int width)
{
    // check input
    XWASSERT(width > 0);
    if(width <= 0) return;

    // copy width
    m_splitterWidth = width;

    // set position
    _setSplitterPos(m_splitterPos);
}

void XWSplitterWindow::setSplitterFillColor(const COLORREF& color)
{
    // copy color
    m_fillColor = color;
}

void XWSplitterWindow::moveSplitter(int pos)
{
    // check input
    XWASSERT(pos >= 0);
    if(pos < 0) return;

    // validate borders
    if(m_orientation == XWUI_SPLIT_VERTICALLY)
    {
        if(pos > height()) 
            pos = height();

    } else if(m_orientation == XWUI_SPLIT_HORIZONTALLY)
    {
        if(pos > width())
            pos = width();
    }

    // set position
    _setSplitterPos(pos);
}

/////////////////////////////////////////////////////////////////////
// splitter painting
/////////////////////////////////////////////////////////////////////
void XWSplitterWindow::onPaintSplitter(HDC hdc, const RECT& paintRect)
{
    // fill with default color
    XGdiHelpers::fillRect(hdc, paintRect, m_fillColor);
}

/////////////////////////////////////////////////////////////////////
// window events (from XWindow)
/////////////////////////////////////////////////////////////////////
void XWSplitterWindow::onResize(int type, int width, int height)
{
    // ignore if windows are no longer set
    if(m_ltWindow == 0 || m_rbWindow == 0) 
    {
        // update layout
        _updateLayout();
        return;
    }

    // update splitter position
    if(m_orientation == XWUI_SPLIT_VERTICALLY)
    {
        // move to keep ratio
        m_splitterPos = (int)(m_splitterRatio * height);

    } else if(m_orientation == XWUI_SPLIT_HORIZONTALLY)
    {
        // move to keep ratio
        m_splitterPos = (int)(m_splitterRatio * width);
    }

    // update layout
    _updateLayout();
}

void XWSplitterWindow::onPaint(HDC hdc, PAINTSTRUCT& ps)
{
    // ignore if windows are no longer set
    if(m_ltWindow == 0 || m_rbWindow == 0) return;

    RECT rcSplitter;

    // get splitter rectangle
    _getSplitterRect(rcSplitter);

    // paint splitter
    onPaintSplitter(hdc, rcSplitter);
}

/////////////////////////////////////////////////////////////////////
// mouse events (from XWindow)
/////////////////////////////////////////////////////////////////////
void XWSplitterWindow::onMouseEnter(int posX, int posY)
{
    // set cursor
    _setResizeCursor();
}

void XWSplitterWindow::onMouseMove(int posX, int posY, WPARAM flags)
{
    // ignore movements outside window rect
    if(posX < 0 || posX > width() ||
       posY < 0 || posY > height()) return;

    // resize if in move mode
    if(m_mouseMoveActive)
    {
        int newPos = m_splitterPos;
        int validPos = 0;

        if(m_orientation == XWUI_SPLIT_VERTICALLY)
        {
            // move
            newPos = m_splitterPos + (posY - m_mousePosY);

        } else if(m_orientation == XWUI_SPLIT_HORIZONTALLY)
        {
            // move
            newPos = m_splitterPos + (posX - m_mousePosX);
        }

        // ignore movements that fail layout constraints
        if(!_validatePosition(newPos, validPos)) return;

        // ignore if position is the same
        if(newPos == m_splitterPos) return;

        // move
        _setSplitterPos(newPos);

        // repaint splitter
        _doPaintSplitter();
    }

    // save mouse position
    m_mousePosX = posX;
    m_mousePosY = posY;
}

bool XWSplitterWindow::onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags)
{
    // check button event
    if(uButtonMsg == WM_LBUTTONDOWN)
    {
        // start mouse move if not active
        if(!m_mouseMoveActive)
        {
            // update flag
            m_mouseMoveActive = true;

            // start mouse capture
            ::SetCapture(hwnd());
        }

    } else if(uButtonMsg == WM_LBUTTONUP)
    {
        // top mouse move if active
        if(m_mouseMoveActive)
        {
            // update flag
            m_mouseMoveActive = false;

            // stop mouse capture
            ::ReleaseCapture();
        }
    }

    return false;
}

bool XWSplitterWindow::onMouseCaptureChanged()
{
    // reset flag
    m_mouseMoveActive = false;

    // reset cursor
    _resetCursor();

    return false;
}

void XWSplitterWindow::onMouseLeave()
{
    // reset cursor if not in moving state
    if(!m_mouseMoveActive)
    {
        _resetCursor();
    }
}

bool XWSplitterWindow::onSetCursor()
{
    // check if cursor is set or mouse move is active
    return (m_originalCursor != 0) || m_mouseMoveActive;
}

/////////////////////////////////////////////////////////////////////
// events
/////////////////////////////////////////////////////////////////////
void XWSplitterWindow::onConnectedObjectRemoved(XWObject* child)
{
    XWASSERT(child);
    if(child == 0) return;

    // reset pointers if needed
    if(child->xwoid() == m_ltWindow->xwoid()) m_ltWindow = 0;
    if(child->xwoid() == m_rbWindow->xwoid()) m_rbWindow = 0;

    // update layout
    _updateLayout();
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XWSplitterWindow::_updateLayout()
{
    // ignore if windows are no longer set
    if(m_ltWindow == 0 || m_rbWindow == 0)
    {
        if(m_ltWindow) m_ltWindow->update(0, 0, width(), height());
        if(m_rbWindow) m_rbWindow->update(0, 0, width(), height());

        return;
    }

    // validate position
    if(!_validatePosition(m_splitterPos, m_splitterPos))
    {
        // update ratio
        _updateRatio();
    }

    // check orientation
    if(m_orientation == XWUI_SPLIT_VERTICALLY)
    {
        // resize splitted windows
        m_ltWindow->update(0, 0, width(), m_splitterPos);
        m_rbWindow->update(0, m_splitterPos + m_splitterWidth, width(), height() - m_splitterPos - m_splitterWidth);

    } else if(m_orientation == XWUI_SPLIT_HORIZONTALLY)
    {
        // resize splitted windows
        m_ltWindow->update(0, 0, m_splitterPos, height());
        m_rbWindow->update(m_splitterPos + m_splitterWidth, 0, width() - m_splitterPos - m_splitterWidth, height());

    } else
    {
        XWASSERT1(0, "XWSplitterWindow: unsupported orientation");
        return;
    }
}

void XWSplitterWindow::_updateRatio()
{
    // check orientation
    if(m_orientation == XWUI_SPLIT_VERTICALLY)
    {
        int _height = height();

        // update ratio
        if(_height != 0)
            m_splitterRatio = (double)m_splitterPos / (double)_height;

    } else if(m_orientation == XWUI_SPLIT_HORIZONTALLY)
    {
        int _width = width();

        // update ratio
        if(_width != 0)
            m_splitterRatio = (double)m_splitterPos / (double)width();
    }
}

bool XWSplitterWindow::_validatePosition(int splitterPos, int& validPosition)
{
    validPosition = splitterPos;

    // check borders
    if(validPosition < 0)
        validPosition = 0;

    // check orientation
    if(m_orientation == XWUI_SPLIT_VERTICALLY)
    {
        // validate position
        if(validPosition + m_splitterWidth >= height())
            validPosition = height() - m_splitterWidth;

        // minimum height
        if(m_ltWindow->verticalPolicy() == IXLayoutItem::eResizeMin ||
           m_ltWindow->verticalPolicy() == IXLayoutItem::eResizeMinMax)
        {
            if(validPosition < m_ltWindow->minHeight())
                validPosition = m_ltWindow->minHeight();
        }

        // maximum height
        if(m_ltWindow->verticalPolicy() == IXLayoutItem::eResizeMax ||
           m_ltWindow->verticalPolicy() == IXLayoutItem::eResizeMinMax)
        {
            if(validPosition > m_ltWindow->maxHeight())
                validPosition = m_ltWindow->maxHeight();
        }

        // minimum height
        if(m_rbWindow->verticalPolicy() == IXLayoutItem::eResizeMin ||
           m_rbWindow->verticalPolicy() == IXLayoutItem::eResizeMinMax)
        {
            if(height() - validPosition - m_splitterWidth < m_rbWindow->minHeight())
                validPosition = height() - m_splitterWidth - m_rbWindow->minHeight();
        }

        // maximum height
        if(m_rbWindow->verticalPolicy() == IXLayoutItem::eResizeMax ||
           m_rbWindow->verticalPolicy() == IXLayoutItem::eResizeMinMax)
        {
            if(height() - validPosition - m_splitterWidth > m_rbWindow->maxHeight())
                validPosition = height() - m_splitterWidth - m_rbWindow->maxHeight();
        }

    } else if(m_orientation == XWUI_SPLIT_HORIZONTALLY)
    {
        // validate position
        if(validPosition + m_splitterWidth >= width())
            validPosition = width() - m_splitterWidth;

        // minimum width
        if(m_ltWindow->horizontalPolicy() == IXLayoutItem::eResizeMin ||
           m_ltWindow->horizontalPolicy() == IXLayoutItem::eResizeMinMax)
        {
            if(validPosition < m_ltWindow->minWidth())
                validPosition = m_ltWindow->minWidth();
        }

        // maximum width
        if(m_ltWindow->horizontalPolicy() == IXLayoutItem::eResizeMax ||
           m_ltWindow->horizontalPolicy() == IXLayoutItem::eResizeMinMax)
        {
            if(validPosition > m_ltWindow->maxWidth())
                validPosition = m_ltWindow->maxWidth();
        }

        // minimum width
        if(m_rbWindow->horizontalPolicy() == IXLayoutItem::eResizeMin ||
           m_rbWindow->horizontalPolicy() == IXLayoutItem::eResizeMinMax)
        {
            if(width() - validPosition - m_splitterWidth < m_rbWindow->minWidth())
                validPosition = width() - m_splitterWidth - m_rbWindow->minWidth();
        }

        // maximum width
        if(m_rbWindow->horizontalPolicy() == IXLayoutItem::eResizeMax ||
           m_rbWindow->horizontalPolicy() == IXLayoutItem::eResizeMinMax)
        {
            if(width() - validPosition - m_splitterWidth > m_rbWindow->maxWidth())
                validPosition = width() - m_splitterWidth - m_rbWindow->maxWidth();
        }
    }

    // check if position has changed
    return (validPosition == splitterPos);
}

bool XWSplitterWindow::_isOverSplitter(int posX, int posY)
{
    // check orientation
    if(m_orientation == XWUI_SPLIT_VERTICALLY)
    {
        return (posX >= 0 && 
                posX <= width() && 
                posY >= m_splitterPos && 
                posY <= m_splitterPos + m_splitterWidth);

    } else if(m_orientation == XWUI_SPLIT_HORIZONTALLY)
    {
        return (posX >= m_splitterPos && 
                posX <= m_splitterPos + m_splitterWidth && 
                posY >= 0 && 
                posY <= height());
    }

    XWASSERT(0);
    return false;
}

void XWSplitterWindow::_setSplitterPos(int pos)
{
    // set position
    m_splitterPos = pos;

    // update layout
    _updateLayout();
    
    // update ratio
    _updateRatio();
}

void XWSplitterWindow::_setResizeCursor()
{
    // reset previous if any
    _resetCursor();

    // check orientation
    if(m_orientation == XWUI_SPLIT_VERTICALLY)
    {
        m_originalCursor = ::SetCursor(XWUtils::getSystemCursor(XWUtils::eCursorResizeV));

    } else if(m_orientation == XWUI_SPLIT_HORIZONTALLY)
    {
        m_originalCursor = ::SetCursor(XWUtils::getSystemCursor(XWUtils::eCursorResizeH));
    }
}

void XWSplitterWindow::_resetCursor()
{
    // set previous cursor if any
    if(m_originalCursor)
    {
        ::SetCursor(m_originalCursor);
        m_originalCursor = 0;
    }
}

void XWSplitterWindow::_getSplitterRect(RECT& rcSplitter)
{
    if(m_orientation == XWUI_SPLIT_VERTICALLY)
    {
        rcSplitter.left = 0;
        rcSplitter.right = width();
        rcSplitter.top = m_splitterPos - m_splitterWidth / 2;
        rcSplitter.bottom = m_splitterPos + m_splitterWidth;

    } else if(m_orientation == XWUI_SPLIT_HORIZONTALLY)
    {
        rcSplitter.top = 0;
        rcSplitter.bottom = height();
        rcSplitter.left = m_splitterPos - m_splitterWidth / 2;
        rcSplitter.right = m_splitterPos + m_splitterWidth;
    }
}

void XWSplitterWindow::_doPaintSplitter()
{
    // ignore if windows are no longer set
    if(m_ltWindow == 0 || m_rbWindow == 0) return;

    RECT rcSplitter;

    // get splitter rectangle
    _getSplitterRect(rcSplitter);

    // get dc
    HDC hdc = ::GetDC(hwnd());
    if(hdc == 0) return;

    // paint splitter
    onPaintSplitter(hdc, rcSplitter);

    // release DC
    ::ReleaseDC(hwnd(), hdc);
}

// XWSplitterWindow
/////////////////////////////////////////////////////////////////////
