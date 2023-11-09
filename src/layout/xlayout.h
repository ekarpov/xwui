// Base functionality for window layout engine
//
/////////////////////////////////////////////////////////////////////

#ifndef _XLAYOUT_H_
#define _XLAYOUT_H_

/////////////////////////////////////////////////////////////////////
// IXLayout - layout manager interface

class IXLayout : public IXLayoutItem
{
public: // construction/destruction
    IXLayout();
    virtual ~IXLayout();

public: // enum layout items
    virtual int             layoutItemCount() const = 0;
    virtual void            removelayoutItemAt(int idx) = 0;
    virtual IXLayoutItem*   layoutItemAt(int idx) const = 0;

public: // layout items by id
    virtual void            removelayoutItem(unsigned long itemId) = 0;
    virtual IXLayoutItem*   layoutItem(unsigned long itemId) const = 0;

public: // content margins
    void    setContentMargins(int left, int top, int right, int bottom);
    int     marginLeft() const      { return m_nMarginLeft; }
    int     marginTop() const       { return m_nMarginTop; }
    int     marginRight() const     { return m_nMarginRight; }
    int     marginBottom() const    { return m_nMarginBottom; }

private: // content margins
    int     m_nMarginLeft;
    int     m_nMarginTop;
    int     m_nMarginRight;
    int     m_nMarginBottom;
};

// IXLayout
/////////////////////////////////////////////////////////////////////

#endif // _XLAYOUT_H_

