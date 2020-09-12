// Base class for windows with own class
//
/////////////////////////////////////////////////////////////////////

#ifndef _XCLASSWINDOW_H_
#define _XCLASSWINDOW_H_

/////////////////////////////////////////////////////////////////////
// XClassWindowT - class window functionality

template<class _XWndType> class XClassWindowT
{
public: // construction/destruction
    XClassWindowT()
    {
    }

    ~XClassWindowT()
    {
    }

public: // window class
    static bool     sRegisterWindowClass()
    {
        static bool IsRegistered = false;

        // check if class already registered
        if(IsRegistered) return true;

        // check if class name was set
        XWASSERT(_XWndType::windowClass());
        if(_XWndType::windowClass() == 0) 
        {
            XWTRACE("XClassWindowT: failed to register class as class name was not set");
            return false;
        }

        WNDCLASSEX wc;

        // class information
        wc.cbSize        = sizeof(WNDCLASSEX);
        wc.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = sXWindowProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = ::GetModuleHandle(NULL);
        wc.hIcon         = ::LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor       = ::LoadCursor(NULL, IDC_ARROW);;
        wc.hbrBackground = 0;
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = _XWndType::windowClass();
        wc.hIconSm       = ::LoadIcon(NULL, IDI_APPLICATION);

        // register class
        if(::RegisterClassExW(&wc)) IsRegistered = true;

        // trace error
        if(!IsRegistered)
        {
            XWTRACE_WERR("XClassWindowT failed to register window class", ::GetLastError());
        }

        return IsRegistered;
    }

private: // window procedure
    static LRESULT CALLBACK sXWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        // check message
        if(uMsg == WM_NCCREATE)
        {
            // create class instance
            _XWndType* wndClassInstance = new _XWndType;

            // store pointer in window structure
            ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)(wndClassInstance));
        }

        // get window instance
        _XWndType* wndClassInstance = (_XWndType*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

        // check if we have window handle
        if(wndClassInstance == 0)
        {
            // ignore any messages prior WM_NCCREATE
            return ::DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }

        // process message
        LRESULT retVal = wndClassInstance->windowProc(hwnd, uMsg, wParam, lParam);

        // destroy class window if window is being destroyed
        if(uMsg == WM_DESTROY)
        {
            // reset pointer to class instance
            ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);

            // delete class
            delete wndClassInstance;
        }

        return retVal;
    }

private: // protect from copy and assignment
    XClassWindowT(const XClassWindowT& ref)  {}
    const XClassWindowT& operator=(const XClassWindowT& ref) { return *this;}
};

// XClassWindowT
/////////////////////////////////////////////////////////////////////

#endif // _XCLASSWINDOW_H_

