/************************************************************************/
/************************************************************************/
/*               Microsoft Windows Libraries for Helios                 */
/*                                                                      */
/*       Copyright (C) 1990-1993,  Perihelion Software Limited          */
/*                         All Rights Reserved                          */
/*                                                                      */
/*   gdi.c        to build windows.lib                                  */
/************************************************************************/
/************************************************************************/

#include <syslib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <codes.h>
#include <nonansi.h>
#include <unistd.h>
#include <signal.h>
#include "windows.h"                 /* include the std API header file */

#include "windefs.h"

BOOL WINAPI ShowWindow(HWND hWnd, int nCmdShow)
{
    return (BOOL)send_2_parameters(IO_ShowWindow, hWnd, nCmdShow);
}


void WINAPI GetWindowRect(HWND hWnd, LPRECT lprc)
{
    word Control = (word)hWnd;
    if (lprc == NULL)
	return;
    send_fn(IO_GetWindowRect, 1, sizeof(RECT), &Control, (BYTE *)lprc);
}


void WINAPI GetClientRect(HWND hWnd, LPRECT lprc)
{
    word Control = (word)hWnd;
    if (lprc == NULL)
	return;
    send_fn(IO_GetClientRect, 1, sizeof(RECT), &Control, (BYTE *)lprc);
}


HPEN WINAPI CreatePen(int style, int width, COLORREF color)
{
    return (HPEN)send_3_parameters(IO_CreatePen, style, width, color);
}


HBRUSH WINAPI CreateSolidBrush(COLORREF color)
{
    return (HBRUSH)send_1_parameter(IO_CreateSolidBrush, color);
}


HFONT WINAPI CreateFont(int height, int width, int escape, int orientation,
			int weight, BYTE italic, BYTE underline, 
			BYTE strikeout, BYTE charset, BYTE precision,
			BYTE clipprecision, BYTE quality, BYTE pitch,
			LPCSTR lpszFace)
{
    word Control[5];
    BYTE *Data = (BYTE *)malloc(strlen(lpszFace)+9);
    if (Data == (BYTE *)NULL)
	return NULL;

    Control[0] = (word)height;
    Control[1] = (word)width;
    Control[2] = (word)escape;
    Control[3] = (word)orientation;
    Control[4] = (word)weight;
    Data[0] = italic;
    Data[1] = underline;
    Data[2] = strikeout;
    Data[3] = charset;
    Data[4] = precision;
    Data[5] = clipprecision;
    Data[6] = quality;
    Data[7] = pitch;
    if (lpszFace == NULL)
	Data[8] = 0;
    else
	strcpy(&Data[8], lpszFace);
    if (!send_message(IO_CreateFont, 5, strlen(lpszFace)+9, &Control[0], &Data[0]))
    {
	free(Data);
	return NULL;
    }

    free(Data);
    return (HFONT)Control[0];
}



HGDIOBJ WINAPI SelectObject(HDC hDC, HGDIOBJ obj)
{
    return (HGDIOBJ)send_2_parameters(IO_SelectObject, hDC, obj);
}



HDC WINAPI GetDC(HWND hWnd)
{
    return (HDC)send_1_parameter(IO_GetDC, hWnd);
}


int WINAPI ReleaseDC(HWND hWnd, HDC hDC)
{
    return (int)send_2_parameters(IO_ReleaseDC, hWnd, hDC);
}



HDC WINAPI BeginPaint(HWND hWnd, PAINTSTRUCT FAR* lpps)
{
    word Control = (word)hWnd;
    if (lpps == NULL)
	return NULL;
    if (!send_message(IO_BeginPaint, 1, sizeof(PAINTSTRUCT), &Control, (BYTE *)lpps))
	return (HDC)NULL;
    return (HDC)Control;
}


void WINAPI EndPaint(HWND hWnd, const PAINTSTRUCT FAR* lpps)
{
    word Control = hWnd;
    send_message(IO_EndPaint, 1, sizeof(PAINTSTRUCT), &Control, (BYTE *)lpps);
}



HGDIOBJ WINAPI GetStockObject(int obj_no)
{
    return (HGDIOBJ)send_1_parameter(IO_GetStockObject, obj_no);
}



BOOL WINAPI DPtoLP(HDC hDC, LPPOINT lppt, int cPoints)
{
    word Control[2];
    if ((lppt == NULL) && (cPoints > 0))
	return FALSE;
    if (cPoints < 0)
	return FALSE;
    Control[0] = (word)hDC;
    Control[1] = (word)cPoints;
    if (!send_message(IO_DPtoLP, 2, cPoints*sizeof(POINT), &Control[0], (BYTE *)lppt))
	return FALSE;
    return (BOOL)Control[0];
}


BOOL WINAPI LPtoDP(HDC hDC, LPPOINT lppt, int cPoints)
{
    word Control[2];
    if ((lppt == NULL) && (cPoints > 0))
	return FALSE;
    if (cPoints < 0)
	return FALSE;
    Control[0] = (word)hDC;
    Control[1] = (word)cPoints;
    if (!send_message(IO_LPtoDP, 2, cPoints*sizeof(POINT), &Control[0], (BYTE *)lppt))
	return FALSE;
    return (BOOL)Control[0];
}


void WINAPI ClientToScreen(HWND hWnd, LPPOINT lppt)
{
    word Control = hWnd;
    if (lppt == NULL)
	return;
    send_message(IO_ClientToScreen, 1, sizeof(POINT), &Control, (BYTE *)lppt);
}


void WINAPI ScreenToClient(HWND hWnd, LPPOINT lppt)
{
    word Control = hWnd;
    if (lppt == NULL)
	return;
    send_message(IO_ScreenToClient, 1, sizeof(POINT), &Control, (BYTE *)lppt);
}


/* this is unbuffered MoveTo,  MoveToEx should be buffered */
DWORD WINAPI MoveTo(HDC hDC, int x, int y)
{
    return (DWORD)send_3_parameters(IO_MoveTo, hDC, x, y);
}


BOOL WINAPI LineTo(HDC hDC, int xEnd, int yEnd)
{
    return (BOOL)send_3_parameters(IO_LineTo, hDC, xEnd, yEnd);
}



int WINAPI FillRect(HDC hDC, const RECT *lprc, HBRUSH hbr)
{
    word Control[2];
    Control[0] = (word)hDC;
    Control[1] = (word)hbr;
    if (lprc == NULL)
	return 0;
    send_message(IO_FillRect, 2, sizeof(RECT), &Control[0], (BYTE*)lprc);
    return 0;
}


BOOL WINAPI TextOut(HDC hDC, int xStart, int yStart, LPCSTR lpszString, int cb)
{
    word Control[4];
    if (lpszString == NULL)
	return FALSE;
    Control[0] = (word)hDC;
    Control[1] = xStart;
    Control[2] = yStart;
    Control[3] = cb;
    if (!send_message(IO_TextOut, 4, strlen(lpszString)+1, &Control[0], (BYTE*)lpszString))
	return FALSE;
    return (BOOL)Control[0];
}



BOOL WINAPI GetTextMetrics(HDC hDC, TEXTMETRIC FAR* lptm)
{
    word Control = (word)hDC;
    if (lptm == NULL)
	return FALSE;
    if (!send_fn(IO_GetTextMetrics, 1, sizeof(TEXTMETRIC), &Control, (BYTE *)lptm))
	return FALSE;
    return (BOOL)Control;
}



UINT WINAPI GetTextAlign(HDC hDC)
{
    return (UINT)send_1_parameter(IO_GetTextAlign, hDC);
}


UINT WINAPI SetTextAlign(HDC hDC, UINT al)
{
    return (UINT)send_2_parameters(IO_SetTextAlign, hDC, al);
}


DWORD WINAPI GetTextExtent(HDC hDC, LPCSTR lpszString, int cb)
{
    word Control[2];
    if (lpszString == NULL)
	return FALSE;
    Control[0] = (word)hDC;
    Control[1] = cb;
    if (!send_message(IO_GetTextExtent, 2, strlen(lpszString)+1, &Control[0], (BYTE*)lpszString))
	return 0L;
    return (DWORD)Control[0];
}


BOOL WINAPI DeleteObject(HGDIOBJ hObject)
{
    return (BOOL)send_1_parameter(IO_DeleteObject, hObject);
}

BOOL WINAPI MoveWindow(HWND hWnd, int x, int y, int width, int height, BOOL bRepaint)
{
    return (BOOL)send_6_parameters(IO_MoveWindow, hWnd, x, y, width, height, bRepaint);
}

void WINAPI AdjustWindowRect(LPRECT lprc, DWORD dwStyle, BOOL bMenu)
{
    word Control[2];
    if (lprc == NULL)
	return;
    Control[0] = (word)dwStyle;
    Control[1] = (word)bMenu;
    send_message(IO_AdjustWindowRect, 2, sizeof(RECT), &Control[0], (BYTE *)lprc);
}


BOOL WINAPI EnableWindow(HWND hWnd, BOOL bEnable)
{
    return (BOOL)send_2_parameters(IO_EnableWindow, hWnd, bEnable);
}


HWND WINAPI GetActiveWindow(void)
{
    return (HWND)send_1_parameter(IO_GetActiveWindow, 0);
}


LONG WINAPI GetClassLong(HWND hWnd, int offset)
{
    return (LONG)send_2_parameters(IO_GetClassLong, hWnd, offset);
}


WORD WINAPI GetClassWord(HWND hWnd, int offset)
{
    return (WORD)send_2_parameters(IO_GetClassWord, hWnd, offset);
}


HWND WINAPI GetDesktopWindow(void)
{
    return (HWND)send_1_parameter(IO_GetDesktopWindow, 0);
}


HWND WINAPI GetFocus(void)
{
    return (HWND)send_1_parameter(IO_GetFocus, 0);
}


HWND WINAPI GetNextWindow(HWND hWnd, UINT uFlag)
{
    return (HWND)send_2_parameters(IO_GetNextWindow, hWnd, uFlag);
}


HWND WINAPI GetParent(HWND hWnd)
{
    return (HWND)send_1_parameter(IO_GetParent, hWnd);
}


HWND WINAPI GetTopWindow(HWND hWnd)
{
    return (HWND)send_1_parameter(IO_GetTopWindow, hWnd);
}


HWND WINAPI GetWindow(HWND hWnd, UINT uRel)
{
    return (HWND)send_2_parameters(IO_GetWindow, hWnd, uRel);
}


LONG WINAPI GetWindowLong(HWND hWnd, int offset)
{
    return (LONG)send_2_parameters(IO_GetWindowLong, hWnd, offset);
}


WORD WINAPI GetWindowWord(HWND hWnd, int offset)
{
    return (WORD)send_2_parameters(IO_GetWindowWord, hWnd, offset);
}


BOOL WINAPI IsChild(HWND hParent, HWND hChild)
{
    return (BOOL)send_2_parameters(IO_IsChild, hParent, hChild);
}


BOOL WINAPI IsWindow(HWND hWnd)
{
    return (BOOL)send_1_parameter(IO_IsWindow, hWnd);
}


BOOL WINAPI IsWindowEnabled(HWND hWnd)
{
    return (BOOL)send_1_parameter(IO_IsWindowEnabled, hWnd);
}


BOOL WINAPI IsWindowVisible(HWND hWnd)
{
    return (BOOL)send_1_parameter(IO_IsWindowVisible, hWnd);
}


LONG WINAPI SetClassLong(HWND hWnd, int offset, LONG lValue)
{
    if (offset < 0)
	return 0L;   /* cannot access the menu name or wndproc */
    return (LONG)send_3_parameters(IO_SetClassLong, hWnd, offset, lValue);
}


WORD WINAPI SetClassWord(HWND hWnd, int offset, WORD nValue)
{
    return (WORD)send_3_parameters(IO_SetClassWord, hWnd, offset, nValue);
}


LONG WINAPI SetWindowLong(HWND hWnd, int offset, LONG lValue)
{
    if (offset == GWL_WNDPROC)
	return 0L;
    return (LONG)send_3_parameters(IO_SetWindowLong, hWnd, offset, lValue);
}


WORD WINAPI SetWindowWord(HWND hWnd, int offset, WORD nValue)
{
    return (WORD)send_3_parameters(IO_SetWindowWord, hWnd, offset, nValue);
}


HWND WINAPI SetCapture(HWND hWnd)
{
    return (HWND)send_1_parameter(IO_SetCapture, hWnd);
}


void WINAPI ReleaseCapture(void)
{
   send_message(IO_ReleaseCapture, 0, 0, NULL, NULL);
}


HCURSOR WINAPI SetCursor(HCURSOR hCursor)
{
    return (HCURSOR)send_1_parameter(IO_SetCursor, hCursor);
}


/* this only allows the loading of existing cursors at the moment */
/* since we do not have a resource file                           */
HCURSOR WINAPI LoadCursor(HINSTANCE hInst, LPCSTR lpszName)
{
    if (hInst != NULL)
	return NULL;
    return (HCURSOR)send_1_parameter(IO_LoadCursor, (word)lpszName);
}


COLORREF WINAPI GetBkColor(HDC hDC)
{
    return (COLORREF)send_1_parameter(IO_GetBkColor, hDC);
}


int WINAPI GetBkMode(HDC hDC)
{
    return (int)send_1_parameter(IO_GetBkMode, hDC);
}


int WINAPI GetDeviceCaps(HDC hDC, int index)
{
    return (int)send_2_parameters(IO_GetDeviceCaps, hDC, index);
}


int WINAPI GetMapMode(HDC hDC)
{
    return (int)send_1_parameter(IO_GetMapMode, hDC);
}


int WINAPI GetSystemMetrics(int index)
{
    return (int)send_1_parameter(IO_GetSystemMetrics, index);
}


COLORREF WINAPI GetTextColor(HDC hDC)
{
    return (COLORREF)send_1_parameter(IO_GetTextColor, hDC);
}


COLORREF WINAPI SetBkColor(HDC hDC, COLORREF cr)
{
    return (COLORREF)send_2_parameters(IO_SetBkColor, hDC, cr);
}


int WINAPI SetBkMode(HDC hDC, int mode)
{
    return (int)send_2_parameters(IO_SetBkMode, hDC, mode);
}


int WINAPI SetMapMode(HDC hDC, int mode)
{
    return (int)send_2_parameters(IO_SetMapMode, hDC, mode);
}


COLORREF WINAPI SetTextColor(HDC hDC, COLORREF cr)
{
    return (COLORREF)send_2_parameters(IO_SetTextColor, hDC, cr);
}


BOOL WINAPI Arc(HDC hDC, int nLeftRect,  int nTopRect,
			 int nRightRect, int nBottomRect,
			 int nXStartArc, int nYStartArc,
			 int nXEndArc,   int nYEndArc)
{
    word Control[9];
    Control[0] = (word)hDC;
    Control[1] = (word)nLeftRect;
    Control[2] = (word)nTopRect;
    Control[3] = (word)nRightRect;
    Control[4] = (word)nBottomRect;
    Control[5] = (word)nXStartArc;
    Control[6] = (word)nYStartArc;
    Control[7] = (word)nXEndArc;
    Control[8] = (word)nYEndArc;
    if (!send_message(IO_Arc, 9, 0, &Control[0], NULL))
	return FALSE;
    return (BOOL)Control[0];
}


BOOL WINAPI Chord(HDC hDC, int nLeftRect,   int nTopRect,
			   int nRightRect,  int nBottomRect,
			   int nXStartLine, int nYStartLine,
			   int nXEndLine,   int nYEndLine)
{
    word Control[9];
    Control[0] = (word)hDC;
    Control[1] = (word)nLeftRect;
    Control[2] = (word)nTopRect;
    Control[3] = (word)nRightRect;
    Control[4] = (word)nBottomRect;
    Control[5] = (word)nXStartLine;
    Control[6] = (word)nYStartLine;
    Control[7] = (word)nXEndLine;
    Control[8] = (word)nYEndLine;
    if (!send_message(IO_Chord, 9, 0, &Control[0], NULL))
	return FALSE;
    return (BOOL)Control[0];
}


HBRUSH WINAPI CreateHatchBrush(int style, COLORREF cr)
{
    return (HBRUSH)send_2_parameters(IO_CreateHatchBrush, style, cr);
}


HBRUSH WINAPI CreatePatternBrush(HBITMAP hBitmap)
{
    return (HBRUSH)send_1_parameter(IO_CreatePatternBrush, hBitmap);
}


BOOL WINAPI Ellipse(HDC hDC, int nLeft, int nTop, int nRight, int nBottom)
{
    return (BOOL)send_5_parameters(IO_Ellipse, hDC, nLeft, nTop, nRight, nBottom);
}


BOOL WINAPI FloodFill(HDC hDC, int X, int Y, COLORREF cr)
{
    return (BOOL)send_4_parameters(IO_FloodFill, hDC, X, Y, cr);
}


DWORD WINAPI GetCurrentPosition(HDC hDC)
{
    return (DWORD)send_1_parameter(IO_GetCurrentPosition, hDC);
}


COLORREF WINAPI GetNearestColor(HDC hDC, COLORREF cr)
{
    return (COLORREF)send_2_parameters(IO_GetNearestColor, hDC, cr);
}


COLORREF WINAPI GetPixel(HDC hDC, int X, int Y)
{
    return (COLORREF)send_3_parameters(IO_GetPixel, hDC, X, Y);
}


int WINAPI GetPolyFillMode(HDC hDC)
{
    return (int)send_1_parameter(IO_GetPolyFillMode, hDC);
}



int WINAPI GetROP2(HDC hDC)
{
    return (int)send_1_parameter(IO_GetROP2, hDC);
}


void WINAPI InvalidateRect(HWND hWnd, const RECT *lprc, BOOL bErase)
{
    word Control[2];
    Control[0] = (word)hWnd;
    Control[1] = (word)bErase;
    if (lprc == NULL)
	send_message(IO_InvalidateRect, 2, 1, &Control[0], (BYTE*)&lprc);
    else
	send_message(IO_InvalidateRect, 2, sizeof(RECT), &Control[0], (BYTE *)lprc);
}


void WINAPI InvertRect(HWND hWnd, const RECT *lprc)
{
    word Control = (word)hWnd;
    if (lprc == NULL)
	return;
    send_message(IO_InvalidateRect, 1, sizeof(RECT), &Control, (BYTE *)lprc);
}


BOOL WINAPI Pie(HDC hDC, int nLeftRect,  int nTopRect,
			 int nRightRect, int nBottomRect,
			 int nXStartArc, int nYStartArc,
			 int nXEndArc,   int nYEndArc)
{
    word Control[9];
    Control[0] = (word)hDC;
    Control[1] = (word)nLeftRect;
    Control[2] = (word)nTopRect;
    Control[3] = (word)nRightRect;
    Control[4] = (word)nBottomRect;
    Control[5] = (word)nXStartArc;
    Control[6] = (word)nYStartArc;
    Control[7] = (word)nXEndArc;
    Control[8] = (word)nYEndArc;
    if (!send_message(IO_Pie, 9, 0, &Control[0], NULL))
	return FALSE;
    return (BOOL)Control[0];
}


BOOL WINAPI Polygon(HDC hDC, const POINT *lppt, int count)
{
    word Control[3];
    HGLOBAL hBuffer;

    hBuffer = SendBuffer((BYTE *)lppt, (word)sizeof(POINT)*(word)count);
    if (hBuffer == NULL)
	return FALSE;
    Control[0] = (word)hDC;
    Control[1] = (word)hBuffer;
    Control[2] = (word)count;
    if (!send_message(IO_Polygon, 3, 0, &Control[0], NULL))
    {
	DeleteBuffer(hBuffer);
	return FALSE;
    }
    DeleteBuffer(hBuffer);
    return (BOOL)Control[0];
}


BOOL WINAPI Polyline(HDC hDC, const POINT *lppt, int count)
{
    word Control[3];
    HGLOBAL hBuffer;

    hBuffer = SendBuffer((BYTE *)lppt, (word)sizeof(POINT)*(word)count);
    if (hBuffer == NULL)
	return FALSE;
    Control[0] = (word)hDC;
    Control[1] = (word)hBuffer;
    Control[2] = (word)count;
    if (!send_message(IO_Polyline, 3, 0, &Control[0], NULL))
    {
	DeleteBuffer(hBuffer);
	return FALSE;
    }
    DeleteBuffer(hBuffer);
    return (BOOL)Control[0];
}


BOOL WINAPI Rectangle(HDC hDC, int nLeft, int nTop, int nRight, int nBottom)
{
    return (BOOL)send_5_parameters(IO_Rectangle, hDC, nLeft, nTop, nRight, nBottom);
}


COLORREF WINAPI SetPixel(HDC hDC, int X, int Y, COLORREF cr)
{
    return (COLORREF)send_4_parameters(IO_SetPixel, hDC, X, Y, cr);
}


int WINAPI SetPolyFillMode(HDC hDC, int mode)
{
    return (int)send_2_parameters(IO_SetPolyFillMode, hDC, mode);
}


int WINAPI SetROP2(HDC hDC, int mode)
{
    return (int)send_2_parameters(IO_SetROP2, hDC, mode);
}


BOOL WINAPI UnrealizeObject(HGDIOBJ hObject)
{
    return (BOOL)send_1_parameter(IO_UnrealizeObject, hObject);
}


void WINAPI ValidateRect(HWND hWnd, const RECT *lprc)
{
    word Control = (word)hWnd;
    send_message(IO_InvalidateRect, 1, sizeof(RECT), &Control, (BYTE *)lprc);
}


HPALETTE WINAPI CreatePalette(const LOGPALETTE FAR *lplgpl)
{
    word Control;
    int size = (int)sizeof(LOGPALETTE) + (lplgpl->palNumEntries-1)*(int)sizeof(PALETTEENTRY);
    if (!send_message(IO_CreatePalette, 1, size, &Control, (BYTE *)lplgpl))
	return NULL;
    return (HPALETTE)Control;
}


UINT WINAPI GetPaletteEntries(HPALETTE hPal, UINT iStart, UINT iNo, PALETTEENTRY FAR *lppe)
{
    word Control[3];
    if (lppe == NULL)
	return 0;
    Control[0] = (word)hPal;
    Control[1] = (word)iStart;
    Control[2] = (word)iNo;
    if (!send_fn(IO_GetPaletteEntries, 3, iNo*sizeof(PALETTEENTRY), &Control[0], (BYTE *)lppe))
	return 0;
    return (UINT)Control[0];
}


UINT WINAPI GetNearestPaletteIndex(HPALETTE hPal, COLORREF cr)
{
    return (UINT)send_2_parameters(IO_GetNearestPaletteIndex, hPal, cr);
}


UINT WINAPI RealizePalette(HDC hDC)
{
    return (UINT)send_1_parameter(IO_RealizePalette, hDC);
}


HPALETTE WINAPI SelectPalette(HDC hDC, HPALETTE hPal, BOOL bBackgnd)
{
    return (HPALETTE)send_3_parameters(IO_SelectPalette, hDC, hPal, bBackgnd);
}


UINT WINAPI SetPaletteEntries(HPALETTE hPal, UINT iStart, UINT iNo, const PALETTEENTRY FAR *lppe)
{
    word Control[3];
    if ((iNo == 0) || (lppe == NULL))
	return 0;
    Control[0] = (word)hPal;
    Control[1] = (word)iStart;
    Control[2] = (word)iNo;
    if (!send_message(IO_SetPaletteEntries, 3, iNo*sizeof(PALETTEENTRY), &Control[0], (BYTE *)lppe))
	return 0;
    return (UINT)Control[0];
}


BOOL WINAPI BitBlt(HDC hDC, int XDest, int YDest, int width, int height,
		   HDC hSrc, int XSrc, int YSrc, DWORD dwRop)
{
    word Control[9];
    Control[0] = (word)hDC;
    Control[1] = (word)XDest;
    Control[2] = (word)YDest;
    Control[3] = (word)width;
    Control[4] = (word)height;
    Control[5] = (word)hSrc;
    Control[6] = (word)XSrc;
    Control[7] = (word)YSrc;
    Control[8] = (word)dwRop;
    if (!send_message(IO_BitBlt, 9, 0, &Control[0], NULL))
	return FALSE;
    return (BOOL)Control[0];
}


HBITMAP WINAPI CreateBitmap(int nWidth, int nHeight, UINT planes, UINT bpp, const VOID FAR* lpvBits)
{
    HGLOBAL hMem;
    HBITMAP hResult;

    if (lpvBits == (VOID FAR *)NULL)
	hMem = NULL;   /* handle for data is null */
    else
    {
	/* first send the bits of data and use the handle as a parameter */
	int size = (nWidth * nHeight * planes * bpp) / 8;
	hMem = SendBuffer((BYTE *)lpvBits, size);
	if (hMem == (HGLOBAL)NULL)
	    return NULL;
    }

    hResult = (HBITMAP)send_5_parameters(IO_CreateBitmap, nWidth, nHeight, planes, bpp, hMem);
    DeleteBuffer(hMem);
    return hResult;
}


HBITMAP WINAPI CreateCompatibleBitmap(HDC hDC, int nWidth, int nHeight)
{
    return (HBITMAP)send_3_parameters(IO_CreateCompatibleBitmap, hDC, nWidth, nHeight);
}

HBITMAP WINAPI CreateDIBitmap(HDC hDC, BITMAPINFOHEADER *lpbih,
			      DWORD dwInit, const void *lpvBits,
			      BITMAPINFO *lpbi, UINT ColorUse)
{
    word Control[4];
    BYTE *Data;
    int  size, bitmap_size, colors_used;
    int  info_size = 0;
    HGLOBAL hMem = NULL;

    if (lpbih == NULL)
	return NULL;

    /* put standard data into control buffer */
    Control[0] = hDC;
    Control[1] = dwInit;
    Control[2] = ColorUse;
    Control[3] = NULL;

    /* work out data size required */
    bitmap_size = 0;
    size = (int)lpbih->biSize;
    if (dwInit == CBM_INIT)
    {
	if ((lpvBits == NULL) || (lpbi == NULL))
	    return NULL;
	if ((lpbih->biCompression != BI_RGB) && (lpbih->biSizeImage == 0))
	    return NULL;
	if (lpbih->biSizeImage == 0)
	    bitmap_size = (int)((lpbih->biWidth*lpbih->biBitCount + 31)/32 * 4 * lpbih->biHeight);
	else
	    bitmap_size = (int)lpbih->biSizeImage;
	info_size = sizeof(BITMAPINFO);
	if (lpbih->biClrUsed == 0)
	    colors_used = 1 << lpbih->biBitCount;
	else
	    colors_used = (int)lpbih->biClrUsed;
	info_size += (colors_used - 1)*sizeof(RGBQUAD);
	size += info_size;
    }

    /* allocate data and copy the data across */
    Data = (BYTE *)malloc(size);
    memcpy(Data, lpbih, (int)lpbih->biSize);
    if (dwInit == CBM_INIT)
    {
	memcpy(&Data[lpbih->biSize], lpbi, info_size);
	Control[3] = hMem = SendBuffer((BYTE *)lpvBits, bitmap_size);
	if (hMem == NULL)
	{
	    free(Data);
	    return NULL;
	}
    }
    if (!send_message(IO_CreateDIBitmap, 4, size, &Control[0], &Data[0]))
    {
	free(Data);
	if (hMem) DeleteBuffer(hMem);
	return NULL;
    }
    free(Data);
    if (hMem) DeleteBuffer(hMem);
    return (HBITMAP)Control[0];
}


LONG WINAPI GetBitmapBits(HBITMAP hBitmap, LONG cbBuffer, void FAR *lpvBits)
{
    word Control[2];

    if (lpvBits == NULL)
	return 0;

    Control[0] = hBitmap;
    Control[1] = cbBuffer;

    if (!send_message(IO_GetBitmapBits, 2, 0, &Control[0], NULL))
	return 0;

    /* now get the data from the buffer */
    if (ReceiveBuffer((HGLOBAL)Control[1], (BYTE *)lpvBits, Control[0]) != Control[0])
    {
	DeleteBuffer((HGLOBAL)Control[1]);
	return 0;
    }

    DeleteBuffer((HGLOBAL)Control[1]);
    return (LONG)Control[0];
}


int WINAPI GetDIBits(HDC hDC, HBITMAP hBitmap, UINT start, UINT lines, 
	void FAR *lpvBits, BITMAPINFO FAR *lpbi, UINT fuColorUse)
{
    word Control[5];

    Control[0] = (word)hDC;
    Control[1] = (word)hBitmap;
    Control[2] = (word)start;
    Control[3] = (word)lines;
    Control[4] = (word)fuColorUse;

    if (!send_message(IO_GetDIBits, 5, sizeof(BITMAPINFO), &Control[0], (BYTE *)lpbi))
	return 0;

    /* now get the data buffer */
    if (lpvBits != (void FAR *)NULL)
	if (ReceiveBuffer((HGLOBAL)Control[1], (BYTE *)lpvBits, Control[2]) != Control[2])
	{
	    DeleteBuffer((HGLOBAL)Control[1]);
	    return 0;
	}

    DeleteBuffer((HGLOBAL)Control[1]);
    return (int)Control[0];
}


LONG WINAPI SetBitmapBits(HBITMAP hBitmap, DWORD cbBuffer, const void FAR *lpvBits)
{
    HGLOBAL hMem;
    LONG    ret;
    if ((lpvBits == NULL) || (cbBuffer == 0))
	return 0;
    hMem = SendBuffer((BYTE *)lpvBits, cbBuffer);
    if (hMem == NULL)
	return 0;
    ret = (LONG)send_3_parameters(IO_SetBitmapBits, hBitmap, cbBuffer, hMem);
    DeleteBuffer(hMem);
    return ret;
}

int WINAPI SetDIBits(HDC hDC, HBITMAP hBitmap, UINT start, UINT count,
		     const void *lpvBits, BITMAPINFO *lpbi, UINT ColorUse)
{
    word Control[6];
    int  bitmap_size, info_size, colors_used;
    HGLOBAL hMem;

    if ((lpvBits == NULL) || (lpbi == NULL))
	return 0;
    
    Control[0] = hDC;
    Control[1] = hBitmap;
    Control[2] = start;
    Control[3] = count;
    Control[4] = ColorUse;

    bitmap_size = (int)((lpbi->bmiHeader.biWidth*lpbi->bmiHeader.biBitCount + 31)/32 * 4 * count);
    info_size = sizeof(BITMAPINFO);
    if (lpbi->bmiHeader.biClrUsed == 0)
	colors_used = 1 << lpbi->bmiHeader.biBitCount;
    else
	colors_used = (int)lpbi->bmiHeader.biClrUsed;
    info_size += (colors_used - 1)*sizeof(RGBQUAD);

    Control[5] = hMem = SendBuffer((BYTE *)lpvBits, bitmap_size);
    if (hMem == NULL)
	return 0;

    if (!send_message(IO_SetDIBits, 6, info_size, &Control[0], (BYTE *)lpbi))
    {
	DeleteBuffer(hMem);
	return 0;
    }
    DeleteBuffer(hMem);
    return (int)Control[0];
}

int WINAPI SetDIBitsToDevice(HDC hDC, int XDest, int YDest, int cx, int cy,
			     int XSrc, int YSrc, UINT uStart, UINT uNo,
			     void *lpvBits, BITMAPINFO *lpbi, UINT ColorUse)
{
    word Control[11];
    int  bitmap_size, info_size, colors_used;
    HGLOBAL hMem;

    if ((lpvBits == NULL) || (lpbi == NULL))
	return 0;

    Control[0] = hDC;
    Control[1] = XDest;
    Control[2] = YDest;
    Control[3] = cx;
    Control[4] = cy;
    Control[5] = XSrc;
    Control[6] = YSrc;
    Control[7] = uStart;
    Control[8] = uNo;
    Control[9] = ColorUse;
    
    bitmap_size = (int)((lpbi->bmiHeader.biWidth*lpbi->bmiHeader.biBitCount + 31)/32 * 4 * uNo);
    info_size = sizeof(BITMAPINFO);
    if (lpbi->bmiHeader.biClrUsed == 0)
	colors_used = 1 << lpbi->bmiHeader.biBitCount;
    else
	colors_used = (int)lpbi->bmiHeader.biClrUsed;
    info_size += (colors_used - 1)*sizeof(RGBQUAD);

    Control[10] = hMem = SendBuffer((BYTE *)lpvBits, bitmap_size);
    if (hMem == NULL)
	return 0;

    if (!send_message(IO_SetDIBitsToDevice, 11, info_size, &Control[0], (BYTE *)lpbi))
    {
	DeleteBuffer(hMem);
	return 0;
    }
    DeleteBuffer(hMem);
    return (int)Control[0];
}


HDC WINAPI CreateCompatibleDC(HDC hDC)
{
    return (HDC)send_1_parameter(IO_CreateCompatibleDC, hDC);
}


BOOL WINAPI AppendMenu(HMENU hMenu, UINT uFlags, UINT ID, LPCSTR lpText)
{
    word Control[3];
    Control[0] = (word)hMenu;
    Control[1] = (word)uFlags;
    Control[2] = (word)ID;
    if (!send_message(IO_AppendMenu, 3, strlen(lpText)+1, &Control[0], (BYTE *)lpText))
	return 0;

    return (UINT)Control[0];
}


BOOL WINAPI CheckMenuItem(HMENU hMenu, UINT idCheck, UINT uCheck)
{
    return (BOOL)send_3_parameters(IO_CheckMenuItem, hMenu, idCheck, uCheck);
}


HMENU WINAPI CreatePopupMenu(void)
{
    return (HMENU)send_1_parameter(IO_CreatePopupMenu, 0);
}


HMENU WINAPI CreateMenu(void)
{
    return (HMENU)send_1_parameter(IO_CreateMenu, 0);
}


BOOL WINAPI DeleteMenu(HMENU hMenu, UINT ID, UINT flags)
{
    return (BOOL)send_3_parameters(IO_DeleteMenu, hMenu, ID, flags);
}


BOOL WINAPI DestroyMenu(HMENU hMenu)
{
    return (BOOL)send_1_parameter(IO_DestroyMenu, hMenu);
}


void WINAPI DrawMenuBar(HWND hWnd)
{
   send_1_parameter(IO_DrawMenuBar, hWnd);
}


BOOL WINAPI EnableMenuItem(HMENU hMenu, UINT ID, UINT flags)
{
    return (BOOL)send_3_parameters(IO_EnableMenuItem, hMenu, ID, flags);
}


HMENU WINAPI GetMenu(HWND hWnd)
{
    return (HMENU)send_1_parameter(IO_GetMenu, hWnd);
}


int WINAPI GetMenuItemCount(HMENU hMenu)
{
    return (int)send_1_parameter(IO_GetMenuItemCount, hMenu);
}


UINT WINAPI GetMenuItemID(HMENU hMenu, int pos)
{
    return (UINT)send_2_parameters(IO_GetMenuItemID, hMenu, pos);
}


UINT WINAPI GetMenuState(HMENU hMenu, UINT ID, UINT flags)
{
    return (UINT)send_3_parameters(IO_GetMenuState, hMenu, ID, flags);
}


int WINAPI GetMenuString(HMENU hMenu, UINT ID, LPSTR str, int max, UINT flags)
{
    word Control[4];
    if (str == NULL)
	return 0;
    Control[0] = hMenu;
    Control[1] = ID;
    Control[2] = max;
    Control[3] = flags;
    if (!send_fn(IO_GetMenuString, 4, max, &Control[0], (BYTE *)str))
	return 0;
    return (int)Control[0];
}


HMENU WINAPI GetSubMenu(HMENU hMenu, int pos)
{
    return (HMENU)send_2_parameters(IO_GetSubMenu, hMenu, pos);
}


HMENU WINAPI GetSystemMenu(HWND hWnd, BOOL bRevert)
{
    return (HMENU)send_2_parameters(IO_GetSystemMenu, hWnd, bRevert);
}


BOOL WINAPI InsertMenu(HMENU hMenu, UINT ID, UINT flags, UINT NewID, LPCSTR lpString)
{
    word Control[4];
    Control[0] = hMenu;
    Control[1] = ID;
    Control[2] = flags;
    Control[3] = NewID;
    if (!send_message(IO_InsertMenu, 4, strlen(lpString)+1, &Control[0], (BYTE *)lpString))
	return FALSE;

    return (BOOL)Control[0];
}


BOOL WINAPI RemoveMenu(HMENU hMenu, UINT ID, UINT flags)
{
    return (BOOL)send_3_parameters(IO_RemoveMenu, hMenu, ID, flags);
}


BOOL WINAPI SetMenu(HWND hWnd, HMENU hMenu)
{
    return (BOOL)send_2_parameters(IO_SetMenu, hWnd, hMenu);
}


HWND WINAPI FindWindow(LPCSTR lpszClassName, LPCSTR lpszWindow)
{
    word Control = (word)strlen(lpszClassName)+1;
    BYTE *Data;
    int  length = strlen(lpszClassName) + strlen(lpszWindow) + 2;
    Data = (BYTE *)malloc(length);
    if (Data == (BYTE *)NULL)
	return NULL;
    if (lpszClassName == NULL)
    {
	Control = 1;
	Data[0] = 0;
    }
    else
	strcpy((char *)&Data[0], lpszClassName);
    if (lpszWindow == NULL)
	Data[Control] = 0;
    else
	strcpy((char *)&Data[Control], lpszWindow);
    if (!send_message(IO_FindWindow, 1, length, &Control, Data))
    {
	free(Data);
	return NULL;
    }
    free(Data);
    return (HWND)Control;
}


BOOL WINAPI RegisterIOMenu(UINT ID)
{
    return (BOOL)send_1_parameter(IO_RegisterIOMenu, ID);
}

BOOL WINAPI DeleteDC(HDC hDC)
{
    return (BOOL)send_1_parameter(IO_DeleteDC, hDC);
}

BOOL WINAPI GetUpdateRect(HWND hWnd, LPRECT lprc, BOOL bErase)
{
    word Control[2];
    if (lprc == NULL)
	return FALSE;
    Control[0] = (word)hWnd;
    Control[1] = (word)bErase;
    if (!send_fn(IO_GetUpdateRect, 2, sizeof(RECT), &Control[0], (BYTE *)lprc))
	return FALSE;
    return (BOOL)Control[0];
}

int WINAPI GetObject(HGDIOBJ hObj, int cbBuffer, void FAR *lpvBuffer)
{
    word Control[2];
    if (lpvBuffer == NULL)
	return 0;
    Control[0] = (word)hObj;
    Control[1] = (word)cbBuffer;
    if (!send_fn(IO_GetObject, 2, cbBuffer, &Control[0], (BYTE *)lpvBuffer))
	return 0;
    return (int)Control[0];
}

/*
 * BLV - additions to support scrollbars
 */
int     WINAPI SetScrollPos(HWND hWnd, int nBar, int nPos, BOOL bRedraw)
{
	return((int) send_4_parameters(IO_SetScrollPos, hWnd, nBar, nPos, bRedraw));
}

int     WINAPI GetScrollPos(HWND hWnd, int nBar)
{
	return((int) send_2_parameters(IO_GetScrollPos, hWnd, nBar));
}

void    WINAPI SetScrollRange(HWND hWnd, int nBar, int nMinPos, int nMaxPos,
		BOOL bRedraw)
{
	(void) send_5_parameters(IO_SetScrollRange, hWnd, nBar, nMinPos, nMaxPos, bRedraw);
}

void    WINAPI GetScrollRange(HWND hWnd, int nBar, int FAR* lpMinPos,
		 int FAR* lpMaxPos)
{
	word	control[2];

	control[0]	= hWnd;
	control[1]	= nBar;

	if (send_message(IO_GetScrollRange, 2, 0, control, NULL))
	{
		*lpMinPos	= (int) control[0];
		*lpMaxPos	= (int) control[1];
	}
}

void    WINAPI ShowScrollBar(HWND hWnd, int nBar, BOOL bShow)
{
	(void) send_3_parameters(IO_ShowScrollBar, hWnd, nBar, bShow);
}

BOOL    WINAPI EnableScrollBar(HWND hWnd, int wSBFlags, UINT wArrowFlags)
{
	return((BOOL) send_3_parameters(IO_EnableScrollBar, hWnd, wSBFlags, wArrowFlags));
}

