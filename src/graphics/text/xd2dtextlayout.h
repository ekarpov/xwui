// Direct2D based text layout and rendering
//
/////////////////////////////////////////////////////////////////////

#ifndef _XD2DTEXTLAYOUT_H_
#define _XD2DTEXTLAYOUT_H_

/////////////////////////////////////////////////////////////////////
// direct write headers
#include <dwrite.h>

// text layout base functionality
#include "xtextlayoutbase.h"

/////////////////////////////////////////////////////////////////////
// XD2DTextLayout - Direct2D based text layout

class XD2DTextLayout : public XTextLayoutBaseT<FLOAT, XDWriteHelpers::XDwTextRun, XDWriteHelpers::XDwTextRunCache, XD2DTextLayout>
{
public: // construction/destruction
    XD2DTextLayout();
    ~XD2DTextLayout();

public: // layout size
    int     contentWidth();
    int     contentHeight();
    void    resize(int layoutWidth);

public: // size hints
    int     getHeightForWidth(int width);

public: // line properties
    bool    getLineMetrics(int lineIdx, int& textBegin, int& textEnd, int& lineHeight);
    bool    getLineFitPos(int lineIdx, int maxLineWidth, int& textPos);

public: // line padding 
    void    setLinePadding(int beforeLine, int afterLine);
    void    getLinePadding(int& beforeLine, int& afterLine) const;

public: // hit testing
    bool    isInsideText(int originX, int originY, int posX, int posY);
    bool    isInsideSelection(int originX, int originY, int posX, int posY);
    bool    getTextFromPos(int originX, int originY, int posX, int posY, unsigned int& textPos);

public: // layout regions
    XRectRegion getTextRegion(int originX, int originY, unsigned int textPos, unsigned int textLength);

public: // selection
    bool    selectTo(int originX, int originY, int selectFomX, int selectFromY, int selectToX, int selectToY);

public: // painting
    void    onPaintD2D(int originX, int originY, ID2D1RenderTarget* pTarget, const RECT& rcPaint); 

public: // Direct2D resource caching 
    void    onInitD2DTarget(ID2D1RenderTarget* pTarget);
    void    onResetD2DTarget();
    void    setD2DResourcesCache(XD2DResourcesCache* pXD2DResourcesCache);

private: // protect from copy and assignment
    XD2DTextLayout(const XD2DTextLayout& ref)  {}
    const XD2DTextLayout& operator=(const XD2DTextLayout& ref) { return *this;}

protected: // region creation interface ( required by XTextLayoutBaseT)
    XRectRegion createRegionFromPoints(FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2);

private: // layout building interface (required by XTextLayoutBaseT)
    void    analyseRichText(const XRichText* richText, const XTextRange& range, std::vector<XDWriteHelpers::XDwTextRun>& runsOut);
    void    layoutTextRuns(std::vector<XDWriteHelpers::XDwTextRun>& textRuns);
    void    shapeAndPostionTextRun(XDWriteHelpers::XDwTextRun& textRun, XDWriteHelpers::XDwTextRunCache& runCache);
    void    getStyleMetrics(const XTextStyle& style, FLOAT& fontHeight, FLOAT& fontAscent);
    void    getInlineObjectMetrics(XDWriteHelpers::XDwTextRun& textRun, FLOAT& objectHeight, FLOAT& objectWidth);
    void    getRunLogicalAttrs(const XDWriteHelpers::XDwTextRun& textRun, XDWriteHelpers::XDwTextRunCache& runCache);
    void    mapGlyphsToChars(const XDWriteHelpers::XDwTextRun& textRun, XDWriteHelpers::XDwTextRunCache& runCache);
    int     getCharJustification(const XDWriteHelpers::XDwTextRunCache& runCache, unsigned int glyphIdx);

private: // layout helpers
    void    _releaseFontCache();

private: // paint helpers
    void    _releasePaintCache();
    FLOAT   _paintLayoutLine(ID2D1RenderTarget* pTarget, FLOAT originX, FLOAT originY, const D2D1_RECT_F& rcPaint, 
                             XTextParagraph& textParagraph, XLayoutLine& layoutLine);
    void    _fillRect(ID2D1RenderTarget* pTarget, const D2D1_RECT_F& fillRect, const COLORREF& color);

private: // cache helpers
    XDWriteHelpers::XDwFontData&    _getFontDataCache(const XTextStyle& style);
    ID2D1Brush*                     _getColorBrush(ID2D1RenderTarget* pTarget, const COLORREF& brushColor);

private: // cache types
    typedef std::unordered_map<xstyle_index_t, XDWriteHelpers::XDwFontData> XDwFontCache;
    typedef XD2DHelpers::XD2DColorBrushCache                                XDwBrushCache;

private: // caches
    XDwFontCache            m_fontCache;
    XDwBrushCache           m_brushCache;
    XD2DResourcesCache*     m_pXD2DResourcesCache;
};

// XD2DTextLayout
/////////////////////////////////////////////////////////////////////

#endif // _XD2DTEXTLAYOUT_H_

