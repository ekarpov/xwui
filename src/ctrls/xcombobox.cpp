// Combobox
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "../layout/xlayoutitem.h"

#include "../xwindow/xhwnd.h"

#include "xcombobox.h"

/////////////////////////////////////////////////////////////////////
// XComboBox - push button window

XComboBox::XComboBox(HWND hWndParent, XWObject* parent, bool bEditable, DWORD dwStyle, DWORD dwExStyle) :
    XWHWND(0, parent)
{
    XWASSERT(hWndParent);

    // create button
    m_hWnd = CreateWindowExW( 
        dwExStyle,
        WC_COMBOBOXW,   
        NULL,       
        WS_CHILD | WS_VSCROLL | CBS_AUTOHSCROLL | (bEditable ? CBS_DROPDOWN : CBS_DROPDOWNLIST) | dwStyle,
        0,0,            // starting position 
        100,10,         // default size
        hWndParent,     // parent window 
        NULL,           // No menu 
        ::GetModuleHandle(NULL), 
        NULL);      

    // check result
    if(m_hWnd == NULL)
    {
        // fatal error
        XWTRACE_WERR_LAST("XComboBox: Failed to create window");
        return;
    }

    // NOTE: for default sizes refer to MSDN:
    //       http://msdn.microsoft.com/en-us/library/ms997619.aspx

    // get default dimensions
    LONG units = ::GetDialogBaseUnits();
    int width = MulDiv(LOWORD(units), 100, 4); // NOTE: just twice as command button size
    int height = MulDiv(HIWORD(units), 10, 8);

    // fix default size
    XWHWND::setFixedSize(width, height);

    // set default size for drop down list
    setDropDownListSize(3);

    // set default text
    setDefaultFont();
}

XComboBox::~XComboBox()
{
}

/////////////////////////////////////////////////////////////////////
// current item
/////////////////////////////////////////////////////////////////////
bool XComboBox::hasSelectedItem() const
{
    return (selectedItem() != CB_ERR);
}

DWORD XComboBox::selectedItem() const
{
    // return selected item or CB_ERR if no item is selected
    return (DWORD)::SendMessageW(hwnd(), CB_GETCURSEL, 0, 0);
}

void XComboBox::selectItem(DWORD dwItemIndex)
{
    // get item data
    LRESULT ret = ::SendMessageW(hwnd(), CB_SETCURSEL, dwItemIndex, 0);

    // check for errors
    if(ret == CB_ERR)
    {
        XWTRACE_WERR_LAST("XComboBox: Failed to select item");
    }
}

void XComboBox::resetSelection()
{
    // reset selection
    ::SendMessageW(hwnd(), CB_SETCURSEL, CB_ERR, 0);
}

/////////////////////////////////////////////////////////////////////
// items
/////////////////////////////////////////////////////////////////////
DWORD XComboBox::addItem(const wchar_t* text, LPARAM lItemData)
{
    // add string to combobox
    DWORD index = (DWORD)::SendMessageW(hwnd(), CB_ADDSTRING, 0, (LPARAM)text);

    // check for errors
    if(index == CB_ERR || index == CB_ERRSPACE)
    {
        // fatal error
        XWTRACE_WERR_LAST("XComboBox: Failed to add item");
        return index;
    }

    // set item data if needed
    if(lItemData != 0)
    {
        setItemData(index, lItemData);
    }

    return index;
}

DWORD XComboBox::insertItem(DWORD dwItemIndex, const wchar_t* text, LPARAM lItemData)
{
    // add string to combobox
    DWORD index = (DWORD)::SendMessageW(hwnd(), CB_INSERTSTRING, dwItemIndex, (LPARAM)text);

    // check for errors
    if(index == CB_ERR || index == CB_ERRSPACE)
    {
        // fatal error
        XWTRACE_WERR_LAST("XComboBox: Failed to insert item");
        return index;
    }

    // set item data if needed
    if(lItemData != 0)
    {
        setItemData(index, lItemData);
    }

    return index;
}

DWORD XComboBox::replaceItem(DWORD dwItemIndex, const wchar_t* text, LPARAM lItemData)
{
    // remove item first
    removeItem(dwItemIndex);

    // insert new item
    return insertItem(dwItemIndex, text, lItemData);
}

void XComboBox::removeItem(DWORD dwItemIndex)
{
    // get item data
    LRESULT ret = ::SendMessageW(hwnd(), CB_DELETESTRING, dwItemIndex, 0);

    // check for errors
    if(ret == CB_ERR)
    {
        XWTRACE_WERR_LAST("XComboBox: Failed to delete item");
    }
}

void XComboBox::resetContent()
{
    // reset content
    LRESULT ret = ::SendMessageW(hwnd(), CB_RESETCONTENT, 0, 0);

    // check for errors
    if(ret == CB_ERR)
    {
        XWTRACE_WERR_LAST("XComboBox: Failed to reset content");
    }
}

/////////////////////////////////////////////////////////////////////
// items data
/////////////////////////////////////////////////////////////////////
DWORD XComboBox::itemCount() const
{
    // get item data
    LRESULT ret = ::SendMessageW(hwnd(), CB_GETCOUNT, 0, 0);

    // check for errors
    if(ret == CB_ERR)
    {
        XWTRACE_WERR_LAST("XComboBox: Failed to get item count");
    }

    return (DWORD)ret;
}

DWORD XComboBox::itemText(DWORD dwItemIndex, wchar_t* text, DWORD maxSize) const
{
    // check if text size is enough
    if(maxSize < itemTextLength(dwItemIndex))
    {
        XWTRACE("XComboBox: Can't get item text because buffer is too small");
        return CB_ERR;
    }

    // get item data
    LRESULT ret = ::SendMessageW(hwnd(), CB_GETLBTEXT, dwItemIndex, (LPARAM)text);

    // check for errors
    if(ret == CB_ERR)
    {
        XWTRACE_WERR_LAST("XComboBox: Failed to get item text");
    }

    return (DWORD)ret;
}

DWORD XComboBox::itemTextLength(DWORD dwItemIndex) const
{
    // get item data
    LRESULT ret = ::SendMessageW(hwnd(), CB_GETLBTEXTLEN, dwItemIndex, 0);

    // check for errors
    if(ret == CB_ERR)
    {
        XWTRACE_WERR_LAST("XComboBox: Failed to get item text length");
    }

    return (DWORD)ret;
}

LRESULT XComboBox::itemData(DWORD dwItemIndex) const
{
    // get item data
    LRESULT ret = ::SendMessageW(hwnd(), CB_GETITEMDATA, dwItemIndex, 0);

    // check for errors
    if(ret == CB_ERR)
    {
        XWTRACE_WERR_LAST("XComboBox: Failed to get item data");
    }

    return ret;
}

bool XComboBox::setItemData(DWORD dwItemIndex, LPARAM lItemData)
{
    // set data
    if(::SendMessageW(hwnd(), CB_SETITEMDATA, dwItemIndex, lItemData) == CB_ERR)
    {
        XWTRACE_WERR_LAST("XComboBox: Failed to set item data");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////
// selection helpers
/////////////////////////////////////////////////////////////////////
LRESULT XComboBox::selectedItemData() const
{
    // get selected item 
    DWORD idx = selectedItem();
    if(idx == CB_ERR) return CB_ERR;

    // get selected item data
    return itemData(idx);
}

void XComboBox::selectItemFromData(LPARAM lItemData)
{
    // loop over all items
    DWORD count = itemCount();
    for(DWORD idx = 0; idx < count; ++idx)
    {
        // check item data
        if(itemData(idx) == lItemData)
        {
            // select
            selectItem(idx);

            // stop
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////
// drop down list size in items
/////////////////////////////////////////////////////////////////////
void XComboBox::setDropDownListSize(int itemCount)
{
    // NOTE: we need to set height directly as not to affect 
    //       layout sizes

    // NOTE: for default sizes refer to MSDN:
    //       http://msdn.microsoft.com/en-us/library/ms997619.aspx

    // get default dimensions
    LONG units = ::GetDialogBaseUnits();

    // reserve enough space
    int height = MulDiv(HIWORD(units), 10, 8) * (itemCount + 1);

    // get current size
    RECT rec;
    if(::GetWindowRect(hwnd(), &rec))
    {
        // current width
        int width = rec.right - rec.left;

        // update height directly to combobox
        if(::MoveWindow(hwnd(), rec.left, rec.top, width, height, FALSE)) return;
    }

    XWTRACE_WERR_LAST("XComboBox: Failed to set drop down list size");
}

/////////////////////////////////////////////////////////////////////
// manage drop down list
/////////////////////////////////////////////////////////////////////
void XComboBox::showDropDownList(bool show)
{
    // check version
    if(XWUtils::sIsWindowsVistaOrAbove())
    {
        // NOTE: CB_SHOWDROPDOWN message is available starting form Windows Vista
        ::SendMessageW(hwnd(), CB_SHOWDROPDOWN, show ? TRUE : FALSE, 0);
    }
}

// XComboBox
/////////////////////////////////////////////////////////////////////

