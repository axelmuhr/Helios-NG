/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    name  of Evans & Sutherland  not be used in advertising or publi-    **/
/**    city pertaining to distribution  of the software without  specif-    **/
/**    ic, written prior permission.                                        **/
/**                                                                         **/
/**    EVANS  & SUTHERLAND  DISCLAIMS  ALL  WARRANTIES  WITH  REGARD  TO    **/
/**    THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILI-    **/
/**    TY AND FITNESS, IN NO EVENT SHALL EVANS &  SUTHERLAND  BE  LIABLE    **/
/**    FOR  ANY  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY  DAM-    **/
/**    AGES  WHATSOEVER RESULTING FROM  LOSS OF USE,  DATA  OR  PROFITS,    **/
/**    WHETHER   IN  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS    **/
/**    ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE  OR PER-    **/
/**    FORMANCE OF THIS SOFTWARE.                                           **/
/*****************************************************************************/

/***********************************************************************
 *
 * $Header: twm.h,v 1.48 88/10/14 07:05:59 toml Exp $
 *
 * twm include file
 *
 * 28-Oct-87 Thomas E. LaStrange	File created
 *
 ***********************************************************************/

#ifndef _TWM_
#define _TWM_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

/* Define this if you want the Focus button on the titlebar */
/* #define FOCUS */

#define BW 2			/* border width */

/* directory to look for bitmaps if the file is not found in the current
 * directory 
 */
#define BITMAPS "/usr/include/X11/bitmaps"

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#define MAX_BUTTONS	5	/* max mouse buttons supported */

/* contexts for button presses */
#define C_NO_CONTEXT	-1
#define C_WINDOW	0
#define C_TITLE		1
#define C_ICON		2
#define C_ROOT		3
#define C_FRAME		4
#define C_NAME		5
#define C_ICONMGR	6
#define NUM_CONTEXTS	7

/* modifiers for button presses */
#define MOD_SIZE	((ShiftMask | ControlMask | Mod1Mask) + 1)

#define TITLE_BAR_SPACE         1	/* 2 pixel space bordering chars */
#define TITLE_BAR_FONT_HEIGHT   15	/* max of 15 pixel high chars */
#define TITLE_BAR_HEIGHT        (TITLE_BAR_FONT_HEIGHT+(2*TITLE_BAR_SPACE))

/* defines for zooming/unzooming */
#define ZOOM_NONE 0
#define ZOOM_VERT 1
#define ZOOM_FULL 2

typedef struct WList
{
    struct WList *next;
    struct WList *prev;
    struct TwmWindow *twm;
    Window w;
    Window icon;
} WList;

typedef struct MyFont
{
    char *name;			/* name of the font */
    XFontStruct *font;		/* font structure */
    int height;			/* height of the font */
    int y;			/* Y coordinate to draw characters */
} MyFont;

typedef struct ColorPair
{
    int fore;
    int back;
} ColorPair;

/* for each window that is on the display, one of these structures
 * is allocated and linked into a list 
 */
typedef struct TwmWindow
{
    struct TwmWindow *next;	/* next twm window */
    struct TwmWindow *prev;	/* previous twm window */
    Window w;			/* the child window */
    Window frame;		/* the frame window */
    Window title_w;		/* the title bar window */
    Window iconify_w;		/* the iconify button */
    Pixmap iconify_pm;
    Window resize_w;		/* the resize button */
    Pixmap resize_pm;
#ifndef NOFOCUS
    Window focus_w;		/* the focus window */
    Pixmap focus_pm;
#endif
    Window hilite_w;		/* the hilite window */
    Pixmap hilite_pm;
    Window icon_w;		/* the icon window */
    Window icon_bm_w;		/* the icon bitmap window */
    GC title_gc;		/* gc for the title bar */
    WList *list;		/* pointer to window list structure */
    int frame_x;		/* x position of frame */
    int frame_y;		/* y position of frame */
    int frame_width;		/* width of frame */
    int frame_height;		/* height of frame */
    int icon_x;			/* icon text x coordinate */
    int icon_y;			/* icon text y coordiante */
    int icon_w_width;		/* width of the icon window */
    int icon_width;		/* width of the icon bitmap */
    int icon_height;		/* height of the icon bitmap */
    int title_height;		/* height of the title bar */
    char *full_name;		/* full name of the window */
    char *name;			/* name of the window */
    char *icon_name;		/* name of the icon */
    int name_width;		/* width of name text */
    XWindowAttributes attr;	/* the child window attributes */
    XSizeHints hints;		/* normal hints */
    XWMHints *wmhints;		/* WM hints */
    int group;			/* group ID */
    int frame_vis;		/* frame visibility */
    int icon_vis;		/* icon visibility */
    XClassHint class;
    short xterm;		/* boolean indicating xterm */
    short iconified;		/* has the window ever been iconified? */
    short icon;			/* is the window an icon now ? */
    short icon_on;		/* is the icon visible */
    short mapped;		/* is the window mapped ? */
    short auto_raise;		/* should we auto-raise this window ? */
    short forced;		/* has had an icon forced upon it */
    short highlight;		/* should highlight this window */
    short iconify_by_unmapping;	/* unmap window to iconify it */
    int save_frame_x;		/* x position of frame */
    int save_frame_y;		/* y position of frame */
    int save_frame_width;	/* width of frame */
    int save_frame_height;	/* height of frame */
    short zoomed;		/* is the window zoomed? */
} TwmWindow;

extern TwmWindow TwmRoot;

extern Display *dpy;
extern int screen;
extern int d_depth;
extern Visual *d_visual;

extern Window Root;
extern Window VersionWindow;
extern Window SizeWindow;
extern Window ResizeWindow;
extern Window InitialWindow;
extern Window IconManager;
extern TwmWindow *IconManagerPtr;
extern Colormap CMap;

extern MyFont TitleBarFont;
extern MyFont MenuFont;
extern MyFont IconFont;
extern MyFont SizeFont;
extern MyFont VersionFont;
extern MyFont InitialFont;
extern MyFont IconManagerFont;
extern MyFont DefaultFont;

extern char *IconManagerGeometry;
extern char *IconDirectory;

extern ColorPair BorderTileC;
extern ColorPair TitleC;
extern ColorPair MenuC;
extern ColorPair MenuTitleC;
extern ColorPair IconC;
extern ColorPair IconManagerC;
extern ColorPair DefaultC;
extern int BorderColor;
extern int MenuShadowColor;
extern int IconBorderColor;

extern Cursor ArrowCursor;
extern Cursor ButtonCursor;
extern Cursor MoveCursor;
extern Cursor ClockCursor;
extern Cursor LeftArrowCursor;
extern Cursor UpperLeftCursor;
extern Cursor DotCursor;
extern Cursor SkullCursor;

extern GC MenuNormalGC;
extern GC MenuReverseGC;
extern GC MenuXorGC;
extern GC MenuTitleGC;
extern GC IconNormalGC;
extern GC VersionNormalGC;
extern GC SizeNormalGC;
extern GC InitialNormalGC;
extern GC DrawGC;
extern GC BorderGC;
extern GC IconManagerGC;

extern XClassHint NoClass;

extern XContext TwmContext;
extern XContext MenuContext;
extern XContext IconManagerContext;

extern int BorderWidth;
extern unsigned long Foreground;
extern unsigned long Background;
extern unsigned long Black;
extern unsigned long White;
extern Pixmap GrayTile;

extern char Version[100];
extern Pixmap UnknownPm;
extern int UnknownWidth;
extern int UnknownHeight;
extern int FirstTime;
extern int ReverseVideo;
extern int FocusRoot;
extern TwmWindow *Focus;
extern int WarpCursor;
extern int DontMoveOff;
extern int DoZoom;
extern int ForceIcon;
extern int NoRaiseMove;
extern int NoRaiseResize;
extern int NoRaiseDeicon;
extern int Monochrome;
extern int TitleFocus;
extern int NoTitlebar;
extern int DecorateTransients;
extern int IconifyByUnmapping;
extern int ShowIconManager;
extern int BackingStore;
extern int SaveUnder;
extern int RandomPlacement;
extern char *Home;
extern int HomeLen;
extern int ParseError;
extern int Highlight;
extern int MyDisplayWidth;
extern int MyDisplayHeight;

extern int TitleBarX;
extern int HandlingEvents;

extern Window JunkRoot;
extern Window JunkChild;
extern int JunkX;
extern int JunkY;
extern int JunkWidth;
extern int JunkHeight;
extern int JunkDepth;
extern int JunkBW;
extern int JunkMask;


extern void Done();
extern void Error();
extern void Other();

#endif _TWM_
