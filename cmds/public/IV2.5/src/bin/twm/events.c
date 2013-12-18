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
 * $Header: events.c,v 1.91.1.1 89/03/30 10:34:27 interran Exp $
 *
 * twm event handling
 *
 * 17-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$Header: events.c,v 1.91.1.1 89/03/30 10:34:27 interran Exp $";
#endif

#include <stdio.h>
#include "twm.h"
#include <X11/Xatom.h>
#include "add_window.h"
#include "menus.h"
#include "events.h"
#include "resize.h"
#include "gram.h"
#include "util.h"
#include "twm.bm"

event_proc EventHandler[LASTEvent]; /* event handler jump table */
static XEvent event;		/* the current event */
static TwmWindow *tmp_win;	/* the current twm window */
static Window w;		/* the window that caused the event */
static int Context = C_NO_CONTEXT;  /* current button press context */
static TwmWindow *ButtonWindow;	/* button press window structure */
static XEvent ButtonEvent;	/* button preee event */
static char *Action;

int ConstMove = FALSE;		/* constrained move variables */
int ConstMoveDir;
int ConstMoveX;
int ConstMoveY;
int ConstMoveXL;
int ConstMoveXR;
int ConstMoveYT;
int ConstMoveYB;

Window DragWindow;		/* variables used in moving windows */
int DragX;
int DragY;
int DragWidth;
int DragHeight;
static int enter_flag;

/***********************************************************************
 *
 *  Procedure:
 *	InitEvents - initialize the event jump table
 *
 ***********************************************************************
 */

void
InitEvents()
{
    int i;

    ResizeWindow = NULL;
    DragWindow = NULL;
    enter_flag = FALSE;

    for (i = 0; i < LASTEvent; i++)
	EventHandler[i] = HandleUnknown;

    EventHandler[Expose] = HandleExpose;
    EventHandler[DestroyNotify] = HandleDestroyNotify;
    EventHandler[MapRequest] = HandleMapRequest;
    EventHandler[MapNotify] = HandleMapNotify;
    EventHandler[UnmapNotify] = HandleUnmapNotify;
    EventHandler[MotionNotify] = HandleMotionNotify;
    EventHandler[ButtonRelease] = HandleButtonRelease;
    EventHandler[ButtonPress] = HandleButtonPress;
    EventHandler[EnterNotify] = HandleEnterNotify;
    EventHandler[LeaveNotify] = HandleLeaveNotify;
    EventHandler[ConfigureRequest] = HandleConfigureRequest;
    EventHandler[ClientMessage] = HandleClientMessage;
    EventHandler[PropertyNotify] = HandlePropertyNotify;
    EventHandler[KeyPress] = HandleKeyPress;
    EventHandler[ColormapNotify] = HandleColormapNotify;
    EventHandler[VisibilityNotify] = HandleVisibilityNotify;
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleEvents - handle X events
 *
 ***********************************************************************
 */

void
HandleEvents()
{
    while (TRUE)
    {
	if ( (DragWindow || ResizeWindow) && !XPending(dpy) )
	{
	    /*
	    ** Hack to support polled dynamics.  Should really
	    ** build up "mode" support that callers can register
	    ** handlers with, etc.
	    */
	    w = DragWindow ? DragWindow : ResizeWindow;
	    XQueryPointer( dpy, w, &(event.xmotion.root), &JunkChild,
			  &(event.xmotion.x_root), &(event.xmotion.y_root),
			  &JunkX, &JunkY, &JunkMask);
	    (*EventHandler[MotionNotify])();
	}
	else
	{
	    XNextEvent(dpy, &event);
	    w = event.xany.window;
	    if (XFindContext(dpy, w, TwmContext, &tmp_win) == XCNOENT)
		tmp_win = NULL;

#ifdef DEBUG
	    if (event.type != MotionNotify)
	    if (tmp_win != NULL)
	    {
		fprintf(stderr, "Event w=%x, t->w=%x, t->frame=%x, t->title=%x, ",
			w, tmp_win->w, tmp_win->frame, tmp_win->title_w);
	    }
	    else
	    {
		fprintf(stderr, "Event w=%x, ", w);
	    }
#endif
	    if (event.type >= 0 && event.type < LASTEvent)
	        (*EventHandler[event.type])();
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleVisiblityNotify - visibility notify event handler
 *
 ***********************************************************************
 */

void
HandleVisibilityNotify()
{
#ifdef DEBUG
    fprintf(stderr, "VisibilityNotify\n");
    fprintf(stderr, "  visibility = %d\n", event.xvisibility.state);
#endif
    if (tmp_win == NULL)
	return;

    if (w == tmp_win->frame)
	tmp_win->frame_vis = event.xvisibility.state;
    else if (w == tmp_win->icon_w)
	tmp_win->icon_vis = event.xvisibility.state;
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleColormapNotify - colormap notify event handler
 *
 ***********************************************************************
 */

void
HandleColormapNotify()
{
#ifdef DEBUG
    fprintf(stderr, "ColormapNotify\n");
#endif
    if (tmp_win != NULL)
	XGetWindowAttributes(dpy, tmp_win->w, &tmp_win->attr);

    if (tmp_win == Focus)
	XInstallColormap(dpy, tmp_win->attr.colormap);
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleKeyPress - key press event handler
 *
 ***********************************************************************
 */

void
HandleKeyPress()
{
    FuncKey *key;
    int len;

    Context = C_NO_CONTEXT;

    if (w == Root)
	Context = C_ROOT;
    if (tmp_win)
    {
	if (w == tmp_win->title_w)
	    Context = C_TITLE;
	if (w == tmp_win->w)
	    Context = C_WINDOW;
	if (w == tmp_win->icon_w)
	    Context = C_ICON;
	if (w == tmp_win->frame)
	    Context = C_FRAME;
    }

    for (key = FuncKeyRoot.next; key != NULL; key = key->next)
    {
	if (key->keycode == event.xkey.keycode &&
	    key->mods == event.xkey.state &&
	    (key->cont == Context || key->cont == C_NAME))
	{
	    /* weed out the functions that don't make sense to execute
	     * from a key press 
	     */
	    if (key->func == F_MOVE || key->func == F_RESIZE)
		return;

	    if (key->cont != C_NAME)
	    {
		ExecuteFunction(key->func, key->action, w,
		    tmp_win, event, Context, FALSE);
	    }
	    else
	    {
		len = strlen(key->win_name);
		for (tmp_win = TwmRoot.next; tmp_win != NULL;
		    tmp_win = tmp_win->next)
		{
		    if (!strncmp(key->win_name, tmp_win->name, len) ||
			!strncmp(key->win_name, tmp_win->class.res_name, len) ||
			!strncmp(key->win_name, tmp_win->class.res_class, len))
		    {
			ExecuteFunction(key->func, key->action, tmp_win->frame,
			    tmp_win, event, C_FRAME, FALSE);
		    }
		}
	    }
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandlePropertyNotify - property notify event handler
 *
 ***********************************************************************
 */

void
HandlePropertyNotify()
{
    char *prop;
    XWMHints *wmhints;
    XSizeHints hints;
    Atom actual;
    int junk1, junk2, len;
    int width, height, x, y;
    unsigned long valuemask;		/* mask for create windows */
    XSetWindowAttributes attributes;	/* attributes for create windows */
    Pixmap pm;

#ifdef DEBUG
    fprintf(stderr, "PropertyNotify = %d\n", event.xproperty.atom);
#endif

    if (tmp_win == NULL)
	return;

    XGetWindowProperty(dpy, tmp_win->w, event.xproperty.atom, 0, 200, False,
	XA_STRING, &actual, &junk1, &junk2, &len, &prop);

    if (actual == None)
	return;

    if (prop == NULL)
	prop = NoName;

    switch (event.xproperty.atom)
    {
    case XA_WM_NAME:
	tmp_win->full_name = prop;
	tmp_win->name = prop;

	tmp_win->name_width = XTextWidth(TitleBarFont.font, tmp_win->name,
	    strlen(tmp_win->name));

	SetupWindow(tmp_win,
	    tmp_win->frame_x, tmp_win->frame_y,
	    tmp_win->frame_width, tmp_win->frame_height);

	XClearWindow(dpy, tmp_win->title_w);

	XDrawImageString(dpy, tmp_win->title_w,
	    tmp_win->title_gc, TitleBarX, TitleBarFont.y,
	    tmp_win->name, strlen(tmp_win->name));

	if (tmp_win->list)
	{
	    XClearWindow(dpy, tmp_win->list->w);
	    XDrawImageString(dpy,tmp_win->list->w, IconManagerGC,
		IconManagerX, IconManagerFont.y,
		tmp_win->name, strlen(tmp_win->name));
	}

	/* if the icon name is NoName, set the name of the icon to be
	 * the same as the window 
	 */
	if (tmp_win->icon_name == NoName)
	{
	    tmp_win->icon_name = tmp_win->name;
	    RedoIconName();
	}
	break;

    case XA_WM_ICON_NAME:
	tmp_win->icon_name = prop;

	RedoIconName();
	break;

    case XA_WM_HINTS:
	tmp_win->wmhints = XGetWMHints(dpy, w);

	if (tmp_win->wmhints && (tmp_win->wmhints->flags & WindowGroupHint))
	    tmp_win->group = tmp_win->wmhints->window_group;

	if (!tmp_win->forced && tmp_win->wmhints &&
	    tmp_win->wmhints->flags & IconWindowHint)
	{
	    tmp_win->icon_w = tmp_win->wmhints->icon_window;
	}

	if (!tmp_win->forced && tmp_win->wmhints &&
	    (tmp_win->wmhints->flags & IconPixmapHint))
	{
	    XGetGeometry(dpy, tmp_win->wmhints->icon_pixmap, &JunkRoot,
		&JunkX, &JunkY, &tmp_win->icon_width, &tmp_win->icon_height,
		&JunkBW, &JunkDepth);

	    pm = XCreatePixmap(dpy, Root, tmp_win->icon_width,
		tmp_win->icon_height, d_depth);

	    XCopyPlane(dpy, tmp_win->wmhints->icon_pixmap, pm, IconNormalGC,
		0,0, tmp_win->icon_width, tmp_win->icon_height, 0, 0, 1 );

	    valuemask = CWBackPixmap;
	    attributes.background_pixmap = pm;

	    if (tmp_win->icon_bm_w)
		XDestroyWindow(dpy, tmp_win->icon_bm_w);

	    tmp_win->icon_bm_w = XCreateWindow(dpy, tmp_win->icon_w,
		0, 0, tmp_win->icon_width, tmp_win->icon_height,
		0, d_depth, CopyFromParent, d_visual,
		valuemask, &attributes);

	    RedoIconName();
	}
	break;

    case XA_WM_NORMAL_HINTS:
	XGetNormalHints(dpy, tmp_win->w, &hints);
	/* don't do anything */
	break;

    default:
#ifdef DEBUG
	fprintf(stderr, "TWM Not handling property %d\n",event.xproperty.atom);
#endif
	break;
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	RedoIconName - procedure to re-position the icon window and name
 *
 ***********************************************************************
 */

RedoIconName()
{
    int x;

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

    if (tmp_win->icon_w_width == tmp_win->icon_width)
	x = 0;
    else
	x = (tmp_win->icon_w_width - tmp_win->icon_width)/2;

    XResizeWindow(dpy, tmp_win->icon_w, tmp_win->icon_w_width,
	tmp_win->icon_height + IconFont.height + 4);
    if (tmp_win->icon_bm_w)
    {
	XMoveWindow(dpy, tmp_win->icon_bm_w, x, 0);
	XMapWindow(dpy, tmp_win->icon_bm_w);
    }
    if (tmp_win->icon)
    {
	XClearArea(dpy, tmp_win->icon_w, 0, 0, 0, 0, False);
	XDrawImageString(dpy, tmp_win->icon_w,
	    IconNormalGC,
	    tmp_win->icon_x, tmp_win->icon_y,
	    tmp_win->icon_name, strlen(tmp_win->icon_name));
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleClientMessage - client message event handler
 *
 ***********************************************************************
 */

void
HandleClientMessage()
{
    /* remove when Xatom.h defines XA_WM_CHANGE_STATE */
    static Atom WM_CHANGE_STATE = None;
    if (WM_CHANGE_STATE == None) {
	WM_CHANGE_STATE = XInternAtom(dpy, "WM_CHANGE_STATE", False);
    }

#ifdef DEBUG
    fprintf(stderr, "ClientMessage = 0x%x\n", event.xclient.message_type);
#endif

    if (event.xclient.message_type == NULL)
    {
	enter_flag = FALSE;
    }
    else if (event.xclient.message_type == WM_CHANGE_STATE)
    {
	if (tmp_win != NULL)
	{
	    if (event.xclient.data.l[0] == IconicState && !tmp_win->icon)
	    {
		XEvent button;

		XQueryPointer( dpy, Root, &JunkRoot, &JunkChild,
			      &(button.xmotion.x_root),
			      &(button.xmotion.y_root),
			      &JunkX, &JunkY, &JunkMask);

		ExecuteFunction(F_ICONIFY, NULL, w, tmp_win, button,
		    FRAME, FALSE);
	    }
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleExpose - expose event handler
 *
 ***********************************************************************
 */

void
HandleExpose()
{
    MenuItem *tmp;
    WList *list;

#ifdef DEBUG
    fprintf(stderr, "Expose %d\n", event.xexpose.count);
#endif

    if (event.xexpose.count != 0)
	return;

    if (w == VersionWindow)
    {
	XDrawImageString(dpy, VersionWindow, VersionNormalGC,
	    twm_width + 10,
	    2 + VersionFont.font->ascent, Version, strlen(Version));
	return;
    }

    if (tmp_win != NULL)
    {
	if (tmp_win->title_w == w)
	{
	    XDrawImageString(dpy, tmp_win->title_w,
		tmp_win->title_gc,
		TitleBarX, TitleBarFont.y,
		tmp_win->name, strlen(tmp_win->name));
	    return;
	}

	if (tmp_win->icon_w == w)
	{
	    XDrawImageString(dpy, tmp_win->icon_w,
		IconNormalGC,
		tmp_win->icon_x, tmp_win->icon_y,
		tmp_win->icon_name, strlen(tmp_win->icon_name));
	    return;
	}
    }

    if (XFindContext(dpy, w, MenuContext, &tmp) == 0)
    {
	if (tmp->func == F_TITLE)
	    XDrawImageString(dpy,w, MenuTitleGC, tmp->y, MenuFont.y,
		tmp->item, strlen(tmp->item));
	else if (tmp->state)
	    XDrawImageString(dpy,w, MenuReverseGC, tmp->y, MenuFont.y,
		tmp->item, strlen(tmp->item));
	else
	    XDrawImageString(dpy,w, MenuNormalGC, tmp->y, MenuFont.y,
		tmp->item, strlen(tmp->item));

	return;
    }

    if (XFindContext(dpy, w, IconManagerContext, &list) == 0)
    {
	XDrawImageString(dpy,w, IconManagerGC, IconManagerX, IconManagerFont.y,
	    list->twm->name, strlen(list->twm->name));

	return;
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleDestroyNotify - DestroyNotify event handler
 *
 ***********************************************************************
 */

void
HandleDestroyNotify()
{
#ifdef DEBUG
    fprintf(stderr, "DestroyNotify\n");
#endif
    if (tmp_win == NULL)
	return;

    if (tmp_win == Focus)
    {
	FocusOnRoot();
    }
    XDeleteContext(dpy, tmp_win->w, TwmContext);
    XDeleteContext(dpy, tmp_win->frame, TwmContext);
    XDeleteContext(dpy, tmp_win->title_w, TwmContext);
    XDeleteContext(dpy, tmp_win->iconify_w, TwmContext);
    XDeleteContext(dpy, tmp_win->resize_w, TwmContext);
    XDeleteContext(dpy, tmp_win->icon_w, TwmContext);
#ifndef NOFOCUS
    XDeleteContext(dpy, tmp_win->focus_w, TwmContext);
#endif
    XDeleteContext(dpy, tmp_win->hilite_w, TwmContext);

    XDestroyWindow(dpy, tmp_win->frame);
    XDestroyWindow(dpy, tmp_win->icon_w);
    if (tmp_win->list != NULL)
    {
	XDeleteContext(dpy, tmp_win->list->w, TwmContext);
	XDeleteContext(dpy, tmp_win->list->icon, TwmContext);
    }
    RemoveIconManager(tmp_win);
    XFreeGC(dpy, tmp_win->title_gc);
    tmp_win->prev->next = tmp_win->next;
    if (tmp_win->next != NULL)
	tmp_win->next->prev = tmp_win->prev;

    free((char *)tmp_win);
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleMapRequest - MapRequest event handler
 *
 ***********************************************************************
 */

void
HandleMapRequest()
{
    int stat;
    XSizeHints hints;
    int zoom_save;

#ifdef DEBUG
    fprintf(stderr, "MapRequest w = 0x%x\n", event.xmaprequest.window);
#endif

    w = event.xmaprequest.window;
    stat = XFindContext(dpy, w, TwmContext, &tmp_win);
    if (stat == XCNOENT)
	tmp_win = NULL;

    if (tmp_win == NULL)
    {
	if (Transient(w) && !DecorateTransients)
	{
	    XMapRaised(dpy, w);
	    return;
	}

	tmp_win = AddWindow(w);
	if (tmp_win->wmhints && (tmp_win->wmhints->flags & StateHint))
	{
	    switch (tmp_win->wmhints->initial_state)
	    {
		case DontCareState:
		case NormalState:
		case ZoomState:
		case InactiveState:
		    XMapWindow(dpy, tmp_win->w);
		    XMapRaised(dpy, tmp_win->frame);
		    break;

		case IconicState:
		    if (tmp_win->wmhints->flags & IconPositionHint)
		    {
			int x, y;

			x = tmp_win->wmhints->icon_x;
			y = tmp_win->wmhints->icon_y;

			if (x > MyDisplayWidth)
			    x = MyDisplayWidth - tmp_win->icon_w_width -
				(2 * BorderWidth);

			if (y > MyDisplayHeight)
			    y = MyDisplayHeight - tmp_win->icon_height -
				IconFont.height - 4 - (2 * BorderWidth);


			XMoveWindow(dpy, tmp_win->icon_w, x, y);
		    }
		    else
		    {
			XMoveWindow(dpy, tmp_win->icon_w, 0, 0);
		    }

		    tmp_win->iconified = TRUE;
		    zoom_save = DoZoom;
		    DoZoom = FALSE;
		    Iconify(tmp_win);
		    DoZoom = zoom_save;
		    break;
	    }
	}
	else
	{
	    DeIconify(tmp_win);
	}
    }
    else
    {
	DeIconify(tmp_win);
    }
    XRaiseWindow(dpy, VersionWindow);
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleMapNotify - MapNotify event handler
 *
 ***********************************************************************
 */

void
HandleMapNotify()
{
#ifdef DEBUG
    fprintf(stderr, "MapNotify\n");
#endif
    if (tmp_win == NULL)
	return;

    XUnmapWindow(dpy, tmp_win->icon_w);
    XMapSubwindows(dpy, tmp_win->title_w);
    XMapSubwindows(dpy, tmp_win->frame);
    if (Focus != tmp_win)
	XUnmapWindow(dpy, tmp_win->hilite_w);

    if (tmp_win->title_height == 0)
	XUnmapWindow(dpy, tmp_win->title_w);

    XMapWindow(dpy, tmp_win->frame);
    tmp_win->mapped = TRUE;
    tmp_win->icon = FALSE;
    tmp_win->icon_on = FALSE;

    XRaiseWindow(dpy, VersionWindow);
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleUnmapNotify - UnmapNotify event handler
 *
 ***********************************************************************
 */

void
HandleUnmapNotify()
{
#ifdef DEBUG
    fprintf(stderr, "UnmapNotify\n");
#endif
    if (tmp_win == NULL)
	return;

    XUnmapWindow(dpy, tmp_win->frame);
    tmp_win->mapped = FALSE;
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleMotionNotify - MotionNotify event handler
 *
 ***********************************************************************
 */

void
HandleMotionNotify()
{
#ifdef DEBUG
    /*
    fprintf(stderr, "MotionNotify\n");
    */
#endif
    if (ConstMove)
    {
	switch (ConstMoveDir)
	{
	    case MOVE_NONE:
		if (event.xmotion.x_root < ConstMoveXL ||
		    event.xmotion.x_root > ConstMoveXR)
		    ConstMoveDir = MOVE_HORIZ;

		if (event.xmotion.y_root < ConstMoveYT ||
		    event.xmotion.y_root > ConstMoveYB)
		    ConstMoveDir = MOVE_VERT;

		XQueryPointer(dpy, DragWindow, &JunkRoot, &JunkChild,
		    &JunkX, &JunkY, &DragX, &DragY, &JunkMask);
		break;

	    case MOVE_VERT:
		ConstMoveY = event.xmotion.y_root - DragY - BorderWidth;
		break;

	    case MOVE_HORIZ:
		ConstMoveX= event.xmotion.x_root - DragX - BorderWidth;
		break;
	}

	if (ConstMoveDir != MOVE_NONE)
	{
	    int xl, yt, xr, yb, w, h;

	    xl = ConstMoveX;
	    yt = ConstMoveY;
	    w = DragWidth + 2 * BorderWidth;
	    h = DragHeight + 2 * BorderWidth;

	    if (DontMoveOff && MoveFunction != F_FORCEMOVE)
	    {
		xr = xl + w;
		yb = yt + h;

		if (xl < 0)
		    xl = 0;
		if (xr > MyDisplayWidth)
		    xl = MyDisplayWidth - w;

		if (yt < 0)
		    yt = 0;
		if (yb > MyDisplayHeight)
		    yt = MyDisplayHeight - h;
	    }
	    MoveOutline(event.xmotion.root, xl, yt, w, h);
	}
	return;
    }

    if (DragWindow != NULL)
    {
	int xl, yt, xr, yb, w, h;

	xl = event.xmotion.x_root - DragX - BorderWidth;
	yt = event.xmotion.y_root - DragY - BorderWidth;
	w = DragWidth + 2 * BorderWidth;
	h = DragHeight + 2 * BorderWidth;

	if (DontMoveOff && MoveFunction != F_FORCEMOVE)
	{
	    xr = xl + w;
	    yb = yt + h;

	    if (xl < 0)
		xl = 0;
	    if (xr > MyDisplayWidth)
		xl = MyDisplayWidth - w;

	    if (yt < 0)
		yt = 0;
	    if (yb > MyDisplayHeight)
		yt = MyDisplayHeight - h;
	}

	MoveOutline(event.xmotion.root, xl, yt, w, h);
	return;
    }

    if (ResizeWindow != NULL)
    {
	XFindContext(dpy, ResizeWindow, TwmContext, &tmp_win);
	DoResize(event.xmotion.x_root, event.xmotion.y_root, tmp_win);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleButtonRelease - ButtonRelease event handler
 *
 ***********************************************************************
 */

void
HandleButtonRelease()
{
    int xl, xr, yt, yb, w, h;

#ifdef DEBUG
    fprintf(stderr, "ButtonRelease\n");
#endif

    if (RootFunction == NULL)
    {
	XUngrabPointer(dpy, CurrentTime);
	XUngrabServer(dpy);
	EventHandler[EnterNotify] = HandleEnterNotify;
	EventHandler[LeaveNotify] = HandleLeaveNotify;
	EventHandler[Expose] = HandleExpose;
    }

    if (DragWindow != NULL)
    {
	XEvent client_event;

	MoveOutline(event.xbutton.root, 0, 0, 0, 0);

	xl = event.xbutton.x_root - DragX - BorderWidth;
	yt = event.xbutton.y_root - DragY - BorderWidth;

	if (ConstMove)
	{
	    if (ConstMoveDir == MOVE_HORIZ)
		yt = ConstMoveY;

	    if (ConstMoveDir == MOVE_VERT)
		xl = ConstMoveX;

	    if (ConstMoveDir == MOVE_NONE)
	    {
		yt = ConstMoveY;
		xl = ConstMoveX;
	    }
	}
	
	w = DragWidth + 2 * BorderWidth;
	h = DragHeight + 2 * BorderWidth;

	if (DontMoveOff && MoveFunction != F_FORCEMOVE)
	{
	    xr = xl + w;
	    yb = yt + h;

	    if (xl < 0)
		xl = 0;
	    if (xr > MyDisplayWidth)
		xl = MyDisplayWidth - w;

	    if (yt < 0)
		yt = 0;
	    if (yb > MyDisplayHeight)
		yt = MyDisplayHeight - h;
	}

	XFindContext(dpy, DragWindow, TwmContext, &tmp_win);
	if (DragWindow == tmp_win->frame)
	    SetupWindow(tmp_win, xl, yt,
		tmp_win->frame_width, tmp_win->frame_height);
	else
	    XMoveWindow(dpy, DragWindow, xl, yt);

	if (!NoRaiseMove)
	    XRaiseWindow(dpy, DragWindow);
	DragWindow = NULL;
	ConstMove = FALSE;

	enter_flag = TRUE;
	client_event.type = ClientMessage;
	client_event.xclient.message_type = NULL;
	client_event.xclient.format = 32;
	XSendEvent(dpy, tmp_win->frame, False, 0, &client_event);
	SetHints(tmp_win);

	return;
    }

    if (ResizeWindow != NULL)
    {
	EndResize();
	EventHandler[EnterNotify] = HandleEnterNotify;
	EventHandler[LeaveNotify] = HandleLeaveNotify;
	XUngrabPointer(dpy, CurrentTime);
	XUngrabServer(dpy);
	return;
    }

    if (ActiveMenu != NULL)
    {
	MenuRoot *tmp;

	for (tmp = ActiveMenu; tmp != NULL; tmp = tmp->prev)
	{
	    XUnmapWindow(dpy, tmp->shadow);
	    XUnmapWindow(dpy, tmp->w);
	}
	XFlush(dpy);
	ActiveMenu = NULL;

	if (ActiveItem != NULL)
	{
	    ExecuteFunction(ActiveItem->func, ActiveItem->action, NULL,
		ButtonWindow, ButtonEvent, Context, TRUE);
	    Action = ActiveItem->action;
	    ActiveItem = NULL;
	    Context = C_NO_CONTEXT;
	    ButtonWindow = NULL;
	}
	return;
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleButtonPress - ButtonPress event handler
 *
 ***********************************************************************
 */

void
HandleButtonPress()
{
    int modifier;

#ifdef DEBUG
    fprintf(stderr, "ButtonPress\n");
#endif

    if (ResizeWindow != NULL ||
	DragWindow != NULL  ||
	ActiveMenu != NULL)
	return;

    XUnmapWindow(dpy, VersionWindow);

    /* check the title bar buttons */

    if (tmp_win && w == tmp_win->iconify_w)
    {
	ExecuteFunction(F_ICONIFY, NULL, w, tmp_win, event, C_TITLE, FALSE);
	return;
    }

    if (tmp_win && w == tmp_win->resize_w)
    {
	ExecuteFunction(F_RESIZE, NULL, w, tmp_win, event, C_TITLE, FALSE);
	return;
    }

#ifndef NOFOCUS
    if (tmp_win && w == tmp_win->focus_w)
    {
	ExecuteFunction(F_FOCUS, NULL, w, tmp_win, event, C_TITLE, FALSE);
	return;
    }
#endif

    Context = C_NO_CONTEXT;

    if (w == Root)
	Context = C_ROOT;
    if (tmp_win)
    {
	if (w == tmp_win->title_w)
	    Context = C_TITLE;
	if (w == tmp_win->w)
	    Context = C_WINDOW;
	if (w == tmp_win->icon_w)
	    Context = C_ICON;
	if (w == tmp_win->frame)
	    Context = C_FRAME;
	if (tmp_win->list &&
	    (w == tmp_win->list->w || w == tmp_win->list->icon))
	{
	    if (tmp_win->icon)
		Context = C_ICONMGR;
	    else
	    {
		ExecuteFunction(F_ICONIFY, NULL, w, tmp_win, event,
		    C_TITLE, FALSE);
		return;
	    }
	}
    }

    /* this section of code checks to see if we were in the middle of
     * a command executed from a menu
     */
    if (RootFunction != NULL)
    {
	if (w == Root)
	{
	    /* if the window was the Root, we don't know for sure it
	     * it was the root.  We must check to see if it happened to be
	     * inside of a client that was getting button press events,
	     * such as an xterm
	     */
	    XTranslateCoordinates(dpy, Root, Root,
		event.xbutton.x, 
		event.xbutton.y, 
		&JunkX, &JunkY, &w);

	    if (w == 0 ||
		(XFindContext(dpy, w, TwmContext, &tmp_win) == XCNOENT))
	    {
		RootFunction = NULL;
		XBell(dpy, screen);
		return;
	    }

	    XTranslateCoordinates(dpy, Root, w,
		event.xbutton.x, 
		event.xbutton.y, 
		&JunkX, &JunkY, &JunkChild);

	    event.xbutton.x = JunkX;
	    event.xbutton.y = JunkY - tmp_win->title_height;
	    Context = C_WINDOW;
	}

	ExecuteFunction(RootFunction, Action, w, tmp_win, event,
	    Context, FALSE);

	RootFunction = NULL;
	return;
    }

    ButtonEvent = event;
    ButtonWindow = tmp_win;

    /* if we get to here, we have to execute a function or pop up a 
     * menu
     */
    modifier = event.xbutton.state & (ShiftMask | ControlMask | Mod1Mask);

    if (Context == C_NO_CONTEXT)
	return;

    RootFunction = NULL;
    if (Mouse[event.xbutton.button][Context][modifier].func == F_MENU)
    {
	PopUpMenu(Mouse[event.xbutton.button][Context][modifier].menu, 
	    event.xbutton.x_root, event.xbutton.y_root);
    }
    else if (Mouse[event.xbutton.button][Context][modifier].func != NULL)
    {
	Action = Mouse[event.xbutton.button][Context][modifier].item ?
	    Mouse[event.xbutton.button][Context][modifier].item->action : NULL;
	ExecuteFunction(Mouse[event.xbutton.button][Context][modifier].func,
	    Action, w, tmp_win, event, Context, FALSE);
    }
    else if (DefaultFunction.func != NULL)
    {
	if (DefaultFunction.func == F_MENU)
	{
	    PopUpMenu(DefaultFunction.menu, 
		event.xbutton.x_root, event.xbutton.y_root);
	}
	else
	{
	    Action = DefaultFunction.item ? DefaultFunction.item->action : NULL;
	    ExecuteFunction(DefaultFunction.func, Action,
	       w, tmp_win, event, Context, FALSE);
	}
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleEnterNotify - EnterNotify event handler
 *
 ***********************************************************************
 */

void
HandleEnterNotify()
{
    MenuItem *tmp;

#ifdef DEBUG
    fprintf(stderr, "EnterNotify\n");
#endif

    if (ActiveMenu == NULL && tmp_win != NULL)
    {
	if (FocusRoot && tmp_win->mapped)
	{
	    if (Focus != NULL && Focus != tmp_win)
		XUnmapWindow(dpy, Focus->hilite_w);

	    XMapWindow(dpy, tmp_win->hilite_w);
	    XInstallColormap(dpy, tmp_win->attr.colormap);
	    XSetWindowBorder(dpy, tmp_win->frame, BorderColor);
	    XSetWindowBorder(dpy, tmp_win->title_w, BorderColor);
	    if (TitleFocus)
		XSetInputFocus(dpy, tmp_win->w, RevertToPointerRoot,
			CurrentTime);
	    Focus = tmp_win;
	}
	if (enter_flag == FALSE && tmp_win->auto_raise)
	{
	    XEvent client_event;

	    XRaiseWindow(dpy, tmp_win->frame);
	    enter_flag = TRUE;
	    client_event.type = ClientMessage;
	    client_event.xclient.message_type = NULL;
	    client_event.xclient.format = 32;
	    XSendEvent(dpy, tmp_win->frame, False, 0, &client_event);
	}
	return;
    }


    if (XFindContext(dpy, w, MenuContext, &tmp) != 0)
	return;

    if (w == tmp->w && tmp->root == ActiveMenu)
    {
	if (ActiveItem != NULL && ActiveItem->state != 0)
	{
#ifdef DEBUG
	    fprintf(stderr, "turning off \"%s\"\n", ActiveItem->item);
#endif
	    XFillRectangle(dpy, ActiveItem->w,MenuXorGC,0,0,1000, 100);
	    if (tmp->pull != NULL)
		XFillRectangle(dpy, ActiveItem->pull, MenuXorGC,0,0,1000, 100);
	    ActiveItem->state = 0;
	}

	if (tmp->state == 0)
	{
#ifdef DEBUG
	    fprintf(stderr, "turning on \"%s\"\n", tmp->item);
#endif
	    XFillRectangle(dpy, tmp->w,MenuXorGC,0,0,1000, 100);
	    if (tmp->pull)
		XFillRectangle(dpy, tmp->pull, MenuXorGC,0,0,1000, 100);
	    tmp->state = 1;
	}
	ActiveItem = tmp;

	return;
    }

    if (w == tmp->pull && tmp->root == ActiveMenu)
    {
	XGrabServer(dpy);
	PopUpMenu(tmp->sub, event.xcrossing.x_root,
	    event.xcrossing.y_root);
	XUngrabServer(dpy);

	return;
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleLeaveNotify - LeaveNotify event handler
 *
 ***********************************************************************
 */

void
HandleLeaveNotify()
{
    MenuItem *tmp;

#ifdef DEBUG
    fprintf(stderr, "LeaveNotify\n");
#endif
    if (tmp_win != NULL)
    {
	XUnmapWindow(dpy, VersionWindow);
	if (FocusRoot)
	{
	    if (event.xcrossing.detail != NotifyInferior)
	    {
		XUnmapWindow(dpy, tmp_win->hilite_w);
		XUninstallColormap(dpy, tmp_win->attr.colormap);
		if (Highlight && tmp_win->highlight)
		{
		    XSetWindowBorderPixmap(dpy, tmp_win->frame, GrayTile);
		    XSetWindowBorderPixmap(dpy, tmp_win->title_w, GrayTile);
		}
		if (TitleFocus)
		    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot,
			    CurrentTime);
		Focus = NULL;
	    }
	}
	return;
    }

    if (XFindContext(dpy, w, MenuContext, &tmp) != 0)
	return;

    if (w == tmp->root->w)
    {
	int rootx, rooty, x, y;
	int wx, wy, ww, wh;

	/* see if the mouse really left the window
	 * or just crossed into a sub-window
	 */

	XQueryPointer(dpy, w, &JunkRoot,
	    &JunkChild, &rootx, &rooty, &x, &y, &JunkMask);

	XGetGeometry(dpy, w, &JunkRoot, &wx, &wy,
	    &ww, &wh, &JunkBW,
	    &JunkDepth);
	
	if (rootx < wx ||
	    rootx > (wx + ww) ||
	    rooty < wy ||
	    rooty > (wy + wh))
	{
	    ActiveItem = NULL;
	    if (tmp->root->prev != NULL)
	    {
		if (ActiveMenu == tmp->root)
		{
		    XUnmapWindow(dpy, ActiveMenu->shadow);
		    XUnmapWindow(dpy, ActiveMenu->w);
		    ActiveMenu = tmp->root->prev;
		}
	    }
	}
	return;
    }

    if (w == tmp->w)
    {
	if (tmp == ActiveItem)
	    ActiveItem = NULL;

	if (tmp->state != 0)
	{
#ifdef DEBUG
	    fprintf(stderr, "turning off \"%s\"\n", tmp->item);
#endif
	    XFillRectangle(dpy, tmp->w,MenuXorGC,0,0,1000, 100);
	    if (tmp->pull != NULL)
		XFillRectangle(dpy, tmp->pull, MenuXorGC,0,0,1000, 100);
	    tmp->state = 0;
	}

	return;
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleConfigureRequest - ConfigureRequest event handler
 *
 ***********************************************************************
 */

void
HandleConfigureRequest()
{
    XWindowChanges xwc;
    unsigned int   xwcm;

#ifdef DEBUG
    fprintf(stderr, "ConfigureRequest\n");
    if (event.xconfigurerequest.value_mask & CWX)
	fprintf(stderr, "  x = %d\n", event.xconfigurerequest.x);
    if (event.xconfigurerequest.value_mask & CWY)
	fprintf(stderr, "  y = %d\n", event.xconfigurerequest.y);
    if (event.xconfigurerequest.value_mask & CWWidth)
	fprintf(stderr, "  width = %d\n", event.xconfigurerequest.width);
    if (event.xconfigurerequest.value_mask & CWHeight)
	fprintf(stderr, "  height = %d\n", event.xconfigurerequest.height);
    if (event.xconfigurerequest.value_mask & CWSibling)
	fprintf(stderr, "  above = 0x%x\n", event.xconfigurerequest.above);
    if (event.xconfigurerequest.value_mask & CWStackMode)
	fprintf(stderr, "  stack = %d\n", event.xconfigurerequest.detail);
#endif

    w = event.xconfigurerequest.window;

    if (tmp_win == NULL && Transient(w))
    {
#ifdef DEBUG
	fprintf(stderr, "  Transient\n");
#endif

	xwcm = event.xconfigurerequest.value_mask & 
	    (CWX | CWY | CWWidth | CWHeight);
	xwc.x = event.xconfigurerequest.x;
	xwc.y = event.xconfigurerequest.y;
	xwc.width = event.xconfigurerequest.width;
	xwc.height = event.xconfigurerequest.height;
	XConfigureWindow(dpy, w, xwcm, &xwc);
	return;
    }

    if (tmp_win == NULL)
	return;


    if (event.xconfigurerequest.value_mask & CWStackMode)
    {
	if (event.xconfigurerequest.detail == Above)
	    XRaiseWindow(dpy, tmp_win->frame);
	else if (event.xconfigurerequest.detail == Below)
	    XLowerWindow(dpy, tmp_win->frame);

	return;
    }

    if (event.xconfigurerequest.value_mask & CWX)
	tmp_win->frame_x = event.xconfigurerequest.x - tmp_win->title_height;
    if (event.xconfigurerequest.value_mask & CWY)
	tmp_win->frame_y = event.xconfigurerequest.y;
    if (event.xconfigurerequest.value_mask & CWWidth)
	tmp_win->frame_width = event.xconfigurerequest.width;
    if (event.xconfigurerequest.value_mask & CWHeight)
	tmp_win->frame_height =
	    event.xconfigurerequest.height + tmp_win->title_height;

    SetupWindow(tmp_win,
	tmp_win->frame_x, tmp_win->frame_y,
	tmp_win->frame_width, tmp_win->frame_height);
}

/***********************************************************************
 *
 *  Procedure:
 *	HandleUnknown - unknown event handler
 *
 ***********************************************************************
 */

void
HandleUnknown()
{
#ifdef DEBUG
    fprintf(stderr, "type = %d\n", event.type);
#endif
}

/***********************************************************************
 *
 *  Procedure:
 *	Transient - checks to see if the window is a transient
 *
 *  Returned Value:
 *	TRUE	- window is a transient
 *	FALSE	- window is not a transient
 *
 *  Inputs:
 *	w	- the window to check
 *
 ***********************************************************************
 */

int
Transient(w)
    Window w;
{
    Window propw;

    return (XGetTransientForHint(dpy, w, &propw));
}
