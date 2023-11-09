// Direct2D based text layout and rendering
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"

#include "../xwgraphicshelpers.h"
#include "../xd2dhelpres.h"
#include "../xd2dresourcescache.h"

#include "xtextinlineobject.h"
#include "xrichtext.h"
#include "xdwhelpers.h"
#include "xd2dtextlayout.h"

/////////////////////////////////////////////////////////////////////
// XD2DTextLayout - Direct2D based text layout

XD2DTextLayout::XD2DTextLayout() :
    m_pXD2DResourcesCache(0)
{
}

XD2DTextLayout::~XD2DTextLayout()
{
    // release resources
    onResetD2DTarget();

    // release shared cache if any
    if(m_pXD2DResourcesCache) m_pXD2DResourcesCache->Release();
    m_pXD2DResourcesCache = 0;

    // release font cache
    _releaseFontCache();
}

/////////////////////////////////////////////////////////////////////
// layout size
/////////////////////////////////////////////////////////////////////
int XD2DTextLayout::contentWidth()
{
    // pass to parent and convert to pixels
    return XD2DHelpers::dipsToPixelsX(XTextLayoutBaseT::contentWidth());
}

int XD2DTextLayout::contentHeight()
{
    // pass to parent and convert to pixels
    return XD2DHelpers::dipsToPixelsY(XTextLayoutBaseT::contentHeight());
}

void XD2DTextLayout::resize(int layoutWidth)
{
    // convert to DIPs and resize
    XTextLayoutBaseT::resize(XD2DHelpers::pixelsToDipsX(layoutWidth));
}

/////////////////////////////////////////////////////////////////////
// size hints
/////////////////////////////////////////////////////////////////////
int XD2DTextLayout::getHeightForWidth(int width)
{
    return XD2DHelpers::dipsToPixelsX(XTextLayoutBaseT::getHeightForWidth(XD2DHelpers::pixelsToDipsX(width)));
}

/////////////////////////////////////////////////////////////////////
// line properties
/////////////////////////////////////////////////////////////////////
bool XD2DTextLayout::getLineMetrics(int lineIdx, int& textBegin, int& textEnd, int& lineHeight)
{
    float lineHeightDips;

    // pass to base layout
    bool retVal = XTextLayoutBaseT::getLineMetrics(lineIdx, textBegin, textEnd, lineHeightDips);

    // convert to dips
    lineHeight = retVal ? XD2DHelpers::dipsToPixelsY(lineHeightDips) : 0;

    return retVal;
}

bool XD2DTextLayout::getLineFitPos(int lineIdx, int maxLineWidth, int& textPos)
{
    // pass to base layout
    return XTextLayoutBaseT::getLineFitPos(lineIdx, XD2DHelpers::pixelsToDipsX(maxLineWidth), textPos);
}

/////////////////////////////////////////////////////////////////////
// line padding 
/////////////////////////////////////////////////////////////////////
void XD2DTextLayout::setLinePadding(int beforeLine, int afterLine)
{
    // copy line spacing
    m_linePaddingAfter = XD2DHelpers::pixelsToDipsY(afterLine);
    m_linePaddingBefore = XD2DHelpers::pixelsToDipsY(beforeLine);
}

void XD2DTextLayout::getLinePadding(int& beforeLine, int& afterLine) const
{
    // copy line spacing
    afterLine = XD2DHelpers::dipsToPixelsY(m_linePaddingAfter);
    beforeLine = XD2DHelpers::dipsToPixelsY(m_linePaddingBefore);
}

/////////////////////////////////////////////////////////////////////
// hit testing
/////////////////////////////////////////////////////////////////////
bool XD2DTextLayout::isInsideText(int originX, int originY, int posX, int posY)
{
    // convert DIP's and pass to parent
    return XTextLayoutBaseT::isInsideText(XD2DHelpers::pixelsToDipsX(originX), XD2DHelpers::pixelsToDipsY(originY),
                                     XD2DHelpers::pixelsToDipsX(posX), XD2DHelpers::pixelsToDipsY(posY));
}

bool XD2DTextLayout::isInsideSelection(int originX, int originY, int posX, int posY)
{
    // convert DIP's and pass to parent
    return XTextLayoutBaseT::isInsideSelection(XD2DHelpers::pixelsToDipsX(originX), XD2DHelpers::pixelsToDipsY(originY),
                                     XD2DHelpers::pixelsToDipsX(posX), XD2DHelpers::pixelsToDipsY(posY));
}

bool XD2DTextLayout::getTextFromPos(int originX, int originY, int posX, int posY, unsigned int& textPos)
{
    // convert DIP's and pass to parent
    return XTextLayoutBaseT::getTextFromPos(XD2DHelpers::pixelsToDipsX(originX), XD2DHelpers::pixelsToDipsY(originY),
                                       XD2DHelpers::pixelsToDipsX(posX), XD2DHelpers::pixelsToDipsY(posY),
                                       textPos);
}

/////////////////////////////////////////////////////////////////////
// layout regions
/////////////////////////////////////////////////////////////////////
XRectRegion XD2DTextLayout::getTextRegion(int originX, int originY, unsigned int textPos, unsigned int textLength)
{
    // convert DIP's and pass to parent
    return XTextLayoutBaseT::getTextRegion(XD2DHelpers::pixelsToDipsX(originX), XD2DHelpers::pixelsToDipsY(originY), textPos, textLength);
}

/////////////////////////////////////////////////////////////////////
// selection
/////////////////////////////////////////////////////////////////////
bool XD2DTextLayout::selectTo(int originX, int originY, int selectFomX, int selectFromY, int selectToX, int selectToY)
{
    // convert DIP's and pass to parent
    return XTextLayoutBaseT::selectTo(XD2DHelpers::pixelsToDipsX(originX), XD2DHelpers::pixelsToDipsY(originY),
                                 XD2DHelpers::pixelsToDipsX(selectFomX), XD2DHelpers::pixelsToDipsY(selectFromY),
                                 XD2DHelpers::pixelsToDipsX(selectToX), XD2DHelpers::pixelsToDipsY(selectToY) );
}

/////////////////////////////////////////////////////////////////////
// painting
/////////////////////////////////////////////////////////////////////
void XD2DTextLayout::onPaintD2D(int originX, int originY, ID2D1RenderTarget* pTarget, const RECT& rcPaint)
{
    // convert paint rectangle to DIPs
    D2D1_RECT_F paintRectDip;
    XD2DHelpers::gdiRectToD2dRect(rcPaint, paintRectDip);
    
    // convert paint origin to DIPs
    FLOAT originDipsX = XD2DHelpers::pixelsToDipsX(originX);
    FLOAT originDipsY = XD2DHelpers::pixelsToDipsY(originY);

    // init background brush if needed
    ID2D1Brush* bgColorBrush = 0;
    if(m_bFillBackground)
    {
        // get background brush
        bgColorBrush = _getColorBrush(pTarget, m_clBackground);

        // stop if some problems
        if(bgColorBrush == 0) return;

        // fill background
        pTarget->FillRectangle(paintRectDip, bgColorBrush);
    }

    // ignore if no text set
    if(m_richText == 0 || m_richText->textLength() == 0) return;

    // update layout if needed
    _updateLayoutIfNeeded();

    // switch off antialiasing mode
    D2D1_ANTIALIAS_MODE antialiasMode = pTarget->GetAntialiasMode();
    pTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

    // clip to required paint area
    pTarget->PushAxisAlignedClip(paintRectDip, D2D1_ANTIALIAS_MODE_ALIASED);

    // NOTE: use pixels here as XTextLayoutBaseT uses pixels, convert to DIPs only before painting

    // paint position
    FLOAT paintPosX = XD2DHelpers::pixelsToDipsX(originX);
    FLOAT paintPosY = XD2DHelpers::pixelsToDipsY(originY);

    // loop over text paragraphs
    for(unsigned int paraIdx = 0; paraIdx < m_textLayout.size() && paintPosY < paintRectDip.bottom; ++paraIdx)
    {
        // active paragraph
        XTextParagraph& textParagraph = m_textLayout.at(paraIdx);

        // loop over layout lines
        for(unsigned int lineIdx = 0; lineIdx < textParagraph.layoutLines.size(); ++lineIdx)
        {
            // active text line
            XLayoutLine& layoutLine = textParagraph.layoutLines.at(lineIdx);

            FLOAT lineHeight, paintRight;

            // line metrics
            _getLineMetrics(originDipsX, originDipsY, layoutLine, paintPosX, paintRight, lineHeight, textParagraph.isRTL);

            // paint only visible lines
            if(paintPosY + lineHeight <= paintRectDip.top) 
            {
                // update paint position
                paintPosY += lineHeight;

                // next
                continue;
            }

            // paint line 
            paintRight = _paintLayoutLine(pTarget, paintPosX, paintPosY, paintRectDip, textParagraph, layoutLine);

            // update paint position
            paintPosY += lineHeight;
        }
    }

    // remove clipping
    pTarget->PopAxisAlignedClip();

    // restore antialiasing mode
    pTarget->SetAntialiasMode(antialiasMode);

    // NOTE: selection requires caches for hit test calculations, 
    //       so avoid clearing cache during selection

    // reset caches to preserve memory
    if(!selectionActive())
    {
        // NOTE: do not reset caches, text analysis is slow and they do not take much memory
        //_resetRunCaches();
    }
}

/////////////////////////////////////////////////////////////////////
// Direct2D resource caching 
/////////////////////////////////////////////////////////////////////
void XD2DTextLayout::onInitD2DTarget(ID2D1RenderTarget* pTarget)
{
    // reset previous resources if any
    onResetD2DTarget();

    // init inline objects if any
    if(m_richText)
    {
        for(size_t objIdx = 0; objIdx < m_richText->inlineObjectCount(); ++objIdx)
            m_richText->getInlineObject(objIdx)->onInitD2DTarget(pTarget);
    }
}

void XD2DTextLayout::onResetD2DTarget()
{
    // reset inline objects if any
    if(m_richText)
    {
        for(size_t objIdx = 0; objIdx < m_richText->inlineObjectCount(); ++objIdx)
            m_richText->getInlineObject(objIdx)->onResetD2DTarget();
    }

    _releasePaintCache();
}

void  XD2DTextLayout::setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache)
{
    // release previous instance if any
    if(m_pXD2DResourcesCache)
    {
        // reset previous resources
        onResetD2DTarget();

        // release cache
        m_pXD2DResourcesCache->Release();
    }

    // copy reference
    m_pXD2DResourcesCache = pXD2DResourcesCache;
    if(m_pXD2DResourcesCache) m_pXD2DResourcesCache->AddRef();

    // reset inline objects if any
    if(m_richText)
    {
        for(size_t objIdx = 0; objIdx < m_richText->inlineObjectCount(); ++objIdx)
            m_richText->getInlineObject(objIdx)->setD2DResourcesCache(pXD2DResourcesCache);
    }
}

/////////////////////////////////////////////////////////////////////
// region creation interface ( required by XTextLayoutBaseT)
/////////////////////////////////////////////////////////////////////
XRectRegion XD2DTextLayout::createRegionFromPoints(FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2)
{
    // convert from DIP's and pass to XWUIGraphics
    return XWUIGraphics::createRectRegionFromPoints(XD2DHelpers::dipsToPixelsX(x1), XD2DHelpers::dipsToPixelsY(y1), 
                           XD2DHelpers::dipsToPixelsX(x2), XD2DHelpers::dipsToPixelsY(y2));
}

/////////////////////////////////////////////////////////////////////
// layout building interface ( required by XTextLayoutBaseT)
/////////////////////////////////////////////////////////////////////
void XD2DTextLayout::analyseRichText(const XRichText* richText, const XTextRange& range, std::vector<XDWriteHelpers::XDwTextRun>& runsOut)
{
    // process text into runs 
    XDWriteHelpers::analyseRichText(m_richText, range, runsOut);
}

void XD2DTextLayout::layoutTextRuns(std::vector<XDWriteHelpers::XDwTextRun>& textRuns)
{
    // re-order runs if needed
     XDWriteHelpers::layoutTextRuns(textRuns);
}

void XD2DTextLayout::shapeAndPostionTextRun(XDWriteHelpers::XDwTextRun& textRun, XDWriteHelpers::XDwTextRunCache& runCache)
{
    // shape text run
    if(!XDWriteHelpers::shapeText(m_richText, textRun, _getFontDataCache(textRun.style), runCache.shape, false))
    {
        // NOTE: fallback font in case some characters are not supported by selected font,
        //       must support as much scripts as possible

        // change to fallback font
        textRun.style.strFontName = XWUIStyle::fallbackFontName();

        // fallback
        XDWriteHelpers::shapeText(m_richText, textRun, _getFontDataCache(textRun.style), runCache.shape, true);
    }

    // position text run
    XDWriteHelpers::placeText(m_richText, textRun, _getFontDataCache(textRun.style), runCache.shape, runCache.place);

    // reset shape properties as they are not needed
    std::vector<DWRITE_SHAPING_TEXT_PROPERTIES> emptyTextProps;
    runCache.shape.textProps.swap(emptyTextProps);

    // NOTE: DirectWrite shapes glyphs in logical order and outputs them from right to left in DrawGlyphRun
    //       This is different from Uniscribe approach, so we rotate glyphs for RTL scipts to make them
    //       consistent and to simplify selection and hist testing
    if(textRun.isRTL)
    {
        // rotate 
        std::reverse(runCache.shape.glyphs.begin(), runCache.shape.glyphs.end());
        std::reverse(runCache.place.advances.begin(), runCache.place.advances.end());
        std::reverse(runCache.place.offsets.begin(), runCache.place.offsets.end());
    }

    // check if run is inline object
    if(textRun.isInlineObject)
    {
        // NOTE: inline object is just a space in text, so all normal functionality should work
        //       after glyph shaping we just update its width for layout to work

        XWASSERT(m_richText);
        if(m_richText == 0) return;

        // get inline object
        XTextInlineObject* inlineObject = m_richText->inlineObjectAt(textRun.range.pos);
        if(inlineObject)
        {
            FLOAT fontHeight, fontAscent;

            // get metrics for style
            getStyleMetrics(textRun.style, fontHeight, fontAscent);

            // shape inline object
            inlineObject->shapeContentD2D(fontHeight, fontAscent);

            // update width
            runCache.place.width = XD2DHelpers::pixelsToDipsX(inlineObject->width());

            // update width for glyph
            if(runCache.place.advances.size() == 1)
                runCache.place.advances.at(0) = runCache.place.width;
        }
    } 
}

void XD2DTextLayout::getStyleMetrics(const XTextStyle& style, FLOAT& fontHeight, FLOAT& fontAscent)
{
    // get style cache
    XDWriteHelpers::XDwFontData& fontCache = _getFontDataCache(style);

    // copy properties
    fontHeight = fontCache.fontHeight;
    fontAscent = fontCache.fontAscent;
}

void XD2DTextLayout::getInlineObjectMetrics(XDWriteHelpers::XDwTextRun& textRun, FLOAT& objectHeight, FLOAT& objectWidth)
{
    // check if run is inline object
    XWASSERT(textRun.isInlineObject);
    if(textRun.isInlineObject)
    {
        XWASSERT(m_richText);
        if(m_richText == 0) return;

        // get inline object
        XTextInlineObject* inlineObject = m_richText->inlineObjectAt(textRun.range.pos);
        if(inlineObject)
        {
            // get size
            objectWidth = XD2DHelpers::pixelsToDipsX(inlineObject->width());
            objectHeight = XD2DHelpers::pixelsToDipsY(inlineObject->height());
        }
    }
}

void XD2DTextLayout::getRunLogicalAttrs(const XDWriteHelpers::XDwTextRun& textRun, XDWriteHelpers::XDwTextRunCache& runCache)
{
    // generate logical attributes
    XDWriteHelpers::getRunLogicalAttrs(m_richText, textRun, runCache.logAttrs);
}

void XD2DTextLayout::mapGlyphsToChars(const XDWriteHelpers::XDwTextRun& textRun, XDWriteHelpers::XDwTextRunCache& runCache)
{
    // map glyphs
    XDWriteHelpers::mapGlyphsToChars(textRun, runCache.shape, runCache.glyphToChar);

    // NOTE: rotate result index if RTL script
    if(textRun.isRTL)
    {
        // rotate
        std::reverse(runCache.glyphToChar.begin(), runCache.glyphToChar.end());
    }
}

int XD2DTextLayout::getCharJustification(const XDWriteHelpers::XDwTextRunCache& runCache, unsigned int glyphIdx)
{
    // check range
    if(glyphIdx < runCache.shape.scriptAttrs.size())
    {
        // return justification class
        return runCache.shape.scriptAttrs.at(glyphIdx).justification;
    }

    return 0; // SCRIPT_JUSTIFY_NONE
}

/////////////////////////////////////////////////////////////////////
// layout helpers
/////////////////////////////////////////////////////////////////////
void XD2DTextLayout::_releaseFontCache()
{
    // release DW caches
    for(XDwFontCache::iterator it = m_fontCache.begin(); it != m_fontCache.end(); ++it)
    {
        // free cache
        XDWriteHelpers::releaseFontData(it->second);
    }

    // clear map
    m_fontCache.clear();
}

/////////////////////////////////////////////////////////////////////
// paint helpers
/////////////////////////////////////////////////////////////////////
void XD2DTextLayout::_releasePaintCache()
{
    // release brushes
    XD2DHelpers::resetColorBrushCache(m_brushCache);
}

FLOAT XD2DTextLayout::_paintLayoutLine(ID2D1RenderTarget* pTarget, FLOAT originX, FLOAT originY, const D2D1_RECT_F& rcPaint, 
                             XTextParagraph& textParagraph, XLayoutLine& layoutLine)
{
    // check if we have run caches already
    if(textParagraph.runCaches.size() == 0)
    {
        // generate shape and position for runs
        _shapeAndPostionRuns(textParagraph.textRuns, textParagraph.runCaches);
    }

    // update paint runs for line if needed
    if(layoutLine.paintRuns.size() == 0)
    {
        // generate paint runs for layout line
        _updateLinePaintRuns(textParagraph, layoutLine, m_layoutWidth);
    }

    // line height
    FLOAT lineHeight = _getLineHeight(layoutLine);

    // init paint rect
    D2D1_RECT_F paintRect;
    paintRect.left = originX;
    paintRect.right = paintRect.left;
    paintRect.top = originY;
    paintRect.bottom = originY + lineHeight;

    // loop over all paint runs
    for(unsigned int paintRunIdx = 0; paintRunIdx < layoutLine.paintRuns.size() && paintRect.right < rcPaint.right; ++paintRunIdx)
    {
        // paint run
        const XTextPaintRun& paintRun = layoutLine.paintRuns.at(paintRunIdx);

        // init paint rect
        paintRect.right = paintRect.left + paintRun.width;

        // paint only visible runs
        if(paintRect.right <= rcPaint.left) 
        {
            // update rect
            paintRect.left = paintRect.right;

            // next
            continue;
        }

        // text properties
        const XDWriteHelpers::XDwTextRun& textRun = textParagraph.textRuns.at(paintRun.runIdx);
        const XDWriteHelpers::XDwTextRunCache& runCache = textParagraph.runCaches.at(paintRun.runIdx);

        // fill background
        if(paintRun.fillBackground)
        {
            D2D1_RECT_F fillRect;
            fillRect.left = paintRect.left;
            fillRect.right = paintRect.left + paintRun.width;
            fillRect.top = paintRect.top;
            fillRect.bottom = paintRect.top + lineHeight;

            _fillRect(pTarget, fillRect, paintRun.backgroundColor);
        }

        // skip runs without visuals
        if(textRun.scriptProps.shapes == DWRITE_SCRIPT_SHAPES_NO_VISUAL)
        {
            XWASSERT(runCache.place.width == 0);

            // just skip to next as this run must have width 0
            continue;
        }

        // check if paint run is inline object
        if(!textRun.isInlineObject)
        {
            // get run style cache
            XDWriteHelpers::XDwFontData& fontData = _getFontDataCache(textRun.style);

            // paint origin
            D2D1_POINT_2F paintOrigin;
            paintOrigin.x = paintRect.left;
            paintOrigin.y = paintRect.top + layoutLine.maxAscent + m_linePaddingBefore;

            // format glyph run
            DWRITE_GLYPH_RUN glyphRun;
            glyphRun.fontFace = fontData.fontFace;
            glyphRun.fontEmSize = fontData.fontEmSize;
            glyphRun.glyphCount = paintRun.glyphCount;
            glyphRun.glyphIndices = runCache.shape.glyphs.data() + paintRun.glyphOffset;
            glyphRun.glyphAdvances = runCache.place.advances.data() + paintRun.glyphOffset;
            glyphRun.glyphOffsets = runCache.place.offsets.data() + paintRun.glyphOffset;
            glyphRun.isSideways = FALSE;

            // NOTE: we ignore bidi level here as RTL scripts should have been shaped in visual order
            glyphRun.bidiLevel = 0; 

            // check if run has been justified
            if(layoutLine.justify && paintRun.justifyPtr)
            {
                // set justified advances instead
                glyphRun.glyphAdvances = paintRun.justifyPtr;
            }

            // get text brush
            ID2D1Brush* textBrush = _getColorBrush(pTarget, paintRun.textColor);
            if(textBrush == 0)
            {
                XWASSERT(false);
                continue;
            }

            // paint text
            pTarget->DrawGlyphRun(paintOrigin, &glyphRun, textBrush);

            // draw underline if needed
            if(textRun.style.bUnderline)
            {
                // NOTE: Underline position is the position of underline relative to the English baseline.

                // underline rect
                D2D1_RECT_F underlineRect;
                underlineRect.left = paintOrigin.x;
                underlineRect.top = paintOrigin.y - fontData.underlinePosition;
                underlineRect.right = paintOrigin.x + paintRun.width;
                underlineRect.bottom = underlineRect.top + fontData.underlineThickness;
                
                // draw this as a rectangle
                pTarget->FillRectangle(&underlineRect, textBrush);
            }

            // draw strikethrough if needed
            if(textRun.style.bStrike)
            {
                // NOTE: Strikethrough position is the position of strikethrough relative to the English baseline.

                // underline rect
                D2D1_RECT_F strikeRect;
                strikeRect.left = paintOrigin.x;
                strikeRect.top = paintOrigin.y - fontData.strikethroughPosition;
                strikeRect.right = paintOrigin.x + paintRun.width;
                strikeRect.bottom = strikeRect.top + fontData.strikethroughThickness;
                
                // draw this as a rectangle
                pTarget->FillRectangle(&strikeRect, textBrush);
            }

        } else
        {
            // validate assumptions
            XWASSERT(paintRun.glyphCount == 1);

            // get inline object
            XTextInlineObject* inlineObject = m_richText->inlineObjectAt(textRun.range.pos);
            XWASSERT(inlineObject);

            // paint inline object
            if(inlineObject)
            {
                FLOAT objectWidth = XD2DHelpers::pixelsToDipsX(inlineObject->width());
                FLOAT objectHeight = XD2DHelpers::pixelsToDipsX(inlineObject->height());

                // horizontal position 
                FLOAT horizontalOffset = 0;
                if(paintRun.justifyPtr && paintRun.justifyPtr[0] > objectWidth)
                {
                    horizontalOffset = (objectWidth - paintRun.justifyPtr[0]) / 2;
                }

                // vertical position
                FLOAT verticalOffset = 0;
                if(layoutLine.height > objectHeight)
                {
                    // update offset
                    verticalOffset = (layoutLine.height - objectHeight) / 2;
                }

                // set object origin
                inlineObject->setOrigin(XD2DHelpers::dipsToPixelsX(paintRect.left + horizontalOffset), 
                                        XD2DHelpers::dipsToPixelsY(paintRect.top + verticalOffset));

                // paint
                inlineObject->onPaintD2D(paintRect.left + horizontalOffset, paintRect.top + verticalOffset, pTarget);
            }
        }

        // update rect
        paintRect.left = paintRect.right;
    }

    // return used width
    return paintRect.right;
}

void XD2DTextLayout::_fillRect(ID2D1RenderTarget* pTarget, const D2D1_RECT_F& fillRect, const COLORREF& color)
{
    // get brush
    ID2D1Brush* d2dBrush = _getColorBrush(pTarget, color);

    // fill 
    if(d2dBrush)
    {
        pTarget->FillRectangle(fillRect, d2dBrush);
    }
}

/////////////////////////////////////////////////////////////////////
// style cache helpers
/////////////////////////////////////////////////////////////////////
XDWriteHelpers::XDwFontData& XD2DTextLayout::_getFontDataCache(const XTextStyle& style)
{
    XWASSERT(m_richText);

    // NOTE: for inline objects style will be always default, 
    //       so same cache will be used for all

    // get hash from style
    xstyle_index_t styleIndex = m_richText->hashFromTextStyle(style);

    // check if we have this style already in cache
    XDwFontCache::iterator it = m_fontCache.find(styleIndex);
    if(it == m_fontCache.end())
    {
        // add emty
        XDWriteHelpers::XDwFontData fontData;

        // reset font data
        ::ZeroMemory(&fontData, sizeof(fontData));

        // add
        it = m_fontCache.insert(XDwFontCache::value_type(styleIndex, fontData)).first;
    }

    // return reference from cache
    return it->second;
}

ID2D1Brush* XD2DTextLayout::_getColorBrush(ID2D1RenderTarget* pTarget, const COLORREF& brushColor)
{
    // convert colors
    D2D1_COLOR_F d2dBrushColor;
    XD2DHelpers::colorrefToD2dColor(brushColor, d2dBrushColor);

    // check if we have this brush already in cache
    ID2D1Brush* brush = XD2DHelpers::findColorBrush(m_brushCache, d2dBrushColor);
    
    // return if found
    if(brush != 0) return brush;

    // try to use shared resource cache if set
    if(m_pXD2DResourcesCache)
    {
        D2D1_COLOR_F d2dColor;
        XD2DHelpers::colorrefToD2dColor(brushColor, d2dColor);

        // get brush from shared cache
        brush = m_pXD2DResourcesCache->getBrush(d2dColor);

    } else
    {
        ID2D1SolidColorBrush* colorBrush = 0;

        // create brush
        HRESULT hr = pTarget->CreateSolidColorBrush(d2dBrushColor, &colorBrush);
        if(FAILED(hr))
        {
            XWTRACE_HRES("XD2DTextLayout::_getColorBrush failed to create brush", hr);
            return 0;
        }

        brush = colorBrush;
    }

    // add to internal cache
    XD2DHelpers::appendColorBrush(m_brushCache, brush, d2dBrushColor);

    // return new brush
    return brush;
}

// XD2DTextLayout
/////////////////////////////////////////////////////////////////////

