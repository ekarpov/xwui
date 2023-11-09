// Image file functionality
//
/////////////////////////////////////////////////////////////////////

#ifndef _XIMAGEFILEFILE_H_
#define _XIMAGEFILEFILE_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
struct IWICBitmapDecoder;
struct IWICBitmapSource;
struct IWICBitmapFrameDecode;
struct IWICMetadataQueryReader;
struct IWICBitmap;
struct ID2DBitmap;
class XMediaSource;

/////////////////////////////////////////////////////////////////////
// XImageFile - image file functionality

class XImageFile
{
public: // construction/destruction
    XImageFile();
    XImageFile(const wchar_t* szImageFilePath);
    ~XImageFile();

public: // read 
    bool    openFile(const wchar_t* szImageFilePath);
    bool    openResource(HMODULE hModule, const wchar_t* szResName, const wchar_t* szResType);
    bool    openStyle(const wchar_t* szImageStylePath);
    bool    open(const XMediaSource& source);
    void    close();

public: // properties
    int     width() const;
    int     height() const;

public: // frames
    bool    setActiveFrame(int nFrameIndex);
    int     activeFrameIndex() const;
    int     frameCount() const;

public: // frame animation
    
    // animation disposal methods
    enum TDisposeMethod
    {
        eDisposeUnknown,
        eDisposeNone,
        eDisposeBackground,
        eDisposePrevious
    };

    bool    hasAnimation();
    bool    getFramePlacement(int& leftOut, int& topOut, int& widthOut, int& heightOut);
    bool    getFrameDelay(int& delayOut);
    bool    getFrameDisposeMethod(TDisposeMethod& methodOut);
    bool    getBackgroundColor(COLORREF& colorOut, BYTE& alphaOut);

public: // save
    bool    create(int width, int height);
    bool    addFrame(HBITMAP hBitmap, int nFrameIndex);
    bool    addFrame(ID2DBitmap* pBitmap, int nFrameIndex);
    bool    save(const wchar_t* szImageFilePath, const char* format);

public: // basic image manipulations
    bool    scale(int width, int height);
    bool    clip(int posX, int posY, int width, int height);
    bool    rotate(int angle);

public: // frame data
    IWICBitmapSource* frameBitmapSource() const { return m_imageFrame; }

public: // image data
    IWICBitmap*     createWicBitmap() const;
    HBITMAP         createGdiBitmap(HDC hdc, bool premultiply) const;

public: // path used to load image (if loaded from file)
    const std::wstring&     path() { return m_strPath; }

private: // worker methods
    bool    _queryMetadataValueInt(IWICMetadataQueryReader* reader, const WCHAR* name, int& valueOut);

private: // protect from copy and assignment
    XImageFile(const XImageFile& ref)  {}
    const XImageFile& operator=(const XImageFile& ref) { return *this;}

private: // data
    IWICBitmapDecoder*      m_imageDecoder;
    IWICBitmapSource*       m_imageFrame;
    IWICBitmapFrameDecode*  m_frameDecode;
    int                     m_frameIndex;
    std::wstring            m_strPath;
};

// XImageFile
/////////////////////////////////////////////////////////////////////

#endif // _XIMAGEFILEFILE_H_

