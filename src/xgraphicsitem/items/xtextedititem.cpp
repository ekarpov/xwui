// Text edit graphics item
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"
#include "../../graphics/xwgraphics.h"

#include "../xgraphicsitem.h"

#include "xtextedititem.h"

/////////////////////////////////////////////////////////////////////
// XTextEditItem - text edit item

XTextEditItem::XTextEditItem(XGraphicsItem* parent) :
    m_richTextEdit(0)
{
    // create rich edit
    m_richTextEdit = new XRichTextEdit;
    m_richTextEdit->AddRef();

    // init observer
    m_richTextEdit->setObserver(this);

    // rich edit needs raw message processing
    setMessageProcessing(true);
    
    // add to parent
    if(parent) 
        parent->addChildItem(this);
}

XTextEditItem::~XTextEditItem()
{
    // close editor
    m_richTextEdit->Release();
    m_richTextEdit = 0;
}

/////////////////////////////////////////////////////////////////////
// parent window
/////////////////////////////////////////////////////////////////////
void XTextEditItem::setParentWindow(HWND hwndParent)
{
    // init editor if needed
    if(!m_richTextEdit->isReady())
    {
        _initRichEdit(hwndParent);
    }

    // pass to parent
    XGraphicsItem::setParentWindow(hwndParent);
}

/////////////////////////////////////////////////////////////////////
// message processing
/////////////////////////////////////////////////////////////////////
LRESULT XTextEditItem::processWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& messageProcessed)
{
    // reset flag
    messageProcessed = false;

    // ignore if not focused
    if(!hasFocus()) return 0;

    // pass to editor 
    return m_richTextEdit->processWindowMessage(hwnd, uMsg, wParam, lParam, messageProcessed);
}

/////////////////////////////////////////////////////////////////////
// scrolling (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
int XTextEditItem::contentWidth()
{
    return m_richTextEdit->contentWidth();
}

int XTextEditItem::contentHeight()
{
    return m_richTextEdit->contentHeight();
}

void XTextEditItem::setScrollOffsetX(int scrollOffsetX)
{
    // pass to editor
    m_richTextEdit->setScrollOffsetX(scrollOffsetX);

    // pass offset to parent
    XGraphicsItem::setScrollOffsetX(scrollOffsetX);
}

void XTextEditItem::setScrollOffsetY(int scrollOffsetY)
{
    // pass to editor
    m_richTextEdit->setScrollOffsetY(scrollOffsetY);

    // pass offset to parent
    XGraphicsItem::setScrollOffsetY(scrollOffsetY);
}

/////////////////////////////////////////////////////////////////////
// focus
/////////////////////////////////////////////////////////////////////
bool XTextEditItem::isFocusable() const
{
    // can have focus
    return true;
}

void XTextEditItem::setFocus(bool bFocus)
{
    // pass to richedit
    m_richTextEdit->setFocus(bFocus);

    // pass to parent
    XGraphicsItem::setFocusProperty(bFocus);
}

/////////////////////////////////////////////////////////////////////
// manipulations (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextEditItem::update(int posX, int posY, int width, int height)
{
    // pass to parent
    XGraphicsItem::update(posX, posY, width, height);
    
    // parent has updated item rectangle, so pass it to editor
    m_richTextEdit->update(m_itemRect);
}

/////////////////////////////////////////////////////////////////////
// properties (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextEditItem::setEnabled(bool bEnabled)
{
    // TODO:

    // pass to parent
    XGraphicsItem::setEnabled(bEnabled);
}

/////////////////////////////////////////////////////////////////////
// mouse events
/////////////////////////////////////////////////////////////////////
bool XTextEditItem::onSetCursor()
{
    POINT pt;

    // get mouse cursor position
    ::GetCursorPos(&pt);

    // convert it to window coordinates
    ::ScreenToClient(parentWindow(), &pt);

    // check if over richedit area
    if(isInside(pt.x, pt.y))
    {
        // pass to rich edit 
        return m_richTextEdit->onSetCursor(pt.x, pt.y);
    }

    // use class cursor
    return false;
}

/////////////////////////////////////////////////////////////////////
// GDI painting (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextEditItem::onPaintGDI(HDC hdc, const RECT& rcPaint)
{
    // ignore if not ready
    if(!m_richTextEdit->isReady()) return;

    // paint editor
    m_richTextEdit->onPaintGDI(hdc, rcPaint);
}

/////////////////////////////////////////////////////////////////////
// Direct2D painting (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextEditItem::onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint)
{
    // paint using GDI
    XGraphicsItem::onPaintD2DFromGDI(pTarget, rcPaint);
}

/////////////////////////////////////////////////////////////////////
// scrollbars (from IXRichEditHostObserver)
/////////////////////////////////////////////////////////////////////
void XTextEditItem::onRichEditVerticalScrollPos(int pos)
{
    // ignore if same
    if(pos == scrollOffsetY()) return;

    // pass to parent (NOTE: not to XGraphicsItem to avoid repaint)
    IXWScrollable::setScrollOffsetY(pos);

    // handle modified content
    handleContentChanged();
}

void XTextEditItem::onRichEditHorizontalScrollPos(int pos)
{
    // ignore if same
    if(pos == scrollOffsetX()) return;

    // pass to parent (NOTE: not to XGraphicsItem to avoid repaint)
    IXWScrollable::setScrollOffsetX(pos);

    // handle modified content
    handleContentChanged();
}

/////////////////////////////////////////////////////////////////////
// content (from IXRichEditHostObserver)
/////////////////////////////////////////////////////////////////////
void XTextEditItem::onRichEditContentRectChanged(const RECT& rcContent)
{
    // inform parent window to update scroll bars if needed
    handleContentChanged();
}

/////////////////////////////////////////////////////////////////////
// helper methods
/////////////////////////////////////////////////////////////////////
void XTextEditItem::_initRichEdit(HWND hwndParent)
{
    XWASSERT(m_richTextEdit);
    XWASSERT(hwndParent);
    if(m_richTextEdit == 0 || hwndParent == 0) return;

    // close previous editor if any
    m_richTextEdit->close();

    // init
    if(!m_richTextEdit->init(hwndParent, m_itemRect))
    {
        XWTRACE("XTextEditItem: failed to init RichEditHost");

        // close editor
        m_richTextEdit->close();
    }
}

// XTextEditItem
/////////////////////////////////////////////////////////////////////


