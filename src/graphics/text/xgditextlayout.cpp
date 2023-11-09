// Uniscribe based text layout and rendering
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"
#include "../xwgraphicshelpers.h"
#include "../xgdiresourcescache.h"

#include "xgdifonts.h"
#include "xtextinlineobject.h"
#include "xrichtext.h"
#include "xuniscribehelpers.h"
#include "xgditextlayout.h"

/////////////////////////////////////////////////////////////////////
// constants
#define XGDITEXTLAYOUT_MASK_COLOR       RGB(121, 121, 121)

/////////////////////////////////////////////////////////////////////
// XGdiTextLayout - Uniscribe based text layout

XGdiTextLayout::XGdiTextLayout() :
    m_hdc(0),
    m_pXGdiResourcesCache(0),
    m_enableDoubleBuffer(false) 
{
}

XGdiTextLayout::~XGdiTextLayout()
{
    // release resources
    onResetGDIResources();

    // release shared cache if any
    if(m_pXGdiResourcesCache) m_pXGdiResourcesCache->Release();
    m_pXGdiResourcesCache = 0;

    // release font caches
    _releaseFontCache();
}

/////////////////////////////////////////////////////////////////////
// layout size
/////////////////////////////////////////////////////////////////////
int XGdiTextLayout::contentWidth(HDC hdc) 
{
    // copy HDC reference
    m_hdc = hdc;

    // compute width
    int retWidth = XTextLayoutBaseT::contentWidth();

    // reset HDC
    m_hdc = 0;

    // return width
    return retWidth;
}

int XGdiTextLayout::contentHeight(HDC hdc) 
{
    // copy HDC reference
    m_hdc = hdc;

    // compute height
    int retHeight = XTextLayoutBaseT::contentHeight();

    // reset HDC
    m_hdc = 0;

    // return height
    return retHeight;
}

/////////////////////////////////////////////////////////////////////
// paint properties
/////////////////////////////////////////////////////////////////////
void XGdiTextLayout::enableDoubleBuffering(bool bEnable)
{
    // copy flag
    m_enableDoubleBuffer = bEnable;

    // check if background fill is enabled
    if(m_enableDoubleBuffer && !m_bFillBackground)
    {
        // NOTE: as we are using back buffer bitmap to paint each line
        //       making colors transparent is tricky as it would require
        //       using TransparentBlt with some choosen color to treat as
        //       transparent. We rather assume that XGdiTextLayout user 
        //       will handle it if needed (e.g. not use double buffering if
        //       background fill is not needed)

        XWTRACE("XGdiTextLayout: note that if double buffering is enabled background will be always filled");
    }
}

/////////////////////////////////////////////////////////////////////
// size hints
/////////////////////////////////////////////////////////////////////
int XGdiTextLayout::getHeightForWidth(HDC hdc, int width)
{
    // copy HDC reference
    m_hdc = hdc;

    // compute height
    int retHeight = XTextLayoutBaseT::getHeightForWidth(width);

    // reset HDC
    m_hdc = 0;

    // return result
    return retHeight;
}

int XGdiTextLayout::getMaxTextForWidth(HDC hdc, int width, int lineIndex)
{
    // return zero if no text
    if(m_richText == 0 || m_richText->textLength() == 0) return 0;

    // TODO: compute maximum text length that would fit for line
    // - this might be needed for single line layout to insert/append ... into long text
    return 0;
}

/////////////////////////////////////////////////////////////////////
// line properties
/////////////////////////////////////////////////////////////////////
int XGdiTextLayout::getLineCount(HDC hdc)
{
    // copy HDC reference
    m_hdc = hdc;

    // compute line count
    int retCount = XTextLayoutBaseT::getLineCount();

    // reset HDC
    m_hdc = 0;

    // return result
    return retCount;
}

bool XGdiTextLayout::getLineMetrics(HDC hdc, int lineIdx, int& textBegin, int& textEnd, int& lineHeight)
{
    // copy HDC reference
    m_hdc = hdc;

    // compute line metrics
    bool retVal = XTextLayoutBaseT::getLineMetrics(lineIdx, textBegin, textEnd, lineHeight);

    // reset HDC
    m_hdc = 0;

    return retVal;
}

bool XGdiTextLayout::getLineFitPos(HDC hdc, int lineIdx, int maxLineWidth, int& textPos)
{
    // copy HDC reference
    m_hdc = hdc;

    // compute line metrics
    bool retVal = XTextLayoutBaseT::getLineFitPos(lineIdx, maxLineWidth, textPos);

    // reset HDC
    m_hdc = 0;

    return retVal;
}

/////////////////////////////////////////////////////////////////////
// hit testing
/////////////////////////////////////////////////////////////////////
bool XGdiTextLayout::isInsideText(HDC hdc, int originX, int originY, int posX, int posY)
{
    // copy HDC reference
    m_hdc = hdc;

    // do hit testing
    bool bRetVal = XTextLayoutBaseT::isInsideText(originX, originY, posX, posY);

    // reset HDC
    m_hdc = 0;

    // return result
    return bRetVal;
}

bool XGdiTextLayout::isInsideSelection(HDC hdc, int originX, int originY, int posX, int posY)
{
    // copy HDC reference
    m_hdc = hdc;

    // do hit testing
    bool bRetVal = XTextLayoutBaseT::isInsideSelection(originX, originY, posX, posY);

    // reset HDC
    m_hdc = 0;

    // return result
    return bRetVal;
}

bool XGdiTextLayout::getTextFromPos(HDC hdc, int originX, int originY, int posX, int posY, unsigned int& textPos)
{
    // copy HDC reference
    m_hdc = hdc;

    // do hit testing
    bool bRetVal = XTextLayoutBaseT::getTextFromPos(originX, originY, posX, posY, textPos);

    // reset HDC
    m_hdc = 0;

    // return result
    return bRetVal;
}

/////////////////////////////////////////////////////////////////////
// regions
/////////////////////////////////////////////////////////////////////
XRectRegion XGdiTextLayout::getTextRegion(HDC hdc, int originX, int originY, int textPos, int textLength)
{
    // copy HDC reference
    m_hdc = hdc;

    // compute region
    XRectRegion textRegion = XTextLayoutBaseT::getTextRegion(originX, originY, textPos, textLength);

    // reset HDC
    m_hdc = 0;

    // return result
    return textRegion;
}

/////////////////////////////////////////////////////////////////////
// selection
/////////////////////////////////////////////////////////////////////
bool XGdiTextLayout::selectTo(HDC hdc, int originX, int originY, int selectFomX, int selectFromY, int selectToX, int selectToY)
{
    // set HDC reference
    m_hdc = hdc;

    // update selection range from points
    bool retVal = XTextLayoutBaseT::selectTo(originX, originY, selectFomX, selectFromY, selectToX, selectToY);

    // reset HDC reference
    m_hdc = 0;

    // return result
    return retVal;
}

/////////////////////////////////////////////////////////////////////
// painting
/////////////////////////////////////////////////////////////////////
void XGdiTextLayout::onPaint(HDC hdc, int originX, int originY, const RECT& rcPaint)
{
    XWASSERT(hdc);
    if(hdc == 0) return;

    // ignore if no text set
    if(m_richText == 0 || m_richText->textLength() == 0) 
    {
        // just clear background if needed
        if(m_bFillBackground)
        {
            // store old color
            COLORREF oldcr = ::SetBkColor(hdc, m_clBackground);

            // use text out as fastest way to fill background
            ::ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &rcPaint, L"", 0, 0);

            // set color back
            ::SetBkColor(hdc, oldcr);
        }

        return;
    }

    // set HDC reference
    m_hdc = hdc;

    // update layout if needed
    _updateLayoutIfNeeded();

    // clip drawing region
    HRGN clipRgn = ::CreateRectRgn(rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom);
    ::SelectClipRgn(hdc, clipRgn);

    // store old DC state
    int nSavedDC = ::SaveDC(hdc);

    // init
    HDC doubleBufferDC = 0;
    HBITMAP bufferBitmap = 0;
    RECT lineRect;

    // use line double buffering if enabled
    if(m_enableDoubleBuffer)
    {
        // init double buffer line rect
        lineRect.left = 0;
        lineRect.right = m_layoutWidth;
        lineRect.top = 0;

        // create in-memory DC
        doubleBufferDC = ::CreateCompatibleDC(hdc);

        // create bitmap buffer
        bufferBitmap = ::CreateCompatibleBitmap(hdc, m_layoutWidth, _maxLineHeight());

        // activate 
        ::SelectObject(doubleBufferDC, bufferBitmap);
    }

    // paint position
    int paintPosX = originX;
    int paintPosY = originY;

    // loop over text paragraphs
    for(unsigned int paraIdx = 0; paraIdx < m_textLayout.size() && paintPosY < rcPaint.bottom; ++paraIdx)
    {
        // active paragraph
        XTextParagraph& textParagraph = m_textLayout.at(paraIdx);

        // loop over layout lines
        for(unsigned int lineIdx = 0; lineIdx < textParagraph.layoutLines.size(); ++lineIdx)
        {
            // active text line
            XLayoutLine& layoutLine = textParagraph.layoutLines.at(lineIdx);

            int lineHeight, paintRight;

            // line metrics
            _getLineMetrics(originX, originY, layoutLine, paintPosX, paintRight, lineHeight, textParagraph.isRTL);

            // paint only visible lines
            if(paintPosY + lineHeight <= rcPaint.top) 
            {
                // update paint position
                paintPosY += lineHeight;

                // next
                continue;
            }

            // fill space before text if needed
            if(m_bFillBackground && paintPosX > rcPaint.left && !m_enableDoubleBuffer)
            {
                // free line space
                _fillBackground(hdc, rcPaint.left, paintPosX, paintPosY, paintPosY + lineHeight);
            }

            // check if double buffering is needed
            if(m_enableDoubleBuffer)
            {
                // update only bottom
                lineRect.bottom = lineHeight;

                // clear double buffer image with background color first
                ::SetBkColor(doubleBufferDC, m_clBackground);
                ::ExtTextOutW(doubleBufferDC, 0, 0, ETO_OPAQUE, &lineRect, L"", 0, 0);

                // paint line to buffer
                if(originX < 0)
                    _paintLayoutLine(doubleBufferDC, originX, 0, lineRect, textParagraph, layoutLine);
                else
                    _paintLayoutLine(doubleBufferDC, 0, 0, lineRect, textParagraph, layoutLine);

                // update inline object origin
                _updateObjectPositions(paintPosX - (originX < 0) ? originX : 0, paintPosY, textParagraph, layoutLine);

            } else
            {
                // paint line normally
                paintRight = _paintLayoutLine(hdc, paintPosX, paintPosY, rcPaint, textParagraph, layoutLine);
            }

            // fill the rest
            if(m_bFillBackground && paintRight < rcPaint.right && !m_enableDoubleBuffer)
            {
                // remaining line space
                _fillBackground(hdc, paintRight, rcPaint.right, paintPosY, paintPosY + lineHeight);
            }

            // double buffering
            if(m_enableDoubleBuffer)
            {
                // copy line buffer at once
                if(originX < 0)
                    ::BitBlt(hdc, 0, paintPosY, m_layoutWidth, lineHeight, doubleBufferDC, 0, 0, SRCCOPY);
                else
                    ::BitBlt(hdc, originX, paintPosY, m_layoutWidth, lineHeight, doubleBufferDC, 0, 0, SRCCOPY);
            }

            // update paint position
            paintPosY += lineHeight;
        }
    }

    // release double buffering resources
    if(m_enableDoubleBuffer)
    {
        ::SelectObject(doubleBufferDC, 0);
        ::DeleteObject(bufferBitmap);
        ::DeleteDC(doubleBufferDC);    
    }

    // fill the rest
    if(m_bFillBackground && paintPosY < rcPaint.bottom)
    {
        // remaining space
        _fillBackground(hdc, rcPaint.left, rcPaint.right, paintPosY, rcPaint.bottom);
    }

    // restore DC state
    ::RestoreDC(hdc, nSavedDC);

    // reset clip region
    ::SelectClipRgn(hdc, 0);
    ::DeleteObject(clipRgn);

    // reset HDC reference
    m_hdc = 0;

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
// GDI resource caching
/////////////////////////////////////////////////////////////////////
void XGdiTextLayout::onInitGDIResources(HDC hdc)
{
    // release previous resources if any
    onResetGDIResources();

    // init resources for inline objects
    if(m_richText)
    {
        for(size_t objIdx = 0; objIdx < m_richText->inlineObjectCount(); ++objIdx)
            m_richText->getInlineObject(objIdx)->onInitGDIResources(hdc);
    }
}

void XGdiTextLayout::onResetGDIResources()
{
    // reset resources for inline objects
    if(m_richText)
    {
        for(size_t objIdx = 0; objIdx < m_richText->inlineObjectCount(); ++objIdx)
            m_richText->getInlineObject(objIdx)->onResetGDIResources();
    }
}

void XGdiTextLayout::setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache)
{
    // release previous instance if any
    if(m_pXGdiResourcesCache)
    {
        // reset resources as they might be using cache
        onResetGDIResources();

        // release
        m_pXGdiResourcesCache->Release();
    }

    // copy reference
    m_pXGdiResourcesCache = pXGdiResourcesCache;
    if(m_pXGdiResourcesCache) m_pXGdiResourcesCache->AddRef();

    // set cache for inline objects
    if(m_richText)
    {
        for(size_t objIdx = 0; objIdx < m_richText->inlineObjectCount(); ++objIdx)
            m_richText->getInlineObject(objIdx)->setGDIResourcesCache(pXGdiResourcesCache);
    }
}

/////////////////////////////////////////////////////////////////////
// region creation interface ( required by XTextLayoutBaseT)
/////////////////////////////////////////////////////////////////////
XRectRegion XGdiTextLayout::createRegionFromPoints(int x1, int y1, int x2, int y2)
{
    // pass to XWUIGraphics
    return XWUIGraphics::createRectRegionFromPoints(x1, y1, x2, y2);
}

/////////////////////////////////////////////////////////////////////
// layout building interface ( required by XTextLayoutBaseT)
/////////////////////////////////////////////////////////////////////
void XGdiTextLayout::analyseRichText(const XRichText* richText, const XTextRange& range, std::vector<XUniscribeHelpers::XUniTextRun>& runsOut)
{
    // process text into runs 
    XUniscribeHelpers::analyseRichText(m_richText, range, runsOut);
}

void XGdiTextLayout::layoutTextRuns(std::vector<XUniscribeHelpers::XUniTextRun>& textRuns)
{
    // re-order runs if needed
    XUniscribeHelpers::layoutTextRuns(textRuns);
}

void XGdiTextLayout::shapeAndPostionTextRun(XUniscribeHelpers::XUniTextRun& textRun, 
                                   XUniscribeHelpers::XUniTextRunCache& runCache)
{
    XWASSERT(m_hdc);
    if(m_hdc == 0) return;

    // shape text run
    if(!XUniscribeHelpers::shapeText(m_hdc, m_richText, textRun, _getFontDataCache(textRun.style), runCache.shape, false))
    {
        // NOTE: fallback font in case some characters are not supported by selected font,
        //       must support as much scripts as possible

        // change to fallback font
        textRun.style.strFontName = XWUIStyle::fallbackFontName();

        // fallback
        XUniscribeHelpers::shapeText(m_hdc, m_richText, textRun, _getFontDataCache(textRun.style), runCache.shape, true);
    }

    // position text run
    XUniscribeHelpers::placeText(m_hdc, runCache.shape, textRun, _getFontDataCache(textRun.style), runCache.place);

    // check if run is inline object
    if(textRun.isInlineObject)
    {
        XWASSERT(m_richText);
        if(m_richText == 0) return;

        // NOTE: inline object is just a space in text, so all normal functionality should work
        //       after glyph shaping we just update its width for layout to work

        // get inline object
        XTextInlineObject* inlineObject = m_richText->inlineObjectAt(textRun.range.pos);
        if(inlineObject)
        {
            int fontHeight, fontAscent;

            // get metrics for style
            getStyleMetrics(textRun.style, fontHeight, fontAscent);

            // shape inline object
            inlineObject->shapeContentGDI(m_hdc, fontHeight, fontAscent);

            // update width
            runCache.place.width = inlineObject->width();

            // update width for glyph
            if(runCache.place.advances.size() == 1)
                runCache.place.advances.at(0) = runCache.place.width;
        }
    }
}

void XGdiTextLayout::getStyleMetrics(const XTextStyle& style, int& fontHeight, int& fontAscent)
{
    // get font cache from style
    XUniscribeHelpers::XUniFontData& fontData = _getFontDataCache(style);
    
    // copy metrics
    fontHeight = fontData.fontHeight;
    fontAscent = fontData.fontAscent;
}

void XGdiTextLayout::getInlineObjectMetrics(XUniscribeHelpers::XUniTextRun& textRun, int& objectHeight, int& objectWidth)
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
            objectWidth = inlineObject->width();
            objectHeight = inlineObject->height();
        }
    }
}

void XGdiTextLayout::getRunLogicalAttrs(const XUniscribeHelpers::XUniTextRun& textRun, 
                               XUniscribeHelpers::XUniTextRunCache& runCache)
{
    // generate logical attributes
    XUniscribeHelpers::getRunLogicalAttrs(m_richText, textRun, runCache.logAttrs);
}

void XGdiTextLayout::mapGlyphsToChars(const XUniscribeHelpers::XUniTextRun& textRun, 
                             XUniscribeHelpers::XUniTextRunCache& runCache)
{
    // map glyphs
    XUniscribeHelpers::mapGlyphsToChars(textRun, runCache.shape, runCache.glyphToChar);
}

void XGdiTextLayout::justifyLayoutLine(XTextParagraph& textParagraph, XLayoutLine& layoutLine, int lineWidth)
{
    // NOTE: this is not in use at the moment, justification is done in generic way in XTextLayoutBaseT

    // do nothing if width is same or bigger
    if(layoutLine.width >= lineWidth) return;

    // compute difference to justify
    int justifyWidth = lineWidth - layoutLine.width;

    std::vector<int>            lineGlyphAdvances;
    std::vector<SCRIPT_VISATTR> lineScriptAttrs;

    // collect all glyphs first
    for(unsigned int runIdx = layoutLine.begin.runIdx; runIdx <= layoutLine.end.runIdx; ++runIdx)
    {
        // get run data
        const XUniscribeHelpers::XUniTextRunCache& runCache = textParagraph.runCaches.at(runIdx);

        // offsets
        unsigned int runStartOffset = (runIdx == layoutLine.begin.runIdx) ? layoutLine.begin.runOffset : 0;
        unsigned int runStopOffset = (runIdx == layoutLine.end.runIdx) ? layoutLine.end.runOffset : (int)runCache.shape.glyphs.size();

        // append glyphs and attributes
        lineGlyphAdvances.insert(lineGlyphAdvances.end(), runCache.place.advances.begin() + runStartOffset, runCache.place.advances.begin() + runStopOffset);

        // TODO:: this is wrong as we are looping on glyphs but logical attrs are on characters, may crash on complex scripts!!!
        lineScriptAttrs.insert(lineScriptAttrs.end(), runCache.shape.scriptAttrs.begin() + runStartOffset, runCache.shape.scriptAttrs.begin() + runStopOffset);
    }

    // reserve enough space for advances
    layoutLine.justifyAdvances.resize(lineGlyphAdvances.size());

    // justify whole line
    HRESULT hr = ::ScriptJustify(lineScriptAttrs.data(), lineGlyphAdvances.data(),
        (int)lineGlyphAdvances.size(), justifyWidth, 2, layoutLine.justifyAdvances.data());

    // check for errors
    if(FAILED(hr))
    {
        XWTRACE_HRES("XGdiTextLayout::justifyLayoutLine failed to justify text run", hr);

        // reset justification
        layoutLine.justifyAdvances.clear();
        layoutLine.justify = false;
    }
}

int XGdiTextLayout::getCharJustification(const XUniscribeHelpers::XUniTextRunCache& runCache, unsigned int glyphIdx)
{
    // check range
    if(glyphIdx < runCache.shape.scriptAttrs.size())
    {
        // return justification class
        return runCache.shape.scriptAttrs.at(glyphIdx).uJustification;
    }

    return SCRIPT_JUSTIFY_NONE;
}

/////////////////////////////////////////////////////////////////////
// layout helpers
/////////////////////////////////////////////////////////////////////
void XGdiTextLayout::_releaseFontCache()
{
    // release Uniscribe caches
    for(XUniFontCache::iterator it = m_fontCache.begin(); it != m_fontCache.end(); ++it)
    {
        // free cache
        XUniscribeHelpers::releaseCache(it->second);
    }

    // clear map
    m_fontCache.clear();
}

/////////////////////////////////////////////////////////////////////
// paint helpers
/////////////////////////////////////////////////////////////////////
int XGdiTextLayout::_paintLayoutLine(HDC hdc, int originX, int originY, const RECT& rcPaint, 
                                     XTextParagraph& textParagraph, XLayoutLine& layoutLine)
{
    XWASSERT(m_richText);

    // line height
    int lineHeight = _getLineHeight(layoutLine);

    // init paint rect
    RECT paintRect;
    paintRect.left = originX;
    paintRect.right = paintRect.left;
    paintRect.top = originY;
    paintRect.bottom = originY + lineHeight;

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

        // check paint limits
        if(paintRect.right > rcPaint.right)
        {
            paintRect.right = rcPaint.right;
        }

        // text properties
        const XUniscribeHelpers::XUniTextRun& textRun = textParagraph.textRuns.at(paintRun.runIdx);
        const XUniscribeHelpers::XUniTextRunCache& runCache = textParagraph.runCaches.at(paintRun.runIdx);

        // get font cache from style
        XUniscribeHelpers::XUniFontData& fontData = _getFontDataCache(textRun.style);

        // update ascent
        int ascentOffset = layoutLine.maxAscent - fontData.fontAscent + m_linePaddingBefore;

        // set font
        ::SelectObject(hdc, fontData.hFont);

        // set colors
        ::SetTextColor(hdc, paintRun.textColor);

        // check if we need to fill background
        if(paintRun.fillBackground)
        {
            ::SetBkColor(hdc, paintRun.backgroundColor);
            ::SetBkMode(hdc, OPAQUE);
        } else
        {
            ::SetBkMode(hdc, TRANSPARENT);
        }

        // paint text
        HRESULT hr = ::ScriptTextOut(hdc, &fontData.scriptCache, paintRect.left, paintRect.top + ascentOffset, 
            paintRun.fillBackground ? (ETO_OPAQUE | ETO_CLIPPED) : (ETO_CLIPPED), &paintRect, 
            &textRun.scriptProps, 0, 0, 
            runCache.shape.glyphs.data() + paintRun.glyphOffset, paintRun.glyphCount,
            runCache.place.advances.data() + paintRun.glyphOffset, 
            paintRun.justifyPtr, 
            runCache.place.offsets.data() + paintRun.glyphOffset);

        // check result
        if(FAILED(hr))
        {
            XWTRACE_HRES("XGdiTextLayout::_paintLayoutLine failed to paint run", hr);

            // just fill background if needed
            if(paintRun.fillBackground)
            {
                ::ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &paintRect, L"", 0, 0);
            };
        }

        // check if paint run is inline object
        if(textRun.isInlineObject)
        {
            // NOTE: we let ScriptTextOut to erase background first and paint inline object on top

            // validate assumptions
            XWASSERT(paintRun.glyphCount == 1);

            // get inline object
            XTextInlineObject* inlineObject = m_richText->inlineObjectAt(textRun.range.pos);
            XWASSERT(inlineObject);

            // paint inline object
            if(inlineObject)
            {
                // horizontal position 
                int horizontalOffset = 0;
                if(paintRun.justifyPtr && paintRun.justifyPtr[0] > inlineObject->width())
                {
                    horizontalOffset = (inlineObject->width() - paintRun.justifyPtr[0]) / 2;
                }

                // vertical position
                int verticalOffset = 0;
                if(layoutLine.height > inlineObject->height())
                {
                    // update offset
                    verticalOffset = (layoutLine.height - inlineObject->height()) / 2;
                }

                // set object origin
                inlineObject->setOrigin(paintRect.left + horizontalOffset, paintRect.top + verticalOffset);

                // paint
                inlineObject->onPaintGDI(paintRect.left + horizontalOffset, paintRect.top + verticalOffset, hdc);
            }
        }

        // update rect
        paintRect.left = paintRect.right;
    }

    // return used width
    return paintRect.right;
}

void XGdiTextLayout::_updateObjectPositions(int offsetX, int offsetY, XTextParagraph& textParagraph, XLayoutLine& layoutLine)
{
    // loop over all paint runs
    for(unsigned int paintRunIdx = 0; paintRunIdx < layoutLine.paintRuns.size(); ++paintRunIdx)
    {
        // paint run
        const XTextPaintRun& paintRun = layoutLine.paintRuns.at(paintRunIdx);

        // text run
        const XUniscribeHelpers::XUniTextRun& textRun = textParagraph.textRuns.at(paintRun.runIdx);

        // check if run is inline object
        if(textRun.isInlineObject)
        {
            // get inline object
            XTextInlineObject* inlineObject = m_richText->inlineObjectAt(textRun.range.pos);
            XWASSERT(inlineObject);

            // update inline object
            if(inlineObject)
                inlineObject->moveOrigin(offsetX, offsetY);
        }
    }
}

void XGdiTextLayout::_fillBackground(HDC hdc, const RECT& rcPaint)
{
    // ignore if not needed
    if(m_bFillBackground)
    {
        // select default color
        ::SetBkColor(hdc, m_clBackground);

        // render empty text block
        ::ExtTextOutW(hdc, rcPaint.left, rcPaint.top, ETO_OPAQUE | ETO_CLIPPED, &rcPaint, L"", 0, 0);
    }
}

void XGdiTextLayout::_fillBackground(HDC hdc, int left, int right, int top, int bottom)
{
    // init rect
    RECT rcPaint;
    rcPaint.left = left;
    rcPaint.right = right;
    rcPaint.top = top;
    rcPaint.bottom = bottom;

    // fill
    _fillBackground(hdc, rcPaint);
}

/////////////////////////////////////////////////////////////////////
// style cache helpers
/////////////////////////////////////////////////////////////////////
XUniscribeHelpers::XUniFontData& XGdiTextLayout::_getFontDataCache(const XTextStyle& style)
{
    XWASSERT(m_richText);

    // NOTE: for inline objects style will be always default, 
    //       so same cache will be used for all

    // get hash from style
    xstyle_index_t styleIndex = m_richText->hashFromTextStyle(style);

    // check if we have this style already in cache
    XUniFontCache::iterator it = m_fontCache.find(styleIndex);
    if(it == m_fontCache.end())
    {
        // add emty
        XUniscribeHelpers::XUniFontData fontData;

        // reset font data
        ::ZeroMemory(&fontData, sizeof(fontData));

        // add
        it = m_fontCache.insert(XUniFontCache::value_type(styleIndex, fontData)).first;
    }

    // return reference from cache
    return it->second;
}

// XGdiTextLayout
/////////////////////////////////////////////////////////////////////


