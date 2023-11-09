// Graphics item base functionality
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"
#include "../layout/xlayoutitem.h"
#include "../layout/xlayout.h"
#include "../layout/xvboxlayout.h"
#include "../layout/xhboxlayout.h"

#include "../ctrls/xpopupmenu.h"

#include "../graphics/xwgraphics.h"

#include "xgraphicsitem.h"

/////////////////////////////////////////////////////////////////////
// XGraphicsItem - graphics item

XGraphicsItem::XGraphicsItem(XGraphicsItem* parent) :
    XWObject(parent),
    m_stateFlags(0),
    m_fillBackground(false),
    m_pXGdiResourcesCache(0),
    m_pXD2DResourcesCache(0),
    m_hwndParent(0),
    m_pLayout(0),
    m_contextMenu(0),
    m_contextMenuEnabled(true),
    m_visible(true),
    m_obscured(false),
    m_clickable(false),
    m_focusable(false),
    m_messageProcessing(false),
    m_scrollingEnabled(true),
    m_graphicsPainter(XWUI_PAINTER_GDI),
    m_mouseItem(0),
    m_focusItem(0),
    m_rpHorizontal(eResizeAny),
    m_rpVertical(eResizeAny),
    m_minWidth(0),
    m_minHeight(0),
    m_maxWidth(0),
    m_maxHeight(0),
    m_itemCursor(0),
    m_originalCursor(0),
    m_pGDIRenderTarget(0)
{
    // zero initial size
    m_itemRect.left = 0;
    m_itemRect.right = 0;
    m_itemRect.top = 0;
    m_itemRect.bottom = 0;

    // init default painter type
    if(sXWUIDefaultPainter() == XWUI_PAINTER_D2D)
        m_graphicsPainter = XWUI_PAINTER_D2D;
    else
        m_graphicsPainter = XWUI_PAINTER_GDI;

    // add itself to a parent as a child
    if(parent) parent->addChildItem(this);
}

XGraphicsItem::~XGraphicsItem()
{
    // stop animations if any
    stopAllAnimations();

    // cancel content loading if any
    cancelAllContent();

    // delete layout if any
    delete m_pLayout;

    // reset caches if any
    onResetGDIResources();
    onResetD2DTarget();

    // release caches if any
    if(m_pXGdiResourcesCache) 
        m_pXGdiResourcesCache->Release();
    if(m_pXD2DResourcesCache)
        m_pXD2DResourcesCache->Release();
}

/////////////////////////////////////////////////////////////////////
// parent window
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::setParentWindow(HWND hwndParent)
{
    // stop animations if any (as they depend on parent window)
    stopAllAnimations();

    // cancel content loading if any (as they depend on parent window)
    cancelAllContent();

    m_hwndParent = hwndParent;

    // set also to child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        // init
        (*it)->setParentWindow(hwndParent);
    }
}

/////////////////////////////////////////////////////////////////////
// message processing
/////////////////////////////////////////////////////////////////////
LRESULT XGraphicsItem::processWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& messageProcessed)
{
    // reset flag by default
    messageProcessed = false;

    // ignore if processing is not needed
    if(!m_messageProcessing) return 0;

    // check message
    switch(uMsg)
    {
    // mouse messages
    case WM_MOUSEMOVE:
    case WM_MOUSEHOVER:
    case WM_MOUSEWHEEL:
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_XBUTTONDBLCLK:
        {
            // find item for mouse event
            XGraphicsItem* item = _findItem(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            if(item)
            {
                // pass to item
                return item->processWindowMessage(hwnd, uMsg, wParam, lParam, messageProcessed);
            }
        }
        break;

    }

    // pass the rest to focused item if any
    if(m_focusItem)
        return m_focusItem->processWindowMessage(hwnd, uMsg, wParam, lParam, messageProcessed);

    return 0;
}

/////////////////////////////////////////////////////////////////////
// standard event handlers
/////////////////////////////////////////////////////////////////////
unsigned long XGraphicsItem::addClickedHandler(const XWObjectEventDelegate& handler)
{
    return addEventHandler(XGITEM_EVENT_CLICKED, handler);
}

/////////////////////////////////////////////////////////////////////
// layout engine
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::setLayout(IXLayout* layout)
{
    // delete old layout if any
    delete m_pLayout;

    // copy layout reference
    m_pLayout = layout;
}

/////////////////////////////////////////////////////////////////////
// child items
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::addChildItem(XGraphicsItem* childItem)
{
    XWASSERT(childItem);
    if(childItem == 0) return;

    // check if we already have this item
    if(_findItem(childItem->xwoid()) != 0)
    {
        // ignore if we already have this
        return;
    }

    // take ownership
    childItem->setParentObject(this);

    // set same parent window
    childItem->setParentWindow(m_hwndParent);

    // set caches 
    childItem->setGDIResourcesCache(m_pXGdiResourcesCache);
    childItem->setD2DResourcesCache(m_pXD2DResourcesCache);

    // change painter type if needed
    if(childItem->painterType() != painterType())
    {
        childItem->setPainterType(painterType());
    }

    // init GDI resources if set
    if(m_pXGdiResourcesCache && m_hwndParent)
    {
        // window DC
        HDC hdc = ::GetDC(m_hwndParent);

        // init resources
        childItem->onInitGDIResources(hdc);

        // release DC
        ::ReleaseDC(m_hwndParent, hdc);
    }

    // init D2D resources if set
    if(m_pXD2DResourcesCache && m_pXD2DResourcesCache->renderTarget())
    {
        // init resources
        childItem->onInitD2DTarget(m_pXD2DResourcesCache->renderTarget());
    }

    // add to list
    m_childItems.push_back(childItem);

    // update properties
    if(!m_messageProcessing && childItem->processingMessages())
    {
        // set flag to parent as well
        m_messageProcessing = true;
    }
}

void XGraphicsItem::deleteChildItem(unsigned long itemId)
{
    // find item
    std::list<XGraphicsItem*>::iterator it = _findItemIt(itemId);
    if(it != m_childItems.end())
    {
        // delete item (will trigger onChildObjectRemoved)
        delete (*it);
    }
}

void XGraphicsItem::deleteAllChildItems()
{
    // NOTE: deleting items will trigger onChildObjectRemoved that will properly clean
    //       internal references. It will also remove item from m_childItems array. So
    //       we make a copy and remove items one by one.

    // make a copy
    std::list<XGraphicsItem*> childItems = m_childItems;

    // delete items one by one
    for(std::list<XGraphicsItem*>::iterator it = childItems.begin(); it != childItems.end(); ++it)
    {
        deleteChildItem((*it)->xwoid());
    }
}

XGraphicsItem* XGraphicsItem::findChildItem(unsigned long itemId)
{
    // find child item
    return _findItem(itemId);
}

XGraphicsItem* XGraphicsItem::findAnimationItem(DWORD animationId)
{
    // check from item animations
    std::vector<DWORD>::iterator it = std::find(m_itemAnimations.begin(), m_itemAnimations.end(), animationId);
    if(it != m_itemAnimations.end()) return this;

    // check from child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        XGraphicsItem* item = (*it)->findAnimationItem(animationId);
        if(item != 0) return item;
    }

    // not found
    return 0;
}

XGraphicsItem* XGraphicsItem::findContentItem(DWORD contentId)
{
    // check from item animations
    std::vector<DWORD>::iterator it = std::find(m_itemContentIds.begin(), m_itemContentIds.end(), contentId);
    if(it != m_itemContentIds.end()) return this;

    // check from child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        XGraphicsItem* item = (*it)->findContentItem(contentId);
        if(item != 0) return item;
    }

    // not found
    return 0;
}

/////////////////////////////////////////////////////////////////////
// Z-ordering (item must be child item)
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::moveItemOnTop(XGraphicsItem* childItem)
{
    XWASSERT(childItem)
    if(childItem == 0) return;

    // ignore if parent is different
    if(childItem->parentObject() == 0 || 
       childItem->parentObject()->xwoid() != xwoid()) 
    {
        XWASSERT1(0, "XGraphicsItem: trying to move item from different parent");
        return;
    }

    // NOTE: last item added is top item, move this to the list end

    // find item
    std::list<XGraphicsItem*>::iterator it = _findItemIt(childItem->xwoid());

    // ignore if item not found
    XWASSERT1(it != m_childItems.end(), "XGraphicsItem: child item not found by id, data might be corrupted");
    if(it == m_childItems.end()) return;

    // remove current reference from list
    m_childItems.erase(it);

    // append item back
    m_childItems.push_back(childItem);
}

void XGraphicsItem::moveItemUp(XGraphicsItem* childItem)
{
    XWASSERT(childItem)
    if(childItem == 0) return;

    // ignore if parent is different
    if(childItem->parentObject() == 0 || 
       childItem->parentObject()->xwoid() != xwoid()) 
    {
        XWASSERT1(0, "XGraphicsItem: trying to move item from different parent");
        return;
    }

    // find item
    std::list<XGraphicsItem*>::iterator it = _findItemIt(childItem->xwoid());

    // ignore if item not found
    XWASSERT1(it != m_childItems.end(), "XGraphicsItem: child item not found by id, data might be corrupted");
    if(it == m_childItems.end()) return;

    // next item
    std::list<XGraphicsItem*>::iterator nextIt = it;
    ++nextIt;

    // check if there is item after this
    if(nextIt != m_childItems.end())
    {
        // swap items
        std::swap(*it, *nextIt);
    }
}

void XGraphicsItem::moveItemDown(XGraphicsItem* childItem)
{
    XWASSERT(childItem)
    if(childItem == 0) return;

    // ignore if parent is different
    if(childItem->parentObject() == 0 || 
       childItem->parentObject()->xwoid() != xwoid()) 
    {
        XWASSERT1(0, "XGraphicsItem: trying to move item from different parent");
        return;
    }

    // find item
    std::list<XGraphicsItem*>::iterator it = _findItemIt(childItem->xwoid());

    // ignore if item not found
    XWASSERT1(it != m_childItems.end(), "XGraphicsItem: child item not found by id, data might be corrupted");
    if(it == m_childItems.end()) return;

    // check if there is item before
    if(it != m_childItems.begin())
    {
        // previous item
        std::list<XGraphicsItem*>::iterator prevIt = it;
        --prevIt;

        // swap items
        std::swap(*it, *prevIt);
    }
}

bool XGraphicsItem::isTopItem(XGraphicsItem* childItem) const
{
    XWASSERT(childItem)
    if(childItem == 0) return false;

    // ignore if parent is different
    if(childItem->parentObject() == 0 || 
       childItem->parentObject()->xwoid() != xwoid()) 
    {
        XWASSERT1(0, "XGraphicsItem: trying to use item from different parent");
        return false;
    }

    // check if item is top item
    return (m_childItems.size() > 0 && childItem->xwoid() == m_childItems.back()->xwoid());
}

bool XGraphicsItem::isBottomItem(XGraphicsItem* childItem) const
{
    XWASSERT(childItem)
    if(childItem == 0) return false;

    // ignore if parent is different
    if(childItem->parentObject() == 0 || 
       childItem->parentObject()->xwoid() != xwoid()) 
    {
        XWASSERT1(0, "XGraphicsItem: trying to use item from different parent");
        return false;
    }

    // check if item is bottom item
    return (m_childItems.size() > 0 && childItem->xwoid() == m_childItems.front()->xwoid());
}

/////////////////////////////////////////////////////////////////////
// position
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::move(int posX, int posY)
{
    // offsets
    int offsetX = posX - m_itemRect.left;
    int offsetY = posY - m_itemRect.top;

    // move rect
    m_itemRect.left = posX;
    m_itemRect.top = posY;
    m_itemRect.right += offsetX;
    m_itemRect.bottom += offsetY;

    // move child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        (*it)->move((*it)->rect().left + offsetX, (*it)->rect().top + offsetY);
    }
}

/////////////////////////////////////////////////////////////////////
// manipulations (from IXLayoutItem)
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::update(int posX, int posY, int width, int height)
{
    // copy new size
    m_itemRect.left = posX;
    m_itemRect.top = posY;
    m_itemRect.right = posX + width;
    m_itemRect.bottom = posY + height;

    // update layout if set
    if(m_pLayout)
    {
        // check if scrolling is enabled
        if(m_scrollingEnabled)
            m_pLayout->update(posX - scrollOffsetX(), posY - scrollOffsetY(), width, height);
        else
            m_pLayout->update(posX, posY, width, height);
    }
}

/////////////////////////////////////////////////////////////////////
// mouse cursors
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::setMouseCursor(HCURSOR cursor)
{
    // set previous cursor if any
    if(m_originalCursor)
    {
        ::SetCursor(m_originalCursor);
        m_originalCursor = 0;
    }

    // copy new cursor
    m_itemCursor = cursor;

    // set new cursor
    if(m_itemCursor)
    {
        m_originalCursor = ::SetCursor(m_itemCursor);
    }
}

void XGraphicsItem::resetMouseCursor()
{
    // set previous cursor if any
    if(m_originalCursor)
    {
        ::SetCursor(m_originalCursor);
    }

    // reset cursors
    m_itemCursor = 0;
    m_originalCursor = 0;
}

/////////////////////////////////////////////////////////////////////
// mouse capture
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::setMouseCapture()
{
    // send message to parent window if set
    if(m_hwndParent)
    {
        // inform parent window to start mouse capture and route all mouse traffic to this item
        ::SendMessageW(m_hwndParent, WM_XWUI_GITEM_SET_MOUSE_CAPTURE, xwoid(), 0); 

        // set state flag
        setStateFlag(STATE_FLAG_MOUSECAPTURE, true);
    }
}

void XGraphicsItem::resetMouseCapture()
{
    // send message to parent window if set
    if(m_hwndParent)
    {
        // inform parent window to stop mouse capture
        ::SendMessageW(m_hwndParent, WM_XWUI_GITEM_RESET_MOUSE_CAPTURE, 0, 0); 
        
        // reset state flag
        setStateFlag(STATE_FLAG_MOUSECAPTURE, false);
    }
}

/////////////////////////////////////////////////////////////////////
// hint text
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::setHintText(const wchar_t* text)
{
    if(text)
        m_hintText = text;
    else
        m_hintText.clear();
}

/////////////////////////////////////////////////////////////////////
// context menu
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::setContextMenu(XPopupMenu* contextMenu)
{
    // delete old menu if any
    if(m_contextMenu) delete m_contextMenu;

    // copy menu (may be null)
    m_contextMenu = contextMenu;

    // init menu
    if(m_contextMenu)
    {
        // update parent object
        m_contextMenu->setParentObject(this);
    }
}

void XGraphicsItem::enableContextMenu(bool enable)
{
    m_contextMenuEnabled = enable;
}

XPopupMenu* XGraphicsItem::contextMenu()
{
    return m_contextMenu;
}

/////////////////////////////////////////////////////////////////////
// content
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::handleContentChanged()
{
    // post message to parent window if set
    if(m_hwndParent)
    {
        // NOTE: post message here, not send, as this will cause parent window
        //       to update graphics item in order for layout to re-position children
        //       and possible scrollbars to update size (or show/hide)

        // inform parent that item content has changed
        ::PostMessageW(m_hwndParent, WM_XWUI_GITEM_CONTENT_CHANGED, 0, 0); 
    }
}

int XGraphicsItem::contentWidthForHeight(int height)
{
    // use content width by default
    return contentWidth();
}

int XGraphicsItem::contentHeightForWidth(int width)
{
    // use content height by default
    return contentHeight();
}

/////////////////////////////////////////////////////////////////////
// scrolling
/////////////////////////////////////////////////////////////////////
bool XGraphicsItem::canScrollContent()
{
    return m_scrollingEnabled;
}

int XGraphicsItem::contentWidth()
{
    // check if we have child items
    if(m_childItems.size())
    {
        // compute content width from child items
        int leftPos = 0;
        int rightPos = 0;
        for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); 
            it != m_childItems.end(); ++it)
        {
            // update top left position
            if(leftPos == 0 || (*it)->rect().left < leftPos) leftPos = (*it)->rect().left;

            // update top right position
            if(rightPos == 0 || (*it)->rect().left + (*it)->contentWidth() > rightPos) 
                rightPos = (*it)->rect().left + (*it)->contentWidth();
        }

        XWASSERT(rightPos - leftPos >= 0);
        return (rightPos - leftPos >= 0) ? (rightPos - leftPos) : 0;

    } else
    {
        // same as width by default
        return width();
    }
}

int XGraphicsItem::contentHeight()
{
    // check if we have child items
    if(m_childItems.size())
    {
        // compute content height from child items
        int topPos = 0;
        int bottomPos = 0;
        int itemHeight = 0;
        for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); 
            it != m_childItems.end(); ++it)
        {
            // update top position
            if(topPos == 0 || (*it)->rect().top < topPos) topPos = (*it)->rect().top;

            // item content height
            itemHeight = (*it)->contentHeight();

            // update bottom position
            if(bottomPos == 0 || (*it)->rect().top + itemHeight > bottomPos) 
                bottomPos = (*it)->rect().top + itemHeight;
        }

        XWASSERT(bottomPos - topPos >= 0);
        return (bottomPos - topPos >= 0) ? (bottomPos - topPos) : 0;

    } else
    {
        // same as height by default
        return height();
    }
}

void XGraphicsItem::setScrollOffsetX(int scrollOffsetX)
{
    // pass to parent
    IXWScrollable::setScrollOffsetX(scrollOffsetX);

    // ignore if default scrolling is not enabled
    if(!m_scrollingEnabled) return;

    // check if layout is set
    if(m_pLayout)
    {
        // update with the same size
        update(m_itemRect.left, m_itemRect.top, width(), height());

    } else
    {
        // move child items
        for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
        {
            (*it)->move((*it)->rect().left - scrollOffsetX, (*it)->rect().top);
        }
    }

    // force repaint during scrolling
    repaint(true);
}

void XGraphicsItem::setScrollOffsetY(int scrollOffsetY)
{
    // pass to parent
    IXWScrollable::setScrollOffsetY(scrollOffsetY);

    // ignore if default scrolling is not enabled
    if(!m_scrollingEnabled) return;

    // check if layout is set
    if(m_pLayout)
    {
        // update with the same size
        update(m_itemRect.left, m_itemRect.top, width(), height());

    } else
    {
        // move child items
        for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
        {
            (*it)->move((*it)->rect().left, (*it)->rect().top - scrollOffsetY);
        }
    }

    // force repaint during scrolling
    repaint(true);
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::setEnabled(bool bEnabled)
{
    // reset focus if set
    if(!bEnabled && hasFocus())
    {
        setFocus(false);
    }

    // set flag
    setStateFlag(STATE_FLAG_DISABLED, !bEnabled);

    // set also to child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        // init
        (*it)->setStateFlag(STATE_FLAG_DISABLED, !bEnabled);
    }
}

bool XGraphicsItem::isEnabled() const
{
    return !getStateFlag(STATE_FLAG_DISABLED);
}

void XGraphicsItem::setVisible(bool bVisible)
{
    // ignore if the same
    if(m_visible == bVisible) return;

    // update state
    _setVisibleImpl(bVisible);

    // copy flag
    m_visible = bVisible;
}

void XGraphicsItem::setObscured(bool bObscured)
{
    // ignore if the same
    if(m_obscured == bObscured) return;

    // update animations
    if(bObscured)
    {
        // pause animations if any
        pauseAnimations();

    } else
    {
        // resume animations if any
        resumeAnimations();
    }

    // copy flag
    m_obscured = bObscured;

    // set also to child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        // init
        (*it)->setObscured(bObscured);
    }
}

bool XGraphicsItem::isObscured() const
{
    return m_obscured;
}

void XGraphicsItem::setClickable(bool clickable)
{
    // copy property
    m_clickable = clickable;
}

bool XGraphicsItem::isClickable() const
{
    return m_clickable;
}

/////////////////////////////////////////////////////////////////////
// painter type
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::setPainterType(XWUIGraphicsPainter type)
{
    m_graphicsPainter = (type != XWUI_PAINTER_AUTOMATIC) ? type : sXWUIDefaultPainter();

    // set also to child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        // init
        (*it)->setPainterType(type);
    }
}

/////////////////////////////////////////////////////////////////////
// focus
/////////////////////////////////////////////////////////////////////
bool XGraphicsItem::isFocusable() const
{
    // ignore if not visible or not enabled
    if(!isVisible() || !isEnabled()) return false;

    // check if any child item is focusable
    for(std::list<XGraphicsItem*>::const_iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        // check item
        if((*it)->isFocusable()) return true;
    }

    return m_focusable;
}

void XGraphicsItem::setFocus(bool bFocus)
{
    // ignore if not visible or not enabled
    if(bFocus && (!isVisible() || !isEnabled())) return;

    // ignore if same
    if(hasFocus() == bFocus) return;

    if(bFocus && m_focusItem == 0)
    {
        // find child item to focus
        _focusNextItem();

    } else
    {
        // pass change to current focus item
        if(m_focusItem) m_focusItem->setFocus(bFocus);
    }

    // set focus property
    setFocusProperty(bFocus);
}

bool XGraphicsItem::hasFocus() const
{
    return getStateFlag(STATE_FLAG_FOCUSED);
}

/////////////////////////////////////////////////////////////////////
// mouse events
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::onMouseEnter(int posX, int posY)
{
    // ignore if not visible or not enabled
    if(!isVisible() || !isEnabled()) return;
    
    // pass to child items if any
    if(m_childItems.size())
    {
        // inform old item if any (should not be set, but do just in case)
        if(m_mouseItem) m_mouseItem->onMouseLeave();

        // find item
        m_mouseItem = _findItem(posX, posY);

        // inform about mouse enter
        if(m_mouseItem) m_mouseItem->onMouseEnter(posX, posY);
    }

    // change cursor if needed
    if(m_itemCursor)
    {
        // reset original cursor if any
        if(m_originalCursor)
            ::SetCursor(m_originalCursor);

        // set new cursor
        m_originalCursor = ::SetCursor(m_itemCursor);
    }

    // update style property
    setStateFlag(STATE_FLAG_MOUSEOVER, true);
}

void XGraphicsItem::onMouseMove(int posX, int posY, WPARAM flags)
{
    // ignore if not visible or not enabled
    if(!isVisible() || !isEnabled()) return;

    // pass to child items if any
    if(m_childItems.size())
    {
        // find item from position
        XGraphicsItem* posItem = _findItem(posX, posY);
        if(posItem)
        {
            // check if item is the same
            if(m_mouseItem && m_mouseItem->xwoid() == posItem->xwoid())
            {
                // pass to active item
                m_mouseItem->onMouseMove(posX, posY, flags);

            } else
            {
                // leave from active mouse item
                if(m_mouseItem) m_mouseItem->onMouseLeave();

                // set new mouse item
                m_mouseItem = posItem;
                m_mouseItem->onMouseEnter(posX, posY);
            }

        } else
        {
            // leave from active mouse item
            if(m_mouseItem) m_mouseItem->onMouseLeave();
            m_mouseItem = 0;
        }
    }
}

bool XGraphicsItem::onMouseHover(int posX, int posY)
{
    // ignore if not visible or not enabled
    if(!isVisible() || !isEnabled()) return false;

    // pass to active item if any
    if(m_mouseItem)
    {
        // pass event to item
        if(m_mouseItem->onMouseHover(posX, posY)) return true;
    }
    
    // show current item hint text
    if(m_hintText.length() > 0)
    {
        // show tooltip
        ::SendMessageW(m_hwndParent, WM_XWUI_GITEM_SHOW_TOOLTIP, 0, (LPARAM)m_hintText.c_str()); 

        // mark as consumed
        return true;
    }

    // ignore event
    return false;
}

bool XGraphicsItem::onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags)
{
    // ignore if not visible or not enabled
    if(!isVisible() || !isEnabled()) return false;

    // pass to mouse item if it can handle mouse clicks
    if(m_mouseItem && m_mouseItem->m_clickable)
    {
        // handle mouse click
        return m_mouseItem->onMouseClick(uButtonMsg, posX, posY, flags);
    }

    // check if item is clickable
    if(m_clickable)
    {        
        if(uButtonMsg == WM_LBUTTONDOWN)
        {
            // update style property
            setStateFlag(STATE_FLAG_PRESSED, true);

            // start mouse capture
            setMouseCapture();

            // mark as processed
            return true;

        } else if(uButtonMsg == WM_LBUTTONUP)
        {
            // update style property
            setStateFlag(STATE_FLAG_PRESSED, false);

            // reset mouse capture
            if(getStateFlag(STATE_FLAG_MOUSECAPTURE))
            {
                resetMouseCapture();
            }

            // send event to listeners (only if mouse is still over)
            if(isInside(posX, posY))
            {
                sendEvent(XGITEM_EVENT_CLICKED);
            }

            // mark as processed
            return true;
        }
    }

    // pass to active item (if not processed)
    if(m_mouseItem)
    {
        if(uButtonMsg == WM_LBUTTONDOWN)
        {
            // set focus first
            if(m_mouseItem->isFocusable() && !m_mouseItem->hasFocus())
            {
                _focusItem(m_mouseItem);
            }
        }

        // handle mouse click
        return m_mouseItem->onMouseClick(uButtonMsg, posX, posY, flags);
    }

    return false;
}

void XGraphicsItem::onMouseLeave()
{
    // ignore if not visible or not enabled
    if(!isVisible() || !isEnabled()) return;

    // inform old item if any
    if(m_mouseItem)
    {
        m_mouseItem->onMouseLeave();
        m_mouseItem = 0;
    }

    // reset cursor if needed
    if(m_originalCursor)
    {
        ::SetCursor(m_originalCursor);
        m_originalCursor = 0;
    }

    // update style property
    setStateFlag(STATE_FLAG_MOUSEOVER, false);
}

bool XGraphicsItem::onMouseWheel(int wheelDelta, int posX, int posY, WPARAM flags)
{
    // ignore if not visible or not enabled
    if(!isVisible() || !isEnabled()) return false;

    // pass to active item if any
    if(m_mouseItem) 
    {
        return m_mouseItem->onMouseWheel(wheelDelta, posX, posY, flags);
    }

    return false;
}

void XGraphicsItem::onMouseCaptureReset()
{
    // reset state flags
    setStateFlag(STATE_FLAG_MOUSECAPTURE, false);
    setStateFlag(STATE_FLAG_PRESSED, false);
}

bool XGraphicsItem::onSetCursor()
{
    // ignore if not visible or not enabled
    if(!isVisible() || !isEnabled()) return false;

    // stop processing if there is custom cursor set
    if(m_itemCursor) return true;

    // pass to active item if any
    if(m_mouseItem) 
    {
        return m_mouseItem->onSetCursor();
    }

    // use system cursor
    return false;
}

/////////////////////////////////////////////////////////////////////
// context menu
/////////////////////////////////////////////////////////////////////
bool XGraphicsItem::onContextMenu(int posX, int posY)
{
    // ignore if not visible or not enabled
    if(!isVisible() || !isEnabled()) return false;

    // pass to active item if any
    if(m_mouseItem)
    {
        // pass event to item
        if(m_mouseItem->onContextMenu(posX, posY)) return true;
    }

    // show context menu if set
    if(m_contextMenuEnabled && m_contextMenu)
    {
        // show context menu
        ::SendMessageW(m_hwndParent, WM_XWUI_GITEM_SHOW_CONTEXT_MENU, xwoid(), MAKELPARAM(posX, posY)); 

        // mark as consumed
        return true;
    }

    // ignore event
    return false;
}

void XGraphicsItem::onContextMenuItem(UINT itemId)
{
    // do nothing in default implementation
}

/////////////////////////////////////////////////////////////////////
// keyboard events (sent only to focused item)
/////////////////////////////////////////////////////////////////////
bool XGraphicsItem::onCharEvent(WPARAM charCode, LPARAM flags)
{
    // ignore if not visible or not enabled
    if(!isVisible() || !isEnabled()) return false;

    // ignore all keys if not focused
    if(!hasFocus()) return false;

    // pass to focused item if any
    bool bProcessed = false;
    if(m_focusItem)
    {
        // pass to focused item
        bProcessed = m_focusItem->onCharEvent(charCode, flags);
    }

    // check common keys
    if(!bProcessed)
    {
        // check key code
        switch(charCode)
        {
        case VK_TAB:
            // focus next item
            _focusNextItem();

            // check if some item has focus
            return (m_focusItem != 0);
            break;
        }
    }

    // not handled
    return false;
}

/////////////////////////////////////////////////////////////////////
// timer events
/////////////////////////////////////////////////////////////////////
bool XGraphicsItem::onTimerEvent()
{
    // ignore by default
    return false;
}

/////////////////////////////////////////////////////////////////////
// graphics item events
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::onItemEvent(LPARAM param)
{
    // do nothing in default implementation
}

void XGraphicsItem::onBroadcastEvent(LPARAM param)
{
    // pass to itself
    onItemEvent(param);

    // pass to all child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); 
        it != m_childItems.end(); ++it)
    {
        (*it)->onBroadcastEvent(param);
    }
}

/////////////////////////////////////////////////////////////////////
// animation events
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::onAnimationTimer(DWORD id)
{
    // do nothing in default implementation
}

void XGraphicsItem::onAnimationValue(DWORD id, float value)
{
    // do nothing in default implementation
}

void XGraphicsItem::onAnimationCompleted(DWORD id)
{
    // find from animations
    std::vector<DWORD>::iterator it = std::find(m_itemAnimations.begin(), m_itemAnimations.end(), id);
    if(it != m_itemAnimations.end())
    {
        // remove from list
        m_itemAnimations.erase(it);
    }
}

/////////////////////////////////////////////////////////////////////
// content loading events
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::onUrlContentLoaded(DWORD id, const WCHAR* path)
{
    // do nothing in default implementation
}

void XGraphicsItem::onUrlContentLoadFailed(DWORD id, DWORD reason)
{
    // do nothing in default implementation
}

void XGraphicsItem::onUrlContentCompleted(DWORD id)
{
    // find from conten ids
    std::vector<DWORD>::iterator it = std::find(m_itemContentIds.begin(), m_itemContentIds.end(), id);
    if(it != m_itemContentIds.end())
    {
        // remove from list
        m_itemContentIds.erase(it);
    }
}

/////////////////////////////////////////////////////////////////////
// hit testing
/////////////////////////////////////////////////////////////////////
bool XGraphicsItem::isInside(int posX, int posY)
{
    // check if point is inside item
    return XWUtils::rectIsInside(m_itemRect, posX, posY);
}

bool XGraphicsItem::rectOverlap(const RECT& rect)
{
    // check if rectangle overlaps with item
    return XWUtils::rectOverlap(m_itemRect, rect);
}

/////////////////////////////////////////////////////////////////////
// GDI painting
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::onPaintGDI(HDC hdc, const RECT& rcPaint)
{
    // ignore if not visible
    if(!isVisible()) return;

    // fill background first if needed
    if(m_fillBackground)
    {
        XGdiHelpers::fillRect(hdc, rcPaint, m_bgColor);
    }

    // paint child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        // ignore not visible items
        if(!(*it)->isVisible()) continue;

        // compute paint rectangle for item
        RECT rcItemPaint;

        // ignore if update rectangle doesn't overlap
        if(!XWUtils::rectIntersect((*it)->rect(), rcPaint, rcItemPaint)) continue;

        // paint
        (*it)->onPaintGDI(hdc, rcItemPaint);
    }
}

/////////////////////////////////////////////////////////////////////
// GDI resource caching
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::onInitGDIResources(HDC hdc)
{
    // release previous resources if any
    onResetGDIResources();

    // make sure we always have GDI cache
    _checkGdiCacheReady(hdc);

    // init resources for child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        // init
        (*it)->onInitGDIResources(hdc);
    }
}

void XGraphicsItem::onResetGDIResources()
{
    // reset cache for child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        // reset
        (*it)->onResetGDIResources();
    }
}

void XGraphicsItem::setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache)
{
    // release previous instance if any
    if(m_pXGdiResourcesCache)
    {
        // reset resources as they might be using cache
        onResetGDIResources();

        // release
        m_pXGdiResourcesCache->Release();
    }

    // copy reference
    m_pXGdiResourcesCache = pXGdiResourcesCache;

    // add reference if needed
    if(m_pXGdiResourcesCache)
        m_pXGdiResourcesCache->AddRef();

    // set cache for child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        // set
        (*it)->setGDIResourcesCache(m_pXGdiResourcesCache);
    }
}

/////////////////////////////////////////////////////////////////////
// Direct2D painting
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint)
{
    // ignore if not visible
    if(!isVisible()) return;

    // convert rect
    D2D1_RECT_F d2dRect;
    XD2DHelpers::gdiRectToD2dRect(rcPaint, d2dRect);

    // NOTE: do not clip regions here as some items may use GDI ineroperability

    // Quote from MSDN: http://msdn.microsoft.com/en-us/library/windows/desktop/dd371323(v=vs.85).aspx
    // "Note  In Windows 7 and earlier, you should not call GetDC between PushAxisAlignedClip/PopAxisAlignedClip commands 
    //  or between PushLayer/PopLayer. However, this restriction does not apply to Windows 8 and later."

    // NOTE: do not clip region
    //  pTarget->PushAxisAlignedClip(d2dRect, D2D1_ANTIALIAS_MODE_ALIASED);

    // fill background first if needed
    if(m_fillBackground)
    {
        // convert color
        D2D1_COLOR_F d2dBackgroundFillColor;
        XD2DHelpers::colorrefToD2dColor(m_bgColor, d2dBackgroundFillColor);

        // get brush from cache and fill
        ID2D1Brush* backgroundFillBrush = createD2DBrush(pTarget, d2dBackgroundFillColor);
        if(backgroundFillBrush)
        {
            pTarget->FillRectangle(d2dRect, backgroundFillBrush);
            backgroundFillBrush->Release();
        }
    }

    // paint child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        // ignore not visible items
        if(!(*it)->isVisible()) continue;

        // compute paint rectangle for item
        RECT rcItemPaint;

        // ignore if update rectangle doesn't overlap
        if(!XWUtils::rectIntersect((*it)->rect(), rcPaint, rcItemPaint)) continue;

        // paint
        (*it)->onPaintD2D(pTarget, rcItemPaint);
    }

    // NOTE: do not clip
    // pTarget->PopAxisAlignedClip();
}

/////////////////////////////////////////////////////////////////////
// Direct2D resource caching
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::onInitD2DTarget(ID2D1RenderTarget* pTarget)
{
    // reset previous target if any
    onResetD2DTarget();

    // make sure we always have D2D cache
    _checkD2DCacheReady(pTarget);

    // init target for child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        // init
        (*it)->onInitD2DTarget(pTarget);
    }
}

void XGraphicsItem::onResetD2DTarget()
{
    // reset target for child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        // reset
        (*it)->onResetD2DTarget();
    }

    // release GDI render target if any
    if(m_pGDIRenderTarget) 
    { 
        m_pGDIRenderTarget->Release();
        m_pGDIRenderTarget = 0;
    }
}

void XGraphicsItem::setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache)
{
    // release previous instance if any
    if(m_pXD2DResourcesCache)
    {
        // reset previous resources
        onResetD2DTarget();

        // release cache
        m_pXD2DResourcesCache->Release();
    }

    // copy reference
    m_pXD2DResourcesCache = pXD2DResourcesCache;

    // add reference if needed
    if(m_pXD2DResourcesCache)
        m_pXD2DResourcesCache->AddRef();

    // init cache for child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        // init
        (*it)->setD2DResourcesCache(m_pXD2DResourcesCache);
    }
}

/////////////////////////////////////////////////////////////////////
// item state
/////////////////////////////////////////////////////////////////////
XWUIStyle::XStyleState XGraphicsItem::styleState() const
{
    // check state
    if(m_stateFlags & STATE_FLAG_DISABLED) return XWUIStyle::eStyleStateDisabled;
    if(m_stateFlags & STATE_FLAG_PRESSED) return XWUIStyle::eStyleStatePressed;
    if(m_stateFlags & STATE_FLAG_MOUSEOVER) return XWUIStyle::eStyleStateMouseOver;
    if(m_stateFlags & STATE_FLAG_FOCUSED) return XWUIStyle::eStyleStateFocused;

    // default style
    return XWUIStyle::eStyleStateDefault;
}

/////////////////////////////////////////////////////////////////////
// update ui
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::updateUI()
{
    // update with the same size
    update(m_itemRect.left, m_itemRect.top, width(), height());

    // repaint
    repaint();
}

/////////////////////////////////////////////////////////////////////
// paint helpers
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::repaint(bool paintNow)
{
    // repaint whole client area
    repaint(m_itemRect, paintNow);
}

void XGraphicsItem::repaint(const RECT& rcPaint, bool paintNow)
{
    // ignore if parent is not set
    if(m_hwndParent == 0) return;

    bool paintDone = false;

    // check if we need to repaint immediately
    if(paintNow)
    {
        // check painter type
        if(painterType() == XWUI_PAINTER_GDI)
        {
            // repaint
            HDC hdc = ::GetDC(m_hwndParent);
            onPaintGDI(hdc, m_itemRect);
            ::ReleaseDC(m_hwndParent, hdc);

            // mark flag
            paintDone = true;

        } else if(painterType() == XWUI_PAINTER_D2D)
        {
            // check if we have target set already
            if(m_pXD2DResourcesCache->renderTarget() != 0)
            {
                // repaint
                m_pXD2DResourcesCache->renderTarget()->BeginDraw();
                onPaintD2D(m_pXD2DResourcesCache->renderTarget(), m_itemRect);
                m_pXD2DResourcesCache->renderTarget()->EndDraw();

                // mark flag
                paintDone = true;
            }
        }
    }

    // inform parent window that repaint is needed
    if(!paintDone)
    {
        ::InvalidateRect(m_hwndParent, &rcPaint, FALSE);
    }
}

void XGraphicsItem::repaint(HRGN hrgn)
{
    XWASSERT(hrgn);
    if(hrgn == 0) return;

    RECT rcPaint;

    // get bounding rectangle
    ::GetRgnBox(hrgn, &rcPaint);
    
    // repaint area
    repaint(rcPaint);
}

/////////////////////////////////////////////////////////////////////
// set size constraints
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::setMinWidth(int minWidth)
{
    XWASSERT(minWidth >= 0);

    // set minimum width
    m_minWidth = minWidth;

    // update policy
    m_rpHorizontal = (m_rpHorizontal == eResizeMax) ? eResizeMinMax : eResizeMin;
}

void XGraphicsItem::setMinHeight(int minHeight)
{
    XWASSERT(minHeight >= 0);

    // set height
    m_minHeight = minHeight;

    // update policy
    m_rpVertical = (m_rpVertical == eResizeMax) ? eResizeMinMax : eResizeMin;
}

void XGraphicsItem::setMaxWidth(int maxWidth)
{
    XWASSERT(maxWidth >= 0);

    // set width
    m_maxWidth = maxWidth;

    // update policy
    m_rpHorizontal = (m_rpHorizontal == eResizeMin) ? eResizeMinMax : eResizeMax;
}

void XGraphicsItem::setMaxHeight(int maxHeight)
{
    XWASSERT(maxHeight >= 0);

    // set height
    m_maxHeight = maxHeight;

    // update policy
    m_rpVertical = (m_rpVertical == eResizeMin) ? eResizeMinMax : eResizeMax;
}

/////////////////////////////////////////////////////////////////////
// fix size
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::setFixedWidth(int width)
{
    XWASSERT(width >= 0);

    // set limits
    m_minWidth = width;
    m_maxWidth = width;

    // update policy
    m_rpHorizontal = eResizeMinMax;
}

void XGraphicsItem::setFixedHeight(int height)
{
    XWASSERT(height >= 0);

    // set limits
    m_minHeight = height;
    m_maxHeight = height;

    // update policy
    m_rpVertical = eResizeMinMax;
}

void XGraphicsItem::setFixedSize(int width, int height)
{
    XWASSERT(width >= 0);
    XWASSERT(height >= 0);

    // set limits
    m_minWidth = width;
    m_maxWidth = width;
    m_minHeight = height;
    m_maxHeight = height;

    // update policy
    m_rpHorizontal = eResizeMinMax;
    m_rpVertical = eResizeMinMax;
}

/////////////////////////////////////////////////////////////////////
// reset size policy
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::resetSizePolicy()
{
    m_rpHorizontal = eResizeAny;
    m_rpVertical = eResizeAny;
}

/////////////////////////////////////////////////////////////////////
// resize policy (from IXLayoutItem)
/////////////////////////////////////////////////////////////////////
IXLayoutItem::TResizePolicy XGraphicsItem::horizontalPolicy() const
{
    // use item policy if set or no layout
    if(m_rpHorizontal != eResizeAny || m_pLayout == 0)
    {
        return m_rpHorizontal;
    }

    // use layout
    return m_pLayout->horizontalPolicy();
}

IXLayoutItem::TResizePolicy XGraphicsItem::verticalPolicy() const
{
    // use item policy if set or no layout
    if(m_rpVertical != eResizeAny || m_pLayout == 0)
    {
        return m_rpVertical;
    }

    // use layout
    return m_pLayout->verticalPolicy();
}

void XGraphicsItem::updateResizePolicies()
{
    // pass to layout if set
    if(m_pLayout)
    {
        m_pLayout->updateResizePolicies();
    }
}

/////////////////////////////////////////////////////////////////////
// size constraints (from IXLayoutItem)
/////////////////////////////////////////////////////////////////////
int XGraphicsItem::minWidth()  const
{
    // use item width if set or no layout
    if(m_rpHorizontal == eResizeMin ||
       m_rpHorizontal == eResizeMinMax ||
       m_pLayout == 0)
    {
        return m_minWidth;
    }

    // use layout 
    return m_pLayout->minWidth();
}

int XGraphicsItem::minHeight() const
{
    // use item width if set or no layout
    if(m_rpVertical == eResizeMin ||
       m_rpVertical == eResizeMinMax ||
       m_pLayout == 0)
    {
        return m_minHeight;
    }

    // use layout 
    return m_pLayout->minHeight();
}

int XGraphicsItem::maxWidth()  const
{
    // use item width if set or no layout
    if(m_rpHorizontal == eResizeMax ||
       m_rpHorizontal == eResizeMinMax ||
       m_pLayout == 0)
    {
        return m_maxWidth;
    }

    // use layout 
    return m_pLayout->maxWidth();
}

int XGraphicsItem::maxHeight() const
{
    // use item width if set or no layout
    if(m_rpVertical == eResizeMax ||
       m_rpVertical == eResizeMinMax ||
       m_pLayout == 0)
    {
        return m_maxHeight;
    }

    // use layout 
    return m_pLayout->maxHeight();
}

/////////////////////////////////////////////////////////////////////
// background fill 
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::setBackgroundFill(const COLORREF& fillColor)
{
    // init backgdound fill
    m_bgColor = fillColor;
    m_fillBackground = true;
}

void XGraphicsItem::clearBackgroundFill()
{
    // reset flag
    m_fillBackground = false;
}

/////////////////////////////////////////////////////////////////////
// enable default content scrolling
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::enableContentScrolling(bool bEnable)
{
    // copy flag
    m_scrollingEnabled = bEnable;
}

/////////////////////////////////////////////////////////////////////
// timer methods
/////////////////////////////////////////////////////////////////////
bool XGraphicsItem::startTimer(UINT uElapseMs)
{
    // ignore if window not set
    if(m_hwndParent == 0) return false;

    if(::SetTimer(m_hwndParent, xwoid(), uElapseMs, 0) == 0)
    {
        XWTRACE_WERR_LAST("XGraphicsItem: failed to start timer");
        return false;
    }

    return true;
}

bool XGraphicsItem::stopTimer()
{
    // ignore if window not set
    if(m_hwndParent == 0) return false;

    if(::KillTimer(m_hwndParent, xwoid()) == 0)
    {
        XWTRACE_WERR_LAST("XGraphicsItem: failed to stop timer");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////
// animation methods
/////////////////////////////////////////////////////////////////////
bool XGraphicsItem::startTimerAnimation(UINT intervalMs, DWORD& idOut)
{
    // ignore if window not set
    XWASSERT(m_hwndParent);
    if(m_hwndParent == 0) return false;

    // remove not active animations just in case
    validateAnimations();

    // start animation
    if(XWAnimationTimer::instance()->startTimerAnimation(intervalMs, m_hwndParent, idOut))
    {
        // add to item animations
        m_itemAnimations.push_back(idOut);

        return true;
    }

    return false;
}

bool XGraphicsItem::startValueAnimation(UINT intervalMs, const XWAnimationTimer::ValueAnimation& animation, DWORD& idOut)
{
    // ignore if window not set
    XWASSERT(m_hwndParent);
    if(m_hwndParent == 0) return false;

    // remove not active animations just in case
    validateAnimations();

    // start animation
    if(XWAnimationTimer::instance()->startValueAnimation(intervalMs, animation, m_hwndParent, idOut))
    {
        // add to item animations
        m_itemAnimations.push_back(idOut);

        return true;
    }

    return false;
}

void XGraphicsItem::stopAnimation(DWORD id)
{
    // find from animations
    std::vector<DWORD>::iterator it = std::find(m_itemAnimations.begin(), m_itemAnimations.end(), id);

    // check if found
    XWASSERT1(it != m_itemAnimations.end(), "XGraphicsItem: trying to stop uknown animation");
    if(it == m_itemAnimations.end()) return;

    // stop animation
    XWAnimationTimer::instance()->stopAnimation(id);

    // remove from list
    m_itemAnimations.erase(it);
}

void XGraphicsItem::validateAnimations()
{
    // loop over all animations
    for(std::vector<DWORD>::iterator it = m_itemAnimations.begin();
        it != m_itemAnimations.end();)
    {
        // check if still exists
        if(XWAnimationTimer::instance()->hasAnimation(*it))
        {
            // next
            ++it;

        } else
        {
            XWASSERT1(0, "XGraphicsItem: item contains not active animation");

            // remove
            it = m_itemAnimations.erase(it);
        }
    }
}

void XGraphicsItem::pauseAnimations()
{
    // loop over all animations
    for(std::vector<DWORD>::iterator it = m_itemAnimations.begin(); 
        it != m_itemAnimations.end(); ++it)
    {
        XWAnimationTimer::instance()->pauseAnimation(*it);
    }
}

void XGraphicsItem::resumeAnimations()
{
    // ingore if item is not visible or obscured
    if(!m_visible || m_obscured) return;

    // loop over all animations
    for(std::vector<DWORD>::iterator it = m_itemAnimations.begin(); 
        it != m_itemAnimations.end(); ++it)
    {
        XWAnimationTimer::instance()->resumeAnimation(*it);
    }
}

void XGraphicsItem::stopAllAnimations()
{
    // loop over all animations
    for(std::vector<DWORD>::iterator it = m_itemAnimations.begin();
        it != m_itemAnimations.end(); ++it)
    {
        // stop animation
        XWAnimationTimer::instance()->stopAnimation(*it);
    }

    // reset list
    m_itemAnimations.clear();
}

/////////////////////////////////////////////////////////////////////
// content loading
/////////////////////////////////////////////////////////////////////
bool XGraphicsItem::isUrlContentLoaded(const WCHAR* url, XMediaSource& srcOut)
{
    // check from content provider
    return XWContentProvider::instance()->isUrlContentLoaded(url, srcOut);
}

bool XGraphicsItem::loadUrlContent(const WCHAR* url, DWORD& idOut)
{
    // start loading
    if(XWContentProvider::instance()->loadUrlContent(url, m_hwndParent, idOut))
    {
        m_itemContentIds.push_back(idOut);
        return true;
    }

    return false;
}

void XGraphicsItem::cancelContentLoad(DWORD id)
{
    // find content id
    std::vector<DWORD>::iterator it = std::find(m_itemContentIds.begin(), m_itemContentIds.end(), id);

    // check if found
    XWASSERT1(it != m_itemContentIds.end(), "XGraphicsItem: trying to cancel uknown content");
    if(it == m_itemContentIds.end()) return;

    // cancel 
    XWContentProvider::instance()->cancelContentLoad(id);

    // remove from list
    m_itemContentIds.erase(it);
}

void XGraphicsItem::cancelAllContent()
{
    // loop over all content ids
    for(std::vector<DWORD>::iterator it = m_itemContentIds.begin();
        it != m_itemContentIds.end(); ++it)
    {
        // cancel loading
        XWContentProvider::instance()->cancelContentLoad(*it);
    }

    // reset list
    m_itemContentIds.clear();
}

/////////////////////////////////////////////////////////////////////
// helper methods
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::onPaintD2DFromGDI(ID2D1RenderTarget* pTarget, const RECT& rcPaint)
{
    // NOTE: create GDI target only if this method is called
    if(m_pGDIRenderTarget == 0)
    {
        // get GDI render target (NOTE: ignore return code)
        pTarget->QueryInterface(__uuidof(ID2D1GdiInteropRenderTarget), (void**)&m_pGDIRenderTarget);
    }

    // render using GDI
    if(m_pGDIRenderTarget) 
    { 
        // init render target
        HDC hdc = 0;
        HRESULT hr = m_pGDIRenderTarget->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hdc);

        // render item using GDI method
        if(SUCCEEDED(hr))
        {
            // render item 
            onPaintGDI(hdc, rcPaint);

            // reset render target
            m_pGDIRenderTarget->ReleaseDC(NULL);

        } else
        {
            XWTRACE_HRES("XGraphicsItem: failed to get DC for GDI compatible paint", hr);
        }
    }
}

/////////////////////////////////////////////////////////////////////
// protected properties
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::setFocusable(bool focusable)
{
    // copy property
    m_focusable = focusable;
}

void XGraphicsItem::setMessageProcessing(bool processing)
{
    // copy property
    m_messageProcessing = processing;
}

void XGraphicsItem::setFocusProperty(bool hasFocus)
{
    // set flag
    setStateFlag(STATE_FLAG_FOCUSED, hasFocus);
}

/////////////////////////////////////////////////////////////////////
// GDI resource cashing
/////////////////////////////////////////////////////////////////////
XGdiResourcesCache* XGraphicsItem::getGdiResourcesCache(HDC hdc)
{
    // make sure we always have GDI cache
    _checkGdiCacheReady(hdc);
    
    return m_pXGdiResourcesCache;
}

/////////////////////////////////////////////////////////////////////
// Direct2D resource cashing
/////////////////////////////////////////////////////////////////////
XD2DResourcesCache* XGraphicsItem::getD2DResourcesCache(ID2D1RenderTarget* pTarget)
{
    // make sure we always have D2D cache
    _checkD2DCacheReady(pTarget);

    return m_pXD2DResourcesCache;
}

ID2D1Brush* XGraphicsItem::createD2DBrush(ID2D1RenderTarget* pTarget, const D2D1_COLOR_F& color)
{
    XWASSERT(pTarget);
    if(pTarget == 0) return 0;

    // make sure we always have D2D cache
    _checkD2DCacheReady(pTarget);

    // check if we have cache
    XWASSERT(m_pXD2DResourcesCache);
    if(m_pXD2DResourcesCache) return m_pXD2DResourcesCache->getBrush(color);

    // create brush without cache
    ID2D1SolidColorBrush* brush = 0;

    // create brush
    HRESULT hr = pTarget->CreateSolidColorBrush(color, &brush);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XGraphicsItem::createD2DBrush failed to create brush", hr);
        return 0;
    }

    return brush;
}

ID2D1Brush* XGraphicsItem::createD2DBrush(ID2D1RenderTarget* pTarget, const COLORREF& color)
{
    // convert color
    D2D1_COLOR_F d2dColor;
    XD2DHelpers::colorrefToD2dColor(color, d2dColor);

    // create brush
    return createD2DBrush(pTarget, d2dColor);
}

/////////////////////////////////////////////////////////////////////
// resource management
/////////////////////////////////////////////////////////////////////
bool XGraphicsItem::hasResourceCache()
{
    // check if cache is set
    return  ( (painterType() == XWUI_PAINTER_GDI && m_pXGdiResourcesCache != 0) ||
              (painterType() == XWUI_PAINTER_D2D && m_pXD2DResourcesCache != 0 && m_pXD2DResourcesCache->renderTarget() != 0) );
}

void XGraphicsItem::reloadResources()
{
    // check painter type
    if(painterType() == XWUI_PAINTER_GDI)
    {
        // check if resources have been initialized
        if(m_pXGdiResourcesCache && m_hwndParent)
        {
            // reset previous resources
            onResetGDIResources();

            // window DC
            HDC hdc = ::GetDC(m_hwndParent);

            // re-init resources
            onInitGDIResources(hdc);

            // release DC
            ::ReleaseDC(m_hwndParent, hdc);
        }

    } else if(painterType() == XWUI_PAINTER_D2D)
    {
        // check if resources have been initialized
        if(m_pXD2DResourcesCache && m_pXD2DResourcesCache->renderTarget())
        {
            // reset previous resources
            onResetD2DTarget();

            // re-load resources
            onInitD2DTarget(m_pXD2DResourcesCache->renderTarget());
        }
    }
}

/////////////////////////////////////////////////////////////////////
// load images to cache
/////////////////////////////////////////////////////////////////////
bool XGraphicsItem::loadBitmap(const XMediaSource& source, std::wstring& hashOut)
{
    // use scaling version
    return loadBitmap(source, 0, 0, hashOut);
}

bool XGraphicsItem::loadBitmap(const XMediaSource& source, int width, int height, std::wstring& hashOut)
{
    // check painter type
    if(painterType() == XWUI_PAINTER_GDI)
    {
        // load bitmap
        if(m_pXGdiResourcesCache)
            return m_pXGdiResourcesCache->loadBitmap(source, width, height, hashOut);

    } else if(painterType() == XWUI_PAINTER_D2D)
    {
        // load bitmap
        if(m_pXD2DResourcesCache)
            return m_pXD2DResourcesCache->loadBitmap(source, width, height, hashOut);
    }

    return false;
}

bool XGraphicsItem::releaseBitmap(std::wstring& bitmapHash)
{
    // check painter type
    if(painterType() == XWUI_PAINTER_GDI)
    {
        // release image
        if(m_pXGdiResourcesCache)
        {
            m_pXGdiResourcesCache->releaseBitmap(bitmapHash);
            return true;
        }

    } else if(painterType() == XWUI_PAINTER_D2D)
    {
        // release image
        if(m_pXD2DResourcesCache)
        {
            m_pXD2DResourcesCache->releaseBitmap(bitmapHash);
            return true;
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////
// item state
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::setStateFlag(TStateFlag flag, bool value)
{
    // ignore if the same
    if(getStateFlag(flag) == value) return;

    // update state
    if(value)
        m_stateFlags |= flag;
    else
        m_stateFlags &= ~((unsigned long)flag);

    // pass event
    onStateFlagChanged(flag, value);
}

bool XGraphicsItem::getStateFlag(TStateFlag flag) const
{
    // pass to style
    return ((m_stateFlags & flag) == flag);
}

void XGraphicsItem::onStateFlagChanged(TStateFlag flag, bool value)
{
    // do nothing in default implementation
}

/////////////////////////////////////////////////////////////////////
// events
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::onChildObjectRemoved(XWObject* child)
{
    if(child == 0) return;

    // find item
    std::list<XGraphicsItem*>::iterator it = _findItemIt(child->xwoid());

    // ignore if item not found
    if(it == m_childItems.end()) return;

    // remove from layout
    if(m_pLayout)
        m_pLayout->removelayoutItem(child->xwoid());

    // remove from child items list
    m_childItems.erase(it);

    // check item references
    if(m_mouseItem && m_mouseItem->xwoid() == child->xwoid()) m_mouseItem = 0;
    
    if(m_focusItem && m_focusItem->xwoid() == child->xwoid()) 
    {
        // reset current item
        m_focusItem = 0;

        // focus next
        _focusNextItem();
    }
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XGraphicsItem::_setVisibleImpl(bool bVisible)
{
    // reset properties
    if(!bVisible)
    {
        // reset focus if any
        if(hasFocus()) setFocus(false);

        // reset states
        m_stateFlags &= ~((unsigned long)(STATE_FLAG_MOUSEOVER | STATE_FLAG_PRESSED | STATE_FLAG_MOUSECAPTURE));

        // pause animations if any
        pauseAnimations();

    } else
    {
        // resume animations if any
        resumeAnimations();
    }

    // set to child items
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        // update state
        (*it)->_setVisibleImpl(bVisible);
    }
}

void XGraphicsItem::_checkGdiCacheReady(HDC hdc)
{
    // make sure we always have GDI cache
    if(m_pXGdiResourcesCache == 0)
    {
        // create own cache
        XGdiResourcesCache* gdiCache = new XGdiResourcesCache;
        gdiCache->init(hdc);

        // set cache
        setGDIResourcesCache(gdiCache);
    }
}

void XGraphicsItem::_checkD2DCacheReady(ID2D1RenderTarget* pTarget)
{
    // make sure we always have D2D cache
    if(m_pXD2DResourcesCache == 0)
    {
        // create own cache
        XD2DResourcesCache* d2dCache = new XD2DResourcesCache;
        d2dCache->init(pTarget);

        // set cache
        setD2DResourcesCache(d2dCache);
    }
}

void XGraphicsItem::_focusNextItem()
{
    // ignore if no items
    if(m_childItems.size() == 0) return;

    // find current focus item if any
    std::list<XGraphicsItem*>::iterator it = m_childItems.end();
    if(m_focusItem)
    {
        // find focus item reference
        it = _findItemIt(m_focusItem->xwoid());
    }

    // check if focus item has been found
    if(it != m_childItems.end())
    {
        std::list<XGraphicsItem*>::iterator findIt = it;

        // start from the end if we are at the beginning
        if(findIt == m_childItems.begin()) findIt = m_childItems.end();

        // previous item
        --findIt;

        // focus next item (in z-order)
        while(findIt != it)
        {
            // check if item is focusable
            if((*findIt)->isFocusable() && (*findIt)->isEnabled())
            {
                // change focus item
                _focusItem((*findIt));

                // stop search
                break;
            }

            // start from the end if we are at the beginning
            if(findIt == m_childItems.begin()) findIt = m_childItems.end();

            // previous item
            --findIt;
        }
    }
    else
    {
        // no active focus item, just search in reverse order
        for(std::list<XGraphicsItem*>::reverse_iterator rit = m_childItems.rbegin(); rit != m_childItems.rend(); ++rit)
        {
            // check if item is focusable
            if((*rit)->isFocusable() && (*rit)->isEnabled())
            {
                // change focus item
                _focusItem((*rit));

                // stop search
                break;
            }
        }
    }
}

void XGraphicsItem::_focusItem(XGraphicsItem* pItem)
{
    // ignore if same
    if(m_focusItem == pItem && m_focusItem->hasFocus()) return;

    // remove focus from previous item
    if(m_focusItem) m_focusItem->setFocus(false);

    // set new focus item
    m_focusItem = pItem;

    // set focus for new item
    if(m_focusItem) m_focusItem->setFocus(true);
}

XGraphicsItem* XGraphicsItem::_findItem(int posX, int posY)
{
    // loop over all items in list (in z-order)
    for(std::list<XGraphicsItem*>::reverse_iterator rit = m_childItems.rbegin(); rit != m_childItems.rend(); ++rit)
    {
        // ignore not visible or not enabled items
        if(!(*rit)->isVisible() || !(*rit)->isEnabled()) continue;

        // check if point is inside
        if((*rit)->isInside(posX,posY)) return (*rit);
    }

    // not found
    return 0;
}

XGraphicsItem* XGraphicsItem::_findItem(unsigned long itemId)
{
    // loop over all items in list 
    for(std::list<XGraphicsItem*>::const_iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        // check id
        if((*it)->xwoid() == itemId) return (*it);

        // search for child items
        XGraphicsItem* item = (*it)->_findItem(itemId);
        if(item != 0) return item;
    }

    // not found
    return 0;
}

std::list<XGraphicsItem*>::iterator XGraphicsItem::_findItemIt(unsigned long itemId)
{
    // find item by id
    for(std::list<XGraphicsItem*>::iterator it = m_childItems.begin(); it != m_childItems.end(); ++it)
    {
        // check id
        if((*it)->xwoid() == itemId) return it;
    }

    // not found
    return m_childItems.end();
}

// XGraphicsItem
/////////////////////////////////////////////////////////////////////
