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
 * $Header: util.c,v 1.2 88/10/15 19:12:02 jim Exp $
 *
 * utility routines for twm
 *
 * 28-Oct-87 Thomas E. LaStrange	File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$Header: util.c,v 1.2 88/10/15 19:12:02 jim Exp $";
#endif

#include <stdio.h>
#include "twm.h"
#include "util.h"
#include "gram.h"
#include "iconify.bm"

int ZoomCount = 8;		/* number of outlines to draw while zooming */

typedef struct ListRoot
{
    struct WList *first;
    struct WList *last;
    int count;
} ListRoot;

int IconManagerWidth = 0;
int IconManagerHeight = 0;
int IconManagerX = iconify_width + 6;
int IconManagerY = 0;
static ListRoot WListRoot = { NULL, NULL, 0 };
static Pixmap Pm = NULL;

WList *
AddIconManager(tmp_win)
TwmWindow *tmp_win;
{
    WList *tmp;
    int h;
    unsigned long valuemask;		/* mask for create windows */
    XSetWindowAttributes attributes;	/* attributes for create windows */

    if (tmp_win == IconManagerPtr)
	return NULL;

    if (LookInList(ICONMGR_NOSHOW, tmp_win->full_name, &tmp_win->class))
	return NULL;

    tmp = (WList *) malloc(sizeof(WList));

    if (WListRoot.first == NULL)
    {
	WListRoot.first = tmp;
	tmp->prev = NULL;
    }
    else
    {
	WListRoot.last->next = tmp;
	tmp->prev = WListRoot.last;
    }
    WListRoot.last = tmp;
    tmp->next = NULL;

    tmp->twm = tmp_win;

    h = IconManagerFont.height + 4;
    if (h < (iconify_height + 2))
	h = iconify_height + 2;

    IconManagerHeight = h * WListRoot.count;
    tmp->w = XCreateSimpleWindow(dpy, IconManager,
	0, IconManagerHeight, 1000, h, 0,
	IconManagerC.fore, IconManagerC.back);
    XSelectInput(dpy, tmp->w, ButtonPressMask | ExposureMask);
    XMapWindow(dpy, tmp->w);
    
    if (Pm == NULL)
    {
	Pm = MakePixmap(tmp->w, IconManagerGC,
	    iconify_bits, iconify_width, iconify_height);
    }
    valuemask = CWEventMask | CWBackPixmap;
    attributes.background_pixmap = Pm;
    attributes.event_mask = ButtonPressMask;

    tmp->icon = XCreateWindow(dpy, tmp->w,
	1, (h - iconify_height)/2,
	iconify_width, iconify_height,
	0, d_depth, CopyFromParent,
	d_visual, valuemask, &attributes);

    XDefineCursor(dpy, tmp->icon, ButtonCursor);

    WListRoot.count += 1;
    IconManagerHeight = h * WListRoot.count;

    XResizeWindow(dpy, IconManager, IconManagerWidth, IconManagerHeight);
    if (IconManagerPtr != NULL)
	SetupWindow(IconManagerPtr,
	    IconManagerPtr->frame_x,
	    IconManagerPtr->frame_y,
	    IconManagerPtr->frame_width,
	    (h * WListRoot.count) + IconManagerPtr->title_height);
    XSaveContext(dpy, tmp->w, IconManagerContext, tmp);

    return (tmp);
}

RemoveIconManager(tmp_win)
TwmWindow *tmp_win;
{
    WList *tmp;
    int h, i;

    if (tmp_win->list == NULL)
	return;

    tmp = tmp_win->list;
    if (tmp->prev == NULL)
	WListRoot.first = tmp->next;
    else
	tmp->prev->next = tmp->next;

    if (tmp->next == NULL)
	WListRoot.last = tmp->prev;
    else
	tmp->next->prev = tmp->prev;
    
    XDeleteContext(dpy, tmp->w, IconManagerContext);
    XDestroyWindow(dpy, tmp->w);
    XDestroyWindow(dpy, tmp->icon);
    WListRoot.count -= 1;
    free(tmp);

    h = IconManagerFont.height + 4;
    IconManagerHeight = h * WListRoot.count;
    if (h < (iconify_height + 2))
	h = iconify_height + 2;

    for (i = 0, tmp = WListRoot.first; tmp != NULL; i++, tmp = tmp->next)
    {
	XMoveWindow(dpy, tmp->w, 0, i * h);
    }

    if (IconManagerPtr != NULL)
	SetupWindow(IconManagerPtr,
	    IconManagerPtr->frame_x,
	    IconManagerPtr->frame_y,
	    IconManagerPtr->frame_width,
	    IconManagerHeight + IconManagerPtr->title_height);
}

/***********************************************************************
 *
 *  Procedure:
 *	MoveOutline - move a window outline
 *
 *  Inputs:
 *	root	    - the window we are outlining
 *	x	    - upper left x coordinate
 *	y	    - upper left y coordinate
 *	width	    - the width of the rectangle
 *	height	    - the height of the rectangle
 *
 ***********************************************************************
 */

void
MoveOutline(root, x, y, width, height)
    Window root;
    int x, y, width, height;
{
    static int	lastx = 0;
    static int	lasty = 0;
    static int	lastWidth = 0;
    static int	lastHeight = 0;
    int		xl, xr, yt, yb;
    int		xthird, ythird;
    XSegment	outline[16];
    XSegment	*r = outline;

    if (x == lastx && y == lasty && width == lastWidth && height == lastHeight)
	return;
    
    xthird = lastWidth/3;
    ythird = lastHeight/3;
    xl = lastx;
    xr = lastx + lastWidth - 1;
    yt = lasty;
    yb = lasty + lastHeight - 1;

    if (lastWidth || lastHeight)
    {
	r->x1 = xl;
	r->y1 = yt;
	r->x2 = xr;
	r++->y2 = yt;

	r->x1 = xl;
	r->y1 = yb;
	r->x2 = xr;
	r++->y2 = yb;

	r->x1 = xl;
	r->y1 = yt;
	r->x2 = xl;
	r++->y2 = yb;

	r->x1 = xr;
	r->y1 = yt;
	r->x2 = xr;
	r++->y2 = yb;

	r->x1 = xl + xthird;
	r->y1 = yt;
	r->x2 = r->x1;
	r++->y2 = yb;

	r->x1 = xl + (2 * xthird);
	r->y1 = yt;
	r->x2 = r->x1;
	r++->y2 = yb;

	r->x1 = xl;
	r->y1 = yt + ythird;
	r->x2 = xr;
	r->y2 = r->y1;
	r++;

	r->x1 = xl;
	r->y1 = yt + (2 * ythird);
	r->x2 = xr;
	r->y2 = r->y1;
	r++;
    }

    lastx = x;
    lasty = y;
    lastWidth = width;
    lastHeight = height;
    xthird = lastWidth/3;
    ythird = lastHeight/3;
    xl = lastx;
    xr = lastx + lastWidth - 1;
    yt = lasty;
    yb = lasty + lastHeight - 1;

    if (lastWidth || lastHeight)
    {
	r->x1 = xl;
	r->y1 = yt;
	r->x2 = xr;
	r++->y2 = yt;

	r->x1 = xl;
	r->y1 = yb;
	r->x2 = xr;
	r++->y2 = yb;

	r->x1 = xl;
	r->y1 = yt;
	r->x2 = xl;
	r++->y2 = yb;

	r->x1 = xr;
	r->y1 = yt;
	r->x2 = xr;
	r++->y2 = yb;

	r->x1 = xl + xthird;
	r->y1 = yt;
	r->x2 = r->x1;
	r++->y2 = yb;

	r->x1 = xl + (2 * xthird);
	r->y1 = yt;
	r->x2 = r->x1;
	r++->y2 = yb;

	r->x1 = xl;
	r->y1 = yt + ythird;
	r->x2 = xr;
	r->y2 = r->y1;
	r++;

	r->x1 = xl;
	r->y1 = yt + (2 * ythird);
	r->x2 = xr;
	r->y2 = r->y1;
	r++;
    }
    if (r != outline)
    {
	XDrawSegments(dpy, root, DrawGC, outline, r - outline);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	Zoom - zoom in or out of an icon
 *
 *  Inputs:
 *	wf	- window to zoom from
 *	wt	- window to zoom to
 *
 ***********************************************************************
 */

void
Zoom(wf, wt)
{
    int fx, fy, fw, fh;
    int tx, ty, tw, th;
    int xl, yt, xr, yb;
    int dx, dy, dw, dh;
    int w, h, i;
    XSegment	outline[4];

    if (!DoZoom)
	return;

    XGetGeometry(dpy, wf, &JunkRoot, &fx, &fy, &fw, &fh, &JunkBW, &JunkDepth);
    XGetGeometry(dpy, wt, &JunkRoot, &tx, &ty, &tw, &th, &JunkBW, &JunkDepth);

    dx = (tx - fx) / ZoomCount;
    dy = (ty - fy) / ZoomCount;
    dw = (tw - fw) / ZoomCount;
    dh = (th - fh) / ZoomCount;

    xl = fx;
    yt = fy;
    xr = fx + fw;
    yb = fy + fh;
    w = fw;
    h = fh;

    for (i = 0; i < ZoomCount; i++)
    {
	outline[0].x1 = xl;
	outline[0].y1 = yt;
	outline[0].x2 = xr;
	outline[0].y2 = yt;

	outline[1].x1 = xr;
	outline[1].y1 = yt;
	outline[1].x2 = xr;
	outline[1].y2 = yb;

	outline[2].x1 = xr;
	outline[2].y1 = yb;
	outline[2].x2 = xl;
	outline[2].y2 = yb;

	outline[3].x1 = xl;
	outline[3].y1 = yb;
	outline[3].x2 = xl;
	outline[3].y2 = yt;

	XDrawSegments(dpy, Root, DrawGC, outline, 4);

	w += dw;
	h += dh;
	xl += dx;
	yt += dy;
	xr = xl + w;
	yb = yt + h;
    }

    xl = fx;
    yt = fy;
    xr = fx + fw;
    yb = fy + fh;
    w = fw;
    h = fh;

    for (i = 0; i < ZoomCount; i++)
    {
	outline[0].x1 = xl;
	outline[0].y1 = yt;
	outline[0].x2 = xr;
	outline[0].y2 = yt;

	outline[1].x1 = xr;
	outline[1].y1 = yt;
	outline[1].x2 = xr;
	outline[1].y2 = yb;

	outline[2].x1 = xr;
	outline[2].y1 = yb;
	outline[2].x2 = xl;
	outline[2].y2 = yb;

	outline[3].x1 = xl;
	outline[3].y1 = yb;
	outline[3].x2 = xl;
	outline[3].y2 = yt;

	XDrawSegments(dpy, Root, DrawGC, outline, 4);

	w += dw;
	h += dh;
	xl += dx;
	yt += dy;
	xr = xl + w;
	yb = yt + h;
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	MakeCenteredPixmap - make a pixmap centered in a space
 *
 *  Returned Value:
 *	pid	- the pixmap id
 *
 *  Inputs:
 *	w	- the window to associate the pixmap with
 *	gc	- the graphics context to use
 *	width	- the width of the pixmap to create
 *	height  - the height of the pixmap to create
 *	data	- pointer to the pixmap data
 *	pwidth	- the width of the pixmap
 *	pheight	- the height of the pixmap
 *
 ***********************************************************************
 */

Pixmap
MakeCenteredPixmap(w, gc, width, height, data, pwidth, pheight)
    Drawable w;
    GC gc;
    int width, height;
    short *data;
    int pwidth, pheight;
{
    XImage ximage;
    Pixmap pid;
    int dx, dy;

    pid = XCreatePixmap(dpy, w, width, height, d_depth);

    ximage.height = pheight;
    ximage.width = pwidth;
    ximage.xoffset = 0;
    ximage.format = XYBitmap;
    ximage.data = (char *) data;
    ximage.byte_order = LSBFirst;
    ximage.bitmap_unit = 16;
    ximage.bitmap_bit_order = LSBFirst;
    ximage.bitmap_pad = 16;
    ximage.bytes_per_line = (pwidth + 15) / 16 * 2;
    ximage.depth = 1;

    dx = (width - pwidth) / 2;
    dy = (height - pheight) / 2;

    XPutImage(dpy, pid, gc, &ximage, 0, 0, dx, dy, pwidth, pheight);
    return (pid);
}

/***********************************************************************
 *
 *  Procedure:
 *	MakePixmap - make a pixmap
 *
 *  Returned Value:
 *	pid	- the pixmap id
 *
 *  Inputs:
 *	w	- the window to associate the pixmap with
 *	gc	- the graphics context to use
 *	data	- pointer to the pixmap data
 *	width	- the width of the pixmap
 *	height	- the height of the pixmap
 *
 ***********************************************************************
 */

Pixmap
MakePixmap(w, gc, data, width, height)
    Drawable w;
    GC gc;
    short *data;
    int width, height;
{
    return MakeCenteredPixmap(w, gc, width, height, data, width, height);
}

/***********************************************************************
 *
 *  Procedure:
 *	ExpandFilename - expand the tilde character to HOME
 *		if it is the first character of the filename
 *
 *  Returned Value:
 *	a pointer to the new name
 *
 *  Inputs:
 *	name	- the filename to expand
 *
 ***********************************************************************
 */

char *
ExpandFilename(name)
char *name;
{
    char *newname;

    if (name[0] != '~')
	return (name);

    newname = (char *)malloc(HomeLen + strlen(name) + 2);
    sprintf(newname, "%s/%s", Home, &name[1]);

    return (newname);
}

/***********************************************************************
 *
 *  Procedure:
 *	GetUnknownIcon - read in the bitmap file for the unknown icon
 *
 *  Inputs:
 *	name - the filename to read
 *
 ***********************************************************************
 */

void
GetUnknownIcon(name)
char *name;
{
    UnknownPm = GetBitmap(name);
    if (UnknownPm != NULL)
    {
	XGetGeometry(dpy, UnknownPm, &JunkRoot, &JunkX, &JunkY,
	    &UnknownWidth, &UnknownHeight, &JunkBW, &JunkDepth);
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	GetBitmap - read in a bitmap file
 *
 *  Returned Value:
 *	the pixmap associated with the bitmap
 *
 *  Inputs:
 *	name	- the filename to read
 *
 ***********************************************************************
 */

Pixmap
GetBitmap(name)
char *name;
{
    char *bigname;
    int status, junk_hotx, junk_hoty;
    Pixmap pm;

    if  (name == NULL)
	return (NULL);

    name = ExpandFilename(name);
    bigname = name;

    status = XReadBitmapFile(dpy, Root, bigname, &JunkWidth,
	&JunkHeight, &pm, &junk_hotx, &junk_hoty);

    if (status != BitmapSuccess && IconDirectory && name[0] != '/')
    {
	bigname = (char *)malloc(strlen(name) + strlen(IconDirectory) + 2);
	sprintf(bigname, "%s/%s", IconDirectory, name);
	status = XReadBitmapFile(dpy, Root, bigname, &JunkWidth,
	    &JunkHeight, &pm, &junk_hotx, &junk_hoty);
    }

    if (status != BitmapSuccess && name[0] != '/')
    {
	bigname = (char *)malloc(strlen(name) + strlen(BITMAPS) + 2);
	sprintf(bigname, "%s/%s", BITMAPS, name);
	status = XReadBitmapFile(dpy, Root, bigname, &JunkWidth,
	    &JunkHeight, &pm, &junk_hotx, &junk_hoty);
    }

    switch(status)
    {
	case BitmapSuccess:
	    break;

	case BitmapFileInvalid:
	    fprintf(stderr, ".twmrc: invalid bitmap file \"%s\"\n", bigname);
	    break;

	case BitmapNoMemory:
	    fprintf(stderr, ".twmrc: out of memory \"%s\"\n", bigname);
	    break;

	case BitmapOpenFailed:
	    fprintf(stderr, ".twmrc: failed to open bitmap file \"%s\"\n",
		bigname);
	    break;

	default:
	    fprintf(stderr,".twmrc: bitmap error = 0x%x on file \"%s\"\n",
		status, bigname);
	    break;
    }

    if (status != BitmapSuccess)
	return (NULL);

    return (pm);
}

int
GetColor(kind, what, name)
int kind;
int *what;
char *name;
{
    XColor color, junkcolor;

#ifndef TOM
    if (!FirstTime)
	return;
#endif

    if (Monochrome != kind)
	return;

    if (!XParseColor(dpy, CMap, name, &color))
    {
	fprintf(stderr, "twm: invalid color \"%s\"\n", name);
	return;
    }

    if (!XAllocColor(dpy, CMap, &color))
    {
	fprintf(stderr, "twm: invalid color \"%s\"\n", name);
	return;
    }

    *what = color.pixel;
}

GetFont(font)
MyFont *font;
{
    char *deffontname = "fixed";
#ifndef TOM
    if (!FirstTime)
	return;
#endif

    if (font->font != NULL)
	XFreeFont(dpy, font->font);

    if ((font->font = XLoadQueryFont(dpy, font->name)) == NULL)
    {
	if (DefaultFont.name) {
	    deffontname = DefaultFont.name;
	}
	if ((font->font = XLoadQueryFont(dpy, deffontname)) == NULL)
	{
	    fprintf(stderr, "twm: couldn't open font \"%s\" or \"%s\"\n",
		    font->name, deffontname);
	    exit(1);
	}

    }
    font->height = font->font->ascent + font->font->descent;
    font->y = font->font->ascent + 1;
}
