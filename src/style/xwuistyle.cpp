// XWUI style
//
/////////////////////////////////////////////////////////////////////

#include "../xwui_config.h"
#include "../graphics/xd2dhelpres.h"

#include "xwuibitmaps.h"
#include "xwuistyle.h"

/////////////////////////////////////////////////////////////////////
// helper functions

// common styles
void    _initTextStyle(XTextStyle& textStyle);

// graphic items
void    _initTextButtonStyleDefault(XTextStyle& textStyle, XWUIStyle::XTextButtonStyle& textButtonStyle);
void    _initTextButtonStyleAction(XTextStyle& textStyle, XWUIStyle::XTextButtonStyle& textButtonStyle);
void    _initLinkButtonStyleDefault(XTextStyle& textStyle, XWUIStyle::XLinkButtonStyle& linkButtonStyle);
void    _initDefaultScrollBarStyle(XWUIStyle::XScrollBarStyle& scrollBarStyle);
void    _initDefaultScrollViewStyle(XWUIStyle::XScrollViewStyle& scrollViewStyle);

/////////////////////////////////////////////////////////////////////
// style data 
struct XWUIStyleData
{
    // common styles
    XTextStyle      textStyle;

    // graphic items
    XWUIStyle::XTextButtonStyle        textButtonStyles[XWUIStyle::eXTextButtonTypeCount];
    XWUIStyle::XLinkButtonStyle        linkButtonStyle;
    XWUIStyle::XScrollBarStyle         scrollBarStyle;
    XWUIStyle::XScrollViewStyle        scrollViewStyle;

    // default styles
    XWUIStyleData()
    {
        // common styles
        _initTextStyle(textStyle);

        // button styles
        _initTextButtonStyleDefault(textStyle, textButtonStyles[XWUIStyle::eXTextButtonTypeDefault]);
        _initTextButtonStyleAction(textStyle, textButtonStyles[XWUIStyle::eXTextButtonTypeAction]);
        _initLinkButtonStyleDefault(textStyle, linkButtonStyle);

        // scroll bar styles
        _initDefaultScrollBarStyle(scrollBarStyle);
        _initDefaultScrollViewStyle(scrollViewStyle);
    };
};

// NOTE: we need to allocate static data on the heap for memory
//       leak checking to work properly. Otherwise std::wstring in
//       static data causes false alarms
static XWUIStyleData* g_pXWUIStyleData = 0;

/////////////////////////////////////////////////////////////////////
// style instance
inline XWUIStyleData* _xwuiStyleData()
{
    if(g_pXWUIStyleData == 0)
    {
        g_pXWUIStyleData = new XWUIStyleData();
    }

    return g_pXWUIStyleData;
}

/////////////////////////////////////////////////////////////////////
// XWUIStyle - UI style

/////////////////////////////////////////////////////////////////////
// state based style properties
/////////////////////////////////////////////////////////////////////
COLORREF XWUIStyle::getStateColor(XStyleState state, const XStyleColors& colors)
{
    // check state
    if(state == eStyleStateDisabled) return colors.disabled;
    if(state == eStyleStatePressed) return colors.pressed;
    if(state == eStyleStateMouseOver) return colors.mouseover;
    if(state == eStyleStateFocused) return colors.focused;

    // normal by default
    return colors.normal;
}

D2D1_COLOR_F XWUIStyle::getStateColorD2D(XStyleState state, const XStyleColors& colors)
{
    // convert to D2D color structure
    D2D1_COLOR_F d2dColor;
    XD2DHelpers::colorrefToD2dColor(getStateColor(state, colors), d2dColor);
    return d2dColor;
}

XMediaSource XWUIStyle::getStateBitmap(XStyleState state, const XStyleBitmaps& bitmaps)
{
    // check state
    if(state == eStyleStateDisabled) return bitmaps.disabled;
    if(state == eStyleStatePressed) return bitmaps.pressed;
    if(state == eStyleStateMouseOver) return bitmaps.mouseover;
    if(state == eStyleStateFocused) return bitmaps.focused;

    // normal by default
    return bitmaps.normal;
}

/////////////////////////////////////////////////////////////////////
// default text style
/////////////////////////////////////////////////////////////////////
const XTextStyle& XWUIStyle::textStyle()
{
    return _xwuiStyleData()->textStyle;
}

// fonts
const wchar_t* XWUIStyle::fallbackFontName()
{
    // NOTE: fallback font in case some characters are not supported by selected font,
    //       must support as much scripts as possible
    return L"Arial";
}

// colors
COLORREF XWUIStyle::getColorText()
{
    return RGB(0, 0, 0);
}

COLORREF XWUIStyle::getColorDisabledText()
{
    return RGB(124, 124, 124);
}

COLORREF XWUIStyle::getColorBackground()
{
    return RGB(255, 255, 255);
}

COLORREF XWUIStyle::getColorSelection()
{
    return RGB(173, 214, 255);
}

COLORREF XWUIStyle::getColorBorder()
{
    return RGB(140, 140, 140);
}

/////////////////////////////////////////////////////////////////////
// style bitmaps
/////////////////////////////////////////////////////////////////////
bool XWUIStyle::setStyleBitmap(const wchar_t* name, const unsigned char* data, size_t size)
{
    return XWUIBitmaps::setBitmapData(name, data, size);
}

bool XWUIStyle::getStyleBitmap(const wchar_t* name, const unsigned char*& dataOut, size_t& sizeOut)
{
    return XWUIBitmaps::getBitmapData(name, dataOut, sizeOut);
}

void XWUIStyle::getButtonBitmaps(XStyleBitmapButtons button, XStyleBitmaps& bitmapsOut, XStyleSize& sizeOut)
{
    // reset bitmaps
    bitmapsOut.disabled.reset();
    bitmapsOut.focused.reset();
    bitmapsOut.mouseover.reset();
    bitmapsOut.normal.reset();
    bitmapsOut.pressed.reset();
    sizeOut.height = 0;
    sizeOut.width = 0;

    // init size
    switch(button)
    {

    // normal size
    case eStyleBitmapButtonAdd:
    case eStyleBitmapButtonRefresh:
    case eStyleBitmapButtonBack:
    case eStyleBitmapButtonDelete:
    case eStyleBitmapButtonCreateNew:
        sizeOut.height = 26;
        sizeOut.width = 26;
        break;

    // small size
    case eStyleBitmapButtonDeleteSmall:
    case eStyleBitmapButtonCloseSmall:
        sizeOut.height = 15;
        sizeOut.width = 15;
        break;

    }

    // init bitmap
    switch(button)
    {

    // normal size
    case eStyleBitmapButtonAdd:
        bitmapsOut.normal.setStylePath(L"add_icon_26.png");
        bitmapsOut.mouseover.setStylePath(L"add_selected_icon_26.png");
        bitmapsOut.pressed.setStylePath(L"add_selected_icon_26.png");
        break;
    case eStyleBitmapButtonRefresh:
        bitmapsOut.normal.setStylePath(L"refresh_icon_26.png");
        bitmapsOut.mouseover.setStylePath(L"refresh_selected_icon_26.png");
        bitmapsOut.pressed.setStylePath(L"refresh_selected_icon_26.png");
        break;
    case eStyleBitmapButtonBack:
        bitmapsOut.normal.setStylePath(L"back_icon_26.png");
        bitmapsOut.mouseover.setStylePath(L"back_selected_icon_26.png");
        bitmapsOut.pressed.setStylePath(L"back_selected_icon_26.png");
        break;
    case eStyleBitmapButtonDelete:
        bitmapsOut.normal.setStylePath(L"delete_icon_26.png");
        bitmapsOut.mouseover.setStylePath(L"delete_selected_icon_26.png");
        bitmapsOut.pressed.setStylePath(L"delete_selected_icon_26.png");
        break;
    case eStyleBitmapButtonCreateNew:
        bitmapsOut.normal.setStylePath(L"create_new_icon_26.png");
        bitmapsOut.mouseover.setStylePath(L"create_new_selected_icon_26.png");
        bitmapsOut.pressed.setStylePath(L"create_new_selected_icon_26.png");
        break;

    // small size
    case eStyleBitmapButtonDeleteSmall:
        bitmapsOut.normal.setStylePath(L"delete_15.png");
        bitmapsOut.mouseover.setStylePath(L"delete_filled_15.png");
        bitmapsOut.pressed.setStylePath(L"delete_filled_15.png");
        break;
    case eStyleBitmapButtonCloseSmall:
        bitmapsOut.normal.setStylePath(L"close_15_filled_transparent.png");
        bitmapsOut.mouseover.setStylePath(L"close_15_filled.png");
        bitmapsOut.pressed.setStylePath(L"close_15_filled.png");
        break;

    }
}

/////////////////////////////////////////////////////////////////////
// get item styles
/////////////////////////////////////////////////////////////////////
const XWUIStyle::XTextButtonStyle& XWUIStyle::textButtonStyle(XTextButtonType type)
{
    return _xwuiStyleData()->textButtonStyles[type];
}

const XWUIStyle::XLinkButtonStyle& XWUIStyle::linkButtonStyle()
{
    return _xwuiStyleData()->linkButtonStyle;
}

const XWUIStyle::XScrollBarStyle& XWUIStyle::scrollBarStyle()
{
    return _xwuiStyleData()->scrollBarStyle;
}

const XWUIStyle::XScrollViewStyle& XWUIStyle::scrollViewStyle()
{
    return _xwuiStyleData()->scrollViewStyle;
}
    
/////////////////////////////////////////////////////////////////////
// set item styles
/////////////////////////////////////////////////////////////////////
void XWUIStyle::setTextButtonStyle(XTextButtonType type, const XTextButtonStyle& style)
{
    _xwuiStyleData()->textButtonStyles[type] = style;
}

void XWUIStyle::setLinkButtonStyle(const XLinkButtonStyle& style)
{
    _xwuiStyleData()->linkButtonStyle = style;
}

void XWUIStyle::setScrollBarStyle(const XScrollBarStyle& style)
{
    _xwuiStyleData()->scrollBarStyle = style;
}

void XWUIStyle::setScrollViewStyle(const XScrollViewStyle& style)
{
    _xwuiStyleData()->scrollViewStyle = style;
}

/////////////////////////////////////////////////////////////////////
// common styles
/////////////////////////////////////////////////////////////////////
void _initTextStyle(XTextStyle& textStyle)
{
    // default text style
    if(XWUtils::sIsWindowsVistaOrAbove())
    {
        textStyle.strFontName = L"Consolas";
        textStyle.nFontSize = 13;
    } else
    {
        textStyle.strFontName = L"Verdana";
        textStyle.nFontSize = 11;
    }
}

/////////////////////////////////////////////////////////////////////
// graphic items
/////////////////////////////////////////////////////////////////////

XWUIStyle::XStyleColors _sameStyleColors(const COLORREF& color)
{
    // init same colors
    XWUIStyle::XStyleColors colors;
    colors.normal = color;
    colors.mouseover = color;
    colors.pressed = color;
    colors.disabled = color;
    colors.focused = color;

    return colors;
}

void _initTextButtonStyleDefault(XTextStyle& textStyle, XWUIStyle::XTextButtonStyle& textButtonStyle)
{
    // colors
    textButtonStyle.fillColors.normal = RGB(234, 234, 234);
    textButtonStyle.fillColors.mouseover = RGB(224, 224, 224);
    textButtonStyle.fillColors.pressed = RGB(224, 224, 224);
    textButtonStyle.fillColors.disabled = textButtonStyle.fillColors.normal;   
    textButtonStyle.fillColors.focused = textButtonStyle.fillColors.normal;   
    textButtonStyle.borderColors.normal = RGB(189, 189, 189);
    textButtonStyle.borderColors.mouseover = RGB(48, 121, 237);
    textButtonStyle.borderColors.pressed = RGB(25, 100, 218);  
    textButtonStyle.borderColors.disabled = textButtonStyle.borderColors.normal; 
    textButtonStyle.borderColors.focused = textButtonStyle.borderColors.mouseover; 
    textButtonStyle.textColors.normal = RGB(33, 50, 97);
    textButtonStyle.textColors.mouseover = RGB(33, 50, 97);   
    textButtonStyle.textColors.pressed = textButtonStyle.textColors.mouseover;    
    textButtonStyle.textColors.disabled = textButtonStyle.textColors.normal;   
    textButtonStyle.textColors.focused = textButtonStyle.textColors.mouseover;   

    // text style
    textButtonStyle.textStyle = textStyle;
    textButtonStyle.textStyle.bBold = true;

    // properties
    textButtonStyle.defaultSize.height = 25;
    textButtonStyle.defaultSize.width = 100;
    textButtonStyle.contentMargins.left = 10;
    textButtonStyle.contentMargins.right = 10;
    textButtonStyle.contentMargins.top = 5;
    textButtonStyle.contentMargins.bottom = 5;
    textButtonStyle.roundRadius = 0;
}

void _initTextButtonStyleAction(XTextStyle& textStyle, XWUIStyle::XTextButtonStyle& textButtonStyle)
{
    // colors
    textButtonStyle.fillColors.normal = RGB(75, 142, 250);
    textButtonStyle.fillColors.mouseover = RGB(65, 133, 243);
    textButtonStyle.fillColors.pressed = RGB(35, 108, 223);
    textButtonStyle.fillColors.disabled = textButtonStyle.fillColors.normal;   
    textButtonStyle.fillColors.focused = textButtonStyle.fillColors.normal;   
    textButtonStyle.borderColors.normal = RGB(48, 121, 237);
    textButtonStyle.borderColors.mouseover = RGB(47, 91, 183);
    textButtonStyle.borderColors.pressed = RGB(25, 100, 218);  
    textButtonStyle.borderColors.disabled = textButtonStyle.borderColors.normal; 
    textButtonStyle.borderColors.focused = textButtonStyle.borderColors.normal; 
    textButtonStyle.textColors.normal = RGB(254, 254, 254);
    textButtonStyle.textColors.mouseover = RGB(255, 255, 255);   
    textButtonStyle.textColors.pressed = textButtonStyle.textColors.mouseover;    
    textButtonStyle.textColors.disabled = textButtonStyle.textColors.normal;   
    textButtonStyle.textColors.focused = textButtonStyle.textColors.normal;   

    // text style
    textButtonStyle.textStyle = textStyle;
    textButtonStyle.textStyle.bBold = true;

    // properties
    textButtonStyle.defaultSize.height = 25;
    textButtonStyle.defaultSize.width = 100;
    textButtonStyle.contentMargins.left = 10;
    textButtonStyle.contentMargins.right = 10;
    textButtonStyle.contentMargins.top = 5;
    textButtonStyle.contentMargins.bottom = 5;
    textButtonStyle.roundRadius = 0;
}

void _initLinkButtonStyleDefault(XTextStyle& textStyle, XWUIStyle::XLinkButtonStyle& linkButtonStyle)
{
    // colors
    linkButtonStyle.textColors.normal = RGB(20, 200, 14);
    linkButtonStyle.textColors.mouseover = RGB(10, 120, 7);   
    linkButtonStyle.textColors.pressed = linkButtonStyle.textColors.mouseover;    
    linkButtonStyle.textColors.disabled = linkButtonStyle.textColors.normal;   
    linkButtonStyle.textColors.focused = linkButtonStyle.textColors.normal;   

    // normal style
    linkButtonStyle.normalStyle = textStyle;
    linkButtonStyle.normalStyle.bBold = true;

    // active style
    linkButtonStyle.activeStyle = textStyle;
    linkButtonStyle.activeStyle.bBold = true;

    // margins
    linkButtonStyle.contentMargins.top = 2;
    linkButtonStyle.contentMargins.bottom = 2;
    linkButtonStyle.contentMargins.left = 3;
    linkButtonStyle.contentMargins.right = 3;
}

void _initDefaultScrollBarStyle(XWUIStyle::XScrollBarStyle& scrollBarStyle)
{
    // colors
    scrollBarStyle.fillColors.normal = RGB(255, 255, 255);
    scrollBarStyle.fillColors.mouseover = RGB(242, 242, 242);
    scrollBarStyle.fillColors.pressed = scrollBarStyle.fillColors.mouseover;
    scrollBarStyle.fillColors.disabled = scrollBarStyle.fillColors.normal;   
    scrollBarStyle.fillColors.focused = scrollBarStyle.fillColors.normal;   
    scrollBarStyle.barColors.normal = RGB(203, 203, 203);
    scrollBarStyle.barColors.mouseover = RGB(144, 144, 144);
    scrollBarStyle.barColors.pressed = RGB(104, 104, 104);  
    scrollBarStyle.barColors.disabled = scrollBarStyle.barColors.normal; 
    scrollBarStyle.barColors.focused = scrollBarStyle.barColors.normal; 

    // properties
    scrollBarStyle.barWidth = 10;
    scrollBarStyle.minBarSize = 25;
}

void _initDefaultScrollViewStyle(XWUIStyle::XScrollViewStyle& scrollViewStyle)
{
    // colors
    scrollViewStyle.fillColors = _sameStyleColors(RGB(255, 255, 255));
}

/////////////////////////////////////////////////////////////////////
// release style data
/////////////////////////////////////////////////////////////////////
void XWUIStyle::releaseStyleResources()
{
    if(g_pXWUIStyleData)
    {
        delete g_pXWUIStyleData;
        g_pXWUIStyleData = 0;
    }

    // release bitmaps
    XWUIBitmaps::releaseBitmapData();
}

// XWUIStyle
/////////////////////////////////////////////////////////////////////

