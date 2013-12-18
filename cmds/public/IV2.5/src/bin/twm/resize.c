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
 * $Header: resize.c,v 1.21.1.1 89/03/20 13:51:58 interran Exp $
 *
 * window resizing borrowed from the "wm" window manager
 *
 * 11-Dec-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$Header: resize.c,v 1.21.1.1 89/03/20 13:51:58 interran Exp $";
#endif

#include <stdio.h>
#include "twm.h"
#include "util.h"
#include "resize.h"
#include "add_window.h"
#include "resize.bm"
#ifndef NOFOCUS
#include "focus.bm"
#else
#define focus_width 0
#endif

#define MINHEIGHT 32
#define MINWIDTH 60

static int dragx;	/* all these variables are used */
static int dragy;	/* in resize operations */
static int dragWidth;
static int dragHeight;

static int origx;
static int origy;
static int origWidth;
static int origHeight;

static int clampTop;
static int clampBottom;
static int clampLeft;
static int clampRight;

static int last_width;
static int last_height;

/***********************************************************************
 *
 *  Procedure:
 *	StartResize - begin a window resize operation
 *
 *  Inputs:
 *	ev	- the event structure (button press)
 *	tmp_win	- the TwmWindow pointer
 *
 ***********************************************************************
 */

void
StartResize(ev, tmp_win)
XEvent ev;
TwmWindow *tmp_win;
{
    Window      junkRoot;
    int         junkbw, junkDepth;

    ResizeWindow = tmp_win->frame;
    XGrabServer(dpy);
    XGrabPointer(dpy, ev.xbutton.root, True,
	ButtonReleaseMask,
	GrabModeAsync, GrabModeAsync,
	Root, MoveCursor, CurrentTime);

    XGetGeometry(dpy, (Drawable) tmp_win->frame, &junkRoot,
	&dragx, &dragy, &dragWidth, &dragHeight, &junkbw,
		 &junkDepth);
    dragx += BorderWidth;
    dragy += BorderWidth;
    origx = dragx;
    origy = dragy;
    origWidth = dragWidth;
    origHeight = dragHeight;
    clampTop = clampBottom = clampLeft = clampRight = 0;

    XMoveWindow(dpy, SizeWindow, 0, 0);
    XMapRaised(dpy, SizeWindow);
    last_width = 0;
    last_height = 0;
    DisplaySize(tmp_win, origWidth, origHeight);
}

/***********************************************************************
 *
 *  Procedure:
 *	AddStartResize - begin a window resize operation from AddWindow
 *
 *  Inputs:
 *	tmp_win	- the TwmWindow pointer
 *
 ***********************************************************************
 */

void
AddStartResize(tmp_win, x, y, w, h)
TwmWindow *tmp_win;
int x, y, w, h;
{
    Window      junkRoot;
    int         junkbw, junkDepth;

    XGrabServer(dpy);
    XGrabPointer(dpy, Root, True,
	ButtonReleaseMask,
	GrabModeAsync, GrabModeAsync,
	Root, MoveCursor, CurrentTime);

    dragx = x + BorderWidth;
    dragy = y + BorderWidth;
    origx = dragx;
    origy = dragy;
    dragWidth = origWidth = w - 2 * BorderWidth;
    dragHeight = origHeight = h - 2 * BorderWidth;
    clampTop = clampBottom = clampLeft = clampRight = 0;

    XMoveWindow(dpy, SizeWindow, 0, InitialFont.height + 4 + BW);
    XMapRaised(dpy, SizeWindow);
    last_width = 0;
    last_height = 0;
    DisplaySize(tmp_win, origWidth, origHeight);
}

/***********************************************************************
 *
 *  Procedure:
 *	DoResize - move the rubberband around.  This is called for
 *		   each motion event when we are resizing 
 *
 *  Inputs:
 *	x_root	- the X corrdinate in the root window
 *	y_root	- the Y corrdinate in the root window
 *	tmp_win	- the current twm window
 *
 ***********************************************************************
 */

void
DoResize(x_root, y_root, tmp_win)
int x_root;
int y_root;
TwmWindow *tmp_win;
{
    int action;

    action = 0;

    if (clampTop) {
	int         delta = y_root - dragy;
	if (dragHeight - delta < MINHEIGHT) {
	    delta = dragHeight - MINHEIGHT;
	    clampTop = 0;
	}
	dragy += delta;
	dragHeight -= delta;
	action = 1;
    }
    else if (y_root <= dragy/* ||
	     y_root == findRootInfo(root)->rooty*/) {
	dragy = y_root;
	dragHeight = origy + origHeight -
	    y_root;
	clampBottom = 0;
	clampTop = 1;
	action = 1;
    }
    if (clampLeft) {
	int         delta = x_root - dragx;
	if (dragWidth - delta < MINWIDTH) {
	    delta = dragWidth - MINWIDTH;
	    clampLeft = 0;
	}
	dragx += delta;
	dragWidth -= delta;
	action = 1;
    }
    else if (x_root <= dragx/* ||
	     x_root == findRootInfo(root)->rootx*/) {
	dragx = x_root;
	dragWidth = origx + origWidth -
	    x_root;
	clampRight = 0;
	clampLeft = 1;
	action = 1;
    }
    if (clampBottom) {
	int         delta = y_root - dragy - dragHeight;
	if (dragHeight + delta < MINHEIGHT) {
	    delta = MINHEIGHT - dragHeight;
	    clampBottom = 0;
	}
	dragHeight += delta;
	action = 1;
    }
    else if (y_root >= dragy + dragHeight - 1/* ||
	   y_root == findRootInfo(root)->rooty
	   + findRootInfo(root)->rootheight - 1*/) {
	dragy = origy;
	dragHeight = 1 + y_root - dragy;
	clampTop = 0;
	clampBottom = 1;
	action = 1;
    }
    if (clampRight) {
	int         delta = x_root - dragx - dragWidth;
	if (dragWidth + delta < MINWIDTH) {
	    delta = MINWIDTH - dragWidth;
	    clampRight = 0;
	}
	dragWidth += delta;
	action = 1;
    }
    else if (x_root >= dragx + dragWidth - 1/* ||
	     x_root == findRootInfo(root)->rootx +
	     findRootInfo(root)->rootwidth - 1*/) {
	dragx = origx;
	dragWidth = 1 + x_root - origx;
	clampLeft = 0;
	clampRight = 1;
	action = 1;
    }
    if (action) {
	MoveOutline(Root,
		    dragx - BorderWidth,
		    dragy - BorderWidth,
		    dragWidth + 2 * BorderWidth,
		    dragHeight + 2 * BorderWidth);
    }

    DisplaySize(tmp_win, dragWidth, dragHeight);
}

/***********************************************************************
 *
 *  Procedure:
 *	DisplaySize - display the size in the dimensions window
 *
 *  Inputs:
 *	tmp_win - the current twm window 
 *	width	- the width of the rubber band
 *	height	- the height of the rubber band
 *
 ***********************************************************************
 */

void
DisplaySize(tmp_win, width, height)
TwmWindow *tmp_win;
int width;
int height;
{
    char str[100];
    int dwidth;
    int dheight;

    if (last_width == width && last_height == height)
	return;

    last_width = width;
    last_height = height;

    dwidth = width;
    dheight = height - tmp_win->title_height;

    if (tmp_win->hints.flags&PMinSize && tmp_win->hints.flags & PResizeInc)
    {
	dwidth -= tmp_win->hints.min_width;
	dheight -= tmp_win->hints.min_height;
    }

    if (tmp_win->hints.flags & PResizeInc)
    {
	dwidth /= tmp_win->hints.width_inc;
	dheight /= tmp_win->hints.height_inc;
    }

    sprintf(str, "%d x %d", dwidth, dheight);

    width = XTextWidth(SizeFont.font, str, strlen(str)) + 20;
    strcat(str, "        ");
    XResizeWindow(dpy, SizeWindow, width, SizeFont.height + 4);
    XRaiseWindow(dpy, SizeWindow);
    XDrawImageString(dpy, SizeWindow, SizeNormalGC,
	10, 2 + SizeFont.font->ascent, str, strlen(str));
}

/***********************************************************************
 *
 *  Procedure:
 *	EndResize - finish the resize operation
 *
 ***********************************************************************
 */

void
EndResize()
{
    TwmWindow *tmp_win;
    Window w;

#ifdef DEBUG
    fprintf(stderr, "EndResize\n");
#endif

    XUnmapWindow(dpy, SizeWindow);
    MoveOutline(Root, 0, 0, 0, 0);

    XFindContext(dpy, ResizeWindow, TwmContext, &tmp_win);

    dragHeight = dragHeight - tmp_win->title_height;

    if (tmp_win->hints.flags&PMinSize && tmp_win->hints.flags & PResizeInc)
    {
	dragWidth -= tmp_win->hints.min_width;
	dragHeight -= tmp_win->hints.min_height;
	if (dragWidth < 0)
	    dragWidth = 0;
	if (dragHeight < 0)
	    dragHeight = 0;
    }

    if (tmp_win->hints.flags & PResizeInc)
    {
	dragWidth /= tmp_win->hints.width_inc;
	dragHeight /= tmp_win->hints.height_inc;

	dragWidth *= tmp_win->hints.width_inc;
	dragHeight *= tmp_win->hints.height_inc;
    }

    if (tmp_win->hints.flags&PMinSize && tmp_win->hints.flags & PResizeInc)
    {
	dragWidth += tmp_win->hints.min_width;
	dragHeight += tmp_win->hints.min_height;
    }

    if (tmp_win->hints.flags&PMinSize)
    {
	if (dragWidth < tmp_win->hints.min_width)
	    dragWidth = tmp_win->hints.min_width;
	if (dragHeight < tmp_win->hints.min_height)
	    dragHeight = tmp_win->hints.min_height;
    }
    if (tmp_win->hints.flags&PMaxSize)
    {	
	if (dragWidth > tmp_win->hints.max_width)
	    dragWidth = tmp_win->hints.max_width;
	if (dragHeight > tmp_win->hints.max_height)
	    dragHeight = tmp_win->hints.max_height;
    }

    dragHeight = dragHeight + tmp_win->title_height;

    SetupWindow(tmp_win,
	dragx - BorderWidth,
	dragy - BorderWidth,
	dragWidth, dragHeight);

#ifdef SUN386
    /* This is a kludge to fix a problem in the Sun 386 server which
     * causes windows to not be repainted after a resize operation.
     */
    w = XCreateSimpleWindow(dpy, tmp_win->frame,
	0, 0, 9999, 9999, 0, Black, Black);

    XMapWindow(dpy, w);
    XDestroyWindow(dpy, w);
    XFlush(dpy);
#endif
 
    if (!NoRaiseResize)
	XRaiseWindow(dpy, tmp_win->frame);

    ResizeWindow = NULL;
    SetHints(tmp_win);
}

/***********************************************************************
 *
 *  Procedure:
 *	AddEndResize - finish the resize operation for AddWindow
 *
 ***********************************************************************
 */

void
AddEndResize(tmp_win)
TwmWindow *tmp_win;
{

#ifdef DEBUG
    fprintf(stderr, "AddEndResize\n");
#endif

    XUnmapWindow(dpy, SizeWindow);

    dragHeight = dragHeight - tmp_win->title_height;

    if (tmp_win->hints.flags&PMinSize && tmp_win->hints.flags & PResizeInc)
    {
	dragWidth -= tmp_win->hints.min_width;
	dragHeight -= tmp_win->hints.min_height;
	if (dragWidth < 0)
	    dragWidth = 0;
	if (dragHeight < 0)
	    dragHeight = 0;
    }

    if (tmp_win->hints.flags & PResizeInc)
    {
	dragWidth /= tmp_win->hints.width_inc;
	dragHeight /= tmp_win->hints.height_inc;

	dragWidth *= tmp_win->hints.width_inc;
	dragHeight *= tmp_win->hints.height_inc;
    }

    if (tmp_win->hints.flags&PMinSize && tmp_win->hints.flags & PResizeInc)
    {
	dragWidth += tmp_win->hints.min_width;
	dragHeight += tmp_win->hints.min_height;
    }

    if (tmp_win->hints.flags&PMinSize)
    {
	if (dragWidth < tmp_win->hints.min_width)
	    dragWidth = tmp_win->hints.min_width;
	if (dragHeight < tmp_win->hints.min_height)
	    dragHeight = tmp_win->hints.min_height;
    }
    if (tmp_win->hints.flags&PMaxSize)
    {	
	if (dragWidth > tmp_win->hints.max_width)
	    dragWidth = tmp_win->hints.max_width;
	if (dragHeight > tmp_win->hints.max_height)
	    dragHeight = tmp_win->hints.max_height;
    }

    AddingX = dragx;
    AddingY = dragy;
    AddingW = dragWidth + (2 * BorderWidth);
    AddingH = dragHeight + tmp_win->title_height + (2 * BorderWidth);
}

/***********************************************************************
 *
 *  Procedure:
 *	SetupWindow - set window sizes, this was called from either
 *		AddWindow, EndResize, or HandleConfigureNotify.
 *
 *  Inputs:
 *	tmp_win	- the TwmWindow pointer
 *	x	- the x coordinate of the frame window
 *	y	- the y coordinate of the frame window
 *	w	- the width of the frame window
 *	h	- the height of the frame window
 *
 *  Special Considerations:
 *	This routine will check to make sure the window is not completely 
 *	off the display, if it is, it'll bring some of it back on.
 *
 ***********************************************************************
 */

void
SetupWindow(tmp_win, x, y, w, h)
TwmWindow *tmp_win;
int x, y, w, h;
{
    XEvent client_event;
    XWindowChanges xwc;
    unsigned int   xwcm;
    int width;

#ifdef DEBUG
    fprintf(stderr, "SetupWindow: x=%d, y=%d, w=%d, h=%d\n",
	x, y, w, h);
#endif

    if (x > MyDisplayWidth)
	x = MyDisplayWidth - 64;
    if (y > MyDisplayHeight)
	y = MyDisplayHeight - 64;

    if (tmp_win == IconManagerPtr)
    {
	IconManagerWidth = w;
	h = IconManagerHeight + tmp_win->title_height;
    }

    tmp_win->frame_x = x;
    tmp_win->frame_y = y;
    tmp_win->frame_width = w;
    tmp_win->frame_height = h;

    XMoveResizeWindow(dpy, tmp_win->frame, x, y, w, h);

    xwcm = CWWidth;
    xwc.width = w;
    XConfigureWindow(dpy, tmp_win->title_w, xwcm, &xwc);

    tmp_win->attr.width = w;
    tmp_win->attr.height = h - tmp_win->title_height;

    XMoveResizeWindow(dpy, tmp_win->w, 0, tmp_win->title_height, w, 
	h - tmp_win->title_height);

    xwcm = CWX;
    xwc.x = w - resize_width - 1;
    XConfigureWindow(dpy, tmp_win->resize_w, xwcm, &xwc);

    xwc.x = w - resize_width - focus_width - 3;
#ifndef NOFOCUS
    XConfigureWindow(dpy, tmp_win->focus_w, xwcm, &xwc);
#endif

    width = w - TitleBarX - focus_width - resize_width - 5 -
	tmp_win->name_width - 10;

    if (width <= 0)
    {
	xwc.x = MyDisplayWidth;
	xwc.width = 1;
    }
    else
    {
	xwc.x = TitleBarX + tmp_win->name_width + 6;
	xwc.width = width;
    }

    xwcm = CWX | CWWidth;
    XConfigureWindow(dpy, tmp_win->hilite_w, xwcm, &xwc);

    client_event.type = ConfigureNotify;
    client_event.xconfigure.display = dpy;
    client_event.xconfigure.event = tmp_win->w;
    client_event.xconfigure.window = tmp_win->w;
    client_event.xconfigure.x = x;
    client_event.xconfigure.y = y + tmp_win->title_height;
    client_event.xconfigure.width = tmp_win->frame_width;
    client_event.xconfigure.height = tmp_win->frame_height -
	tmp_win->title_height;
    client_event.xconfigure.border_width = BorderWidth;
    XSendEvent(dpy, tmp_win->w, False,
	StructureNotifyMask, &client_event);
}

/***********************************************************************
 *
 *  Procedure:
 *	SetHints - set window hints so that if twm is killed the windows
 *		will start up in the same places they were at when twm was
 *		killed.
 *
 *  Inputs:
 *	tmp_win	- the TwmWindow pointer
 *
 ***********************************************************************
 */

void
SetHints(tmp_win)
TwmWindow *tmp_win;
{
    XWMHints wmhints;
    XSizeHints hints;
    int x, y, w, h;

    /*
    wmhints = *(tmp_win->wmhints);

    if (tmp_win->icon)
	wmhints.initial_state = IconicState;
    else
	wmhints.initial_state = NormalState;

    XGetGeometry(dpy, tmp_win->icon_w, &JunkRoot, &x, &y, &w, &h,
	&JunkBW, &JunkDepth);
    wmhints.icon_x = x;
    wmhints.icon_y = y;

    wmhints.flags |= (StateHint | IconPositionHint);
    XSetWMHints(dpy, tmp_win->w, &wmhints);
    */

    XGetGeometry(dpy, tmp_win->frame, &JunkRoot, &x, &y, &w, &h,
	&JunkBW, &JunkDepth);
    hints = tmp_win->hints;
    hints.x = x;
    hints.y = y + tmp_win->title_height;
    hints.width = w;
    hints.height = h - tmp_win->title_height;

    hints.flags |= (USPosition | USSize);
    XSetNormalHints(dpy, tmp_win->w, &hints);
}

/**********************************************************************
 *  Rutgers mod #1   - rocky.
 *  Procedure:  
 *         fullzoom - zooms window to full height of screen or
 *                    to full height and width of screen. (Toggles
 *                    so that it can undo the zoom - even when switching
 *                    between fullzoom and vertical zoom.)
 *  
 *  Inputs:
 *         tmp_win - the TwmWindow pointer
 *
 *
 **********************************************************************
 */


void
fullzoom(tmp_win,flag)
TwmWindow *tmp_win;
int flag;
{
    Window      junkRoot;
    int         junkbw, junkDepth;
    TwmWindow   *test_win;

      XGetGeometry(dpy, (Drawable) tmp_win->frame, &junkRoot,
		   &dragx, &dragy, &dragWidth, &dragHeight, &junkbw,
		   &junkDepth);
      dragx += BorderWidth;
      dragy += BorderWidth;

    if (tmp_win->zoomed == flag) {
      dragHeight = tmp_win->save_frame_height;
      dragWidth = tmp_win->save_frame_width;
      dragx = tmp_win->save_frame_x;
      dragy = tmp_win->save_frame_y;
      tmp_win->zoomed = ZOOM_NONE;
    }
    else if (tmp_win->zoomed == ZOOM_VERT && flag == ZOOM_FULL)
      { dragHeight = MyDisplayHeight - 2*BorderWidth;
	dragWidth = MyDisplayWidth - 2*BorderWidth;
	dragx = 0;
	dragy = 0;
	tmp_win->zoomed = ZOOM_FULL;
      }
    else if (tmp_win->zoomed == ZOOM_FULL && flag == ZOOM_VERT)
      { dragHeight = MyDisplayHeight - 2*BorderWidth;
	dragy = 0;
	dragx = tmp_win->save_frame_x;
	dragWidth = tmp_win->save_frame_width;
	tmp_win->zoomed = ZOOM_VERT;
      }
    else     if (flag == ZOOM_VERT)
      {      
	tmp_win->save_frame_x = dragx;
	tmp_win->save_frame_y = dragy;
	tmp_win->save_frame_width = dragWidth;
	tmp_win->save_frame_height = dragHeight;
	tmp_win->zoomed = ZOOM_VERT;
	dragHeight = MyDisplayHeight - 2*BorderWidth;
	dragy=0;
	
      }
    else if (flag == ZOOM_FULL)
      {
	tmp_win->save_frame_x = dragx;
	tmp_win->save_frame_y = dragy;
	tmp_win->save_frame_width = dragWidth;
	tmp_win->save_frame_height = dragHeight;
	tmp_win->zoomed = ZOOM_FULL;
	dragx = 0;
	dragy = 0;
	dragHeight = MyDisplayHeight - 2*BorderWidth;
	dragWidth = MyDisplayWidth - 2*BorderWidth;
      }
    else {
      fprintf(stderr, "flag for zooming/unzooming is not valid!\n");
    }
    if (!NoRaiseResize)
	XRaiseWindow(dpy, tmp_win->frame);

    dragHeight = dragHeight - tmp_win->title_height;

    if (tmp_win->hints.flags&PMinSize && tmp_win->hints.flags & PResizeInc)
    {
	dragWidth -= tmp_win->hints.min_width;
	dragHeight -= tmp_win->hints.min_height;
    }

    if (tmp_win->hints.flags & PResizeInc)
    {
	dragWidth /= tmp_win->hints.width_inc;
	dragHeight /= tmp_win->hints.height_inc;

	dragWidth *= tmp_win->hints.width_inc;
	dragHeight *= tmp_win->hints.height_inc;
    }

    if (tmp_win->hints.flags&PMinSize && tmp_win->hints.flags & PResizeInc)
    {
	dragWidth += tmp_win->hints.min_width;
	dragHeight += tmp_win->hints.min_height;
    }

    dragHeight = dragHeight + tmp_win->title_height;

    SetupWindow(tmp_win, dragx , dragy , dragWidth, dragHeight);
    SetHints(tmp_win); 
    XUngrabPointer(dpy, CurrentTime);
    XUngrabServer(dpy);
}
