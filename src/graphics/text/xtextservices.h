// ITextServices interface helpers
//
/////////////////////////////////////////////////////////////////////

#ifndef _XTEXTSERVICES_H_
#define _XTEXTSERVICES_H_

/////////////////////////////////////////////////////////////////////
// text services 
#include <richedit.h>
#include <richole.h>
#include <textserv.h>
#include <tom.h>

/////////////////////////////////////////////////////////////////////
// XTextServices - ITextServices helper functions

namespace XTextServices
{
    // init services
    bool        initTextServices();
    void        releaseTextServices();

    // service IIDs (IID_ITextServices, IID_ITextHost)
    const IID*  textServicesIID();
    const IID*  textHostIID();

    // create text services
    bool        createTextServices(HWND hwnd, const RECT& rcClient, ITextHost* textHost, ITextServices** textServicesOut);

    // text object model
    HRESULT     queryTextDocument(IRichEditOle* richEditOle, ITextDocument** textDocumentOut);
    HRESULT     queryTextRange(IRichEditOle* richEditOle, LONG textPos, LONG textLength, ITextRange** textRangeOut);
    HRESULT     queryTextRangeFont(IRichEditOle* richEditOle, LONG textPos, LONG textLength, ITextFont** textFontOut);
    HRESULT     queryTextSelection(IRichEditOle* richEditOle, ITextSelection** textSelectionOut);
};

// XTextServices
/////////////////////////////////////////////////////////////////////

#endif // _XTEXTSERVICES_H_

