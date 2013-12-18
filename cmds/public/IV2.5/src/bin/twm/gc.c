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
 * $Header: gc.c,v 1.15 88/10/13 06:35:07 toml Exp $
 *
 * Open the fonts and create the GCs
 *
 * 31-Mar-88 Tom LaStrange        Initial Version.
 *
 **********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$Header: gc.c,v 1.15 88/10/13 06:35:07 toml Exp $";
#endif lint

#include <stdio.h>
#include "twm.h"
#include "util.h"

static XFontStruct *dfont;		/* my default font */
static char *dfontname;

/***********************************************************************
 *
 *  Procedure:
 *	CreateGCs - open fonts and create all the needed GC's.  I only
 *		    want to do this once, hence the first_time flag.
 *
 ***********************************************************************
 */

void
CreateGCs()
{
    static int first_time = TRUE;
    XGCValues	    gcv;
    unsigned long   gcm, mask;

    if (!first_time)
	return;

    first_time = FALSE;

    /* create GC's */

    if (ReverseVideo)
    {
	DefaultC.back = Black;
	DefaultC.fore = White;
    }
    else
    {
	DefaultC.fore = Black;
	DefaultC.back = White;
    }

    gcm = 0;
    gcm |= GCPlaneMask;	    gcv.plane_mask = AllPlanes;
    gcm |= GCForeground;    gcv.foreground = BorderTileC.fore;
    gcm |= GCBackground;    gcv.background = BorderTileC.back;

    BorderGC = XCreateGC(dpy, Root, gcm, &gcv);

    gcm = 0;
    gcm |= GCFont;	    gcv.font = MenuFont.font->fid;
    gcm |= GCPlaneMask;	    gcv.plane_mask = AllPlanes;
    gcm |= GCForeground;    gcv.foreground = MenuC.fore;
    gcm |= GCBackground;    gcv.background = MenuC.back;

    MenuNormalGC = XCreateGC(dpy, Root, gcm, &gcv);

    gcv.foreground = MenuC.back;
    gcv.background = MenuC.fore;

    MenuReverseGC = XCreateGC(dpy, Root, gcm, &gcv);

    mask = MenuC.fore ^ MenuC.back;

    gcm = 0;
    gcm |= GCFunction;	    gcv.function = GXxor;
    gcm |= GCFont;	    gcv.font = MenuFont.font->fid;
    gcm |= GCPlaneMask;	    gcv.plane_mask = mask;
    gcm |= GCForeground;    gcv.foreground = mask;
    gcm |= GCBackground;    gcv.background = MenuC.back;

    MenuXorGC = XCreateGC(dpy, Root, gcm, &gcv);

    gcm = 0;
    gcm |= GCFont;	    gcv.font = MenuFont.font->fid;
    gcm |= GCPlaneMask;	    gcv.plane_mask = AllPlanes;
    gcm |= GCForeground;    gcv.foreground = MenuTitleC.fore;
    gcm |= GCBackground;    gcv.background = MenuTitleC.back;

    MenuTitleGC = XCreateGC(dpy, Root, gcm, &gcv);

    mask = DefaultC.fore ^ DefaultC.back;
    gcm = 0;
    gcm |= GCFunction;	    gcv.function = GXxor;
    gcm |= GCLineWidth;	    gcv.line_width = 0;
    gcm |= GCForeground;    gcv.foreground = mask;
    gcm |= GCPlaneMask;	    gcv.plane_mask = mask;
    gcm |= GCSubwindowMode; gcv.subwindow_mode = IncludeInferiors;

    DrawGC = XCreateGC(dpy, Root, gcm, &gcv);

    gcm = 0;
    gcm |= GCFont;	    gcv.font = IconFont.font->fid;
    gcm |= GCPlaneMask;	    gcv.plane_mask = AllPlanes;
    gcm |= GCForeground;    gcv.foreground = IconC.fore;
    gcm |= GCBackground;    gcv.background = IconC.back;

    IconNormalGC = XCreateGC(dpy, Root, gcm, &gcv);

    gcm = 0;
    gcm |= GCFont;	    gcv.font = VersionFont.font->fid;
    gcm |= GCPlaneMask;	    gcv.plane_mask = AllPlanes;
    gcm |= GCForeground;    gcv.foreground = DefaultC.fore;
    gcm |= GCBackground;    gcv.background = DefaultC.back;

    VersionNormalGC = XCreateGC(dpy, Root, gcm, &gcv);

    gcm = 0;
    gcm |= GCFont;	    gcv.font = SizeFont.font->fid;
    gcm |= GCPlaneMask;	    gcv.plane_mask = AllPlanes;
    gcm |= GCForeground;    gcv.foreground = DefaultC.fore;
    gcm |= GCBackground;    gcv.background = DefaultC.back;

    SizeNormalGC = XCreateGC(dpy, Root, gcm, &gcv);

    gcm = 0;
    gcm |= GCFont;	    gcv.font = InitialFont.font->fid;
    gcm |= GCPlaneMask;	    gcv.plane_mask = AllPlanes;
    gcm |= GCForeground;    gcv.foreground = DefaultC.fore;
    gcm |= GCBackground;    gcv.background = DefaultC.back;

    InitialNormalGC = XCreateGC(dpy, Root, gcm, &gcv);

    gcm = 0;
    gcm |= GCFont;	    gcv.font = IconManagerFont.font->fid;
    gcm |= GCPlaneMask;	    gcv.plane_mask = AllPlanes;
    gcm |= GCForeground;    gcv.foreground = IconManagerC.fore;
    gcm |= GCBackground;    gcv.background = IconManagerC.back;

    IconManagerGC = XCreateGC(dpy, Root, gcm, &gcv);
}
