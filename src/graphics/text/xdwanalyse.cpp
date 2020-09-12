// Direct Write text analysis helper functions
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"

#include "xrichtext.h"
#include "xdwanalyse.h"

/////////////////////////////////////////////////////////////////////
// XDWriteAnalyse - analysis helper
XDWriteAnalyse::XDWriteAnalyse(IDWriteFactory* dwFactory, const WCHAR* localeName, DWRITE_READING_DIRECTION readingDirection) :
    m_localeName(localeName),
    m_direction(readingDirection),
    m_text(0),
    m_textLength(0),
    m_ulRef(0),
    m_pIDWriteFactory(dwFactory)
{
    XWASSERT(m_localeName);
    XWASSERT(m_pIDWriteFactory);
}

XDWriteAnalyse::~XDWriteAnalyse()
{
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XDWriteAnalyse::analyseRichText(IDWriteTextAnalyzer* textAnalyzer, const XRichText* richText, 
                                     const XTextRange& range, std::vector<XDwCharAnalysis>& scriptAnalysis)
{
    XWASSERT(textAnalyzer);
    if(textAnalyzer == 0) return;

    // init input
    if(!_validateAndInitInput(richText, range))
    {
        XWTRACE("XDWriteAnalyse::analyseRichText: failed to initialize");
        return;
    }

    // reset output
    scriptAnalysis.clear();

    // init result
    m_scriptAnalysis.resize(m_textLength);

    XDwCharAnalysis defaultAnalysis;
    ::ZeroMemory(&defaultAnalysis, sizeof(XDwCharAnalysis));

    // fill default values
    std::fill(m_scriptAnalysis.begin(), m_scriptAnalysis.end(), defaultAnalysis);

    // analyse script properties
    HRESULT hr = textAnalyzer->AnalyzeScript(this, 0, m_textLength, this);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XDWriteAnalyse::analyseRichText: failed to analyse script", hr);
    }

    // analyse bidi levels
    hr = textAnalyzer->AnalyzeBidi(this, 0, m_textLength, this);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XDWriteAnalyse::analyseRichText: failed to analyse bidi levels", hr);
    }

    // swap results
    scriptAnalysis.swap(m_scriptAnalysis);

    // reset internal pointers
    m_text = 0;
    m_textLength = 0;

    // reset analysis data
    m_scriptAnalysis.clear();
}

void XDWriteAnalyse::analyseLineBreaks(IDWriteTextAnalyzer* textAnalyzer, const XRichText* richText, 
                                       const XTextRange& range, std::vector<DWRITE_LINE_BREAKPOINT>& scriptBreaks)
{
    XWASSERT(textAnalyzer);
    if(textAnalyzer == 0) return;

    // init input
    if(!_validateAndInitInput(richText, range))
    {
        XWTRACE("XDWriteAnalyse::analyseRichText: failed to initialize");
        return;
    }

    // reset output
    scriptBreaks.clear();

    // init result
    m_scriptBreaks.resize(m_textLength);

    DWRITE_LINE_BREAKPOINT defaultBreaks;
    ::ZeroMemory(&defaultBreaks, sizeof(DWRITE_LINE_BREAKPOINT));

    // fill default values
    std::fill(m_scriptBreaks.begin(), m_scriptBreaks.end(), defaultBreaks);

    // analyse line breaks
    HRESULT hr = textAnalyzer->AnalyzeLineBreakpoints(this, 0, m_textLength, this);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XDWriteAnalyse::analyseLineBreaks: failed to analyse line breakpoints", hr);
    }

    // swap results
    scriptBreaks.swap(m_scriptBreaks);

    // reset internal pointers
    m_text = 0;
    m_textLength = 0;

    // reset analysis data
    m_scriptBreaks.clear();
}

/////////////////////////////////////////////////////////////////////
// IUnknown 
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XDWriteAnalyse::QueryInterface(REFIID riid, void** ppvObject)
{
    // reset pointer first
    *ppvObject = 0;

    ///// check required interface

    // IUnknown
    if(riid == IID_IUnknown)
        *ppvObject = (IUnknown*)(IDispatch*)this;

    // IDWriteTextAnalysisSource
    else if(riid == __uuidof(IDWriteTextAnalysisSource))
        *ppvObject = (IDWriteTextAnalysisSource*)this;

    // IDWriteTextAnalysisSink
    else if(riid == __uuidof(IDWriteTextAnalysisSink))
        *ppvObject = (IDWriteTextAnalysisSink*)this;

    // check if interface is not supported
    if (!*ppvObject)
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) XDWriteAnalyse::AddRef()
{
    return ::InterlockedIncrement(&m_ulRef);
}

STDMETHODIMP_(ULONG) XDWriteAnalyse::Release()
{
    if(::InterlockedDecrement(&m_ulRef) == 0)
    {
        delete this;
        return 0;
    }

    return m_ulRef;
}

/////////////////////////////////////////////////////////////////////
// IDWriteTextAnalysisSource 
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XDWriteAnalyse::GetTextAtPosition(UINT32 textPosition, WCHAR const** textString, UINT32* textLength)
{
    XWASSERT(m_text);
    XWASSERT(textString);
    XWASSERT(textLength);

    // copy values
    if(textPosition < m_textLength)
    {
        *textString = m_text + textPosition;
        *textLength = m_textLength - textPosition;
    } else
    {
        *textString = 0;
        *textLength = 0;
    }

    return S_OK;
}

STDMETHODIMP XDWriteAnalyse::GetTextBeforePosition(UINT32 textPosition, WCHAR const** textString, UINT32* textLength)
{
    XWASSERT(m_text);
    XWASSERT(textString);
    XWASSERT(textLength);

    // copy values
    if(textPosition > 0 && textPosition < m_textLength)
    {
        *textString = m_text + textPosition - 1;
        *textLength = textPosition - 1;
    } else
    {
        *textString = 0;
        *textLength = 0;
    }

    return S_OK;
}

STDMETHODIMP_(DWRITE_READING_DIRECTION) XDWriteAnalyse::GetParagraphReadingDirection()
{
    return m_direction;
}

STDMETHODIMP XDWriteAnalyse::GetLocaleName(UINT32 textPosition, UINT32* textLength, WCHAR const** localeName)
{
    // set locale
    if(localeName)
    {
        *localeName = m_localeName;
    }

    return S_OK;
}

STDMETHODIMP XDWriteAnalyse::GetNumberSubstitution(UINT32 textPosition, UINT32* textLength, IDWriteNumberSubstitution** numberSubstitution)
{
    // should not be called
    XWASSERT(false);
    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////
// IDWriteTextAnalysisSink 
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XDWriteAnalyse::SetScriptAnalysis(UINT32 textPosition, UINT32 textLength, DWRITE_SCRIPT_ANALYSIS const* scriptAnalysis)
{
    XWASSERT(scriptAnalysis);
    if(scriptAnalysis == 0) return E_INVALIDARG;

    // flatten results
    for(UINT32 fillPos = 0; fillPos < textLength && textPosition + fillPos < m_scriptAnalysis.size(); ++fillPos)
    {
        // copy
        m_scriptAnalysis.at(textPosition + fillPos).scriptAnalysis = *scriptAnalysis;
    }

    return S_OK;
}

STDMETHODIMP XDWriteAnalyse::SetLineBreakpoints(UINT32 textPosition, UINT32 textLength,const DWRITE_LINE_BREAKPOINT* lineBreakpoints)
{
    XWASSERT(lineBreakpoints);
    if(lineBreakpoints == 0) return E_INVALIDARG;

    // copy
    if(textLength > 0)
    {
        ::memcpy(m_scriptBreaks.data() + textPosition, lineBreakpoints, textLength * sizeof(DWRITE_LINE_BREAKPOINT));
    }

    return S_OK;
}

STDMETHODIMP XDWriteAnalyse::SetBidiLevel(UINT32 textPosition, UINT32 textLength, UINT8 explicitLevel, UINT8 resolvedLevel)
{
    // flatten results
    for(UINT32 fillPos = 0; fillPos < textLength && textPosition + fillPos < m_scriptAnalysis.size(); ++fillPos)
    {
        // copy
        m_scriptAnalysis.at(textPosition + fillPos).bidiLevel = resolvedLevel;
    }

    return S_OK;
}

STDMETHODIMP XDWriteAnalyse::SetNumberSubstitution(UINT32 textPosition, UINT32 textLength, IDWriteNumberSubstitution* numberSubstitution)
{
    // should not be called
    XWASSERT(false);
    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////
// helper methods
/////////////////////////////////////////////////////////////////////
bool XDWriteAnalyse::_validateAndInitInput(const XRichText* richText, const XTextRange& range)
{
    // check internal state
    XWASSERT(m_localeName);
    XWASSERT(m_pIDWriteFactory);
    if(m_localeName == 0 || m_pIDWriteFactory == 0)
    {
        XWTRACE("XDWriteAnalyse::_validateAndInitInput: factory or locale not set");
        return false;
    }

    XWASSERT(richText);

    // check input
    if(richText == 0 || range.length == 0) 
    {
        XWTRACE("XDWriteAnalyse::_validateAndInitInput: input is not valid");
        return false;
    }

    // validate text range
    if(range.pos >= richText->textLength() || range.pos + range.length > richText->textLength())
    {
        XWTRACE("XDWriteAnalyse::_validateAndInitInput: text range is not valid");
        return false;
    }

    // set text
    m_text = richText->data() + range.pos;
    m_textLength = range.length;

    return true;
}

// XDWriteAnalyse
/////////////////////////////////////////////////////////////////////
