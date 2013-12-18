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
 * $Header: menus.h,v 1.18 88/10/13 06:35:53 toml Exp $
 *
 * twm menus include file
 *
 * 17-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

#ifndef _MENUS_
#define _MENUS_

#define TWM_ROOT	"bLoB_GoOp"	/* my private root menu */
#define TWM_WINDOWS	"TwmWindows"	/* for f.menu "TwmWindows" */

#define MAX_FILE_SIZE 4096	/* max chars to read from file for cut */

typedef struct MenuItem
{
    struct MenuItem *next;	/* next menu item */
    struct MenuItem *prev;	/* prev menu item */
    struct MenuRoot *sub;	/* MenuRoot of a pull right menu */
    struct MenuRoot *root;	/* back pointer to my MenuRoot */
    char *item;			/* the character string displayed */
    int y;			/* y coordinate for text */
    char *action;		/* action to be performed */
    Window w;			/* the item window */
    Window pull;		/* the pull right window (if any) */
    int func;			/* twm built in function */
    int state;			/* video state, 0 = normal, 1 = reversed */
} MenuItem;

typedef struct MenuRoot
{
    struct MenuItem *first;	/* first item in menu */
    struct MenuItem *last;	/* last item in menu */
    struct MenuRoot *prev;	/* previous root menu if pull right */
    struct MenuRoot *next;	/* next in list of root menus */
    char *name;			/* name of root */
    Window w;			/* the window of the menu */
    Window shadow;		/* the shadow window */
    int mapped;			/* has the menu ever been mapped ? */
    int width;			/* width of the menu */
    int items;			/* number of items in the menu */
    int pull;			/* is there a pull right entry ? */
    int active;			/* this menu is active */
} MenuRoot;

typedef struct MouseButton
{
    int func;			/* the function number */
    int mask;			/* modifier mask */
    MenuRoot *menu;		/* menu if func is F_MENU */
    MenuItem *item;		/* action to perform if func != F_MENU */
} MouseButton;

typedef struct FuncKey
{
    struct FuncKey *next;	/* next in the list of function keys */
    char *name;			/* key name */
    KeySym keysym;		/* X keysym */
    KeyCode keycode;		/* X keycode */
    int cont;			/* context */
    int mods;			/* modifiers */
    int func;			/* function to perform */
    char *win_name;		/* window name (if any) */
    char *action;		/* action string (if any) */
} FuncKey;

extern int RootFunction;
extern MouseButton Mouse[MAX_BUTTONS+1][NUM_CONTEXTS][MOD_SIZE];
extern MouseButton DefaultFunction;
extern MouseButton WindowFunction;
extern MenuRoot *MenuList;
extern MenuRoot *LastMenu;
extern MenuRoot *ActiveMenu;
extern MenuItem *ActiveItem;
extern MenuRoot *Windows;
extern int MoveFunction;

extern int ConstMove;		/* constrained move variables */
extern int ConstMoveDir;
extern int ConstMoveX;
extern int ConstMoveY;
extern int ConstMoveXL;
extern int ConstMoveXR;
extern int ConstMoveYT;
extern int ConstMoveYB;

#define MOVE_NONE	0	/* modes of constrained move */
#define MOVE_VERT	1
#define MOVE_HORIZ	2

extern void InitMenus();
extern MenuRoot *NewMenuRoot();
extern MenuItem *AddToMenu();
extern void PopUpMenu();
extern MenuRoot *FindMenuRoot();
extern FuncKey FuncKeyRoot;
extern void AddFuncKey();
extern void ExecuteFunction();
extern int DeferExecution();
extern void Execute();
extern void FocusOnRoot();

#endif _MENUS_
