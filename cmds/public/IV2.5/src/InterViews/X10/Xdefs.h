/*
 * General Xlib definitions from <X/Xlib.h>.
 * Copyright (c) 1985 Massachusetts Institute of Technology
 */

#ifndef Xdefs_h
#define Xdefs_h

typedef void* XWindow;
typedef void* XFont;
typedef void* XBitmap;
typedef void* XPixmap;
typedef void* XCursor;
typedef long XLocator;

#define XStatus int
#define Xdpyno() (_XlibCurrentDisplay->fd)
#define XRootWindow (_XlibCurrentDisplay->root)
#define XBlackPixmap (_XlibCurrentDisplay->black)
#define XWhitePixmap (_XlibCurrentDisplay->white)
#define XDisplayType() (_XlibCurrentDisplay->dtype)
#define XDisplayPlanes() (_XlibCurrentDisplay->dplanes)
#define XDisplayCells() (_XlibCurrentDisplay->dcells)
#define XProtocolVersion() (_XlibCurrentDisplay->vnumber)
#define XDisplayName() (_XlibCurrentDisplay->displayname)

/*
 * Display datatype maintaining display specific data.
 */
typedef struct _display {
	int fd;			/* Network socket. */
	XWindow root;		/* Root window id. */
	int vnumber;		/* X protocol version number. */
	int dtype;		/* X server display device type. */
	int dplanes;		/* Number of display bit planes. */
	int dcells;		/* Number of display color map cells. */
	struct _qevent* head;	/* Head of input event queue. */
	struct _qevent* tail;	/* Tail of input event queue. */
	int qlen;		/* Length of input event queue */
	int request;		/* Id of last request. */
	char* lastdraw;		/* Last draw request. */
	char* buffer;		/* Output buffer starting address. */
	char* bufptr;		/* Output buffer index pointer. */
	char* bufmax;		/* Output buffer maximum+1 address. */
	int squish;		/* Squish MouseMoved events? */
	XPixmap black, white;   /* Constant tile pixmaps */
	char* displayname;	/* "host:display" string on this connect */
	int width, height;	/* width and height of display */
} XDisplay;

extern XDisplay* _XlibCurrentDisplay;

XDisplay* XOpenDisplay(const char*);
void XSetDisplay(XDisplay*);
void XCloseDisplay(XDisplay*);
void XFlush();
void XSync(int discard);

int DisplayHeight();
int DisplayWidth();

void XMouseControl(int acc, int thresh);
void XFeepControl(int);
void XFeep(int);
void XKeyClickControl(int);
void XAutoRepeatOn();
void XAutoRepeatOff();
void XLockUpDown();
void XLockToggle();
void XScreenSaver(int time, int pattime, int video);

#endif
