// Media source
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"

#include "xmediasource.h"

/////////////////////////////////////////////////////////////////////
// XMediaSource - media source

/////////////////////////////////////////////////////////////////////
// construction/destruction
/////////////////////////////////////////////////////////////////////
XMediaSource::XMediaSource() :
    m_sourceType(eXMediaSourceFile),
    m_resNameInt(0),
    m_resTypeInt(0),
    m_resModule(0)
{
}

XMediaSource::XMediaSource(const wchar_t* path, XMediaSourceType type) :
    m_sourceType(eXMediaSourceFile),
    m_resNameInt(0),
    m_resTypeInt(0),
    m_resModule(0)
{
    // init source
    if(type == eXMediaSourceFile)
        setFilePath(path);
    else if(type == eXMediaSourceResource)
        setResource(0, path, 0);
    else if(type == eXMediaSourceStyle)
        setStylePath(path);
}

XMediaSource::XMediaSource(HMODULE hModule, const wchar_t* szResName, const wchar_t* szResType)
{
    // init source
    setResource(hModule, szResName, szResType);
}

XMediaSource::~XMediaSource()
{
}

/////////////////////////////////////////////////////////////////////
// interface
/////////////////////////////////////////////////////////////////////
bool XMediaSource::isSet() const
{
    // check type
    if(m_sourceType == eXMediaSourceResource)
        return (m_resNameInt != 0) || (m_resName.length() > 0);
    else
        return m_sourcePath.length() > 0;
}

void XMediaSource::reset()
{
    // reset source
    m_sourceType = eXMediaSourceFile;
    m_sourcePath.clear();
    m_resName.clear();
    m_resType.clear();
    m_resNameInt = 0;
    m_resTypeInt = 0;
    m_resModule = 0;
}

/////////////////////////////////////////////////////////////////////
// set source 
/////////////////////////////////////////////////////////////////////
void XMediaSource::setFilePath(const wchar_t* filePath)
{
    XWASSERT(filePath);
    if(filePath == 0) return;

    // reset previous value
    reset();

    // set as file
    m_sourceType = eXMediaSourceFile;
    m_sourcePath = filePath;
}

void XMediaSource::setResource(HMODULE hModule, const wchar_t* szResName, const wchar_t* szResType)
{
    XWASSERT(szResName);
    if(szResName == 0) return;

    // reset previous value
    reset();

    // set as resource
    m_sourceType = eXMediaSourceResource;
    m_resModule = hModule;

    // copy name
    if(IS_INTRESOURCE(szResName))
        m_resNameInt = szResName;
    else
        m_resName = szResName;

    // copy type
    if(szResType)
    {
        if(IS_INTRESOURCE(szResType))
            m_resTypeInt = szResType;
        else
            m_resType = szResType;
    }
}

void XMediaSource::setStylePath(const wchar_t* stylePath)
{
    XWASSERT(stylePath);
    if(stylePath == 0) return;

    // reset previous value
    reset();

    // set as style bitmap
    m_sourceType = eXMediaSourceStyle;
    m_sourcePath = stylePath;
}

// XMediaSource
/////////////////////////////////////////////////////////////////////

