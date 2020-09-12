// Popup menu
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xpopupmenu.h"

/////////////////////////////////////////////////////////////////////
// XPopupMenu - popup menu

XPopupMenu::XPopupMenu(XWObject* parent) :
    XWObject(parent),
    m_hMenu(0)
{
    // create menu
    m_hMenu = ::CreatePopupMenu();

    XWASSERT1(m_hMenu, "XPopupMenu: failed to create menu");
}

XPopupMenu::~XPopupMenu()
{
    // destroy menu
    if(m_hMenu)
    {
        if(!::DestroyMenu(m_hMenu))
        {
            XWTRACE_WERR_LAST("XPopupMenu: failed to destroy menu");
        }

        m_hMenu = 0;
    }
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
bool XPopupMenu::track(HWND hwnd, int posX, int posY, UINT& selectedItem)
{
    // show menu
    BOOL retVal = ::TrackPopupMenuEx(m_hMenu, 
                                     TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON,
                                     posX,
                                     posY,
                                     hwnd,
                                     0);

    // copy return value
    if(retVal != 0) selectedItem = retVal;

    // check if user selected item
    return (retVal != 0);
}

/////////////////////////////////////////////////////////////////////
// items
/////////////////////////////////////////////////////////////////////
void XPopupMenu::insertItem(UINT pos, const MenuItem& item)
{
    // convert item
    MENUITEMINFO mii;
    _convertItem(item, mii);

    // insert item
    if(::InsertMenuItemW(m_hMenu, pos, TRUE, &mii) == 0)
    {
        XWTRACE_WERR_LAST("XPopupMenu: failed to insert menu item");
    }
}

void XPopupMenu::insertTextItem(UINT pos, const wchar_t* text, UINT id)
{
    XWASSERT(text);
    if(text == 0) return;

    // init item
    MenuItem item;
    item.text = text;
    item.id = id;

    // insert item
    insertItem(pos, item);
}

void XPopupMenu::insertItems(UINT pos, const std::vector<MenuItem>& items)
{
    // insert items
    for(std::vector<MenuItem>::const_iterator it = items.begin();
        it != items.end(); ++it)
    {
        // insert item
        insertItem(pos, *it);

        // next position
        pos++;
    }
}

void XPopupMenu::insertSubMenu(UINT pos, const wchar_t* text, const std::vector<MenuItem>& items)
{
    MENUITEMINFO mii;

    XWASSERT(text);
    if(text == 0) return;

    // create submenu
    HMENU subMenu = ::CreateMenu();
    if(subMenu == 0)
    {
        XWTRACE_WERR_LAST("XPopupMenu: failed to create submenu");
        return;
    }

    // append items
    UINT subPos = 0;
    for(std::vector<MenuItem>::const_iterator it = items.begin();
        it != items.end(); ++it)
    {
        // convert
        _convertItem(*it, mii);

        // insert item
        if(::InsertMenuItemW(subMenu, subPos, TRUE, &mii) == 0)
        {
            XWTRACE_WERR_LAST("XPopupMenu: failed to insert submenu item");

            // destroy submenu and exit
            ::DestroyMenu(subMenu);
            return;
        }

        // next position
        subPos++;
    }

    // init
    ::ZeroMemory(&mii, sizeof(MENUITEMINFO));
    mii.cbSize = sizeof(MENUITEMINFO);

    // flags
    mii.fMask = MIIM_SUBMENU | MIIM_STRING;
    mii.dwTypeData = (WCHAR*)text;
    mii.cch = (UINT)wcslen(text);
    mii.hSubMenu = subMenu;

    // insert submenu menu
    if(::InsertMenuItemW(m_hMenu, pos, TRUE, &mii) == 0)
    {
        XWTRACE_WERR_LAST("XPopupMenu: failed to insert submenu item");

        // destroy submenu
        ::DestroyMenu(subMenu);
    }
}

void XPopupMenu::appendItem(const MenuItem& item)
{
    // insert at the end
    insertItem(_menuItemCount(), item);
}

void XPopupMenu::appendTextItem(const wchar_t* text, UINT id)
{
    // insert at the end
    insertTextItem(_menuItemCount(), text, id);
}

void XPopupMenu::appendItems(const std::vector<MenuItem>& items)
{
    // insert at the end
    insertItems(_menuItemCount(), items);
}

void XPopupMenu::appendSubMenu(const wchar_t* text, const std::vector<MenuItem>& items)
{
    // insert at the end
    insertSubMenu(_menuItemCount(), text, items);
}

/////////////////////////////////////////////////////////////////////
// manage items
/////////////////////////////////////////////////////////////////////
void XPopupMenu::enableMenuItem(UINT id, bool enable)
{
    MENUITEMINFO mii;

    // init
    ::ZeroMemory(&mii, sizeof(MENUITEMINFO));
    mii.cbSize = sizeof(MENUITEMINFO);

    // state
    mii.fMask = MIIM_STATE;
    mii.fState = enable ? MFS_ENABLED : MFS_DISABLED;

    // update
    if(::SetMenuItemInfoW(m_hMenu, id, FALSE, &mii) == 0)
    {
        XWTRACE_WERR_LAST("XPopupMenu: failed change menu item enabled state");
    }
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XPopupMenu::_convertItem(const MenuItem& item, MENUITEMINFO& itemOut)
{
    // init
    ::ZeroMemory(&itemOut, sizeof(MENUITEMINFO));
    itemOut.cbSize = sizeof(MENUITEMINFO);

    // flags
    if(item.id) itemOut.fMask |= MIIM_ID;
    if(item.bitmap) itemOut.fMask |= MIIM_ID;
    if(item.text.length()) itemOut.fMask |= MIIM_STRING;
    if(item.separator) itemOut.fMask |= MIIM_FTYPE;

    // type
    if(item.separator) itemOut.fMask |= MFT_SEPARATOR;

    // data
    itemOut.wID = item.id;
    itemOut.hbmpItem = item.bitmap;
    itemOut.dwTypeData = (WCHAR*)item.text.c_str();
    itemOut.cch = (UINT)item.text.length();
}

UINT XPopupMenu::_menuItemCount()
{
    // get item count
    int itemCount = ::GetMenuItemCount(m_hMenu);
    if(itemCount < 0)
    {
        XWTRACE_WERR_LAST("XPopupMenu: failed to get menu item count");
        return 0;
    }

    return (UINT)itemCount;
}

// XPopupMenu
/////////////////////////////////////////////////////////////////////
