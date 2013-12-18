/************************************************************************/
/* GDITEST.c                                                            */
/*                                                                      */
/* This program demonstrates some GDI and Bitmap functions for the      */
/* Helios Windows graphics library.                                     */
/************************************************************************/

#include <nonansi.h>
#include <sem.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

/* defines */
#define WINDOW_CLASS            "GDITest"
#define num_routines            11
#define num_colors              7
#define num_styles              5
#define num_hatches             7
#define IDM_FILE_CLEAR          100
#define IDM_FILE_PERIODIC       114
#define IDM_FILE_EXIT           101
#define IDM_DEMO_STOP           102
#define IDM_DEMO_DIB            115
#define IDM_DEMO_RANDOM         103
#define IDM_DEMO_ARC            104
#define IDM_DEMO_CHORD          105
#define IDM_DEMO_ELLIPSE        106
#define IDM_DEMO_FILLRECT       107
#define IDM_DEMO_LINETO         108
#define IDM_DEMO_PIE            109
#define IDM_DEMO_POLYGON        110
#define IDM_DEMO_POLYLINE       111
#define IDM_DEMO_RECTANGLE      112
#define IDM_DEMO_SETPIXEL       113

/* structures */

typedef struct CalcRoutines {
                                UINT      ID;
                                VoidFnPtr fn;
                            } CalcRoutines;


/* forward declarations */
static BOOL InitApplication(HINSTANCE);
static BOOL TidyApplication(HINSTANCE);
static long MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void painter(void);
static void clear_window(HWND);
static void check_item(UINT);
static VoidFnPtr get_fn(UINT);
static WINT get_rand(WINT, WINT);
static HBRUSH create_brush(void);
static HPEN   create_pen(void);
static void do_dib(void);
static void calculate_arc(void);
static void calculate_chord(void);
static void calculate_ellipse(void);
static void calculate_fillrect(void);
static void calculate_lineto(void);
static void calculate_pie(void);
static void calculate_polygon(void);
static void calculate_polyline(void);
static void calculate_rectangle(void);
static void calculate_setpixel(void);
static void random(void);

/* global variables */
HWND      hWnd;
HMENU     hMenu;
HMENU     hFilePopup;
HMENU     hDemoPopup;
UINT      uLastCommand;
WINT      xMax;
WINT      yMax;
int       bits_per_pixel;
int       planes;
BOOL      PainterNeeded=TRUE;
Semaphore Running;
Semaphore PainterRequired;
Semaphore DemoRun;
Semaphore SingleRun;
Semaphore Done;

HDC       hDC;
HDC       hMemDC;
HBITMAP   hBitmap=NULL;

CalcRoutines Routines[num_routines] = {
                                        {IDM_DEMO_ARC, calculate_arc},
                                        {IDM_DEMO_CHORD, calculate_chord},
                                        {IDM_DEMO_ELLIPSE, calculate_ellipse},
                                        {IDM_DEMO_FILLRECT, calculate_fillrect},
                                        {IDM_DEMO_LINETO, calculate_lineto},
                                        {IDM_DEMO_PIE, calculate_pie},
                                        {IDM_DEMO_POLYGON, calculate_polygon},
                                        {IDM_DEMO_POLYLINE, calculate_polyline},
                                        {IDM_DEMO_RECTANGLE, calculate_rectangle},
                                        {IDM_DEMO_SETPIXEL, calculate_setpixel},
                                        {IDM_DEMO_RANDOM, random}
                                      };

COLORREF colors[num_colors] = {
                                        RGB(0, 0, 0),
                                        RGB(255, 0, 0),
                                        RGB(0, 255, 0),
                                        RGB(0, 0, 255),
                                        RGB(255, 255, 0),
                                        RGB(0, 255, 255),
                                        RGB(255, 0, 255),
                              };


int WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nShow)
{
    MSG msg;
    /* register class and create window */
    if (!InitApplication(hInst))
        return 255;

    /* spawn the painter */
    if (Fork(2000, painter, 0))
        while (GetMessage(&msg, NULL, NULL, NULL))
            DispatchMessage(&msg);

    /* tidy up after the application */
    TidyApplication(hInst);
    return 0;
}


static BOOL InitApplication(HINSTANCE hInst)
{
    WNDCLASS wc;

    InitSemaphore(&Running, 1);
    InitSemaphore(&PainterRequired, 0);
    InitSemaphore(&DemoRun, 0);
    InitSemaphore(&SingleRun, 0);
    InitSemaphore(&Done, 0);

    /* register class */
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInst;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = COLOR_APPWORKSPACE + 1;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WINDOW_CLASS;
    if (!RegisterClass(&wc))
        return FALSE;

    /* create a menu for the application */
    hMenu = CreateMenu();
    hFilePopup = CreatePopupMenu();
    hDemoPopup = CreatePopupMenu();
    AppendMenu(hMenu, MF_POPUP, hFilePopup, "&File");
    AppendMenu(hMenu, MF_POPUP, hDemoPopup, "&Demo");

    AppendMenu(hFilePopup, MF_STRING, IDM_FILE_CLEAR, "&Clear");
    AppendMenu(hFilePopup, MF_STRING, IDM_FILE_PERIODIC, "&Periodic update");
    AppendMenu(hFilePopup, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFilePopup, MF_STRING, IDM_FILE_EXIT, "E&xit");
    CheckMenuItem(hFilePopup, IDM_FILE_PERIODIC, MF_BYCOMMAND | MF_CHECKED);

    AppendMenu(hDemoPopup, MF_STRING, IDM_DEMO_STOP, "&Stop");
    AppendMenu(hDemoPopup, MF_STRING, IDM_DEMO_DIB, "&DIB");
    AppendMenu(hDemoPopup, MF_SEPARATOR, 0, NULL);
    AppendMenu(hDemoPopup, MF_STRING, IDM_DEMO_RANDOM, "&Random");
    AppendMenu(hDemoPopup, MF_SEPARATOR, 0, NULL);
    AppendMenu(hDemoPopup, MF_STRING, IDM_DEMO_ARC, "&Arc");
    AppendMenu(hDemoPopup, MF_STRING, IDM_DEMO_CHORD, "&Chord");
    AppendMenu(hDemoPopup, MF_STRING, IDM_DEMO_ELLIPSE, "&Ellipse");
    AppendMenu(hDemoPopup, MF_STRING, IDM_DEMO_FILLRECT, "&FillRect");
    AppendMenu(hDemoPopup, MF_STRING, IDM_DEMO_LINETO, "&LineTo");
    AppendMenu(hDemoPopup, MF_STRING, IDM_DEMO_PIE, "&Pie");
    AppendMenu(hDemoPopup, MF_STRING, IDM_DEMO_POLYGON, "P&olygon");
    AppendMenu(hDemoPopup, MF_STRING, IDM_DEMO_POLYLINE, "Pol&yline");
    AppendMenu(hDemoPopup, MF_STRING, IDM_DEMO_RECTANGLE, "Rec&tangle");
    AppendMenu(hDemoPopup, MF_STRING, IDM_DEMO_SETPIXEL, "Se&tpixel");
    CheckMenuItem(hDemoPopup, IDM_DEMO_STOP, MF_BYCOMMAND | MF_CHECKED);

    uLastCommand = IDM_DEMO_STOP;

    /* create a window */
    hWnd = CreateWindow(WINDOW_CLASS, "GDI Demo", WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, CW_USEDEFAULT, 
                        CW_USEDEFAULT, CW_USEDEFAULT, 
                        NULL, hMenu, hInst, NULL);
    ShowWindow(hWnd, SW_NORMAL);
    return TRUE;
}

static HBRUSH create_brush(void)
{
    int j = get_rand(0, num_hatches);
    if (j==0)
        return CreateSolidBrush(colors[get_rand(0, num_colors)]);
    else
        return CreateHatchBrush(j-1, colors[get_rand(0, num_colors)]);
}

static HPEN create_pen(void)
{
    return CreatePen(get_rand(0,num_styles), 3, colors[get_rand(0,num_colors)]);
}

static BOOL TidyApplication(HINSTANCE hInst)
{
    if (IsWindow(hWnd))
        DestroyWindow(hWnd);
    UnregisterClass(WINDOW_CLASS, hInst);
    DestroyMenu(hFilePopup);
    DestroyMenu(hDemoPopup);
    DestroyMenu(hMenu);
    
    return TRUE;
}


static long MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
        hDC = GetDC(hWnd);
        bits_per_pixel = GetDeviceCaps(hDC, BITSPIXEL);
        planes = GetDeviceCaps(hDC, PLANES);
        break;
    
    case WM_SIZE:
        WINAPISendMessage(hWnd, WM_COMMAND, IDM_DEMO_STOP, 0);
        if (hBitmap != NULL)
        {
            DeleteDC(hMemDC);
            DeleteObject(hBitmap);
        }
        hMemDC = CreateCompatibleDC(hDC);
        hBitmap = CreateCompatibleBitmap(hDC, LOWORD(lParam), HIWORD(lParam));
        xMax = LOWORD(lParam);
        yMax = HIWORD(lParam);
        SelectObject(hMemDC, hBitmap);
        clear_window(hWnd);
        UpdateWindow(hWnd);
        return DefWindowProc(hWnd, msg, wParam, lParam);

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        RECT        rect;

        BeginPaint(hWnd, &ps);

        memcpy(&rect, &ps.rcPaint, sizeof(RECT));
        BitBlt(ps.hdc, rect.left, rect.top, rect.right - rect.left,
               rect.bottom - rect.top, hMemDC, rect.left, rect.top, SRCCOPY);

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_COMMAND:
        switch(wParam)
        {
        case IDM_FILE_EXIT:
            PostMessage(hWnd, WM_CLOSE, 0, 0L);
            break;

        case IDM_FILE_CLEAR:
            clear_window(hWnd);
            UpdateWindow(hWnd);
            break;

        case IDM_FILE_PERIODIC:
            WINAPISendMessage(hWnd, WM_COMMAND, IDM_DEMO_STOP, 0);
            if (PainterNeeded)
                CheckMenuItem(hFilePopup, IDM_FILE_PERIODIC, MF_BYCOMMAND | MF_UNCHECKED);
            else
                CheckMenuItem(hFilePopup, IDM_FILE_PERIODIC, MF_BYCOMMAND | MF_CHECKED);
            PainterNeeded = !PainterNeeded;
            break;

        case IDM_DEMO_STOP:
            if (TestSemaphore(&DemoRun))
            {
                Wait(&DemoRun);
                check_item(IDM_DEMO_STOP);
                Wait(&Done);
            }
            break;

        case IDM_DEMO_DIB:
            do_dib();
            break;

        case IDM_DEMO_RANDOM:
        case IDM_DEMO_ARC:
        case IDM_DEMO_CHORD:
        case IDM_DEMO_ELLIPSE:
        case IDM_DEMO_FILLRECT:
        case IDM_DEMO_LINETO:
        case IDM_DEMO_PIE:
        case IDM_DEMO_POLYGON:
        case IDM_DEMO_POLYLINE:
        case IDM_DEMO_RECTANGLE:
        case IDM_DEMO_SETPIXEL:
            if (uLastCommand != wParam)
            {
                VoidFnPtr fn;

                WINAPISendMessage(hWnd, WM_COMMAND, IDM_DEMO_STOP, 0);

                clear_window(hWnd);

                if ((fn = get_fn(wParam)) == NULL)
                    break;
                
                Signal(&DemoRun);
                Fork(2000, fn, 0);
                if (PainterNeeded)
                    Signal(&PainterRequired);
                check_item(wParam);
            }
            break;
        }
        break;
    
    case WM_CLOSE:
        WINAPISendMessage(hWnd, WM_COMMAND, IDM_DEMO_STOP, 0);
        DefWindowProc(hWnd, msg, wParam, lParam);
        DeleteDC(hMemDC);
        DeleteObject(hBitmap);
        ReleaseDC(hWnd, hDC);
        Wait(&Running);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return DefWindowProc(hWnd, msg, wParam, lParam);
    
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}


static void painter(void)
{
    while (TestSemaphore(&Running))
    {
        Wait(&PainterRequired);
        do {
            Delay(OneSec);
            InvalidateRect(hWnd, NULL, FALSE);
            UpdateWindow(hWnd);
        } while (TestSemaphore(&DemoRun));
    }
}

static void clear_window(HWND hWnd)
{
    RECT rect;
    HBRUSH hbr = GetStockObject(WHITE_BRUSH);
    GetClientRect(hWnd, &rect);
    FillRect(hMemDC, &rect, hbr);
    InvalidateRect(hWnd, &rect, FALSE);
}

static void check_item(UINT ID)
{
    CheckMenuItem(hDemoPopup, uLastCommand, MF_BYCOMMAND | MF_UNCHECKED);
    CheckMenuItem(hDemoPopup, ID, MF_BYCOMMAND | MF_CHECKED);
    uLastCommand = ID;
}

static VoidFnPtr get_fn(UINT ID)
{
    int i;
    for (i=0; i<num_routines; i++)
        if (Routines[i].ID == ID)
            return Routines[i].fn;
    return NULL;
}

static WINT get_rand(WINT min, WINT max)
{
    if (min == max)
        return min;
    else
        return (WINT)((WINT)rand()%(max-min)+min);
}

static void do_dib(void)
{
    BITMAPINFO *bi;
    BYTE *lpBits;
    HBITMAP hBM;
    LOGPALETTE *pLogPal;
    HPALETTE hPal, hOldPal=NULL;
    int line_size, i;
    int color=0;
    BYTE c;
    PALETTEENTRY *	pPal;
    RGBQUAD * pColours;
    
    bi = (BITMAPINFO *)malloc(sizeof(BITMAPINFO) + 4*sizeof(RGBQUAD));
    if (bi == NULL)
        return;
    line_size = 52;
    lpBits = (BYTE *)malloc(line_size*100);
    if (lpBits == NULL)
    {
        free(bi);
        return;
    }
    pLogPal = (LOGPALETTE *)malloc(sizeof(LOGPALETTE)+4*sizeof(PALETTEENTRY));
    if (pLogPal == NULL)
    {
        free(bi);
        free(lpBits);
        return;
    }

    /* setup infoheader */
    bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi->bmiHeader.biWidth = 100;
    bi->bmiHeader.biHeight = 100;
    bi->bmiHeader.biPlanes = 1;
    bi->bmiHeader.biBitCount = 4;
    bi->bmiHeader.biCompression = BI_RGB;
    bi->bmiHeader.biSizeImage = 0;
    bi->bmiHeader.biXPelsPerMeter = 0;
    bi->bmiHeader.biYPelsPerMeter = 0;
    bi->bmiHeader.biClrUsed = 5;
    bi->bmiHeader.biClrImportant = 5;

    /* now setup color data */
    pLogPal->palVersion = 0x300;
    pLogPal->palNumEntries = 5;
    pLogPal->palPalEntry[0].peRed = 0;
    pLogPal->palPalEntry[0].peGreen = 50;
    pLogPal->palPalEntry[0].peBlue = 100;
    pLogPal->palPalEntry[0].peFlags = 0;

    pPal = pLogPal->palPalEntry;
    
    pPal[1].peRed = 200;
    pPal[1].peGreen = 150;
    pPal[1].peBlue = 100;
    pPal[1].peFlags = 0;

    pPal[2].peRed = 100;
    pPal[2].peGreen = 200;
    pPal[2].peBlue = 150;
    pPal[2].peFlags = 0;

    pPal[3].peRed = 102;
    pPal[3].peGreen = 102;
    pPal[3].peBlue = 102;
    pPal[3].peFlags = 0;

    pPal[4].peRed = 150;
    pPal[4].peGreen = 200;
    pPal[4].peBlue = 50;
    pPal[4].peFlags = 0;

    hPal = CreatePalette(pLogPal);
    hOldPal = SelectPalette(hDC, hPal, FALSE);
    if (!RealizePalette(hDC))
        printf("RealizePalette failed\n");

    bi->bmiColors[0].rgbRed = 0;
    bi->bmiColors[0].rgbGreen = 50;
    bi->bmiColors[0].rgbBlue = 100;
    bi->bmiColors[0].rgbReserved = 0;

    pColours = bi->bmiColors;
    
    pColours[1].rgbRed = 200;
    pColours[1].rgbGreen = 150;
    pColours[1].rgbBlue = 100;
    pColours[1].rgbReserved = 0;
    
    pColours[2].rgbRed = 100;
    pColours[2].rgbGreen = 200;
    pColours[2].rgbBlue = 150;
    pColours[2].rgbReserved = 0;

    pColours[3].rgbRed = 102;
    pColours[3].rgbGreen = 102;
    pColours[3].rgbBlue = 102;
    pColours[3].rgbReserved = 0;

    pColours[4].rgbRed = 150;
    pColours[4].rgbGreen = 200;
    pColours[4].rgbBlue = 50;
    pColours[4].rgbReserved = 0;

    /* set the bit colors */
    for (i=0; i<100; i++)
    {
        c = (BYTE)color | (BYTE)(color<<4);
        memset(&lpBits[i*line_size], c, 50);
        if ((i % 4) == 3)
            color = (color+1) % 5;
    }

    hBM = CreateDIBitmap(hDC, &bi->bmiHeader, CBM_INIT, lpBits, bi, DIB_RGB_COLORS);

    for (i=0; i<50; i++)
    {
        memset(&lpBits[52*line_size+i], 0x22, 1);
        SetDIBitsToDevice(hDC, 0, 0, 100, 100, 0, 0, 0, 100, lpBits, bi, DIB_RGB_COLORS);
    }

    SelectPalette(hDC, hOldPal, FALSE);
    DeleteObject(hPal);

    free(bi);
    free(lpBits);
    free(pLogPal);
}

static void calculate_arc(void)
{
    RECT rect;
    WINT XStart, YStart, XEnd, YEnd;
    WINT temp;
    HPEN hOldPen;
    do {
        rect.left = get_rand(0, xMax);
        rect.top = get_rand(0, yMax);
        rect.right = get_rand(0, xMax);
        rect.bottom = get_rand(0, yMax);
        if (rect.left > rect.right)
        {
            temp = rect.left;
            rect.left = rect.right;
            rect.right = temp;
        }
        if (rect.top > rect.bottom)
        {
            temp = rect.top;
            rect.top = rect.bottom;
            rect.bottom = temp;
        }
        XStart = get_rand(rect.left, rect.right);
        YStart = get_rand(rect.top, rect.bottom);
        XEnd = get_rand(rect.left, rect.right);
        YEnd = get_rand(rect.top, rect.bottom);
        hOldPen = SelectObject(hMemDC, create_pen());
        SetROP2(hMemDC, get_rand(1,17));
        Arc(hMemDC, rect.left, rect.top, rect.right, rect.bottom, XStart, YStart, XEnd, YEnd);
        DeleteObject(SelectObject(hMemDC, hOldPen));
        if (TestSemaphore(&PainterRequired) < 0)
        {
            InvalidateRect(hWnd, &rect, FALSE);
            UpdateWindow(hWnd);
        }
    } while ((TestSemaphore(&DemoRun)) && (!TestSemaphore(&SingleRun)));

    if (!TestSemaphore(&SingleRun));
        Signal(&Done);
}

static void calculate_chord(void)
{
    WINT XStart, YStart, XEnd, YEnd;
    RECT rect;
    WINT temp;
    HPEN hOldPen;
    HBRUSH hOldBrush;
    do {
        rect.left = get_rand(0, xMax);
        rect.top = get_rand(0, yMax);
        rect.right = get_rand(0, xMax);
        rect.bottom = get_rand(0, yMax);
        if (rect.left > rect.right)
        {
            temp = rect.left;
            rect.left = rect.right;
            rect.right = temp;
        }
        if (rect.top > rect.bottom)
        {
            temp = rect.top;
            rect.top = rect.bottom;
            rect.bottom = temp;
        }
        XStart = get_rand(rect.left, rect.right);
        YStart = get_rand(rect.top, rect.bottom);
        XEnd = get_rand(rect.left, rect.right);
        YEnd = get_rand(rect.top, rect.bottom);
        hOldPen = SelectObject(hMemDC, create_pen());
        hOldBrush = SelectObject(hMemDC, create_brush()); 
        SetBkColor(hMemDC, colors[get_rand(0, num_colors)]);
        SetBkMode(hMemDC, get_rand(1,3));
        SetROP2(hMemDC, get_rand(1,17)); 
        Chord(hMemDC, rect.left, rect.top, rect.right, rect.bottom, XStart, YStart, XEnd, YEnd);
        DeleteObject(SelectObject(hMemDC, hOldPen));
        DeleteObject(SelectObject(hMemDC, hOldBrush));
        if (TestSemaphore(&PainterRequired) < 0)
        {
            InvalidateRect(hWnd, &rect, FALSE);
            UpdateWindow(hWnd);
        }
    } while ((TestSemaphore(&DemoRun)) && (!TestSemaphore(&SingleRun)));
    
    if (!TestSemaphore(&SingleRun));
        Signal(&Done);
}

static void calculate_ellipse(void)
{
    WINT temp;
    RECT rect;
    HPEN hOldPen;
    HBRUSH hOldBrush;
    do {
        rect.left = get_rand(0, xMax);
        rect.top = get_rand(0, yMax);
        rect.right = get_rand(0, xMax);
        rect.bottom = get_rand(0, yMax);
        if (rect.left > rect.right)
        {
            temp = rect.left;
            rect.left = rect.right;
            rect.right = temp;
        }
        if (rect.top > rect.bottom)
        {
            temp = rect.top;
            rect.top = rect.bottom;
            rect.bottom = temp;
        }
        hOldPen = SelectObject(hMemDC, create_pen());
        hOldBrush = SelectObject(hMemDC, create_brush());
        SetBkColor(hMemDC, colors[get_rand(0, num_colors)]);
        SetBkMode(hMemDC, get_rand(1,3));
        SetROP2(hMemDC, get_rand(1,17));
        Ellipse(hMemDC, rect.left, rect.top, rect.right, rect.bottom);
        DeleteObject(SelectObject(hMemDC, hOldPen));
        DeleteObject(SelectObject(hMemDC, hOldBrush));
        if (TestSemaphore(&PainterRequired) < 0)
        {
            InvalidateRect(hWnd, &rect, FALSE);
            UpdateWindow(hWnd);
        }
    } while ((TestSemaphore(&DemoRun)) && (!TestSemaphore(&SingleRun)));
    
    if (!TestSemaphore(&SingleRun));
        Signal(&Done);
}

static void calculate_fillrect(void)
{
    RECT rect;
    WINT temp;
    HBRUSH hBrush;
    do {
        rect.left = get_rand(0, xMax);
        rect.top = get_rand(0, yMax);
        rect.right = get_rand(0, xMax);
        rect.bottom = get_rand(0, yMax);
        if (rect.left > rect.right)
        {
            temp = rect.left;
            rect.left = rect.right;
            rect.right = temp;
        }
        if (rect.top > rect.bottom)
        {
            temp = rect.top;
            rect.top = rect.bottom;
            rect.bottom = temp;
        }
        SetBkColor(hMemDC, colors[get_rand(0, num_colors)]);
        SetBkMode(hMemDC, get_rand(1,3));
        hBrush = create_brush();
        FillRect(hMemDC, &rect, hBrush);
        DeleteObject(hBrush);
        if (TestSemaphore(&PainterRequired) < 0)
        {
            InvalidateRect(hWnd, &rect, FALSE);
            UpdateWindow(hWnd);
        }
    } while ((TestSemaphore(&DemoRun)) && (!TestSemaphore(&SingleRun)));
    if (!TestSemaphore(&SingleRun));
        Signal(&Done);
}

static void calculate_lineto(void)
{
    RECT rect;
    HPEN hOldPen;
    WINT temp;
    do {
        rect.left = get_rand(0, xMax);
        rect.top = get_rand(0, yMax);
        rect.right = get_rand(0, xMax);
        rect.bottom = get_rand(0, yMax);
        hOldPen = SelectObject(hMemDC, create_pen());
        SetROP2(hMemDC, get_rand(1,17));
        MoveTo(hMemDC, rect.left, rect.top);
        LineTo(hMemDC, rect.right, rect.bottom);
        DeleteObject(SelectObject(hMemDC, hOldPen));
        if (TestSemaphore(&PainterRequired) < 0)
        {
            if (rect.left > rect.right)
            {
                temp = rect.left;
                rect.left = rect.right;
                rect.right = temp;
            }
            if (rect.top > rect.bottom)
            {
                temp = rect.top;
                rect.top = rect.bottom;
                rect.bottom = temp;
            }
            InvalidateRect(hWnd, &rect, FALSE);
            UpdateWindow(hWnd);
        }
    } while ((TestSemaphore(&DemoRun)) && (!TestSemaphore(&SingleRun)));

    if (!TestSemaphore(&SingleRun));
        Signal(&Done);
}

static void calculate_pie(void)
{
    WINT XStart, YStart, XEnd, YEnd;
    RECT rect;
    WINT temp;
    HPEN hOldPen;
    HBRUSH hOldBrush;
    do {
        rect.left = get_rand(0, xMax);
        rect.top = get_rand(0, yMax);
        rect.right = get_rand(0, xMax);
        rect.bottom = get_rand(0, yMax);
        if (rect.left > rect.right)
        {
            temp = rect.left;
            rect.left = rect.right;
            rect.right = temp;
        }
        if (rect.top > rect.bottom)
        {
            temp = rect.top;
            rect.top = rect.bottom;
            rect.bottom = temp;
        }
        XStart = get_rand(rect.left, rect.right);
        YStart = get_rand(rect.top, rect.bottom);
        XEnd = get_rand(rect.left, rect.right);
        YEnd = get_rand(rect.top, rect.bottom);
        hOldPen = SelectObject(hMemDC, create_pen());
        hOldBrush = SelectObject(hMemDC, create_brush());
        SetBkColor(hMemDC, colors[get_rand(0, num_colors)]);
        SetBkMode(hMemDC, get_rand(1,3));
        SetROP2(hMemDC, get_rand(1,17));
        Pie(hMemDC, rect.left, rect.top, rect.right, rect.bottom, XStart, YStart, XEnd, YEnd);
        DeleteObject(SelectObject(hMemDC, hOldPen));
        DeleteObject(SelectObject(hMemDC, hOldBrush));
        if (TestSemaphore(&PainterRequired) < 0)
        {
            InvalidateRect(hWnd, &rect, FALSE);
            UpdateWindow(hWnd);
        }
    } while ((TestSemaphore(&DemoRun)) && (!TestSemaphore(&SingleRun)));

    if (!TestSemaphore(&SingleRun));
        Signal(&Done);
}

static void calculate_polygon(void)
{
    WINT points, i;
    RECT rect;
    POINT p[9];
    HPEN hOldPen;
    HBRUSH hOldBrush;
    do {
        rect.left = 0x7fff;
        rect.top = 0x7fff;
        rect.right = 0;
        rect.bottom = 0;
        points = get_rand(2, 9);
        for (i=0; i<points; i++)
        {
            p[i].x = get_rand(0, xMax);
            p[i].y = get_rand(0, yMax);
            if (p[i].x < rect.left)
                rect.left = p[i].x;
            if (p[i].x > rect.right)
                rect.right = p[i].x;
            if (p[i].y < rect.top)
                rect.top = p[i].y;
            if (p[i].y > rect.bottom)
                rect.bottom = p[i].y;
        }
        hOldPen = SelectObject(hMemDC, create_pen());
        hOldBrush = SelectObject(hMemDC, create_brush());
        SetBkColor(hMemDC, colors[get_rand(0, num_colors)]);
        SetBkMode(hMemDC, get_rand(1,3));
        SetPolyFillMode(hMemDC, get_rand(1,3));
        SetROP2(hMemDC, get_rand(1,17));
        Polygon(hMemDC, &p[0], points);
        DeleteObject(SelectObject(hMemDC, hOldPen));
        DeleteObject(SelectObject(hMemDC, hOldBrush));
        if (TestSemaphore(&PainterRequired) < 0)
        {
            InvalidateRect(hWnd, &rect, FALSE);
            UpdateWindow(hWnd);
        }
    } while ((TestSemaphore(&DemoRun)) && (!TestSemaphore(&SingleRun)));
    
    if (!TestSemaphore(&SingleRun));
        Signal(&Done);
}

static void calculate_polyline(void)
{
    WINT points, i;
    RECT rect;
    POINT p[9];
    HPEN hOldPen;
    do {
        rect.left = 0x7fff;
        rect.top = 0x7fff;
        rect.right = 0;
        rect.bottom = 0;
        points = get_rand(2, 9);
        for (i=0; i<points; i++)
        {
            p[i].x = get_rand(0, xMax);
            p[i].y = get_rand(0, yMax);
            if (p[i].x < rect.left)
                rect.left = p[i].x;
            if (p[i].x > rect.right)
                rect.right = p[i].x;
            if (p[i].y < rect.top)
                rect.top = p[i].y;
            if (p[i].y > rect.bottom)
                rect.bottom = p[i].y;
        }
        hOldPen = SelectObject(hMemDC, create_pen());
        SetROP2(hMemDC, get_rand(1,17));
        Polyline(hMemDC, &p[0], points);
        DeleteObject(SelectObject(hMemDC, hOldPen));
        if (TestSemaphore(&PainterRequired) < 0)
        {
            InvalidateRect(hWnd, &rect, FALSE);
            UpdateWindow(hWnd);
        }
    } while ((TestSemaphore(&DemoRun)) && (!TestSemaphore(&SingleRun)));
    
    if (!TestSemaphore(&SingleRun));
        Signal(&Done);
}

static void calculate_rectangle(void)
{
    WINT temp;
    RECT rect;
    HPEN hOldPen;
    HBRUSH hOldBrush;
    do {
        rect.left = get_rand(0, xMax);
        rect.top = get_rand(0, yMax);
        rect.right = get_rand(0, xMax);
        rect.bottom = get_rand(0, yMax);
        if (rect.left > rect.right)
        {
            temp = rect.left;
            rect.left = rect.right;
            rect.right = temp;
        }
        if (rect.top > rect.bottom)
        {
            temp = rect.top;
            rect.top = rect.bottom;
            rect.bottom = temp;
        }
        hOldPen = SelectObject(hMemDC, create_pen());
        hOldBrush = SelectObject(hMemDC, create_brush());
        SetBkColor(hMemDC, colors[get_rand(0, num_colors)]);
        SetBkMode(hMemDC, get_rand(1,3));
        SetROP2(hMemDC, get_rand(1,17));
        Rectangle(hMemDC, rect.left, rect.top, rect.right, rect.bottom);
        DeleteObject(SelectObject(hMemDC, hOldPen));
        DeleteObject(SelectObject(hMemDC, hOldBrush));
        if (TestSemaphore(&PainterRequired) < 0)
        {
            InvalidateRect(hWnd, &rect, FALSE);
            UpdateWindow(hWnd);
        }
    } while ((TestSemaphore(&DemoRun)) && (!TestSemaphore(&SingleRun)));
    
    if (!TestSemaphore(&SingleRun));
        Signal(&Done);
}

static void calculate_setpixel(void)
{
    RECT rect;
    do {
        rect.left = get_rand(0, xMax);
        rect.right = rect.left + 1;
        rect.top = get_rand(0, yMax);
        rect.bottom = rect.top + 1;
        SetPixel(hMemDC, rect.left, rect.top, colors[get_rand(0, num_colors)]);
        if (TestSemaphore(&PainterRequired) < 0)
        {
            InvalidateRect(hWnd, &rect, FALSE);
            UpdateWindow(hWnd);
        }
    } while ((TestSemaphore(&DemoRun)) && (!TestSemaphore(&SingleRun)));
    if (!TestSemaphore(&SingleRun));
        Signal(&Done);
}

static void random(void)
{
    int i;
    Signal(&SingleRun);
    do
    {
        i = get_rand(0, num_routines-1);
        Routines[i].fn();
    } while (TestSemaphore(&DemoRun));
    Wait(&SingleRun);
    Signal(&Done);
}

