// OLE IDataObject interface implementation
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xoledataobject.h"

/////////////////////////////////////////////////////////////////////
// XOleDataObject - OLE IDataObject implementation

/////////////////////////////////////////////////////////////////////
// construction/destruction
/////////////////////////////////////////////////////////////////////
XOleDataObject::XOleDataObject() :
    m_ulRef(0),
    m_hasData(false),
    m_formatIndex(0),
    m_bRelease(FALSE)
{
}

XOleDataObject::~XOleDataObject()
{
    // release content if any
    _releaseContent();
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
bool XOleDataObject::setBitmapData(HBITMAP bitmap, bool copyBitmap)
{
    // check input
    XWASSERT(bitmap);
    if(bitmap == 0) return false;

    // release previous content if any
    _releaseContent();

    // bitmap
	m_oleStgmed.tymed = TYMED_GDI;
	m_oleStgmed.hBitmap = bitmap;
	m_oleStgmed.pUnkForRelease = NULL;

    // format
	m_oleFormat.cfFormat = CF_BITMAP;           // Clipboard format = CF_BITMAP
	m_oleFormat.ptd = NULL;						// Target Device = Screen
	m_oleFormat.dwAspect = DVASPECT_CONTENT;	// Level of detail = Full content
	m_oleFormat.lindex = -1;					// Index = Not applicaple
	m_oleFormat.tymed = TYMED_GDI;				// Storage medium = HBITMAP handle

    // copy bitmap if needed
    if(copyBitmap)
    {
        m_oleStgmed.hBitmap = (HBITMAP)::OleDuplicateData(m_oleStgmed.hBitmap, CF_BITMAP, 0);
        if(m_oleStgmed.hBitmap == 0) return false;
    }

    // update state
    m_bRelease = copyBitmap ? TRUE : FALSE;
    m_hasData = true;

    return true;
}

/////////////////////////////////////////////////////////////////////
// create OLE object
/////////////////////////////////////////////////////////////////////
HRESULT XOleDataObject::createOleObject(IOleClientSite* oleClientSite, IStorage* storage, IOleObject** objectOut)
{
    // check input
    XWASSERT(oleClientSite);
    XWASSERT(storage);
    XWASSERT(objectOut);
    if(oleClientSite == 0 || storage == 0 || objectOut == 0) return E_INVALIDARG;

    // ignore if no data set
    if(!m_hasData) return E_NOT_VALID_STATE; 

    // create object
    return ::OleCreateFromData(this, IID_IOleObject, OLERENDER_FORMAT, 
        &m_oleFormat, oleClientSite, storage, (LPVOID*)objectOut);
}

HRESULT XOleDataObject::createStaticOleObject(IOleClientSite* oleClientSite, IStorage* storage, IOleObject** objectOut)
{
    // check input
    XWASSERT(oleClientSite);
    XWASSERT(storage);
    XWASSERT(objectOut);
    if(oleClientSite == 0 || storage == 0 || objectOut == 0) return E_INVALIDARG;

    // ignore if no data set
    if(!m_hasData) return E_NOT_VALID_STATE; 

    // create object
    return ::OleCreateStaticFromData(this, IID_IOleObject, OLERENDER_FORMAT, 
        &m_oleFormat, oleClientSite, storage, (LPVOID*)objectOut);
}

/////////////////////////////////////////////////////////////////////
// IUnknown 
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XOleDataObject::QueryInterface(REFIID riid, void** ppvObject)
{
    // reset pointer first
    *ppvObject = 0;

    ///// check required interface

    // IUnknown
    if(riid == IID_IUnknown)
        *ppvObject = (IUnknown*)(IDataObject*)this;

    // IDataObject
    else if(riid == IID_IDataObject)
        *ppvObject = (IDataObject*)this;

    // IEnumFORMATETC
    else if(riid == IID_IEnumFORMATETC)
        *ppvObject = (IEnumFORMATETC*)this;

    // check if interface is not supported
    if (!*ppvObject)
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) XOleDataObject::AddRef()
{
    return ::InterlockedIncrement(&m_ulRef);
}

STDMETHODIMP_(ULONG) XOleDataObject::Release()
{
    if(::InterlockedDecrement(&m_ulRef) == 0)
    {
        delete this;
        return 0;
    }

    return m_ulRef;
}

/////////////////////////////////////////////////////////////////////
// IDataObject 
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XOleDataObject::GetData(FORMATETC* pformatetcIn, STGMEDIUM* pmedium)
{
    // check input
    XWASSERT(pformatetcIn);
    XWASSERT(pmedium);
    if(pformatetcIn == 0 || pmedium == 0) return E_INVALIDARG;

    // ignore if no data set
    if(!m_hasData) return E_NOT_VALID_STATE; 

    // match format
    if(!_matchFormat(pformatetcIn)) return DV_E_FORMATETC;

    // copy supported formats
    switch(m_oleFormat.tymed)
    {
	case TYMED_ISTREAM:
	case TYMED_ISTORAGE:
		pmedium->pstm = m_oleStgmed.pstm;
		pmedium->pstm->AddRef();
        break;

	case TYMED_HGLOBAL: 
	case TYMED_GDI: 
	case TYMED_MFPICT: 
	case TYMED_ENHMF:
	case TYMED_FILE:
		pmedium->hGlobal = ::OleDuplicateData(m_oleStgmed.hGlobal, m_oleFormat.cfFormat, 0);
        break;

	default:
		return DV_E_FORMATETC;
    }

	// copy the rest
	pmedium->tymed = m_oleStgmed.tymed;
	pmedium->pUnkForRelease = m_oleStgmed.pUnkForRelease;
    if(pmedium->pUnkForRelease)
        pmedium->pUnkForRelease->AddRef();

    return S_OK;
}

STDMETHODIMP XOleDataObject::GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium)
{
    // ignore if no data set
    if(!m_hasData) return E_NOT_VALID_STATE; 

    // do nothing in default implementation
    return E_NOTIMPL;
}

STDMETHODIMP XOleDataObject::QueryGetData(FORMATETC* pformatetc)
{
    // check input
    XWASSERT(pformatetc);
    if(pformatetc == 0) return E_INVALIDARG;

    // ignore if no data set
    if(!m_hasData) return E_NOT_VALID_STATE; 

    // match format
    return _matchFormat(pformatetc) ? S_OK : DV_E_FORMATETC;
}

STDMETHODIMP XOleDataObject::GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut)
{
	if(pformatetcOut) 
        pformatetcOut->ptd = NULL;

    // do nothing in default implementation
    return E_NOTIMPL;
}

STDMETHODIMP XOleDataObject::SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease)
{
    // check input
    XWASSERT(pformatetc);
    XWASSERT(pmedium);
	if(pformatetc == 0 || pmedium == 0) return E_INVALIDARG;

    // release old data if any
    _releaseContent();

    // just copy
    m_oleFormat = *pformatetc;
    m_oleStgmed = *pmedium;
    m_bRelease = fRelease;
    m_hasData = true;

    return S_OK;
}

STDMETHODIMP XOleDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
{
    // check input
    XWASSERT(ppenumFormatEtc);
	if(ppenumFormatEtc == 0) return E_INVALIDARG;

    // ignore if no data set
    if(!m_hasData) return E_NOT_VALID_STATE; 

    // copy reference to itself
    *ppenumFormatEtc = this;
    AddRef();

    return S_OK;
}

STDMETHODIMP XOleDataObject::DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
    // do nothing in default implementation
    return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP XOleDataObject::DUnadvise(DWORD dwConnection)
{
    // do nothing in default implementation
    return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP XOleDataObject::EnumDAdvise(IEnumSTATDATA** ppenumAdvise)
{
    // do nothing in default implementation
    return OLE_E_ADVISENOTSUPPORTED;
}

/////////////////////////////////////////////////////////////////////
// IEnumFORMATETC 
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XOleDataObject::Next(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched)
{
	// check input
    XWASSERT(rgelt);
	if(rgelt == 0) return E_INVALIDARG;

    // ignore if no data set
    if(!m_hasData) return E_NOT_VALID_STATE; 

    // we support only one format
    if(m_formatIndex > 0) 
    {
	    // store result
	    if(pceltFetched != 0) *pceltFetched = 0;
        return S_FALSE;
    }

	// copy format
    *rgelt = m_oleFormat;
    if(m_oleFormat.ptd)
    {
		// allocate memory for the DVTARGETDEVICE if necessary
		rgelt->ptd = (DVTARGETDEVICE*)::CoTaskMemAlloc(sizeof(DVTARGETDEVICE));

		// copy the contents of the source DVTARGETDEVICE
		*(rgelt->ptd) = *(m_oleFormat.ptd);
    }

	// store result
	if(pceltFetched != 0) 
		*pceltFetched = 1;

    // advance index
    m_formatIndex++;

    return (celt == 1) ? S_OK : S_FALSE;
}

STDMETHODIMP XOleDataObject::Skip(ULONG celt)
{
    // ignore if no data set
    if(!m_hasData) return E_NOT_VALID_STATE; 

    m_formatIndex += celt;

    // we support only one format
    return (m_formatIndex == 0) ? S_OK : S_FALSE;
}

STDMETHODIMP XOleDataObject::Reset(void)
{
    // ignore if no data set
    if(!m_hasData) return E_NOT_VALID_STATE; 

    // reset index
	m_formatIndex = 0;

	return S_OK;
}

STDMETHODIMP XOleDataObject::Clone(IEnumFORMATETC** ppenum)
{
    // check input
    XWASSERT(ppenum);
    if(ppenum == 0) return E_INVALIDARG;

    // ignore if no data set
    if(!m_hasData) return E_NOT_VALID_STATE; 

    // copy reference to itself
    *ppenum = this;
    AddRef();

    return S_OK;
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
void XOleDataObject::_releaseContent()
{
    if(m_hasData && m_bRelease)
        ::ReleaseStgMedium(&m_oleStgmed);

    // reset state
    m_hasData = false;
    m_bRelease = FALSE;
}

bool XOleDataObject::_matchFormat(FORMATETC* pformatetc)
{
    XWASSERT(pformatetc);
    if(pformatetc == 0) return false;

	if(pformatetc->lindex != -1 && 
       pformatetc->lindex != m_oleFormat.lindex) return false;

	if((pformatetc->tymed & m_oleFormat.tymed)   &&
		pformatetc->cfFormat == m_oleFormat.cfFormat && 
		pformatetc->dwAspect == m_oleFormat.dwAspect) return true;

    return false;
}

// XOleDataObject
/////////////////////////////////////////////////////////////////////

