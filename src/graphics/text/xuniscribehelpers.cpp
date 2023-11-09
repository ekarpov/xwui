// Uniscribe helper functions
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"

#include "xgdifonts.h"
#include "xrichtext.h"
#include "xuniscribehelpers.h"

using namespace XUniscribeHelpers;

// loop protecton constant
#define XUNISCRIBE_HELPERS_DUMMY_COUNT      1024

/////////////////////////////////////////////////////////////////////
// global data
static SCRIPT_CONTROL   s_XUniscribeScriptControl;
static bool             s_XUniscribeScriptControlInitialized = false;
static SCRIPT_STATE     s_XUniscribeScriptState;
static bool             s_XUniscribeScriptStateInitialized = false;

/////////////////////////////////////////////////////////////////////
// XUniscribeHelpers

// global script control parameters
SCRIPT_CONTROL* _scriptControl()
{
    // init default if needed
    if(!s_XUniscribeScriptControlInitialized)
    {
        // reset all
        ::ZeroMemory(&s_XUniscribeScriptControl, sizeof(SCRIPT_CONTROL));

        // NOTE: set fMergeNeutralItems to reduce number of runs from ScriptItemize
        //       http://stackoverflow.com/questions/6822170/how-to-reduce-the-number-of-runs-returned-from-uniscribe-scriptitemize

        // common options
        s_XUniscribeScriptControl.fMergeNeutralItems = 1;

        // TODO: read registry and system settings

        // mark as initialized
        s_XUniscribeScriptControlInitialized = true;
    }

    return &s_XUniscribeScriptControl;
}

// global script state parameters
SCRIPT_STATE* _scriptState()
{
    // init default if needed
    if(!s_XUniscribeScriptStateInitialized)
    {
        // reset all
        ::ZeroMemory(&s_XUniscribeScriptState, sizeof(SCRIPT_STATE));
        s_XUniscribeScriptState.uBidiLevel = 0; // assume left to right by default

        // TODO: read registry and system settings

        // mark as initialized
        s_XUniscribeScriptStateInitialized = true;
    }

    return &s_XUniscribeScriptState;
}

/////////////////////////////////////////////////////////////////////
// helper functions
void _initFontData(HDC hdc, const XTextStyle& style, XUniFontData& fontCache)
{
    // get font from GDI cache
    fontCache.hFont = XGdiFonts::getGDIFont(style);

    // select font to device context
    ::SelectObject(hdc, fontCache.hFont);

    // get text metrics
    TEXTMETRIC tm;
    ::ZeroMemory( &tm, sizeof( TEXTMETRIC ) );
    ::GetTextMetrics( hdc, &tm );    

    // font properties
    fontCache.fontAscent = tm.tmAscent;    
    fontCache.fontHeight = tm.tmHeight;
}

/////////////////////////////////////////////////////////////////////
// split text into runs based on text style and script
/////////////////////////////////////////////////////////////////////
void XUniscribeHelpers::analyseRichText(const XRichText* richText, const XTextRange& range, std::vector<XUniTextRun>& runsOut,
                            const SCRIPT_CONTROL* scriptCtrl, const SCRIPT_STATE* scriptState)
{
    XWASSERT(richText);

    // check input
    if(richText == 0 || range.length == 0) 
    {
        XWTRACE("XUniscribeHelpers::analyseRichText: input is not valid");
        return;
    }

    // validate text range
    if(range.pos >= richText->textLength() || range.pos + range.length > richText->textLength())
    {
        XWTRACE("XUniscribeHelpers::analyseRichText: text range is not valid");
        return;
    }

    // get text string
    const wchar_t* text = richText->data() + range.pos;

    // ignore if no text
    if(text == 0)
    {
        XWTRACE("XUniscribeHelpers::analyseRichText: no text to analyse");
        return;
    }

    // init state and control if needed
    if(scriptCtrl == 0) scriptCtrl = _scriptControl();
    if(scriptState == 0) scriptState = _scriptState();

    std::vector<SCRIPT_ITEM> scriptItems;
    scriptItems.resize(16); // initial size (lower numbers seemed to be always rejected even if there are fewer return items)

    ///// 1. Break text into script items (dummyIndex is used to avoid possible infinite loop)
    HRESULT hr = E_OUTOFMEMORY;
    for(int dummyIndex = 0; dummyIndex < XUNISCRIBE_HELPERS_DUMMY_COUNT; ++dummyIndex)
    {
        int itemsCount = 0;

        // NOTE: keep max items for ScriptItemize one item smaller to avoid 
        //       possible buffer overruns

        // try to split text into scipt items
        hr = ::ScriptItemize(text, range.length, (int)scriptItems.size() - 1,
                             scriptCtrl, scriptState, scriptItems.data(), &itemsCount);

        // check result
        if(SUCCEEDED(hr))
        {
            // resize items buffer to keep only valid items (itemsCount + array end marker)
            scriptItems.resize(itemsCount + 1);

            // stop loop
            break;

        } else if(hr == E_OUTOFMEMORY)
        {
            // resize items buffer and try once again
            scriptItems.resize(2 * scriptItems.size());
        }
    }

    // report error if any
    if(FAILED(hr))
    {
        XWTRACE_HRES("XUniscribeHelpers::analyseRichText failed to itemize text", hr);
        return;
    }

    // validate data
    if(scriptItems.front().iCharPos > (int)range.pos || 
       scriptItems.back().iCharPos - scriptItems.front().iCharPos < (int)range.length)
    {
        XWTRACE("XUniscribeHelpers::analyseRichText: script items returned by ScriptItemize are not valid");
        return;
    }

    // global Unsicribe script properties table
    const SCRIPT_PROPERTIES**   scriptPropList = 0;
    int                         scriptPropCount = 0;

    // get global properties
    hr = ::ScriptGetProperties(&scriptPropList, &scriptPropCount);

    if(FAILED(hr))
    {
        // report
        XWTRACE_HRES("XUniscribeHelpers::analyseRichText failed to get global Uniscribe "
                     "script properties, all scripts will be handled as complex", hr);

        // reset counter
        scriptPropCount = 0;
    }

    ///// 2. Merge script items and style runs into combined runs
    unsigned int scriptIndex = 0;
    unsigned int textPos = range.pos;
    while(textPos < range.pos + range.length)
    {
        XUniTextRun uniTextRun;
        uniTextRun.range.pos = textPos;
        uniTextRun.isInlineObject = false;
        uniTextRun.isRTL = false;
        uniTextRun.isComplex = true; // set as complex by default

        // get text run
        textPos = richText->getTextRun(textPos, uniTextRun.style, uniTextRun.isInlineObject, range.length - (textPos - range.pos));

        // check if text position is over maximum
        if(textPos > range.pos + range.length)
        {
            textPos = range.pos + range.length;
        }

        // update range
        uniTextRun.range.length = textPos - uniTextRun.range.pos;

        // sanity check
        XWASSERT(textPos > uniTextRun.range.pos);
        if(textPos <= uniTextRun.range.pos) break;

        // copy script properties to text run
        uniTextRun.scriptProps = scriptItems.at(scriptIndex).a;

        // global properties index
        int scriptPropIndex = scriptItems.at(scriptIndex).a.eScript;

        // set script properties
        if(scriptPropIndex < scriptPropCount)
        {
            uniTextRun.isComplex = scriptPropList[scriptPropIndex]->fComplex;
        }

        // check if script is RTL
        uniTextRun.isRTL = (uniTextRun.scriptProps.fRTL != 0);

        // check if ranges overlap
        if(scriptIndex < scriptItems.size() - 1 && 
              uniTextRun.range.pos + uniTextRun.range.length >= range.pos + (unsigned int)scriptItems.at(scriptIndex + 1).iCharPos)
        {
            // update length
            uniTextRun.range.length = range.pos + scriptItems.at(scriptIndex + 1).iCharPos - uniTextRun.range.pos;

            // jumpt to next script
            ++scriptIndex;

            // update text position
            textPos = uniTextRun.range.pos + uniTextRun.range.length;
        }

        // add combined text run
        runsOut.push_back(uniTextRun);
    }
}

/////////////////////////////////////////////////////////////////////
// re-order text runs in visual order
/////////////////////////////////////////////////////////////////////
void XUniscribeHelpers::layoutTextRuns(std::vector<XUniTextRun>& uniTextRuns)
{
    std::vector<int> logicalToVisual;
    int bidiCount = 0;

    // fill embedding levels
    std::vector<BYTE> bidiLevels;
    for(unsigned int runIdx = 0; runIdx < uniTextRuns.size(); ++runIdx)
    {
        // append bidi levels
        bidiLevels.push_back(uniTextRuns.at(runIdx).scriptProps.s.uBidiLevel);

        // update total bidi count
        bidiCount += uniTextRuns.at(runIdx).scriptProps.s.uBidiLevel;
    }

    // do not reorder if there is only one RTL segment
    if(bidiCount < 2) return;

    // reserve space for output indexes
    logicalToVisual.resize(bidiLevels.size());

    // layout items
    HRESULT hr = ::ScriptLayout((int)bidiLevels.size(), bidiLevels.data(), 0, logicalToVisual.data());
    if(FAILED(hr))
    {
        // ignore reordering
        XWTRACE_HRES("XUniscribeHelpers::layoutTextRuns failed to layout text runs", hr);
        return;
    }

    // reorder items in visual order
    std::vector<XUniTextRun> tmpRuns;
    for(unsigned int runIdx = 0; runIdx < logicalToVisual.size(); ++runIdx)
    {
        // sanity check
        if(logicalToVisual.at(runIdx) < 0 || logicalToVisual.at(runIdx) >= (int)uniTextRuns.size())
        {
            XWTRACE("XUniscribeHelpers::layoutTextRuns: runs reordering failed because ScriptLayout returned not valid data");
            return;
        }

        // copy in visual order
        tmpRuns.push_back(uniTextRuns.at(logicalToVisual.at(runIdx)));
    }
    
    // sanity check
    if(tmpRuns.size() != uniTextRuns.size())
    {
        XWTRACE("XUniscribeHelpers::layoutTextRuns failed because ScriptLayout returned not valid data");
        return;
    }

    // return items in visual order
    uniTextRuns = tmpRuns;
}

/////////////////////////////////////////////////////////////////////
// generate shape data from script
/////////////////////////////////////////////////////////////////////
bool XUniscribeHelpers::shapeText(HDC hdc, const XRichText* richText, XUniTextRun& textRun,
                                  XUniFontData& fontCache, XUniScriptShape& scriptShape, bool forceShape)
{
    XWASSERT(hdc);
    XWASSERT(richText);
    XWASSERT(textRun.range.pos < richText->textLength());
    XWASSERT(textRun.range.pos + textRun.range.length <= richText->textLength());

    // check input
    if(hdc == 0 || richText == 0 || 
       textRun.range.pos >= richText->textLength() ||
       textRun.range.pos + textRun.range.length > richText->textLength())
    {
        XWTRACE("XUniscribeHelpers::shapeText: input is not valid");
        return false;
    }

    // ignore empty runs
    if(textRun.range.length == 0)
    {
        XWTRACE("XUniscribeHelpers::shapeText: empty run ignored");
        return false;
    }

    // check if we need to init cache
    if(fontCache.hFont == 0)
    {
        _initFontData(hdc, textRun.style, fontCache);
    }

    // text for run
    const wchar_t* text = richText->data() + textRun.range.pos;

    // initial sizes for shape
    scriptShape.glyphs.resize(textRun.range.length * 3 / 2 + 16); // recommended in ScriptShape docs
    scriptShape.scriptAttrs.resize(scriptShape.glyphs.size());
    scriptShape.logCluster.resize(textRun.range.length);

    // shape glyphs (dummyIndex is used to avoid possible infinite loop)
    HDC tmpHdc = (fontCache.scriptCache != 0) ? 0 : hdc;
    for(int dummyIndex = 0; dummyIndex < XUNISCRIBE_HELPERS_DUMMY_COUNT; ++dummyIndex)
    {
        int glyphCount = 0;

        // shape 
        HRESULT hr = ::ScriptShape(tmpHdc, &fontCache.scriptCache, text, textRun.range.length, (int)scriptShape.glyphs.size(),
            &textRun.scriptProps, scriptShape.glyphs.data(), scriptShape.logCluster.data(), scriptShape.scriptAttrs.data(), &glyphCount);

        // check result
        if(SUCCEEDED(hr))
        {
            // resize buffers
            scriptShape.glyphs.resize(glyphCount);
            scriptShape.scriptAttrs.resize(glyphCount);

            // mark script as complex if number of glyhps is different from number of characters
            if(!textRun.isComplex && scriptShape.glyphs.size() != textRun.range.length)
            {
                textRun.isComplex = true;
            }

            // stop
            break;

        } else if(hr == E_OUTOFMEMORY)
        {
            // reserve more space
            scriptShape.scriptAttrs.resize(scriptShape.glyphs.size() * 2);
            scriptShape.glyphs.resize(scriptShape.glyphs.size() * 2);

        } else if(hr == E_PENDING && tmpHdc == 0)
        {
            // cache doesn't have enough information, hdc if needed
            tmpHdc = hdc;

            // select font to device context
            ::SelectObject(hdc, fontCache.hFont);

        } else if(hr == USP_E_SCRIPT_NOT_IN_FONT)
        {
            // check if we already tried to change script
            if(textRun.scriptProps.eScript == SCRIPT_UNDEFINED)
            {
                // text run cannot be shaped, ignore
                XWTRACE("XUniscribeHelpers::shapeText failed to shape glyphs due to font and script missmatch");
                break;
            }

            // check if we must shape anyway
            if(forceShape)
            {
                // try to change script properties and continue
                textRun.scriptProps.eScript = SCRIPT_UNDEFINED;

            } else
            {
                // caller must to font fallback
                return false;
            }
        
        } else
        {
            // some other error
            XWTRACE_HRES("XUniscribeHelpers::shapeText failed to shape glyphs", hr);

            // stop 
            break;
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////
// generate position data for text run
/////////////////////////////////////////////////////////////////////
void XUniscribeHelpers::placeText(HDC hdc, const XUniScriptShape& scriptShape, XUniTextRun& textRun, 
                                  XUniFontData& fontCache, XUniScriptPlace& scriptPlace)
{
    XWASSERT(hdc);

    // check input
    if(hdc == 0)
    {
        XWTRACE("XUniscribeHelpers::placeText: input is not valid");
        return;
    }

    // ignore empty runs
    if(textRun.range.length == 0)
    {
        XWTRACE("XUniscribeHelpers::placeText: empty run ignored");
        scriptPlace.width = 0;
        return;
    }

    // check if we need to init cache
    if(fontCache.hFont == 0)
    {
        _initFontData(hdc, textRun.style, fontCache);
    }

    // resize buffers
    scriptPlace.advances.resize(scriptShape.glyphs.size());
    scriptPlace.offsets.resize(scriptShape.glyphs.size());

    // position glyphs (dummyIndex is used to avoid possible infinite loop)
    HDC tmpHdc = (fontCache.scriptCache != 0) ? 0 : hdc;
    for(int dummyIndex = 0; dummyIndex < XUNISCRIBE_HELPERS_DUMMY_COUNT; ++dummyIndex)
    {
        int glyphCount = 0;

        // place 
        HRESULT hr = ::ScriptPlace(tmpHdc, &fontCache.scriptCache, scriptShape.glyphs.data(), (int)scriptShape.glyphs.size(), 
            scriptShape.scriptAttrs.data(), &textRun.scriptProps, scriptPlace.advances.data(), scriptPlace.offsets.data(), &scriptPlace.abcOffsets);

        // check result
        if(SUCCEEDED(hr))
        {
            // compute total width
            scriptPlace.width = scriptPlace.abcOffsets.abcA + scriptPlace.abcOffsets.abcB + scriptPlace.abcOffsets.abcC;

            // stop
            break;

        } else if(hr == E_PENDING && tmpHdc == 0)
        {
            // cache doesn't have enough information, hdc if needed
            tmpHdc = hdc;

            // select font to device context
            ::SelectObject(hdc, fontCache.hFont);

        } else
        {
            // some other error
            XWTRACE_HRES("XUniscribeHelpers::placeText failed to place glyphs", hr);

            // stop 
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////
// get mapping from glyphs to characters for complex scripts
/////////////////////////////////////////////////////////////////////
void XUniscribeHelpers::mapGlyphsToChars(const XUniTextRun& textRun, const XUniScriptShape& scriptShape, 
                                         std::vector<unsigned int>& glyphToChar)
{
    // check if script is complex
    if(!textRun.isComplex && !textRun.scriptProps.fRTL && textRun.range.length == scriptShape.glyphs.size())
    {
        XWTRACE("XUniscribeHelpers::mapGlyphsToChars: glyph mapping is not needed for non complex scipts");
    }

    // validate input
    if(scriptShape.logCluster.size() != textRun.range.length)
    {
        XWASSERT(false);
        
        // fill with default values just to make sure it won't crash
        glyphToChar.resize(scriptShape.glyphs.size());

        // fill 
        std::fill(glyphToChar.begin(), glyphToChar.end(), 0);

        return;
    }

    // ignore if not input
    if(textRun.range.length == 0) return;

    // check script direction
    bool isRTL = textRun.scriptProps.fRTL;

    // NOTE: fill index in logical order and later rotate for RTL scripts

    // loop over all characters
    WORD glyphIdx = scriptShape.logCluster.at(0);
    unsigned int clusterCharIdx = 0;
    for(unsigned int idx = 1; idx < textRun.range.length; ++idx)
    {
        // glyph index is the same for the whole cluster
        if(glyphIdx != scriptShape.logCluster.at(idx))
        {
            // cluster size
            int clusterSize = scriptShape.logCluster.at(idx) - glyphIdx;

            // validate log cluster assumptions
            XWASSERT((isRTL && clusterSize < 0) || (!isRTL == clusterSize > 0));

            // adjust if negative
            if(clusterSize < 0) clusterSize = -clusterSize;

            // append index
            for(; clusterSize > 0; --clusterSize)
            {
                glyphToChar.push_back(clusterCharIdx);                
            }

            // copy current glyph index
            glyphIdx = scriptShape.logCluster.at(idx);

            // save first cluster character
            clusterCharIdx = idx;
        }
    }

    // fill the rest with the last character
    if(scriptShape.glyphs.size() > glyphToChar.size())
    {
        size_t fillOffset = glyphToChar.size();

        // reserve size
        glyphToChar.resize(scriptShape.glyphs.size());

        // fill
        std::fill(glyphToChar.begin() + fillOffset, glyphToChar.end(), clusterCharIdx);
    }

    // rotate result index if RTL
    if(isRTL)
    {
        // rotate
        std::reverse(glyphToChar.begin(), glyphToChar.end());
    }
}

/////////////////////////////////////////////////////////////////////
// get logical line attributes
/////////////////////////////////////////////////////////////////////
void XUniscribeHelpers::getRunLogicalAttrs(const XRichText* richText, const XUniTextRun& textRun, std::vector<SCRIPT_LOGATTR>& logAttrs)
{
    // reserve enough space
    logAttrs.resize(textRun.range.length);

    // validate text run
    if(richText == 0 || textRun.range.length == 0 || 
       textRun.range.pos >= richText->textLength() || 
       textRun.range.pos + textRun.range.length > richText->textLength())
    {
        XWASSERT(false);
        XWTRACE("XUniscribeHelpers::getRunLogicalAttrs input is not valid");
        return;
    }

    // get logical attributes from text
    HRESULT hr = ::ScriptBreak(richText->data() + textRun.range.pos, textRun.range.length, &textRun.scriptProps, logAttrs.data());

    // check result and fill default values if needed
    if(FAILED(hr))
    {
        XWTRACE_HRES("XUniscribeHelpers::getRunLogicalAttrs failed to break string", hr);

        // fill default values
        SCRIPT_LOGATTR defaultAttr;
        ::ZeroMemory(&defaultAttr, sizeof(SCRIPT_LOGATTR));

        // fill
        std::fill(logAttrs.begin(), logAttrs.end(), defaultAttr);
    }
}

/////////////////////////////////////////////////////////////////////
// release cache
/////////////////////////////////////////////////////////////////////
void XUniscribeHelpers::releaseCache(XUniFontData& fontCache)
{
    // free cache if any
    if(fontCache.scriptCache)
    {
        ::ScriptFreeCache(&fontCache.scriptCache);
        fontCache.scriptCache = 0;
    }

    // do not release font as it is shared by global cache
    fontCache.hFont = 0;
}

// XUniscribeHelpers
/////////////////////////////////////////////////////////////////////

