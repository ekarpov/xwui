// RichTextEdit windowless RichText editor
//
/////////////////////////////////////////////////////////////////////

#include "../../xwui_config.h"

#include "../xgdihelpres.h"
#include "../ximagefile.h"
#include "../ximage.h"
#include "../../xactive/xoledataobject.h"

#include "xtextinlineobject.h"
#include "xtextinlineimage.h"
#include "xrichtext.h"
#include "xrichtextparser.h"
#include "xtextservices.h"
#include "xrichtextedit.h"

/////////////////////////////////////////////////////////////////////
// NOTE: ITextServices interface https://msdn.microsoft.com/en-us/library/windows/desktop/bb787617(v=vs.85).aspx

/////////////////////////////////////////////////////////////////////
// constants

// supported property bits
#define XRICHEDIT_PROPERTY_BITS     (TXTBIT_MULTILINE | TXTBIT_READONLY | TXTBIT_WORDWRAP | \
                                     TXTBIT_RICHTEXT | TXTBIT_SAVESELECTION | TXTBIT_AUTOWORDSEL | \
                                     TXTBIT_USEPASSWORD | TXTBIT_ALLOWBEEP | TXTBIT_DISABLEDRAG)

/////////////////////////////////////////////////////////////////////
// XRichTextEdit - windowless RichText editor

XRichTextEdit::XRichTextEdit() :
    m_hwndContainer(0),
    m_ulRef(0),
    m_textServices(0),
    m_richEditOle(0),
    m_richEditObserver(0),
    m_parseInsertPos(0)
{
    // default properties
    m_propertyBits = TXTBIT_MULTILINE | TXTBIT_RICHTEXT | TXTBIT_SAVESELECTION | TXTBIT_AUTOWORDSEL;
    m_maxLength = INFINITE;
    m_fillBackground = true;

    // get system default font properties
    LOGFONT fnt;
    if(XWUtils::sGetDefaultFontProps(fnt))
    {
        // absolute value
        if(fnt.lfHeight < 0)
            fnt.lfHeight = -fnt.lfHeight;

        // copy properties
        m_textStyle.nFontSize = XGdiHelpers::dpiDipsToPixelsY(fnt.lfHeight);
        m_textStyle.bBold = (fnt.lfWeight == FW_BOLD);
        m_textStyle.bItalic = (fnt.lfItalic == TRUE);
        m_textStyle.bUnderline = (fnt.lfUnderline == TRUE);
        m_textStyle.bStrike = (fnt.lfStrikeOut == TRUE);
        m_textStyle.strFontName = fnt.lfFaceName;
    }

    // default colors
    m_crBackground = RGB(255, 255, 255);
    m_crSelectionText = RGB(0, 0, 0);
    m_crSelectionBackground = RGB(173, 214, 255);

    // reset data
    ::ZeroMemory(&m_charFormat, sizeof(CHARFORMAT2));
    ::ZeroMemory(&m_paraFormat, sizeof(PARAFORMAT2));

    // default text format
    m_charFormat.cbSize = sizeof(CHARFORMAT2);
    m_charFormat.dwMask = CFM_OFFSET | CFM_CHARSET | CFM_COLOR | CFM_BACKCOLOR;
    m_charFormat.yOffset = 0;
    m_charFormat.bCharSet = DEFAULT_CHARSET;
    m_charFormat.bPitchAndFamily = DEFAULT_PITCH;
    m_charFormat.crTextColor = RGB(0, 0, 0);
    m_charFormat.crBackColor = RGB(255, 255, 255);

    // copy default style to text format
    _textStyleToCharFormat(m_textStyle, &m_charFormat);

    // default paragraph format
    m_paraFormat.cbSize = sizeof(PARAFORMAT2);
    m_paraFormat.dwMask = PFM_TABSTOPS | PFM_ALIGNMENT | PFM_SPACEBEFORE | PFM_SPACEAFTER | 
                          PFM_RIGHTINDENT | PFM_STARTINDENT | PFM_OFFSETINDENT;
    m_paraFormat.cTabCount = 1;
    m_paraFormat.rgxTabs[0] = lDefaultTab;    
    m_paraFormat.wAlignment = PFA_LEFT;
    m_paraFormat.bLineSpacingRule = 0; 

    // NOTE: dxOffset holds indentation of the second and subsequent lines, relative to the 
    //       indentation of the first line, in twips. We always set it to zero and change only dxStartIndent
    //       so that all indentations will be the same
    m_paraFormat.dxStartIndent = 0;
    m_paraFormat.dxOffset = 0;
}

XRichTextEdit::~XRichTextEdit()
{
    // close
    close();
}

/////////////////////////////////////////////////////////////////////
// initialization
/////////////////////////////////////////////////////////////////////
bool XRichTextEdit::init(HWND hwnd, const RECT& rcClient)
{
    XWASSERT(hwnd);
    if(hwnd == 0) return false;

    // close old resources if any
    close();

    // create text service
    if(!XTextServices::createTextServices(hwnd, rcClient, this, &m_textServices))
    {
        XWTRACE("XRichTextEdit: failed to create text services interface");
        return false;
    }

    // init container
    m_hwndContainer = hwnd;
    m_rcClient = rcClient;

    // reset content rect
    ::ZeroMemory(&m_rcContent, sizeof(RECT));

    // notify that we are in-place active
    HRESULT hr = m_textServices->OnTxInPlaceActivate(&rcClient);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XRichTextEdit: failed to activate text services", hr);
        return false;
    }

    // default properties
    hr = m_textServices->OnTxPropertyBitsChange(XRICHEDIT_PROPERTY_BITS, m_propertyBits);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XRichTextEdit failed to update default properties", hr);
    }

    // enable required notifications
    if(!sendWindowMessage(EM_SETEVENTMASK, 0, ENM_REQUESTRESIZE | ENM_SCROLL | ENM_LINK))
    {
        XWTRACE("XRichTextEdit: failed to set event mask, notifications will not be sent");
    }

    // get OLE interface
    if(!sendWindowMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&m_richEditOle) || m_richEditOle == 0)
    {
        XWTRACE("XRichTextEdit: failed to query IRichEditOle interface from RichEdit");
        return false;
    }

    // set OLE callback

    // success
    return true;
}

void XRichTextEdit::close()
{
    // release interfaces
    if(m_richEditOle) m_richEditOle->Release();
    m_richEditOle = 0;

    // release text services
    if(m_textServices) 
    {
        // deactivate control
        m_textServices->OnTxInPlaceDeactivate();

        // release interface
        m_textServices->Release();
    }

    // reset references
    m_textServices = 0;
    m_hwndContainer = 0;
}

bool XRichTextEdit::isReady() const
{
    return (m_textServices != 0) && (m_richEditOle != 0);
}

/////////////////////////////////////////////////////////////////////
// observer
/////////////////////////////////////////////////////////////////////
void XRichTextEdit::setObserver(IXRichEditObserver* observer)
{
    // set observer reference
    m_richEditObserver = observer;
}

/////////////////////////////////////////////////////////////////////
// text style
/////////////////////////////////////////////////////////////////////
void XRichTextEdit::setTextStyle(const XTextStyle& style)
{
    // copy style
    m_textStyle = style;

    // copy style to text format
    _textStyleToCharFormat(m_textStyle, &m_charFormat);

    // update editor if ready
    _notifyPropertyChanged(TXTBIT_CHARFORMATCHANGE);
}

/////////////////////////////////////////////////////////////////////
// paragraph properties
/////////////////////////////////////////////////////////////////////
void XRichTextEdit::setAlignment(TTextAlignment alignment)
{
    // update alignment
    if(alignment == eTextAlignCenter)
        m_paraFormat.wAlignment = PFA_CENTER;
    else if(alignment == eTextAlignRight)
        m_paraFormat.wAlignment = PFA_RIGHT;
    else if(alignment == eTextAlignJustify)
        m_paraFormat.wAlignment = PFA_JUSTIFY;
    else
        m_paraFormat.wAlignment = PFA_LEFT;

    // update editor if ready
    _notifyPropertyChanged(TXTBIT_PARAFORMATCHANGE);
}

void XRichTextEdit::setLineSpacing(TLineSpacing spacing)
{
    // update mask
    if(spacing != eLineSpacingNone)
    {
        // set spacing
        if(spacing == eLineSpacingOneHalf)
            m_paraFormat.bLineSpacingRule = 1;
        else if(spacing == eLineSpacingDouble)
            m_paraFormat.bLineSpacingRule = 2;
        else
            m_paraFormat.bLineSpacingRule = 0;

        // update mask
        m_paraFormat.dwMask |= PFM_LINESPACING;

    } else
    {
        // switch off line spacing
        m_paraFormat.dwMask &= ~PFM_LINESPACING;
    }

    // update editor if ready
    _notifyPropertyChanged(TXTBIT_PARAFORMATCHANGE);
}

void XRichTextEdit::setParaSpacing(DWORD left, DWORD right, DWORD top, DWORD bottom)
{
    // update properties
    m_paraFormat.dxStartIndent = XGdiHelpers::pixelsToTwipsX(left);
    m_paraFormat.dxRightIndent = XGdiHelpers::pixelsToTwipsX(right);
    m_paraFormat.dySpaceBefore = XGdiHelpers::pixelsToTwipsY(top);
    m_paraFormat.dySpaceAfter = XGdiHelpers::pixelsToTwipsY(bottom);

    // update editor if ready
    _notifyPropertyChanged(TXTBIT_PARAFORMATCHANGE);
}

void XRichTextEdit::setTextBackgroundColor(COLORREF color)
{
    // copy color
    m_charFormat.crBackColor = color;

    // update editor if ready
    _notifyPropertyChanged(TXTBIT_CHARFORMATCHANGE);
}

TTextAlignment XRichTextEdit::alignment() const
{
    // get format from paragraph
    if(m_paraFormat.wAlignment == PFA_CENTER)
        return eTextAlignCenter;
    else if(m_paraFormat.wAlignment == PFA_RIGHT)
        return eTextAlignRight;
    else if(m_paraFormat.wAlignment == PFA_JUSTIFY)
        return eTextAlignRight;
    else
        return eTextAlignLeft;
}

TLineSpacing XRichTextEdit::lineSpacing() const
{
    // check if mask is set
    if(m_paraFormat.dwMask & PFM_LINESPACING)
    {
        // get from paragraph
        if(m_paraFormat.bLineSpacingRule == 1)
            return eLineSpacingOneHalf;
        else if(m_paraFormat.bLineSpacingRule == 2)
            return eLineSpacingDouble;
        else
            return eLineSpacingSingle;
    } else
    {
        return eLineSpacingNone;
    }
}

void XRichTextEdit::getParaSpacing(DWORD& left, DWORD& right, DWORD& top, DWORD& bottom)
{
    // get margins
    left = XGdiHelpers::twipsToPixelsX(m_paraFormat.dxStartIndent); 
    right = XGdiHelpers::twipsToPixelsX(m_paraFormat.dxRightIndent); 
    top = XGdiHelpers::twipsToPixelsY(m_paraFormat.dySpaceBefore);
    bottom = XGdiHelpers::twipsToPixelsY(m_paraFormat.dySpaceAfter);
}

void XRichTextEdit::getTextBackgroundColor(COLORREF& colorOut)
{
    // copy color
    colorOut = m_charFormat.crBackColor;
}

/////////////////////////////////////////////////////////////////////
// editor properties
/////////////////////////////////////////////////////////////////////
void XRichTextEdit::setTextLimit(DWORD textLimit)
{
    // copy value
    m_maxLength = textLimit;

    // update editor if ready
    sendWindowMessage(EM_LIMITTEXT, m_maxLength, 0);
}

/////////////////////////////////////////////////////////////////////
// editor colors
/////////////////////////////////////////////////////////////////////
void XRichTextEdit::setColor(EditorColor color, COLORREF value)
{
    // check type
    switch(color)
    {
    case eColorText:
        // copy color
        m_charFormat.crTextColor = value;

        // update editor if ready
        _notifyPropertyChanged(TXTBIT_CHARFORMATCHANGE);
        break;

    case eColorBackground:
        // copy color
        m_crBackground = value;

        // update editor if ready
        _notifyPropertyChanged(TXTBIT_BACKSTYLECHANGE);
        break;

    case eColorSelectionText:
        // copy color
        m_crSelectionText = value;
        break;

    case eColorSelectionBackground:
        // copy color
        m_crSelectionBackground = value;
        break;

    default:
        XWASSERT1(0, "XRichTextEdit: unsupported color type");
        break;
    }
}

void XRichTextEdit::getColor(EditorColor color, COLORREF& valueOut)
{
    // check type
    switch(color)
    {
    case eColorText:
        // copy color
        valueOut = m_charFormat.crTextColor;
        break;

    case eColorBackground:
        // copy color
        valueOut = m_crBackground;
        break;

    case eColorSelectionText:
        // copy color
        valueOut = m_crSelectionText;
        break;

    case eColorSelectionBackground:
        // copy color
        valueOut = m_crSelectionBackground;
        break;

    default:
        XWASSERT1(0, "XRichTextEdit: unsupported color type");
        break;
    }
}

void XRichTextEdit::enableBackgroundFill(bool enable)
{
    // copy flag
    m_fillBackground = enable;

    // update editor if ready
    _notifyPropertyChanged(TXTBIT_BACKSTYLECHANGE);
}

/////////////////////////////////////////////////////////////////////
// edit text
/////////////////////////////////////////////////////////////////////
bool XRichTextEdit::insertText(int pos, const wchar_t* text, UINT length)
{
    // check input
    XWASSERT(text);
    XWASSERT(isReady());
    if(text == 0 || !isReady()) return false;

    // compute length if needed
    if(length == 0)
    {
        length = (UINT)::wcslen(text);
        if(length == 0) return true;
    }

    bool retVal = false;

    // get range
    ITextRange* textRange = _getDocumentRange(pos, 0);
    if(textRange)
    {
        // convert to BSTR
        BSTR bstrText = ::SysAllocStringLen(text, length);

        // insert text
        if(bstrText)
        {
            HRESULT hr = textRange->SetText(bstrText);
            if(SUCCEEDED(hr))
            {
                retVal = true;

            } else
            {
                XWTRACE_HRES("XRichTextEdit: failed to insert text", hr);
            }

            // free string
            ::SysFreeString(bstrText);

        } else
        {
            XWTRACE("XRichTextEdit: failed to convert string to BSTR");
        }

        textRange->Release();
    }

    return retVal;
}

bool XRichTextEdit::insertRichText(int pos, const XRichText* richText)
{
    // check input
    XWASSERT(richText);
    XWASSERT(isReady());
    if(richText == 0 || !isReady()) return false;
    
    // process rich text
    for(unsigned int textPos = pos; textPos < richText->textLength(); )
    {
        XTextStyle style;
        bool hasInlineObject;

        // run start position
        unsigned int startPos = textPos;

        // get text run (will return run end position)
        textPos = richText->getTextRun(textPos, style, hasInlineObject);

        // add text run
        if(!hasInlineObject)
        {
            // insert text
            if(!insertText(startPos, richText->data() + startPos, textPos - startPos)) return false;

            // apply style
            if(!setTextStyle(startPos, textPos - startPos, style)) return false;

        } else
        {
            // get inline object
            XTextInlineObject* inlineObject = richText->inlineObjectAt(startPos);
            if(inlineObject)
            {
                // insert inline object
                if(!insertInlineObject(startPos, inlineObject))
                {
                    XWTRACE("XRichTextEdit: failed to insert inline object from RichText, object will be ignored");
                }
            }
        }
    }

    // set colors
    for(unsigned int textPos = pos; textPos < richText->textLength(); )
    {
        COLORREF color;

        // run start position
        unsigned int startPos = textPos;

        // get text run (will return run end position)
        textPos = richText->getColorRun(textPos);

        // text color
        if(richText->textColor(startPos, color))
            setTextColor(startPos, textPos - startPos, color);

        // background color
        if(richText->backgroundColor(startPos, color))
            setBackgroundColor(startPos, textPos - startPos, color);
    }

    return true;
}

bool XRichTextEdit::insertFormatText(int pos, const wchar_t* text)
{
    // check input
    XWASSERT(text);
    XWASSERT(isReady());
    if(text == 0 || !isReady()) return false;
    
    XTextStyle style = textStyle();

    // get style from position
    getTextStyle(pos, style);

    // save insertion position
    m_parseInsertPos = pos;

    // parse text
    m_textParser.parseBegin(style, this);
    m_textParser.parse(text, ::wcslen(text));
    m_textParser.parseEnd();

    return true;
}

bool XRichTextEdit::deleteText(int pos, int length)
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return false;

    // ignore if nothing to delete
    if(length == 0) return true;

    bool retVal = false;

    // get range
    ITextRange* textRange = _getDocumentRange(pos, length);
    if(textRange)
    {
        // NOTE: Text that replaces the current text in this range. 
        //       If null, the current text is deleted.

        // remove range text
        HRESULT hr = textRange->SetText(0);
        if(SUCCEEDED(hr))
        {
            retVal = true;

        } else
        {
            XWTRACE_HRES("XRichTextEdit: failed to delete text range", hr);
        }

        textRange->Release();
    }

    return retVal;
}

bool XRichTextEdit::clearText()
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return false;

    // delete whole text
    HRESULT hr = m_textServices->TxSetText(0);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XRichTextEdit: failed to clear editor text", hr);
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////
// read text
/////////////////////////////////////////////////////////////////////
bool XRichTextEdit::getText(int pos, int length, std::wstring& textOut)
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return false;

    // reset output
    textOut.clear();

    // ignore if range is empty
    if(length == 0) return true;

    bool retVal = false;

    // get range
    ITextRange* textRange = _getDocumentRange(pos, length);
    if(textRange)
    {
        BSTR bstrText;

        // get text
        HRESULT hr = textRange->GetText(&bstrText);
        if(SUCCEEDED(hr))
        {
            // convert string
            if(bstrText)
                textOut.assign(bstrText, ::SysStringLen(bstrText));

            // free string
            ::SysFreeString(bstrText);

            retVal = true;
        }

        textRange->Release();
    }

    return retVal;
}

bool XRichTextEdit::getRichText(int pos, int length, XRichText* richTextOut)
{
    // check input
    XWASSERT(richTextOut);
    XWASSERT(isReady());
    if(richTextOut == 0 || !isReady()) return false;

    // reset output
    richTextOut->reset();

    // ignore if range is empty
    if(length == 0) return true;

    bool retVal = false;

    // get range
    ITextRange* textRange = _getDocumentRange(pos, length);
    if(textRange)
    {
        // TODO:

        textRange->Release();
    }

    return retVal;
}

bool XRichTextEdit::getFormatText(int pos, int length, std::wstring& formatTextOut)
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return false;

    // reset output
    formatTextOut.clear();

    // ignore if range is empty
    if(length == 0) return true;

    bool retVal = false;

    // get range
    ITextRange* textRange = _getDocumentRange(pos, length);
    if(textRange)
    {
        // TODO: 

        textRange->Release();
    }

    return retVal;
}

int XRichTextEdit::getTextLength()
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return 0;

    GETTEXTLENGTHEX gtlex;
    gtlex.flags = GTL_NUMCHARS;
    gtlex.codepage = 1200; // Unicode characters

    // send window message
    LRESULT res = 0;
    if(sendWindowMessage(EM_GETTEXTLENGTHEX, (WPARAM)&gtlex, 0, &res))
    {
        // return result
        return (int)res;

    } else
    {
        XWTRACE("XRichTextEdit: failed to read text length from control");
        return 0;
    }
}

int XRichTextEdit::getLineCount()
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return 0;

    // send window message
    LRESULT res = 0;
    if(sendWindowMessage(EM_GETLINECOUNT, 0, 0, &res))
    {
        // return result
        return (int)res;

    } else
    {
        XWTRACE("XRichTextEdit: failed to read line count from control");
        return 0;
    }
}

/////////////////////////////////////////////////////////////////////
// text style 
/////////////////////////////////////////////////////////////////////
bool XRichTextEdit::setTextStyle(int pos, int length, const XTextStyle& style)
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return false;

    bool retVal = false;

    HRESULT hr = S_OK;

    // get range font
    ITextFont* textFont = _getRangeFont(pos, length);
    while(textFont)
    {
        // font name
        if(style.strFontName.length())
        {
            // convert to BSTR
            BSTR bstrText = ::SysAllocStringLen(style.strFontName.data(), (UINT)style.strFontName.length());

            // set name
            hr = textFont->SetName(bstrText);

            // release string
            ::SysFreeString(bstrText);

            // font name
            if(FAILED(hr)) break;
        }

        // font weight
        if(style.nFontSize)
        {
            hr = textFont->SetSize(XGdiHelpers::pixelsToPointsY(style.nFontSize));
            if(FAILED(hr)) break;
        }

        // font style
        hr = textFont->SetBold(style.bBold ? tomTrue : tomFalse);
        if(FAILED(hr)) break;
        hr = textFont->SetItalic(style.bItalic ? tomTrue : tomFalse);
        if(FAILED(hr)) break;
        hr = textFont->SetUnderline(style.bUnderline ? tomTrue : tomFalse);
        if(FAILED(hr)) break;
        hr = textFont->SetStrikeThrough(style.bStrike ? tomTrue : tomFalse);
        if(FAILED(hr)) break;

        retVal = true;
        break;
    }
    
    if(FAILED(hr))
    {
        XWTRACE_HRES("XRichTextEdit: failed to set text style", hr);
    }

    // release font
    if(textFont) 
        textFont->Release();

    return retVal;
}

bool XRichTextEdit::getTextStyle(int pos, XTextStyle& style)
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return false;

    bool retVal = false;

    HRESULT hr = S_OK;

    // get range font
    ITextFont* textFont = _getRangeFont(pos, 1);
    while(textFont)
    {
        BSTR bstrText;

        // font name
        HRESULT hr = textFont->GetName(&bstrText);
        if(FAILED(hr)) break;

        // convert string
        if(bstrText)
            style.strFontName.assign(bstrText, ::SysStringLen(bstrText));

        // release string
        ::SysFreeString(bstrText);

        float fontSize = 0;
        long  value = 0;

        // font size
        hr = textFont->GetSize(&fontSize);
        if(FAILED(hr)) break;
        style.nFontSize = XGdiHelpers::pointsToPixelsY(fontSize);

        // font style
        hr = textFont->GetBold(&value);
        if(FAILED(hr)) break;
        style.bBold = (value == tomTrue);

        hr = textFont->GetItalic(&value);
        if(FAILED(hr)) break;
        style.bItalic = (value == tomTrue);

        hr = textFont->GetUnderline(&value);
        if(FAILED(hr)) break;
        style.bUnderline = (value == tomTrue);

        hr = textFont->GetStrikeThrough(&value);
        if(FAILED(hr)) break;
        style.bStrike = (value == tomTrue);

        retVal = true;
        break;
    }
    
    if(FAILED(hr))
    {
        XWTRACE_HRES("XRichTextEdit: failed to read text style", hr);
    }

    // release font
    if(textFont) 
        textFont->Release();

    return retVal;
}

/////////////////////////////////////////////////////////////////////
// text color
/////////////////////////////////////////////////////////////////////
bool XRichTextEdit::setTextColor(int pos, int length, COLORREF textColor)
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return false;

    // ignore if range is empty
    if(length == 0) return true;

    bool retVal = false;

    // get range font
    ITextFont* textFont = _getRangeFont(pos, length);
    if(textFont)
    {
        HRESULT hr = textFont->SetForeColor(textColor);
        if(SUCCEEDED(hr))
        {
            retVal = true;

        } else
        {
            XWTRACE_HRES("XRichTextEdit: failed to set text color", hr);
        }

        // release font
        textFont->Release();
    }
    
    return retVal;
}

bool XRichTextEdit::resetTextColor(int pos, int length)
{
    return setTextColor(pos, length, tomAutoColor);
}

bool XRichTextEdit::getTextColor(int pos, COLORREF& colorOut)
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return false;

    bool retVal = false;

    // get range font
    ITextFont* textFont = _getRangeFont(pos, 1);
    if(textFont)
    {
        long value;

        HRESULT hr = textFont->GetForeColor(&value);
        if(SUCCEEDED(hr))
        {
            // NOTE: return value is COLORREF if the high-order byte is zero, and the 
            //       three low-order bytes specify an RGB color.
            if((value & 0xFF000000) == 0)
            {
                colorOut = (COLORREF)value;
                retVal = true;

            } else if((value & 0xFF000000) == 0x01000000) // PALETTEINDEX
            {
                XWTRACE("XRichTextEdit: palette index in text color not supported");

            } else if(value == -9999997) // tomAutocolor
            {
                // get system color
                colorOut = TxGetSysColor(COLOR_WINDOWTEXT);
                retVal = true;

            } else
            {
                XWTRACE("XRichTextEdit: unknown text color type ignored");
            }

        } else
        {
            XWTRACE_HRES("XRichTextEdit: failed to read text color", hr);
        }

        // release font
        textFont->Release();
    }
    
    return retVal;
}

bool XRichTextEdit::setBackgroundColor(int pos, int length, COLORREF backgroundColor)
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return false;

    // ignore if range is empty
    if(length == 0) return true;

    bool retVal = false;

    // get range font
    ITextFont* textFont = _getRangeFont(pos, length);
    if(textFont)
    {
        HRESULT hr = textFont->SetBackColor(backgroundColor);
        if(SUCCEEDED(hr))
        {
            retVal = true;

        } else
        {
            XWTRACE_HRES("XRichTextEdit: failed to set background color", hr);
        }

        // release font
        textFont->Release();
    }
    
    return retVal;
}

bool XRichTextEdit::resetBackgroundColor(int pos, int length)
{
    return setBackgroundColor(pos, length, tomAutoColor);
}

bool XRichTextEdit::getBackgroundColor(int pos, COLORREF& colorOut)
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return false;

    bool retVal = false;

    // get range font
    ITextFont* textFont = _getRangeFont(pos, 1);
    if(textFont)
    {
        long value;

        HRESULT hr = textFont->GetBackColor(&value);
        if(SUCCEEDED(hr))
        {
            // NOTE: return value is COLORREF if the high-order byte is zero, and the 
            //       three low-order bytes specify an RGB color.
            if((value & 0xFF000000) == 0)
            {
                colorOut = (COLORREF)value;
                retVal = true;

            } else if((value & 0xFF000000) == 0x01000000) // PALETTEINDEX
            {
                XWTRACE("XRichTextEdit: palette index in background color not supported");

            } else if(value == -9999997) // tomAutocolor
            {
                // get system color
                colorOut = TxGetSysColor(COLOR_WINDOW);
                retVal = true;

            } else
            {
                XWTRACE("XRichTextEdit: unknown background color type ignored");
            }            

        } else
        {
            XWTRACE_HRES("XRichTextEdit: failed to read background color", hr);
        }

        // release font
        textFont->Release();
    }
    
    return retVal;
}

/////////////////////////////////////////////////////////////////////
// inline objects
/////////////////////////////////////////////////////////////////////
bool XRichTextEdit::insertInlineObject(int textPos, XTextInlineObject* inlineObject, const XTextStyle* style)
{
    // check input
    XWASSERT(inlineObject);
    XWASSERT(isReady());
    if(inlineObject == 0 || !isReady()) return false;

    // init required interfaces
    IStorage* storage = _createStorage();
    IOleClientSite* oleClientSite = _getOleClientSite();

    // create OLE object
    IOleObject* oleObject = 0;
    if(storage && oleClientSite)
    {
        HRESULT hr = inlineObject->createOleObject(oleClientSite, storage, &oleObject);
        if(FAILED(hr))
        {
            XWTRACE_HRES("XRichTextEdit: failed to create OLE object from Inline object", hr);
        }
    }

    // check
    if(oleObject == 0)
    {
        if(storage) storage->Release();
        if(oleClientSite) oleClientSite->Release();
        return false;
    }

    HRESULT hr = ::OleSetContainedObject(oleObject, TRUE);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XRichTextEdit: failed to set contained OLE object, trying to ignore", hr);
    }

    REOBJECT reobj;
    
    // init
    ::ZeroMemory(&reobj, sizeof(REOBJECT));
    reobj.cbStruct = sizeof(REOBJECT);
    reobj.cp = textPos;
	reobj.dvaspect = DVASPECT_CONTENT;
	reobj.poleobj = oleObject;
	reobj.polesite = oleClientSite;
	reobj.pstg = storage;

    // get class id from oject
    hr = oleObject->GetUserClassID(&reobj.clsid);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XRichTextEdit: failed to query CLSID from OLE object, trying to ignore", hr);
    }

    bool retVal = true;

    // insert object
    hr = m_richEditOle->InsertObject(&reobj);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XRichTextEdit: failed to insert inline object", hr);
        retVal = false;
    }

    // release interface
    oleObject->Release();
    storage->Release();
    oleClientSite->Release();

    // stop if failed
    if(!retVal) return false;

    // set underlying style if needed
    if(style)
        setTextStyle(textPos, 1, *style);

    return true;
}

int XRichTextEdit::getInlineObjectCount()
{
    // IRichEditOle::GetObjectCount
    //https://msdn.microsoft.com/en-us/library/windows/desktop/bb774347(v=vs.85).aspx

    // TODO:
    return 0;
}

bool XRichTextEdit::getInlineObject(int index, IDataObject** objectOut, int* textPosOut)
{
    // IRichEditOle::GetObject
    // https://msdn.microsoft.com/en-us/library/windows/desktop/bb774345(v=vs.85).aspx
    
    // REOBJECT
    //https://msdn.microsoft.com/en-us/library/windows/desktop/bb787946(v=vs.85).aspx

    // TODO:
    return false;
}

/////////////////////////////////////////////////////////////////////
// urls
/////////////////////////////////////////////////////////////////////
bool XRichTextEdit::setTextUrl(int pos, int length, const wchar_t* url)
{
    // TODO:
    return false;
}

/////////////////////////////////////////////////////////////////////
// cursor
/////////////////////////////////////////////////////////////////////
int XRichTextEdit::cursorPos()
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return 0;

    // get current selection
    int pos, length;
    if(getTextSelection(pos, length))
    {
        // NOTE: this will not work if there is backward selection
        return pos + length;
    }

    return 0;
}

void XRichTextEdit::setCursorEnd()
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return;

    CHARRANGE crange;
    crange.cpMin = -1;
    crange.cpMax = -1;

    // move cursor at the end
    if(!sendWindowMessage(EM_EXSETSEL, 0, (LPARAM)&crange))
    {
        XWTRACE("XRichTextEdit: failed to set cursor to document end");
    }
}

void XRichTextEdit::setCursorPos(int pos)
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return;

    CHARRANGE crange;
    crange.cpMin = pos;
    crange.cpMax = pos;

    // select text
    if(!sendWindowMessage(EM_EXSETSEL, 0, (LPARAM)&crange))
    {
        XWTRACE("XRichTextEdit: failed to set cursor position");
    }
}

/////////////////////////////////////////////////////////////////////
// selection
/////////////////////////////////////////////////////////////////////
bool XRichTextEdit::hasSelection()
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return false;

    // check if there is selection
    int pos, length;
    return getTextSelection(pos, length) && (length > 0);
}

bool XRichTextEdit::selectText(int pos, int length)
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return false;

    CHARRANGE crange;
    crange.cpMin = pos;
    crange.cpMax = pos + length;

    // select text
    if(!sendWindowMessage(EM_EXSETSEL, 0, (LPARAM)&crange))
    {
        XWTRACE("XRichTextEdit: failed to select text");
        return false;
    }

    return true;
}

bool XRichTextEdit::getTextSelection(int& pos, int& length)
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return false;

    // get selection
    CHARRANGE crange;
    if(!sendWindowMessage(EM_EXGETSEL, 0, (LPARAM)&crange))
    {
        XWTRACE("XRichTextEdit: failed to read text selection range");
        return false;
    }

    // NOTE: If the cpMin and cpMax members are equal, the range is empty. 
    //       The range includes everything if cpMin is 0 and cpMax is –1.
    //       cpMin - Character position index immediately preceding the first character in the range.
    //       cpMax - Character position immediately following the last character in the range.

    // convert to text range
    pos = crange.cpMin;
    if(crange.cpMax >= crange.cpMin)
        length = crange.cpMax - crange.cpMin;
    else 
        length = getTextLength();

    return true;
}

bool XRichTextEdit::resetSelection()
{
    // check input
    XWASSERT(isReady());
    if(!isReady()) return false;

    // get current selection
    int pos, length;
    if(!getTextSelection(pos, length)) return false;

    // reset selection
    pos += length;
    length = 0;

    // update selection
    return selectText(pos, length);
}

/////////////////////////////////////////////////////////////////////
// content size
/////////////////////////////////////////////////////////////////////
int XRichTextEdit::contentWidth()
{
    // NOTE: when word wrap is active RichEdit fails to report proper content width
    //       in its EN_REQUESTRESIZE notification so we assume content widht is the 
    //       same as client width

    if(!hasWordWrap())
        return (m_rcContent.right - m_rcContent.left);
    else
        return (m_rcClient.right - m_rcClient.left);        
}

int XRichTextEdit::contentHeight()
{
    return (m_rcContent.bottom - m_rcContent.top);
}

/////////////////////////////////////////////////////////////////////
// scrolling info
/////////////////////////////////////////////////////////////////////
long XRichTextEdit::scrollOffsetX()
{
    // get scroll information from text services
    if(m_textServices)
    {
        LONG lMin, lMax, lPos, lPage;
        BOOL fEnabled;

        // get horizontal scroll position
        HRESULT hr = m_textServices->TxGetHScroll(&lMin, &lMax, &lPos, &lPage, &fEnabled);
        if(FAILED(hr))
        {
            XWTRACE_HRES("XRichTextEdit: failed to get horizontal scroll", hr);
            return 0;
        }

        // return scroll position
        return lPos;
    }

    return 0;
}

long XRichTextEdit::scrollOffsetY()
{
    // get scroll information from text services
    if(m_textServices)
    {
        LONG lMin, lMax, lPos, lPage;
        BOOL fEnabled;

        // get vertical scroll position
        HRESULT hr = m_textServices->TxGetVScroll(&lMin, &lMax, &lPos, &lPage, &fEnabled);
        if(FAILED(hr))
        {
            XWTRACE_HRES("XRichTextEdit: failed to get vertical scroll", hr);
            return 0;
        }

        // return scroll position
        return lPos;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////
// scrolling content
/////////////////////////////////////////////////////////////////////
void XRichTextEdit::setScrollOffsetX(int pos)
{
    if(!sendWindowMessage(WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, pos), 0))
    {
        XWTRACE("XRichTextEdit: failed to set horizontal scroll offset");
    }
}

void XRichTextEdit::setScrollOffsetY(int pos)
{
    if(!sendWindowMessage(WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, pos), 0))
    {
        XWTRACE("XRichTextEdit: failed to set vertical scroll offset");
    }
}

/////////////////////////////////////////////////////////////////////
// message processing
/////////////////////////////////////////////////////////////////////
bool XRichTextEdit::sendWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* resultOut)
{
    LRESULT res = 0;

    if(m_textServices)
    {
        // send message to text services
        HRESULT hr = m_textServices->TxSendMessage(uMsg, wParam, lParam, &res);
        if(SUCCEEDED(hr))
        {
            // copy result if needed
            if(resultOut) *resultOut = res;

            // message sent
            return true;

        } else
        {
            XWTRACE_HRES("XRichTextEdit::sendWindowMessage failed to send message", hr);
        }
    }

    return false;
}

LRESULT XRichTextEdit::processWindowMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& messageProcessed)
{
    LRESULT res = 0;

    // pass to text services if set
    if(m_textServices)
    {
        HRESULT hr;

        // pass window message
        hr = m_textServices->TxSendMessage(uMsg, wParam, lParam, &res);

        // check result
        if(SUCCEEDED(hr))
        {
            // check if message has been processed
            messageProcessed = (hr != S_FALSE);

        } else
        {
            // report error
            XWTRACE_HRES("XRichTextEdit::processWindowMessage failed to send message", hr);
        } 

    }

    return res;
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
void XRichTextEdit::update(const RECT& rcClient)
{
    // copy new size
    m_rcClient = rcClient; 

    // inform text services
    if(m_textServices)
    {
        // pass to text services
        HRESULT hr = m_textServices->OnTxPropertyBitsChange(TXTBIT_CLIENTRECTCHANGE, 
				TXTBIT_CLIENTRECTCHANGE);

        if(FAILED(hr))
        {
            XWTRACE_HRES("XRichTextEdit failed to update size", hr);
        }
    }
}

/////////////////////////////////////////////////////////////////////
// properties
/////////////////////////////////////////////////////////////////////
void XRichTextEdit::setEnabled(bool bEnabled)
{
    // TODO:
}

bool XRichTextEdit::isEnabled() const
{
    // TODO:
    return false;
}

/////////////////////////////////////////////////////////////////////
// focus
/////////////////////////////////////////////////////////////////////
void XRichTextEdit::setFocus(bool bFocus)
{
    // change focus
    if(!sendWindowMessage(bFocus ? WM_SETFOCUS : WM_KILLFOCUS, 0, 0))
    {
        XWTRACE("XRichTextEdit: failed to change focus");
    }
}

/////////////////////////////////////////////////////////////////////
// cursor
/////////////////////////////////////////////////////////////////////
bool XRichTextEdit::onSetCursor(int posX, int posY)
{
    // set cursor
    if(m_textServices)
    {
        // pass to text services
        HRESULT hr = m_textServices->OnTxSetCursor(DVASPECT_CONTENT, -1, 0, 0, 0, 0, 0, posX, posY);
        if(FAILED(hr))
        {
            XWTRACE_HRES("XRichTextEdit failed to set text services cursor", hr);
            return false;
        }

        return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////
// GDI painting
/////////////////////////////////////////////////////////////////////
void XRichTextEdit::onPaintGDI(HDC hdc, const RECT& rcPaint)
{
    // copy rect
    RECTL clientRect;
    clientRect.left = m_rcClient.left;
    clientRect.right = m_rcClient.right;
    clientRect.top = m_rcClient.top;
    clientRect.bottom = m_rcClient.bottom;

    RECT updateRect = rcPaint;

    // paint
    if(m_textServices) 
    {
        // draw rich edit
        HRESULT hr = m_textServices->TxDraw(DVASPECT_CONTENT, 0, 0, 0, hdc, 0, 
                                            &clientRect, 0, &updateRect, 0, 0, TXTVIEW_ACTIVE);
        
        // check for errors
        if(FAILED(hr))
        {
            XWTRACE_HRES("XRichTextEdit: failed to draw text services", hr);
        }
    }
}

/////////////////////////////////////////////////////////////////////
// TODO: Direct2D painting (Windows 8 only)
/////////////////////////////////////////////////////////////////////
    //void    onPaintD2D(ID2D1RenderTarget* pTarget, const RECT& rcPaint); 
    //void    onInitD2DTarget(ID2D1RenderTarget* pTarget);
    //void    onResetD2DTarget();

/////////////////////////////////////////////////////////////////////
// IUnknown 
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XRichTextEdit::QueryInterface(REFIID riid, void** ppvObject)
{
    // reset pointer first
    *ppvObject = 0;

    ///// check required interface

    // IUnknown
    if(riid == IID_IUnknown)
        *ppvObject = (IUnknown*)(ITextHost*)this;

    // ITextHost
    if(riid == *XTextServices::textHostIID())
        *ppvObject = (ITextHost*)this;

    // check if interface is not supported
    if (!*ppvObject)
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) XRichTextEdit::AddRef()
{
    return ::InterlockedIncrement(&m_ulRef);
}

STDMETHODIMP_(ULONG) XRichTextEdit::Release()
{
    if(::InterlockedDecrement(&m_ulRef) == 0)
    {
        delete this;
        return 0;
    }

    return m_ulRef;
}

/////////////////////////////////////////////////////////////////////
// ITextHost 
/////////////////////////////////////////////////////////////////////
HDC XRichTextEdit::TxGetDC()
{
    // get DC from container window
    return ::GetDC(m_hwndContainer);
}

INT XRichTextEdit::TxReleaseDC(HDC hdc)
{
    // release DC 
    return ::ReleaseDC(m_hwndContainer, hdc);
}

BOOL XRichTextEdit::TxShowScrollBar(INT fnBar, BOOL fShow)
{
    // NOTE: scroll bar notifications stop working if ENM_REQUESTRESIZE is requested
    //       These events will be handled from TxNotify
    return TRUE;
}

BOOL XRichTextEdit::TxEnableScrollBar (INT fuSBFlags, INT fuArrowflags)
{
    // NOTE: scroll bar notifications stop working if ENM_REQUESTRESIZE is requested
    //       These events will be handled from TxNotify
    return TRUE;
}

BOOL XRichTextEdit::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw)
{
    // NOTE: scroll bar notifications stop working if ENM_REQUESTRESIZE is requested
    //       These events will be handled from TxNotify
    return TRUE;
}

BOOL XRichTextEdit::TxSetScrollPos (INT fnBar, INT nPos, BOOL fRedraw)
{
    // NOTE: scroll bar notifications stop working if ENM_REQUESTRESIZE is requested
    //       These events will be handled from TxNotify
    return TRUE;
}

void XRichTextEdit::TxInvalidateRect(LPCRECT prc, BOOL fMode)
{
    // invalidate rectangle 
    ::InvalidateRect(m_hwndContainer, prc, fMode);
}

void XRichTextEdit::TxViewChange(BOOL fUpdate)
{
    // update container window if needed
    if(fUpdate)
        ::UpdateWindow(m_hwndContainer);
}

BOOL XRichTextEdit::TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight)
{
    // create caret
    return ::CreateCaret(m_hwndContainer, hbmp, xWidth, yHeight);
}

BOOL XRichTextEdit::TxShowCaret(BOOL fShow)
{
    // show or hide caret (ignore if in read-only mode)
    if(fShow && !isReadOnly())
        return ::ShowCaret(m_hwndContainer);
    else 
        return ::HideCaret(m_hwndContainer);
}

BOOL XRichTextEdit::TxSetCaretPos(INT x, INT y)
{
    // set caret position
    return ::SetCaretPos(x, y);
}

BOOL XRichTextEdit::TxSetTimer(UINT idTimer, UINT uTimeout)
{
    // create timer
    return (::SetTimer(m_hwndContainer, idTimer, uTimeout, NULL) != 0);
}

void XRichTextEdit::TxKillTimer(UINT idTimer)
{
    // kill timer
    ::KillTimer(m_hwndContainer, idTimer);
}

void XRichTextEdit::TxScrollWindowEx (INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll)
{
    // NOTE: do nothing, RichEdit will scroll content

    // scroll window
    //::ScrollWindowEx(m_hwndContainer, dx, dy, lprcScroll, lprcClip, hrgnUpdate, lprcUpdate, fuScroll);
}

void XRichTextEdit::TxSetCapture(BOOL fCapture)
{
    if (fCapture)
        ::SetCapture(m_hwndContainer);	
    else
        ::ReleaseCapture();
}

void XRichTextEdit::TxSetFocus()
{
    ::SetFocus(m_hwndContainer);
}

void XRichTextEdit::TxSetCursor(HCURSOR hcur, BOOL fText)
{
    ::SetCursor(hcur);
}

BOOL XRichTextEdit::TxScreenToClient (LPPOINT lppt)
{
    return ::ScreenToClient(m_hwndContainer, lppt);
}

BOOL XRichTextEdit::TxClientToScreen (LPPOINT lppt)
{
    return ::ClientToScreen(m_hwndContainer, lppt);
}

HRESULT XRichTextEdit::TxActivate( LONG * plOldState)
{
    return S_OK;
}

HRESULT XRichTextEdit::TxDeactivate( LONG lNewState )
{
    return S_OK;
}

HRESULT XRichTextEdit::TxGetClientRect(LPRECT prc)
{
    // copy client rect
    if(prc) *prc = m_rcClient;

    return S_OK;
}

HRESULT XRichTextEdit::TxGetViewInset(LPRECT prc)
{
    // no border needed
    if(prc) 
    {
        // NOTE: if content margins are needed they can be added here
        //       https://msdn.microsoft.com/en-us/library/windows/desktop/bb787700(v=vs.85).aspx

        prc->left = prc->right = 
            prc->top = prc->bottom = 0;
    }

    return S_OK;
}

HRESULT XRichTextEdit::TxGetCharFormat(const CHARFORMATW **ppCF )
{
    // set default format
    if(ppCF) 
    {
        // set reference
        *ppCF = &m_charFormat;
    }

    return S_OK;
}

HRESULT XRichTextEdit::TxGetParaFormat(const PARAFORMAT **ppPF)
{
    // set default format
    if(ppPF)
    {
        // set reference
        *ppPF = &m_paraFormat;
    }
    
    return S_OK;
}

COLORREF XRichTextEdit::TxGetSysColor(int nIndex)
{
    // set known colors
    switch(nIndex)
    {

    // known colors
    case COLOR_HIGHLIGHTTEXT:   return m_crSelectionText;
    case COLOR_HIGHLIGHT:       return m_crSelectionBackground;
    case COLOR_WINDOW:          return m_crBackground;
    case COLOR_WINDOWTEXT:      return m_charFormat.crTextColor;

    // use system default
    default:
        return ::GetSysColor(nIndex);
    }
}

HRESULT XRichTextEdit::TxGetBackStyle(TXTBACKSTYLE *pstyle)
{
    // check if we need to fill background
    if(pstyle) *pstyle = m_fillBackground ? TXTBACK_OPAQUE : TXTBACK_TRANSPARENT;

    return S_OK;
}

HRESULT XRichTextEdit::TxGetMaxLength(DWORD *plength)
{
    // maximum length
    if(plength) *plength = m_maxLength;

    return S_OK;
}

HRESULT XRichTextEdit::TxGetScrollBars(DWORD *pdwScrollBar)
{
    // check input
    if(pdwScrollBar == 0) return E_INVALIDARG;

    // reset scrollbar properties
    *pdwScrollBar = 0;

    // check editor type
    if(isMultiline())
    {
        // scrollbars are not needed, just auto scroll 
        *pdwScrollBar = (ES_AUTOHSCROLL | ES_AUTOVSCROLL);

    } else
    {
        // scrollbars are not needed for single line edit, just auto scroll horizontally
        *pdwScrollBar |= ES_AUTOHSCROLL;
    }

    return S_OK;
}

HRESULT XRichTextEdit::TxGetPasswordChar(TCHAR *pch)
{
    if(pch) *pch = L'*';
         
    return S_OK;
}

HRESULT XRichTextEdit::TxGetAcceleratorPos(LONG *pcp)
{
    // TODO:
    if(pcp) *pcp = -1;

    return S_OK;
}

HRESULT XRichTextEdit::TxGetExtent(LPSIZEL lpExtent)
{
    // use whole client space
    if(lpExtent)
    {
        // convert to HIMETRIC
        lpExtent->cx = XGdiHelpers::pixToHimetricX(m_rcClient.right - m_rcClient.left);
        lpExtent->cy = XGdiHelpers::pixToHimetricY(m_rcClient.bottom - m_rcClient.top);
    }

    return S_OK;
}

HRESULT XRichTextEdit::OnTxCharFormatChange (const CHARFORMATW* pcf)
{
    XWASSERT(pcf);
    if(pcf == 0) E_INVALIDARG;

    // copy
    ::CopyMemory(&m_charFormat, pcf, pcf->cbSize);

    return S_OK;
}

HRESULT XRichTextEdit::OnTxParaFormatChange (const PARAFORMAT* ppf)
{
    XWASSERT(ppf);
    if(ppf == 0) E_INVALIDARG;

    // copy
    ::CopyMemory(&m_paraFormat, ppf, ppf->cbSize);

    return S_OK;
}

HRESULT XRichTextEdit::TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits)
{
    // copy properties
    if(pdwBits) *pdwBits = m_propertyBits; 

    return S_OK;
}

HRESULT XRichTextEdit::TxNotify(DWORD iNotify, void *pv)
{
    // process known notifications
    if(iNotify == EN_REQUESTRESIZE)
    {
        // requested size
        REQRESIZE *preqsz = (REQRESIZE *)pv;

        // validate
        if(preqsz == 0) return E_INVALIDARG;

        // update content rectangle
        m_rcContent = preqsz->rc;

        // inform observer
        if(m_richEditObserver) 
            m_richEditObserver->onRichEditContentRectChanged(preqsz->rc);

    } else if(iNotify == EN_VSCROLL)
    {
        // inform observer
        if(m_richEditObserver) 
            m_richEditObserver->onRichEditVerticalScrollPos(scrollOffsetY());

    } else if(iNotify == EN_HSCROLL)
    {
        // inform observer
        if(m_richEditObserver) 
            m_richEditObserver->onRichEditHorizontalScrollPos(scrollOffsetX());

    } else if(iNotify == EN_LINK)
    {
        // TODO:
        return S_OK;
    }

    return S_OK;
}

// Far East Methods for getting the Input Context
HIMC XRichTextEdit::TxImmGetContext()
{
    // TODO:
    return 0;
}

void XRichTextEdit::TxImmReleaseContext( HIMC himc )
{
    // TODO:
}

HRESULT XRichTextEdit::TxGetSelectionBarWidth (LONG *lSelBarWidth)
{
    // TODO:
    return S_OK;
}

/////////////////////////////////////////////////////////////////////
// rich text parsing (from IXRichTextParserObserver)
/////////////////////////////////////////////////////////////////////
void XRichTextEdit::onRichTextParserText(const wchar_t* text, const XTextStyle& style)
{
    COLORREF color;

    // get color from cursor position
    getTextColor(cursorPos(), color);

    // set style and same color
    onRichTextParserColoredText(text, style, color);
}

void XRichTextEdit::onRichTextParserColoredText(const wchar_t* text, const XTextStyle& style, const COLORREF& color)
{
    // ignore if not text
    XWASSERT(text);
    if(text == 0) return;

    int len = (int)::wcslen(text);
    if(len == 0) return;
    
    // insert text 
    insertText(m_parseInsertPos, text, len);

    // set style
    setTextStyle(m_parseInsertPos, len, style);

    // set color
    setTextColor(m_parseInsertPos, len, color);

    // move insertion point
    m_parseInsertPos += len;
}

void XRichTextEdit::onRichTextParserLink(const XTextRange& range, const wchar_t* url)
{
    // set url
    setTextUrl(m_parseInsertPos + range.pos, range.length, url);
}

void XRichTextEdit::onRichTextParserImage(const wchar_t* imageUri, int width, int height)
{
    // create inline image
    XTextInlineImage* inlineImage = new XTextInlineImage;
    inlineImage->AddRef();

    // load
    if(!inlineImage->setImageUri(imageUri, width, height))
    {
        XWTRACE("XRichTextEdit: failed to load inline image");
        
        // release image and exit
        inlineImage->Release();
        return;
    }

    // insert image
    insertInlineObject(m_parseInsertPos, inlineImage);

    // release reference
    inlineImage->Release();

    // move insertion point
    m_parseInsertPos += 1;
}

/////////////////////////////////////////////////////////////////////
// helper methods
/////////////////////////////////////////////////////////////////////
void XRichTextEdit::_textStyleToCharFormat(const XTextStyle& style, CHARFORMAT2* pcf2)
{
    // font size
    if(style.nFontSize)
    {
        // convert font height from style 
        pcf2->yHeight = XGdiHelpers::pixelsToTwipsY(style.nFontSize);

        // mark mask
        pcf2->dwMask |= CFM_SIZE;
    }

    // font face name length
    size_t fontNameLen = style.strFontName.length();

    // copy font face if its name fits
    if(fontNameLen > 0 && fontNameLen + 1 < LF_FACESIZE)
    {
        // copy face name
        ::wcsncpy_s(pcf2->szFaceName, LF_FACESIZE, style.strFontName.c_str(), fontNameLen);

        // mark mask
        pcf2->dwMask |= CFM_FACE;
    }

    // font properties
    pcf2->dwMask |= CFM_BOLD | CFM_ITALIC | CFM_STRIKEOUT | CFM_UNDERLINE;
    if(style.bBold) pcf2->dwEffects |= CFM_BOLD;
    if(style.bItalic) pcf2->dwEffects |= CFE_ITALIC;
    if(style.bStrike) pcf2->dwEffects |= CFE_STRIKEOUT;
    if(style.bUnderline) pcf2->dwEffects |= CFE_UNDERLINE;
}

void XRichTextEdit::_notifyPropertyChanged(DWORD bit)
{
    // ignore if not ready
    if(m_textServices == 0) return;

    // notify text services
    HRESULT hr = m_textServices->OnTxPropertyBitsChange(bit, bit);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XRichTextEdit failed to notify property bit change", hr);
    }
}

void XRichTextEdit::_setPropertyBit(DWORD bit, bool value)
{
    // store value
    if(value)
        m_propertyBits |= bit;
    else
        m_propertyBits &= ~bit;

    if(m_textServices == 0) return;

    // pass to text services
    HRESULT hr = m_textServices->OnTxPropertyBitsChange(bit, value ? bit : 0);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XRichTextEdit failed to update property bit", hr);
    }
}

bool XRichTextEdit::_getPropertyBit(DWORD bit) const
{
    // get bit value
    return (m_propertyBits & bit) != 0;
}

/////////////////////////////////////////////////////////////////////
// document methods
/////////////////////////////////////////////////////////////////////
IStorage* XRichTextEdit::_createStorage()
{
    IStorage* storage = 0;

    // create storage
    HRESULT hr = ::StgCreateStorageEx(0, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_DIRECT | STGM_CREATE, 
        STGFMT_STORAGE, 0, 0, 0, IID_IStorage, (LPVOID*)&storage);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XRichTextEdit: failed to create storage", hr);
    }

    return storage;
}

IOleClientSite* XRichTextEdit::_getOleClientSite()
{
    // ignore if not initialized
    XWASSERT(m_richEditOle);
    if(m_richEditOle == 0) return 0;

    IOleClientSite* oleClientSite = 0;

    // query client site
    HRESULT hr = m_richEditOle->GetClientSite(&oleClientSite);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XRichTextEdit: failed to qurey OLE client site", hr);
    }

    return oleClientSite;
}

ITextDocument* XRichTextEdit::_getDocument()
{
    // ignore if not initialized
    XWASSERT(m_richEditOle);
    if(m_richEditOle == 0) return 0;

    ITextDocument* textDocument = 0;

    // query document
    HRESULT hr = XTextServices::queryTextDocument(m_richEditOle, &textDocument);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XRichTextEdit: failed to query text document", hr);
    }

    return textDocument;
}

ITextRange* XRichTextEdit::_getDocumentRange(int pos, int length)
{
    // ignore if not initialized
    XWASSERT(m_richEditOle);
    if(m_richEditOle == 0) return 0;

    ITextRange* textRange = 0;

    // query range
    HRESULT hr = XTextServices::queryTextRange(m_richEditOle, pos, length, &textRange);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XRichTextEdit: failed to query text document range", hr);
    }

    return textRange;
}

ITextFont* XRichTextEdit::_getRangeFont(int pos, int length)
{
    // ignore if not initialized
    XWASSERT(m_richEditOle);
    if(m_richEditOle == 0) return 0;

    ITextFont* textFont = 0;

    // query font
    HRESULT hr = XTextServices::queryTextRangeFont(m_richEditOle, pos, length, &textFont);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XRichTextEdit: failed to query text document range font", hr);
    }

    return textFont;
}

ITextSelection* XRichTextEdit::_getSelection()
{
    // ignore if not initialized
    XWASSERT(m_richEditOle);
    if(m_richEditOle == 0) return 0;

    ITextSelection* textSelection = 0;

    // query selection
    HRESULT hr = XTextServices::queryTextSelection(m_richEditOle, &textSelection);
    if(FAILED(hr))
    {
        XWTRACE_HRES("XRichTextEdit: failed to query text selection", hr);
    }

    return textSelection;
}

// XRichTextEdit
/////////////////////////////////////////////////////////////////////

