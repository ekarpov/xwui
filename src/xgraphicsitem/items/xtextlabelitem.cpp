// Text label (single line of text)
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"
#include "../../graphics/xwgraphics.h"

#include "../xgraphicsitem.h"

#include "xtextlabelitem.h"

/////////////////////////////////////////////////////////////////////
// XTextLabelItem - single line text label

/////////////////////////////////////////////////////////////////////
// construction/destruction
/////////////////////////////////////////////////////////////////////
XTextLabelItem::XTextLabelItem(XGraphicsItem* parent) :
    m_cutToFit(false),
    m_isTextCut(false),
    m_isFormattedText(false),
    m_isClickable(false),
    m_isOverText(false),
    m_fitWidth(0),
    m_leftMargin(0),    
    m_topMargin(0),
    m_rightMargin(0),
    m_bottomMargin(0)
{
    // set default painter 
    setPainterType(XWUI_PAINTER_AUTOMATIC);

    // item properties
    enableContentScrolling(false);

    // layout properties
    m_textLayout.setWordWrap(false);
    m_textLayout.setSingleLineMode(true);

    // style
    setTextStyle(XWUIStyle::textStyle());

    // add to parent
    if(parent)
        parent->addChildItem(this);
}

XTextLabelItem::~XTextLabelItem()
{
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
void XTextLabelItem::setContentMargins(int left, int top, int right, int bottom)
{
    // copy margins
    m_leftMargin = left > 0 ? left : 0;
    m_topMargin = top > 0 ? top : 0;
    m_rightMargin = right > 0 ? right : 0;
    m_bottomMargin = bottom > 0 ? bottom : 0;

    // update size
    _updateItemSize();
}

void XTextLabelItem::enableCutToFit(bool enable)
{
    // ignore if the same
    if(m_cutToFit == enable) return;

    // copy flag
    m_cutToFit = enable;

    // set full text first
    if(!m_cutToFit)
        _setFullText();

    // reset state
    m_fitWidth = 0;

    // update size
    _updateItemSize();
}

void XTextLabelItem::setClickable(bool clickable)
{
    // set flag
    m_isClickable = clickable;
}

/////////////////////////////////////////////////////////////////////
// text
/////////////////////////////////////////////////////////////////////
void XTextLabelItem::setTextStyle(const XTextStyle& style)
{
    // pass to layout
    m_textLayout.setTextStyle(style);

    // update size
    _updateItemSize();
}

void XTextLabelItem::setTextColor(COLORREF textColor)
{
    // pass to layout
    m_textLayout.setTextColor(textColor);
}

void XTextLabelItem::setText(const wchar_t* text)
{
    // make text copy 
    if(text)
        m_labelText = text;
    else
        m_labelText.clear();

    // copy flag
    m_isFormattedText = false;

    // pass to layout
    m_textLayout.setText(text);

    // update size
    _updateItemSize();
}

void XTextLabelItem::setFormattedText(const wchar_t* text)
{
    // make text copy 
    if(text)
        m_labelText = text;
    else
        m_labelText.clear();

    // copy flag
    m_isFormattedText = true;

    // pass to layout
    m_textLayout.setFormattedText(text);

    // update size
    _updateItemSize();
}

/////////////////////////////////////////////////////////////////////
// text alignment
/////////////////////////////////////////////////////////////////////
void XTextLabelItem::setAlignment(TTextAlignment textAlignment)
{
    // pass to layout
    m_textLayout.setAlignment(textAlignment);
}

TTextAlignment XTextLabelItem::alignment() const
{
    // pass to layout
    return m_textLayout.alignment();
}

/////////////////////////////////////////////////////////////////////
// background  
/////////////////////////////////////////////////////////////////////
void XTextLabelItem::setBackgroundFill(const COLORREF& fillColor)
{
    // pass to layout
    m_textLayout.setBackgroundFill(fillColor);
}

void XTextLabelItem::clearBackgroundFill()
{
    // pass to layout
    m_textLayout.clearBackgroundFill();
}

/////////////////////////////////////////////////////////////////////
// item size and position (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextLabelItem::update(int posX, int posY, int width, int height)
{
    // pass to parent first
    XGraphicsItem::update(posX, posY, width, height);

    // ignore if item width is zero
    if(width == 0) return;

    // pass to layout
    m_textLayout.resize(width - (m_leftMargin + m_rightMargin));

    // check if we need to fit text
    if(m_cutToFit)
        _fitContent(width);
}

int XTextLabelItem::contentWidth()
{
    // pass to layout
    return m_textLayout.contentWidth(XWUtils::GetWindowDC(parentWindow())) + m_leftMargin + m_rightMargin;
}

int XTextLabelItem::contentHeight()
{
    // pass to layout
    return m_textLayout.contentHeight(XWUtils::GetWindowDC(parentWindow())) + m_topMargin + m_bottomMargin;
}

/////////////////////////////////////////////////////////////////////
// painter type (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextLabelItem::setPainterType(XWUIGraphicsPainter type)
{
    // pass to layout
    m_textLayout.setPainterType(type);

    // pass to parent
    XGraphicsItem::setPainterType(type);
}

/////////////////////////////////////////////////////////////////////
// mouse events (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextLabelItem::onMouseEnter(int posX, int posY)
{
    // update clickable state if needed
    _updateClickableState(posX, posY);

    // pass to parent
    XGraphicsItem::onMouseEnter(posX, posY);
}

void XTextLabelItem::onMouseMove(int posX, int posY, WPARAM flags)
{
    // update clickable state if needed
    _updateClickableState(posX, posY);

    // pass to parent
    XGraphicsItem::onMouseMove(posX, posY, flags);
}

bool XTextLabelItem::onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags)
{
    // update clickable state if needed
    _updateClickableState(posX, posY);

    // pass to parent
    return XGraphicsItem::onMouseClick(uButtonMsg, posX, posY, flags);
}

void XTextLabelItem::onMouseLeave()
{
    // reset clickable state if needed
    _resetClickableState();

    // pass to parent
    XGraphicsItem::onMouseLeave();
}

/////////////////////////////////////////////////////////////////////
// GDI painting (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextLabelItem::enableGDIDoubleBuffering(bool enable)
{
    // pass to layout
    m_textLayout.enableGDIDoubleBuffering(enable);
}

void XTextLabelItem::onPaintGDI(HDC hdc, const RECT& rcPaint)
{
    // intersect paint area with item rect
    RECT paintRect;
    ::IntersectRect(&paintRect, &rcPaint, &m_itemRect);

    // pass to layout
    m_textLayout.onPaintGDI(hdc, m_itemRect.left + m_leftMargin, m_itemRect.top + m_topMargin, paintRect);
}

/////////////////////////////////////////////////////////////////////
// GDI resource caching (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextLabelItem::onInitGDIResources(HDC hdc)
{
    // pass to parent
    XGraphicsItem::onInitGDIResources(hdc);

    // pass to layout
    m_textLayout.onInitGDIResources(hdc);

    // update size if needed
    if(minHeight() == 0)
        _updateItemSize();
}

void XTextLabelItem::onResetGDIResources()
{
    // pass to layout
    m_textLayout.onResetGDIResources();

    // pass to parent
    XGraphicsItem::onResetGDIResources();
}

void XTextLabelItem::setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache)
{
    // pass to layout
    m_textLayout.setGDIResourcesCache(pXGdiResourcesCache);

    // pass to parent
    XGraphicsItem::setGDIResourcesCache(pXGdiResourcesCache);
}

/////////////////////////////////////////////////////////////////////
// Direct2D painting (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextLabelItem::onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint)
{
    // intersect paint area with item rect
    RECT paintRect;
    ::IntersectRect(&paintRect, &rcPaint, &m_itemRect);

    // pass to layout
    m_textLayout.onPaintD2D(m_itemRect.left + m_leftMargin, m_itemRect.top + m_topMargin, pTarget, paintRect);
}

/////////////////////////////////////////////////////////////////////
// Direct2D resource caching (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextLabelItem::onInitD2DTarget(ID2D1RenderTarget* pTarget)
{
    // pass to parent
    XGraphicsItem::onInitD2DTarget(pTarget);

    // pass to layout
    m_textLayout.onInitD2DTarget(pTarget);

    // update size if needed
    if(minHeight() == 0)
        _updateItemSize();
}

void XTextLabelItem::onResetD2DTarget()
{
    // pass to layout
    m_textLayout.onResetD2DTarget();

    // pass to parent
    XGraphicsItem::onResetD2DTarget();
}

void XTextLabelItem::setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache)
{
    // pass to layout
    m_textLayout.setD2DResourcesCache(pXD2DResourcesCache);

    // pass to parent
    XGraphicsItem::setD2DResourcesCache(pXD2DResourcesCache);
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XTextLabelItem::_setFullText()
{
    if(m_isTextCut)
    {
        if(m_isFormattedText)
            m_textLayout.setFormattedText(m_labelText.c_str());
        else
            m_textLayout.setText(m_labelText.c_str());

        // reset flag
        m_isTextCut = false;
    }
}

void XTextLabelItem::_fitContent(int width)
{
    // ignore if window not set
    if(parentWindow() == 0) return;

    // ignore same width
    if(m_fitWidth == width) return;

    // set full text first
    _setFullText();

    // check if we need to fit text
    if(contentWidth() > width)
    {
        // max line width
        int maxLineWidth = width - (m_leftMargin + m_rightMargin);

        // get max text position
        int textPos = 0;
        if(m_textLayout.getLineFitPos(XWUtils::GetWindowDC(parentWindow()), 0, maxLineWidth, textPos) && 
            textPos < (int)m_labelText.length())
        {
            std::wstring cutText;

            // cut last chars to fit dots
            if(textPos > 3) 
                textPos -= 3;

            // cut text
            if(m_isFormattedText)
                XWUtils::cutFormattedText(m_labelText, textPos, cutText);
            else
                cutText.assign(m_labelText.begin(), m_labelText.begin() + textPos);

            // append dots
            cutText += L"...";

            // set text
            if(m_isFormattedText)
                m_textLayout.setFormattedText(cutText.c_str());
            else
                m_textLayout.setText(cutText.c_str());

            // mark text as cut
            m_isTextCut = true;
        }
    }

    // check if text was cut
    if(m_isTextCut)
    {
        // set full text as hint if text was cut
        if(m_isFormattedText)
        {
            // strip tags first
            std::wstring hintText;
            XWUtils::stripFormatTags(m_labelText, hintText);

            // set text
            setHintText(hintText.c_str());

        } else
        {
            // set text
            setHintText(m_labelText.c_str());
        }

    } else
    {
        // reset hint text if any
        setHintText(0);
    }

    // save updated fidth
    m_fitWidth = width;
}

void XTextLabelItem::_updateItemSize()
{
    // reset layout policy
    resetSizePolicy();

    // fix height
    setFixedHeight(contentHeight());

    // check if label can be smaller than content
    if(!m_cutToFit)
    {
        // minimum width
        setMinWidth(contentWidth());
    }

    // request update
    handleContentChanged();
}

void XTextLabelItem::_resetClickableState()
{
    // ignore if not clickable
    if(!m_isClickable) return;

    // reset state
    if(m_isOverText)
    {
        // update cursor
        XGraphicsItem::resetMouseCursor();

        // update text style
        m_textLayout.richText()->setUnderline(false, m_textLayout.richText()->totalRange());

        // reset flag
        m_isOverText = false;

        // update UI
        repaint();
    }
}

void XTextLabelItem::_updateClickableState(int posX, int posY)
{
    // ignore if not clickable
    if(!m_isClickable) return;

    // text width
    int contentWidth = m_textLayout.contentWidth(XWUtils::GetWindowDC(parentWindow()));

    // check if cursor if over text
    bool isOvertText = (posX >= m_itemRect.left + m_leftMargin) &&
                       (posX <= m_itemRect.left + m_leftMargin + contentWidth);

    // check if state has changed
    if(isOvertText != m_isOverText)
    {
        // save flag
        m_isOverText = isOvertText;

        // update state
        XGraphicsItem::setClickable(m_isOverText);

        // update state
        if(m_isOverText)
        {
            // update cursor
            XGraphicsItem::setMouseCursor(XWUtils::getSystemCursor(XWUtils::eCursorHand));

            // update text style
            m_textLayout.richText()->setUnderline(true, m_textLayout.richText()->totalRange());

        } else
        {
            // update cursor
            XGraphicsItem::resetMouseCursor();

            // update text style
            m_textLayout.richText()->setUnderline(false, m_textLayout.richText()->totalRange());
        }

        // update UI
        repaint();
    }
}

// XTextLabelItem
/////////////////////////////////////////////////////////////////////
