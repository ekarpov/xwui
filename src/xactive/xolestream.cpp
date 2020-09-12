// OLE IStream interface implementation
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xolestream.h"

/////////////////////////////////////////////////////////////////////
// XOleMemoryStream - in memory stream implementation

/////////////////////////////////////////////////////////////////////
// construction/destruction
/////////////////////////////////////////////////////////////////////
XOleMemoryStream::XOleMemoryStream() :
    m_ulRef(0),
    m_isReadOnly(false),
    m_readPos(0),
    m_dataSize(0),
    m_dataRef(0)
{
}

XOleMemoryStream::~XOleMemoryStream()
{
    // release resources if any
    close();
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
bool XOleMemoryStream::initReadOnly(const unsigned char* data, ULONG size)
{
    // check input
    XWASSERT(data);
    XWASSERT(size);
    if(data == 0 || size == 0) return false;

    // close previous instance if any
    close();

    // copy data reference
    m_dataRef = data;
    m_dataSize = size;

    // set flag
    m_isReadOnly = true;

    return true;
}

bool XOleMemoryStream::initReadWrite(ULONG maxSize)
{
    // close previous instance if any
    close();

    // TODO: copy max size if not zero

    return true;
}

void XOleMemoryStream::close()
{
    // reset state
    m_isReadOnly = false;
    m_dataSize = 0;
    m_readPos = 0;
    m_dataRef = 0;
    m_dataBuffer.clear();
}

/////////////////////////////////////////////////////////////////////
// IUnknown 
/////////////////////////////////////////////////////////////////////
STDMETHODIMP XOleMemoryStream::QueryInterface(REFIID riid, void** ppvObject)
{
    // reset pointer first
    *ppvObject = 0;

    ///// check required interface

    // IUnknown
    if(riid == IID_IUnknown)
        *ppvObject = (IUnknown*)(IDataObject*)this;

    // IStream
    else if(riid == IID_IStream)
        *ppvObject = (IStream*)this;

    // ISequentialStream
    else if(riid == IID_ISequentialStream)
        *ppvObject = (ISequentialStream*)this;

    // check if interface is not supported
    if (!*ppvObject)
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) XOleMemoryStream::AddRef()
{
    return ::InterlockedIncrement(&m_ulRef);
}

STDMETHODIMP_(ULONG) XOleMemoryStream::Release()
{
    if(::InterlockedDecrement(&m_ulRef) == 0)
    {
        delete this;
        return 0;
    }

    return m_ulRef;
}


/////////////////////////////////////////////////////////////////////
// IStream 
/////////////////////////////////////////////////////////////////////
HRESULT XOleMemoryStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition)
{
    ULONG seekPos = 0;

    // check origin
    if(dwOrigin == STREAM_SEEK_SET)
    {
        // update seek position
        seekPos = (ULONG)dlibMove.QuadPart;

    } else if(dwOrigin == STREAM_SEEK_CUR)
    {
        // update seek position
        seekPos = m_readPos + (ULONG)dlibMove.QuadPart;

    } else if(dwOrigin == STREAM_SEEK_END)
    {
        // update seek position
        seekPos = _getDataSize() - (ULONG)dlibMove.QuadPart;

    } else
    {
        // unsupported seek type
        return E_INVALIDARG;
    }
    
    // validate new position
    if(seekPos < 0 || seekPos > _getDataSize()) return E_INVALIDARG;

    // update read position
    m_readPos = seekPos;

    // new read position
    if(plibNewPosition)
        plibNewPosition->QuadPart = m_readPos;

    return S_OK;
}

HRESULT XOleMemoryStream::SetSize(ULARGE_INTEGER libNewSize)
{
    // update size
    if(m_isReadOnly)
    {
        // not supported for readonly stream
        return E_INVALIDARG;

    } else
    {
        // resize data buffer
        m_dataBuffer.resize((size_t)libNewSize.QuadPart);

        // update read position if needed
        if(m_readPos > libNewSize.QuadPart)
            m_readPos = (ULONG)libNewSize.QuadPart;
    }

    return S_OK;
}

HRESULT XOleMemoryStream::CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten) 
{
    // check input
    XWASSERT(pstm);
    if(pstm == 0) return STG_E_INVALIDPOINTER;

    ULONG dataRead = 0;
    ULONG dataWritten = 0;
    HRESULT hr;

    // check if there is data left
    if(m_readPos < _getDataSize())
    {
        // size to read
        dataRead = (m_readPos + (ULONG)cb.QuadPart <= _getDataSize()) ? (ULONG)cb.QuadPart : (_getDataSize() - m_readPos);

        // copy data
        if(m_isReadOnly)
            hr = pstm->Write(m_dataRef + m_readPos, dataRead, &dataWritten);
        else
            hr = pstm->Write(m_dataBuffer.data() + m_readPos, dataRead, &dataWritten);

        if(FAILED(hr)) return hr;
    }

    // update read position
    m_readPos += dataRead;

    // update size
    if(pcbRead)
        pcbRead->QuadPart = dataRead;
    if(pcbWritten)
        pcbWritten->QuadPart = dataWritten;

    return S_OK;
}

HRESULT XOleMemoryStream::Commit(DWORD grfCommitFlags)
{
    // not supported
    return S_OK;
}

HRESULT XOleMemoryStream::Revert()
{
    // not supported
    return E_NOTIMPL;
}

HRESULT XOleMemoryStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    // not supported
    return E_NOTIMPL;
}

HRESULT XOleMemoryStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    // not supported
    return E_NOTIMPL;
}

HRESULT XOleMemoryStream::Stat(STATSTG* pstatstg, DWORD grfStatFlag)
{
    // check input
    XWASSERT(pstatstg);
    if(pstatstg == 0) return STG_E_INVALIDPOINTER;

    // reset output
    ::ZeroMemory(pstatstg, sizeof(STATSTG));

    // fill supported fields
    pstatstg->pwcsName = 0;
    pstatstg->type = STGTY_LOCKBYTES;
    pstatstg->cbSize.QuadPart = _getDataSize();
    pstatstg->clsid = CLSID_NULL;

    return S_OK;
}

HRESULT XOleMemoryStream::Clone(IStream** ppstm)
{
    // not supported
    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////
// ISequentialStream 
/////////////////////////////////////////////////////////////////////
HRESULT XOleMemoryStream::Read(void* pv, ULONG cb, ULONG* pcbRead)
{
    // check input
    XWASSERT(pv);
    if(pv == 0) return STG_E_INVALIDPOINTER;

    // check requested size
    if(cb == 0) return E_INVALIDARG;

    ULONG dataRead = 0;

    // check if there is data left
    if(m_readPos < _getDataSize())
    {
        // size to read
        dataRead = (m_readPos + cb <= _getDataSize()) ? cb : (_getDataSize() - m_readPos);

        // read data
        if(m_isReadOnly)
            ::CopyMemory(pv, m_dataRef + m_readPos, dataRead);
        else
            ::CopyMemory(pv, m_dataBuffer.data() + m_readPos, dataRead);
    }

    // update read position
    m_readPos += dataRead;

    // update size
    if(pcbRead)
        *pcbRead = dataRead;

    return (dataRead == cb) ? S_OK : S_FALSE;
}

HRESULT XOleMemoryStream::Write(const void *pv, ULONG cb, ULONG* pcbWritten)
{
    // check input
    XWASSERT(pv);
    if(pv == 0) return STG_E_INVALIDPOINTER;

    // check requested size
    if(cb == 0) return E_INVALIDARG;

    if(m_isReadOnly)
    {
        // cannot write to readonly buffer
        return STG_E_CANTSAVE;

    } else
    {
        // TODO: write data to buffer
        return E_NOTIMPL;
    }
}

/////////////////////////////////////////////////////////////////////
// worker methods
/////////////////////////////////////////////////////////////////////
ULONG XOleMemoryStream::_getDataSize()
{
    return m_isReadOnly ? m_dataSize : (ULONG)m_dataBuffer.size();
}

// XOleMemoryStream
/////////////////////////////////////////////////////////////////////
