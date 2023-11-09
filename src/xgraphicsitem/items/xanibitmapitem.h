// Animation bitmap graphics item
//
/////////////////////////////////////////////////////////////////////

#ifndef _XANIBITMAPITEM_H_
#define _XANIBITMAPITEM_H_

/////////////////////////////////////////////////////////////////////
// XAniBitmapItem - animation bitmap

class XAniBitmapItem : public XImageItem
{
public: // construction/destruction
    XAniBitmapItem(XGraphicsItem* parent = 0);
    ~XAniBitmapItem();
        
public: // interface
    void    setBitmaps(int width, int height, const std::vector<XMediaSource>& bitmaps);
    void    setAnimationInterval(unsigned long intervalMs);

public: // animation
    void    startAnimation();
    void    stopAnimation();

protected: // timer
    bool    onTimerEvent();

public: // scrolling interface (from IXWScrollable)
    int     contentWidth();
    int     contentHeight();

protected: // GDI resource caching (from XGraphicsItem)
    void    onInitGDIResources(HDC hdc);
    void    onResetGDIResources();

protected: // Direct2D resource caching (from XGraphicsItem)
    void    onInitD2DTarget(ID2D1RenderTarget* pTarget);
    void    onResetD2DTarget();

private: // worker methods
    void    _loadBitmaps();
    void    _releaseBitmaps();
    void    _setActiveBitmap();

private: // data
    std::vector<XMediaSource>       m_bitmaps;
    std::vector<std::wstring>       m_bitmapHashes;
    size_t          m_activeBitmap;
    bool            m_animationActive;
    int             m_bitmapWidth;
    int             m_bitmapHeight;
    unsigned long   m_animationInterval;
};

// XAniBitmapItem
/////////////////////////////////////////////////////////////////////

#endif // _XANIBITMAPITEM_H_


