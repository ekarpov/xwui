// Template methods for text layout
//
/////////////////////////////////////////////////////////////////////

#ifndef _XTEXTLAYOUTBASE_H_
#define _XTEXTLAYOUTBASE_H_

/////////////////////////////////////////////////////////////////////
// XTextLayoutBaseT - text layout generic methods

template<typename _XNum, typename _XTextRun, typename _XTextRunCache, typename _XLayoutType> class XTextLayoutBaseT
{
public: // construction/destruction
    XTextLayoutBaseT() :
        m_richText(0),
        m_layoutWidth((_XNum)INT_MAX),
        m_linePaddingBefore(0),
        m_linePaddingAfter(0),
        m_wordWrap(false),
        m_singleLineMode(false),
        m_textAlignment(eTextAlignLeft),
        m_hasSelection(false),
        m_selectionActive(false),
        m_selectionStyleChanged(false),
        m_selectionStartGlyph(0),
        m_selectionEndGlyph(0),
        m_bFillBackground(false),
        m_clText(RGB(0, 0, 0)),  // default text color
        m_clBackground(RGB(255, 255, 255)), // fill with white by default
        m_clSelectionText(RGB(255, 255, 255)),  
        m_clSelectionBackground(RGB(173, 214, 255)),
        m_useSelectionTextColor(false)
    {
    }

    ~XTextLayoutBaseT()
    {
    }

public: // text to render
    void    setText(XRichText* richText)
    {
        // reset previous layout if any
        _resetLayout();

        // copy rich text pointer
        m_richText = richText;
    }

public: // word wrap
    void    setWordWrap(bool bWordWrap)
    {
        // ignore if same
        if(bWordWrap == m_wordWrap) return;

        // copy flag
        m_wordWrap = bWordWrap;

        // reset line wrapping cache
        _resetLineCache();
    }

    bool    wordWrap() const
    { 
        return m_wordWrap; 
    }

public: // single line
    void    setSingleLineMode(bool singleLine)
    {
        // ignore if same
        if(m_singleLineMode == singleLine) return;

        // copy flag
        m_singleLineMode = singleLine;

        // reset layout
        _resetLayout();
    }

    bool    singleLineMode() const
    { 
        return m_singleLineMode; 
    }

public: // text alignment
    void    setAlignment(TTextAlignment textAlignment)
    {
        // ignore if same
        if(m_textAlignment == textAlignment) return;

        // copy flag
        m_textAlignment = textAlignment;

        // reset line wrapping cache
        _resetLineCache();
    }

    TTextAlignment  alignment() const   
    { 
        return m_textAlignment; 
    }

public: // line padding 
    void    setLinePadding(_XNum beforeLine, _XNum afterLine)
    {
        // copy line spacing
        m_linePaddingAfter = afterLine;
        m_linePaddingBefore = beforeLine;
    }

    void    getLinePadding(_XNum& beforeLine, _XNum& afterLine) const
    {
        // copy line spacing
        afterLine = m_linePaddingAfter;
        beforeLine = m_linePaddingBefore;
    }

public: // size hints
    _XNum getHeightForWidth(_XNum width)
    {
        // save old width
        _XNum widthTmp = m_layoutWidth;

        // set new width
        resize(width);

        // compute height
        _XNum retHeight = contentHeight();

        // set width back
        resize(widthTmp);

        return retHeight;
    }

    _XNum getMaxTextForWidth(_XNum width)
    {
        // return zero if no text
        if(m_richText == 0 || m_richText->textLength() == 0) return 0;

        // TODO: compute maximum text length that would fit for line
        // - this might be needed for single line layout to insert/append ... into long text
        return 0;
    }

public: // line properties
    int getLineCount()
    {
        // return zero if no text
        if(m_richText == 0 || m_richText->textLength() == 0) return 0;

        // update layout if needed
        _updateLayoutIfNeeded();

        // count lines
        size_t lineCount = 0;
        for(unsigned int paraIdx = 0; paraIdx < m_textLayout.size(); ++paraIdx)
        {
            // active paragraph
            const XTextParagraph& textParagraph = m_textLayout.at(paraIdx);

            // line count
            lineCount += textParagraph.layoutLines.size();
        }

        return (int)lineCount;
    }

    bool getLineMetrics(int lineIdx, int& textBegin, int& textEnd, _XNum& lineHeight)
    {
        // reset output
        textBegin = 0;
        textEnd = 0;
        lineHeight = 0;

        // return no text
        if(m_richText == 0 || m_richText->textLength() == 0) return false;

        // update layout if needed
        _updateLayoutIfNeeded();

        // find line
        int lineCount = 0;
        for(unsigned int paraIdx = 0; paraIdx < m_textLayout.size(); ++paraIdx)
        {
            // active paragraph
            const XTextParagraph& textParagraph = m_textLayout.at(paraIdx);

            // line count
            if(lineCount + (int)textParagraph.layoutLines.size() <= lineIdx)
            {
                lineCount += (int)textParagraph.layoutLines.size();
                continue;
            }

            // get line 
            const XLayoutLine& layoutLine = textParagraph.layoutLines.at(lineIdx - lineCount);

            XGlyphCursor glyphCursor;
            glyphCursor.paraIdx = paraIdx;

            // text begin
            glyphCursor.textPos = layoutLine.begin;
            textBegin = (int) _textPosFromCursor(glyphCursor);

            // text end
            glyphCursor.textPos = layoutLine.end;
            textEnd = (int) _textPosFromCursor(glyphCursor);

            // line height
            lineHeight = layoutLine.height;

            // line found
            return true;
        }

        // line not found
        return false;
    }

    bool getLineFitPos(int lineIdx, _XNum maxLineWidth, int& textPos)
    {
        // return no text
        if(m_richText == 0 || m_richText->textLength() == 0) return false;

        // update layout if needed
        _updateLayoutIfNeeded();

        // find line
        int lineCount = 0;
        for(unsigned int paraIdx = 0; paraIdx < m_textLayout.size(); ++paraIdx)
        {
            // active paragraph
            XTextParagraph& textParagraph = m_textLayout.at(paraIdx);

            // line count
            if(lineCount + (int)textParagraph.layoutLines.size() <= lineIdx)
            {
                lineCount += (int)textParagraph.layoutLines.size();
                continue;
            }

            // get line 
            XLayoutLine& layoutLine = textParagraph.layoutLines.at(lineIdx - lineCount);

            XGlyphCursor glyphCursor;
            glyphCursor.paraIdx = paraIdx;

            // check if line will fit
            if(layoutLine.width <= maxLineWidth)
            {
                // full line will fit
                glyphCursor.textPos = layoutLine.end;

            } else
            {
                // find glyph position
                _findGlyphFromLine(maxLineWidth, textParagraph, layoutLine, glyphCursor.textPos);
            }

            textPos = (int) _textPosFromCursor(glyphCursor);

            // line found
            return true;
        }

        // line not found
        return false;
    }

public: // layout size
    void resize(_XNum layoutWidth)
    {
        // reset line layouts if width has changed and word wrapping is needed
        if(m_layoutWidth != layoutWidth && m_wordWrap)
        {
            // reset
            _resetLineCache();
        }

        // copy width
        m_layoutWidth = layoutWidth;
    }

    _XNum contentWidth()
    {
        // return zero if no text
        if(m_richText == 0 || m_richText->textLength() == 0) return 0;

        // update layout if needed
        _updateLayoutIfNeeded();

        // find maximum width
        _XNum maxWidth = 0;
        for(unsigned int paraIdx = 0; paraIdx < m_textLayout.size(); ++paraIdx)
        {
            // active paragraph
            const XTextParagraph& textParagraph = m_textLayout.at(paraIdx);

            // loop over layout lines
            for(unsigned int lineIdx = 0; lineIdx < textParagraph.layoutLines.size(); ++lineIdx)
            {
                // line width
                _XNum lineWidth = textParagraph.layoutLines.at(lineIdx).width;

                // check for maximum
                if(maxWidth < lineWidth)
                {
                    maxWidth = lineWidth;
                }
            }
        }
    
        return maxWidth;
    }

    _XNum contentHeight()
    {
        // return zero if no text
        if(m_richText == 0 || m_richText->textLength() == 0) return 0;

        // update layout if needed
        _updateLayoutIfNeeded();

        // compute height
        _XNum layoutHeight = 0;
        for(unsigned int paraIdx = 0; paraIdx < m_textLayout.size(); ++paraIdx)
        {
            // active paragraph
            const XTextParagraph& textParagraph = m_textLayout.at(paraIdx);

            // loop over layout lines
            for(unsigned int lineIdx = 0; lineIdx < textParagraph.layoutLines.size(); ++lineIdx)
            {
                // update height
                layoutHeight += textParagraph.layoutLines.at(lineIdx).height + m_linePaddingBefore + m_linePaddingAfter;
            }
        }

        return layoutHeight;
    }

public: // hit testing
    bool isInsideText(_XNum originX, _XNum originY, _XNum posX, _XNum posY)
    {
        XLayoutLine layoutLine;
        unsigned int paraIdx;

        return _layoutLineFromPos(originX, originY, posX, posY, layoutLine, paraIdx);
    }

    bool isInsideSelection(_XNum originX, _XNum originY, _XNum posX, _XNum posY)
    {
        // ignore if no selection
        if(!m_hasSelection) return false;
        
        XGlyphCursor glyphCursor;
        if(_cursorFromPos(originX, originY, posX, posY, glyphCursor))
        {
            // check if glyph is inside selection
            if(glyphCursor.paraIdx >= m_selectionBegin.paraIdx && glyphCursor.paraIdx <= m_selectionEnd.paraIdx)
            {
                // check cursors
                if(glyphCursor.paraIdx == m_selectionBegin.paraIdx)
                {
                    if( (glyphCursor.textPos.runIdx < m_selectionBegin.textPos.runIdx) ||
                        ((glyphCursor.textPos.runIdx == m_selectionBegin.textPos.runIdx &&
                          glyphCursor.textPos.runOffset < m_selectionBegin.textPos.runOffset)) ) return false;
                } 
                
                if(glyphCursor.paraIdx == m_selectionEnd.paraIdx)
                {
                    if( (glyphCursor.textPos.runIdx > m_selectionEnd.textPos.runIdx) ||
                        ((glyphCursor.textPos.runIdx == m_selectionEnd.textPos.runIdx &&
                          glyphCursor.textPos.runOffset > m_selectionEnd.textPos.runOffset)) ) return false;
                }

                return true;
            }
        }

        return false;
    }

    bool getTextFromPos(_XNum originX, _XNum originY, _XNum posX, _XNum posY, unsigned int& textPos)
    {
        XGlyphCursor glyphCursor;
        if(_cursorFromPos(originX, originY, posX, posY, glyphCursor))
        {
            // convert to text position
            textPos = (unsigned int) _textPosFromCursor(glyphCursor);

            // position found
            return true;
        }

        // text position is not inside layout
        return false;
    }

public: // layout regions
    XRectRegion getTextRegion(_XNum originX, _XNum originY, unsigned int textPos, unsigned int textLength)
    {
        XRectRegion textRegion;

        // position
        _XNum textPosX = originX;
        _XNum textPosY = originY;

        // loop over all paragraphs
        for(unsigned int paraIdx = 0; paraIdx < m_textLayout.size(); ++paraIdx)
        {
            // active text paragraph
            XTextParagraph& textParagraph = m_textLayout.at(paraIdx);

            // stop if text is over range
            if(textParagraph.range.pos > textPos + textLength) break;

            // loop over layout lines
            for(unsigned int layoutLineIdx = 0; layoutLineIdx < textParagraph.layoutLines.size(); ++layoutLineIdx)
            {
                // active layout line
                XLayoutLine& layoutLine = textParagraph.layoutLines.at(layoutLineIdx);

                _XNum lineHeight, paintRight;

                // line metrics
                _getLineMetrics(originX, originY, layoutLine, textPosX, paintRight, lineHeight, textParagraph.isRTL);

                // skip if paragraph doesn't match text range (we need to count textPosY though)
                if(textPos + textLength < textParagraph.range.pos) 
                {
                    // update line position
                    textPosY += lineHeight;

                    continue;
                }

                // check if we have run caches already
                if(textParagraph.runCaches.size() == 0)
                {
                    // generate shape and position for runs
                    _shapeAndPostionRuns(textParagraph.textRuns, textParagraph.runCaches);
                }

                // init text position
                textPosX = originX;

                // loop over all text runs
                for(unsigned int runIdx = layoutLine.begin.runIdx; runIdx < textParagraph.textRuns.size() && runIdx <= layoutLine.end.runIdx; ++runIdx)
                {
                    // active run
                    const _XTextRun& textRun = textParagraph.textRuns.at(runIdx);
                    _XTextRunCache& runCache = textParagraph.runCaches.at(runIdx);

                    // stop if text is over range
                    if(textRun.range.pos > textPos + textLength) break;

                    // skip if text is not in range
                    if(textPos + textLength < textRun.range.pos) 
                    {
                        // update position
                        textPosX += runCache.place.width;

                        continue;
                    }

                    // compute glyph map (if they are not in cache already)
                    if(textRun.isComplex && runCache.glyphToChar.size() != runCache.shape.glyphs.size())
                    {
                        mapGlyphsToChars(textRun, runCache);
                    }

                    // offsets
                    unsigned int runStartOffset = (runIdx == layoutLine.begin.runIdx) ? layoutLine.begin.runOffset : 0;
                    unsigned int runStopOffset = (runIdx == layoutLine.end.runIdx) ? layoutLine.end.runOffset : (int)runCache.shape.glyphs.size();

                    // compute text width
                    _XNum textWidth = 0;

                    // loop over all glyphs
                    for(unsigned int glyphIdx = runStartOffset; glyphIdx < runStopOffset; ++glyphIdx)
                    {
                        // corresponding character
                        unsigned int charPos = textRun.isComplex ? runCache.glyphToChar.at(glyphIdx) : glyphIdx;

                        // glyph width
                        _XNum glyphWidth = 0;
                        
                        if(layoutLine.justify && glyphIdx < layoutLine.justifyAdvances.size())
                            glyphWidth = layoutLine.justifyAdvances.at(glyphIdx);
                        else
                            glyphWidth = runCache.place.advances.at(glyphIdx);

                        // check if character is inside
                        if(textRun.range.pos + charPos >= textPos && 
                           textRun.range.pos + charPos < textPos + textLength)
                        {
                            // increase width
                            textWidth += glyphWidth;

                        } else if((textRun.range.pos + charPos < textPos && !textRun.isRTL) || 
                                  (textRun.range.pos + charPos >= textPos + textLength && textRun.isRTL))
                        {
                            // update offset
                            textPosX += glyphWidth;
                        }
                    }

                    // create region
                    if(textWidth > 0)
                    {
                        // create region from rectangle
                        XRectRegion runRegion = createRegionFromPoints(textPosX, textPosY, 
                            textPosX + textWidth, textPosY + lineHeight);

                        // merge regions
                        XWUIGraphics::appendRectRegion(textRegion, runRegion);
                    }

                    // update position
                    textPosX += runCache.place.width;
                }

                // update line position
                textPosY += lineHeight;
            }
        }

        // result region
        return textRegion;
    }

public: // selection
    bool hasSelection() const
    {
        return m_hasSelection;
    }

    bool selectionActive() const
    {
        return m_selectionActive;
    }

    void selectionBegin()
    {
        // mark flag
        m_selectionActive = true;

        // reset selection
        m_selectionBegin.paraIdx = (int)m_textLayout.size() + 1;
        m_selectionEnd.paraIdx = (int)m_textLayout.size() + 1;
    }

    bool selectTo(_XNum originX, _XNum originY, _XNum selectFomX, _XNum selectFromY, _XNum selectToX, _XNum selectToY)
    {
        // ignore if not active
        if(!m_selectionActive) return false;

        // copy previous seleciton values
        XGlyphCursor selectionBegin = m_selectionBegin;
        XGlyphCursor selectionEnd = m_selectionEnd;

        // update selection range from points
        _updateSelectionRange(originX, originY, selectFomX, selectFromY, selectToX, selectToY);

        // ignore is selection hasn't changed
        if(selectionBegin.paraIdx == m_selectionBegin.paraIdx &&
           selectionBegin.textPos.runIdx == m_selectionBegin.textPos.runIdx &&
           selectionBegin.textPos.runOffset == m_selectionBegin.textPos.runOffset &&
           selectionEnd.paraIdx == m_selectionEnd.paraIdx &&
           selectionEnd.textPos.runIdx == m_selectionEnd.textPos.runIdx &&
           selectionEnd.textPos.runOffset == m_selectionEnd.textPos.runOffset)
        {
            // no updates needed
            return false;
        }

        // update selection flags for lines
        _updateSelection();

        // reset paint information
        _resetPaintRuns();

        // update needed
        return true;
    }

    void selectionEnd()
    {
        // mark flag
        m_selectionActive = false;
    }

    void clearSelection()
    {
        // reset selection start
        m_selectionBegin.paraIdx = 0;
        m_selectionBegin.textPos.runIdx = 0;
        m_selectionBegin.textPos.runOffset = 0;

        // copy same values
        m_selectionEnd = m_selectionBegin;

        // reset selection flag
        m_hasSelection = false;

        // loop over all paragraphs
        for(unsigned int paraIdx = 0; paraIdx < m_textLayout.size(); ++paraIdx)
        {
            // active paragraph
            XTextParagraph& textParagraph = m_textLayout.at(paraIdx);

            // reset selection
            textParagraph.hasSelection = false;
            textParagraph.selectionBegin.runIdx = 0;
            textParagraph.selectionBegin.runOffset = 0;
            textParagraph.selectionEnd = textParagraph.selectionBegin;
        }

        // reset paint information
        _resetPaintRuns();
    }

    void getSelectedText(XTextRange& selectedText)
    {
        // reset returned region
        selectedText.pos = 0;
        selectedText.length = 0;

        // ignore if no selection
        if(!m_hasSelection) return;

        // get selection start run
        size_t selectionStart = _textPosFromCursor(m_selectionBegin);
        size_t selectionEnd = _textPosFromCursor(m_selectionEnd);

        // validate
        if(selectionStart >= 0 && selectionStart <= selectionEnd)
        {
            // set range
            selectedText.pos = (unsigned int)selectionStart;
            selectedText.length = (unsigned int)(selectionEnd - selectionStart) + 1;
        }
    }

public: // selection colors
    void setSelectionColor(COLORREF clFillColor)
    {
        // set new color
        m_clSelectionBackground = clFillColor;
    
        // reset colors
        if(m_hasSelection)
        {
            _resetPaintRuns();
        }
    }

    void setSelectionTextColor(COLORREF clTextColor)
    {
        // set new color
        m_clSelectionText = clTextColor;
    
        // set flag
        m_useSelectionTextColor = true;

        // reset colors
        if(m_hasSelection)
        {
            _resetPaintRuns();
        }
    }

    void resetSelectionTextColor()
    {
        // reset flag
        m_useSelectionTextColor = false;

        // reset colors
        if(m_hasSelection)
        {
            _resetPaintRuns();
        }
    }

public: // background  
    void enableBackgroundFill(COLORREF clFillColor)
    {
        m_bFillBackground = true;
        m_clBackground = clFillColor;
        
        // reset colors
        _resetPaintRuns();
    }

    void disableBackgroundFill()
    {
        m_bFillBackground = false;
        
        // reset colors
        _resetPaintRuns();
    }

    bool backgroundFillEnabled() const 
    { 
        return m_bFillBackground; 
    }

    COLORREF backgroundFillColor() const 
    { 
        return m_clBackground; 
    }

public: // rich text changes
    void onRichTextModified()
    {
        // clear selection if any
        clearSelection();

        // release cache if any
        _resetLayout();
    }

    void onRichTextStyleChanged(const XTextRange& range)
    {
        // NOTE: in very specific case when there is an active selection we would need 
        //       to update selection begin and end after style has been modified
        m_selectionStyleChanged = m_hasSelection;

        // loop over all paragraphs
        for(unsigned int paraIdx = 0; paraIdx < m_textLayout.size(); ++paraIdx)
        {
            // active paragraph
            XTextParagraph& textParagraph = m_textLayout.at(paraIdx);

            // ignore if paragraph has been reset already
            if(textParagraph.textRuns.size() == 0) continue;

            // reset paragraph in case ranges overlap
            if((textParagraph.range.pos >= range.pos || textParagraph.range.pos  + textParagraph.range.length > range.pos) &&
               (textParagraph.range.pos <= range.pos + range.length || textParagraph.range.pos  + textParagraph.range.length < range.pos + range.length))
            {
                // clear paragraph cache data
                textParagraph.textRuns.clear();
                textParagraph.layoutLines.clear();
                textParagraph.runCaches.clear();
            }
        }
    }

    void onRichTextColorChanged(const XTextRange& range)
    {
        // same as style changes
        onRichTextStyleChanged(range);
    }

protected: // layout types

    // NOTE: the following types expected from template parameters
    //       XNum - measurement type (int, float, etc.)
    //       XTextRun - text run representation, must contain:
    //              XTextRange  range;
    //              XTextStyle  style;
    //              bool        isInlineObject;
    //              bool        isComplex;
    //       XTextRunCache - text run cache representation, must contain:

    // cursor to position in run array (paragraph specific)
    struct XTextCursor 
    {
        unsigned int    runIdx;
        unsigned int    runOffset;
    };

    // glyph position (global for all text)
    struct XGlyphCursor
    {
        unsigned int    paraIdx;    // paragraph index
        XTextCursor     textPos;    // position inside paragraph
    };

    // text paint runs
    struct XTextPaintRun
    {
        _XNum           width; 
        unsigned int    runIdx;
        unsigned int    glyphOffset;
        unsigned int    glyphCount;
        COLORREF        textColor;
        COLORREF        backgroundColor;
        bool            fillBackground;
        bool            selected;
        _XNum*          justifyPtr;
    };

    ///// layout line properties (position glyphs)
    struct XLayoutLine
    {
        _XNum           width; 
        _XNum           height; 
        _XNum           maxAscent; 
        XTextCursor     begin;
        XTextCursor     end;
        bool            justify;

        std::vector<_XNum>          justifyAdvances;
        std::vector<XTextPaintRun>  paintRuns;
    };

    ///// text paragraph (divided based on line breaks in text)
    struct XTextParagraph
    {
        XTextRange      range;

        bool            hasSelection;
        bool            isRTL;              // paragraph contains only RTL scripts
        XTextCursor     selectionBegin;
        XTextCursor     selectionEnd;

        std::vector<_XTextRun>      textRuns;
        std::vector<_XTextRunCache> runCaches;
        std::vector<XLayoutLine>    layoutLines;

    };

protected: // region creation interface
    virtual XRectRegion createRegionFromPoints(_XNum x1, _XNum y1, _XNum x2, _XNum y2) = 0;

protected: // layout building interface
    virtual void    analyseRichText(const XRichText* richText, const XTextRange& range, std::vector<_XTextRun>& runsOut) = 0;
    virtual void    layoutTextRuns(std::vector<_XTextRun>& textRuns) = 0;
    virtual void    shapeAndPostionTextRun(_XTextRun& textRun, _XTextRunCache& runCache) = 0;
    virtual void    getStyleMetrics(const XTextStyle& style, _XNum& fontHeight, _XNum& fontAscent) = 0;
    virtual void    getInlineObjectMetrics(_XTextRun& textRun, _XNum& objectHeight, _XNum& objectWidth) = 0;
    virtual void    getRunLogicalAttrs(const _XTextRun& textRun, _XTextRunCache& runCache) = 0;
    virtual void    mapGlyphsToChars(const _XTextRun& textRun, _XTextRunCache& runCache) = 0;
    virtual int     getCharJustification(const _XTextRunCache& runCache, unsigned int glyphPos) = 0;

protected: // layout building
    void    _resetLayout()
    {
        // clear all caches
        m_textLayout.clear();
    }

    void _resetLineCache()
    {
        // reset line layout
        for(unsigned int idx = 0; idx < m_textLayout.size(); ++idx)
        {
            m_textLayout.at(idx).layoutLines.clear();
        }
    }

    void _resetRunCaches()
    {
        // loop over all paragraphs in layout
        for(unsigned int idx = 0; idx < m_textLayout.size(); ++idx)
        {
            // NOTE: to really free memory we need to swap with empty vector
            std::vector<_XTextRunCache> dummyVector;

            // clear caches
            m_textLayout.at(idx).runCaches.swap(dummyVector);
        }
    }

    void _resetPaintRuns()
    {
        // loop over paragraphs
        for(unsigned int paraIdx = 0; paraIdx < m_textLayout.size(); ++paraIdx)
        {
            // active paragraph
            XTextParagraph& textParagraph = m_textLayout.at(paraIdx);

            // loop over layout lines
            for(unsigned int lineIdx = 0; lineIdx < textParagraph.layoutLines.size(); ++lineIdx)
            {
                // reset paint caches
                textParagraph.layoutLines.at(lineIdx).paintRuns.clear();
            }
        }
    }

    void    _updateLayoutIfNeeded()
    {
        // ignore if no text
        if(m_richText == 0 || m_richText->textLength() == 0) return;

        // check if cache needs to be updated
        if(m_textLayout.size() == 0)
        {
            // create new layout from text
            _createLayout(m_layoutWidth);

        } else 
        {
            // update only missing parts if needed
            _updateLayout(m_layoutWidth);
        }

        // update selection positions if needed
        if(m_hasSelection && m_selectionStyleChanged)
        {
            // restore selection cursors in new formating
            m_selectionBegin = _cursorFromGlyphOffset(m_selectionBegin, m_selectionStartGlyph);
            m_selectionEnd = _cursorFromGlyphOffset(m_selectionEnd, m_selectionEndGlyph);

            // reset flag
            m_selectionStyleChanged = false;

            // update selection flags for lines
            _updateSelection();
        }
    }

    void    _updateParagraphRTL(XTextParagraph& textParagraph)
    {
        // NOTE: in case of mixed text we set paragraph direction based on majority of runs
        unsigned int rtlCount = 0;
        unsigned int ltrCount = 0;

        // loop over all text runs
        for(unsigned int runIdx = 0; runIdx < textParagraph.textRuns.size(); ++runIdx)
        {
            // check style specified by user
            if(textParagraph.textRuns.at(runIdx).style.isRTL)
                rtlCount += textParagraph.textRuns.at(runIdx).range.length;
            else
                ltrCount += textParagraph.textRuns.at(runIdx).range.length;
        }

        // set flag
        textParagraph.isRTL = (rtlCount > ltrCount);
    }

    void    _analyseParagraph(XTextParagraph& textParagraph)
    {
        // process whole text into runs first
        analyseRichText(m_richText, textParagraph.range, textParagraph.textRuns);

        // re-order runs if needed
        layoutTextRuns(textParagraph.textRuns);

        // update RTL flag for paragraph
        _updateParagraphRTL(textParagraph);
    }

    void    _updateLayout(_XNum paintWidth)
    {
        // loop over all paragraphs
        for(unsigned int paraIdx = 0; paraIdx < m_textLayout.size(); ++paraIdx)
        {
            // active paragraph
            XTextParagraph& textParagraph = m_textLayout.at(paraIdx);

            // analyse paragraph if needed
            if(textParagraph.textRuns.size() == 0)
            {
                // clear layout caches just in case
                textParagraph.layoutLines.clear();
                textParagraph.runCaches.clear();

                // split to runs
                _analyseParagraph(textParagraph);
            }

            // update paragraph layout
            if(textParagraph.layoutLines.size() == 0)
            {
                _layoutParagraph(textParagraph, paintWidth);
            }
        }

        // copy used width
        m_layoutWidth = paintWidth;
    }

    void    _createLayout(_XNum paintWidth)
    {
        // reset previous cache if any
        _resetLayout();

        // ignore if no text
        if(m_richText == 0 || m_richText->textLength() == 0) return;

        // split text into paragraphs based on line breaks
        std::vector<XTextRange> textLines;
        if(!m_singleLineMode)
            m_richText->splitToLines(textLines);
        else
            textLines.push_back(XTextRange(0, m_richText->textLength()));

        // loop over all paragraphs
        for(unsigned int lineIdx = 0; lineIdx < textLines.size(); ++lineIdx)
        {
            XTextParagraph textParagraph;
            textParagraph.range = textLines.at(lineIdx);
            textParagraph.hasSelection = false;
            textParagraph.isRTL = false;

            // split paragraph into runs
            _analyseParagraph(textParagraph);

            // ignore empty paragraphs
            if(textParagraph.textRuns.size() == 0)
            {
                XWTRACE("XTextLayoutBaseT::_updateLayout empty paragraph ignored");

                // ignore
                continue;
            }

            // update paragraph layout
            _layoutParagraph(textParagraph, paintWidth);

            // append paragraph to layout
            m_textLayout.push_back(textParagraph);
        }

        // update flags
        m_layoutWidth = paintWidth;
    }

    void _shapeAndPostionRuns(std::vector<_XTextRun>& textRuns, std::vector<_XTextRunCache>& runCaches)
    {
        XWASSERT(m_richText);

        // loop over all text runs
        for(unsigned int runIdx = 0; runIdx < textRuns.size(); ++runIdx)
        {
            _XTextRunCache runCache;

            // shape and position text run
            shapeAndPostionTextRun(textRuns.at(runIdx), runCache);

            // validate assumptions
            XWASSERT(runCache.shape.glyphs.size() == runCache.place.advances.size());

            // add to cache
            runCaches.push_back(runCache);
        }
    }

    _XNum _computeRunWidth(XTextParagraph& textParagraph, unsigned int runIdx, 
                                         unsigned int runStartOffset, unsigned int runStopOffset)
    {
        // get runs
        const _XTextRun& textRun = textParagraph.textRuns.at(runIdx);
        const _XTextRunCache& runCache = textParagraph.runCaches.at(runIdx);

        // check if whole run width can be used
        if(runStartOffset == 0 && runStopOffset == runCache.place.advances.size())
        {
            // use whole width
            return runCache.place.width;

        } else
        {
            // sum advances
            _XNum retWidth = 0;
            for(unsigned int idx = runStartOffset; idx < runStopOffset; ++idx)
            {
                retWidth += runCache.place.advances.at(idx);
            }

            return retWidth;
        }
    }

    void _updateLayoutLineWidth(XTextParagraph& textParagraph, XLayoutLine& layoutLine)
    {
        // reset width
        layoutLine.width = 0;

        // loop over all runs
        for(unsigned int runIdx = layoutLine.begin.runIdx; runIdx <= layoutLine.end.runIdx; ++runIdx)
        {
            unsigned int runStartOffset = (runIdx == layoutLine.begin.runIdx) ? layoutLine.begin.runOffset : 0;
            unsigned int runStopOffset = (runIdx == layoutLine.end.runIdx) ? layoutLine.end.runOffset : 
                                                                             (int)textParagraph.runCaches.at(runIdx).place.advances.size();

            // append width
            layoutLine.width += _computeRunWidth(textParagraph, runIdx, runStartOffset, runStopOffset);
        }
    }

    void _updateLayoutLineHeight(XTextParagraph& textParagraph, XLayoutLine& layoutLine)
    {
        XWASSERT(m_richText);

        // inline object flag
        bool hasInlineObjects = false;

        // reset properties
        layoutLine.height = 0;
        layoutLine.maxAscent = 0;

        // loop over all runs
        for(unsigned int runIdx = layoutLine.begin.runIdx; runIdx <= layoutLine.end.runIdx; ++runIdx)
        {
            // ignore inline objects from calculation at this point
            if(textParagraph.textRuns.at(runIdx).isInlineObject)
            {
                // mark flag and continue
                hasInlineObjects = true;
                continue;
            }

            _XNum fontHeight, fontAscent;

            // get current line metrics from style
            getStyleMetrics(textParagraph.textRuns.at(runIdx).style, fontHeight, fontAscent);

            // update height
            XWASSERT(fontHeight > 0);
            if(layoutLine.height < fontHeight)
            {
                // use maximum height
                layoutLine.height = fontHeight;
            }

            // update ascent
            XWASSERT(fontAscent > 0);
            if(layoutLine.maxAscent < fontAscent)
            {
                // use maximum ascent
                layoutLine.maxAscent = fontAscent;
            }
        }

        // ignore if no inline objects found
        if(!hasInlineObjects) return;

        // NOTE: now we have computed maximum line height and ascent for text only, 
        //       update those now with inline object sizes

        // loop over inline objects and update final line height
        for(unsigned int runIdx = layoutLine.begin.runIdx; runIdx <= layoutLine.end.runIdx; ++runIdx)
        {
            if(textParagraph.textRuns.at(runIdx).isInlineObject)
            {
                _XNum objectHeight, objectWidth;

                // get inline object metrics
                getInlineObjectMetrics(textParagraph.textRuns.at(runIdx), objectHeight, objectWidth);

                // check if inline object height is bigger
                if(objectHeight > layoutLine.height)
                {
                    // we need to update ascent too to position text lower
                    layoutLine.maxAscent += objectHeight - layoutLine.height;

                    // update line height
                    layoutLine.height = objectHeight;
                }
            }
        }
    }

    void _wordWrapLayoutLine(XTextParagraph& textParagraph, XLayoutLine& layoutLine, _XNum lineWidth, 
                                            const XTextCursor& lineBegin, const XTextCursor& lineEnd, bool& skipSpace)
    {
        // validate assumptions
        XWASSERT(layoutLine.width > lineWidth);
    
        // reset flag
        skipSpace = false;

        // reset current line 
        layoutLine.width = 0;
        layoutLine.begin = lineBegin;
        layoutLine.end = lineBegin;

        // TODO: this method is not working well if style is changing in the middle of the word. In this case line 
        //       can be split on style change boundary.

        // loop over all runs
        for(unsigned int runIdx = lineBegin.runIdx; runIdx <= lineEnd.runIdx; ++runIdx)
        {
            // get run data
            const _XTextRun& textRun = textParagraph.textRuns.at(runIdx);
            _XTextRunCache& runCache = textParagraph.runCaches.at(runIdx);

            // offsets
            unsigned int runStartOffset = (runIdx == lineBegin.runIdx) ? lineBegin.runOffset : 0;
            unsigned int runStopOffset = (runIdx == lineEnd.runIdx) ? lineEnd.runOffset : (int)runCache.shape.glyphs.size();

            // check if whole run width can be used
            if(runStartOffset == 0 && runStopOffset == runCache.shape.glyphs.size() && 
                layoutLine.width + runCache.place.width < lineWidth)
            {
                // append width
                layoutLine.width += runCache.place.width;

                // include run to line
                layoutLine.end.runIdx = runIdx;
                layoutLine.end.runOffset = runStopOffset;

                // next run
                continue;

            } else if(runStartOffset > 0 && runIdx == lineBegin.runIdx && runStartOffset == lineBegin.runOffset)
            {
                // we start in the middle of the run, check if we can fit it whole
                _XNum leftWidth = 0;
                for(unsigned int glyphIdx = runStartOffset; glyphIdx < runStopOffset; ++glyphIdx)
                {
                    // append width
                    leftWidth += runCache.place.advances.at(glyphIdx);
                }

                // check if we can fit
                if(layoutLine.width + leftWidth < lineWidth)
                {
                    // append width
                    layoutLine.width += leftWidth;

                    // include run to line
                    layoutLine.end.runIdx = runIdx;
                    layoutLine.end.runOffset = runStopOffset;

                    // next run
                    continue;
                }
            }

            // generate logical attributes (if they are not in cache already)
            if(runCache.logAttrs.size() != textRun.range.length)
            {
                getRunLogicalAttrs(textRun, runCache);
            }

            // compute glyph map (if they are not in cache already)
            if(textRun.isComplex && runCache.glyphToChar.size() == 0)
            {
                mapGlyphsToChars(textRun, runCache);
            }
    
            // check input
            if(runStartOffset >= runCache.place.advances.size() || 
               runStopOffset > runCache.place.advances.size()) 
            {
                // ignore this in release
                XWASSERT(false);
                continue;
            }

            // loop over run glyphs and try to fit them
            _XNum glyphWidth = runCache.place.advances.at(runStartOffset);
            for(unsigned int glyphIdx = runStartOffset + 1; glyphIdx < runStopOffset && layoutLine.width + glyphWidth < lineWidth; ++glyphIdx)
            {
                // corresponding character
                unsigned int charPos = textRun.isComplex ? runCache.glyphToChar.at(glyphIdx) : glyphIdx;

                // check if text can be split on glyph
                if(runCache.logAttrs.at(charPos).fWhiteSpace || runCache.logAttrs.at(charPos).fSoftBreak)
                {
                    // add to line
                    layoutLine.width += glyphWidth;
                    layoutLine.end.runIdx = runIdx;
                    layoutLine.end.runOffset = glyphIdx; // do not include glyph itself
                    glyphWidth = 0;

                    // mark if ending space can be skipped
                    skipSpace = runCache.logAttrs.at(charPos).fWhiteSpace;
                }

                // append width
                glyphWidth += runCache.place.advances.at(glyphIdx);
            }

            // line cannot be empty
            if(layoutLine.width <= 0)
            {
                // add at least single glyph
                layoutLine.width = runCache.place.advances.at(runStartOffset);
                layoutLine.end.runIdx = runIdx;
                layoutLine.end.runOffset = runStartOffset + 1; 

                // just add as many glyphs as will fit regardless if this will be correct or not
                for(unsigned int glyphIdx = runStartOffset + 1; glyphIdx < runStopOffset; ++glyphIdx)
                {
                    // check if glyph can fit
                    if(layoutLine.width + runCache.place.advances.at(glyphIdx) >= lineWidth)
                    {
                        // stop
                        break;
                    }

                    // add glyph
                    layoutLine.width += runCache.place.advances.at(glyphIdx);
                    layoutLine.end.runOffset = glyphIdx + 1; 
                }
            }

            // line is full, stop
            break;
        }
    }

    void _justifyLayoutLine(XTextParagraph& textParagraph, XLayoutLine& layoutLine, _XNum lineWidth)
    {
        // do nothing if width is same or bigger
        if(layoutLine.width >= lineWidth) return;

        // reset justified advances
        layoutLine.justifyAdvances.clear();

        // compute difference to justify
        _XNum justifyWidth = lineWidth - layoutLine.width;

        // number of characters to justify
        int justifyCharCount = 0;

        // collect original advances
        for(unsigned int runIdx = layoutLine.begin.runIdx; runIdx <= layoutLine.end.runIdx; ++runIdx)
        {
            // get run data
            const _XTextRun& textRun = textParagraph.textRuns.at(runIdx);
            _XTextRunCache& runCache = textParagraph.runCaches.at(runIdx);

            // offsets
            unsigned int runStartOffset = (runIdx == layoutLine.begin.runIdx) ? layoutLine.begin.runOffset : 0;
            unsigned int runStopOffset = (runIdx == layoutLine.end.runIdx) ? layoutLine.end.runOffset : (int)runCache.shape.glyphs.size();

            // generate logical attributes (if they are not in cache already)
            if(runCache.logAttrs.size() != textRun.range.length)
            {
                getRunLogicalAttrs(textRun, runCache);
            }

            // compute glyph map (if they are not in cache already)
            if(textRun.isComplex && runCache.glyphToChar.size() != runCache.shape.glyphs.size())
            {
                mapGlyphsToChars(textRun, runCache);
            }    

            // validate that logical attributes and glyph map have been computed
            if(runCache.logAttrs.size() != textRun.range.length || 
               (textRun.isComplex && runCache.glyphToChar.size() != runCache.shape.glyphs.size()))
            {
                XWASSERT(false);
            
                // something wrong with data, ignore whole line
                layoutLine.justifyAdvances.clear();
                layoutLine.justify = false;
                return;
            }

            // count justification possibilities
            for(unsigned int glyphIdx = runStartOffset; glyphIdx < runStopOffset; ++glyphIdx)
            {
                // character index
                UINT16 charIndex = textRun.isComplex ? runCache.glyphToChar.at(glyphIdx) : glyphIdx;

                // check character properties
                if((!textRun.isComplex && runCache.logAttrs.at(charIndex).fWhiteSpace) ||
                   (textRun.isComplex && getCharJustification(runCache, glyphIdx) != SCRIPT_JUSTIFY_ARABIC_BLANK))
                {
                    // add possibility
                    ++justifyCharCount;
                }

                // append original advances (will be updated later)
                layoutLine.justifyAdvances.push_back(runCache.place.advances.at(glyphIdx));
            }
        }

        // ignore if nothing to justify
        if(justifyCharCount == 0) return;

        // justificaiton to append per space or kashida character
        _XNum justifyExtra = (_XNum)((float)justifyWidth / (float)justifyCharCount);

        // loop again now to increase advances
        int justifyAdvancesIndex = 0;
        for(unsigned int runIdx = layoutLine.begin.runIdx; runIdx <= layoutLine.end.runIdx && justifyWidth > 0; ++runIdx)
        {
            // get run data
            const _XTextRun& textRun = textParagraph.textRuns.at(runIdx);
            const _XTextRunCache& runCache = textParagraph.runCaches.at(runIdx);

            // offsets
            unsigned int runStartOffset = (runIdx == layoutLine.begin.runIdx) ? layoutLine.begin.runOffset : 0;
            unsigned int runStopOffset = (runIdx == layoutLine.end.runIdx) ? layoutLine.end.runOffset : (int)runCache.shape.glyphs.size();

            // find justification possibilities
            for(unsigned int glyphIdx = runStartOffset; glyphIdx < runStopOffset && justifyWidth > 0; ++glyphIdx)
            {
                // character index
                UINT16 charIndex = textRun.isComplex ? runCache.glyphToChar.at(glyphIdx) : glyphIdx;

                // check character properties
                if((!textRun.isComplex && runCache.logAttrs.at(charIndex).fWhiteSpace) ||
                   (textRun.isComplex && getCharJustification(runCache, glyphIdx) != SCRIPT_JUSTIFY_ARABIC_BLANK))
                {
                    // check if there is room to justify still
                    if(justifyWidth < justifyExtra) justifyExtra = justifyWidth;

                    // increase advance
                    layoutLine.justifyAdvances.at(justifyAdvancesIndex) += justifyExtra;
                    justifyWidth -= justifyExtra;
                }

                // next
                ++justifyAdvancesIndex;
            }
        }
    }

    void _layoutParagraph(XTextParagraph& textParagraph, _XNum paintWidth)
    {
        // check if we have run caches already
        if(textParagraph.runCaches.size() == 0)
        {
            // generate shape and position for runs
            _shapeAndPostionRuns(textParagraph.textRuns, textParagraph.runCaches);
        }

        // validate caches
        if(textParagraph.textRuns.size() == 0 || textParagraph.runCaches.size() == 0 ||
           textParagraph.textRuns.size() != textParagraph.runCaches.size())
        {
            XWASSERT(false);
            return;
        }

        // end of line cursor
        XTextCursor endOfLine;
        endOfLine.runIdx = (int)textParagraph.runCaches.size() - 1;
        endOfLine.runOffset = (int)textParagraph.runCaches.back().shape.glyphs.size();

        // use whole line first
        XLayoutLine layoutLine;
        layoutLine.begin.runIdx = 0;
        layoutLine.begin.runOffset = 0;
        layoutLine.end = endOfLine;
        layoutLine.justify = false;

        // compute initial width (NOTE: should be fast as already computed run widths will be used)
        _updateLayoutLineWidth(textParagraph, layoutLine);

        // use whole range if word wrapping is not needed
        if(!m_wordWrap || layoutLine.width <= paintWidth)
        {
            // compute line height
            _updateLayoutLineHeight(textParagraph, layoutLine);

            // append line
            textParagraph.layoutLines.push_back(layoutLine);
            return;
        }

        // word wrap lines
        while(layoutLine.width > paintWidth && layoutLine.begin.runIdx < (int)textParagraph.runCaches.size())
        {
            _XNum previousWidth = layoutLine.width;

            // space flag
            bool skipSpace = false;

            // wrap line
            _wordWrapLayoutLine(textParagraph, layoutLine, paintWidth, layoutLine.begin, endOfLine, skipSpace);

            // there should be always something
            XWASSERT(layoutLine.width);

            // justify line if needed
            if((m_textAlignment == eTextAlignJustify) && layoutLine.width < paintWidth)
            {
                // set flag
                layoutLine.justify = true;

                // justify layout line
                _justifyLayoutLine(textParagraph, layoutLine, paintWidth);
            }

            // update line height (width has been computed already)
            _updateLayoutLineHeight(textParagraph, layoutLine);

            // append line
            textParagraph.layoutLines.push_back(layoutLine);

            // remaining line
            layoutLine.begin = layoutLine.end;
            layoutLine.end = endOfLine;
            layoutLine.width = previousWidth - layoutLine.width;
            layoutLine.justifyAdvances.clear();
            layoutLine.justify = false;

            // skip space if at the end of line
            if(skipSpace)
            {
                layoutLine.begin.runOffset++;
            }

            // jump to next run if stopped on run boundary
            if(layoutLine.begin.runOffset >= textParagraph.runCaches.at(layoutLine.begin.runIdx).shape.glyphs.size())
            {
                layoutLine.begin.runIdx++;
                layoutLine.begin.runOffset = 0;
            }
        }

        // stop if at the end
        if(layoutLine.begin.runIdx >= (unsigned int)textParagraph.runCaches.size() - 1 && 
           layoutLine.begin.runOffset >= (unsigned int)textParagraph.runCaches.back().shape.glyphs.size())
        {
            return;
        }

        // append last line if any
        if(layoutLine.width > 0)
        {
            // update line height (width has been computed already)
            _updateLayoutLineHeight(textParagraph, layoutLine);

            // append line
            textParagraph.layoutLines.push_back(layoutLine);
        }
    }

    void _updateSelectionColors(const XTextParagraph& textParagraph, const XLayoutLine& layoutLine, XTextPaintRun& paintRun)
    {
        // skip if no selection for line
        if(!m_hasSelection || !textParagraph.hasSelection) return;

        // check if paint run is inside selection
        if(paintRun.runIdx >= textParagraph.selectionBegin.runIdx && paintRun.runIdx <= textParagraph.selectionEnd.runIdx)
        {
            // selection start
            if(paintRun.runIdx == textParagraph.selectionBegin.runIdx)
            {
                // check if start position overlaps
                if(paintRun.glyphOffset + paintRun.glyphCount < textParagraph.selectionBegin.runOffset)
                {
                    // paint run doesn't have selection
                    return;

                } else if(paintRun.glyphOffset < textParagraph.selectionBegin.runOffset && 
                          paintRun.glyphOffset + paintRun.glyphCount >= textParagraph.selectionBegin.runOffset)
                {
                    // selection starts inside paint run, so shorten run up to selection
                    paintRun.glyphCount = textParagraph.selectionBegin.runOffset - paintRun.glyphOffset;
                    return;
                }
            }

            // selection end
            if(paintRun.runIdx == textParagraph.selectionEnd.runIdx)
            {
                // check if end position overlaps
                if(paintRun.glyphOffset > textParagraph.selectionEnd.runOffset)
                {
                    // paint run doesn't have selection
                    return;

                } else if(paintRun.glyphOffset < textParagraph.selectionEnd.runOffset && 
                          paintRun.glyphOffset + paintRun.glyphCount >= textParagraph.selectionEnd.runOffset)
                {
                    // selection ends before paint run, so shorten it up to selection end
                    paintRun.glyphCount = textParagraph.selectionEnd.runOffset - paintRun.glyphOffset;

                } else if(paintRun.glyphOffset == textParagraph.selectionEnd.runOffset)
                {
                    // selection ends on run border, make single glyph run
                    paintRun.glyphCount = 1;
                }
            }

            // update paint run colors to selection
            if(m_useSelectionTextColor) paintRun.textColor = m_clSelectionText;
            paintRun.backgroundColor = m_clSelectionBackground;
            paintRun.fillBackground = true;
            paintRun.selected = true;
        }
    }

    void _appendPaintRuns(XTextParagraph& textParagraph, XLayoutLine& layoutLine, 
                                  unsigned int runIdx, unsigned int startOffset, unsigned int stopOffset)
    {
        XWASSERT(m_richText);

        // get runs
        const _XTextRun& textRun = textParagraph.textRuns.at(runIdx);
        _XTextRunCache& runCache = textParagraph.runCaches.at(runIdx);

        // check if script is complex
        if(textRun.isComplex)
        {
            // compute glyph map first if not in cache already
            if(runCache.glyphToChar.size() == 0)
            {
                // map glyphs
                mapGlyphsToChars(textRun, runCache);
            }

            // check output
            XWASSERT(runCache.glyphToChar.size() == runCache.shape.glyphs.size());
            if(runCache.glyphToChar.size() != runCache.shape.glyphs.size())
            {
                XWTRACE("XTextLayoutBaseT::_appendPaintRuns failed to compute paint run for complex script");
                return;
            }

        } else
        {
            // validate assumptions about non complex scripts
            XWASSERT(textRun.range.length == (int)runCache.shape.glyphs.size());
        }

        // loop over all glyphs
        unsigned int textPos = 0;
        unsigned int glyphPos = startOffset;
        while(glyphPos < stopOffset)
        {
            // init paint run
            XTextPaintRun paintRun;
            paintRun.runIdx = runIdx;
            paintRun.justifyPtr = 0;
            paintRun.selected = false;

            // check if script is complex
            if(textRun.isComplex)
            {
                // text position from glyph
                textPos = runCache.glyphToChar.at(glyphPos);

                // init paint range
                paintRun.glyphOffset = glyphPos;
                paintRun.glyphCount = 1;

                // add glyphs with the same color to this run
                while(paintRun.glyphOffset + paintRun.glyphCount < stopOffset)
                {
                    // stop if colors are different
                    if(!m_richText->sameColors(textRun.range.pos + textPos, 
                                               textRun.range.pos + runCache.glyphToChar.at(paintRun.glyphOffset + paintRun.glyphCount)))
                    {
                        break;
                    }

                    // add glyph to run
                    paintRun.glyphCount++;
                }

            } else
            {
                // text position from glyph
                textPos = glyphPos;

                // get color run
                unsigned int colorRunEnd = m_richText->getColorRun(textRun.range.pos + textPos, stopOffset - textPos);

                // paint range
                paintRun.glyphOffset = textPos;
                paintRun.glyphCount = colorRunEnd - textRun.range.pos - paintRun.glyphOffset;
            }

            // init default colors
            paintRun.textColor = m_clText;
            paintRun.backgroundColor = m_clBackground;

            // init flags
            bool isTextColorSet = m_richText->textColor(textRun.range.pos + textPos, paintRun.textColor);
            paintRun.fillBackground = m_richText->backgroundColor(textRun.range.pos + textPos, paintRun.backgroundColor);

            // always fill background if needed
            if(m_bFillBackground) paintRun.fillBackground = true;

            // update color run in case it overlaps with selection range
            _updateSelectionColors(textParagraph, layoutLine, paintRun);

            // check if run is the same as text run
            if((paintRun.glyphOffset == 0) && (paintRun.glyphCount == runCache.shape.glyphs.size()))
            {
                // use same width
                paintRun.width = runCache.place.width;

            } else
            {
                // compute text width
                paintRun.width = 0;
                for(unsigned int idx = 0; idx < paintRun.glyphCount; ++idx)
                {
                    paintRun.width += runCache.place.advances.at(paintRun.glyphOffset + idx);
                }
            }

            // append paint run
            layoutLine.paintRuns.push_back(paintRun);

            // next
            glyphPos = paintRun.glyphOffset + paintRun.glyphCount;
        }
    }

    void _updateLinePaintRuns(XTextParagraph& textParagraph, XLayoutLine& layoutLine, _XNum lineWidth)
    {
        // loop over all runs in line
        for(unsigned int runIdx = layoutLine.begin.runIdx; runIdx <= layoutLine.end.runIdx; ++runIdx)
        {
            unsigned int startOffset = (runIdx == layoutLine.begin.runIdx) ? layoutLine.begin.runOffset : 0;
            unsigned int stopOffset = (runIdx == layoutLine.end.runIdx) ? layoutLine.end.runOffset : 
                                                                            (int)textParagraph.runCaches.at(runIdx).shape.glyphs.size();

            // format paint runs from layout run
            _appendPaintRuns(textParagraph, layoutLine, runIdx, startOffset, stopOffset);
        }

        // validate line
        XWASSERT(layoutLine.justify == (layoutLine.justifyAdvances.size() > 0));

        // update paint runs justification offsets
        if(layoutLine.justify && layoutLine.justifyAdvances.size() > 0)
        {
            // loop over all paint runs
            unsigned int justifyOffset = 0;
            for(unsigned int paintRunIdx = 0; paintRunIdx < layoutLine.paintRuns.size(); ++paintRunIdx)
            {
                // paint run
                XTextPaintRun& paintRun = layoutLine.paintRuns.at(paintRunIdx);

                // set paint run offset
                XWASSERT(justifyOffset + paintRun.glyphCount <= layoutLine.justifyAdvances.size());
                if(justifyOffset + paintRun.glyphCount <= layoutLine.justifyAdvances.size())
                {
                    // set pointer to justified advances
                    paintRun.justifyPtr = layoutLine.justifyAdvances.data() + justifyOffset;

                    // update width
                    paintRun.width = 0;
                    for(unsigned int glyphIdx = 0; glyphIdx < paintRun.glyphCount; ++glyphIdx)
                    {
                        paintRun.width += paintRun.justifyPtr[glyphIdx];
                    }
                }

                // add glyphs count
                justifyOffset += paintRun.glyphCount;
            }
        }
    }

    void _updatePaintRuns(XTextParagraph& textParagraph, _XNum lineWidth)
    {
        // loop over all layout lines
        for(unsigned int lineIdx = 0; lineIdx < textParagraph.layoutLines.size(); ++lineIdx)
        {
            // current line
            XLayoutLine& layoutLine = textParagraph.layoutLines.at(lineIdx);

            // ignore if paint runs have been already created
            if(layoutLine.paintRuns.size() > 0) continue;

            // update paint runs for line
            _updateLinePaintRuns(textParagraph, layoutLine, lineWidth);
        }
    }

protected: // layout search helpers
    _XNum _getLineHeight(const XLayoutLine& layoutLine) const
    {
        return layoutLine.height + m_linePaddingBefore + m_linePaddingAfter;
    }

    _XNum _getLineWidth(const XLayoutLine& layoutLine) const
    {
        // check justification
        return layoutLine.justify ? m_layoutWidth : layoutLine.width;
    }

    void _getLineMetrics(_XNum originX, _XNum originY, const XLayoutLine& layoutLine, 
                                _XNum& textBegin, _XNum& textEnd, _XNum& lineHeight, bool isRTL) const
    {
        // text start
        textBegin = originX;

        // take alignment into account
        if(layoutLine.width < m_layoutWidth)
        {
            // NOTE: align right RTL paragraphs

            if(m_textAlignment == eTextAlignRight || isRTL)
                textBegin = originX + m_layoutWidth - layoutLine.width;
            else if(m_textAlignment == eTextAlignCenter)
                textBegin = originX + (m_layoutWidth - layoutLine.width) / 2;
        } 
    
        // check justification
        if(m_textAlignment == eTextAlignJustify && layoutLine.justify)
        {
            // use whole width
            textBegin = originX;
            textEnd = textBegin + m_layoutWidth;

        } else
        {
            // text end
            textEnd = textBegin + layoutLine.width;
        }

        // line height
        lineHeight = _getLineHeight(layoutLine);
    }

    _XNum _maxLineHeight()
    {
        _XNum lineHeightMax = 1; // NOTE: use at least something as it cannot be zero

        // find maximum
        for(unsigned int layoutLineIdx = 0; layoutLineIdx < m_textLayout.size(); ++layoutLineIdx)
        {
            // active text line
            const XTextParagraph& textParagraph = m_textLayout.at(layoutLineIdx);

            // loop over layout lines
            for(unsigned int lineIdx = 0; lineIdx < textParagraph.layoutLines.size(); ++lineIdx)
            {
                // line height
                int lineHeight = _getLineHeight(textParagraph.layoutLines.at(lineIdx));
            
                // update maximum height
                if(lineHeightMax < lineHeight)
                {
                    lineHeightMax = lineHeight;
                }
            }
        }

        return lineHeightMax;
    }

    bool _layoutLineFromPos(_XNum originX, _XNum originY, _XNum posX, _XNum posY, XLayoutLine& layoutLineOut, unsigned int& paraIdx)
    {
        // position
        _XNum textPosX = originX;
        _XNum textPosY = originY;

        // loop over all paragraphs
        for(paraIdx = 0; paraIdx < m_textLayout.size(); ++paraIdx)
        {
            // text paragraph
            XTextParagraph& textParagraph = m_textLayout.at(paraIdx);

            // check if we have run caches already
            if(textParagraph.runCaches.size() == 0)
            {
                // generate shape and position for runs
                _shapeAndPostionRuns(textParagraph.textRuns, textParagraph.runCaches);
            }

            // loop over layout lines
            for(unsigned int layoutLineIdx = 0; layoutLineIdx < textParagraph.layoutLines.size(); ++layoutLineIdx)
            {
                // layout line
                XLayoutLine& layoutLine = textParagraph.layoutLines.at(layoutLineIdx);

                _XNum textBegin, textEnd, lineHeight;

                // line metrics
                _getLineMetrics(originX, originY, layoutLine, textBegin, textEnd, lineHeight, textParagraph.isRTL);

                // check if position is inside line
                if(posY >= textPosY && posY <= textPosY + layoutLine.height && 
                   posX >= textBegin && posX <= textEnd)
                {
                    layoutLineOut = layoutLine;
                    return true;
                }

                // update total height
                textPosY += lineHeight;

                // stop if over Y position
                if(textPosY > posY) return false;
            }
        }

        return false;
    }

    bool _cursorFromPos(_XNum originX, _XNum originY, _XNum posX, _XNum posY, XGlyphCursor& cursorOut)
    {
        XLayoutLine layoutLine;
        if(_layoutLineFromPos(originX, originY, posX, posY, layoutLine, cursorOut.paraIdx))
        {
            // active paragraph
            XTextParagraph& textParagraph = m_textLayout.at(cursorOut.paraIdx);

            _XNum textBegin, textEnd, lineHeight;

            // line metrics
            _getLineMetrics(originX, originY, layoutLine, textBegin, textEnd, lineHeight, textParagraph.isRTL);

            // find glyph position
            _findGlyphFromLine(posX - textBegin, textParagraph, layoutLine, cursorOut.textPos);

            return true;
        }

        return false;
    }

    void _findGlyphFromLine(_XNum offsetX, XTextParagraph& textParagraph, XLayoutLine& layoutLine, XTextCursor& glyphPos)
    {
        // return first glyph if offset is before line start
        if(offsetX < 0)
        {
            // line beginning
            glyphPos = layoutLine.begin;
            return;
        }

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

        // return last glyph if offset is over line width
        if(offsetX > _getLineWidth(layoutLine))
        {
            // last glyph in line
            glyphPos = layoutLine.end;
            if(glyphPos.runOffset > 0) glyphPos.runOffset--;
            return;
        }

        _XNum charPosX = 0;

        // loop over all paint runs
        for(unsigned int paintRunIdx = 0; paintRunIdx < layoutLine.paintRuns.size(); ++paintRunIdx)
        {
            // active paint run
            const XTextPaintRun& paintRun = layoutLine.paintRuns.at(paintRunIdx);

            // set active run
            glyphPos.runIdx = paintRun.runIdx;

            // check if point is inside run
            if(offsetX >= charPosX && offsetX < charPosX + paintRun.width)
            {
                // check just in case
                if(paintRun.runIdx < 0 || paintRun.runIdx >= textParagraph.runCaches.size())
                {
                    XWASSERT(false);
                    break;
                }

                // get corresponding cache
                const _XTextRunCache& runCache = textParagraph.runCaches.at(paintRun.runIdx);

                // find glyph
                for(unsigned int glyphIdx = 0; glyphIdx < paintRun.glyphCount; ++glyphIdx)
                {
                    _XNum glyphWidth = 0;

                    // get glyph width
                    if(layoutLine.justify && paintRun.justifyPtr)
                        glyphWidth = paintRun.justifyPtr[glyphIdx];
                    else
                        glyphWidth = runCache.place.advances.at(paintRun.glyphOffset + glyphIdx);

                    // check if point is inside glyph
                    if(offsetX >= charPosX && offsetX < charPosX + glyphWidth)
                    {
                        // set offset
                        glyphPos.runOffset = paintRun.glyphOffset + glyphIdx;

                        // glyph found
                        return;
                    }

                    // next glyph
                    charPosX += glyphWidth;
                }
            }

            // next paint run
            charPosX += paintRun.width;
        }

        // glyph not found so use last glyph in line
        glyphPos = layoutLine.end;
        if(glyphPos.runOffset > 0) glyphPos.runOffset--;
    }

    // find global text position from cursor
    size_t _textPosFromCursor(const XGlyphCursor& glyphCursor)
    {
        // ignore if out of range
        if(glyphCursor.paraIdx >= m_textLayout.size()) return 0;

        // active paragraph
        XTextParagraph& textParagraph = m_textLayout.at(glyphCursor.paraIdx);

        // check if we have run caches already
        if(textParagraph.runCaches.size() == 0)
        {
            // generate shape and position for runs
            _shapeAndPostionRuns(textParagraph.textRuns, textParagraph.runCaches);
        }

        // ignore if run is our of range
        if(glyphCursor.textPos.runIdx >= textParagraph.textRuns.size()) return 0;

        // active text run
        const _XTextRun& textRun = textParagraph.textRuns.at(glyphCursor.textPos.runIdx);

        // text position
        return textRun.range.pos + glyphCursor.textPos.runOffset;
    }

    // find global glyph position from cursor
    size_t _glyphOffsetFromCursor(const XGlyphCursor& glyphCursor)
    {
        // ignore if out of range
        if(glyphCursor.paraIdx >= m_textLayout.size()) return 0;

        size_t glyphPos = 0;

        // active paragraph
        XTextParagraph& textParagraph = m_textLayout.at(glyphCursor.paraIdx);

        // check if we have run caches already
        if(textParagraph.runCaches.size() == 0)
        {
            // generate shape and position for runs
            _shapeAndPostionRuns(textParagraph.textRuns, textParagraph.runCaches);
        }

        // append all glyph counts
        for(unsigned int runIdx = 0; runIdx <= glyphCursor.textPos.runIdx && runIdx < textParagraph.runCaches.size(); ++runIdx)
        {
            // check if we found position
            if(runIdx == glyphCursor.textPos.runIdx)
            {
                // append only offset
                glyphPos += glyphCursor.textPos.runOffset;

                // stop search
                return glyphPos;

            } else
            {
                // append whole run
                glyphPos += textParagraph.runCaches.at(runIdx).shape.glyphs.size();
            }
        }

        return glyphPos;
    }

    // make cursor position from global glyph offset
    XGlyphCursor _cursorFromGlyphOffset(const XGlyphCursor& glyphCursor, size_t glyphOffset)
    {
        // init return cursor
        XGlyphCursor retCur = glyphCursor;

        // ignore if out of range
        if(glyphCursor.paraIdx >= m_textLayout.size()) return retCur;

        // active paragraph
        XTextParagraph& textParagraph = m_textLayout.at(glyphCursor.paraIdx);

        // check if we have run caches already
        if(textParagraph.runCaches.size() == 0)
        {
            // generate shape and position for runs
            _shapeAndPostionRuns(textParagraph.textRuns, textParagraph.runCaches);
        }

        // loop over all glyph counts
        for(retCur.textPos.runIdx = 0; glyphOffset > 0 && retCur.textPos.runIdx < textParagraph.runCaches.size(); ++retCur.textPos.runIdx)
        {
            // check if whole run can be substracted
            if(glyphOffset >= textParagraph.runCaches.at(retCur.textPos.runIdx).shape.glyphs.size())
            {
                // substract
                glyphOffset -= textParagraph.runCaches.at(retCur.textPos.runIdx).shape.glyphs.size();

            } else
            {
                // we found final position
                retCur.textPos.runOffset = (unsigned int)glyphOffset;

                // stop search
                return retCur;
            }
        }

        return retCur;
    }

protected: // selection helpers
    void _updateSelectionRange(_XNum originX, _XNum originY, _XNum fromX, _XNum fromY, _XNum toX, _XNum toY)
    {
        // reset previous selection
        m_selectionBegin.paraIdx = 0;
        m_selectionBegin.textPos.runIdx = 0;
        m_selectionBegin.textPos.runOffset = 0;
        m_selectionEnd = m_selectionBegin;

        // ignore if no layout
        if(m_textLayout.size() == 0) return;

        // ignore if no difference
        if(fromX == toX && fromY == toY) return;

        // copy points
        _XNum selectionStartX, selectionStartY, selectionEndX, selectionEndY;

        // swap points if needed
        if(fromY > toY || ((fromY == toY) && (fromX > toX)))
        {
            // swap points
            selectionStartX = toX; selectionStartY = toY; 
            selectionEndX = fromX; selectionEndY = fromY; 

        } else
        {
            // copy normaly
            selectionStartX = fromX; selectionStartY = fromY; 
            selectionEndX = toX; selectionEndY = toY; 
        }

        // flags
        bool selectionFound = false;

        // ignore if selection starts and ends above layout
        if(selectionStartY < originY && selectionEndY < originY) return;

        // paint position
        _XNum charPosX = originX;
        _XNum charPosY = originY;

        // find points
        for(unsigned int paraIdx = 0; paraIdx < m_textLayout.size() && !selectionFound; ++paraIdx)
        {
            // active text paragraph
            XTextParagraph& textParagraph = m_textLayout.at(paraIdx);

            // loop over layout lines
            for(unsigned int layoutLineIdx = 0; layoutLineIdx < textParagraph.layoutLines.size() && !selectionFound; ++layoutLineIdx)
            {
                // active layout line
                XLayoutLine& layoutLine = textParagraph.layoutLines.at(layoutLineIdx);

                _XNum lineHeight, paintRight;

                // line metrics
                _getLineMetrics(originX, originY, layoutLine, charPosX, paintRight, lineHeight, textParagraph.isRTL);

                // check if both points are on the same line
                if(selectionStartY >= charPosY && selectionStartY <= charPosY + lineHeight &&
                   selectionEndY >= charPosY && selectionEndY <= charPosY + lineHeight)
                {
                    // no selection if both points are after line text
                    if(selectionStartX > paintRight && selectionEndX > paintRight) return;

                    // no selection if both points are before line text
                    if(selectionStartX < charPosX && selectionEndX < charPosX) return;

                    // set line index for selection
                    m_selectionBegin.paraIdx = paraIdx;
                    m_selectionEnd.paraIdx = paraIdx;

                    // find selection
                    _findGlyphFromLine(selectionStartX - charPosX, textParagraph, layoutLine, m_selectionBegin.textPos);
                    _findGlyphFromLine(selectionEndX - charPosX, textParagraph, layoutLine, m_selectionEnd.textPos);

                    // stop search
                    selectionFound = true;
                    break;
                }

                // check if selection start is on this line
                if(selectionStartY >= charPosY && selectionStartY <= charPosY + lineHeight)
                {
                    // check if selection is inside text
                    if(selectionStartX <= paintRight)
                    {
                        // init line
                        m_selectionBegin.paraIdx = paraIdx;

                        // find glyph position
                        _findGlyphFromLine(selectionStartX - charPosX, textParagraph, layoutLine, m_selectionBegin.textPos);

                    } else
                    {
                        // selection start is after text so just move selection start point to next line
                        selectionStartX = originX;
                        selectionStartY += lineHeight;
                    }
                }

                // check if selection end is on this line
                if(selectionEndY >= charPosY && selectionEndY <= charPosY + lineHeight)
                {
                    // init line
                    m_selectionEnd.paraIdx = paraIdx;

                    // find glyph position
                    _findGlyphFromLine(selectionEndX - charPosX, textParagraph, layoutLine, m_selectionEnd.textPos);

                    // stop search
                    selectionFound = true;
                    break;
                }

                // next line
                charPosY += lineHeight;
            }
        }

        // check if selection ends below layout
        if(!selectionFound)
        {
            // do nothing if it also starts there
            if(selectionStartY > charPosY) return;

            // select last glyph
            if(m_textLayout.size() > 0 && m_textLayout.back().layoutLines.size() > 0)
            {
                m_selectionEnd.paraIdx = (int) m_textLayout.size() - 1;
                m_selectionEnd.textPos = m_textLayout.back().layoutLines.back().end;
                m_selectionEnd.textPos.runOffset--;

            } else
            {
                XWASSERT(false);
                return;
            }
        }

        // update flag
        m_hasSelection = true; 

        // store selection glyph positions in case style will change for selected text
        m_selectionStartGlyph = _glyphOffsetFromCursor(m_selectionBegin);
        m_selectionEndGlyph = _glyphOffsetFromCursor(m_selectionEnd);

        // reset selection style changed flag
        m_selectionStyleChanged = false;
    }

    void _updateSelection()
    {
        // loop over all paragraphs
        for(unsigned int paraIdx = 0; paraIdx < m_textLayout.size(); ++paraIdx)
        {
            // active paragraph
            XTextParagraph& textParagraph = m_textLayout.at(paraIdx);

            // just reset selection if global flag is not set
            if(!m_hasSelection)
            {
                textParagraph.hasSelection = false;
                textParagraph.selectionBegin.runIdx = 0;
                textParagraph.selectionBegin.runOffset = 0;
                textParagraph.selectionEnd = textParagraph.selectionBegin;
                continue;
            }

            // check if paragraph is inside selection
            if(paraIdx >= m_selectionBegin.paraIdx && paraIdx <= m_selectionEnd.paraIdx)
            {
                // set flag
                textParagraph.hasSelection = true;

                // selection start
                if(paraIdx == m_selectionBegin.paraIdx)
                {
                    // copy start position
                    textParagraph.selectionBegin = m_selectionBegin.textPos;

                } else
                {
                    // start from the beginning
                    textParagraph.selectionBegin.runIdx = 0;
                    textParagraph.selectionBegin.runOffset = 0;
                }

                // selection end
                if(paraIdx == m_selectionEnd.paraIdx)
                {
                    // copy start position
                    textParagraph.selectionEnd = m_selectionEnd.textPos;

                } else
                {
                    // check if we have run caches already
                    if(textParagraph.runCaches.size() == 0)
                    {
                        // generate shape and position for runs
                        _shapeAndPostionRuns(textParagraph.textRuns, textParagraph.runCaches);
                    }

                    // select up to end of line
                    if(textParagraph.runCaches.size() > 0 && textParagraph.runCaches.back().shape.glyphs.size() > 0)
                    {
                        textParagraph.selectionEnd.runIdx = (int)textParagraph.runCaches.size() - 1;
                        textParagraph.selectionEnd.runOffset = (int)textParagraph.runCaches.back().shape.glyphs.size() - 1;

                    } else
                    {
                        XWASSERT(false);
                        textParagraph.selectionEnd.runIdx = textParagraph.selectionBegin.runIdx;
                        textParagraph.selectionEnd.runOffset = textParagraph.selectionBegin.runOffset;
                    }
                }

            } else
            {
                // reset selection flag for line
                textParagraph.hasSelection = false;
            }
        }
    }

protected: // layout data
    std::vector<XTextParagraph> m_textLayout;
    XRichText*                  m_richText;

protected: // layout properties
    _XNum           m_layoutWidth;
    _XNum           m_linePaddingBefore;
    _XNum           m_linePaddingAfter;
    bool            m_wordWrap;
    bool            m_singleLineMode;
    TTextAlignment  m_textAlignment;

protected: // selection
    XGlyphCursor    m_selectionBegin;
    XGlyphCursor    m_selectionEnd;
    bool            m_hasSelection;
    bool            m_selectionActive;
    bool            m_selectionStyleChanged;
    size_t          m_selectionStartGlyph;
    size_t          m_selectionEndGlyph;

protected: // colors
    bool            m_bFillBackground;
    COLORREF        m_clText;
    COLORREF        m_clBackground;
    COLORREF        m_clSelectionText;
    COLORREF        m_clSelectionBackground;
    bool            m_useSelectionTextColor;
};

// XTextLayoutBaseT
/////////////////////////////////////////////////////////////////////

#endif // _XTEXTLAYOUTBASE_H_

