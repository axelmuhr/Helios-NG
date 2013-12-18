/************************************************************************/
/* HELLO.C                                                              */
/*                                                                      */
/* This application using the Helios PC graphics library for Microsoft  */
/* Windows is a basic Hello Helios application.  The application opens  */
/* a window in which it displays the "hello helios!" text in response   */
/* to a paint message.                                                  */
/*                                                                      */
/* This application is used in the PC graphics library for windows      */
/* manual as a simple sample application.                               */
/*                                                                      */
/* Perihelion Software Ltd                                              */
/* Charlton Rd. Shepton Mallet, Somerset UK BA4 5QE                     */
/*                                                                      */
/* This application is provided copyright free.                         */
/************************************************************************/

#include <windows.h>
#include <string.h>

/* defines and global variables                                         */

char szClassName[]   = "MyTestClass";
char szWindowTitle[] = "Test Application";
char szWindowText[]  = "Hello Helios!";
#define WINDOW_STYLE    WS_OVERLAPPEDWINDOW

/* Callback function prototype                                          */

LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);

/* The standard C main routine is replaced with a WinMain in the        */
/* windows API.                                                         */

int WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, 
                LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    HWND     hWnd;
    MSG      msg;
    
    /* the first thing is to register a window class */
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInst;
    wc.hIcon         = NULL;                         /* no icon         */
    wc.hCursor       = LoadCursor(NULL, IDC_CROSS);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH); /* white background */
    wc.lpszMenuName  = NULL;                 /* can't have a menu name  */
    wc.lpszClassName = szClassName;

    /* now register the class */
    if (!RegisterClass(&wc))
        return 0;

    /* now open a window */
    hWnd = CreateWindow(szClassName,    /* class name - see above       */
        szWindowTitle,                  /* text in window title bar     */
        WINDOW_STYLE,                   /* style of window              */
        CW_USEDEFAULT, CW_USEDEFAULT,   /* default X,Y placement        */
        CW_USEDEFAULT, CW_USEDEFAULT,   /* default width and height     */
        NULL,                           /* no parent window             */
        NULL,                           /* no menu for this window      */
        hInst,                          /* instance handle              */
        NULL);                          /* no lpvData                   */

    if (hWnd == NULL)
    {
        /* no window created - unregister class and return */
        UnregisterClass(szClassName, hInst);
        return 0;
    }

    /* now actually show the window */
    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);

    /* now implement the message loop */
    while (GetMessage(&msg, NULL, 0, 0))
    {
        /* TranslateMessage(&msg); */
        DispatchMessage(&msg);
    }

    /* application is completed when reaching this point so tidy up */
    UnregisterClass(szClassName, hInst);
    return 0;
}


/* The registered window class provides a callback function which is    */
/* called each time a message arrives for a window of that class.       */

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    RECT        rect;
    PAINTSTRUCT ps;
    HDC         hDC;
    DWORD       dwTextSize;
    int         X;
    int         Y;

    switch (msg)
    {
        case WM_DESTROY:
            DefWindowProc(hWnd, msg, wParam, lParam);
            PostQuitMessage(0);
            break;

        case WM_PAINT:
            /* display the text in response to a request to paint       */
            hDC = BeginPaint(hWnd, &ps);
            GetClientRect(hWnd, &rect);         /* find the client area */

            dwTextSize = GetTextExtent(hDC, szWindowText, strlen(szWindowText));
            SetTextAlign(hDC, TA_CENTER | TA_TOP);

            X = (rect.right - rect.left) / 2 + rect.left;
            Y = (rect.bottom - rect.top - HIWORD(dwTextSize)) / 2;

            TextOut(hDC, X, Y, szWindowText, strlen(szWindowText));

            EndPaint(hWnd, &ps);
            break;

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}

