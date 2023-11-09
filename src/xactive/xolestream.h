// OLE IStream interface implementation
//
/////////////////////////////////////////////////////////////////////

#ifndef _XOLESTREAM_H_
#define _XOLESTREAM_H_

/////////////////////////////////////////////////////////////////////
// XOleMemoryStream - in memory stream implementation

class XOleMemoryStream : public IStream
{
public: // construction/destruction
    XOleMemoryStream();
    virtual ~XOleMemoryStream();

public: // interface
    bool    initReadOnly(const unsigned char* data, ULONG size);
    bool    initReadWrite(ULONG maxSize = 0);
    void    close();

public: // IUnknown 
    STDMETHODIMP            QueryInterface(REFIID riid, void** ppvObject);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

public: // IStream 
    STDMETHODIMP    Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition);        
    STDMETHODIMP    SetSize(ULARGE_INTEGER libNewSize);        
    STDMETHODIMP    CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten);        
    STDMETHODIMP    Commit(DWORD grfCommitFlags);        
    STDMETHODIMP    Revert();        
    STDMETHODIMP    LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);        
    STDMETHODIMP    UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);        
    STDMETHODIMP    Stat(STATSTG* pstatstg, DWORD grfStatFlag);        
    STDMETHODIMP    Clone(IStream** ppstm);

public: // ISequentialStream 
    STDMETHODIMP    Read(void* pv, ULONG cb, ULONG* pcbRead);        
    STDMETHODIMP    Write(const void *pv, ULONG cb, ULONG* pcbWritten);

private: // worker methods
    ULONG           _getDataSize();

private: // data
    unsigned long               m_ulRef;
    bool                        m_isReadOnly;
    ULONG                       m_readPos;
    ULONG                       m_dataSize;
    const unsigned char*        m_dataRef;
    std::vector<unsigned char>  m_dataBuffer;
};

// XOleMemoryStream
/////////////////////////////////////////////////////////////////////

#endif // _XOLESTREAM_H_

