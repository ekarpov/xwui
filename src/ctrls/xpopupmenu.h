// Popup menu
//
/////////////////////////////////////////////////////////////////////

#ifndef _XPOPUPMENU_H_
#define _XPOPUPMENU_H_

/////////////////////////////////////////////////////////////////////
// XPopupMenu - popup menu

class XPopupMenu : public XWObject
{
public: // construction/destruction
    XPopupMenu(XWObject* parent = 0);
    ~XPopupMenu();

public: // interface
    bool    track(HWND hwnd, int posX, int posY, UINT& selectedItem);

public: // types
    struct MenuItem
    {
        UINT            id;
        std::wstring    text;
        HBITMAP         bitmap;
        bool            separator;

        MenuItem() : id(0), bitmap(0), separator(false) {}
    };

public: // items
    void    insertItem(UINT pos, const MenuItem& item);
    void    insertTextItem(UINT pos, const wchar_t* text, UINT id);
    void    insertItems(UINT pos, const std::vector<MenuItem>& items);
    void    insertSubMenu(UINT pos, const wchar_t* text, const std::vector<MenuItem>& items);
    void    appendItem(const MenuItem& item);
    void    appendTextItem(const wchar_t* text, UINT id);
    void    appendItems(const std::vector<MenuItem>& items);
    void    appendSubMenu(const wchar_t* text, const std::vector<MenuItem>& items);

public: // manage items
    void    enableMenuItem(UINT id, bool enable);

public: // menu handle
    HMENU   handle() const { return m_hMenu; }

private: // worker methods
    void    _convertItem(const MenuItem& item, MENUITEMINFO& itemOut);
    UINT    _menuItemCount();

private: // data
    HMENU   m_hMenu;
};

// XPopupMenu
/////////////////////////////////////////////////////////////////////

#endif // _XPOPUPMENU_H_

