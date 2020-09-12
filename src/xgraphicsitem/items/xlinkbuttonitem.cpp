// Text link button grapchics item implementation
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"
#include "../../graphics/xwgraphics.h"

#include "../xgraphicsitem.h"

#include "xtextitem.h"
#include "xlinkbuttonitem.h"

/////////////////////////////////////////////////////////////////////
// XLinkButtonItem - link button graphics item

XLinkButtonItem::XLinkButtonItem(XGraphicsItem* parent)
{
    // init with default style
    _init(XWUIStyle::linkButtonStyle());

    // add to parent
    if(parent)
        parent->addChildItem(this);
}

XLinkButtonItem::XLinkButtonItem(const XWUIStyle::XLinkButtonStyle& style, XGraphicsItem* parent)
{
    _init(style);

    // add to parent
    if(parent)
        parent->addChildItem(this);
}

XLinkButtonItem::~XLinkButtonItem()
{
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XLinkButtonItem::setText(const wchar_t* text)
{
    // set text for text item
    m_textItem->richText()->setText(text);

    // handle update content
    _handleContentChanged();
}

/////////////////////////////////////////////////////////////////////
// style
/////////////////////////////////////////////////////////////////////
void XLinkButtonItem::setStyle(const XWUIStyle::XLinkButtonStyle& style)
{
    // copy style
    m_btnStyle = style;

    // handle update content
    _handleContentChanged();
}

const XWUIStyle::XLinkButtonStyle& XLinkButtonItem::getStyle() const
{
    return m_btnStyle;
}

/////////////////////////////////////////////////////////////////////
// manipulations (from XGraphicsItem)
/////////////////////////////////////////////////////////////////////
void XLinkButtonItem::update(int posX, int posY, int width, int height)
{
    // use margins
    m_textItem->update(posX + m_btnStyle.contentMargins.left, posY + m_btnStyle.contentMargins.top, 
        width - (m_btnStyle.contentMargins.left + m_btnStyle.contentMargins.right), 
        height - (m_btnStyle.contentMargins.top + m_btnStyle.contentMargins.bottom) );

    // pass to parent
    XGraphicsItem::update(posX, posY, width, height);
}

/////////////////////////////////////////////////////////////////////
// mouse events
/////////////////////////////////////////////////////////////////////
bool XLinkButtonItem::onMouseClick(UINT uButtonMsg, int posX, int posY, WPARAM flags)
{
    // pass to parent first (to update state)
    XGraphicsItem::onMouseClick(uButtonMsg, posX, posY, flags);

    // mark as handled
    return true;
}

/////////////////////////////////////////////////////////////////////
// style state
/////////////////////////////////////////////////////////////////////
void XLinkButtonItem::onStateFlagChanged(TStateFlag flag, bool value)
{
    // handle updated state
    _handleStateChanged();

    // repaint UI
    repaint();
}

/////////////////////////////////////////////////////////////////////
// worker method
/////////////////////////////////////////////////////////////////////
void XLinkButtonItem::_init(const XWUIStyle::XLinkButtonStyle& style)
{
    // create text item
    m_textItem = new XTextItem(this);
    m_textItem->setWordWrap(false);
    m_textItem->clearBackgroundFill();
//    m_textItem->setLinePadding(0, 0);
//    m_textItem->setAlignment(eTextAlignLeft);

    // common properties
    setFocusable(false);
    setClickable(true);

    // copy style
    m_btnStyle = style;

    // set pointer cursor
    XGraphicsItem::setMouseCursor(XWUtils::getSystemCursor(XWUtils::eCursorHand));
}

void XLinkButtonItem::_handleContentChanged()
{
    // find maximum size for all text styles
    int maxWidth, maxHeight;

    // normal size
    m_textItem->richText()->setTextStyle(m_btnStyle.normalStyle);
    maxWidth = m_textItem->contentWidth();
    maxHeight = m_textItem->contentHeight();

    // active size
    m_textItem->richText()->setTextStyle(m_btnStyle.activeStyle);
    int width = m_textItem->contentWidth();
    int height = m_textItem->contentHeight();
    if(width > maxWidth) maxWidth = width;
    if(height > maxHeight) maxHeight = height;

    // add margins
    maxWidth += m_btnStyle.contentMargins.left + m_btnStyle.contentMargins.right;
    maxHeight += m_btnStyle.contentMargins.top + m_btnStyle.contentMargins.bottom;

    // set fixed size
    setFixedSize(maxWidth, maxHeight);

    // update current style
    _handleStateChanged();
}

void XLinkButtonItem::_handleStateChanged()
{
    // check state
    if(getStateFlag(STATE_FLAG_MOUSEOVER) || getStateFlag(STATE_FLAG_PRESSED) || getStateFlag(STATE_FLAG_FOCUSED))
    {
        m_textItem->richText()->setTextStyle(m_btnStyle.activeStyle);
    } else
    {
        m_textItem->richText()->setTextStyle(m_btnStyle.normalStyle);
    }

    // set color
    m_textItem->richText()->setTextColor(XWUIStyle::getStateColor(styleState(), m_btnStyle.textColors));
}

// XLinkButtonItem
/////////////////////////////////////////////////////////////////////
