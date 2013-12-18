/*
 * Window management definitions from <X/Xlib.h>
 * Copyright (c) 1985 Massachusetts Institute of Technology
 */

#ifndef Xwindow_h
#define Xwindow_h

#include <InterViews/X10/Xdefs.h>

/* 
 * Data returned by XQueryWindow.
 */
typedef struct _WindowInfo {
	short width, height;	/* Width and height. */
	short x, y;		/* X and y coordinates. */
	short bdrwidth;		/* Border width. */
	short mapped;		/* IsUnmapped, IsMapped or IsInvisible.*/
	short type;		/* IsTransparent, IsOpaque or IsIcon. */
	XWindow assoc_wind;	/* Associated icon or opaque Window. */
} XWindowInfo;

/*
 * Data structures used by XCreateWindows XCreateTransparencies and
 * XCreateWindowBatch.
 */
typedef struct _OpaqueFrame {
	XWindow self;		/* window id of the window, filled in later */
	short x, y;		/* where to create the window */
	short width, height;	/* width and height */
	short bdrwidth;		/* border width */
	XPixmap border;		/* border pixmap */
	XPixmap background;	/* background */
} XOpaqueFrame;

typedef struct _TransparentFrame {
	XWindow self;		/* window id of the window, filled in later */
	short x, y;		/* where to create the window */
	short width, height;	/* width and height */
} XTransparentFrame;

typedef struct _BatchFrame {
	short type;		/* One of (IsOpaque, IsTransparent). */
	XWindow parent;		/* Window if of the window's parent. */
	XWindow self;		/* Window id of the window, filled in later. */
	short x, y;		/* Where to create the window. */
	short width, height;	/* Window width and height. */
	short bdrwidth;		/* Window border width. */
	XPixmap border;		/* Window border pixmap */
	XPixmap background;	/* Window background pixmap. */
} XBatchFrame;

XWindow XCreateWindow(
    XWindow, int x, int y, int width, int height, int borderwidth,
    XPixmap border, XPixmap bg
);
XWindow XCreateTransparency(XWindow, int x, int y, int width, int height);
void XDestroyWindow(XWindow);
void XDestroySubwindows(XWindow);
int XCreateWindows(XWindow, XOpaqueFrame defs[], int);
int XCreateTransparencies(XWindow, XTransparentFrame defs[], int);
int XCreateWindowBatch(XWindow, XBatchFrame defs, int);
XWindow XCreate(
    const char* prompt, const char* program, const char* geometry,
    const char* deflt, XOpaqueFrame* frame, int minwidth, int minheight
);
XWindow XCreateTerm(
    const char* prompt, const char* program, const char* geometry,
    const char* deflt, XOpaqueFrame* frame, int minwidth, int minheight,
    int xadder, int yadder, int* cwidth, int* cheight, struct _FontInfo* f,
    int fwidth, int fheight
);

void XMapWindow(XWindow);
void XMapSubwindows(XWindow);
void XUnmapWindow(XWindow);
void XUnmapTransparent(XWindow);
void XUnmapSubwindows(XWindow);
void XMoveWindow(XWindow, int x, int y);
void XChangeWindow(XWindow, int width, int height);
void XConfigureWindow(XWindow, int x, int y, int width, int height);
void XRaiseWindow(XWindow);
void XLowerWindow(XWindow);
void XCircWindowUp(XWindow);
void XCircWindowDown(XWindow);

XStatus XQueryWindow(XWindow, XWindowInfo*);
XStatus XQueryTree(XWindow, XWindow*, int*, XWindow**);

void XChangeBackground(XWindow, XPixmap);
void XChangeBorder(XWindow, XPixmap);
void XTileAbsolute(XWindow);
void XTileRelative(XWindow);
void XClipDrawThrough(XWindow);
void XClipClipped(XWindow);

void XStoreName(XWindow, const char*);
XStatus XFetchName(XWindow, const char**);

void XSetResizeHint(XWindow, int w0, int h0, int winc, int hinc);
void XGetResizeHint(XWindow, int* w0, int* h0, int* winc, int* hinc);
void XSetIconWindow(XWindow, XWindow);
void XClearIconWindow(XWindow);

#endif
