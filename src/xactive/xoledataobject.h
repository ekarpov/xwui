// OLE IDataObject interface implementation
//
/////////////////////////////////////////////////////////////////////

#ifndef _XOLEDATAOBJECT_H_
#define _XOLEDATAOBJECT_H_

/////////////////////////////////////////////////////////////////////
// XOleDataObject - OLE IDataObject implementation

class XOleDataObject : public IDataObject,
                       public IEnumFORMATETC
{
public: // construction/destruction
    XOleDataObject();
    virtual ~XOleDataObject();

public: // interface
    bool        setBitmapData(HBITMAP bitmap, bool copyBitmap = false);

public: // create OLE object
    HRESULT     createOleObject(IOleClientSite* oleClientSite, IStorage* storage, IOleObject** objectOut);
    HRESULT     createStaticOleObject(IOleClientSite* oleClientSite, IStorage* storage, IOleObject** objectOut);

public: // IUnknown 
    STDMETHODIMP            QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

public: // IDataObject 
    STDMETHODIMP    GetData(FORMATETC* pformatetcIn, STGMEDIUM* pmedium);
    STDMETHODIMP    GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium);
    STDMETHODIMP    QueryGetData(FORMATETC* pformatetc);
    STDMETHODIMP    GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut);
    STDMETHODIMP    SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease);
    STDMETHODIMP    EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc);
    STDMETHODIMP    DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
    STDMETHODIMP    DUnadvise(DWORD dwConnection);
    STDMETHODIMP    EnumDAdvise(IEnumSTATDATA** ppenumAdvise);

public: // IEnumFORMATETC 
    STDMETHODIMP    Next(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched);        
    STDMETHODIMP    Skip(ULONG celt);        
    STDMETHODIMP    Reset(void);       
    STDMETHODIMP    Clone(IEnumFORMATETC** ppenum);

private: // worker methods
    void            _releaseContent();
    bool            _matchFormat(FORMATETC* pformatetc);

private: // data
    unsigned long   m_ulRef;
    bool            m_hasData;
    int             m_formatIndex;
	FORMATETC       m_oleFormat;
	STGMEDIUM       m_oleStgmed;
    BOOL	        m_bRelease;
};

// XOleDataObject
/////////////////////////////////////////////////////////////////////

#endif // _XOLEDATAOBJECT_H_

