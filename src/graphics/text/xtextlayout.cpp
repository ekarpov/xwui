// Text layout
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"

#include "../xwgraphicshelpers.h"
#include "../xd2dhelpres.h"
#include "../ximagefile.h"
#include "../ximage.h"
#include "../xd2dresourcescache.h"
#include "../xgdiresourcescache.h"

#include "xtextinlineobject.h"
#include "xtextinlineimage.h"
#include "xrichtext.h"
#include "xrichtextparser.h"
#include "xdwhelpers.h"
#include "xd2dtextlayout.h"
#include "xuniscribehelpers.h"
#include "xgditextlayout.h"
#include "xtextlayout.h"

/////////////////////////////////////////////////////////////////////
// XTextLayout - text layout funnctionality

/////////////////////////////////////////////////////////////////////
// construction/destruction
/////////////////////////////////////////////////////////////////////
XTextLayout::XTextLayout() :
    m_defaultTextColor(RGB(0, 0, 0)),
    m_gdiTextLayout(0),
    m_d2dTextLayout(0),
    m_width(0),
    m_runAnimation(false),
    m_hwndParent(0)
{
    // init rich text
    m_richText.addObserver(this);
}

XTextLayout::~XTextLayout()
{
    // release layouts if any
    if(m_gdiTextLayout) delete m_gdiTextLayout;
    m_gdiTextLayout = 0;
    if(m_d2dTextLayout) delete m_d2dTextLayout;
    m_d2dTextLayout = 0;
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XTextLayout::setPainterType(XWUIGraphicsPainter painterType)
{
    // check type
    if(painterType == XWUI_PAINTER_AUTOMATIC) 
        painterType = sXWUIDefaultPainter();

    if(painterType == XWUI_PAINTER_GDI && m_gdiTextLayout == 0)
    {
        // create layout
        m_gdiTextLayout = new XGdiTextLayout;

        // copy old text if any
        if(m_d2dTextLayout)
        {
            // copy layout properties
            m_gdiTextLayout->setWordWrap(m_d2dTextLayout->wordWrap());
            if(m_d2dTextLayout->backgroundFillEnabled())
                m_gdiTextLayout->enableBackgroundFill(m_d2dTextLayout->backgroundFillColor());
            else
                m_gdiTextLayout->disableBackgroundFill();
            m_gdiTextLayout->setAlignment(m_d2dTextLayout->alignment());

            // delete previous type
            delete m_d2dTextLayout;
            m_d2dTextLayout = 0;
        }

        // set text
        m_gdiTextLayout->setText(&m_richText);

    } else if(painterType == XWUI_PAINTER_D2D && m_d2dTextLayout == 0)
    {
        // check if DirectWrite is supported
        if(!XD2DHelpers::isDirect2DLoaded() || !XDWriteHelpers::isDirectWriteLoaded())
        {
            XWTRACE("XTextLayout::setPainterType Direct2D is not loaded, GDI layout will be used instead");

            // set GDI painter type
            setPainterType(XWUI_PAINTER_GDI);
            return;
        }

        // create layout
        m_d2dTextLayout = new XD2DTextLayout;

        // copy old text if any
        if(m_gdiTextLayout)
        {
            m_d2dTextLayout->setWordWrap(m_gdiTextLayout->wordWrap());
            if(m_gdiTextLayout->backgroundFillEnabled())
                m_d2dTextLayout->enableBackgroundFill(m_gdiTextLayout->backgroundFillColor());
            else
                m_d2dTextLayout->disableBackgroundFill();
            m_d2dTextLayout->setAlignment(m_gdiTextLayout->alignment());

            // delete previous type
            delete m_gdiTextLayout;
            m_gdiTextLayout = 0;
        }

        // set text
        m_d2dTextLayout->setText(&m_richText);
    }
}

/////////////////////////////////////////////////////////////////////
// size
/////////////////////////////////////////////////////////////////////
void XTextLayout::resize(int width)
{
    // check state
    if(!_validateState()) return;

    // copy width
    m_width = width;

    // resize active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->resize(width);
    else if(m_d2dTextLayout)
        m_d2dTextLayout->resize(width);
}

/////////////////////////////////////////////////////////////////////
// content size
/////////////////////////////////////////////////////////////////////
int XTextLayout::contentWidth(HDC hdc)
{
    // check input
    if(!_validateInput(hdc)) return 0;

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->contentWidth(hdc);
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->contentWidth();

    return 0;
}

int XTextLayout::contentHeight(HDC hdc)
{
    // check input
    if(!_validateInput(hdc)) return 0;

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->contentHeight(hdc);
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->contentHeight();

    return 0;
}

/////////////////////////////////////////////////////////////////////
// pain text
/////////////////////////////////////////////////////////////////////
void XTextLayout::setTextStyle(const XTextStyle& style)
{
    // check state
    if(!_validateState()) return;

    // copy default style
    m_defaultStyle = style;

    // assign style to whole text
    m_richText.setTextStyle(m_defaultStyle);
}

void XTextLayout::setTextColor(COLORREF textColor)
{
    // check state
    if(!_validateState()) return;

    // copy default color
    m_defaultTextColor = textColor;

    // assign color to whole text
    m_richText.setTextColor(m_defaultTextColor);
}

void XTextLayout::setText(const wchar_t* text)
{
    // check state
    if(!_validateState()) return;

    // set text
    m_richText.setText(text);

    // set default style and color
    m_richText.setTextStyle(m_defaultStyle);
    m_richText.setTextColor(m_defaultTextColor);
}

/////////////////////////////////////////////////////////////////////
// formatted text 
/////////////////////////////////////////////////////////////////////
void XTextLayout::setFormattedText(const wchar_t* text)
{
    // check state
    if(!_validateState()) return;

    // reset current text
    m_richText.reset();

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
void XTextLayout::enableAnimation(HWND parentWindow, bool enable)
{
    // ignore if the same
    if(enable == m_runAnimation) return;

    // validate input    
    if(enable && parentWindow == 0)
    {
        XWASSERT1(0, "XTextLayout: parent window must be set if animation is enabled");
        return;
    }

    // copy 
    m_runAnimation = enable;
    m_hwndParent = parentWindow;

    // get inline objects
    std::vector<XTextInlineObject*> inlineObjects;
    m_richText.getInlineObjects(inlineObjects);

    // pass to inline objects 
    for(std::vector<XTextInlineObject*>::iterator it = inlineObjects.begin();
        it != inlineObjects.end(); ++it)
    {
        (*it)->enableAnimation(parentWindow, enable);
    }
}

void XTextLayout::pauseAnimation()
{
    // get inline objects
    std::vector<XTextInlineObject*> inlineObjects;
    m_richText.getInlineObjects(inlineObjects);

    // pass to inline objects 
    for(std::vector<XTextInlineObject*>::iterator it = inlineObjects.begin();
        it != inlineObjects.end(); ++it)
    {
        (*it)->pauseAnimation();
    }
}

void XTextLayout::resumeAnimation()
{
    // get inline objects
    std::vector<XTextInlineObject*> inlineObjects;
    m_richText.getInlineObjects(inlineObjects);

    // pass to inline objects 
    for(std::vector<XTextInlineObject*>::iterator it = inlineObjects.begin();
        it != inlineObjects.end(); ++it)
    {
        (*it)->resumeAnimation();
    }
}

/////////////////////////////////////////////////////////////////////
// word wrap
/////////////////////////////////////////////////////////////////////
void XTextLayout::setWordWrap(bool bWordWrap)
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->setWordWrap(bWordWrap);
    else if(m_d2dTextLayout)
        m_d2dTextLayout->setWordWrap(bWordWrap);
}

bool XTextLayout::wordWrap() const
{
    // check state
    if(!_validateState()) return false;

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->wordWrap();
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->wordWrap();

    return false;
}

/////////////////////////////////////////////////////////////////////
// single line mode
/////////////////////////////////////////////////////////////////////
void XTextLayout::setSingleLineMode(bool singleLine)
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->setSingleLineMode(singleLine);
    else if(m_d2dTextLayout)
        m_d2dTextLayout->setSingleLineMode(singleLine);
}

bool XTextLayout::singleLineMode() const
{
    // check state
    if(!_validateState()) return false;

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->singleLineMode();
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->singleLineMode();

    return false;
}

/////////////////////////////////////////////////////////////////////
// alignment
/////////////////////////////////////////////////////////////////////
void XTextLayout::setAlignment(TTextAlignment textAlignment)
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->setAlignment(textAlignment);
    else if(m_d2dTextLayout)
        m_d2dTextLayout->setAlignment(textAlignment);
}

TTextAlignment XTextLayout::alignment() const
{
    // check state
    if(!_validateState()) return eTextAlignLeft;

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->alignment();
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->alignment();

    return eTextAlignLeft;
}

/////////////////////////////////////////////////////////////////////
// line spacing 
/////////////////////////////////////////////////////////////////////
void XTextLayout::setLinePadding(int beforeLine, int afterLine)
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->setLinePadding(beforeLine, afterLine);
    else if(m_d2dTextLayout)
        m_d2dTextLayout->setLinePadding(beforeLine, afterLine);
}

void XTextLayout::getLinePadding(int& beforeLine, int& afterLine) const
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->setLinePadding(beforeLine, afterLine);
    else if(m_d2dTextLayout)
        m_d2dTextLayout->setLinePadding(beforeLine, afterLine);
}

/////////////////////////////////////////////////////////////////////
// size calculations
/////////////////////////////////////////////////////////////////////
int XTextLayout::getHeightForWidth(HDC hdc, int width)
{
    // check input
    if(!_validateInput(hdc)) return 0;

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->getHeightForWidth(hdc, width);
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->getHeightForWidth(width);

    return 0;
}

/////////////////////////////////////////////////////////////////////
// line properties
/////////////////////////////////////////////////////////////////////
int XTextLayout::getLineCount(HDC hdc)
{
    // check input
    if(!_validateInput(hdc)) return 0;

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->getLineCount(hdc);
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->getLineCount();

    return 0;
}

bool XTextLayout::getLineMetrics(HDC hdc, int lineIdx, int& textBegin, int& textEnd, int& lineHeight)
{
    // check input
    if(!_validateInput(hdc)) return false;

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->getLineMetrics(hdc, lineIdx, textBegin, textEnd, lineHeight);
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->getLineMetrics(lineIdx, textBegin, textEnd, lineHeight);

    return false;
}

bool XTextLayout::getLineFitPos(HDC hdc, int lineIdx, int maxLineWidth, int& textPos)
{
    // check input
    if(!_validateInput(hdc)) return false;

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->getLineFitPos(hdc, lineIdx, maxLineWidth, textPos);
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->getLineFitPos(lineIdx, maxLineWidth, textPos);

    return false;
}

/////////////////////////////////////////////////////////////////////
// hit testing
/////////////////////////////////////////////////////////////////////
bool XTextLayout::isInsideText(HDC hdc, int originX, int originY, int posX, int posY)
{
    // check input
    if(!_validateInput(hdc)) return false;

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->isInsideText(hdc, originX, originY, posX, posY);
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->isInsideText(originX, originY, posX, posY);

    return false;
}

bool XTextLayout::isInsideSelection(HDC hdc, int originX, int originY, int posX, int posY)
{
    // check input
    if(!_validateInput(hdc)) return false;

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->isInsideSelection(hdc, originX, originY, posX, posY);
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->isInsideSelection(originX, originY, posX, posY);

    return false;
}

bool XTextLayout::getTextFromPos(HDC hdc, int originX, int originY, int posX, int posY, unsigned int& textPos)
{
    // check input
    if(!_validateInput(hdc)) return false;

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->getTextFromPos(hdc, originX, originY, posX, posY, textPos);
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->getTextFromPos(originX, originY, posX, posY, textPos);

    return false;
}

/////////////////////////////////////////////////////////////////////
// regions
/////////////////////////////////////////////////////////////////////
XRectRegion XTextLayout::getTextRegion(HDC hdc, int originX, int originY, int textPos, int textLength)
{
    // check input
    if(!_validateInput(hdc)) return XRectRegion();

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->getTextRegion(hdc, originX, originY, textPos, textLength);
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->getTextRegion(originX, originY, textPos, textLength);

    return XRectRegion();
}

/////////////////////////////////////////////////////////////////////
// selection
/////////////////////////////////////////////////////////////////////
bool XTextLayout::selectionActive() const
{
    // check state
    if(!_validateState()) return false;

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->selectionActive();
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->selectionActive();

    return false;
}

void XTextLayout::selectionBegin()
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->selectionBegin();
    else if(m_d2dTextLayout)
        m_d2dTextLayout->selectionBegin();
}

void XTextLayout::selectionEnd()
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->selectionEnd();
    else if(m_d2dTextLayout)
        m_d2dTextLayout->selectionEnd();
}

bool XTextLayout::selectTo(HDC hdc, int originX, int originY, int selectFomX, int selectFromY, int selectToX, int selectToY)
{
    // check input
    if(!_validateInput(hdc)) return false;

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->selectTo(hdc, originX, originY, selectFomX, selectFromY, selectToX, selectToY);
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->selectTo(originX, originY, selectFomX, selectFromY, selectToX, selectToY);

    return false;
}

void XTextLayout::clearSelection()
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->clearSelection();
    else if(m_d2dTextLayout)
        m_d2dTextLayout->clearSelection();
}

void XTextLayout::getSelectedText(XTextRange& selectedText)
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->getSelectedText(selectedText);
    else if(m_d2dTextLayout)
        m_d2dTextLayout->getSelectedText(selectedText);
}

/////////////////////////////////////////////////////////////////////
// selection colors
/////////////////////////////////////////////////////////////////////
void XTextLayout::setSelectionColor(COLORREF clFillColor)
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->setSelectionColor(clFillColor);
    else if(m_d2dTextLayout)
        m_d2dTextLayout->setSelectionColor(clFillColor);
}

void XTextLayout::setSelectionTextColor(COLORREF clTextColor)
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->setSelectionTextColor(clTextColor);
    else if(m_d2dTextLayout)
        m_d2dTextLayout->setSelectionTextColor(clTextColor);
}

void XTextLayout::resetSelectionTextColor()
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->resetSelectionTextColor();
    else if(m_d2dTextLayout)
        m_d2dTextLayout->resetSelectionTextColor();
}

/////////////////////////////////////////////////////////////////////
// background  
/////////////////////////////////////////////////////////////////////
void XTextLayout::setBackgroundFill(COLORREF clFillColor)
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->enableBackgroundFill(clFillColor);
    else if(m_d2dTextLayout)
        m_d2dTextLayout->enableBackgroundFill(clFillColor);
}

void XTextLayout::clearBackgroundFill()
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->disableBackgroundFill();
    else if(m_d2dTextLayout)
        m_d2dTextLayout->disableBackgroundFill();
}

bool XTextLayout::backgroundFillEnabled() const
{
    // check state
    if(!_validateState()) return false;

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->backgroundFillEnabled();
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->backgroundFillEnabled();

    return false;
}

COLORREF XTextLayout::backgroundFillColor() const
{
    // check state
    if(!_validateState()) return XWUIStyle::getColorBackground();

    // pass to active layout
    if(m_gdiTextLayout)
        return m_gdiTextLayout->backgroundFillColor();
    else if(m_d2dTextLayout)
        return m_d2dTextLayout->backgroundFillColor();

    return XWUIStyle::getColorBackground();
}

/////////////////////////////////////////////////////////////////////
// GDI properties
/////////////////////////////////////////////////////////////////////
void XTextLayout::enableGDIDoubleBuffering(bool enable)
{
    // NOTE: in Direct2D mode just ignore this, this is not an error state

    // pass to layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->enableDoubleBuffering(enable);
}

/////////////////////////////////////////////////////////////////////
// GDI painting 
/////////////////////////////////////////////////////////////////////
void XTextLayout::onInitGDIResources(HDC hdc)
{
    // pass to layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->onInitGDIResources(hdc);
}

void XTextLayout::onResetGDIResources()
{
    // pass to layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->onResetGDIResources();
}

void XTextLayout::onPaintGDI(HDC hdc, int originX, int originY, const RECT& rcPaint)
{
    XWASSERT1(m_gdiTextLayout, "XTextLayout: layout is not in GDI mode")
    if(m_gdiTextLayout == 0) return;

    // pass to layout
    m_gdiTextLayout->onPaint(hdc, originX, originY, rcPaint);
}

void XTextLayout::setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache)
{
    // pass to layout
    if(m_gdiTextLayout) 
        m_gdiTextLayout->setGDIResourcesCache(pXGdiResourcesCache);
}

/////////////////////////////////////////////////////////////////////
// Direct2D painting 
/////////////////////////////////////////////////////////////////////
void XTextLayout::onInitD2DTarget(ID2D1RenderTarget* pTarget)
{
    // pass to layout
    if(m_d2dTextLayout)
        m_d2dTextLayout->onInitD2DTarget(pTarget);
}

void XTextLayout::onResetD2DTarget()
{
    // pass to layout
    if(m_d2dTextLayout)
        m_d2dTextLayout->onResetD2DTarget();
}

void XTextLayout::onPaintD2D(int originX, int originY, ID2D1RenderTarget* pTarget, const RECT& rcPaint)
{
    XWASSERT1(m_d2dTextLayout, "XTextLayout: layout is not in Direct2D mode")
    if(m_d2dTextLayout == 0) return;

    // pass to layout
    m_d2dTextLayout->onPaintD2D(originX, originY, pTarget, rcPaint);
}

void XTextLayout::setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache)
{
    // pass to layout
    if(m_d2dTextLayout)
        m_d2dTextLayout->setD2DResourcesCache(pXD2DResourcesCache);
}

/////////////////////////////////////////////////////////////////////
// rich text changes (from IXRichTextObserver)
/////////////////////////////////////////////////////////////////////
void XTextLayout::onRichTextModified()
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->onRichTextModified();
    else if(m_d2dTextLayout)
        m_d2dTextLayout->onRichTextModified();
}

void XTextLayout::onRichTextStyleChanged(const XTextRange& range)
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->onRichTextStyleChanged(range);
    else if(m_d2dTextLayout)
        m_d2dTextLayout->onRichTextStyleChanged(range);
}

void XTextLayout::onRichTextColorChanged(const XTextRange& range)
{
    // check state
    if(!_validateState()) return;

    // pass to active layout
    if(m_gdiTextLayout)
        m_gdiTextLayout->onRichTextColorChanged(range);
    else if(m_d2dTextLayout)
        m_d2dTextLayout->onRichTextColorChanged(range);
}

/////////////////////////////////////////////////////////////////////
// rich text parsing (from IXRichTextParserObserver)
/////////////////////////////////////////////////////////////////////
void XTextLayout::onRichTextParserText(const wchar_t* text, const XTextStyle& style)
{
    // check state
    if(!_validateState()) return;

    // append text
    m_richText.appendText(text, &style, &m_defaultTextColor);
}

void XTextLayout::onRichTextParserColoredText(const wchar_t* text, const XTextStyle& style, const COLORREF& color)
{
    // check state
    if(!_validateState()) return;

    // append text
    m_richText.appendText(text, &style, &color);
}

void XTextLayout::onRichTextParserImage(const wchar_t* imageUri, int width, int height)
{
    // check state
    if(!_validateState()) return;

    // create inline image
    XTextInlineImage* inlineImage = new XTextInlineImage;
    inlineImage->AddRef();

    // load
    if(!inlineImage->setImageUri(imageUri, width, height))
    {
        XWTRACE("XTextLayout: failed to load inline image");
        
        // release image and exit
        inlineImage->Release();
        return;
    }

    // enable animation if needed 
    inlineImage->enableAnimation(m_hwndParent, m_runAnimation);

    // append image
    m_richText.appendInlineObject(inlineImage, &m_defaultStyle);

    // release reference
    inlineImage->Release();
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
bool XTextLayout::_validateState() const
{
    // check if layout is initialized
    if(m_gdiTextLayout == 0 && m_d2dTextLayout == 0)
    {
        XWASSERT1(0, "XTextLayout: layout is not initialized");
        return false;
    }

    return true;
}

bool XTextLayout::_validateInput(HDC hdc) const
{
    // validate state first
    if(!_validateState()) return false;

    // HDC must be set in GDI painter mode
    if(m_gdiTextLayout != 0 && hdc == 0)
    {
        XWASSERT1(0, "XTextLayout: device context must be set in GDI mode");
        return false;
    }

    return true;
}

// XTextLayout
/////////////////////////////////////////////////////////////////////
