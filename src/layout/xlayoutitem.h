// Layout item interface
//
/////////////////////////////////////////////////////////////////////

#ifndef _XLAYOUTITEM_H_
#define _XLAYOUTITEM_H_

/////////////////////////////////////////////////////////////////////
// IXLayoutItem - layout item interface

class IXLayoutItem
{
public: // construction/destruction
    IXLayoutItem();
    virtual ~IXLayoutItem();

public: // properties
    virtual bool            isVisible() const;
    virtual unsigned long   layoutItemId() const;

public: // manipulations
    virtual void    update(int posX, int posY, int width, int height);

public: // resize policy
    enum TResizePolicy
    {
        eResizeAny,         // any size is possible
        eResizeMin,         // respect minimum size
        eResizeMax,         // respect maximum size
        eResizeMinMax       // respect maximum and minimum sizes
    };

    virtual TResizePolicy   horizontalPolicy() const;
    virtual TResizePolicy   verticalPolicy() const;
    virtual void            updateResizePolicies();

public: // size constraints
    virtual int minWidth()  const;
    virtual int minHeight() const;
    virtual int maxWidth()  const;
    virtual int maxHeight() const;
};

// IXLayoutItem
/////////////////////////////////////////////////////////////////////

#endif // _XLAYOUTITEM_H_

