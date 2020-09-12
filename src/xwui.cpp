// xWUI - eXtended Windows UI Library
// Copyright (c) 2020 Evgeny Karpov
//
/////////////////////////////////////////////////////////////////////

#include "xwui.h"

#include <comdef.h>

/////////////////////////////////////////////////////////////////////
// use XP like version of common controls
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

/////////////////////////////////////////////////////////////////////
// required windows libraries
#pragma comment (lib, "Comctl32.lib")

/////////////////////////////////////////////////////////////////////
// global data

// painter type
XWUIGraphicsPainter g_xwuiGraphicsPainter = XWUI_PAINTER_GDI;

/////////////////////////////////////////////////////////////////////
// init library

bool    sInitXWUILibrary(XWUIGraphicsPainter painter)
{
    // NOTE: COINIT_APARTMENTTHREADED is required if ActiveX controls
    //       are to be used

    // NOTE: for some ActiveX controls (like WebBrowser) we need to use
    //       OleInitialize as it enables clipboard support and drag-and-drop
    //
    //       More information: https://msdn.microsoft.com/en-us/library/aa770041(v=vs.85).aspx
    //       MSDN: Your application should use OleInitialize rather than CoInitialize to start 
    //             COM. OleInitialize enables support for the Clipboard, drag-and-drop operations, 
    //             OLE, and in-place activation

    // NOTE: OleInitialize identifies the concurrency model as single-thread apartment (STA)
    //       OleInitialize calls CoInitializeEx internally with COINIT_APARTMENTTHREADED.

    // init COM runtime 
    HRESULT res = ::OleInitialize(0);
    if(res != S_OK && res != S_FALSE)
    {
        XWTRACE_HRES("Failed to initialize COM subsytem, ActiveX controls will not work", res);
    }

    // init common controls
    INITCOMMONCONTROLSEX iccx;
    iccx.dwSize=sizeof(INITCOMMONCONTROLSEX);
    iccx.dwICC=   
            ICC_LISTVIEW_CLASSES        // listview, header
        |   ICC_TREEVIEW_CLASSES        // treeview, tooltips
        |   ICC_BAR_CLASSES             // toolbar, statusbar, trackbar, tooltips
        |   ICC_TAB_CLASSES             // tab, tooltips
        |   ICC_UPDOWN_CLASS            // updown
        |   ICC_PROGRESS_CLASS          // progress
        |   ICC_HOTKEY_CLASS            // hotkey
        |   ICC_ANIMATE_CLASS           // animate
        |   ICC_WIN95_CLASSES    
        |   ICC_DATE_CLASSES            // month picker, date picker, time picker, updown
        |   ICC_USEREX_CLASSES          // comboex
        |   ICC_COOL_CLASSES            // rebar (coolbar) control
        |   ICC_INTERNET_CLASSES 
        |   ICC_PAGESCROLLER_CLASS      // page scroller
        |   ICC_NATIVEFNTCTL_CLASS      // native font control
        ;
    InitCommonControlsEx(&iccx);

    // load rich Edit library
    if(::LoadLibraryW(L"Msftedit.dll") == 0)
    {
        XWTRACE("Failed to load Msftedit.dll, backing up to older RichEdit control");
        ::LoadLibraryW(L"Riched20.dll");
    }

    // use Direct2D by default if available
    if(painter == XWUI_PAINTER_AUTOMATIC)
    {
        // TODO: first check if Direct2D is supported
        // TODO: check if screen dpi is default and PC is not very powerful then use GDI painter

        painter = XWUI_PAINTER_D2D;
    }

    // init graphics resources
    if(!sInitXWUIGraphics((painter == XWUI_PAINTER_D2D), (painter == XWUI_PAINTER_D2D)))
    {
        XWTRACE("Failed to init graphics system, some UI elements will not work");
    }

    // check if Direct2D has been loaded
    if(painter == XWUI_PAINTER_D2D && (!XD2DHelpers::isDirect2DLoaded() || !XDWriteHelpers::isDirectWriteLoaded()))
    {
        // Direct2D not available, use GDI instead
        painter = XWUI_PAINTER_GDI;
    }

    // set default painter
    g_xwuiGraphicsPainter = painter;

    // register extended controls
    if(!sRegisterXWUIControls())
    {
        XWTRACE("Failed to register extended controls, some UI elements will not work");
    }

    return true;
}

void    sCloseXWUILibrary()
{
    // close animation timer if any
    XWAnimationTimer::closeIstance();

    // release content provider if any
    XWContentProvider::releaseInstance();

    // delete message hooks
    sXWUIClearMessageHooks();

    // release locale
    XWUILocale::releaseLocaleResources();

    // release style 
    XWUIStyle::releaseStyleResources();

    // release global resources
    XWUtils::releaseResources();

    // close graphics resources
    sCloseXWUIGraphics();

    // close COM runtime
    ::OleUninitialize();
}

/////////////////////////////////////////////////////////////////////
// register extended controls
bool    sRegisterXWUIControls()
{
    bool bRetVal = true;

    // register control classes
//    bRetVal &= XTextLabelWindow::sRegisterWindowClass();

    return bRetVal;
}

/////////////////////////////////////////////////////////////////////
// message loop
WPARAM  sXWUIRunMessageLoop()
{
    // message loop
    MSG Msg;
    while(::GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        // process message hooks first
        if(!sXWUIProcessMessageHooks(&Msg))
        {
            ::TranslateMessage(&Msg);
            ::DispatchMessage(&Msg);
        }
    }

    return Msg.wParam;
}

/////////////////////////////////////////////////////////////////////
// default painter
XWUIGraphicsPainter sXWUIDefaultPainter()
{
    return g_xwuiGraphicsPainter;
}

/////////////////////////////////////////////////////////////////////
