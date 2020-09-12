// Direct Write helper functions
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"
#include "../xd2dhelpres.h"

#include "xrichtext.h"
#include "xdwfonts.h"
#include "xdwhelpers.h"

/////////////////////////////////////////////////////////////////////
// static data

// DLL instance
HINSTANCE g_hDirectWriteLibrary = 0;

// DirectWrite factory
IDWriteFactory* g_pIDWriteFactory = 0;

// DirectWrite text analyzer
IDWriteTextAnalyzer* g_pIDWriteTextAnalyzer = 0;

// export function
typedef HRESULT (WINAPI *DWriteCreateFactoryPtr)(DWRITE_FACTORY_TYPE, REFIID, IUnknown **);

DWriteCreateFactoryPtr g_pDWriteCreateFactoryFunc;

// loop protecton constant
#define XDIRECTWRITE_HELPERS_DUMMY_COUNT      1024

/////////////////////////////////////////////////////////////////////
// helper functions
void _initFontData(const XTextStyle& style, XDWriteHelpers::XDwFontData& fontCache)
{
    // load font
    fontCache.fontFace = XDWFonts::getDirectWriteFont(style);

    // ignore if no font set
    XWASSERT(fontCache.fontFace);
    if(fontCache.fontFace == 0) return;

    // update other properties
    fontCache.fontEmSize = (FLOAT)style.nFontSize;

    // use default value if not set
    if(fontCache.fontEmSize == 0) 
    {
        // get default font properties
        LOGFONT fnt;
        XWUtils::sGetDefaultFontProps(fnt);

        // NOTE: negative lfHeight represents the size of the em unit.

        // default size
        if(fnt.lfHeight < 0)
            fontCache.fontEmSize = (FLOAT)(-fnt.lfHeight);

        // as last resort use some pre-defined constant
        if(fontCache.fontEmSize <= 0) fontCache.fontEmSize = 12.0f;
    }

    // ignore if no EM set
    XWASSERT(fontCache.fontEmSize);
    if(fontCache.fontEmSize == 0) return;

    // get font metrics
    DWRITE_FONT_METRICS dwFontMetrics;
    fontCache.fontFace->GetMetrics(&dwFontMetrics);

    // conversion ratio
    FLOAT ratio = fontCache.fontEmSize / (FLOAT)dwFontMetrics.designUnitsPerEm;

    // copy metrics
    fontCache.fontAscent = dwFontMetrics.ascent * ratio;
    fontCache.fontHeight = (dwFontMetrics.ascent + dwFontMetrics.descent + dwFontMetrics.lineGap) * ratio;

    fontCache.underlinePosition = dwFontMetrics.underlinePosition * ratio;
    fontCache.underlineThickness = dwFontMetrics.underlineThickness * ratio;
    fontCache.strikethroughPosition = dwFontMetrics.strikethroughPosition * ratio;
    fontCache.strikethroughThickness = dwFontMetrics.strikethroughThickness * ratio;
}

/////////////////////////////////////////////////////////////////////
// XDWriteHelpers - DirectWrite helper functions

/////////////////////////////////////////////////////////////////////
// dynamic loading
/////////////////////////////////////////////////////////////////////
bool XDWriteHelpers::isDirectWriteLoaded()
{
    return (g_hDirectWriteLibrary != 0 && g_pDWriteCreateFactoryFunc != 0 && g_pIDWriteFactory != 0);
}

bool XDWriteHelpers::loadDirectWriteLibrary()
{
    // check if already loaded
    if(XDWriteHelpers::isDirectWriteLoaded()) return true;

    // load DirectWrite DLL
    g_hDirectWriteLibrary = ::LoadLibraryW(L"dwrite.dll");
    if(g_hDirectWriteLibrary == 0)
    {
        XWTRACE_WERR_LAST("XDWriteHelpers: Failed to load DirectWrite library");
        return false;
    }

    // locate factory function
    g_pDWriteCreateFactoryFunc = (DWriteCreateFactoryPtr)::GetProcAddress(g_hDirectWriteLibrary, "DWriteCreateFactory");
    if(g_pDWriteCreateFactoryFunc == 0)
    {
        XWTRACE_WERR_LAST("XDWriteHelpers: Failed to get factory function from DirectWrite library");
        ::FreeLibrary(g_hDirectWriteLibrary);
        g_hDirectWriteLibrary = 0;
        return false;
    }

    // create global factory
    HRESULT hr = g_pDWriteCreateFactoryFunc(DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory), (IUnknown**)&g_pIDWriteFactory);

    if(FAILED(hr))
    {
        XWTRACE_HRES("XD2DHelpers: Failed to create DirectWrite factory", hr);
        XDWriteHelpers::freeDirectWriteLibrary();
        return false;
    }

    return true;
}

void XDWriteHelpers::freeDirectWriteLibrary()
{
    // check if it has been loaded
    if(!XDWriteHelpers::isDirectWriteLoaded()) return;

    // release text analyzer if any
    if(g_pIDWriteTextAnalyzer)
    {
        g_pIDWriteTextAnalyzer->Release();
        g_pIDWriteTextAnalyzer = 0;
    }

    // release factory if any
    if(g_pIDWriteFactory)
    {
        g_pIDWriteFactory->Release();
        g_pIDWriteFactory = 0;
    }

    // free library
    if(!::FreeLibrary(g_hDirectWriteLibrary))
    {
        XWTRACE_WERR_LAST("XDWriteHelpers: Failed to release DirectWrite library");
    }

    // reset pointers
    g_hDirectWriteLibrary = 0;
    g_pDWriteCreateFactoryFunc = 0;
}

/////////////////////////////////////////////////////////////////////
// global factory
/////////////////////////////////////////////////////////////////////
IDWriteFactory* XDWriteHelpers::getDirectWriteFactory()
{
    // return global factory
    return g_pIDWriteFactory;
}

/////////////////////////////////////////////////////////////////////
// global text analyzer
/////////////////////////////////////////////////////////////////////
IDWriteTextAnalyzer* XDWriteHelpers::getDirectWriteTextAnalyzer()
{
    // create analyzer first if needed
    if(g_pIDWriteTextAnalyzer == 0 && g_pIDWriteFactory != 0)
    {
        // create
        HRESULT hr = g_pIDWriteFactory->CreateTextAnalyzer(&g_pIDWriteTextAnalyzer);

        // check result
        if(FAILED(hr) || g_pIDWriteTextAnalyzer == 0)
        {
            XWTRACE_HRES("XDWriteHelpers::getDirectWriteTextAnalyzer: failed to create DirectWrite text analyzer", hr);
        }
    }

    // return global text anayzer
    return g_pIDWriteTextAnalyzer;
}

/////////////////////////////////////////////////////////////////////
// split text into runs based on text style and script
/////////////////////////////////////////////////////////////////////
void XDWriteHelpers::analyseRichText(const XRichText* richText, const XTextRange& range, std::vector<XDwTextRun>& runsOut,
                            const wchar_t* localeName, bool isRTL)
{
    // ignore if not loaded
    if(g_pIDWriteFactory == 0)
    {
        XWTRACE("XDWriteHelpers::analyseRichText failed as DirectWrite is not loaded");
        return;
    }

    // set active locale if needed
    if(localeName == 0)
    {
        localeName = XWUILocale::getSelectedLocaleW();
    }

    // create text analyzer
    XDWriteAnalyse* dwTextAnalyse = new XDWriteAnalyse(g_pIDWriteFactory, localeName, 
        isRTL ? DWRITE_READING_DIRECTION_RIGHT_TO_LEFT : DWRITE_READING_DIRECTION_LEFT_TO_RIGHT);

    if(dwTextAnalyse == 0) return;

    dwTextAnalyse->AddRef();

    // get global text analysis interface
    IDWriteTextAnalyzer* textAnalyzer = getDirectWriteTextAnalyzer();
    if(textAnalyzer == 0) return;

    // analyse text
    std::vector<XDWriteAnalyse::XDwCharAnalysis> scriptAnalysis;
    dwTextAnalyse->analyseRichText(textAnalyzer, richText, range, scriptAnalysis);

    // validate output
    XWASSERT(scriptAnalysis.size() == range.length);
    if(scriptAnalysis.size() != range.length) return;

    // release analyser
    dwTextAnalyse->Release();
    dwTextAnalyse = 0;

    xstyle_index_t styleIndex = richText->styleIndexAt(range.pos);
    XDWriteAnalyse::XDwCharAnalysis runScript = scriptAnalysis.front();
    XTextRange runRange = range;
    runRange.length = 1;

    // merge styles and script properties
    for(unsigned int charPos = 1; charPos <= range.length; ++charPos)
    {
        // append style if end of range or change in style or script
        if(charPos == range.length || 
           richText->isInlineObjectAt(runRange.pos) ||
           richText->isInlineObjectAt(range.pos + charPos) ||
           styleIndex != richText->styleIndexAt(range.pos + charPos) ||
           scriptAnalysis.at(charPos).bidiLevel != runScript.bidiLevel ||
           scriptAnalysis.at(charPos).scriptAnalysis.script != runScript.scriptAnalysis.script ||
           scriptAnalysis.at(charPos).scriptAnalysis.shapes != runScript.scriptAnalysis.shapes)
        {
            // init run
            XDWriteHelpers::XDwTextRun dwTextRun;
            dwTextRun.bidiLevel = runScript.bidiLevel;
            dwTextRun.scriptProps = runScript.scriptAnalysis;
            dwTextRun.range = runRange;
            dwTextRun.isInlineObject = richText->isInlineObjectAt(runRange.pos);
            dwTextRun.style = richText->textStyle(runRange.pos);

            // NOTE: The implicit resolved bidi level of the run. Odd levels indicate right-to-left languages 
            //       like Hebrew and Arabic, while even levels indicate left-to-right languages like English 
            //       and Japanese (when written horizontally). For right-to-left languages, the text origin is 
            //       on the right, and text should be drawn to the left.
            dwTextRun.isRTL = ((runScript.bidiLevel % 2) != 0);

            // NOTE: DirectWrite doesn't provide same information as Unsicribe about
            //       script properties, e.g. is it complex or not. So we just
            //       assume here that at least RTL scripts are complex. This will be
            //       later refined when text is shaped
            dwTextRun.isComplex = dwTextRun.isRTL;

            // append to result
            runsOut.push_back(dwTextRun);

            // init next
            if(charPos < range.length)
            {
                styleIndex = richText->styleIndexAt(range.pos + charPos);
                runScript = scriptAnalysis.at(charPos);
                runRange.pos = range.pos + charPos;
                runRange.length = 1;
            }

        } else
        {
            // merge
            runRange.length++;
        }
    }
}

/////////////////////////////////////////////////////////////////////
// re-order text runs in visual order
/////////////////////////////////////////////////////////////////////
void XDWriteHelpers::layoutTextRuns(std::vector<XDwTextRun>& dwTextRuns)
{
    // NOTE: DirectWrite doesn't provide functionality similar to Uniscribe
    //       for run re-ordering, so in this version we just re-use Uniscribe

    std::vector<int> logicalToVisual;
    int bidiCount = 0;

    // fill embedding levels
    std::vector<BYTE> bidiLevels;
    for(unsigned int runIdx = 0; runIdx < dwTextRuns.size(); ++runIdx)
    {
        // append bidi levels
        bidiLevels.push_back(dwTextRuns.at(runIdx).bidiLevel);

        // update total bidi count
        ++bidiCount;
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
        XWTRACE_HRES("XDWriteHelpers::layoutTextRuns failed to layout text runs", hr);
        return;
    }

    // reorder items in visual order
    std::vector<XDwTextRun> tmpRuns;
    for(unsigned int runIdx = 0; runIdx < logicalToVisual.size(); ++runIdx)
    {
        // sanity check
        if(logicalToVisual.at(runIdx) < 0 || logicalToVisual.at(runIdx) >= (int)dwTextRuns.size())
        {
            XWTRACE("XDWriteHelpers::layoutTextRuns: runs reordering failed because ScriptLayout returned not valid data");
            return;
        }

        // copy in visual order
        tmpRuns.push_back(dwTextRuns.at(logicalToVisual.at(runIdx)));
    }
    
    // sanity check
    if(tmpRuns.size() != dwTextRuns.size())
    {
        XWTRACE("XDWriteHelpers::layoutTextRuns failed because ScriptLayout returned not valid data");
        return;
    }

    // return items in visual order
    dwTextRuns = tmpRuns;
}

/////////////////////////////////////////////////////////////////////
// shape and position helpers
bool    _XDWriteHelpersValidateInput(const XRichText* richText, const XDWriteHelpers::XDwTextRun& textRun, 
            XDWriteHelpers::XDwFontData& fontCache, const wchar_t*& localeName)
{
    // ignore if not loaded
    if(g_pIDWriteFactory == 0)
    {
        XWTRACE("_XDWriteHelpersValidateInput: failed as DirectWrite is not loaded");
        return false;
    }

    XWASSERT(richText);
    XWASSERT(textRun.range.pos < richText->textLength());
    XWASSERT(textRun.range.pos + textRun.range.length <= richText->textLength());

    // check input
    if(richText == 0 || 
       textRun.range.pos >= richText->textLength() ||
       textRun.range.pos + textRun.range.length > richText->textLength())
    {
        XWTRACE("_XDWriteHelpersValidateInput: input is not valid");
        return false;
    }

    // ignore empty runs
    if(textRun.range.length == 0)
    {
        XWTRACE("_XDWriteHelpersValidateInput: empty run ignored");
        return false;
    }

    // check if we need to init cache
    if(fontCache.fontFace == 0)
    {
        _initFontData(textRun.style, fontCache);

        // ignore if not possible to get font
        if(fontCache.fontFace == 0) return false;
    }

    // set active locale if needed
    if(localeName == 0)
    {
        localeName = XWUILocale::getSelectedLocaleW();
    }

    return true;
}

/////////////////////////////////////////////////////////////////////
// generate shape data for text run
/////////////////////////////////////////////////////////////////////
bool XDWriteHelpers::shapeText(const XRichText* richText, XDwTextRun& textRun, 
                      XDwFontData& fontCache, XDwScriptShape& scriptShape, bool forceShape, const wchar_t* localeName)
{
    // check input
    if(!_XDWriteHelpersValidateInput(richText, textRun, fontCache, localeName))
    {
        XWTRACE("XDWriteHelpers::shapeText failed to validate input");
        return false;
    }

    // text for run
    const wchar_t* text = richText->data() + textRun.range.pos;
    
    XWASSERT(text);
    if(text == 0) return false;

    // get global text analysis interface
    IDWriteTextAnalyzer* textAnalyzer = getDirectWriteTextAnalyzer();
    if(textAnalyzer == 0) return false;

    // initial sizes for shape
    scriptShape.glyphs.resize(3 * textRun.range.length / 2 + 16); // recommended in MSDN docs
    scriptShape.clusterMap.resize(textRun.range.length);
    scriptShape.textProps.resize(textRun.range.length);
    scriptShape.scriptAttrs.resize(scriptShape.glyphs.size());

    // shape glyphs (dummyIndex is used to avoid possible infinite loop)
    for(int dummyIndex = 0; dummyIndex < XDIRECTWRITE_HELPERS_DUMMY_COUNT; ++dummyIndex)
    {
        UINT32 glyphCount = 0;

        // get glyphs
        HRESULT hr = textAnalyzer->GetGlyphs(text, textRun.range.length, fontCache.fontFace, FALSE, textRun.isRTL, 
            &textRun.scriptProps, localeName, 0, 0, 0, 0, 
            (UINT32)scriptShape.glyphs.size(), scriptShape.clusterMap.data(), 
            scriptShape.textProps.data(), scriptShape.glyphs.data(), 
            scriptShape.scriptAttrs.data(), &glyphCount);

        // check result
        if(SUCCEEDED(hr))
        {
            // resize buffers
            scriptShape.glyphs.resize(glyphCount);
            scriptShape.scriptAttrs.resize(glyphCount);

            // check for missing indices
            if(!forceShape)
            {
                // NOTE: when characters are not present in the font this method returns the index 0,
                //       which is the undefined glyph or ".notdef" glyph.

                for(std::vector<UINT16>::const_iterator it = scriptShape.glyphs.begin();
                    it != scriptShape.glyphs.end(); ++it)
                {
                    if(*it == 0) return false;
                }
            }

            // mark script as complex if number of glyhps is different from number of characters
            if(!textRun.isComplex && scriptShape.glyphs.size() != textRun.range.length)
            {
                textRun.isComplex = true;
            }

            // stop
            break;

        } else if(hr == ERROR_INSUFFICIENT_BUFFER)
        {
            // reserve more space
            scriptShape.scriptAttrs.resize(scriptShape.glyphs.size() * 2);
            scriptShape.glyphs.resize(scriptShape.glyphs.size() * 2);

        } else
        {
            // some other error
            XWTRACE_HRES("XDWriteHelpers::shapeText failed to shape glyphs", hr);

            // stop 
            break;
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////
// generate position data for text run
/////////////////////////////////////////////////////////////////////
void XDWriteHelpers::placeText(const XRichText* richText, XDwTextRun& textRun, XDwFontData& fontCache,
                      XDwScriptShape& scriptShape, XDwScriptPlace& scriptPlace, const wchar_t* localeName)
{
    // check input
    if(!_XDWriteHelpersValidateInput(richText, textRun, fontCache, localeName))
    {
        XWTRACE("XDWriteHelpers::placeText: failed to validate input");
        return;
    }

    // ignore if no glyphs provided
    if(scriptShape.glyphs.size() == 0 ||
       scriptShape.clusterMap.size() == 0 ||
       scriptShape.textProps.size() == 0 ||
       scriptShape.scriptAttrs.size() == 0)
    {
        XWTRACE("XDWriteHelpers::placeText: not valid shape provided, run ignored");
        return;
    }

    // check if run doesn't containt visuals
    if(textRun.scriptProps.shapes == DWRITE_SCRIPT_SHAPES_NO_VISUAL)
    {
        // init buffers
        scriptPlace.advances.resize(scriptShape.glyphs.size());
        scriptPlace.offsets.resize(scriptShape.glyphs.size());

        DWRITE_GLYPH_OFFSET defaultOffset;
        defaultOffset.advanceOffset = 0;
        defaultOffset.ascenderOffset = 0;

        // we want to explicitly ignore these items
        std::fill(scriptPlace.advances.begin(), scriptPlace.advances.end(), 0.0f);
        std::fill(scriptPlace.offsets.begin(), scriptPlace.offsets.end(), defaultOffset);

        // reset width as well
        scriptPlace.width = 0.0f;

        return;
    }

    // text for run
    const wchar_t* text = richText->data() + textRun.range.pos;
    
    XWASSERT(text);
    if(text == 0) return;

    // get global text analysis interface
    IDWriteTextAnalyzer* textAnalyzer = getDirectWriteTextAnalyzer();
    if(textAnalyzer == 0) return;

    // init buffers
    scriptPlace.advances.resize(scriptShape.glyphs.size());
    scriptPlace.offsets.resize(scriptShape.glyphs.size());

    // position glyphs
    HRESULT hr = textAnalyzer->GetGlyphPlacements(text, scriptShape.clusterMap.data(), 
        scriptShape.textProps.data(), textRun.range.length, scriptShape.glyphs.data(),
        scriptShape.scriptAttrs.data(), (UINT32)scriptShape.glyphs.size(), fontCache.fontFace, 
        fontCache.fontEmSize, FALSE, textRun.isRTL, &textRun.scriptProps, localeName,
        0, 0, 0, scriptPlace.advances.data(), scriptPlace.offsets.data());

    // check result
    if(SUCCEEDED(hr))
    {
        // compute width
        scriptPlace.width = 0;
        for(unsigned int advanceIdx = 0; advanceIdx < scriptPlace.advances.size(); ++advanceIdx)
        {
            scriptPlace.width += scriptPlace.advances.at(advanceIdx);
        }

    } else
    {
        // report error
        XWTRACE_HRES("XDWriteHelpers::placeText: failed to get glyphs placements", hr);

        // reset output
        scriptPlace.advances.clear();
        scriptPlace.offsets.clear();
    }
}

/////////////////////////////////////////////////////////////////////
// get logical line attributes
/////////////////////////////////////////////////////////////////////
void XDWriteHelpers::getRunLogicalAttrs(const XRichText* richText, const XDwTextRun& textRun, 
                                        std::vector<XDwScriptLogAttr>& logAttrs, const wchar_t* localeName)
{
    // ignore if not loaded
    if(g_pIDWriteFactory == 0)
    {
        XWTRACE("XDWriteHelpers::analyseRichText failed as DirectWrite is not loaded");
        return;
    }

    // set active locale if needed
    if(localeName == 0)
    {
        localeName = XWUILocale::getSelectedLocaleW();
    }

    // create text analyzer
    XDWriteAnalyse* dwTextAnalyse = new XDWriteAnalyse(g_pIDWriteFactory, localeName, 
        textRun.isRTL ? DWRITE_READING_DIRECTION_RIGHT_TO_LEFT : DWRITE_READING_DIRECTION_LEFT_TO_RIGHT);

    if(dwTextAnalyse == 0) return;

    dwTextAnalyse->AddRef();

    // get global text analysis interface
    IDWriteTextAnalyzer* textAnalyzer = getDirectWriteTextAnalyzer();
    if(textAnalyzer == 0) return;

    // analyse line breakpoints in text
    std::vector<DWRITE_LINE_BREAKPOINT> scriptBreaks;
    dwTextAnalyse->analyseLineBreaks(textAnalyzer, richText, textRun.range, scriptBreaks);

    // validate output
    XWASSERT(scriptBreaks.size() == textRun.range.length);
    if(scriptBreaks.size() != textRun.range.length) return;

    // release analyser
    dwTextAnalyse->Release();
    dwTextAnalyse = 0;

    // NOTE: for simplicity of text layout later we use Uniscribe compatible logical attributes

    // reserve space
    logAttrs.resize(scriptBreaks.size());

    // copy values
    for(unsigned int charPos = 0; charPos < scriptBreaks.size(); ++charPos)
    {
        logAttrs.at(charPos).fWhiteSpace = scriptBreaks.at(charPos).isWhitespace;
        logAttrs.at(charPos).fSoftBreak = (scriptBreaks.at(charPos).breakConditionBefore == DWRITE_BREAK_CONDITION_CAN_BREAK);
    }
}

/////////////////////////////////////////////////////////////////////
// get mapping from glyphs to characters for complex scripts
/////////////////////////////////////////////////////////////////////
void XDWriteHelpers::mapGlyphsToChars(const XDwTextRun& textRun, XDwScriptShape& scriptShape, std::vector<UINT16>& glyphToChar)
{
    // check if script is complex
    if(!textRun.isComplex && !textRun.isRTL && textRun.range.length == scriptShape.glyphs.size())
    {
        XWTRACE("XDWriteHelpers::mapGlyphsToChars: glyph mapping is not needed for non complex scipts");
    }

    // reset output
    glyphToChar.clear();

    // reserve space for expected size
    glyphToChar.reserve(scriptShape.glyphs.size());

    // validate input
    if(scriptShape.clusterMap.size() != textRun.range.length || 
       textRun.range.length == 0 || scriptShape.glyphs.size() == 0)
    {
        XWASSERT(false);

        // fill with default values just to make sure it won't crash later
        glyphToChar.resize(scriptShape.glyphs.size());
        std::fill(glyphToChar.begin(), glyphToChar.end(), 0);

        return;
    }

    // loop over cluster map
    WORD glyphIdx = scriptShape.clusterMap.at(0);
    for(unsigned int mapIdx = 1; mapIdx < scriptShape.clusterMap.size(); ++mapIdx)
    {
        // validate index
        if(glyphIdx >= scriptShape.glyphs.size() || 
           scriptShape.clusterMap.at(mapIdx) >= scriptShape.glyphs.size())
        {
            XWASSERT(false);
            XWTRACE("XDWriteHelpers::mapGlyphsToChars: cluster map is not valid");

            // stop 
            break;
        }

        int clusterSize = scriptShape.clusterMap.at(mapIdx) - glyphIdx;

        // append
        while(clusterSize > 0)
        {
            // append mapping
            glyphToChar.push_back(glyphIdx);

            --clusterSize;
        }

        // next cluster
        glyphIdx = scriptShape.clusterMap.at(mapIdx);
    }

    // validate assumptions
    XWASSERT((glyphToChar.size() <= scriptShape.glyphs.size()));

    // map the rest to last character
    while(glyphToChar.size() < scriptShape.glyphs.size())
    {
        glyphToChar.push_back(textRun.range.length - 1);
    }
}

/////////////////////////////////////////////////////////////////////
// release font data
/////////////////////////////////////////////////////////////////////
void XDWriteHelpers::releaseFontData(XDwFontData& fontData)
{
    // release font face
    if(fontData.fontFace)
    {
        fontData.fontFace->Release();
        fontData.fontFace = 0;
    }
}

// XDWriteHelpers
/////////////////////////////////////////////////////////////////////

