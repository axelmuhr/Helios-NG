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
 * $Header: twm.c,v 1.2.1.2 89/03/20 19:28:46 interran Exp $
 *
 * twm - "Tom's Window Manager"
 *
 * 27-Oct-87 Thomas E. LaStrange	File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Header: twm.c,v 1.2.1.2 89/03/20 19:28:46 interran Exp $";
#endif

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include "twm.h"
#include "add_window.h"
#include "gc.h"
#include "parse.h"
#include "version.h"
#include "menus.h"
#include "events.h"
#include "util.h"
#include "gram.h"

#include "twm.bm"
#include "gray.bm"

TwmWindow TwmRoot;		/* the head of the twm window list */

Display *dpy;			/* which display are we talking to */
int screen;			/* the default screen */
int d_depth;			/* copy of DefaultDepth(dpy, screen) */
Visual *d_visual;		/* copy of DefaultVisual(dpy, screen) */

Window Root;			/* the root window */
Window VersionWindow;		/* the twm version window */
Window SizeWindow;		/* the resize dimensions window */
Window ResizeWindow;		/* the window we are resizing */
Window InitialWindow;		/* the window name we are creating */
Window IconManager;		/* a list of the windows */
TwmWindow *IconManagerPtr= NULL;	/* The twm structure for IconManager */
Colormap CMap;			/* default color map */

MyFont TitleBarFont;		/* title bar font structure */
MyFont MenuFont;		/* menu font structure */
MyFont IconFont;		/* icon font structure */
MyFont SizeFont;		/* resize font structure */
MyFont VersionFont;		/* version font structure */
MyFont InitialFont;		/* window creation font structure */
MyFont IconManagerFont;		/* window list font structure */
MyFont DefaultFont;

char *IconManagerGeometry = "=150x5+0+0";	/* window list geometry */
char *IconDirectory = NULL;	/* icon directory to search */

ColorPair BorderTileC;		/* border tile colors */
ColorPair TitleC;		/* titlebar colors */
ColorPair MenuC;		/* menu colors */
ColorPair MenuTitleC;		/* menu title colors */
ColorPair IconC;		/* icon colors */
ColorPair IconManagerC;		/* icon manager colors */
ColorPair DefaultC;		/* default colors */
int BorderColor;		/* color of window borders */
int MenuShadowColor;		/* menu shadow color */
int IconBorderColor;		/* icon border color */

Cursor ArrowCursor;		/* title bar cursor */
Cursor ButtonCursor;		/* title bar button cursor */
Cursor MoveCursor;		/* move and resize cursor */
Cursor ClockCursor;		/* wait a while cursor */
Cursor LeftArrowCursor;		/* menu cursor */
Cursor UpperLeftCursor;		/* upper Left corner cursor */
Cursor DotCursor;		/* dot cursor for f.move, etc. from menus */
Cursor SkullCursor;		/* skull and cross bones, f.destroy */

GC MenuNormalGC;		/* normal GC for menus */
GC MenuReverseGC;		/* reverse video GC for menus */
GC MenuXorGC;			/* XOR GC for menus */
GC MenuTitleGC;			/* normal GC for menu titles */
GC IconNormalGC;		/* GC for icons */
GC VersionNormalGC;		/* GC for the version window */
GC SizeNormalGC;		/* GC for the resize window */
GC InitialNormalGC;		/* GC for the initial creation window */
GC DrawGC;			/* GC to draw lines for move and resize */
GC BorderGC;			/* GC to create the "gray" pixmap */
GC IconManagerGC;		/* GC for the window list */

XClassHint NoClass;		/* for applications with no class */

XContext TwmContext;		/* context for twm windows */
XContext MenuContext;		/* context for all menu windows */
XContext IconManagerContext;	/* context for all window list windows */

int BorderWidth = BW;		/* border width of twm windows */
unsigned long Black;
unsigned long White;
Pixmap GrayTile;

char Version[100];		/* place to build the version string */
Pixmap UnknownPm = NULL;	/* the unknown icon pixmap */
int UnknownWidth = 0;		/* width of the unknown icon */
int UnknownHeight = 0;		/* height of the unknown icon */
int FirstTime = TRUE;		/* first time we've read .twmrc */
int ReverseVideo = FALSE;	/* flag to do reverse video */
int FocusRoot = TRUE;		/* is the input focus on the root ? */
TwmWindow *Focus = NULL;	/* the twm window that has focus */
int WarpCursor = FALSE;		/* warp cursor on de-iconify ? */
int ForceIcon = FALSE;		/* force the icon to the user specified */
int NoRaiseMove = FALSE;	/* don't raise window following move */
int NoRaiseResize = FALSE;	/* don't raise window following resize */
int NoRaiseDeicon = FALSE;	/* don't raise window on deiconify */
int DontMoveOff = FALSE;	/* don't allow windows to be moved off */
int DoZoom = FALSE;		/* zoom in and out of icons */
int Monochrome = COLOR;		/* is the display monochrome ? */
int TitleFocus = TRUE;		/* focus on window in title bar ? */
int NoTitlebar = FALSE;		/* put title bars on windows */
int DecorateTransients = FALSE;	/* put title bars on transients */
int IconifyByUnmapping = FALSE;	/* simply unmap windows when iconifying */
int ShowIconManager = FALSE;	/* display the window list */
int BackingStore = TRUE;	/* use backing store for menus */
int SaveUnder = TRUE;		/* use save under's for menus */
int RandomPlacement = FALSE;	/* randomly place windows that no give hints */
char *Home;			/* the HOME environment variable */
int HomeLen;			/* length of Home */
int ParseError;			/* error parsing the .twmrc file */
int Highlight = TRUE;		/* we should highlight the window borders */
int MyDisplayWidth;		/* my copy of DisplayWidth(dpy, screen) */
int MyDisplayHeight;		/* my copy of DisplayHeight(dpy, screen) */

int TitleBarX = TITLE_BAR_HEIGHT + 4;	/* x coordinate ditto */
int HandlingEvents = FALSE;	/* are we handling events yet? */

Window JunkRoot;		/* junk window */
Window JunkChild;		/* junk window */
int JunkX;			/* junk variable */
int JunkY;			/* junk variable */
int JunkWidth;			/* junk variable */
int JunkHeight;			/* junk variable */
int JunkDepth;			/* junk variable */
int JunkBW;			/* junk variable */
int JunkMask;			/* junk variable */

/***********************************************************************
 *
 *  Procedure:
 *	main - start of twm
 *
 ***********************************************************************
 */

main(argc, argv)
    int argc;
    char *argv[];
{
    Window w, root, parent, *children;
    int nchildren, i;
    int m, d, y;
    char *display_name;
    unsigned long valuemask;	/* mask for create windows */
    XSetWindowAttributes attributes;	/* attributes for create windows */
#ifdef sun
    void (*old_handler) ();
#else
    int (*old_handler) ();
#endif sun
    int mask;

    display_name = NULL;

    if (argc != 1 && argc != 3)
    {
	fprintf(stderr, "Usage: twm [-display display]\n");
	exit(1);
    }

    if (argc == 3)
    {
	if (strncmp(argv[1], "-d", 2) == 0)
	    display_name = argv[2];
	else
	{
	    fprintf(stderr, "Usage: twm [-display display]\n");
	    exit(1);
	}
    }

    old_handler = signal(SIGINT, SIG_IGN);
    if (old_handler != SIG_IGN)
	signal(SIGINT, Done);

    old_handler = signal(SIGHUP, SIG_IGN);
    if (old_handler != SIG_IGN)
	signal(SIGHUP, Done);

    signal(SIGQUIT, Done);
    signal(SIGTERM, Done);

    Home = (char *)getenv("HOME");
    if (Home == NULL)
	Home = "./";

    HomeLen = strlen(Home);

    NoClass.res_name = NoName;
    NoClass.res_class = NoName;

    TwmRoot.next = NULL;
    TwmRoot.prev = NULL;
    TwmRoot.w = NULL;
    TwmRoot.title_w = NULL;
    TwmRoot.iconify_w = NULL;
    TwmRoot.resize_w = NULL;

    if ((dpy = XOpenDisplay(display_name)) == NULL)
    {
	fprintf(stderr, "twm: can't open the display\n");
	exit(1);
    }


    if (fcntl(ConnectionNumber(dpy), F_SETFD, 1) == -1)
    {
	fprintf(stderr, "twm: child cannot disinherit TCP fd\n");
	exit(1);
    }

    screen = DefaultScreen(dpy);
    d_depth = DefaultDepth(dpy, screen);
    d_visual = DefaultVisual(dpy, screen);
    Root = RootWindow(dpy, screen);
    CMap = DefaultColormap(dpy, screen);
    MyDisplayWidth = DisplayWidth(dpy, screen);
    MyDisplayHeight = DisplayHeight(dpy, screen);

    XSetErrorHandler(Other);
    XSelectInput(dpy, Root,
	SubstructureRedirectMask | KeyPressMask |
	ButtonPressMask | ButtonReleaseMask | ExposureMask);
    XSync(dpy, False);

    XSetErrorHandler(Error);
    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);

    TwmContext = XUniqueContext();
    MenuContext = XUniqueContext();
    IconManagerContext = XUniqueContext();

    /* define cursors */

    ArrowCursor = XCreateFontCursor(dpy, XC_top_left_arrow);
    MoveCursor = XCreateFontCursor(dpy, XC_fleur);
    LeftArrowCursor = XCreateFontCursor(dpy, XC_sb_left_arrow);
    ButtonCursor = XCreateFontCursor(dpy, XC_center_ptr);
    ClockCursor = XCreateFontCursor(dpy, XC_watch);
    UpperLeftCursor = XCreateFontCursor(dpy, XC_top_left_corner);
    DotCursor = XCreateFontCursor(dpy, XC_dot);
    SkullCursor = XCreateFontCursor(dpy, XC_pirate);

    /* setup default colors */

    Black = BlackPixel(dpy, screen);
    White = WhitePixel(dpy, screen);

    DefaultC.fore = Black;
    DefaultC.back = White;
    BorderColor = Black;
    BorderTileC.fore = Black;
    BorderTileC.back = White;
    TitleC.fore = Black;
    TitleC.back = White;
    MenuC.fore = Black;
    MenuC.back = White;
    MenuTitleC.fore = Black;
    MenuTitleC.back = White;
    MenuShadowColor = Black;
    IconC.fore = Black;
    IconC.back = White;
    IconBorderColor = Black;
    IconManagerC.fore = Black;
    IconManagerC.back = White;

    /* setup default fonts */

    TitleBarFont.font = NULL;
    TitleBarFont.name = "8x13";		GetFont(&TitleBarFont);
    MenuFont.font = NULL;
    MenuFont.name = "8x13";		GetFont(&MenuFont);
    IconFont.font = NULL;
    IconFont.name = "8x13";		GetFont(&IconFont);
    SizeFont.font = NULL;
    SizeFont.name = "fixed";		GetFont(&SizeFont);
    VersionFont.font = NULL;
    VersionFont.name = "8x13";		GetFont(&VersionFont);
    InitialFont.font = NULL;
    InitialFont.name = "9x15";		GetFont(&InitialFont);
    IconManagerFont.font = NULL;
    IconManagerFont.name = "8x13";	GetFont(&IconManagerFont);
    DefaultFont.font = NULL;
    DefaultFont.name = "fixed";		GetFont(&DefaultFont);

    if (DisplayCells(dpy, screen) < 3)
	Monochrome = MONOCHROME;

    ParseTwmrc(NULL);
    FirstTime = FALSE;
    CreateGCs();

    XGrabServer(dpy);
    XSync(dpy, False);

    JunkX = 0;
    JunkY = 0;
    IconManagerWidth = 150;
    IconManagerHeight = 5;

    mask = XParseGeometry(IconManagerGeometry, &JunkX, &JunkY,
	&IconManagerWidth, &IconManagerHeight);
    if (mask & XNegative)
	JunkX = MyDisplayWidth - IconManagerWidth - (2 * BorderWidth) + JunkX;
    if (mask & YNegative)
	JunkY = MyDisplayHeight - IconManagerHeight - (2 * BorderWidth) + JunkY;

    IconManager = XCreateSimpleWindow(dpy, Root,
	JunkX, JunkY, IconManagerWidth, IconManagerHeight, 0,
	Black, IconManagerC.back);
    XSetStandardProperties(dpy, IconManager, "TWM Icon Manager",
	"TWM Icon Manager", None, NULL, 0, NULL);

    XQueryTree(dpy, Root, &root, &parent, &children, &nchildren);
    GrayTile = MakePixmap(Root, BorderGC, gray_bits, 
     gray_width, gray_height);

    AddWindow(IconManager);
    for (i = 0; i < nchildren; i++)
    {
	if (MappedNotOverride(children[i]))
	{
	    AddWindow(children[i]);
	}
    }
    if (ShowIconManager)
    {
	XMapWindow(dpy, IconManagerPtr->w);
	XMapWindow(dpy, IconManagerPtr->frame);
    }

    InitialWindow = XCreateSimpleWindow(dpy, Root,
	0, 0, 5, InitialFont.height + 4, BW, DefaultC.fore, DefaultC.back);

    /* contruct the version string */
    sprintf(Version, "%s", &Revision[1]);
    Version[strlen(Version) - 1] = '\0';
    sscanf(&Date[7], "%d/%d/%d", &y, &m, &d);
    sprintf(Version, "%s  Date: %d/%d/%d %s", Version, m, d, y, &Date[16]);
    Version[strlen(Version) - 2] = '\0';

    VersionWindow = XCreateSimpleWindow(dpy, Root,
	0, 0,
	twm_width + XTextWidth(VersionFont.font, Version,
	    strlen(Version)) + 20, VersionFont.height + 4, BW,
	    DefaultC.fore, DefaultC.back);

    valuemask = CWBackPixmap;
    attributes.background_pixmap = MakePixmap(VersionWindow, VersionNormalGC,
	twm_bits, twm_width, twm_height);

    XCreateWindow(dpy, VersionWindow,
	4, 1,
	twm_width, twm_height,
	0, d_depth, CopyFromParent,
	d_visual, valuemask, &attributes);

    XSelectInput(dpy, VersionWindow, ExposureMask);
    XMapSubwindows(dpy, VersionWindow);
    XMapWindow(dpy, VersionWindow);

    SizeWindow = XCreateSimpleWindow(dpy, Root,
	0, 0,
	100,
	SizeFont.height + 4,
	BW,
	DefaultC.fore, DefaultC.back);


    XUngrabServer(dpy);

    HandlingEvents = TRUE;
    InitEvents();
    HandleEvents();
}

/***********************************************************************
 *
 *  Procedure:
 *	Done - cleanup and exit twm
 *
 *  Returned Value:
 *	none
 *
 *  Inputs:
 *	none
 *
 *  Outputs:
 *	none
 *
 *  Special Considerations:
 *	none
 *
 ***********************************************************************
 */

void
Done()
{
    TwmWindow *tmp;			/* temp twm window structure */
    unsigned x, y;
    XWindowChanges xwc;		/* change window structure */
    unsigned int xwcm;		/* change window mask */

    /* put a border back around all windows */

    for (tmp = TwmRoot.next; tmp != NULL; tmp = tmp->next)
    {
	XGetGeometry(dpy, tmp->w, &JunkRoot, &x, &y, &JunkWidth, &JunkHeight,
	    &JunkBW, &JunkDepth);

	xwcm = CWX | CWY | CWBorderWidth;

	xwc.x = x - (2 * BorderWidth);
	xwc.y = y - (2 * BorderWidth);
	xwc.border_width = BorderWidth;

	XConfigureWindow(dpy, tmp->w, xwcm, &xwc);
    }

    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    XCloseDisplay(dpy);
    exit(0);
}

/***********************************************************************
 *
 *  Procedure:
 *	Error - X error handler.  If we got here it is probably,
 *		because the client window went away and we haven't 
 *		got the DestroyNotify yet.
 *
 *  Inputs:
 *	dpy	- the connection to the X server
 *	event	- the error event structure
 *
 ***********************************************************************
 */

void
Error(dpy, event)
Display *dpy;
XErrorEvent *event;
{
    TwmWindow *tmp;			/* temp twm window structure */
    char buffer[BUFSIZ];

    /* Look for the window in the list, if it's there, remove it
     * from the list, and continue on, else ignore the error.
     * This is assumes that twm makes no errors. HaHa
    for (tmp = TwmRoot.next; tmp != NULL; tmp = tmp->next)
    {
	if (tmp->w == event->resourceid)
	{
	    if (tmp == Focus)
	    {
		FocusOnRoot();
	    }
	    XDeleteContext(dpy, tmp->w, TwmContext);
	    XDeleteContext(dpy, tmp->frame, TwmContext);
	    XDeleteContext(dpy, tmp->title_w, TwmContext);
	    XDeleteContext(dpy, tmp->iconify_w, TwmContext);
	    XDeleteContext(dpy, tmp->resize_w, TwmContext);
	    XDeleteContext(dpy, tmp->icon_w, TwmContext);
#ifndef NOFOCUS
	    XDeleteContext(dpy, tmp->focus_w, TwmContext);
#endif
	    XDeleteContext(dpy, tmp->hilite_w, TwmContext);

	    XDestroyWindow(dpy, tmp->frame);
	    XDestroyWindow(dpy, tmp->icon_w);
	    tmp->prev->next = tmp->next;
	    if (tmp->next != NULL)
		tmp->next->prev = tmp->prev;

	    free((char *)tmp);
	    return;
	}
    }
     */

    return;

    /*
    XGetErrorText(dpy, event->error_code, buffer, BUFSIZ);
    (void) fprintf(stderr, "X Error: %s\n", buffer);
    (void) fprintf(stderr, "  Request Major code: %d\n", event->request_code);
    (void) fprintf(stderr, "  Request Minor code: %d\n", event->minor_code);
    (void) fprintf(stderr, "  ResourceId 0x%x\n", event->resourceid);
    (void) fprintf(stderr, "  Error Serial #%d\n", event->serial);
    (void) fprintf(stderr, "  Current Serial #%d\n", dpy->request);

    Done();
    */
}

/***********************************************************************
 *
 *  Procedure:
 *	Other - error handler called if something else has set 
 *		the attributes on the root window.  Typically
 *		another window manager.
 *
 ***********************************************************************
 */

void
Other(dpy, event)
Display *dpy;
XErrorEvent *event;
{
    fprintf(stderr, "twm: Are you running another window manager?\n");
    exit(1);
}
