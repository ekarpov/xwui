// Combobox
//
/////////////////////////////////////////////////////////////////////

#ifndef _XCOMBOBOX_H_
#define _XCOMBOBOX_H_

/////////////////////////////////////////////////////////////////////
// XComboBox - push button window

class XComboBox : public XWHWND
{
public: // construction/destruction
    XComboBox(HWND hWndParent, XWObject* parent = 0, bool bEditable = true, DWORD dwStyle = 0, DWORD dwExStyle = 0);
    ~XComboBox();

public: // current item
    bool    hasSelectedItem() const;
    DWORD   selectedItem() const;
    void    selectItem(DWORD dwItemIndex);
    void    resetSelection();

public: // items
    DWORD   addItem(const wchar_t* text, LPARAM lItemData = 0);
    DWORD   insertItem(DWORD dwItemIndex, const wchar_t* text, LPARAM lItemData = 0);
    DWORD   replaceItem(DWORD dwItemIndex, const wchar_t* text, LPARAM lItemData = 0);
    void    removeItem(DWORD dwItemIndex);
    void    resetContent();

public: // items data
    DWORD   itemCount() const;
    DWORD   itemText(DWORD dwItemIndex, wchar_t* text, DWORD maxSize) const;
    DWORD   itemTextLength(DWORD dwItemIndex) const;
    LRESULT itemData(DWORD dwItemIndex) const;
    bool    setItemData(DWORD dwItemIndex, LPARAM lItemData);

public: // selection helpers
    LRESULT selectedItemData() const;
    void    selectItemFromData(LPARAM lItemData);

public: // drop down list size in items
    void    setDropDownListSize(int itemCount);

public: // manage drop down list
    void    showDropDownList(bool show);

public: // events
    XWEventMask selectionChanged() const { return mkCommandEvent(CBN_SELCHANGE); }
    XWEventMask textChanged() const { return mkCommandEvent(CBN_EDITCHANGE); }
};

// XComboBox
/////////////////////////////////////////////////////////////////////

#endif // _XCOMBOBOX_H_


