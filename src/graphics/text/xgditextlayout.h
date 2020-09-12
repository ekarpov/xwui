// Uniscribe based text layout and rendering
//
/////////////////////////////////////////////////////////////////////

#ifndef _XGDITEXTLAYOUT_H_
#define _XGDITEXTLAYOUT_H_

/////////////////////////////////////////////////////////////////////
// text layout base functionality
#include "xtextlayoutbase.h"

/////////////////////////////////////////////////////////////////////
// forward declarations
class XGdiResourcesCache;

/////////////////////////////////////////////////////////////////////
// XGdiTextLayout - Uniscribe based text layout

class XGdiTextLayout : public XTextLayoutBaseT<int, XUniscribeHelpers::XUniTextRun, XUniscribeHelpers::XUniTextRunCache, XGdiTextLayout>
{
public: // construction/destruction
    XGdiTextLayout();
    ~XGdiTextLayout();

public: // layout size
    int     contentWidth(HDC hdc);
    int     contentHeight(HDC hdc);

public: // paint properties
    void    enableDoubleBuffering(bool bEnable);

public: // size hints
    int     getHeightForWidth(HDC hdc, int width);
    int     getMaxTextForWidth(HDC hdc, int width, int lineIndex);

public: // line properties
    int     getLineCount(HDC hdc);
    bool    getLineMetrics(HDC hdc, int lineIdx, int& textBegin, int& textEnd, int& lineHeight);
    bool    getLineFitPos(HDC hdc, int lineIdx, int maxLineWidth, int& textPos);

public: // hit testing
    bool    isInsideText(HDC hdc, int originX, int originY, int posX, int posY);
    bool    isInsideSelection(HDC hdc, int originX, int originY, int posX, int posY);
    bool    getTextFromPos(HDC hdc, int originX, int originY, int posX, int posY, unsigned int& textPos);

public: // regions
    XRectRegion getTextRegion(HDC hdc, int originX, int originY, int textPos, int textLength);

public: // selection
    bool    selectTo(HDC hdc, int originX, int originY, int selectFomX, int selectFromY, int selectToX, int selectToY);

public: // painting
    void    onPaint(HDC hdc, int originX, int originY, const RECT& rcPaint);   

public: // GDI resource caching
    void    onInitGDIResources(HDC hdc);
    void    onResetGDIResources();
    void    setGDIResourcesCache(XGdiResourcesCache* pXGdiResourcesCache);

private: // protect from copy and assignment
    XGdiTextLayout(const XGdiTextLayout& ref)  {}
    const XGdiTextLayout& operator=(const XGdiTextLayout& ref) { return *this;}

protected: // region creation interface ( required by XTextLayoutBaseT)
    XRectRegion createRegionFromPoints(int x1, int y1, int x2, int y2);

protected: // layout building interface ( required by XTextLayoutBaseT)
    void    analyseRichText(const XRichText* richText, const XTextRange& range, std::vector<XUniscribeHelpers::XUniTextRun>& runsOut);
    void    layoutTextRuns(std::vector<XUniscribeHelpers::XUniTextRun>& textRuns);
    void    shapeAndPostionTextRun(XUniscribeHelpers::XUniTextRun& textRun, 
                                   XUniscribeHelpers::XUniTextRunCache& runCache);
    void    getStyleMetrics(const XTextStyle& style, int& fontHeight, int& fontAscent);
    void    getInlineObjectMetrics(XUniscribeHelpers::XUniTextRun& textRun, int& objectHeight, int& objectWidth);
    void    getRunLogicalAttrs(const XUniscribeHelpers::XUniTextRun& textRun, 
                               XUniscribeHelpers::XUniTextRunCache& runCache);
    void    mapGlyphsToChars(const XUniscribeHelpers::XUniTextRun& textRun, 
                             XUniscribeHelpers::XUniTextRunCache& runCache);
    void    justifyLayoutLine(XTextParagraph& textParagraph, XLayoutLine& layoutLine, int lineWidth);
    int     getCharJustification(const XUniscribeHelpers::XUniTextRunCache& runCache, unsigned int glyphIdx);

private: // layout helpers
    void    _releaseFontCache();

private: // paint helpers
    int     _paintLayoutLine(HDC hdc, int originX, int originY, const RECT& rcPaint, 
                             XTextParagraph& textParagraph, XLayoutLine& layoutLine);
    void    _updateObjectPositions(int offsetX, int offsetY, XTextParagraph& textParagraph, XLayoutLine& layoutLine);
    void    _fillBackground(HDC hdc, const RECT& rcPaint);
    void    _fillBackground(HDC hdc, int left, int right, int top, int bottom);

private: // style cache helpers
    XUniscribeHelpers::XUniFontData& _getFontDataCache(const XTextStyle& style);

private: // style cache type
    typedef std::unordered_map<xstyle_index_t, XUniscribeHelpers::XUniFontData>   XUniFontCache;

private: // style cache
    XUniFontCache   m_fontCache;
    HDC             m_hdc;

private: // data
    XGdiResourcesCache*     m_pXGdiResourcesCache;
    bool                    m_enableDoubleBuffer;
};

// XGdiTextLayout
/////////////////////////////////////////////////////////////////////

#endif // _XGDITEXTLAYOUT_H_


