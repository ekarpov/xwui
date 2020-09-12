// XWUI style
//
/////////////////////////////////////////////////////////////////////

#ifndef _XWUI_STYLE_H_
#define _XWUI_STYLE_H_

/////////////////////////////////////////////////////////////////////
// XWUIStyle - UI style

namespace XWUIStyle
{
    ///// style types

    // size
    struct XStyleSize
    {
        int             width;
        int             height;
    };

    // margins
    struct XStyleMargins
    {
        int             left;
        int             right;
        int             top;
        int             bottom;
    };

    enum XStyleState
    {
        eStyleStateDefault,
        eStyleStateMouseOver,
        eStyleStatePressed,
        eStyleStateDisabled,
        eStyleStateFocused,

        eStyleStateCount    // must be the last
    };

    // state based colors
    struct XStyleColors
    {
        COLORREF        normal;
        COLORREF        mouseover;
        COLORREF        pressed;
        COLORREF        disabled;
        COLORREF        focused;
    };

    // state based bitmaps
    struct XStyleBitmaps
    {
        XMediaSource    normal;
        XMediaSource    mouseover;
        XMediaSource    pressed;
        XMediaSource    disabled;
        XMediaSource    focused;
    };

    // spinner size
    enum XSpinnerSize
    {
        eSpinnerSize26,
        eSpinnerSize32
    };

    // bitmap buttons
    enum XStyleBitmapButtons
    {
        // normal size
        eStyleBitmapButtonAdd,
        eStyleBitmapButtonRefresh,
        eStyleBitmapButtonBack,
        eStyleBitmapButtonDelete,
        eStyleBitmapButtonCreateNew,

        // small size
        eStyleBitmapButtonDeleteSmall,
        eStyleBitmapButtonCloseSmall
    };

    // state based style properties
    COLORREF        getStateColor(XStyleState state, const XStyleColors& colors);
    D2D1_COLOR_F    getStateColorD2D(XStyleState state, const XStyleColors& colors);
    XMediaSource    getStateBitmap(XStyleState state, const XStyleBitmaps& bitmaps);

    ///// common styles

    // default text style
    const XTextStyle&   textStyle();

    // fonts
    const wchar_t*  fallbackFontName();

    // colors
    COLORREF        getColorText();
    COLORREF        getColorDisabledText();
    COLORREF        getColorBackground();
    COLORREF        getColorSelection();
    COLORREF        getColorBorder();

    ///// style bitmaps
    bool    setStyleBitmap(const wchar_t* name, const unsigned char* data, size_t size);
    bool    getStyleBitmap(const wchar_t* name, const unsigned char*& dataOut, size_t& sizeOut);

    void    getButtonBitmaps(XStyleBitmapButtons button, XStyleBitmaps& bitmapsOut, XStyleSize& sizeOut);

    ///// graphics item styles

    // XTextButtonItem style
    struct XTextButtonStyle
    {
        // colors
        XStyleColors    fillColors;
        XStyleColors    borderColors;
        XStyleColors    textColors;

        // text style
        XTextStyle      textStyle;

        // properties
        XStyleSize      defaultSize;
        XStyleMargins   contentMargins;
        int             roundRadius;
    };

    // XLinkButtonItem style
    struct XLinkButtonStyle
    {
        // colors
        XStyleColors    textColors;

        // text styles
        XTextStyle      normalStyle;
        XTextStyle      activeStyle;
        
        // properties
        XStyleMargins   contentMargins;
    };

    // XTextButtonItem types
    enum XTextButtonType
    {
        eXTextButtonTypeDefault,
        eXTextButtonTypeAction,
        eXTextButtonTypeCancel,

        eXTextButtonTypeCount       // not a type, must be the last
    };

    // XScrollBarItem style
    struct XScrollBarStyle
    {
        // colors
        XStyleColors    fillColors;
        XStyleColors    barColors;

        // properties
        int             barWidth;
        int             minBarSize;
    };

    // XScrollViewItem style
    struct XScrollViewStyle
    {
        // colors
        XStyleColors    fillColors;
    };

    // get item styles
    const XTextButtonStyle&     textButtonStyle(XTextButtonType type);
    const XLinkButtonStyle&     linkButtonStyle();
    const XScrollBarStyle&      scrollBarStyle();
    const XScrollViewStyle&     scrollViewStyle();
    
    // set item styles
    void        setTextButtonStyle(XTextButtonType type, const XTextButtonStyle& style);
    void        setLinkButtonStyle(const XLinkButtonStyle& style);
    void        setScrollBarStyle(const XScrollBarStyle& style);
    void        setScrollViewStyle(const XScrollViewStyle& style);
    
    // release style data
    void    releaseStyleResources();
};

// XWUIStyle
/////////////////////////////////////////////////////////////////////

#endif // _XWUI_STYLE_H_


