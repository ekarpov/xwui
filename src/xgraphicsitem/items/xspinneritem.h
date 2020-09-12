// Spinner progress bar
//
/////////////////////////////////////////////////////////////////////

#ifndef _XSPINNERITEM_H_
#define _XSPINNERITEM_H_

/////////////////////////////////////////////////////////////////////
// forward declarations
class XAniBitmapItem;

/////////////////////////////////////////////////////////////////////
// spinner item size

/////////////////////////////////////////////////////////////////////
// XSpinnerItem - spinner progress item

class XSpinnerItem : public XGraphicsItem
{
public: // construction/destruction
    XSpinnerItem(XWUIStyle::XSpinnerSize size, XGraphicsItem* parent = 0);
    ~XSpinnerItem();

public: // interface
    void    startProgress();
    void    stopProgress();

public: // manipulations (from IXLayoutItem)
    void    update(int posX, int posY, int width, int height);

public: // content size
    int     contentHeight();

private: // data
    XAniBitmapItem*     m_progressBitmap;
};

// XSpinnerItem
/////////////////////////////////////////////////////////////////////

#endif // _XSPINNERITEM_H_

