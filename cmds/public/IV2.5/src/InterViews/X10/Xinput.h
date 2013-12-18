/*
 * Xlib definitions relevant to input from <X/Xlib.h>.
 * Copyright (c) 1985 Massachusetts Institute of Technology
 */

#ifndef Xinput_h
#define Xinput_h

#include <InterViews/X10/Xdefs.h>

/* Input Event Codes */

#define NoEvent		 0x0000
#define KeyPressed	 0x0001		/* keyboard key pressed */
#define KeyReleased	 0x0002		/* keyboard key released */
#define ButtonPressed	 0x0004		/* mouse button pressed */
#define ButtonReleased	 0x0008		/* mouse button released */
#define EnterWindow	 0x0010		/* mouse entering window */
#define LeaveWindow	 0x0020		/* mouse leaving window */
#define MouseMoved	 0x0040		/* mouse moves within window */
#define ExposeWindow	 0x0080		/* full window changed and/or exposed */
#define ExposeRegion	 0x0100		/* region of window exposed */
#define ExposeCopy	 0x0200		/* region exposed by X_CopyArea */
#define RightDownMotion	 0x0400		/* mouse moves with right button down */
#define MiddleDownMotion 0x0800		/* mouse moves with middle button down */
#define LeftDownMotion	 0x1000		/* mouse moves with left button down */
#define UnmapWindow	 0x2000		/* window is unmapped */
#define FocusChange	 0x4000		/* keyboard focus changed */

/* Event detail bits */

#define ControlMask	0x4000		/* Control key */
#define MetaMask	0x2000		/* Meta (Symbol) key */
#define ShiftMask	0x1000		/* Shift key */
#define ShiftLockMask	0x0800		/* ShiftLock key */
#define LeftMask	0x0400		/* Left button */
#define MiddleMask	0x0200		/* Middle button */
#define RightMask	0x0100		/* Right button */
#define ValueMask	0x00ff		/* Key/button code */

#define KeyState(x) (((x) & (ControlMask|MetaMask|ShiftMask)) >> 12)
#define FullKeyState(x) (((x) & (ControlMask|MetaMask|ShiftMask|ShiftLockMask)) >> 11)
#define ButtonState(x) (((x) & (LeftMask|MiddleMask|RightMask)) >> 8)

/* Button event detail codes */

#define RightButton	0
#define MiddleButton	1
#define LeftButton	2

/* Enter/Leave event detail codes */

#define IntoOrFromSubwindow	1
#define VirtualCrossing		2

#define XQLength() (_XlibCurrentDisplay->qlen)

/* Definition of a generic event.  It must be cast to a specific event
 * type before one can read event-specific data */

typedef struct _XEvent {
    	unsigned long type;   /* of event (KeyPressed, ExposeWindow, etc.) */
	XWindow window;	      /* which selected this event */
	long pad_l1, pad_l2;  /* event-specific data */
	XWindow subwindow;    /* child window (if any) where event happened */
	long pad_l4; 	      /* event-specific data */
} XEvent;


/*
 * _QEvent datatype for use in input queueing.
 */
typedef struct _qevent {
	struct _qevent *next;
	XEvent event;
} _QEvent;


/*
 * Definitions of specific events
 * In all of the following, fields whose names begin with "pad" contain
 * no meaningful value.
 */

struct _XKeyOrButtonEvent {
	unsigned long type;   /* of event (KeyPressed, ButtonReleased, etc. */
	XWindow window;       /* which selected this event */
	unsigned short time;  /* in 10 millisecond ticks */
	short detail;	      /* event-dependent data (key state, etc.) */
	short x;    	      /* mouse x coordinate within event window */
	short y;    	      /* mouse y coordinate within event window */
	XWindow subwindow;    /* child window (if any) mouse was in */
	XLocator location;    /* absolute coordinates of mouse */
};

typedef struct _XKeyOrButtonEvent XKeyOrButtonEvent;

typedef struct _XKeyOrButtonEvent XKeyEvent;
typedef struct _XKeyOrButtonEvent XKeyPressedEvent;
typedef struct _XKeyOrButtonEvent XKeyReleasedEvent;

typedef struct _XKeyOrButtonEvent XButtonEvent;
typedef struct _XKeyOrButtonEvent XButtonPressedEvent;
typedef struct _XKeyOrButtonEvent XButtonReleasedEvent;

struct _XMouseOrCrossingEvent {
	unsigned long type;	/* EnterWindow, LeaveWindow, or MouseMoved */
	XWindow window;		/* which selected this event */
	short pad_s2;
	short detail;		/* event-dependent data (key state, etc. ) */
	short x;		/* mouse x coordinate within event window */
	short y;		/* mouse y coordinate within event window */
	XWindow subwindow;	/* child window (if any) mouse was in */
	XLocator location;	/* absolute coordinates of mouse */
};

typedef struct _XMouseOrCrossingEvent XMouseOrCrossingEvent;

typedef struct _XMouseOrCrossingEvent XMouseEvent;
typedef struct _XMouseOrCrossingEvent XMouseMovedEvent;

typedef struct _XMouseOrCrossingEvent XCrossingEvent;
typedef struct _XMouseOrCrossingEvent XEnterWindowEvent;
typedef struct _XMouseOrCrossingEvent XLeaveWindowEvent;

struct _XExposeEvent {
	unsigned long type;	/* ExposeWindow or ExposeRegion */
	XWindow window;		/* that selected this event */
	short pad_s2; 	      
	short detail;		/* 0 or ExposeCopy */
	short width;		/* width of exposed area */
	short height;		/* height of exposed area */
	XWindow subwindow;	/* child window (if any) actually exposed */
	short y;		/* top of exposed area (0 for ExposeWindow) */
	short x;		/* left edge of exposed area */
};

typedef struct _XExposeEvent XExposeEvent;
typedef struct _XExposeEvent XExposeWindowEvent;
typedef struct _XExposeEvent XExposeRegionEvent;

typedef struct _XExposeCopyEvent {
    	unsigned long type;	/* ExposeCopy */
	XWindow window;		/* that selected this event */
	long pad_l1;
	long pad_l2;	      
	XWindow subwindow;	/* child window (if any) actually exposed */
	long pad_l4;	      
} XExposeCopyEvent;
	
typedef struct _XUnmapEvent {
	unsigned long type;	/* UnmapWindow */
	XWindow window;		/* that selected this event */
	long pad_l1;
	long pad_l2;	      
	XWindow subwindow;	/* child window (if any) actually unmapped */
	long pad_l4;	      
} XUnmapEvent;

typedef struct _XFocusChangeEvent {
	unsigned long type;	/* FocusChange */
	XWindow window;		/* that selected this event */
	short pad_s2;
	short detail;		/* EnterWindow or LeaveWindow */
	long pad_l2;	      
	XWindow subwindow;	/* child (if any) of actual focus change */
	long pad_l4;	      
} XFocusChangeEvent;

typedef struct _XErrorEvent {
	long pad;
	long serial;		/* serial number of failed request */
	char error_code;    	/* error code of failed request */
	char request_code;	/* request code of failed request */
	char func;  	        /* function field of failed request */
	char pad_b7;
	XWindow window;	    	/* Window of failed request */
	long pad_l3;
	long pad_l4;
} XErrorEvent;

typedef short KeyMapEntry [8];

/* define values for keyboard map table */
/* these values will vanish in the next version; DO NOT USE THEM! */
#define SHFT	(short) -2
#define CNTL	(short) -3
#define LOCK	(short) -4
#define SYMBOL	(short) -5
#define KEYPAD	(short) -6
#define CURSOR	(short) -7
#define PFX	(short) -8
#define FUNC1	(short) -9
#define FUNC2	(short) -10
#define FUNC3	(short) -11
#define FUNC4	(short) -12
#define FUNC5	(short) -13
#define FUNC6	(short) -14
#define FUNC7	(short) -15
#define FUNC8	(short) -16
#define FUNC9	(short) -17
#define FUNC10	(short) -18
#define FUNC11	(short) -19
#define FUNC12	(short) -20
#define FUNC13	(short) -21
#define FUNC14	(short) -22
#define FUNC15	(short) -23
#define FUNC16	(short) -24
#define FUNC17	(short) -25
#define FUNC18	(short) -26
#define FUNC19	(short) -27
#define FUNC20	(short) -28
#define E1	(short) -29
#define E2	(short) -30
#define E3	(short) -31
#define E4	(short) -32
#define E5	(short) -33
#define E6	(short) -34

XStatus XQueryMouse(XWindow, int* x, int* y, XWindow* sub);
XStatus XQueryMouseButtons(XWindow, int* x, int* y, XWindow* sub, short* state);
XStatus XUpdateMouse(XWindow, int* x, int* y, XWindow* sub);
void XWarpMouse(XWindow, int x, int y);
void XCondWarpMouse(
    XWindow, XWindow, int dx, int dy, int sx, int sy, int swidth, int sheight
);
XStatus XInterpretLocator(XWindow, int* x, int* y, XWindow* sub, XLocator loc);

void XSelectInput(XWindow, int);
int XPending();
void XNextEvent(XEvent*);
void XPutBackEvent(XEvent*);
void XPeekEvent(XEvent*);
void XWindowEvent(XWindow, int, XEvent*);
void XMaskEvent(int, XEvent*);

XStatus XGrabMouse(XWindow, XCursor, int);
void XUngrabMouse();
XStatus XGrabButton(XWindow, XCursor, int buttonmask, int eventmask);
void XUngrabButton(int);
void XGrabServer();
void XUngrabServer();
void XFocusKeyboard(XWindow);
char* XLookupMapping(XKeyPressedEvent*, int*);
void XRebindCode(int keycode, int shift, char*, int);

#endif
