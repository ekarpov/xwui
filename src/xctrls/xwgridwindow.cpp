// Grid window
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

// dependencies
#include "../layout/xwlayouts.h"
#include "../xwindow/xwindow.h"
#include "../ctrls/xwcontrols.h"
#include "../graphics/xwgraphics.h"

#include "xwgridwindow.h"

/////////////////////////////////////////////////////////////////////
// constants 
#define XWGRID_SUBCLASS_ID_EDITBOX                  1
#define XWGRID_SUBCLASS_ID_COMBOBOX                 2
#define XWGRID_SUBCLASS_ID_COMBOBOX_PARENT          3

/////////////////////////////////////////////////////////////////////
// XWGridWindow - grid window

/////////////////////////////////////////////////////////////////////
// construction/destruction
/////////////////////////////////////////////////////////////////////
XWGridWindow::XWGridWindow(DWORD dwStyle, XWObject* parent, HWND hWndParent, DWORD dwExStyle) :
    XWindow(dwStyle, parent, hWndParent, dwExStyle),
    m_textFont(0),
    m_modifiedFont(0),
    m_fontHeight(0),
    m_columnCount(0),
    m_rowMinHeight(XWGRID_DEFAULT_MIN_SIZE),
    m_rowMaxHeight(XWGRID_DEFAULT_MAX_SIZE),
    m_rowFixedHeight(XWGRID_VALUE_NOT_SET),
    m_fixedRowHeight(false),
    m_layoutUpdateNeeded(false),
    m_contentWidth(0),
    m_contentHeight(0),
    m_contextRow(XWGRID_VALUE_NOT_SET),
    m_contextColumn(XWGRID_VALUE_NOT_SET)
{
    // reset state
    reset();

    // default text style
    m_gridStyle.textStyle = XWUIStyle::textStyle();

    // default colors
    m_gridStyle.textColor = XWUIStyle::getColorText();
    m_gridStyle.borderColor = XWUIStyle::getColorBorder();
    m_gridStyle.fillColor = XWUIStyle::getColorBackground();
    m_gridStyle.selectedColor = XWUIStyle::getColorSelection();
    m_gridStyle.disabledColor = XWUIStyle::getColorDisabledText();

    // default sizes
    m_gridStyle.spacing = XGdiHelpers::dpiScalePixelsY(3);
    m_gridStyle.lineWidth = XGdiHelpers::dpiScalePixelsY(1);

    // init defaut style
    _initStyle();
}

XWGridWindow::~XWGridWindow()
{
    // release content
    reset();
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XWGridWindow::init(int columnCount)
{
    // check input
    XWASSERT(columnCount > 0);
    if(columnCount <= 0) return;

    // reset previous content
    reset();

    // copy column number
    m_columnCount = columnCount;

    // init columns
    m_gridColumns.resize(m_columnCount);
}

void XWGridWindow::reset()
{
    // cancel editing if any
    _cancelEditing();

    // reset data
    m_gridData.clear();
    m_gridColumns.clear();
    m_columnCount = 0;
    m_contextRow = XWGRID_VALUE_NOT_SET;
    m_contextColumn = XWGRID_VALUE_NOT_SET;

    // request layout update
    m_layoutUpdateNeeded = true;
}

/////////////////////////////////////////////////////////////////////
// manage rows
/////////////////////////////////////////////////////////////////////
void XWGridWindow::appendRow()
{
    // append row with default values
    m_gridData.push_back(GridRow(m_columnCount));

    // mark flag
    m_layoutUpdateNeeded = true;
}

void XWGridWindow::insertRow(int rowAt)
{
    // check if row exists
    if(rowAt >= 0 && rowAt < (int)m_gridData.size())
    {
        // insert
        _gridRowsT::iterator it = m_gridData.insert(m_gridData.begin() + rowAt, GridRow(m_columnCount));

    } else
    {
        XWASSERT1(0, "XWGridWindow: failed to insert row, index not found");
    }

    // mark flag
    m_layoutUpdateNeeded = true;
}

void XWGridWindow::removeRow(int rowAt)
{
    // check if row exists
    if(rowAt >= 0 && rowAt < (int)m_gridData.size())
    {
        // remove
        m_gridData.erase(m_gridData.begin() + rowAt);

    } else
    {
        XWASSERT1(0, "XWGridWindow: failed to remove row, index not found");
    }

    // mark flag
    m_layoutUpdateNeeded = true;
}

/////////////////////////////////////////////////////////////////////
// properties 
/////////////////////////////////////////////////////////////////////
int XWGridWindow::columnCount()
{
    return m_columnCount;
}

int XWGridWindow::rowCount()
{
    return (int)m_gridData.size();
}

/////////////////////////////////////////////////////////////////////
// editors
/////////////////////////////////////////////////////////////////////
void XWGridWindow::cancelEditing()
{
    _cancelEditing();
}

void XWGridWindow::completeEditing()
{
    _completeEditing();
}

/////////////////////////////////////////////////////////////////////
// set data
/////////////////////////////////////////////////////////////////////
void XWGridWindow::setCellText(int row, int column, const wchar_t* text)
{
    // validate index
    if(!_validateIndex(row, column)) return;

    // update cell type
    m_gridData.at(row).cells.at(column).type = eCellTypeText;

    // set text
    if(text)
        m_gridData.at(row).cells.at(column).text = text;
    else
        m_gridData.at(row).cells.at(column).text.clear();

    // mark flag
    m_layoutUpdateNeeded = true;
}

void XWGridWindow::setCellTextHint(int row, int column, const wchar_t* hint)
{
    // validate index
    if(!_validateIndex(row, column)) return;

    // set hint text
    if(hint)
        m_gridData.at(row).cells.at(column).hint = hint;
    else
        m_gridData.at(row).cells.at(column).hint.clear();
}

void XWGridWindow::setCellList(int row, int column, const wchar_t** listItems, int itemCount)
{
    // validate input
    XWASSERT(listItems);
    XWASSERT(itemCount > 0);
    if(listItems == 0 || itemCount <= 0) return;

    // validate index
    if(!_validateIndex(row, column)) return;

    // validate list items
    for(int idx = 0; idx < itemCount; ++idx)
    {
        XWASSERT1(listItems[idx], "XWGridWindow: cell list value is not valid");
        if(listItems[idx] == 0) return;
    }

    // update cell type
    m_gridData.at(row).cells.at(column).type = eCellTypeList;

    // set list data
    m_gridData.at(row).cells.at(column).listIndex = XWGRID_VALUE_NOT_SET;
    m_gridData.at(row).cells.at(column).listItems = listItems;
    m_gridData.at(row).cells.at(column).listItemCount = itemCount;

    // list items are editable by default
    m_gridData.at(row).cells.at(column).editable = true;

    // mark flag
    m_layoutUpdateNeeded = true;
}

void XWGridWindow::setCellListIndex(int row, int column, int index)
{
    // validate input
    XWASSERT(index >= XWGRID_VALUE_NOT_SET);
    if(index < XWGRID_VALUE_NOT_SET) return;

    // validate index
    if(!_validateIndex(row, column)) return;

    // validate list index
    if(index != XWGRID_VALUE_NOT_SET)
    {
        if(index < 0 || index >= m_gridData.at(row).cells.at(column).listItemCount)
        {
            XWASSERT1(0, "XWGridWindow: list index is not valid");
            index = XWGRID_VALUE_NOT_SET;
        }
    }

    // set list index
    m_gridData.at(row).cells.at(column).listIndex = index;

    // set text value from index
    if(index != XWGRID_VALUE_NOT_SET)
    {
        m_gridData.at(row).cells.at(column).text = 
            m_gridData.at(row).cells.at(column).listItems[index];
    } else
    {
        m_gridData.at(row).cells.at(column).text.clear();
    }

    // mark flag
    m_layoutUpdateNeeded = true;
}

void XWGridWindow::setCellBitmap(int row, int column, HBITMAP bitmap)
{
    // validate index
    if(!_validateIndex(row, column)) return;

    // NOTE: if bitmap is not set change cell type to text

    // update cell type
    if(bitmap)
        m_gridData.at(row).cells.at(column).type = eCellTypeBitmap;
    else
        m_gridData.at(row).cells.at(column).type = eCellTypeText; 

    // set bitmap
    m_gridData.at(row).cells.at(column).bitmap = bitmap;

    // mark flag
    m_layoutUpdateNeeded = true;
}

void XWGridWindow::setCellBitmapHovered(int row, int column, HBITMAP bitmap)
{
    // validate index
    if(!_validateIndex(row, column)) return;

    // set bitmap
    m_gridData.at(row).cells.at(column).bitmapHovered = bitmap;
}

void XWGridWindow::setCellBitmapClicked(int row, int column, HBITMAP bitmap)
{
    // validate index
    if(!_validateIndex(row, column)) return;

    // set bitmap
    m_gridData.at(row).cells.at(column).bitmapCliked = bitmap;
}

void XWGridWindow::setCellEditable(int row, int column, bool editable)
{
    // validate index
    if(!_validateIndex(row, column)) return;

    // set flag
    m_gridData.at(row).cells.at(column).editable = editable;
}

void XWGridWindow::setCellModified(int row, int column, bool modified)
{
    // validate index
    if(!_validateIndex(row, column)) return;

    // set flag
    m_gridData.at(row).cells.at(column).modified = modified;
}

void XWGridWindow::setCellClickable(int row, int column, bool clickable)
{
    // validate index
    if(!_validateIndex(row, column)) return;

    // set flag
    m_gridData.at(row).cells.at(column).clickable = clickable;
}

void XWGridWindow::setCellSelectable(int row, int column, bool selectable)
{
    // validate index
    if(!_validateIndex(row, column)) return;

    // set flag
    m_gridData.at(row).cells.at(column).selectable = selectable;
}

void XWGridWindow::setCellSelected(int row, int column, bool selected)
{
    // validate index
    if(!_validateIndex(row, column)) return;

    // set flag
    m_gridData.at(row).cells.at(column).selected = selected;
}

void XWGridWindow::setCellUserData(int row, int column, LRESULT data)
{
    // validate index
    if(!_validateIndex(row, column)) return;

    // set user data
    m_gridData.at(row).cells.at(column).userData = data;
}

/////////////////////////////////////////////////////////////////////
// get data 
/////////////////////////////////////////////////////////////////////
const wchar_t* XWGridWindow::cellText(int row, int column)
{
    // validate index
    if(!_validateIndex(row, column)) return 0;

    // get text
    return m_gridData.at(row).cells.at(column).text.c_str();
}

int XWGridWindow::cellTextLength(int row, int column)
{
    // validate index
    if(!_validateIndex(row, column)) return 0;

    // get text length
    return (int)m_gridData.at(row).cells.at(column).text.length();
}

int XWGridWindow::cellListIndex(int row, int column)
{
    // validate index
    if(!_validateIndex(row, column)) return XWGRID_VALUE_NOT_SET;

    // get index
    return m_gridData.at(row).cells.at(column).listIndex;
}

LRESULT XWGridWindow::cellUserData(int row, int column)
{
    // validate index
    if(!_validateIndex(row, column)) return 0;

    // get data
    return m_gridData.at(row).cells.at(column).userData;
}

/////////////////////////////////////////////////////////////////////
// cell type
/////////////////////////////////////////////////////////////////////
XWGridWindow::TCellType XWGridWindow::cellType(int row, int column)
{
    // validate index
    if(!_validateIndex(row, column)) return eCellTypeText;

    // get type
    return m_gridData.at(row).cells.at(column).type;
}

/////////////////////////////////////////////////////////////////////
// style
/////////////////////////////////////////////////////////////////////
void XWGridWindow::setStyle(const GridStyle& style)
{
    // copy style
    m_gridStyle = style;

    // init style properties
    _initStyle();
}

void XWGridWindow::getStyle(GridStyle& style)
{
    // copy style
    style = m_gridStyle;
}

/////////////////////////////////////////////////////////////////////
// cell layout
/////////////////////////////////////////////////////////////////////
void XWGridWindow::setColumnMinMaxWidth(int column, int minWidth, int maxWidth)
{
    // validate column
    if(!_validateColumn(column)) return;

    // treat negative values as not set
    if(minWidth < 0) minWidth = XWGRID_VALUE_NOT_SET;
    if(maxWidth < 0) maxWidth = XWGRID_VALUE_NOT_SET;

    // update column constraints
    m_gridColumns.at(column).minWidth = minWidth;
    m_gridColumns.at(column).maxWidth = maxWidth;

    // require full layout update
    m_layoutUpdateNeeded = true;
}

void XWGridWindow::setColumnFixedWidth(int column, int width)
{
    // validate column
    if(!_validateColumn(column)) return;

    // treat negative values as not set
    if(width < 0) width = XWGRID_VALUE_NOT_SET;

    // update flag
    m_gridColumns.at(column).fixedWidth = (width != XWGRID_VALUE_NOT_SET);

    // set width
    if(m_gridColumns.at(column).fixedWidth)
        m_gridColumns.at(column).width = width;

    // require full layout update
    m_layoutUpdateNeeded = true;
}

void XWGridWindow::resetColumnFixedWidth(int column)
{
    // validate column
    if(!_validateColumn(column)) return;

    // reset flag
    m_gridColumns.at(column).fixedWidth = false;

    // require full layout update
    m_layoutUpdateNeeded = true;
}

void XWGridWindow::setColumnStretch(int column, int stretch)
{
    // validate column
    if(!_validateColumn(column)) return;

    // treat negative values as not set
    if(stretch < 0) stretch = 0;

    // update stretch
    m_gridColumns.at(column).stretch = stretch;

    // require full layout update
    m_layoutUpdateNeeded = true;
}

void XWGridWindow::setColumnFitContent(int column, bool fitContent)
{
    // validate column
    if(!_validateColumn(column)) return;

    // update flag
    m_gridColumns.at(column).fitContent = fitContent;

    // require full layout update
    m_layoutUpdateNeeded = true;
}

void XWGridWindow::setRowMinMaxHeight(int minHeight, int maxHeight)
{
    // treat negative values as not set
    if(minHeight < 0) minHeight = XWGRID_VALUE_NOT_SET;
    if(maxHeight < 0) maxHeight = XWGRID_VALUE_NOT_SET;

    // update
    m_rowMinHeight = minHeight;
    m_rowMaxHeight = maxHeight;

    // require full layout update
    m_layoutUpdateNeeded = true;
}

void XWGridWindow::setRowFixedHeight(int height)
{
    // treat negative values as not set
    if(height < 0) height = XWGRID_VALUE_NOT_SET;

    // update
    m_rowFixedHeight = height;

    // update flag
    m_fixedRowHeight = (height != XWGRID_VALUE_NOT_SET);

    // require full layout update
    m_layoutUpdateNeeded = true;
}

void XWGridWindow::reseRowFixedHeight()
{
    // reset 
    m_rowFixedHeight = XWGRID_VALUE_NOT_SET;

    // reset flag
    m_fixedRowHeight = false;

    // require full layout update
    m_layoutUpdateNeeded = true;
}

/////////////////////////////////////////////////////////////////////
// event context cell
/////////////////////////////////////////////////////////////////////
bool XWGridWindow::getContextCell(int& row, int& column)
{
    // check if context cell is set
    if(m_contextRow >= 0 && m_contextRow < (int)m_gridData.size() &&
       m_contextColumn >= 0 && m_contextColumn < m_columnCount)
    {
        // copy
        row = m_contextRow;
        column = m_contextColumn;

        return true;
    }

    // not set
    return false;
}

/////////////////////////////////////////////////////////////////////
// scrolling (from IXWScrollable)
/////////////////////////////////////////////////////////////////////
bool XWGridWindow::canScrollContent()
{
    // grid can scroll its content
    return true;
}

int XWGridWindow::contentWidth()
{
    // check if layout update is needed
    _updateLayoutIfNeeded();

    return m_contentWidth;
}

int XWGridWindow::contentHeight()
{
    // check if layout update is needed
    _updateLayoutIfNeeded();

    return m_contentHeight;
}

void XWGridWindow::setScrollOffsetX(int scrollOffsetX)
{
    // move editor if active
    _moveEditor(XWindow::scrollOffsetX() - scrollOffsetX, 0);

    // pass to parent
    XWindow::setScrollOffsetX(scrollOffsetX);

    // repaint
    repaint();
}

void XWGridWindow::setScrollOffsetY(int scrollOffsetY)
{
    // move editor if active
    _moveEditor(0, XWindow::scrollOffsetY() - scrollOffsetY);

    // pass to parent
    XWindow::setScrollOffsetY(scrollOffsetY);

    // repaint
    repaint();
}

/////////////////////////////////////////////////////////////////////
// events (from XWindow)
/////////////////////////////////////////////////////////////////////
void XWGridWindow::onPaint(HDC hdc, PAINTSTRUCT& ps)
{
    // check if layout update is needed
    _updateLayoutIfNeeded();

    // select font
    ::SelectObject(hdc, m_textFont);

    // text color
    if(isEnabled())
        ::SetTextColor(hdc, m_gridStyle.textColor);
    else
        ::SetTextColor(hdc, m_gridStyle.disabledColor);        

    // fill color
    ::SetBkColor(hdc, m_gridStyle.fillColor);
    ::SetBkMode(hdc, OPAQUE);

    // select pen
    HPEN hPen = ::CreatePen(PS_SOLID, m_gridStyle.lineWidth, m_gridStyle.borderColor);
    HGDIOBJ oldPen = ::SelectObject(hdc, hPen);

    // loop over rows
    int posY = 0;
    for(_gridRowsT::const_iterator rit = m_gridData.begin(); rit != m_gridData.end(); ++rit)
    {
        int rowHeight = rit->height + 2 * m_gridStyle.spacing;

        // ignore not visible rows
        if(posY + rit->height >= scrollOffsetY()) 
        {
            // double check that data is correct
            XWASSERT(rit->cells.size() == m_gridColumns.size());
            if(rit->cells.size() != m_gridColumns.size()) continue;

            // loop over columns
            int posX = 0;
            for(size_t columnIdx = 0; columnIdx < rit->cells.size(); ++columnIdx)
            {
                // column width
                int columnWidth = m_gridColumns.at(columnIdx).width + 2 * m_gridStyle.spacing;

                // ignore not visible columns
                if(posX + columnWidth >= scrollOffsetX()) 
                {
                    // get cell
                    const GridCellData& cell = rit->cells.at(columnIdx);

                    // paint cell
                    _paintCell(hdc, posX - scrollOffsetX(), 
                                    posY - scrollOffsetY(), 
                                    columnWidth + 2 * m_gridStyle.lineWidth, 
                                    rowHeight + 2 * m_gridStyle.lineWidth, 
                                    cell);
                }

                // move position
                posX += columnWidth + m_gridStyle.lineWidth;
            }
        }

        // move position
        posY += rowHeight + m_gridStyle.lineWidth;
    }

    // fill the rest if needed
    if(posY + m_gridStyle.lineWidth < height())
    {
        RECT fillRect;
        fillRect.left = 0;
        fillRect.right = width();
        fillRect.top = posY + m_gridStyle.lineWidth;
        fillRect.bottom = height();

        ::ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &fillRect, L"", 0, 0);
    }

    // remove pen
    ::SelectObject(hdc, oldPen);
    ::DeleteObject(hPen);
}

void XWGridWindow::onResize(int type, int width, int height)
{
    // update layout
    _updateLayout();

    // update editor if active
    _updateEditor();
}

void XWGridWindow::onContentChanged()
{
    // mark flag
    m_layoutUpdateNeeded = true;
}

/////////////////////////////////////////////////////////////////////
// mouse events (from XWindow)
/////////////////////////////////////////////////////////////////////
void XWGridWindow::onMouseHover(int posX, int posY)
{
    int row = 0;
    int column = 0;

    // find cell
    if(_findIndex(posX, posY, row, column))
    {
        // TODO: show hint text (show value if hint not set)
    }
}

bool XWGridWindow::onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags)
{
    // check button event
    if(uButtonMsg == WM_LBUTTONDOWN || uButtonMsg == WM_LBUTTONDBLCLK)
    {
        // set focus
        setFocus();

        // find cell
        bool cellFound = _findIndex(posX, posY, m_contextRow, m_contextColumn);

        // update selection
        _updateSelection(m_contextRow, m_contextColumn);

        // start editing if needed
        _updateEditing(m_contextRow, m_contextColumn);

        // report event if item is clickable
        if(cellFound && m_gridData.at(m_contextRow).cells.at(m_contextColumn).clickable)
        {
            notifyParentWindow(XWGRID_NOTIFY_CELL_CLICKED);
        }

        // consume event
        return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////
// keyboard events (from XWindow)
/////////////////////////////////////////////////////////////////////
bool XWGridWindow::onCharEvent(WPARAM charCode, LPARAM flags)
{
    // ignore key
    return false;
}

/////////////////////////////////////////////////////////////////////
// process messages (from XWindow)
/////////////////////////////////////////////////////////////////////
LRESULT XWGridWindow::processMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // process known messages
    switch(uMsg)
    {
    case WM_KEYDOWN:
        if(_handleKeyPressed(wParam, lParam)) return 0;
        break;
    }

    // pass to parent
    return XWindow::processMessage(hwnd, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
bool XWGridWindow::_validateColumn(int column)
{
    // validate
    if(column >= 0 && column < m_columnCount) return true;

    // index is not valid
    XWASSERT1(0, "XWGridWindow: column index is not valid");
    return false;
}

bool XWGridWindow::_validateIndex(int row, int column)
{
    // validate column first
    if(!_validateColumn(column)) return false;

    // validate row
    if(row >= 0 && row < (int)m_gridData.size()) return true;

    // index is not valid
    XWASSERT1(0, "XWGridWindow: row index is not valid");
    return false;
}

bool XWGridWindow::_isValidIndex(int row, int column)
{
    return (column >= 0 && column < m_columnCount) &&
           (row >= 0 && row < (int)m_gridData.size());
}

void XWGridWindow::_initStyle()
{
    // properties
    setFocusable(true);

    // create text font
    m_textFont = XGdiFonts::getGDIFont(m_gridStyle.textStyle);

    // use default if not available
    if(m_textFont == 0)
        m_textFont = XWUtils::sGetDefaultFont();

    // use bold font for modified cells
    if(!m_gridStyle.textStyle.bBold)
    {
        // use bold version
        m_gridStyle.textStyle.bBold = true;
        m_modifiedFont = XGdiFonts::getGDIFont(m_gridStyle.textStyle);

        // set bold back
        m_gridStyle.textStyle.bBold = false;
    }

    // use text font if modified font is not available
    if(m_modifiedFont == 0)
        m_modifiedFont = m_textFont;

    // font height
    XGdiFonts::getGDIFontHeight(hwnd(), m_textFont, m_fontHeight);
}

void XWGridWindow::_updateColumnLayoutState(int& stretchCount, int& fixedCount)
{
    // reset output
    stretchCount = 0;
    fixedCount = 0;

    // loop over all columns
    for(_gridColumnsT::const_iterator cit = m_gridColumns.begin(); cit != m_gridColumns.end(); ++cit)
    {
        if(!cit->fixedWidth && !cit->fitContent)
        {
            stretchCount += cit->stretch;

        } else
        {
            fixedCount++;
        }
    }
}

void XWGridWindow::_fitColumns()
{
    // stop if there is nothing to modify
    if(m_columnCount == 0 || m_contentWidth == width()) return;

    int stretchCount = 0;
    int fixedCount = 0;
        
    // update layout properties
    _updateColumnLayoutState(stretchCount, fixedCount);

    // ignore if all columns have fixed size
    if(fixedCount == m_columnCount) return;

    // check if we need to squeeze or expand columns
    if(m_contentWidth > width())
    {
        // use minimum width as column width
        for(_gridColumnsT::iterator cit = m_gridColumns.begin(); cit != m_gridColumns.end(); ++cit)
        {
            // skip fixed size columns
            if(cit->fixedWidth || cit->fitContent) continue;

            // set width to minimum 
            cit->width = cit->minWidth;;
        }

        // update content size
        _updateContentSize();

        // check if we can squeeze
        if(m_contentWidth >= width()) return;
    }

    // size we need to split
    int fillSize = width() - m_contentWidth;

    double stretchFactor = stretchCount ? (double) fillSize / (double) stretchCount : 
                                          (double) fillSize / (double) (m_columnCount - fixedCount);

    // split content over columns
    for(_gridColumnsT::iterator cit = m_gridColumns.begin(); cit != m_gridColumns.end() && fillSize > 0; ++cit)
    {
        // skip fixed size columns
        if(cit->fixedWidth || cit->fitContent) continue;

        // extra size to add
        int extraWidth = (int) (stretchCount ? cit->stretch * stretchFactor : stretchFactor);

        // make sure we do not use extra space
        if(extraWidth > fillSize) extraWidth = fillSize;

        // add to column
        cit->width += extraWidth;

        // decrease extra space
        fillSize -= extraWidth;
    }

    // increase last column to use rest of the space if needed, this may happen
    // due to rounding errors (expected to be only few pixels at most)
    if(fillSize > 0)
    {
        m_gridColumns.back().width += fillSize;
    }

    // update content size
    m_contentWidth = width();
}

void XWGridWindow::_updateLayout()
{
    // get window dc
    HDC hdc = ::GetDC(hwnd());

    // select font
    ::SelectObject(hdc, m_textFont);

    // loop over all rows
    for(_gridRowsT::iterator rit = m_gridData.begin(); rit != m_gridData.end(); ++rit)
    {
        // double check that data is correct
        XWASSERT(rit->cells.size() == m_gridColumns.size());
        if(rit->cells.size() != m_gridColumns.size()) continue;

        // loop over columns
        for(size_t columnIdx = 0; columnIdx < rit->cells.size(); ++columnIdx)
        {
            // active cell
            GridCellData& cell = rit->cells.at(columnIdx);

            int cellWidth = 0;
            int cellHeight = 0;

            // check cell type
            if(cell.type == eCellTypeText || cell.type == eCellTypeList)
            {
                // check if cell is modified
                if(cell.modified)
                {
                    // select modified font
                    ::SelectObject(hdc, m_modifiedFont);
                }

                // text size
                if(cell.text.length())
                {
                    // compute size rect
                    SIZE size;
                    ::GetTextExtentPoint32W(hdc, cell.text.c_str(), (int)cell.text.length(), &size);

                    cellWidth = size.cx;
                    cellHeight = size.cy;
                }

                // check if cell is modified
                if(cell.modified)
                {
                    // select normal font back
                    ::SelectObject(hdc, m_textFont);
                }

            } else if(cell.type == eCellTypeBitmap)
            {
                XWASSERT(cell.bitmap);
                if(cell.bitmap)
                {
                    // NOTE: assume that bitmaps are of the same size
                    XGdiHelpers::getBitmapSize(cell.bitmap, cellWidth, cellHeight);
                }

            } else
            {
                XWASSERT1(0, "XWGridWindow: unknown cell type");
                continue;
            }

            // update row height 
            if(m_fixedRowHeight)
            {
                rit->height = m_rowFixedHeight;

            } else
            {
                // set row height
                if(rit->height < cellHeight)
                    rit->height = cellHeight;

                // respect minimum 
                if(m_rowMinHeight != XWGRID_VALUE_NOT_SET && m_rowMinHeight > rit->height)
                    rit->height = m_rowMinHeight;

                // respect maximum
                if(m_rowMaxHeight != XWGRID_VALUE_NOT_SET && m_rowMaxHeight < rit->height)
                    rit->height = m_rowMaxHeight;
            }

            // active column
            GridColumn& column = m_gridColumns.at(columnIdx);

            // update column width
            if(!column.fixedWidth)
            {
                // update content size
                if(column.maxContentWidth < cellWidth)
                    column.maxContentWidth = cellWidth;

                // set column width
                column.width = column.maxContentWidth;

                // respect minimum
                if(column.minWidth != XWGRID_VALUE_NOT_SET && column.minWidth > column.width)
                    column.width = column.minWidth;

                // respect maximum
                if(column.maxWidth != XWGRID_VALUE_NOT_SET && column.maxWidth < column.width)
                    column.width = column.maxWidth;
            }
        }
    }

    // release device context
    ::ReleaseDC(hwnd(), hdc); 

    // update content size
    _updateContentSize();

    // try to fit columns to size
    _fitColumns();

    // reset flag
    m_layoutUpdateNeeded = false;
}

void XWGridWindow::_updateLayoutIfNeeded()
{
    if(m_layoutUpdateNeeded)
        _updateLayout();
}

void XWGridWindow::_updateContentSize()
{
    // content width
    m_contentWidth = m_gridStyle.lineWidth;
    for(_gridColumnsT::const_iterator cit = m_gridColumns.begin(); cit != m_gridColumns.end(); ++cit)
    {
        m_contentWidth += cit->width + m_gridStyle.lineWidth + 2 * m_gridStyle.spacing;
    }

    // content height
    m_contentHeight = m_gridStyle.lineWidth;
    for(_gridRowsT::const_iterator rit = m_gridData.begin(); rit != m_gridData.end(); ++rit)
    {
        m_contentHeight += rit->height + m_gridStyle.lineWidth + 2 * m_gridStyle.spacing;
    }
}

bool XWGridWindow::_findIndex(int posX, int posY, int& row, int& column)
{
    // reset output
    row = XWGRID_VALUE_NOT_SET;
    column = XWGRID_VALUE_NOT_SET;

    // find column
    int findPosX = -1 * scrollOffsetX();
    for(size_t columnIdx = 0; columnIdx < m_gridColumns.size(); ++columnIdx)
    {
        const GridColumn& columnRef = m_gridColumns.at(columnIdx);

        // match
        if(posX > findPosX && posX < findPosX + columnRef.width + m_gridStyle.lineWidth + 2 * m_gridStyle.spacing)
        {
            // column found
            column = (int)columnIdx;
            break;
        }

        findPosX += columnRef.width + m_gridStyle.lineWidth + 2 * m_gridStyle.spacing;
    }

    // check if found
    if(column == XWGRID_VALUE_NOT_SET) return false;

    // find row
    int findPosY = -1 * scrollOffsetY();
    for(size_t rowIdx = 0; rowIdx < m_gridData.size(); ++rowIdx)
    {
        const GridRow& rowRef = m_gridData.at(rowIdx);

        // match
        if(posY > findPosY && posY < findPosY + rowRef.height + m_gridStyle.lineWidth + 2 * m_gridStyle.spacing)
        {
            // row found
            row = (int)rowIdx;
            return true;
        }

        findPosY += rowRef.height + m_gridStyle.lineWidth + 2 * m_gridStyle.spacing;
    }

    // reset column
    column = XWGRID_VALUE_NOT_SET;

    // cell not found
    return false;
}

void XWGridWindow::_getCellPos(int row, int column, int& posX, int& posY)
{
    // loop over rows
    posY = -1 * scrollOffsetY();
    for(size_t rowIdx = 0; rowIdx < m_gridData.size(); ++rowIdx)
    {
        const GridRow& rowRef = m_gridData.at(rowIdx);

        // double check that data is correct
        XWASSERT(rowRef.cells.size() == m_gridColumns.size());
        if(rowRef.cells.size() != m_gridColumns.size()) continue;

        // loop over columns
        posX =  -1 * scrollOffsetX();
        for(size_t columnIdx = 0; columnIdx < rowRef.cells.size(); ++columnIdx)
        {
            // column width
            int columnWidth = m_gridColumns.at(columnIdx).width;

            // check cell index
            if(rowIdx == row && columnIdx == column) return;

            // move position
            posX += columnWidth + m_gridStyle.lineWidth + 2 * m_gridStyle.spacing;
        }

        // move position
        posY += rowRef.height + m_gridStyle.lineWidth + 2 * m_gridStyle.spacing;
    }
}

void XWGridWindow::_findSelection(int& row, int& column)
{
    for(size_t rowIdx = 0; rowIdx != m_gridData.size(); ++rowIdx)
    {
        const GridRow& rowRef = m_gridData.at(rowIdx);

        for(size_t columnIdx = 0; columnIdx != rowRef.cells.size(); ++columnIdx)
        {
            if(rowRef.cells.at(columnIdx).selected)
            {
                row = (int)rowIdx;
                column = (int)columnIdx;

                // stop
                return;
            }
        }
    }

    // not found
    row = XWGRID_VALUE_NOT_SET;
    column = XWGRID_VALUE_NOT_SET;
}

void XWGridWindow::_updateSelection(int row, int column)
{
    // reset selection if any
    for(_gridRowsT::iterator rit = m_gridData.begin(); rit != m_gridData.end(); ++rit)
    {
        for(std::vector<GridCellData>::iterator cit = rit->cells.begin(); cit != rit->cells.end(); ++cit)
        {
            // reset selection
            cit->selected = false;
        }
    }

    // select cell 
    if(_isValidIndex(row, column))
    {        
        if(m_gridData.at(row).cells.at(column).selectable)
            m_gridData.at(row).cells.at(column).selected = true;
    }

    // repaint
    repaint();
}

void XWGridWindow::_createTextEditor()
{
    // ignore if already created
    if(m_gridEditor.textEditor) return;

    // create editor
    m_gridEditor.textEditor = new XLineEdit(hwnd(), this);
    m_gridEditor.textEditor->setFont(m_textFont);

    // subclass editor 
    if(!::SetWindowSubclass(m_gridEditor.textEditor->hwnd(), _subclassProc, XWGRID_SUBCLASS_ID_EDITBOX, (DWORD_PTR)this))
    {
        XWTRACE_WERR_LAST("XWGridWindow: failed to subclass editbox window");
    }
}

void XWGridWindow::_createListEditor()
{
    // ignore if already created
    if(m_gridEditor.listEditor) return;

    // create editor
    m_gridEditor.listEditor = new XComboBox(hwnd(), this, false);
    m_gridEditor.listEditor->setFont(m_textFont);

    // subclass combobox
    if(!::SetWindowSubclass(m_gridEditor.listEditor->hwnd(), _subclassProc, XWGRID_SUBCLASS_ID_COMBOBOX, (DWORD_PTR)this))
    {
        XWTRACE_WERR_LAST("XWGridWindow: failed to subclass combobox window");
    }

    // NOTE: we need to subclass also combobox parent window
    HWND comboParent = ::GetParent(m_gridEditor.listEditor->hwnd());
    if(comboParent && comboParent != hwnd())
    {
        if(!::SetWindowSubclass(m_gridEditor.listEditor->hwnd(), _subclassProc, XWGRID_SUBCLASS_ID_COMBOBOX_PARENT, (DWORD_PTR)this))
        {
            XWTRACE_WERR_LAST("XWGridWindow: failed to subclass combobox parent window");
        }
    }
}

void XWGridWindow::_completeEditing()
{
    // ignore if not editing
    if(!m_gridEditor.editing) return;

    bool isModified = false;

    XWASSERT(_isValidIndex(m_gridEditor.row, m_gridEditor.column));
    GridCellData& cellRef = m_gridData.at(m_gridEditor.row).cells.at(m_gridEditor.column);

    // check edited cell type
    if(cellRef.type == eCellTypeText)
    {
        XWASSERT(m_gridEditor.textEditor);
        if(m_gridEditor.textEditor)
        {
            // read text from editor
            m_strBuffer = m_gridEditor.textEditor->readText();

            // hide editor
            m_gridEditor.textEditor->hide();

            // check if modified
            isModified = (cellRef.text != m_strBuffer);

            // copy new value
            cellRef.text = m_strBuffer;
        }

    } else if(cellRef.type == eCellTypeList)
    {
        XWASSERT(m_gridEditor.listEditor);
        if(m_gridEditor.listEditor)
        {
            // check if item was selected
            if(m_gridEditor.listEditor->hasSelectedItem())
            {
                // get index from editor
                LRESULT selectedData = m_gridEditor.listEditor->selectedItemData();

                // validate
                if(selectedData >= 0 && selectedData < cellRef.listItemCount)
                {
                    // check if modified
                    isModified = (cellRef.listIndex != selectedData);

                    // copy value
                    cellRef.listIndex = (int)selectedData;
                    cellRef.text = cellRef.listItems[cellRef.listIndex];
                }
            }

            // hide editor
            m_gridEditor.listEditor->hide();
        }

    } else
    {
        XWASSERT1(0, "XWGridWindow: unsupported cell type");
    }

    // set modified flag
    if(!cellRef.modified)
        cellRef.modified = isModified;

    // report if modified
    if(isModified)
    {
        // set editor context
        m_contextRow = m_gridEditor.row;
        m_contextColumn = m_gridEditor.column;

        // inform about new value
        notifyParentWindow(XWGRID_NOTIFY_CELL_MODIFIED);
    }

    // reset editing flag
    m_gridEditor.editing = false;

    // layout update may be needed
    m_layoutUpdateNeeded = true;
}

void XWGridWindow::_cancelEditing()
{
    // ignore if not editing
    if(!m_gridEditor.editing) return;

    // check editor type
    if(m_gridEditor.editorType == eCellTypeText)
    {
        XWASSERT(m_gridEditor.textEditor);
        
        // hide editor
        if(m_gridEditor.textEditor)
            m_gridEditor.textEditor->hide();

    } else if(m_gridEditor.editorType == eCellTypeList)
    {
        XWASSERT(m_gridEditor.listEditor);
        
        // hide editor
        if(m_gridEditor.listEditor)
            m_gridEditor.listEditor->hide();

    } else
    {
        XWASSERT1(0, "XWGridWindow: unsupported cell editor type");
    }

    // reset editing flag
    m_gridEditor.editing = false;

    // layout update may be needed
    m_layoutUpdateNeeded = true;

    // set focus to grid
    setFocus();
}

void XWGridWindow::_updateEditing(int row, int column)
{
    // save editing if any
    _completeEditing();

    // start editing if possible
    if(_isValidIndex(row, column) && m_gridData.at(row).cells.at(column).editable)
    {   
        GridCellData& cellRef = m_gridData.at(row).cells.at(column);

        // init editor state
        m_gridEditor.row = row;
        m_gridEditor.column = column;
        m_gridEditor.editing = true;
        m_gridEditor.editorType = cellRef.type;

        int posX = 0;
        int posY = 0;

        // find position
        _getCellPos(row, column, posX, posY);

        // check edited cell type
        if(cellRef.type == eCellTypeText)
        {
            // create editor if needed
            _createTextEditor();

            // position editor
            m_gridEditor.textEditor->update(posX + m_gridStyle.lineWidth, 
                                            posY + m_gridStyle.lineWidth, 
                                            m_gridColumns.at(column).width + 2 * m_gridStyle.spacing, 
                                            m_gridData.at(row).height + 2 * m_gridStyle.spacing);

            // set editor text
            m_gridEditor.textEditor->setText(cellRef.text.c_str());

            // show editor
            m_gridEditor.textEditor->show();
            m_gridEditor.textEditor->setFocus();

            // move cursor at the end
            m_gridEditor.textEditor->setCursorPos((int)cellRef.text.length());

        } else if(cellRef.type == eCellTypeList)
        {
            // create editor if needed
            _createListEditor();

            // position editor
            m_gridEditor.listEditor->update(posX + m_gridStyle.lineWidth, 
                                            posY + m_gridStyle.lineWidth, 
                                            m_gridColumns.at(column).width + 2 * m_gridStyle.spacing, 
                                            m_gridData.at(row).height + 2 * m_gridStyle.spacing);

            // clear old values
            m_gridEditor.listEditor->resetContent();

            // set values
            for(int idx = 0; idx < cellRef.listItemCount; ++idx)
            {
                m_gridEditor.listEditor->addItem(cellRef.listItems[idx], idx);
            }

            // select current item
            if(cellRef.listIndex != XWGRID_VALUE_NOT_SET)
                m_gridEditor.listEditor->selectItem(cellRef.listIndex);

            // show editor
            m_gridEditor.listEditor->show();
            m_gridEditor.listEditor->setFocus();

        } else
        {
            XWASSERT1(0, "XWGridWindow: unsupported cell type");
        }
    }
}

XWHWND* XWGridWindow::_activeEditor()
{
    // ignore if not editing
    if(!m_gridEditor.editing) return 0;

    // check type
    if(m_gridEditor.editorType == eCellTypeText)
    {
        return m_gridEditor.textEditor;

    } else if(m_gridEditor.editorType == eCellTypeList)
    {
        return m_gridEditor.listEditor;
    }

    return 0;
}

void XWGridWindow::_moveEditor(int dX, int dY)
{
    // active editor
    XWHWND* editorWindow = _activeEditor();
    if(editorWindow == 0) return;

    LONG posX, posY;

    // get position relative to parent
    editorWindow->getRelativePos(hwnd(), posX, posY);

    // move
    editorWindow->move(posX + dX, posY + dY);
}

void XWGridWindow::_updateEditor()
{
    // active editor
    XWHWND* editorWindow = _activeEditor();
    if(editorWindow == 0) return;

    // validate edited cell just in case
    XWASSERT(_isValidIndex(m_gridEditor.row, m_gridEditor.column));
    if(!_isValidIndex(m_gridEditor.row, m_gridEditor.column)) return;

    int posX = 0;
    int posY = 0;

    // find position
    _getCellPos(m_gridEditor.row, m_gridEditor.column, posX, posY);

    // position editor
    editorWindow->update(posX + m_gridStyle.lineWidth, 
                         posY + m_gridStyle.lineWidth, 
                         m_gridColumns.at(m_gridEditor.column).width + 2 * m_gridStyle.spacing, 
                         m_gridData.at(m_gridEditor.row).height + 2 * m_gridStyle.spacing);
}

bool XWGridWindow::_handleKeyPressed(WPARAM wParam, LPARAM lParam)
{
    int row = 0;
    int column = 0;

    // process known keys
    if(wParam == VK_RETURN ||
       wParam == VK_UP ||
       wParam == VK_DOWN ||
       wParam == VK_LEFT ||
       wParam == VK_RIGHT)
    {
        // find current selection if any
        _findSelection(row, column);

        // if selection is not valid select first cell
        if(!_isValidIndex(row, column)) 
        {
             _updateSelection(0, 0);
            return true;
        }

        // process keys
        if(wParam == VK_RETURN)
        {
            // start editing selected cell
            _updateEditing(row, column);

            // automatically show drop down list for list items
            if(m_gridEditor.editing && m_gridEditor.editorType == eCellTypeList)
            {
                if(m_gridEditor.listEditor)
                    m_gridEditor.listEditor->showDropDownList(true);
            }

        } else if(wParam == VK_UP)
        {
            // move selection up if possible
            if(row > 0)
                _updateSelection(row - 1, column);

        } else if(wParam == VK_DOWN)
        {
            // move selection down if possible
            if(row + 1 < (int)m_gridData.size())
                _updateSelection(row + 1, column);

        } else if(wParam == VK_LEFT)
        {
            // move selection left if possible
            if(column > 0)
                _updateSelection(row, column - 1);

        } else if(wParam == VK_RIGHT)
        {
            // move selection right if possible
            if(column + 1 < m_columnCount)
                _updateSelection(row, column + 1);
        }

    } else if(wParam == VK_DELETE)
    {
        // find current selection if any
        _findSelection(row, column);

        // check if selection exists
        if(_isValidIndex(row, column))
        {
            GridCellData& cellRef = m_gridData.at(row).cells.at(column);

            // clear text value if cell is editable
            if(cellRef.editable && cellRef.type == eCellTypeText)
            {
                // clear text value
                cellRef.text.clear();

                // update
                repaint();
            }
        }

    } else
    {
        // ignore key
        return false;
    }

    // consume key
    return true;
}

/////////////////////////////////////////////////////////////////////
// paint methods
/////////////////////////////////////////////////////////////////////
void XWGridWindow::_paintCell(HDC hdc, int posX, int posY, int width, int height, const GridCellData& cell)
{
    // check type
    if(cell.type == eCellTypeText || cell.type == eCellTypeList)
    {
        // check if cell is modified
        if(cell.modified)
        {
            // select modified font
            ::SelectObject(hdc, m_modifiedFont);
        }

        // check if cell is selected
        if(cell.selected)
        {
            // selection color
            ::SetBkColor(hdc, m_gridStyle.selectedColor);
        }

        // paint rect
        ::Rectangle(hdc, posX, posY, posX + width, posY + height);

        // clip rect
        RECT clipRect;
        clipRect.left = posX + m_gridStyle.lineWidth;
        clipRect.top = posY + m_gridStyle.lineWidth;
        clipRect.right = posX + width - m_gridStyle.lineWidth;
        clipRect.bottom = posY + height - m_gridStyle.lineWidth;

        // text
        ::ExtTextOutW(hdc, posX + m_gridStyle.lineWidth + m_gridStyle.spacing, 
                           posY + m_gridStyle.lineWidth + m_gridStyle.spacing, 
                           ETO_OPAQUE | ETO_CLIPPED, &clipRect, cell.text.c_str(), (UINT)cell.text.length(), 0);

        // check if cell is modified
        if(cell.modified)
        {
            // select normal font back
            ::SelectObject(hdc, m_textFont);
        }

        // check if cell is selected
        if(cell.selected)
        {
            // normal color
            ::SetBkColor(hdc, m_gridStyle.fillColor);
        }

    } else if(cell.type == eCellTypeBitmap)
    {
        // TODO:
    }
}

/////////////////////////////////////////////////////////////////////
// subclass procedure
/////////////////////////////////////////////////////////////////////
LRESULT CALLBACK XWGridWindow::_subclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    // get window pointer
    XWGridWindow* gridWnd = (XWGridWindow*)dwRefData;
    if(gridWnd) 
    {
        // process known messages
        switch(uMsg)
        {
        case WM_KEYDOWN:
            // process known keys
            if(wParam == VK_RETURN)
            {
                // complete editing
                gridWnd->_completeEditing();

                // consume key
                return 0;

            } else if(wParam == VK_ESCAPE)
            {
                // cancel editing
                gridWnd->_cancelEditing();

                // consume key
                return 0;
            }
            break;

        case WM_KEYUP:
        case WM_CHAR:
            // consume keys
            if(wParam == VK_RETURN || wParam == VK_ESCAPE) return 0;
            break;

        case WM_LBUTTONDOWN:
            // set focus
            ::SetFocus(hwnd);
            break;

        case WM_KILLFOCUS:
            // complete editing
            gridWnd->_completeEditing();
            break;
        }
    }

    return ::DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

// XWGridWindow
/////////////////////////////////////////////////////////////////////
