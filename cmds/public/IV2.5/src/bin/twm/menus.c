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
 * $Header: menus.c,v 1.2.1.2 89/03/22 08:38:45 interran Exp $
 *
 * twm menu code
 *
 * 17-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[] =
"$Header: menus.c,v 1.2.1.2 89/03/22 08:38:45 interran Exp $";
#endif

#include <stdio.h>
#include <signal.h>
#include "twm.h"
#include "gc.h"
#include "menus.h"
#include "events.h"
#include "util.h"
#include "gram.h"
#include "pull.bm"

#ifdef macII
#define vfork fork
#endif

int RootFunction = NULL;
MenuRoot *MenuList = NULL;		/* head of the menu list */
MenuRoot *LastMenu = NULL;		/* the last menu */
MenuRoot *ActiveMenu = NULL;		/* the active menu */
MenuRoot *Windows = NULL;		/* the TwmWindows menu */
MenuItem *ActiveItem = NULL;		/* the active menu item */
MouseButton Mouse[MAX_BUTTONS+1][NUM_CONTEXTS][MOD_SIZE];
MouseButton DefaultFunction;
MouseButton WindowFunction;
FuncKey FuncKeyRoot;			/* head of function key list */
int MoveFunction;			/* either F_MOVE or F_FORCEMOVE */
 
/***********************************************************************
 *
 *  Procedure:
 *	InitMenus - initialize menu roots
 *
 ***********************************************************************
 */

void
InitMenus()
{
    int i, j, k;
    FuncKey *key, *tmp;

    for (i = 0; i < MAX_BUTTONS+1; i++)
	for (j = 0; j < NUM_CONTEXTS; j++)
	    for (k = 0; k < MOD_SIZE; k++)
	    {
		Mouse[i][j][k].func = NULL;
		Mouse[i][j][k].item = NULL;
	    }

    DefaultFunction.func = NULL;
    WindowFunction.func = NULL;

    Mouse[1][C_TITLE][0].func = F_RAISE;
    Mouse[2][C_TITLE][0].func = F_MOVE;
    Mouse[3][C_TITLE][0].func = F_LOWER;

    Mouse[1][C_ICON][0].func = F_ICONIFY;
    Mouse[2][C_ICON][0].func = F_MOVE;
    Mouse[3][C_ICON][0].func = F_LOWER;

    Mouse[1][C_ICONMGR][0].func = F_ICONIFY;
    Mouse[2][C_ICONMGR][0].func = F_ICONIFY;
    Mouse[3][C_ICONMGR][0].func = F_ICONIFY;

    for (key = FuncKeyRoot.next; key != NULL;)
    {
	free(key->name);
	tmp = key;
	key = key->next;
	free(tmp);
    }
    FuncKeyRoot.next = NULL;

}

/***********************************************************************
 *
 *  Procedure:
 *	AddFuncKey - add a function key to the list
 *
 *  Inputs:
 *	name	- the name of the key
 *	cont	- the context to look for the key press in
 *	mods	- modifier keys that need to be pressed
 *	func	- the function to perform
 *	win_name- the window name (if any)
 *	action	- the action string associated with the function (if any)
 *
 ***********************************************************************
 */

void
AddFuncKey(name, cont, mods, func, win_name, action)
    char *name;
    int cont, mods, func;
    char *win_name;
    char *action;
{
    FuncKey *tmp;
    KeySym keysym;
    KeyCode keycode;

    /*
     * Don't let a 0 keycode go through, since that means AnyKey to the
     * XGrabKey call in GrabKeys().
     */
    if ((keysym = XStringToKeysym(name)) == NoSymbol ||
	(keycode = XKeysymToKeycode(dpy, keysym)) == 0)
    {
	fprintf(stderr, "twm: unknown key name \"%s\"\n", name);
	return;
    }

    /* see if there already is a key defined for this context */
    for (tmp = FuncKeyRoot.next; tmp != NULL; tmp = tmp->next)
    {
	if (tmp->keysym == keysym &&
	    tmp->cont == cont &&
	    tmp->mods == mods)
	    break;
    }

    if (tmp == NULL)
    {
	tmp = (FuncKey *) malloc(sizeof(FuncKey));
	tmp->next = FuncKeyRoot.next;
	FuncKeyRoot.next = tmp;
    }

    tmp->name = name;
    tmp->keysym = keysym;
    tmp->keycode = keycode;
    tmp->cont = cont;
    tmp->mods = mods;
    tmp->func = func;
    tmp->win_name = win_name;
    tmp->action = action;
}

/***********************************************************************
 *
 *  Procedure:
 *	NewMenuRoot - create a new menu root
 *
 *  Returned Value:
 *	(MenuRoot *)
 *
 *  Inputs:
 *	name	- the name of the menu root
 *
 ***********************************************************************
 */

MenuRoot *
NewMenuRoot(name)
    char *name;
{
    MenuRoot *tmp;
    unsigned long valuemask;
    XSetWindowAttributes attributes;

    CreateGCs();

    tmp = (MenuRoot *) malloc(sizeof(MenuRoot));
    tmp->name = name;
    tmp->prev = NULL;
    tmp->first = NULL;
    tmp->last = NULL;
    tmp->items = 0;
    tmp->width = 0;
    tmp->mapped = FALSE;
    tmp->pull = FALSE;
    tmp->active = TRUE;
    tmp->shadow = XCreateSimpleWindow(dpy, Root,
	0, 0, 10, 10, 1, MenuShadowColor, MenuShadowColor);
    tmp->w = XCreateSimpleWindow(dpy, Root,
	0, 0, 10, 10, 1, MenuC.fore, MenuC.back);
    XSelectInput(dpy, tmp->w, LeaveWindowMask);

    if (SaveUnder)
    {
	valuemask = CWSaveUnder;
	attributes.save_under = True;
	XChangeWindowAttributes(dpy, tmp->shadow, valuemask, &attributes);
	XChangeWindowAttributes(dpy, tmp->w, valuemask, &attributes);
    }

    if (MenuList == NULL)
    {
	MenuList = tmp;
	MenuList->next = NULL;
    }

    if (LastMenu == NULL)
    {
	LastMenu = tmp;
	LastMenu->next = NULL;
    }
    else
    {
	LastMenu->next = tmp;
	LastMenu = tmp;
	LastMenu->next = NULL;
    }

    if (strcmp(name, TWM_WINDOWS) == 0)
	Windows = tmp;

    return (tmp);
}

/***********************************************************************
 *
 *  Procedure:
 *	AddToMenu - add an item to a root menu
 *
 *  Returned Value:
 *	(MenuItem *)
 *
 *  Inputs:
 *	menu	- pointer to the root menu to add the item
 *	item	- the text to appear in the menu
 *	action	- the string to possibly execute
 *	sub	- the menu root if it is a pull-right entry
 *	func	- the numeric function
 *
 ***********************************************************************
 */

MenuItem *
AddToMenu(menu, item, action, sub, func)
    MenuRoot *menu;
    char *item, *action;
    MenuRoot *sub;
    int func;
{
    unsigned long valuemask;
    XSetWindowAttributes attributes;
    MenuItem *tmp;
    int width;

#ifdef DEBUG
    fprintf(stderr, "adding menu item=\"%s\", action=%s, sub=%d, f=%d\n",
	item, action, sub, func);
#endif

    tmp = (MenuItem *) malloc(sizeof(MenuItem));
    tmp->root = menu;

    if (menu->first == NULL)
    {
	menu->first = tmp;
	tmp->prev = NULL;
    }
    else
    {
	menu->last->next = tmp;
	tmp->prev = menu->last;
    }
    menu->last = tmp;

    tmp->item = item;
    tmp->action = action;
    tmp->next = NULL;
    tmp->sub = NULL;
    tmp->pull = NULL;
    tmp->state = 0;
    tmp->func = func;

    width = XTextWidth(MenuFont.font, item, strlen(item));
    if (width <= 0)
	width = 1;
    if (width > menu->width)
	menu->width = width;

    if (tmp->func != F_TITLE)
    {
	tmp->w = XCreateSimpleWindow(dpy, menu->w,
	    0, menu->items * (MenuFont.height + 4),
	    width, MenuFont.height + 4,
	    0,
	    MenuC.fore, MenuC.back);
	XSelectInput(dpy, tmp->w, EnterWindowMask
	    | LeaveWindowMask | ExposureMask);
    }
    else
    {
	tmp->w = XCreateSimpleWindow(dpy, menu->w,
	    -1, menu->items * (MenuFont.height + 4),
	    width, MenuFont.height + 2,
	    1,
	    MenuC.fore, MenuTitleC.back);
	XSelectInput(dpy, tmp->w, ExposureMask);
    }

    if (sub != NULL)
    {
	Pixmap pm;

	tmp->sub = sub;
	pm = MakeCenteredPixmap(tmp->w, MenuNormalGC,
	    pull_width, MenuFont.height + 4,
	    pull_bits, pull_width, pull_height);

	valuemask = CWEventMask | CWBackPixmap;
	attributes.background_pixmap = pm;
	attributes.event_mask = EnterWindowMask | LeaveWindowMask;

	tmp->pull = XCreateWindow(dpy, tmp->w,
	    0, 0,
	    pull_width, MenuFont.height + 4,
	    0, d_depth, CopyFromParent,
	    d_visual, valuemask, &attributes);

	XMapWindow(dpy, tmp->pull);

	menu->pull = TRUE;
	XSaveContext(dpy, tmp->pull, MenuContext, tmp);
    }
    menu->items += 1;

    XSaveContext(dpy, tmp->w, MenuContext, tmp);

    if (menu->items == 1)
	XSaveContext(dpy, tmp->root->w, MenuContext, tmp);

    return (tmp);
}

/***********************************************************************
 *
 *  Procedure:
 *	PopUpMenu - pop up a pull down menu
 *
 *  Inputs:
 *	menu	- the root pointer of the menu to pop up
 *	x	- the x location of the mouse
 *	y	- the y location of the mouse
 *
 ***********************************************************************
 */

void
PopUpMenu(menu, x, y)
    MenuRoot *menu;
    int x, y;
{
    unsigned long valuemask;
    XSetWindowAttributes attributes;
    int m_height;
    XWindowChanges xwc, pwc;
    unsigned int xwcm, pwcm;
    MenuItem *tmp, *tmp1;
    TwmWindow *tmp_win;

    if (menu == NULL)
	return;

    if (menu == Windows)
    {
	/* this is the twm windows menu,  let's go ahead and build it */

	for (tmp = menu->first; tmp != NULL;)
	{
	    XDeleteContext(dpy, tmp->w, MenuContext);
	    XDestroyWindow(dpy, tmp->w);

	    tmp1 = tmp;
	    tmp = tmp->next;
	    free(tmp1);
	}

	if (ActiveMenu != NULL)
	    menu->prev = ActiveMenu;
	else
	    menu->prev = NULL;
	menu->first = NULL;
	menu->last = NULL;
	menu->items = 0;
	menu->width = 0;
	menu->mapped = FALSE;
	menu->pull = FALSE;

	AddToMenu(menu, "TWM Windows", NULL, NULL, F_TITLE);
	for (tmp_win = TwmRoot.next; tmp_win != NULL; tmp_win = tmp_win->next)
	{
	    AddToMenu(menu, tmp_win->name, tmp_win, NULL, F_POPUP);
	}
    }

    if (menu->items == 0)
	return;

    XGrabPointer(dpy, Root, True,
	ButtonReleaseMask,
	GrabModeAsync, GrabModeAsync,
	Root, LeftArrowCursor, CurrentTime);

    if (ActiveMenu != NULL)
	ActiveMenu->active = FALSE;

    menu->active = TRUE;
    ActiveMenu = menu;

    if (menu->mapped != TRUE)
    {
	if (menu->pull == TRUE)
	{
	    menu->width += pull_width + 10;
	}

	xwcm = 0;
	xwcm |= CWWidth;
	xwc.width = menu->width + 10;

	pwcm = 0;
	pwcm |= CWX;
	pwc.x = xwc.width - pull_width;

	valuemask = CWBackingStore;
	attributes.backing_store = Always;

	for (tmp = menu->first; tmp != NULL; tmp = tmp->next)
	{
	    if (BackingStore)
		XChangeWindowAttributes(dpy, tmp->w, valuemask, &attributes);

	    XConfigureWindow(dpy, tmp->w, xwcm, &xwc);
	    if (tmp->pull != NULL)
	    {
		XConfigureWindow(dpy, tmp->pull, pwcm, &pwc);
	    }
	    if (tmp->func != F_TITLE)
		tmp->y = 5;
	    else
	    {
		tmp->y = xwc.width - XTextWidth(MenuFont.font, tmp->item,
		    strlen(tmp->item));
		tmp->y /= 2;
	    }
	}
    }
    menu->mapped = TRUE;

    m_height = menu->items * (MenuFont.height + 4);

    if ((x + 10) > MyDisplayWidth)
	x = (MyDisplayWidth - 30);

    if ((y + m_height + 10) > MyDisplayHeight)
	y = (MyDisplayHeight - m_height);

    xwcm = CWX | CWY | CWWidth | CWHeight;

    xwc.x = x - menu->width + 10;
    if (xwc.x < 0)
	xwc.x = 0;
    xwc.y = y - ((MenuFont.height + 4) / 2);
    xwc.width = menu->width + 10;
    xwc.height = m_height;

    XConfigureWindow(dpy, menu->w, xwcm, &xwc);

    xwc.x = xwc.x + 5;
    xwc.y = xwc.y + 5;

    XConfigureWindow(dpy, menu->shadow, xwcm, &xwc);
    XWarpPointer(dpy, None, menu->w, 0, 0, 0, 0, 
     menu->width - 10, (MenuFont.height + 4) / 2);
    XMapSubwindows(dpy, menu->w);
    XRaiseWindow(dpy, menu->shadow);
    XMapRaised(dpy, menu->w);
    XMapWindow(dpy, menu->shadow);
}

/***********************************************************************
 *
 *  Procedure:
 *	FindMenuRoot - look for a menu root
 *
 *  Returned Value:
 *	(MenuRoot *)  - a pointer to the menu root structure 
 *
 *  Inputs:
 *	name	- the name of the menu root 
 *
 ***********************************************************************
 */

MenuRoot *
FindMenuRoot(name)
    char *name;
{
    MenuRoot *tmp;

    for (tmp = MenuList; tmp != NULL; tmp = tmp->next)
    {
	if (strcmp(name, tmp->name) == 0)
	    return (tmp);
    }
    return NULL;
}

/***********************************************************************
 *
 *  Procedure:
 *	ExecuteFunction - execute a twm root function
 *
 *  Inputs:
 *	func	- the function to execute
 *	action	- the menu action to execute 
 *	w	- the window to execute this function on
 *	tmp_win	- the twm window structure
 *	event	- the event that caused the function
 *	context - the context in which the button was pressed
 *	pulldown- flag indicating execution from pull down menu
 *
 ***********************************************************************
 */

void
ExecuteFunction(func, action, w, tmp_win, event, context, pulldown)
    int func;
    char *action;
    Window w;
    TwmWindow *tmp_win;
    XEvent event;
    int context;
    int pulldown;
{
    static Time last_time = 0;

    char tmp[200];
    char *ptr;
    int len;
    char buff[MAX_FILE_SIZE];
    int count, fd;
    MenuRoot *root, *tmp_root;
    MenuItem *item, *tmp_item;

    XGrabPointer(dpy, Root, True,
	ButtonReleaseMask,
	GrabModeAsync, GrabModeAsync,
	Root, ClockCursor, CurrentTime);

    switch (func)
    {
    case F_NOP:
    case F_TITLE:
	break;

    case F_AUTORAISE:
	if (DeferExecution(context, func, DotCursor))
	    return;
	tmp_win->auto_raise = !tmp_win->auto_raise;
	break;

    case F_SHOWLIST:
	DeIconify(IconManagerPtr);
	XRaiseWindow(dpy, IconManagerPtr->frame);
	break;

    case F_HIDELIST:
	XUnmapWindow(dpy, IconManagerPtr->frame);
	XUnmapWindow(dpy, IconManagerPtr->icon_w);
	break;

    case F_BEEP:
	XBell(dpy, screen);
	break;

    case F_POPUP:
	tmp_win = (TwmWindow *)action;
	if (WindowFunction.func != NULL)
	{
	   ExecuteFunction(WindowFunction.func, WindowFunction.item->action,
	       w, tmp_win, event, C_FRAME, FALSE);
	}
	else
	{
	    DeIconify(tmp_win);
	    XMapWindow(dpy, tmp_win->w);
	    XMapRaised(dpy, tmp_win->frame);
	}
	break;

    case F_RESIZE:
	if (DeferExecution(context, func, MoveCursor))
	    return;

	if (pulldown)
	    XWarpPointer(dpy, None, Root, 
		0, 0, 0, 0, event.xbutton.x_root, event.xbutton.y_root);

	if (w != tmp_win->icon_w)
	{
	    EventHandler[EnterNotify] = HandleUnknown;
	    EventHandler[LeaveNotify] = HandleUnknown;
	    EventHandler[Expose] = HandleUnknown;
	    StartResize(event, tmp_win);
	    return;
	}
	break;


    case F_ZOOM:
	if (DeferExecution(context, func, DotCursor))
	    return;
	fullzoom(tmp_win, ZOOM_VERT);
	break;

    case F_FULLZOOM:
	if (DeferExecution(context, func, DotCursor))
	    return;
	fullzoom(tmp_win, ZOOM_FULL);
	break;

    case F_MOVE:
    case F_FORCEMOVE:
	if (DeferExecution(context, func, MoveCursor))
	    return;

	MoveFunction = func;

	if (pulldown)
	    XWarpPointer(dpy, None, Root, 
		0, 0, 0, 0, event.xbutton.x_root, event.xbutton.y_root);

	EventHandler[EnterNotify] = HandleUnknown;
	EventHandler[LeaveNotify] = HandleUnknown;
	EventHandler[Expose] = HandleUnknown;

	/* redraw the text in the title bar or the icon window if
	 * needed, we have disabled expose event handling so we must
	 * do it here
	 */
	if (context == C_TITLE)
	{
	    XDrawImageString(dpy, tmp_win->title_w,
		tmp_win->title_gc,
		TitleBarX, TitleBarFont.y,
		tmp_win->name, strlen(tmp_win->name));
	}
	else if (context == C_ICON)
	{
	    XDrawImageString(dpy, tmp_win->icon_w,
		IconNormalGC,
		tmp_win->icon_x, tmp_win->icon_y,
		tmp_win->icon_name, strlen(tmp_win->icon_name));
	}

	XGrabServer(dpy);
	XGrabPointer(dpy, event.xbutton.root, True,
	    ButtonReleaseMask,
	    GrabModeAsync, GrabModeAsync,
	    Root, MoveCursor, CurrentTime);

	if (context == C_ICON)
	    w = tmp_win->icon_w;
	else if (w != tmp_win->icon_w)
	    w = tmp_win->frame;

	DragX = event.xbutton.x;
	DragY = event.xbutton.y;

	if (context == C_WINDOW)
	    DragY += tmp_win->title_height;
		 
	DragWindow = w;

	XGetGeometry(dpy, w, &JunkRoot, &JunkX, &JunkY,
	    &DragWidth, &DragHeight, &JunkBW,
	    &JunkDepth);

	MoveOutline((Window)event.xbutton.root,
	    event.xbutton.x_root-DragX-BorderWidth,
	    event.xbutton.y_root-DragY-BorderWidth,
	    DragWidth + 2 * BorderWidth,
	    DragHeight + 2 * BorderWidth);

	if ((event.xbutton.time - last_time) < 400)
	{
	    int width, height;

	    ConstMove = TRUE;
	    ConstMoveDir = MOVE_NONE;
	    ConstMoveX = event.xbutton.x_root - DragX - BorderWidth;
	    ConstMoveY = event.xbutton.y_root - DragY - BorderWidth;
	    width = DragWidth + 2 * BorderWidth;
	    height = DragHeight + 2 * BorderWidth;
	    ConstMoveXL = ConstMoveX + width/3;
	    ConstMoveXR = ConstMoveX + 2*(width/3);
	    ConstMoveYT = ConstMoveY + height/3;
	    ConstMoveYB = ConstMoveY + 2*(height/3);

	    XWarpPointer(dpy, None, DragWindow,
		0, 0, 0, 0, DragWidth/2, DragHeight/2);

	    XQueryPointer(dpy, DragWindow, &JunkRoot, &JunkChild,
		&JunkX, &JunkY, &DragX, &DragY, &JunkMask);
	}
	last_time = event.xbutton.time;
	return;
        break;

    case F_FUNCTION:
	{
	    MenuRoot *mroot;
	    MenuItem *mitem;

	    if ((mroot = FindMenuRoot(action)) == NULL)
	    {
		fprintf(stderr, "twm: couldn't find function \"%s\"\n", action);
		return;
	    }

	    if (NeedToDefer(mroot) && DeferExecution(context, func, DotCursor))
		return;
	    else
	    {
		for (mitem = mroot->first; mitem != NULL; mitem = mitem->next)
		{
		    ExecuteFunction(mitem->func, mitem->action, w, tmp_win,
			event, context, pulldown);
		}
	    }
	}
	break;

    case F_DEICONIFY:
    case F_ICONIFY:
	if (DeferExecution(context, func, DotCursor))
	    return;

	if (tmp_win->icon)
	{
	    DeIconify(tmp_win);
	}
        else if (func == F_ICONIFY)
	{
	    TwmWindow *t;

	    if (!tmp_win->iconified)
	    {
		int final_x, final_y;

		if (tmp_win->wmhints &&
		    tmp_win->wmhints->flags & IconPositionHint)
		{
		    final_x = tmp_win->wmhints->icon_x;
		    final_y = tmp_win->wmhints->icon_y;
		}
		else
		{
		    final_x = event.xbutton.x_root - 5;
		    final_y = event.xbutton.y_root - 5;
		}

		if (final_x > MyDisplayWidth)
		    final_x = MyDisplayWidth - tmp_win->icon_w_width -
			(2 * BorderWidth);

		if (final_y > MyDisplayHeight)
		    final_y = MyDisplayHeight - tmp_win->icon_height -
			IconFont.height - 4 - (2 * BorderWidth);

		XMoveWindow(dpy, tmp_win->icon_w, final_x, final_y);
		tmp_win->iconified = TRUE;
	    }

	    Iconify(tmp_win);
	}
	SetHints(tmp_win);
	break;

    case F_RAISELOWER:
	if (DeferExecution(context, func, DotCursor))
	    return;

	{
	    int vis;

	    if (w == tmp_win->icon_w)
		vis = tmp_win->icon_vis;
	    else
	    {
		w = tmp_win->frame;
		vis = tmp_win->frame_vis;
	    }

	    if (vis == VisibilityUnobscured)
		XLowerWindow(dpy, w);
	    else
		XRaiseWindow(dpy, w);
	}
	break;
	
    case F_RAISE:
	if (DeferExecution(context, func, DotCursor))
	    return;

	if (w == tmp_win->icon_w)
	    XRaiseWindow(dpy, tmp_win->icon_w);
	else
	    XRaiseWindow(dpy, tmp_win->frame);

	break;

    case F_LOWER:
	if (DeferExecution(context, func, DotCursor))
	    return;

	if (w == tmp_win->icon_w)
	    XLowerWindow(dpy, tmp_win->icon_w);
	else
	    XLowerWindow(dpy, tmp_win->frame);

	break;

    case F_FOCUS:
	if (DeferExecution(context, func, DotCursor))
	    return;

	if (tmp_win->icon == FALSE)
	{
	    if (Focus != NULL && Focus != tmp_win)
	    {
		if (Highlight && Focus->highlight)
		{
		    XSetWindowBorderPixmap(dpy, Focus->frame, GrayTile);
		    XSetWindowBorderPixmap(dpy, Focus->title_w, GrayTile);
		}
		XUnmapWindow(dpy, Focus->hilite_w);
	    }
	    XMapWindow(dpy, tmp_win->hilite_w);
	    XSetWindowBorder(dpy, tmp_win->frame, BorderColor);
	    XSetWindowBorder(dpy, tmp_win->title_w, BorderColor);
	    XSetInputFocus(dpy, tmp_win->w, RevertToPointerRoot,
		    CurrentTime);
	    FocusRoot = FALSE;
	    Focus = tmp_win;
	}
	break;

    case F_DESTROY:
	if (DeferExecution(context, func, SkullCursor))
	    return;

	if (tmp_win == IconManagerPtr)
	    XBell(dpy, screen);
	else
	    XKillClient(dpy, tmp_win->w);
	break;

    case F_CIRCLEUP:
	XCirculateSubwindowsUp(dpy, Root);
	break;

    case F_CIRCLEDOWN:
	XCirculateSubwindowsDown(dpy, Root);
	break;

    case F_VERSION:
	XMapRaised(dpy, VersionWindow);
	break;

    case F_EXEC:
	Execute(action);
	break;

    case F_UNFOCUS:
	FocusOnRoot();
	break;

    case F_CUT:
	strcpy(tmp, action);
	strcat(tmp, "\n");
	XStoreBytes(dpy, tmp, strlen(tmp));
	break;

    case F_CUTFILE:
	ptr = XFetchBytes(dpy, &count);
	if (count != 0)
	{
	    if (sscanf(ptr, "%s", tmp) == 1)
	    {
		ptr = ExpandFilename(tmp);
		fd = open(ptr, 0);
		if (fd >= 0)
		{
		    count = read(fd, buff, MAX_FILE_SIZE - 1);
		    if (count > 0)
			XStoreBytes(dpy, buff, count);

		    close(fd);
		}
		else
		{
		    fprintf(stderr, "twm: couldn't open \"%s\"\n", tmp);
		}
	    }
	    XFree(ptr);
	}
	else
	{
	    fprintf(stderr, "twm: nothing in the cut buffer\n");
	}
	break;

    case F_FILE:
	action = ExpandFilename(action);
	fd = open(action, 0);
	if (fd >= 0)
	{
	    count = read(fd, buff, MAX_FILE_SIZE - 1);
	    if (count > 0)
		XStoreBytes(dpy, buff, count);

	    close(fd);
	}
	else
	{
	    fprintf(stderr, "twm: couldn't open \"%s\"\n", action);
	}
	break;

    case F_TWMRC:
	len = strlen(action);
	if (len == 0)
	    ptr = NULL;
	else
	{
	    ptr = (char *)malloc(len+1);
	    if (ptr == NULL)
	    {
		fprintf(stderr, "twm: out of memory\n");
		exit(1);
	    }
	    strcpy(ptr, action);
	    ptr = ExpandFilename(ptr);
	}

	/* first get rid of the existing menu structure and destroy all
	 * windows */
	for (root = MenuList; root != NULL;)
	{
	    for (item = root->last; item != NULL;)
	    {
		if (item->pull != NULL)
		{
		    XDeleteContext(dpy, item->pull, MenuContext);
		    XDestroyWindow(dpy, item->pull);
		}
		XDeleteContext(dpy, item->w, MenuContext);
		XDestroyWindow(dpy, item->w);

		tmp_item = item;
		item = item->prev;
		free(tmp_item);
	    }

	    XDeleteContext(dpy, root->w, MenuContext);
	    XDestroyWindow(dpy, root->shadow);
	    XDestroyWindow(dpy, root->w);

	    tmp_root = root;
	    root = root->next;
	    free(tmp_root);
	}
	MenuList = NULL;
	LastMenu = NULL;
	/*
	ActiveMenu = NULL;
	ActiveItem = NULL;
	*/

	UngrabAllButtons();
	UngrabAllKeys();

	ParseTwmrc(ptr);

#ifdef TOM
	/* I started to write code to redo colors and stuff, but I 
	 * didn't get it all done, that's why it is ifdef'ed out
	 */
	for (tmp_win = TwmRoot.next; tmp_win != NULL; tmp_win = tmp_win->next)
	{
	    XUnmapWindow(dpy, tmp_win->title_w);
	    XSetWindowBackground(dpy, tmp_win->title_w, TitleC.back);
	    XSetForeground(dpy, tmp_win->title_gc, TitleC.fore);
	    XSetBackground(dpy, tmp_win->title_gc, TitleC.back);
	    XDestroyWindow(dpy, tmp_win->iconify_w, TitleC.back);
#ifndef NOFOCUS
	    XDestroyWindow(dpy, tmp_win->focus_w, TitleC.back);
#endif
	    XDestroyWindow(dpy, tmp_win->resize_w, TitleC.back);
	    XDestroyWindow(dpy, tmp_win->hilite_w, TitleC.back);
	    CreateTitleButtons(tmp_win);
	    XMapWindow(dpy, tmp_win->title_w);
	    SetupWindow(tmp_win, tmp_win->frame_x, tmp_win->frame_y,
		tmp_win->frame_width, tmp_win->frame_height);
	    if (tmp_win == Focus || !Highlight || !tmp_win->highlight)
	    {
		XSetWindowBorder(dpy, tmp_win->frame, BorderColor);
		XSetWindowBorder(dpy, tmp_win->title_w, BorderColor);
	    }
	    if (tmp_win == Focus)
		XMapWindow(tmp_win->hilite_w);
	}
#endif
	GrabAllButtons();
	GrabAllKeys();
	break;

    case F_REFRESH:
	w = XCreateSimpleWindow(dpy, Root,
	    0, 0, 9999, 9999, 0, Black, Black);
	XMapWindow(dpy, w);
	XDestroyWindow(dpy, w);
	XFlush(dpy);
	break;

    case F_WINREFRESH:
	if (DeferExecution(context, func, DotCursor))
	    return;

	if (context == C_ICON)
	    w = XCreateSimpleWindow(dpy, tmp_win->icon_w,
		0, 0, 9999, 9999, 0, Black, Black);
	else
	    w = XCreateSimpleWindow(dpy, tmp_win->frame,
		0, 0, 9999, 9999, 0, Black, Black);

	XMapWindow(dpy, w);
	XDestroyWindow(dpy, w);
	XFlush(dpy);
	break;

    case F_QUIT:
	Done();
	break;
    }
    XUngrabPointer(dpy, CurrentTime);
}

/***********************************************************************
 *
 *  Procedure:
 *	DeferExecution - defer the execution of a function to the
 *	    next button press if the context is C_ROOT
 *
 *  Inputs:
 *	context	- the context in which the mouse button was pressed
 *	func	- the function to defer
 *	cursor	- the cursor to display while waiting
 *
 ***********************************************************************
 */

int
DeferExecution(context, func, cursor)
int context, func;
Cursor cursor;
{
    if (context == C_ROOT)
    {
	XGrabPointer(dpy, Root, True,
	    ButtonPressMask | ButtonReleaseMask,
	    GrabModeAsync, GrabModeAsync,
	    Root, cursor, CurrentTime);

	RootFunction = func;

	return (TRUE);
    }
    
    return (FALSE);
}


/***********************************************************************
 *
 *  Procedure:
 *	NeedToDefer - checks each function in the list to see if it
 *		is one that needs to be defered.
 *
 *  Inputs:
 *	root	- the menu root to check
 *
 ***********************************************************************
 */

NeedToDefer(root)
MenuRoot *root;
{
    MenuItem *mitem;

    for (mitem = root->first; mitem != NULL; mitem = mitem->next)
    {
	switch (mitem->func)
	{
	case F_RESIZE:
	case F_MOVE:
	case F_FORCEMOVE:
	case F_DEICONIFY:
	case F_ICONIFY:
	case F_RAISELOWER:
	case F_RAISE:
	case F_LOWER:
	case F_FOCUS:
	case F_DESTROY:
	case F_WINREFRESH:
	case F_ZOOM:
	case F_FULLZOOM:
	    return TRUE;
	}
    }
    return FALSE;
}

/***********************************************************************
 *
 *  Procedure:
 *	Execute - execute the string by /bin/sh
 *
 *  Inputs:
 *	s	- the string containing the command
 *
 ***********************************************************************
 */

void
Execute(s)
    char *s;
{
    int status, pid, w;
#ifdef sun
    register void (*istat) (), (*qstat) ();
#else
    register int (*istat) (), (*qstat) ();
#endif sun

    if ((pid = vfork()) == 0)
    {
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGHUP, SIG_DFL);
	execl("/bin/sh", "sh", "-c", s, 0);
	_exit(127);
    }
    istat = signal(SIGINT, SIG_IGN);
    qstat = signal(SIGQUIT, SIG_IGN);
    while ((w = wait(&status)) != pid && w != -1);
    if (w == -1)
	status = -1;
    signal(SIGINT, istat);
    signal(SIGQUIT, qstat);
}

/***********************************************************************
 *
 *  Procedure:
 *	FocusOnRoot - put input focus on the root window
 *
 ***********************************************************************
 */

void
FocusOnRoot()
{
    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    if (Focus != NULL)
    {
	if (Highlight && Focus->highlight)
	{
	    XSetWindowBorderPixmap(dpy, Focus->frame, GrayTile);
	    XSetWindowBorderPixmap(dpy, Focus->title_w, GrayTile);
	}
	XUnmapWindow(dpy, Focus->hilite_w);
    }
    Focus = NULL;
    FocusRoot = TRUE;
}

DeIconify(tmp_win)
TwmWindow *tmp_win;
{
    TwmWindow *t;

    /* de-iconify group members (if any) */
    if (tmp_win->group == tmp_win->w)
    {
	for (t = TwmRoot.next; t != NULL; t = t->next)
	{
	    if (tmp_win->group == t->group &&
		tmp_win->group != t->w && t->icon)
	    {
		if (t->icon_on)
		    Zoom(t->icon_w, t->frame);
		else
		    Zoom(tmp_win->icon_w, t->frame);

		XMapWindow(dpy, t->w);
		if (NoRaiseDeicon)
		    XMapWindow(dpy, t->frame);
		else
		    XMapRaised(dpy, t->frame);

		XUnmapWindow(dpy, t->icon_w);
		XUnmapWindow(dpy, t->list->icon);
		t->icon = FALSE;
		t->icon_on = FALSE;
	    }
	}
    }

    /* now de-iconify the main window */
    if (tmp_win->icon)
    {
	if (tmp_win->icon_on)
	    Zoom(tmp_win->icon_w, tmp_win->frame);
	else if (tmp_win->group != NULL)
	{
	    for (t = TwmRoot.next; t != NULL; t = t->next)
	    {
		if (tmp_win->group == t->w && t->icon_on)
		{
		    Zoom(t->icon_w, tmp_win->frame);
		    break;
		}
	    }
	}
    }


    XMapWindow(dpy, tmp_win->w);
    if (NoRaiseDeicon)
	XMapWindow(dpy, tmp_win->frame);
    else
	XMapRaised(dpy, tmp_win->frame);

    XUnmapWindow(dpy, tmp_win->icon_w);
    if (tmp_win->list)
	XUnmapWindow(dpy, tmp_win->list->icon);
    if (WarpCursor && tmp_win->icon)
    {
	XWarpPointer(dpy, None, tmp_win->frame,
	    0, 0, 0, 0, 30, 8);
    }
    tmp_win->icon = FALSE;
    tmp_win->icon_on = FALSE;
}

Iconify(tmp_win)
TwmWindow *tmp_win;
{
    TwmWindow *t;
    int iconify;

    iconify = !tmp_win->iconify_by_unmapping;
    if (iconify)
    {
	XMapSubwindows(dpy, tmp_win->icon_w);
	XMapRaised(dpy, tmp_win->icon_w);
    }
    if (tmp_win->list)
	XMapWindow(dpy, tmp_win->list->icon);

    /* iconify group members first */
    if (tmp_win->group == tmp_win->w)
    {
	for (t = TwmRoot.next; t != NULL; t = t->next)
	{
	    if (tmp_win->group == t->group && tmp_win->group != t->w)
	    {
		if (iconify)
		{
		    if (t->icon_on)
			Zoom(t->icon_w, tmp_win->icon_w);
		    else
			Zoom(t->frame, tmp_win->icon_w);
		}

		XUnmapWindow(dpy, t->frame);
		XUnmapWindow(dpy, t->w);
		XUnmapWindow(dpy, t->icon_w);
		if (Highlight && tmp_win->highlight)
		{
		    XSetWindowBorderPixmap(dpy, tmp_win->frame, GrayTile);
		    XSetWindowBorderPixmap(dpy, tmp_win->title_w, GrayTile);
		}
		if (t == Focus)
		{
		    XSetInputFocus(dpy, PointerRoot,
			RevertToPointerRoot, CurrentTime);
		    Focus = NULL;
		    FocusRoot = TRUE;
		}
		XMapWindow(dpy, t->list->icon);
		t->icon = TRUE;
		t->icon_on = FALSE;
	    }
	}
    }

    if (iconify)
	Zoom(tmp_win->frame, tmp_win->icon_w);
    XUnmapWindow(dpy, tmp_win->frame);
    XUnmapWindow(dpy, tmp_win->w);

    if (Highlight && tmp_win->highlight)
    {
	XSetWindowBorderPixmap(dpy, tmp_win->frame, GrayTile);
	XSetWindowBorderPixmap(dpy, tmp_win->title_w, GrayTile);
    }

    if (tmp_win == Focus)
    {
	XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot,
	    CurrentTime);
	Focus = NULL;
	FocusRoot = TRUE;
    }
    tmp_win->icon = TRUE;
    if (iconify)
	tmp_win->icon_on = TRUE;
    else
	tmp_win->icon_on = FALSE;
}
