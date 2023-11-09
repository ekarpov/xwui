// Direct Write text analysis helper functions
//
/////////////////////////////////////////////////////////////////////

#ifndef _XDWANALYSE_H_
#define _XDWANALYSE_H_

/////////////////////////////////////////////////////////////////////
// direct write headers
#include <dwrite.h>

/////////////////////////////////////////////////////////////////////
// XDWriteAnalyse - analysis helper

class XDWriteAnalyse : public IDWriteTextAnalysisSource,
                       public IDWriteTextAnalysisSink
{
public: // construction/destruction
    XDWriteAnalyse(IDWriteFactory* dwFactory, const WCHAR* localeName, DWRITE_READING_DIRECTION readingDirection);
    ~XDWriteAnalyse();

public: // analysis types
    struct XDwCharAnalysis
    {
        UINT8                   bidiLevel;
        DWRITE_SCRIPT_ANALYSIS  scriptAnalysis;
    };

public: // interface
    void    analyseRichText(IDWriteTextAnalyzer* textAnalyzer, const XRichText* richText, 
                            const XTextRange& range, std::vector<XDwCharAnalysis>& scriptAnalysis);
    void    analyseLineBreaks(IDWriteTextAnalyzer* textAnalyzer, const XRichText* richText, 
                              const XTextRange& range, std::vector<DWRITE_LINE_BREAKPOINT>& scriptBreaks);

public: // IUnknown 
    STDMETHODIMP            QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

public: // IDWriteTextAnalysisSource 
    STDMETHODIMP GetTextAtPosition(UINT32 textPosition, WCHAR const** textString, UINT32* textLength);
    STDMETHODIMP GetTextBeforePosition(UINT32 textPosition, WCHAR const** textString, UINT32* textLength);
    STDMETHODIMP_(DWRITE_READING_DIRECTION) GetParagraphReadingDirection();
    STDMETHODIMP GetLocaleName(UINT32 textPosition, UINT32* textLength, WCHAR const** localeName);
    STDMETHODIMP GetNumberSubstitution(UINT32 textPosition, UINT32* textLength, IDWriteNumberSubstitution** numberSubstitution);

public: // IDWriteTextAnalysisSink 
    STDMETHODIMP SetScriptAnalysis(UINT32 textPosition, UINT32 textLength, DWRITE_SCRIPT_ANALYSIS const* scriptAnalysis);
    STDMETHODIMP SetLineBreakpoints(UINT32 textPosition, UINT32 textLength,const DWRITE_LINE_BREAKPOINT* lineBreakpoints);
    STDMETHODIMP SetBidiLevel(UINT32 textPosition, UINT32 textLength, UINT8 explicitLevel, UINT8 resolvedLevel);
    STDMETHODIMP SetNumberSubstitution(UINT32 textPosition, UINT32 textLength, IDWriteNumberSubstitution* numberSubstitution);

private: // helper methods
    bool    _validateAndInitInput(const XRichText* richText, const XTextRange& range);

private: // analysis data
    const WCHAR*                m_localeName;
    DWRITE_READING_DIRECTION    m_direction;
    const WCHAR*                m_text;
    UINT32                      m_textLength;

    std::vector<XDwCharAnalysis>        m_scriptAnalysis;
    std::vector<DWRITE_LINE_BREAKPOINT> m_scriptBreaks;

private: // COM data
    unsigned long   m_ulRef;
    IDWriteFactory* m_pIDWriteFactory;
};

// XDWriteAnalyse
/////////////////////////////////////////////////////////////////////

#endif // _XDWANALYSE_H_

