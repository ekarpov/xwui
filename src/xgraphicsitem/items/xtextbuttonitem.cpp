// Text button grapchics item implementation
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"
#include "../../graphics/xwgraphics.h"

#include "../xgraphicsitem.h"

#include "xtextitem.h"
#include "xtextbuttonitem.h"

/////////////////////////////////////////////////////////////////////
// XTextButtonItem - text button graphics item

XTextButtonItem::XTextButtonItem(XGraphicsItem* parent)
{
    // init with default style
    _init(XWUIStyle::textButtonStyle(XWUIStyle::eXTextButtonTypeDefault));

    // add to parent
    if(parent)
        parent->addChildItem(this);
}

XTextButtonItem::XTextButtonItem(const XWUIStyle::XTextButtonStyle& style, XGraphicsItem* parent)
{
    _init(style);

    // add to parent
    if(parent)
        parent->addChildItem(this);
}

XTextButtonItem::~XTextButtonItem()
{
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XTextButtonItem::setText(const wchar_t* text)
{
    // set text for text item
    m_textItem->richText()->setText(text);

    // handle update content
    _handleContentChanged();
}

/////////////////////////////////////////////////////////////////////
// style
/////////////////////////////////////////////////////////////////////
void XTextButtonItem::setStyle(const XWUIStyle::XTextButtonStyle& style)
{
    // copy style
    m_btnStyle = style;

    // handle update content
    _handleContentChanged();
}

const XWUIStyle::XTextButtonStyle& XTextButtonItem::getStyle() const
{
    return m_btnStyle;
}

/////////////////////////////////////////////////////////////////////
// manipulations (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XTextButtonItem::update(int posX, int posY, int width, int height)
{
    // NOTE: we ignore width and height here as we assume fixed size for button
    //       (default size can be changed via style)
    width = minWidth();
    height = minHeight();

    // position text item
    int textWidth = m_textItem->contentWidth();
    int textHeight = m_textItem->contentHeight();
    int textPosX = posX + (width - textWidth) / 2;
    int textPosY = posY + height - textHeight - m_btnStyle.contentMargins.bottom;
    m_textItem->update(textPosX, textPosY, textWidth, textHeight);

    // pass to parent
    XGraphicsItem::update(posX, posY, width, height);
}

/////////////////////////////////////////////////////////////////////
// mouse events
/////////////////////////////////////////////////////////////////////
bool XTextButtonItem::onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags)
{
    // pass to parent first (to update state)
    XGraphicsItem::onMouseClick(uButtonMsg, posX, posY, flags);

    // mark as handled
    return true;
}

/////////////////////////////////////////////////////////////////////
// keyboard events (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
bool XTextButtonItem::onCharEvent(WPARAM charCode, LPARAM flags)
{
    // check enter or space
    if(charCode == VK_RETURN || charCode == VK_SPACE)
    {
        // send clicked event
        sendEvent(XGITEM_EVENT_CLICKED);

        // mark as processed
        return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////
// GDI painting
/////////////////////////////////////////////////////////////////////
void XTextButtonItem::onPaintGDI(HDC hdc, const RECT& rcPaint)
{
    // pen and brush
    HPEN borderPen = ::CreatePen(PS_SOLID, 1, 
        XWUIStyle::getStateColor(styleState(), m_btnStyle.borderColors));
    HBRUSH fillBrush = ::CreateSolidBrush(XWUIStyle::getStateColor(styleState(), m_btnStyle.fillColors));

    // select pen and brush
    HGDIOBJ oldPen = ::SelectObject(hdc, borderPen);
    HGDIOBJ oldBrush = ::SelectObject(hdc, fillBrush);

    ::RoundRect(hdc, m_itemRect.left, m_itemRect.top, m_itemRect.right, m_itemRect.bottom, m_btnStyle.roundRadius, m_btnStyle.roundRadius);

    // draw selection if button is pressed
    if(getStateFlag(STATE_FLAG_PRESSED))
    {
        HPEN pressedPen = ::CreatePen(PS_SOLID, 1, m_btnStyle.borderColors.pressed);
        ::SelectObject(hdc, pressedPen);
        ::SelectObject(hdc, ::GetStockObject(NULL_BRUSH));

        // draw shadow rect
        ::RoundRect(hdc, m_itemRect.left + 1, m_itemRect.top + 1, m_itemRect.right - 1, m_itemRect.bottom - 1, m_btnStyle.roundRadius, m_btnStyle.roundRadius);

        ::SelectObject(hdc, borderPen);
        ::DeleteObject(pressedPen);
    }

    // free gdi resources
    ::SelectObject(hdc, oldPen);
    ::SelectObject(hdc, oldBrush);
    ::DeleteObject(borderPen);
    ::DeleteObject(fillBrush);

    // paint child items
    XGraphicsItem::onPaintGDI(hdc, rcPaint);
}

/////////////////////////////////////////////////////////////////////
// Direct2D painting
/////////////////////////////////////////////////////////////////////
void XTextButtonItem::onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint)
{
    // convert item rectangle to DIPs
    D2D1_RECT_F itemRect;
    XD2DHelpers::gdiRectToD2dRect(m_itemRect, itemRect);

    // rounded rectangle
    D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(itemRect, (float)m_btnStyle.roundRadius, (float)m_btnStyle.roundRadius);

    // fill brush (use brush cache)
    ID2D1Brush* fillBrush = createD2DBrush(pTarget, XWUIStyle::getStateColorD2D(styleState(), m_btnStyle.fillColors));
    if(fillBrush == 0) return;

    // draw filled rectangle
    pTarget->FillRoundedRectangle(roundedRect, fillBrush);
    fillBrush->Release();

    fillBrush = createD2DBrush(pTarget, XWUIStyle::getStateColorD2D(styleState(), m_btnStyle.borderColors));
    if(fillBrush == 0) return;

    // draw border
    pTarget->DrawRoundedRectangle(roundedRect, fillBrush);
    fillBrush->Release();

    // draw selection if button is pressed
    if(getStateFlag(STATE_FLAG_PRESSED))
    {
        RECT pressedRect = m_itemRect;
        pressedRect.left += 1;
        pressedRect.top += 1;
        pressedRect.right -= 1;
        pressedRect.bottom -= 1;

        XD2DHelpers::gdiRectToD2dRect(pressedRect, itemRect);
        roundedRect = D2D1::RoundedRect(itemRect, (float)m_btnStyle.roundRadius, (float)m_btnStyle.roundRadius);

        // convert to D2D color structure
        D2D1_COLOR_F d2dColor;
        XD2DHelpers::colorrefToD2dColor(m_btnStyle.borderColors.pressed, d2dColor);

        fillBrush = createD2DBrush(pTarget, d2dColor);
        if(fillBrush == 0) return;

        // draw border
        pTarget->DrawRoundedRectangle(roundedRect, fillBrush);
        fillBrush->Release();
    }

    // paint child items
    XGraphicsItem::onPaintD2D(pTarget, rcPaint);
}

/////////////////////////////////////////////////////////////////////
// style state
/////////////////////////////////////////////////////////////////////
void XTextButtonItem::onStateFlagChanged(TStateFlag flag, bool value)
{
    // handle update content
    _handleContentChanged();

    // update UI
    repaint();
}

/////////////////////////////////////////////////////////////////////
// worker method
/////////////////////////////////////////////////////////////////////
void XTextButtonItem::_init(const XWUIStyle::XTextButtonStyle& style)
{
    // create text item
    m_textItem = new XTextItem(this);
    m_textItem->setWordWrap(false);
    m_textItem->clearBackgroundFill();
//    m_textItem->setLinePadding(0, 0);
//    m_textItem->setAlignment(eTextAlignLeft);

    // common properties
    setFocusable(true);
    setClickable(true);

    // set style
    setStyle(style);
}

void XTextButtonItem::_handleContentChanged()
{
    // update text style
    m_textItem->richText()->setTextStyle(m_btnStyle.textStyle);
    m_textItem->richText()->setTextColor(XWUIStyle::getStateColor(styleState(), m_btnStyle.textColors));

    // preffered width
    int btnWidth = m_textItem->contentWidth();
    if(btnWidth < m_btnStyle.defaultSize.width) btnWidth = m_btnStyle.defaultSize.width;

    // preffered height
    int btnHeight = m_textItem->contentHeight();
    if(btnHeight < m_btnStyle.defaultSize.height) btnHeight = m_btnStyle.defaultSize.height;

    // set fixed size
    setFixedSize(btnWidth, btnHeight);
}

// XTextButtonItem
/////////////////////////////////////////////////////////////////////
