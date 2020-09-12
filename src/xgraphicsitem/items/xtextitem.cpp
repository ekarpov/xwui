// Text graphics item
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"
#include "../../graphics/xwgraphics.h"

#include "../xgraphicsitem.h"

#include "xtextitem.h"

/////////////////////////////////////////////////////////////////////
// XTextItem - text item

XTextItem::XTextItem(XGraphicsItem* parent) :
    m_activeCursor(eCursorDefault),
    m_originX(0),
    m_originY(0),
    m_runAnimation(false),
    m_selectionEnabled(false),
    m_dragAndDropEnabled(false),
    m_dragAndDropActive(false),
    m_linkColor(RGB(0, 0, 255)),
    m_activeRegion(0),
    m_observerRef(0)
{
    // set default painter 
    setPainterType(XWUI_PAINTER_AUTOMATIC);

    // default settings
    initDefault();

    // add to parent
    if(parent)
        parent->addChildItem(this);
}

XTextItem::~XTextItem()
{
    // delete regions
    _deleteClickableRegions();
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XTextItem::layoutText(int width)
{
    // ignore if item width is zero
    if(width == 0) return;

    // pass to layout
    m_textLayout.resize(width);
}

/////////////////////////////////////////////////////////////////////
// text item events observer
/////////////////////////////////////////////////////////////////////
void XTextItem::addObserver(IXTextItemObserver* observerRef)
{
    // TODO: observer array
    m_observerRef = observerRef;
}

void XTextItem::removeObserver(IXTextItemObserver* observerRef)
{
    // TODO: observer array
    if(m_observerRef == observerRef) 
    {
        m_observerRef = 0;
    }
}

/////////////////////////////////////////////////////////////////////
// default settings
/////////////////////////////////////////////////////////////////////
void XTextItem::initDefault()
{
    // default settings
    enableSelection(false);
    enableDragAndDrop(false);
    setWordWrap(false);
    setAlignment(eTextAlignLeft);
    setLinePadding(1, 1);
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
void XTextItem::enableSelection(bool enable)
{
    // ignore if same
    if(m_selectionEnabled == enable) return;

    // copy flag
    m_selectionEnabled = enable;
}

void XTextItem::enableDragAndDrop(bool enable)
{
    // TODO: http://msdn.microsoft.com/en-us/library/windows/desktop/gg153554(v=vs.85).aspx
    // splash screen (transparent window) http://code.logos.com/blog/2008/09/displaying_a_splash_screen_with_c_introduction.html

    // ignore if same
    if(m_dragAndDropEnabled == enable) return;

    // copy flag
    m_dragAndDropEnabled = enable;

    // if enabled then enable selection as well
    if(m_dragAndDropEnabled)
    {
        enableSelection(true);
    }
}

/////////////////////////////////////////////////////////////////////
// text
/////////////////////////////////////////////////////////////////////
void XTextItem::setTextStyle(const XTextStyle& style)
{
    // copy default style
    m_defaultStyle = style;

    // pass to layout
    m_textLayout.setTextStyle(style);
}

void XTextItem::setText(const wchar_t* text)
{
    // pass to layout
    m_textLayout.setText(text);
}

/////////////////////////////////////////////////////////////////////
// formatted text 
/////////////////////////////////////////////////////////////////////
void XTextItem::setFormattedText(const wchar_t* text)
{
    // reset current text
    m_textLayout.richText()->reset();

    // parse text
    if(text)
    {
        m_textParser.parseBegin(m_defaultStyle, this);
        m_textParser.parse(text, ::wcslen(text));
        m_textParser.parseEnd();
    }
}

/////////////////////////////////////////////////////////////////////
// inline object animation 
/////////////////////////////////////////////////////////////////////
void XTextItem::enableAnimation(bool enable)
{
    // ignore if same
    if(m_runAnimation == enable) return;

    // copy flag
    m_runAnimation = enable;

    // pass to layout if window is set
    if(parentWindow() != 0)
    {
        m_textLayout.enableAnimation(parentWindow(), m_runAnimation);
    }
}

/////////////////////////////////////////////////////////////////////
// advanced text properties
/////////////////////////////////////////////////////////////////////
XRichText* XTextItem::richText()
{
    // return text
    return m_textLayout.richText();
}

/////////////////////////////////////////////////////////////////////
// word wrap
/////////////////////////////////////////////////////////////////////
void XTextItem::setWordWrap(bool bWordWrap)
{
    // pass to layout
    m_textLayout.setWordWrap(bWordWrap);
}

bool XTextItem::wordWrap() const
{
    // pass to layout
    return m_textLayout.wordWrap();
}

/////////////////////////////////////////////////////////////////////
// alignment
/////////////////////////////////////////////////////////////////////
void XTextItem::setAlignment(TTextAlignment textAlignment)
{
    // pass to layout
    m_textLayout.setAlignment(textAlignment);
}

TTextAlignment XTextItem::alignment() const
{
    // pass to layout
    return m_textLayout.alignment();
}

/////////////////////////////////////////////////////////////////////
// line spacing 
/////////////////////////////////////////////////////////////////////
void XTextItem::setLinePadding(int beforeLine, int afterLine)
{
    // pass to layout
    m_textLayout.setLinePadding(beforeLine, afterLine);
}

void XTextItem::getLinePadding(int& beforeLine, int& afterLine) const
{
    // pass to layout
    m_textLayout.getLinePadding(beforeLine, afterLine);
}

/////////////////////////////////////////////////////////////////////
// size
/////////////////////////////////////////////////////////////////////
int XTextItem::getHeightForWidth(int width)
{
    // pass to layout
    return m_textLayout.getHeightForWidth(XWUtils::GetWindowDC(parentWindow()), width);
}

/////////////////////////////////////////////////////////////////////
// line properties
/////////////////////////////////////////////////////////////////////
int XTextItem::getLineCount()
{
    // pass to layout
    return m_textLayout.getLineCount(XWUtils::GetWindowDC(parentWindow()));
}

bool XTextItem::getLineMetrics(int lineIdx, int& textBegin, int& textEnd, int& lineHeight)
{
    // pass to layout
    return m_textLayout.getLineMetrics(XWUtils::GetWindowDC(parentWindow()), lineIdx, textBegin, textEnd, lineHeight);
}

/////////////////////////////////////////////////////////////////////
// hit testing
/////////////////////////////////////////////////////////////////////
bool XTextItem::isInsideText(int posX, int posY)
{
    // pass to layout
    return m_textLayout.isInsideText(XWUtils::GetWindowDC(parentWindow()), 
        m_originX - m_scrollOffsetX, m_originY - m_scrollOffsetY, posX, posY);
}

bool XTextItem::isInsideSelection(int posX, int posY)
{
    // pass to layout
    return m_textLayout.isInsideSelection(XWUtils::GetWindowDC(parentWindow()), 
        m_originX - m_scrollOffsetX, m_originY - m_scrollOffsetY, posX, posY);
}

bool XTextItem::getTextFromPos(int posX, int posY, unsigned int& textPos)
{
    // pass to layout
    return m_textLayout.getTextFromPos(XWUtils::GetWindowDC(parentWindow()), 
        m_originX - m_scrollOffsetX, m_originY - m_scrollOffsetY, posX, posY, textPos);
}

/////////////////////////////////////////////////////////////////////
// clickable text regions
/////////////////////////////////////////////////////////////////////
void XTextItem::addClickableText(const wchar_t* command, const XTextRange& range, const XTextStyle* hoverStyle, xstyle_index_t styleMask)
{
    // init clickable region
    XClickableTextRegion clickableRegion;
    clickableRegion.range = range;

    // init styles if set
    if(hoverStyle != 0 && styleMask != 0)
    {
        // copy style
        clickableRegion.hoverStyle = m_textLayout.richText()->hashFromTextStyle(*hoverStyle);
        clickableRegion.styleMask = styleMask;

    } else
    {
        // no style change required
        clickableRegion.hoverStyle = 0;
        clickableRegion.styleMask = 0;
    }

    // set event
    if(command)
        clickableRegion.command = command;

    // update text color
    m_textLayout.richText()->setTextColor(m_linkColor, range);

    // add
    m_clickableRegions.push_back(clickableRegion);
}

/////////////////////////////////////////////////////////////////////
// selection
/////////////////////////////////////////////////////////////////////
void XTextItem::clearSelection()
{
    // pass to layout
    m_textLayout.clearSelection();

    // update UI
    repaint();
}

void XTextItem::getSelectedText(XTextRange& selectedText)
{
    // pass to layout
    m_textLayout.getSelectedText(selectedText);
}

/////////////////////////////////////////////////////////////////////
// selection colors
/////////////////////////////////////////////////////////////////////
void XTextItem::setSelectionColor(COLORREF clFillColor)
{
    // pass to layout
    m_textLayout.setSelectionColor(clFillColor);
}

void XTextItem::setSelectionTextColor(COLORREF clTextColor)
{
    // pass to layout
    m_textLayout.setSelectionTextColor(clTextColor);
}

void XTextItem::resetSelectionTextColor()
{
    // pass to layout
    m_textLayout.resetSelectionTextColor();
}

/////////////////////////////////////////////////////////////////////
// link color (clickable area)
/////////////////////////////////////////////////////////////////////
void XTextItem::setLinkColor(COLORREF clLinkColor)
{
    // copy color
    m_linkColor = clLinkColor;
}

/////////////////////////////////////////////////////////////////////
// background  
/////////////////////////////////////////////////////////////////////
void XTextItem::setBackgroundFill(COLORREF clFillColor)
{
    // pass to layout
    m_textLayout.setBackgroundFill(clFillColor);
}

void XTextItem::clearBackgroundFill()
{
    // pass to layout
    m_textLayout.clearBackgroundFill();
}

bool XTextItem::backgroundFillEnabled() const
{
    // pass to layout
    return m_textLayout.backgroundFillEnabled();
}

COLORREF XTextItem::backgroundFillColor() const
{
    // pass to layout
    return m_textLayout.backgroundFillColor();
}

/////////////////////////////////////////////////////////////////////
// scrolling
/////////////////////////////////////////////////////////////////////
int XTextItem::contentWidth()
{
    // pass to layout
    return m_textLayout.contentWidth(XWUtils::GetWindowDC(parentWindow()));
}

int XTextItem::contentHeight()
{
    // pass to layout
    return m_textLayout.contentHeight(XWUtils::GetWindowDC(parentWindow()));
}

/////////////////////////////////////////////////////////////////////
// properties (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextItem::setObscured(bool bObscured)
{
    if(m_runAnimation)
    {
        // manage animations
        if(bObscured)
            m_textLayout.pauseAnimation();
        else
            m_textLayout.resumeAnimation();
    }
}

/////////////////////////////////////////////////////////////////////
// parent window (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextItem::setParentWindow(HWND hwndParent)
{
    // pass to parent first
    XGraphicsItem::setParentWindow(hwndParent);

    // enable animation 
    m_textLayout.enableAnimation(hwndParent, m_runAnimation);
}

/////////////////////////////////////////////////////////////////////
// painter type (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextItem::setPainterType(XWUIGraphicsPainter type)
{
    // pass to layout
    m_textLayout.setPainterType(type);

    // pass to parent
    XGraphicsItem::setPainterType(type);
}

/////////////////////////////////////////////////////////////////////
// position (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextItem::move(int posX, int posY)
{
    // pass to parent first
    XGraphicsItem::move(posX, posY);

    // copy new origin
    m_originX = posX;
    m_originY = posY;
}

/////////////////////////////////////////////////////////////////////
// manipulations (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextItem::update(int posX, int posY, int width, int height)
{
    // pass to parent first
    XGraphicsItem::update(posX, posY, width, height);

    // copy new origin
    m_originX = posX;
    m_originY = posY;

    // ignore if item width is zero
    if(width == 0) return;

    // pass to layout
    m_textLayout.resize(width);
}

/////////////////////////////////////////////////////////////////////
// properties (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextItem::setEnabled(bool bEnabled)
{
    // TODO: selection is not possible, change fonts? use "style stack" for that

    // pass to parent
    XGraphicsItem::setEnabled(bEnabled);
}

/////////////////////////////////////////////////////////////////////
// mouse events (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextItem::onMouseEnter(int posX, int posY)
{
    // pass to parent
    XGraphicsItem::onMouseEnter(posX, posY);

    // update regions
    _updateClickableRegions();
}

void XTextItem::onMouseMove(int posX, int posY, WPARAM flags)
{
    // pass to parent
    XGraphicsItem::onMouseMove(posX, posY, flags);

    if(m_textLayout.selectionActive())
    {
        // stop selection if mouse is not pressed anymore (may happen when mouse leaves window)
        if(!(flags & MK_LBUTTON))
        {
            _selectionEnd(posX, posY);
            return;
        }

        // update selection
        _selectTo(posX, posY);

        // maximum scrolling offset
        int scrollContentHeight = contentHeight() - height();

        // scroll content if needed
        if(scrollContentHeight > 0)
        {
            if((posY < m_itemRect.top) || (posY + m_itemRect.top > height()))
            {
                // needed offset
                int scrollOffset;
                if(posY < m_itemRect.top)
                    scrollOffset = scrollOffsetY() + (posY - m_itemRect.top);
                else
                    scrollOffset = scrollOffsetY() + (posY - m_itemRect.top) - height();

                // check for overflows
                if(scrollOffset < 0) 
                    scrollOffset = 0;
                else if(scrollOffset > scrollContentHeight)
                    scrollOffset = scrollContentHeight;

                // update click point position
                m_clickPoint.y += (scrollOffsetY() - scrollOffset);

                // set scroll offset
                IXWScrollable::setScrollOffsetY(scrollOffset);

                // request full content update
                handleContentChanged();
            }
        }

    } else if(m_dragAndDropActive)
    {
        // TODO:
    }

    // update mouse cursor shape
    _updateMouseCursor(posX, posY);
}

bool XTextItem::onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags)
{
    // pass to parent
    XGraphicsItem::onMouseClick(uButtonMsg, posX, posY, flags);

    // ignore if selection is not enabled and control is not editable
    if(!m_selectionEnabled) return false;

    if(uButtonMsg == WM_LBUTTONDOWN)
    {
        if(m_selectionEnabled)
        {
            // clear old selection if any

            // TODO: check where click happens, if this is on selection area then 
            //       wait for button up event to clear selection, otherwise there
            //       might be text drag and drop starting

            // start selection
            _selectionBegin(posX, posY);

            // mark as consumed
            return true;
        }

    } else if(uButtonMsg == WM_LBUTTONUP)
    {
        // stop selection
        _selectionEnd(posX, posY);
        
        // check if point is the same
        if(m_clickPoint.x == posX && m_clickPoint.y == posY)
        {
            // inform active clickable item if any
            if(m_activeRegion)
            {
                // inform observer
                if(m_observerRef) 
                    m_observerRef->onTextRangeClicked(m_activeRegion->range, m_activeRegion->command);
            }
        }

        // mark as consumed
        return true;
    }

    return false;
}

void XTextItem::onMouseLeave()
{
    // pass to parent
    XGraphicsItem::onMouseLeave();

    // inform active clickable region that mouse has left
    _onClickableRangeMouseLeave(m_activeRegion);

    // reset active region
    m_activeRegion = 0;

    // destroy regions
    _deleteClickableRegions();
}

void XTextItem::onMouseCaptureReset()
{
    if(m_textLayout.selectionActive())
    {
        // pass to layout
        m_textLayout.selectionEnd();
    }

    // pass to parent
    XGraphicsItem::onMouseCaptureReset();
}

bool XTextItem::onSetCursor()
{
    // block class cursor
    return true;
}

/////////////////////////////////////////////////////////////////////
// keyboard events (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
bool XTextItem::onCharEvent(WPARAM charCode, LPARAM flags)
{
    // TODO:
    return false;
}

/////////////////////////////////////////////////////////////////////
// GDI painting (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextItem::enableGDIDoubleBuffering(bool enable)
{
    // pass to layout
    m_textLayout.enableGDIDoubleBuffering(enable);
}

void XTextItem::onPaintGDI(HDC hdc, const RECT& rcPaint)
{
    // intersect paint area with item rect
    RECT paintRect;
    ::IntersectRect(&paintRect, &rcPaint, &m_itemRect);

    // pass to layout
    m_textLayout.onPaintGDI(hdc, m_originX - m_scrollOffsetX, m_originY - m_scrollOffsetY, paintRect);
}

/////////////////////////////////////////////////////////////////////
// GDI resource caching (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextItem::onInitGDIResources(HDC hdc)
{
    // pass to parent
    XGraphicsItem::onInitGDIResources(hdc);

    // pass to layout
    m_textLayout.onInitGDIResources(hdc);
}

void XTextItem::onResetGDIResources()
{
    // pass to layout
    m_textLayout.onResetGDIResources();

    // pass to parent
    XGraphicsItem::onResetGDIResources();
}

void XTextItem::setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache)
{
    // pass to layout
    m_textLayout.setGDIResourcesCache(pXGdiResourcesCache);

    // pass to parent
    XGraphicsItem::setGDIResourcesCache(pXGdiResourcesCache);
}

/////////////////////////////////////////////////////////////////////
// Direct2D painting (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextItem::onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint)
{
    // intersect paint area with item rect
    RECT paintRect;
    ::IntersectRect(&paintRect, &rcPaint, &m_itemRect);

    // pass to layout
    m_textLayout.onPaintD2D(m_originX - m_scrollOffsetX, m_originY - m_scrollOffsetY, pTarget, paintRect);
}

/////////////////////////////////////////////////////////////////////
// Direct2D resource caching (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextItem::onInitD2DTarget(ID2D1RenderTarget* pTarget)
{
    // pass to parent
    XGraphicsItem::onInitD2DTarget(pTarget);

    // pass to layout
    m_textLayout.onInitD2DTarget(pTarget);
}

void XTextItem::onResetD2DTarget()
{
    // pass to layout
    m_textLayout.onResetD2DTarget();

    // pass to parent
    XGraphicsItem::onResetD2DTarget();
}

void XTextItem::setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache)
{
    // pass to layout
    m_textLayout.setD2DResourcesCache(pXD2DResourcesCache);

    // pass to parent
    XGraphicsItem::setD2DResourcesCache(pXD2DResourcesCache);
}

/////////////////////////////////////////////////////////////////////
// rich text parsing (from IXRichTextParserObserver)
/////////////////////////////////////////////////////////////////////
void XTextItem::onRichTextParserText(const wchar_t* text, const XTextStyle& style)
{
    // pass to layout
    m_textLayout.onRichTextParserText(text, style);
}

void XTextItem::onRichTextParserColoredText(const wchar_t* text, const XTextStyle& style, const COLORREF& color)
{
    // pass to layout
    m_textLayout.onRichTextParserColoredText(text, style, color);
}

void XTextItem::onRichTextParserLink(const XTextRange& range, const wchar_t* url)
{
    XTextStyle hoverStyle;
    hoverStyle.bUnderline = true;

    addClickableText(url, range, &hoverStyle, XTEXTSTYLE_UNDERLINE_FLAG);
}

void XTextItem::onRichTextParserImage(const wchar_t* imageUri, int width, int height)
{
    // pass to layout
    m_textLayout.onRichTextParserImage(imageUri, width, height);
}

/////////////////////////////////////////////////////////////////////
// helper methods
/////////////////////////////////////////////////////////////////////
void XTextItem::_deleteClickableRegions()
{
    // loop over all items
    for(unsigned int idx = 0; idx < m_clickableRegions.size(); ++idx)
    {
        // delete old region if any
        m_clickableRegions.at(idx).region.clear();
    }
}

void XTextItem::_updateClickableRegions()
{
    // loop over all items
    for(unsigned int idx = 0; idx < m_clickableRegions.size(); ++idx)
    {
        // active region
        XClickableTextRegion& textRegion = m_clickableRegions.at(idx);

        // delete old region if any
        textRegion.region.clear();

        // get region (NOTE: ignore scrolling)
        textRegion.region = m_textLayout.getTextRegion(XWUtils::GetWindowDC(parentWindow()), m_originX, m_originY, 
            textRegion.range.pos, textRegion.range.length);
    }
}

void XTextItem::_updateMouseCursor(int posX, int posY)
{
    // ignore if selection is not enabled and there are no clickable regions
    if(!m_selectionEnabled && m_clickableRegions.size() == 0) return;

    // if selection active always use selection cursor
    if(m_textLayout.selectionActive())
    {
        // selection cursor
        _setActiveCursor(eCursorSelection);
        return;
    }

    // check point position
    bool bOverText = (m_selectionEnabled && isInsideText(posX, posY));
    bool bOverSelectedText = (m_selectionEnabled && isInsideSelection(posX, posY));

    // NOTE: we do not update regions while scrolling, so update positions here instead
    posX += m_scrollOffsetX;
    posY += m_scrollOffsetY;

    // check if mouse is over active region
    bool bOverLinkText = false; 
    if(m_activeRegion && XWUIGraphics::isInsideRectRegion(m_activeRegion->region, posX, posY))
    {
        // mark flag
        bOverLinkText = true;

    } else
    {
        // handle mouse leave for active region
        _onClickableRangeMouseLeave(m_activeRegion);

        // reset active region
        m_activeRegion = 0;

        // check if over other region
        for(unsigned int idx = 0; idx < m_clickableRegions.size(); ++idx)
        {
            // active region
            XClickableTextRegion& textRegion = m_clickableRegions.at(idx);

            // check if point is inside
            if(XWUIGraphics::isInsideRectRegion(textRegion.region, posX, posY))
            {
                // mark flag
                bOverLinkText = true;

                // set new active region
                m_activeRegion = &textRegion;

                // handle mouse enter for active region
                _onClickableRangeMouseEnter(m_activeRegion);

                // stop search
                break;
            }
        }
    }


    // select cursors
    if(bOverLinkText)
    {
        // pointer cursor
        _setActiveCursor(eCursorPointer);

    } else if(bOverSelectedText)
    {
        // default cursor
        _setActiveCursor(eCursorDefault);

    } else if(bOverText)
    {
        // selection cursor
        _setActiveCursor(eCursorSelection);

    } else
    {
        // default cursor
        _setActiveCursor(eCursorDefault);
    }
}

void XTextItem::_onClickableRangeMouseEnter(XClickableTextRegion* region)
{
    // ignore if no region set
    if(region == 0) return;

    // inform observer if set
    if(m_observerRef) 
    {
        // inform observer
        m_observerRef->onTextRangeMouseEnter(region->range, region->command);
    }

    // update text style if mask set
    if(region->styleMask)
    {
        // clear old styles
        region->originalStyles.clear();

        // copy original styles
        m_textLayout.richText()->getTextStyles(region->range, region->originalStyles);

        // apply new styles
        m_textLayout.richText()->applyTextStyleMask(region->range, region->hoverStyle, region->styleMask);
    }

    // update region if observer set or style mask
    if(m_observerRef || region->styleMask != 0)
    {
        // request UI update
        if(region->region.size() > 0)
            repaint();
    }
}

void XTextItem::_onClickableRangeMouseLeave(XClickableTextRegion* region)
{
    // ignore if no region set
    if(region == 0) return;

    // inform observer if set
    if(m_observerRef) 
    {
        // handle mouse leave
        m_observerRef->onTextRangeMouseLeave(region->range, region->command);
    }

    // restore original styles if set
    if(region->originalStyles.size() == region->range.length)
    {
        // restore
        m_textLayout.richText()->applyTextStyles(region->range, region->originalStyles);
    }

    // clear original styles if any
    region->originalStyles.clear();

    // update region if observer set or style mask
    if(m_observerRef || region->styleMask != 0)
    {
        // request UI update
        if(region->region.size() > 0)
            repaint();
    }
}

/////////////////////////////////////////////////////////////////////
// selection helpers
/////////////////////////////////////////////////////////////////////
void XTextItem::_selectionBegin(int posX, int posY)
{
    // copy click point
    m_clickPoint.x = posX;
    m_clickPoint.y = posY;

    // pass to layout
    m_textLayout.selectionBegin();

    // start mouse capture
    setMouseCapture();
}

void XTextItem::_selectTo(int posX, int posY)
{
    bool updateUI = false;

    // pass to layout
    updateUI = m_textLayout.selectTo(XWUtils::GetWindowDC(parentWindow()), 
        m_originX - m_scrollOffsetX, m_originY - m_scrollOffsetY, m_clickPoint.x, m_clickPoint.y, posX, posY);

    // update UI
    if(updateUI)
    {
        repaint();
    }
}

void XTextItem::_selectionEnd(int posX, int posY)
{
    // pass to layout
    m_textLayout.selectionEnd();

    // clear selection if point is the same
    if(m_clickPoint.x == posX && m_clickPoint.y == posY)
    {
        // clear
        clearSelection();
    }

    // reset mouse capture
    if(getStateFlag(STATE_FLAG_MOUSECAPTURE))
    {
        resetMouseCapture();
    }
}

/////////////////////////////////////////////////////////////////////
// cursor helpers
/////////////////////////////////////////////////////////////////////
void XTextItem::_setActiveCursor(TActiveCursor cursor)
{
    // NOTE: MSDN says that "The cursor is set only if the new cursor is different 
    //       from the previous cursor; otherwise, the function returns immediately."
    
    // ignore if the same
    if(m_activeCursor == cursor) return;

    // update cursor
    if(cursor == eCursorSelection)
    {
        // selection cursor
        XGraphicsItem::setMouseCursor(XWUtils::getSystemCursor(XWUtils::eCursorIBeam));

    } else if(cursor == eCursorPointer)
    {
        // pointer cursor
        XGraphicsItem::setMouseCursor(XWUtils::getSystemCursor(XWUtils::eCursorHand));

    } else
    {
        // default cursor
        XGraphicsItem::resetMouseCursor();
    }

    // copy cursor
    m_activeCursor = cursor;
}

// XTextItem
/////////////////////////////////////////////////////////////////////


