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

/**********************************************************************
 *
 * $Header: add_window.c,v 1.50.1.3 89/03/30 13:57:53 interran Exp $
 *
 * Add a new window, put the titlbar and other stuff around
 * the window
 *
 * 31-Mar-88 Tom LaStrange        Initial Version.
 *
 **********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$Header: add_window.c,v 1.50.1.3 89/03/30 13:57:53 interran Exp $";
#endif lint

#include <stdio.h>
#include "twm.h"
#include <X11/Xatom.h>
#include "add_window.h"
#include "util.h"
#include "resize.h"
#include "gram.h"
#include "list.h"
#include "events.h"
#include "menus.h"

#include "iconify.bm"
#include "resize.bm"
#ifndef NOFOCUS
#include "focus.bm"
#else
#define focus_width 0
#endif
#include "hilite.bm"

int AddingX;
int AddingY;
int AddingW;
int AddingH;

static int PlaceX = 50;
static int PlaceY = 50;

char NoName[] = "No Name"; /* name if no name is specified */

/***********************************************************************
 *
 *  Procedure:
 *	AddWindow - add a new window to the twm list
 *
 *  Returned Value:
 *	(TwmWindow *) - pointer to the TwmWindow structure
 *
 *  Inputs:
 *	w	- the window id of the window to add
 *
 ***********************************************************************
 */

TwmWindow *
AddWindow(w)
Window w;
{
    TwmWindow *tmp_win;			/* new twm window structure */
    int stat;
    char *prop;
    unsigned long valuemask;		/* mask for create windows */
    XSetWindowAttributes attributes;	/* attributes for create windows */
    int width, len;			/* tmp variable */
    int junk1, junk2, junk3;
    int x;
    Pixmap pm;			/* tmp pixmap variable */
    XWindowChanges xwc;		/* change window structure */
    unsigned int xwcm;		/* change window mask */
    int dont_know;		/* don't know where to put the window */
    XColor blob, cret;
    XEvent event;
    XGCValues	    gcv;
    unsigned long   gcm, mask;

#ifdef DEBUG
    fprintf(stderr, "AddWindow: w = 0x%x\n", w);
#endif

    /* allocate space for the twm window */
    tmp_win = (TwmWindow *)malloc(sizeof(TwmWindow));
    tmp_win->w = w;
    tmp_win->zoomed = ZOOM_NONE;

    if (w == IconManager)
	IconManagerPtr = tmp_win;

    XSelectInput(dpy, tmp_win->w, PropertyChangeMask);
    XGetWindowAttributes(dpy, tmp_win->w, &tmp_win->attr);
    XFetchName(dpy, tmp_win->w, &tmp_win->name);
    tmp_win->class = NoClass;
    XGetClassHint(dpy, tmp_win->w, &tmp_win->class);

#ifdef DEBUG
    fprintf(stderr, "  name = \"%s\"\n", tmp_win->name);
#endif
    tmp_win->wmhints = XGetWMHints(dpy, tmp_win->w);
    if (tmp_win->wmhints && (tmp_win->wmhints->flags & WindowGroupHint))
	tmp_win->group = tmp_win->wmhints->window_group;
    else
	tmp_win->group = NULL;

    if (XGetNormalHints(dpy, tmp_win->w, &tmp_win->hints) == 0)
	tmp_win->hints.flags = 0;

    if (strncmp("xterm", tmp_win->class.res_name, 5) == 0 ||
    	strncmp("XTerm", tmp_win->class.res_class, 5) == 0 ||
        strncmp("hpterm", tmp_win->class.res_name, 6) == 0 ||
    	strncmp("HPterm", tmp_win->class.res_class, 6) == 0)
	tmp_win->xterm = TRUE;
    else
	tmp_win->xterm = FALSE;

    dont_know = TRUE;
    if (tmp_win->hints.flags & PPosition)
    {
#ifdef DEBUG
	fprintf(stderr, "  program specified hints\n");
#endif
	tmp_win->attr.x = tmp_win->hints.x;
	tmp_win->attr.y = tmp_win->hints.y;
    }
    if (tmp_win->hints.flags & PSize)
    {
	tmp_win->attr.width = tmp_win->hints.width;
	tmp_win->attr.height = tmp_win->hints.height;
    }
    if (tmp_win->hints.flags & USPosition)
    {
#ifdef DEBUG
	fprintf(stderr, "  user specified hints\n");
#endif
	dont_know = FALSE;
	tmp_win->attr.x = tmp_win->hints.x;
	tmp_win->attr.y = tmp_win->hints.y;
    }
    if (tmp_win->hints.flags & USSize)
    {
	tmp_win->attr.width = tmp_win->hints.width;
	tmp_win->attr.height = tmp_win->hints.height;
    }

    if (tmp_win->name == NULL)
	tmp_win->name = NoName;
    if (tmp_win->class.res_name == NULL)
    	tmp_win->class.res_name = NoName;
    if (tmp_win->class.res_class == NULL)
    	tmp_win->class.res_class = NoName;

    tmp_win->full_name = tmp_win->name;

    tmp_win->highlight = !(short)LookInList(NO_HILITE, tmp_win->full_name, 
	&tmp_win->class);
    tmp_win->auto_raise = (short)LookInList(AUTO_RAISE, tmp_win->full_name, 
	&tmp_win->class);
    tmp_win->iconify_by_unmapping = IconifyByUnmapping;
    if (IconifyByUnmapping)
    {
	tmp_win->iconify_by_unmapping = 
	    !(short)LookInList(DONT_ICONIFY_BY_UNMAPPING, tmp_win->full_name,
		&tmp_win->class);
    }
    if (NoTitlebar || LookInList(NO_TITLE, tmp_win->full_name, &tmp_win->class))
	tmp_win->title_height = 0;
    else
	tmp_win->title_height = TITLE_BAR_HEIGHT + BorderWidth;

    if (HandlingEvents && dont_know && !RandomPlacement)
    {
	if (!(tmp_win->wmhints && tmp_win->wmhints->flags & StateHint &&
	    tmp_win->wmhints->initial_state == IconicState) &&
	   (tmp_win->xterm || (tmp_win->attr.x == 0 && tmp_win->attr.y == 0)))
	{
	    /* better wait until all the mouse buttons have been 
	     * released.
	     */
	    while (TRUE)
	    {
		XUngrabServer(dpy);
		XSync(dpy, False);
		XGrabServer(dpy);
		XQueryPointer(dpy, Root, &JunkRoot, &JunkChild,
		    &JunkX, &JunkY, &AddingX, &AddingY, &JunkMask);

		if (JunkMask != 0)
		    continue;

		stat = XGrabPointer(dpy, Root, False,
		    ButtonPressMask | ButtonReleaseMask,
		    GrabModeAsync, GrabModeAsync,
		    Root, UpperLeftCursor, CurrentTime);

		if (stat == GrabSuccess)
		    break;
	    }

	    width = XTextWidth(InitialFont.font, tmp_win->name,
		strlen(tmp_win->name)) + 20;
	    XResizeWindow(dpy, InitialWindow, width, InitialFont.height + 4);
	    XMapRaised(dpy, InitialWindow);
	    XDrawImageString(dpy, InitialWindow, InitialNormalGC,
		10, 2 + InitialFont.font->ascent,
		tmp_win->name, strlen(tmp_win->name));

	    AddingW = tmp_win->attr.width + 2 * BorderWidth;
	    AddingH = tmp_win->attr.height + tmp_win->title_height +
		2 * BorderWidth;

	    while (TRUE)
	    {
		XQueryPointer(dpy, Root, &JunkRoot, &JunkChild,
		    &JunkX, &JunkY, &AddingX, &AddingY, &JunkMask);

		MoveOutline(Root, AddingX, AddingY, AddingW, AddingH);

		if (XCheckTypedEvent(dpy, ButtonPress, &event))
		{
		    AddingX = event.xbutton.x_root;
		    AddingY = event.xbutton.y_root;
		    break;
		}
	    }

	    if (event.xbutton.button == Button2)
	    {
		XWarpPointer(dpy, None, Root, 0, 0, 0, 0,
		    AddingX + AddingW/2, AddingY + AddingH/2);
		AddStartResize(tmp_win, AddingX, AddingY, AddingW, AddingH);

		while (TRUE)
		{
		    int lastx, lasty;

		    XQueryPointer(dpy, Root, &JunkRoot, &JunkChild,
			&JunkX, &JunkY, &AddingX, &AddingY, &JunkMask);

		    if (lastx != AddingX || lasty != AddingY)
		    {
			DoResize(AddingX, AddingY, tmp_win);

			lastx = AddingX;
			lasty = AddingY;
		    }

		    if (XCheckTypedEvent(dpy, ButtonRelease, &event))
		    {
			AddEndResize(tmp_win);
			break;
		    }
		}
	    }

	    MoveOutline(Root, 0, 0, 0, 0);
	    XUnmapWindow(dpy, InitialWindow);
	    XUngrabPointer(dpy, CurrentTime);

	    tmp_win->attr.x = AddingX;
	    tmp_win->attr.y = AddingY + tmp_win->title_height;
	    tmp_win->attr.width = AddingW - 2 * BorderWidth;
	    tmp_win->attr.height = AddingH - tmp_win->title_height -
		(2 * BorderWidth);

	    XUngrabServer(dpy);
	}
    }
    else if (HandlingEvents && dont_know && RandomPlacement)
    {
	if ((PlaceX + tmp_win->attr.width) > MyDisplayWidth)
	    PlaceX = 50;
	if ((PlaceY + tmp_win->attr.height) > MyDisplayHeight)
	    PlaceY = 50;

	tmp_win->attr.x = PlaceX;
	tmp_win->attr.y = PlaceY;
	PlaceX += 30;
	PlaceY += 30;
    }

    if (tmp_win->attr.y < tmp_win->title_height)
	tmp_win->attr.y = tmp_win->title_height;

    xwcm = CWX | CWY | CWWidth | CWHeight | CWBorderWidth;

#ifdef DEBUG
	fprintf(stderr, "  position window  %d, %d  %dx%d\n", 
	    tmp_win->attr.x,
	    tmp_win->attr.y,
	    tmp_win->attr.width,
	    tmp_win->attr.height);
#endif
    xwc.x = tmp_win->attr.x + tmp_win->attr.border_width;
    xwc.y = tmp_win->attr.y + tmp_win->attr.border_width;
    xwc.width = tmp_win->attr.width;
    xwc.height = tmp_win->attr.height;
    xwc.border_width = 0;

    XConfigureWindow(dpy, tmp_win->w, xwcm, &xwc);

    tmp_win->name_width = XTextWidth(TitleBarFont.font, tmp_win->name,
	strlen(tmp_win->name));

    if (XGetIconName(dpy, tmp_win->w, &tmp_win->icon_name) == 0)
	tmp_win->icon_name = tmp_win->name;

    if (tmp_win->icon_name == NULL)
	tmp_win->icon_name = tmp_win->name;

    tmp_win->iconified = FALSE;
    tmp_win->icon = FALSE;
    tmp_win->icon_on = FALSE;

    /* add the window into the twm list */
    tmp_win->next = TwmRoot.next;
    if (TwmRoot.next != NULL)
	TwmRoot.next->prev = tmp_win;
    tmp_win->prev = &TwmRoot;
    TwmRoot.next = tmp_win;

    /* create windows */

    tmp_win->frame_x = tmp_win->attr.x;
    tmp_win->frame_y = tmp_win->attr.y - tmp_win->title_height;

    XGrabServer(dpy);
    tmp_win->frame = XCreateSimpleWindow(dpy, Root,
	tmp_win->frame_x,
	tmp_win->frame_y,
	tmp_win->attr.width,
	tmp_win->attr.height + tmp_win->title_height,
	BorderWidth,
	BorderColor, TitleC.back);
    
    tmp_win->title_w = XCreateSimpleWindow(dpy, tmp_win->frame,
	-BorderWidth, -BorderWidth,
	tmp_win->attr.width, TITLE_BAR_HEIGHT,
	BorderWidth,
	BorderColor, TitleC.back);

    if (Highlight && tmp_win->highlight)
    {
	XSetWindowBorderPixmap(dpy, tmp_win->frame, GrayTile);
	XSetWindowBorderPixmap(dpy, tmp_win->title_w, GrayTile);
    }

    gcm = 0;
    gcm |= GCFont;	    gcv.font = TitleBarFont.font->fid;
    gcm |= GCPlaneMask;	    gcv.plane_mask = AllPlanes;
    gcm |= GCForeground;    gcv.foreground = TitleC.fore;
    gcm |= GCBackground;    gcv.background = TitleC.back;

    tmp_win->title_gc = XCreateGC(dpy, Root, gcm, &gcv);

    CreateTitleButtons(tmp_win);

	
    XDefineCursor(dpy, tmp_win->frame, ArrowCursor);
    XDefineCursor(dpy, tmp_win->title_w, ArrowCursor);
    XDefineCursor(dpy, tmp_win->iconify_w, ButtonCursor);
#ifndef NOFOCUS
    XDefineCursor(dpy, tmp_win->focus_w, ButtonCursor);
#endif
    XDefineCursor(dpy, tmp_win->resize_w, ButtonCursor);

    XSelectInput(dpy, tmp_win->w, StructureNotifyMask | PropertyChangeMask |
	ColormapChangeMask);
    XSelectInput(dpy, tmp_win->frame,
	SubstructureRedirectMask | VisibilityChangeMask |
	ButtonPressMask | ButtonReleaseMask |
	EnterWindowMask | LeaveWindowMask);

    XSelectInput(dpy, tmp_win->title_w, 
	KeyPressMask |
	ButtonPressMask | ButtonReleaseMask | ExposureMask);

    XAddToSaveSet(dpy, tmp_win->w);
    XReparentWindow(dpy, tmp_win->w, tmp_win->frame, 0, tmp_win->title_height);

    SetupWindow(tmp_win,
	tmp_win->frame_x,
	tmp_win->frame_y,
	tmp_win->attr.width,
	tmp_win->attr.height + tmp_win->title_height);

    pm = NULL;
    tmp_win->forced = FALSE;

    /* now go through the steps to get an icon window,  if ForceIcon is 
     * set, then no matter what else is defined, the bitmap from the
     * .twmrc file is used
     */
    if (ForceIcon)
    {
	Pixmap bm;
	XImage *image;
	unsigned mask;

        if ((bm = (Pixmap)LookInNameList(ICONS, tmp_win->full_name)) == NULL)
	    bm = (Pixmap)LookInList(ICONS, tmp_win->full_name, &tmp_win->class);
	if (bm != NULL)
	{
	    XGetGeometry(dpy, bm, &JunkRoot, &JunkX, &JunkY,
		&tmp_win->icon_width, &tmp_win->icon_height,
		&JunkBW, &JunkDepth);

	    pm = XCreatePixmap(dpy, Root, tmp_win->icon_width,
		tmp_win->icon_height, d_depth);

	    /* the copy plane works on color ! */
	    XCopyPlane(dpy, bm, pm, IconNormalGC,
		0,0, tmp_win->icon_width, tmp_win->icon_height, 0, 0, 1 );

	    tmp_win->forced = TRUE;
	}
    }

    /* if the pixmap is still NULL, we didn't get one from the above code,
     * that could mean that ForceIcon was not set, or that the window
     * was not in the Icons list, now check the WM hints for an icon
     */
    if (pm == NULL && tmp_win->wmhints &&
	tmp_win->wmhints->flags & IconPixmapHint)
    {
    
	XGetGeometry(dpy,   tmp_win->wmhints->icon_pixmap,
             &JunkRoot, &JunkX, &JunkY,
	     &tmp_win->icon_width, &tmp_win->icon_height, &JunkBW, &JunkDepth);

	pm = XCreatePixmap(dpy, Root, tmp_win->icon_width, tmp_win->icon_height,
	    d_depth);

	XCopyPlane(dpy, tmp_win->wmhints->icon_pixmap, pm, IconNormalGC,
	    0,0, tmp_win->icon_width, tmp_win->icon_height, 0, 0, 1 );
    }

    /* if we still haven't got an icon, let's look in the Icon list 
     * if ForceIcon is not set
     */
    if (pm == NULL && !ForceIcon)
    {
	Pixmap bm;
	XImage *image;
	unsigned mask;

        if ((bm = (Pixmap)LookInNameList(ICONS, tmp_win->full_name)) == NULL)
	    bm = (Pixmap)LookInList(ICONS, tmp_win->full_name, &tmp_win->class);
	if (bm != NULL)
	{
	    XGetGeometry(dpy, bm, &JunkRoot, &JunkX, &JunkY,
		&tmp_win->icon_width, &tmp_win->icon_height,
		&JunkBW, &JunkDepth);

	    pm = XCreatePixmap(dpy, Root, tmp_win->icon_width,
		tmp_win->icon_height, d_depth);

	    /* the copy plane works on color ! */
	    XCopyPlane(dpy, bm, pm, IconNormalGC,
		0,0, tmp_win->icon_width, tmp_win->icon_height, 0, 0, 1 );

	    /*
	    this code works on a monochrome system

	    XPutImage(dpy, pm, IconNormalGC,
		image, 0, 0, 0, 0, tmp_win->icon_width, tmp_win->icon_height);
	    */
	}
    }

    /* if we still don't have an icon, assign the UnknownIcon */

    if (pm == NULL && UnknownPm != NULL)
    {
	tmp_win->icon_width = UnknownWidth;
	tmp_win->icon_height = UnknownHeight;

	pm = XCreatePixmap(dpy, Root, tmp_win->icon_width,
	    tmp_win->icon_height, d_depth);

	/* the copy plane works on color ! */
	XCopyPlane(dpy, UnknownPm, pm, IconNormalGC,
	    0,0, tmp_win->icon_width, tmp_win->icon_height, 0, 0, 1 );
    }

    if (pm == NULL)
    {
	tmp_win->icon_height = 0;
	tmp_win->icon_width = 0;
	valuemask = 0;
    }
    else
    {
	valuemask = CWBackPixmap;
	attributes.background_pixmap = pm;
    }

    tmp_win->icon_w_width = XTextWidth(IconFont.font,
	tmp_win->icon_name, strlen(tmp_win->icon_name));

    tmp_win->icon_w_width += 6;
    if (tmp_win->icon_w_width < tmp_win->icon_width)
    {
	tmp_win->icon_x = (tmp_win->icon_width - tmp_win->icon_w_width)/2;
	tmp_win->icon_x += 3;
	tmp_win->icon_w_width = tmp_win->icon_width;
    }
    else
    {
	tmp_win->icon_x = 3;
    }
    tmp_win->icon_y = tmp_win->icon_height + IconFont.height;

    if (tmp_win->wmhints && tmp_win->wmhints->flags & IconWindowHint)
    {
	tmp_win->icon_w = tmp_win->wmhints->icon_window;
    }
    else
    {
	tmp_win->icon_w = XCreateSimpleWindow(dpy, Root,
	    0,0,
	    tmp_win->icon_w_width,
	    tmp_win->icon_height + IconFont.height + 4,
	    2, IconBorderColor, IconC.back);
    }

    XSelectInput(dpy, tmp_win->icon_w,
	KeyPressMask | VisibilityChangeMask |
	ButtonPressMask | ButtonReleaseMask | ExposureMask);

    tmp_win->icon_bm_w = NULL;
    if (pm != NULL)
    {
	if (tmp_win->icon_w_width == tmp_win->icon_width)
	    x = 0;
	else
	    x = (tmp_win->icon_w_width - tmp_win->icon_width)/2;

	tmp_win->icon_bm_w = XCreateWindow(dpy, tmp_win->icon_w,
	    x, 0, tmp_win->icon_width, tmp_win->icon_height,
	    0, d_depth, CopyFromParent,
	    d_visual, valuemask, &attributes);
    }

    XDefineCursor(dpy, tmp_win->icon_w, ArrowCursor);

    GrabButtons(tmp_win);
    GrabKeys(tmp_win);

    tmp_win->list = AddIconManager(tmp_win);
    if (tmp_win->list)
    {
	XSaveContext(dpy, tmp_win->list->w, TwmContext, tmp_win);
	XSaveContext(dpy, tmp_win->list->icon, TwmContext, tmp_win);
    }

    XSaveContext(dpy, tmp_win->w, TwmContext, tmp_win);
    XSaveContext(dpy, tmp_win->frame, TwmContext, tmp_win);
    XSaveContext(dpy, tmp_win->title_w, TwmContext, tmp_win);
    XSaveContext(dpy, tmp_win->iconify_w, TwmContext, tmp_win);
    XSaveContext(dpy, tmp_win->resize_w, TwmContext, tmp_win);
    XSaveContext(dpy, tmp_win->icon_w, TwmContext, tmp_win);
#ifndef NOFOCUS
    XSaveContext(dpy, tmp_win->focus_w, TwmContext, tmp_win);
#endif
    XSaveContext(dpy, tmp_win->hilite_w, TwmContext, tmp_win);

    SetHints(tmp_win);
    XUngrabServer(dpy);

    return (tmp_win);
}

/***********************************************************************
 *
 *  Procedure:
 *	MappedNotOverride - checks to see if we should really
 *		put a twm frame on the window
 *
 *  Returned Value:
 *	TRUE	- go ahead and frame the window
 *	FALSE	- don't frame the window
 *
 *  Inputs:
 *	w	- the window to check
 *
 ***********************************************************************
 */

int
MappedNotOverride(w)
    Window w;
{
    XWindowAttributes wa;

    XGetWindowAttributes(dpy, w, &wa);
    return ((wa.map_state != IsUnmapped) && (wa.override_redirect != True));
}

/***********************************************************************
 *
 *  Procedure:
 *	GrabAllButtons - grab needed buttons for all windows
 *
 ***********************************************************************
 */

void
GrabAllButtons()
{
    TwmWindow *tmp_win;

    for (tmp_win = TwmRoot.next; tmp_win != NULL; tmp_win = tmp_win->next)
    {
	GrabButtons(tmp_win);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	GrabAllKeys - grab needed keys for all windows
 *
 ***********************************************************************
 */

void
GrabAllKeys()
{
    TwmWindow *tmp_win;

    for (tmp_win = TwmRoot.next; tmp_win != NULL; tmp_win = tmp_win->next)
    {
	GrabKeys(tmp_win);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	UngrabAllButtons - ungrab buttons for all windows
 *
 ***********************************************************************
 */

void
UngrabAllButtons()
{
    TwmWindow *tmp_win;

    for (tmp_win = TwmRoot.next; tmp_win != NULL; tmp_win = tmp_win->next)
    {
	UngrabButtons(tmp_win);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	UngrabAllKeys - ungrab keys for all windows
 *
 ***********************************************************************
 */

void
UngrabAllKeys()
{
    TwmWindow *tmp_win;

    for (tmp_win = TwmRoot.next; tmp_win != NULL; tmp_win = tmp_win->next)
    {
	UngrabKeys(tmp_win);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	GrabButtons - grab needed buttons for the window
 *
 *  Inputs:
 *	tmp_win - the twm window structure to use
 *
 ***********************************************************************
 */

void
GrabButtons(tmp_win)
TwmWindow *tmp_win;
{
    int i, j;

    for (i = 0; i < MAX_BUTTONS+1; i++)
    {
	for (j = 0; j < MOD_SIZE; j++)
	{
	    if (Mouse[i][C_WINDOW][j].func != NULL)
	    {
		XGrabButton(dpy, i, j, tmp_win->w,
		    True, ButtonPressMask | ButtonReleaseMask,
		    GrabModeAsync, GrabModeAsync, None, ArrowCursor);
	    }
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	GrabKeys - grab needed keys for the window
 *
 *  Inputs:
 *	tmp_win - the twm window structure to use
 *
 ***********************************************************************
 */

void
GrabKeys(tmp_win)
TwmWindow *tmp_win;
{
    FuncKey *tmp;

    for (tmp = FuncKeyRoot.next; tmp != NULL; tmp = tmp->next)
    {
	switch (tmp->cont)
	{
	case C_WINDOW:
	    XGrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->w, True,
		GrabModeAsync, GrabModeAsync);
	    break;

	case C_ICON:
	    XGrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->icon_w, True,
		GrabModeAsync, GrabModeAsync);

	case C_TITLE:
	    XGrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->title_w, True,
		GrabModeAsync, GrabModeAsync);
	    break;

	case C_NAME:
	    XGrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->w, True,
		GrabModeAsync, GrabModeAsync);
	    XGrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->icon_w, True,
		GrabModeAsync, GrabModeAsync);
	    XGrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->title_w, True,
		GrabModeAsync, GrabModeAsync);
	    break;
	/*
	case C_ROOT:
	    XGrabKey(dpy, tmp->keycode, tmp->mods, Root, True,
		GrabModeAsync, GrabModeAsync);
	    break;
	*/
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	UngrabButtons - ungrab buttons for windows
 *
 *  Inputs:
 *	tmp_win - the twm window structure to use
 *
 ***********************************************************************
 */

void
UngrabButtons(tmp_win)
TwmWindow *tmp_win;
{
    int i;

    for (i = 0; i < MAX_BUTTONS+1; i++)
    {
	XUngrabButton(dpy, i, AnyModifier, tmp_win->w);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	UngrabKeys - ungrab keys for windows
 *
 *  Inputs:
 *	tmp_win - the twm window structure to use
 *
 ***********************************************************************
 */

void
UngrabKeys(tmp_win)
TwmWindow *tmp_win;
{
    FuncKey *tmp;

    for (tmp = FuncKeyRoot.next; tmp != NULL; tmp = tmp->next)
    {
	switch (tmp->cont)
	{
	case C_WINDOW:
	    XUngrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->w);
	    break;

	case C_ICON:
	    XUngrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->icon_w);

	case C_TITLE:
	    XUngrabKey(dpy, tmp->keycode, tmp->mods, tmp_win->title_w);
	    break;

	case C_ROOT:
	    XUngrabKey(dpy, tmp->keycode, tmp->mods, Root);
	    break;
	}
    }
}

CreateTitleButtons(tmp_win)
TwmWindow *tmp_win;
{
    unsigned long valuemask;		/* mask for create windows */
    XSetWindowAttributes attributes;	/* attributes for create windows */

    /* the three buttons have the pixmap as the background of the
     * window, that way I don't have to worry about repainting them
     * on expose events.
     */

    valuemask = CWEventMask | CWBackPixmap;
    attributes.event_mask = ButtonPressMask | ButtonReleaseMask;
    tmp_win->iconify_pm = MakePixmap(tmp_win->title_w,
	tmp_win->title_gc, iconify_bits, iconify_width, iconify_height);
    attributes.background_pixmap = tmp_win->iconify_pm;

    tmp_win->iconify_w = XCreateWindow(dpy, tmp_win->title_w,
	TITLE_BAR_SPACE, TITLE_BAR_SPACE,
	iconify_width, iconify_height,
	0, d_depth, CopyFromParent,
	d_visual, valuemask, &attributes);

#ifndef NOFOCUS
    tmp_win->focus_pm = MakePixmap(tmp_win->title_w,
	tmp_win->title_gc, focus_bits, focus_width, focus_height);
    attributes.background_pixmap = tmp_win->focus_pm;

    tmp_win->focus_w = XCreateWindow(dpy, tmp_win->title_w,
	tmp_win->attr.width - resize_width -3 - focus_width, TITLE_BAR_SPACE,
	iconify_width, iconify_height,
	0, d_depth, CopyFromParent,
	d_visual, valuemask, &attributes);
#endif

    tmp_win->resize_pm = MakePixmap(tmp_win->title_w,
	tmp_win->title_gc, resize_bits, resize_width, resize_height);
    attributes.background_pixmap = tmp_win->resize_pm;

    tmp_win->resize_w = XCreateWindow(dpy, tmp_win->title_w,
	tmp_win->attr.width - resize_width - 1,
	TITLE_BAR_SPACE,
	resize_width, resize_height,
	0, d_depth, CopyFromParent,
	d_visual, valuemask, &attributes);
	
    valuemask = CWBackPixmap;
    tmp_win->hilite_pm = MakePixmap(tmp_win->title_w,
	tmp_win->title_gc, hilite_bits, hilite_width, hilite_height);
    attributes.background_pixmap = tmp_win->hilite_pm;

    tmp_win->hilite_w = XCreateWindow(dpy, tmp_win->title_w,
	TitleBarX, 1,
	10, hilite_height,
	0, d_depth, CopyFromParent,
	d_visual, valuemask, &attributes);

    XMapSubwindows(dpy, tmp_win->title_w);
    XUnmapWindow(dpy, tmp_win->hilite_w);
}
