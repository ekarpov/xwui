// Grid window
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWGRIDWINDOW_H_
#define _XWGRIDWINDOW_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
class XLineEdit;
class XComboBox;

/////////////////////////////////////////////////////////////////////
// grid notification messages
#define XWGRID_NOTIFY_CELL_SELECTED             1
#define XWGRID_NOTIFY_CELL_MODIFIED             2
#define XWGRID_NOTIFY_CELL_CLICKED              3
#define XWGRID_NOTIFY_CELL_DBL_CLICKED          4

/////////////////////////////////////////////////////////////////////
// constants
#define XWGRID_VALUE_NOT_SET                    -1
#define XWGRID_DEFAULT_MIN_SIZE                 10
#define XWGRID_DEFAULT_MAX_SIZE                 1024

/////////////////////////////////////////////////////////////////////
// XWGridWindow - grid window

class XWGridWindow : public XWindow
{
public: // construction/destruction
    XWGridWindow(DWORD dwStyle, XWObject* parent = 0, HWND hWndParent = 0, DWORD dwExStyle = 0);
    ~XWGridWindow();

public: // interface
    void    init(int columnCount);
    void    reset();

public: // manage rows
    void    appendRow();
    void    insertRow(int rowAt);
    void    removeRow(int rowAt);

public: // properties 
    int     columnCount();
    int     rowCount();

public: // editing
    void    cancelEditing();
    void    completeEditing();

public: // set data
    void    setCellText(int row, int column, const wchar_t* text);
    void    setCellTextHint(int row, int column, const wchar_t* hint);
    void    setCellList(int row, int column, const wchar_t** listItems, int itemCount);
    void    setCellListIndex(int row, int column, int index);
    void    setCellBitmap(int row, int column, HBITMAP bitmap);
    void    setCellBitmapHovered(int row, int column, HBITMAP bitmap);
    void    setCellBitmapClicked(int row, int column, HBITMAP bitmap);
    void    setCellEditable(int row, int column, bool editable);
    void    setCellModified(int row, int column, bool modified);
    void    setCellClickable(int row, int column, bool clickable);
    void    setCellSelectable(int row, int column, bool selectable);
    void    setCellSelected(int row, int column, bool selected);
    void    setCellUserData(int row, int column, LRESULT data);

public: // get data 
    const wchar_t*  cellText(int row, int column);
    int             cellTextLength(int row, int column);
    int             cellListIndex(int row, int column);
    LRESULT         cellUserData(int row, int column);

public: // cell type
    enum TCellType
    {
        eCellTypeText,
        eCellTypeList,
        eCellTypeBitmap
    };

    TCellType   cellType(int row, int column);

public: // style
    struct GridStyle
    {
        XTextStyle      textStyle;
        COLORREF        textColor;
        COLORREF        borderColor;
        COLORREF        fillColor;
        COLORREF        selectedColor;
        COLORREF        disabledColor;
        int             spacing;
        int             lineWidth;
    };

    void    setStyle(const GridStyle& style);
    void    getStyle(GridStyle& style);

public: // layout
    void    setColumnMinMaxWidth(int column, int minWidth, int maxWidth);
    void    setColumnFixedWidth(int column, int width);
    void    resetColumnFixedWidth(int column);
    void    setColumnStretch(int column, int stretch);
    void    setColumnFitContent(int column, bool fitContent);
    void    setRowMinMaxHeight(int minHeight, int maxHeight);
    void    setRowFixedHeight(int height);
    void    reseRowFixedHeight();

public: // events
    XWEventMask cellSelected() const    { return mkCommandEvent(XWGRID_NOTIFY_CELL_SELECTED); }
    XWEventMask cellModified() const    { return mkCommandEvent(XWGRID_NOTIFY_CELL_MODIFIED); }
    XWEventMask cellClicked() const     { return mkCommandEvent(XWGRID_NOTIFY_CELL_CLICKED); }
    XWEventMask cellDblClicked() const  { return mkCommandEvent(XWGRID_NOTIFY_CELL_DBL_CLICKED); }

public: // event context cell
    bool    getContextCell(int& row, int& column);

public: // scrolling (from IXWScrollable)
    bool    canScrollContent();
    int     contentWidth();
    int     contentHeight();
    void    setScrollOffsetX(int scrollOffsetX);
    void    setScrollOffsetY(int scrollOffsetY);

protected: // events (from XWindow)
    void    onPaint(HDC hdc, PAINTSTRUCT& ps);
    void    onResize(int type, int width, int height);
    void    onContentChanged();

protected: // mouse events (from XWindow)
    void    onMouseHover(int posX, int posY);
    bool    onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags);

protected: // keyboard events (from XWindow)
    bool    onCharEvent(WPARAM charCode, LPARAM flags);

private: // process messages (from XWindow)
    LRESULT processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private: // internal types
    struct GridCellData
    {
        TCellType       type;
        bool            editable;
        bool            modified;
        bool            clickable;
        bool            selectable;
        bool            hovered;
        bool            clicked;
        bool            selected;
        std::wstring    text;
        std::wstring    hint;
        HBITMAP         bitmap;
        HBITMAP         bitmapHovered;
        HBITMAP         bitmapCliked;
        int             listIndex;
        const wchar_t** listItems;
        int             listItemCount;
        LRESULT         userData; 


        // default values
        GridCellData() : type(eCellTypeText), editable(false), modified(false), clickable(false), selectable(false),
                         hovered(false), clicked(false), selected(false), bitmap(0), bitmapHovered(0), bitmapCliked(0),
                         listIndex(-1), listItems(0), listItemCount(0), userData(0) {}
    };

    struct GridRow
    {
        int                             height;
        std::vector<GridCellData>       cells;

        // default values
        GridRow() : height(0) {}
        GridRow(int columnCount) : height(0), cells(columnCount) { }
    };

    struct GridColumn
    {
        int             width;
        int             stretch;
        int             minWidth;
        int             maxWidth;
        int             maxContentWidth;
        bool            fixedWidth;
        bool            fitContent;

        // default values
        GridColumn() : width(0), stretch(0), minWidth(XWGRID_DEFAULT_MIN_SIZE), maxWidth(XWGRID_DEFAULT_MAX_SIZE),
                       maxContentWidth(XWGRID_VALUE_NOT_SET), fixedWidth(false), fitContent(false) {}
    };

    struct GridEditor
    {
        int             row;
        int             column;
        bool            editing;
        TCellType       editorType;

        // editors
        XLineEdit*      textEditor;
        XComboBox*      listEditor;

        // default values
        GridEditor() : row(XWGRID_VALUE_NOT_SET), column(XWGRID_VALUE_NOT_SET), editing(false),
                       textEditor(0), listEditor(0) {}
    };

    typedef std::vector<GridRow>        _gridRowsT;
    typedef std::vector<GridColumn>     _gridColumnsT;

private: // worker methods
    bool        _validateColumn(int column);
    bool        _validateIndex(int row, int column);
    bool        _isValidIndex(int row, int column);
    void        _initStyle();
    void        _updateColumnLayoutState(int& stretchCount, int& fixedCount);
    void        _fitColumns();
    void        _updateLayout();
    void        _updateLayoutIfNeeded();
    void        _updateContentSize();
    bool        _findIndex(int posX, int posY, int& row, int& column);
    void        _getCellPos(int row, int column, int& posX, int& posY);
    void        _findSelection(int& row, int& column);
    void        _updateSelection(int row, int column);
    void        _createTextEditor();
    void        _createListEditor();
    void        _completeEditing();
    void        _cancelEditing();
    void        _updateEditing(int row, int column);
    XWHWND*     _activeEditor();
    void        _moveEditor(int dX, int dY);
    void        _updateEditor();
    bool        _handleKeyPressed(WPARAM wParam, LPARAM lParam);

private: // paint methods
    void        _paintCell(HDC hdc, int posX, int posY, int width, int height, const GridCellData& cell);

private: // subclassing
    static LRESULT CALLBACK _subclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

private: // grid data
    _gridRowsT              m_gridData;
    _gridColumnsT           m_gridColumns;

private: // data
    GridStyle               m_gridStyle;
    GridEditor              m_gridEditor;
    HFONT                   m_textFont;
    HFONT                   m_modifiedFont;
    std::wstring            m_strBuffer;
    int                     m_fontHeight;
    int                     m_columnCount;
    int                     m_rowMinHeight;
    int                     m_rowMaxHeight;
    int                     m_rowFixedHeight;
    bool                    m_fixedRowHeight;
    bool                    m_layoutUpdateNeeded;
    int                     m_contentWidth;
    int                     m_contentHeight;
    int                     m_contextRow;
    int                     m_contextColumn;
};

// XWGridWindow
/////////////////////////////////////////////////////////////////////

#endif // _XWGRIDWINDOW_H_

