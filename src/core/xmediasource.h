// Media source
//
/////////////////////////////////////////////////////////////////////

#ifndef _XMEDIASOURCE_H_
#define _XMEDIASOURCE_H_

/////////////////////////////////////////////////////////////////////
// image source type
enum XMediaSourceType
{
    eXMediaSourceFile,
    eXMediaSourceResource,
    eXMediaSourceStyle
};

/////////////////////////////////////////////////////////////////////
// XMediaSource - media source

class XMediaSource
{
public: // construction/destruction
    XMediaSource();
    XMediaSource(const wchar_t* path, XMediaSourceType type = eXMediaSourceFile);
    XMediaSource(HMODULE hModule, const wchar_t* szResName, const wchar_t* szResType);
    ~XMediaSource();

public: // interface
    bool    isSet() const;
    void    reset();

public: // set source 
    void    setFilePath(const wchar_t* filePath);
    void    setResource(HMODULE hModule, const wchar_t* szResName, const wchar_t* szResType);
    void    setStylePath(const wchar_t* stylePath);

public: // get source 
    XMediaSourceType    type() const        { return m_sourceType; }
    const wchar_t*      path() const        { return m_sourcePath.c_str(); }
    const HMODULE       resModule() const   { return m_resModule; }
    const WCHAR*        resName() const     { return m_resNameInt ? m_resNameInt : m_resName.c_str(); }
    const WCHAR*        resType() const     { return m_resTypeInt ? m_resTypeInt : m_resType.c_str(); }

private: // data
    XMediaSourceType    m_sourceType;
    std::wstring        m_sourcePath;
    std::wstring        m_resName;
    std::wstring        m_resType;
    const WCHAR*        m_resNameInt;        
    const WCHAR*        m_resTypeInt;        
    HMODULE             m_resModule; 
};

// XMediaSource
/////////////////////////////////////////////////////////////////////

#endif // _XMEDIASOURCE_H_

