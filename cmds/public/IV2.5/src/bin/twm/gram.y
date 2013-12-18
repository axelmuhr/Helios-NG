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
 * $Header: gram.y,v 1.45.1.1 89/03/20 13:51:42 interran Exp $
 *
 * .twmrc command grammer
 *
 * 07-Jan-86 Thomas E. LaStrange	File created
 *
 ***********************************************************************/

%{
static char RCSinfo[]=
"$Header: gram.y,v 1.45.1.1 89/03/20 13:51:42 interran Exp $";

#include <stdio.h>
#include "twm.h"
#include "menus.h"
#include "list.h"
#include "util.h"

static char *Action = "";
static char *Name = "";
static MenuRoot	*root,
		*pull = NULL;


MenuRoot *GetRoot();

static char *ptr;
static int Button;
static int list;
static int mods = 0, cont = 0;
static int color;

extern int yylineno;
%}

%union
{
    int num;
    char *ptr;
};

%token <num> LB RB MENUS MENU BUTTON TBUTTON DEFAULT_FUNCTION
%token <num> F_MENU F_UNFOCUS F_REFRESH F_FILE F_TWMRC F_CIRCLEUP F_QUIT
%token <num> F_NOP F_TITLE F_VERSION F_EXEC F_CUT F_CIRCLEDOWN F_SOURCE
%token <num> F_CUTFILE F_MOVE F_ICONIFY F_FOCUS F_RESIZE F_RAISE F_LOWER
%token <num> F_POPUP F_DEICONIFY F_FORCEMOVE WINDOW_FUNCTION
%token <num> F_DESTROY F_WINREFRESH F_BEEP DONT_MOVE_OFF ZOOM
%token <num> F_SHOWLIST F_HIDELIST NO_BACKINGSTORE NO_SAVEUNDER
%token <num> F_ZOOM F_FULLZOOM F_AUTORAISE
%token <num> ICONMGR_FOREGROUND ICONMGR_BACKGROUND ICONMGR_FONT ICONMGR
%token <num> ICONMGR_GEOMETRY SHOW_ICONMGR ICONMGR_NOSHOW
%token <num> F_RAISELOWER DECORATE_TRANSIENTS RANDOM_PLACEMENT
%token <num> ICONIFY_BY_UNMAPPING DONT_ICONIFY_BY_UNMAPPING
%token <num> WARPCURSOR NUMBER BORDERWIDTH TITLE_FONT REVERSE_VIDEO
%token <num> RESIZE_FONT NO_TITLE AUTO_RAISE FORCE_ICON NO_HILITE
%token <num> MENU_FONT ICON_FONT UNKNOWN_ICON ICONS ICON_DIRECTORY
%token <num> META SHIFT CONTROL WINDOW TITLE ICON ROOT FRAME
%token <num> COLON EQUALS BORDER_COLOR TITLE_FOREGROUND TITLE_BACKGROUND
%token <num> MENU_FOREGROUND MENU_BACKGROUND MENU_SHADOW_COLOR
%token <num> MENU_TITLE_FOREGROUND MENU_TITLE_BACKGROUND
%token <num> ICON_FOREGROUND ICON_BACKGROUND ICON_BORDER_COLOR
%token <num> NO_RAISE_ON_MOVE NO_RAISE_ON_DEICONIFY NO_RAISE_ON_RESIZE
%token <num> COLOR MONOCHROME NO_TITLE_FOCUS FUNCTION F_FUNCTION
%token <num> BORDER_TILE_FOREGROUND BORDER_TILE_BACKGROUND
%token <ptr> STRING

%type <ptr> string
%type <num> action button number tbutton full fullkey

%start twmrc 

%%
twmrc		: stmts
		;

stmts		: /* Empty */
		| stmts stmt
		;

stmt		: error
		| FORCE_ICON		{ if (FirstTime) ForceIcon = TRUE; }
		| REVERSE_VIDEO		{ if (FirstTime) ReverseVideo = TRUE; }
		| ICON_FONT string	{ IconFont.name = $2;
					  GetFont(&IconFont);
					}
		| RESIZE_FONT string	{ SizeFont.name = $2;
					  GetFont(&SizeFont);
					}
		| MENU_FONT string	{ MenuFont.name = $2;
					  GetFont(&MenuFont);
					}
		| TITLE_FONT string	{ TitleBarFont.name = $2;
					  GetFont(&TitleBarFont);
					}
		| ICONMGR_FONT string	{ IconManagerFont.name=$2;
					  GetFont(&IconManagerFont);
					}
		| ICONMGR_GEOMETRY string{ if (FirstTime) IconManagerGeometry=$2;}
		| UNKNOWN_ICON string	{ if (FirstTime) GetUnknownIcon($2); }
		| ICON_DIRECTORY string	{ if (FirstTime)
					    IconDirectory = ExpandFilename($2);
					}
		| WARPCURSOR		{ if (FirstTime) WarpCursor = TRUE; }
		| NO_RAISE_ON_MOVE	{ if (FirstTime) NoRaiseMove = TRUE; }
		| NO_RAISE_ON_RESIZE	{ if (FirstTime) NoRaiseResize = TRUE; }
		| NO_RAISE_ON_DEICONIFY	{ if (FirstTime) NoRaiseDeicon = TRUE; }
		| DONT_MOVE_OFF		{ if (FirstTime) DontMoveOff = TRUE; }
		| NO_BACKINGSTORE	{ if (FirstTime) BackingStore = FALSE; }
		| NO_SAVEUNDER		{ if (FirstTime) SaveUnder = FALSE; }
		| ZOOM number		{ if (FirstTime)
					  {
						DoZoom = TRUE;
						ZoomCount = $2;
					  }
					}
		| ZOOM			{ if (FirstTime) DoZoom = TRUE; }
		| BORDERWIDTH number	{ if (FirstTime) BorderWidth = $2; }
		| NO_TITLE_FOCUS	{ if (FirstTime) TitleFocus = FALSE; }
		| RANDOM_PLACEMENT	{ if (FirstTime) RandomPlacement=TRUE; }
		| DECORATE_TRANSIENTS	{ if (FirstTime) DecorateTransients =
					    TRUE; }
		| ICONIFY_BY_UNMAPPING	{ if (FirstTime) IconifyByUnmapping =
					    TRUE; }
		| SHOW_ICONMGR	{ if (FirstTime) ShowIconManager =
					    TRUE; }
		| button string		{ root = GetRoot($2);
					  Mouse[$1][C_ROOT][0].func = F_MENU;
					  Mouse[$1][C_ROOT][0].menu = root;
					}
		| button action		{ Mouse[$1][C_ROOT][0].func = $2;
					  if ($2 == F_MENU)
					  {
					    pull->prev = NULL;
					    Mouse[$1][C_ROOT][0].menu = pull;
					  }
					  else
					  {
					    root = GetRoot(TWM_ROOT);
					    Mouse[$1][C_ROOT][0].item = 
					    AddToMenu(root,"x",Action,0,$2);
					  }
					  Action = "";
					  pull = NULL;
					}
		| string fullkey	{ AddFuncKey($1, cont, mods,
						$2, Name, Action);
					  Action = "";
					  pull = NULL;
					  cont = 0;
					  mods = 0;
					}
		| button full		{ Mouse[$1][cont][mods].func = $2;
					  if ($2 == F_MENU)
					  {
					    pull->prev = NULL;
					    Mouse[$1][cont][mods].menu = pull;
					  }
					  else
					  {
					    root = GetRoot(TWM_ROOT);
					    Mouse[$1][cont][mods].item = 
					    AddToMenu(root,"x",Action,0,$2);
					  }
					  Action = "";
					  pull = NULL;
					  cont = 0;
					  mods = 0;
					}
		| tbutton action	{ Mouse[$1][C_TITLE][0].func = $2;
					  Mouse[$1][C_ICON][0].func = $2;
					  if ($2 == F_MENU)
					  {
					    pull->prev = NULL;
					    Mouse[$1][C_TITLE][0].menu = pull;
					    Mouse[$1][C_ICON][0].menu = pull;
					  }
					  else
					  {
					    root = GetRoot(TWM_ROOT);
					    Mouse[$1][C_TITLE][0].item = 
					    AddToMenu(root,"x",Action,0,$2);
					    Mouse[$1][C_ICON][0].item =
						Mouse[$1][C_TITLE][0].item;
					  }
					  Action = "";
					  pull = NULL;
					}
		| DONT_ICONIFY_BY_UNMAPPING { list = DONT_ICONIFY_BY_UNMAPPING;}
		  win_list
		| ICONMGR_NOSHOW	{ list = ICONMGR_NOSHOW; }
		  win_list
		| NO_HILITE		{ list = NO_HILITE; }
		  win_list
		| NO_HILITE		{ if (FirstTime) Highlight = FALSE; }
		| NO_TITLE		{ list = NO_TITLE; }
		  win_list
		| NO_TITLE		{ if (FirstTime) NoTitlebar = TRUE; }
		| AUTO_RAISE		{ list = AUTO_RAISE; }
		  win_list
		| MENU string		{ root = GetRoot($2); }
		  menu
		| FUNCTION string	{ root = GetRoot($2); }
		  function
		| ICONS 		{ list = ICONS; }
		  icon_list
		| COLOR 		{ color = COLOR; }
		  color_list
		| MONOCHROME 		{ color = MONOCHROME; }
		  color_list
		| DEFAULT_FUNCTION action { DefaultFunction.func = $2;
					  if ($2 == F_MENU)
					  {
					    pull->prev = NULL;
					    DefaultFunction.menu = pull;
					  }
					  else
					  {
					    root = GetRoot(TWM_ROOT);
					    DefaultFunction.item = 
					    AddToMenu(root,"x",Action,0,$2);
					  }
					  Action = "";
					  pull = NULL;
					}
		| WINDOW_FUNCTION action { WindowFunction.func = $2;
					   root = GetRoot(TWM_ROOT);
					   WindowFunction.item = 
					   AddToMenu(root,"x",Action,0,$2);
					   Action = "";
					   pull = NULL;
					}
		;


full		: EQUALS keys COLON context COLON action  { $$ = $6; }
		;

fullkey		: EQUALS keys COLON contextkey COLON action  { $$ = $6; }
		;

keys		: /* Empty */
		| keys key
		;

key		: META			{ mods |= Mod1Mask; }
		| SHIFT			{ mods |= ShiftMask; }
		| CONTROL		{ mods |= ControlMask; }
		;

context		: WINDOW		{ cont = C_WINDOW; }
		| TITLE			{ cont = C_TITLE; }
		| ICON			{ cont = C_ICON; }
		| ROOT			{ cont = C_ROOT; }
		| FRAME			{ cont = C_FRAME; }
		| ICONMGR		{ cont = C_ICONMGR; }
		;

contextkey	: WINDOW		{ cont = C_WINDOW; }
		| TITLE			{ cont = C_TITLE; }
		| ICON			{ cont = C_ICON; }
		| ROOT			{ cont = C_ROOT; }
		| FRAME			{ cont = C_FRAME; }
		| string		{ Name = $1; cont = C_NAME; }
		;

color_list	: LB color_entries RB
		;

color_entries	: /* Empty */
		| color_entries color_entry
		;

color_entry	: BORDER_COLOR string	{ GetColor(color, &BorderColor, $2); }
		| BORDER_TILE_FOREGROUND string { GetColor(color,
						&BorderTileC.fore, $2); }
		| BORDER_TILE_BACKGROUND string { GetColor(color,
						&BorderTileC.back, $2); }
		| TITLE_FOREGROUND string { GetColor(color,
						&TitleC.fore, $2); }
		| TITLE_BACKGROUND string { GetColor(color,
						&TitleC.back, $2); }
		| MENU_FOREGROUND string { GetColor(color,
						&MenuC.fore, $2); }
		| MENU_BACKGROUND string { GetColor(color,
						&MenuC.back, $2); }
		| MENU_TITLE_FOREGROUND string { GetColor(color,
						    &MenuTitleC.fore, $2); }
		| MENU_TITLE_BACKGROUND string { GetColor(color,
						    &MenuTitleC.back, $2); }
		| MENU_SHADOW_COLOR string { GetColor(color,
						    &MenuShadowColor, $2); }
		| ICON_FOREGROUND string { GetColor(color,
						&IconC.fore, $2); }
		| ICON_BACKGROUND string { GetColor(color,
						&IconC.back, $2); }
		| ICON_BORDER_COLOR string { GetColor(color,
						&IconBorderColor, $2); }
		| ICONMGR_FOREGROUND string { GetColor(color,
						&IconManagerC.fore, $2); }
		| ICONMGR_BACKGROUND string { GetColor(color,
						&IconManagerC.back, $2); }
		;

win_list	: LB win_entries RB
		;

win_entries	: /* Empty */
		| win_entries win_entry
		;

win_entry	: string		{ if (FirstTime) AddToList(list, $1, 0); }
		;

icon_list	: LB icon_entries RB
		;

icon_entries	: /* Empty */
		| icon_entries icon_entry
		;

icon_entry	: string string		{   if (FirstTime)
					    { 
						Pixmap pm;
						
						pm = GetBitmap($2);
						if (pm != NULL)
						    AddToList(list, $1, pm);
					    }
					}
		;

function	: LB function_entries RB
		;

function_entries: /* Empty */
		| function_entries function_entry
		;

function_entry	: action		{ AddToMenu(root, "", Action, NULL, $1);
					  Action = "";
					}
		;

menu		: LB menu_entries RB
		;

menu_entries	: /* Empty */
		| menu_entries menu_entry
		;

menu_entry	: string action		{ AddToMenu(root, $1, Action, pull, $2);
					  Action = "";
					  pull = NULL;
					}
		;

action		: F_NOP			{ $$ = F_NOP; }
		| F_BEEP		{ $$ = F_BEEP; }
		| F_QUIT		{ $$ = F_QUIT; }
		| F_FOCUS		{ $$ = F_FOCUS; }
		| F_REFRESH		{ $$ = F_REFRESH; }
		| F_WINREFRESH		{ $$ = F_WINREFRESH; }
		| F_SOURCE string	{ Action = $2; $$ = F_TWMRC; }
		| F_MOVE		{ $$ = F_MOVE; }
		| F_FORCEMOVE		{ $$ = F_FORCEMOVE; }
		| F_ICONIFY		{ $$ = F_ICONIFY; }
		| F_DEICONIFY		{ $$ = F_DEICONIFY; }
		| F_UNFOCUS		{ $$ = F_UNFOCUS; }
		| F_RESIZE		{ $$ = F_RESIZE; }
		| F_ZOOM		{ $$ = F_ZOOM; }
		| F_FULLZOOM		{ $$ = F_FULLZOOM; }
		| F_RAISE		{ $$ = F_RAISE; }
		| F_RAISELOWER		{ $$ = F_RAISELOWER; }
		| F_LOWER		{ $$ = F_LOWER; }
		| F_DESTROY		{ $$ = F_DESTROY; }
		| F_TWMRC		{ $$ = F_TWMRC; }
		| F_VERSION		{ $$ = F_VERSION; }
		| F_TITLE		{ $$ = F_TITLE; }
		| F_CIRCLEUP		{ $$ = F_CIRCLEUP; }
		| F_CIRCLEDOWN		{ $$ = F_CIRCLEDOWN; }
		| F_CUTFILE		{ $$ = F_CUTFILE; }
		| F_SHOWLIST		{ $$ = F_SHOWLIST; }
		| F_HIDELIST		{ $$ = F_HIDELIST; }
		| F_AUTORAISE		{ $$ = F_AUTORAISE; }
		| F_MENU string		{ pull = GetRoot($2);
					  pull->prev = root;
					  $$ = F_MENU;
					}
		| F_FILE string		{ Action = $2; $$ = F_FILE; }
		| F_EXEC string		{ Action = $2; $$ = F_EXEC; }
		| F_CUT string		{ Action = $2; $$ = F_CUT; }
		| F_FUNCTION string	{ Action = $2; $$ = F_FUNCTION; }
		;

button		: BUTTON		{ $$ = $1;
					  if ($1 == 0)
						yyerror();

					  if ($1 > MAX_BUTTONS)
					  {
						$$ = 0;
						yyerror();
					  }
					}
		;
tbutton		: TBUTTON		{ $$ = $1;
					  if ($1 == 0)
						yyerror();

					  if ($1 > MAX_BUTTONS)
					  {
						$$ = 0;
						yyerror();
					  }
					}
		;

string		: STRING		{ ptr = (char *)malloc(strlen($1)+1);
					  strcpy(ptr, $1);
					  RemoveDQuote(ptr);
					  $$ = ptr;
					}
number		: NUMBER		{ $$ = $1; }
		;

%%
yyerror(s) char *s;
{
    fprintf(stderr, "twm: syntax error, line %d\n", yylineno);
    ParseError = 1;
}
RemoveDQuote(str)
char *str;
{
    strcpy(str, &str[1]);
    str[strlen(str)-1] = '\0';
}

MenuRoot *
GetRoot(name)
char *name;
{
    MenuRoot *tmp;

    tmp = FindMenuRoot(name);
    if (tmp == NULL)
	tmp = NewMenuRoot(name);

    return tmp;
}

