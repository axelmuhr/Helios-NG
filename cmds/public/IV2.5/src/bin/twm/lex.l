%{
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
 * $Header: lex.l,v 1.43.1.1 89/03/20 13:51:47 interran Exp $
 *
 * .twmrc lex file
 *
 * 12-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

static char RCSinfo[]=
"$Header: lex.l,v 1.43.1.1 89/03/20 13:51:47 interran Exp $";

#include <stdio.h>
#include "gram.h"

extern int ParseError;

%}
%k 4000
%e 3000
%p 6000
%n 4000
%a 3000

qstring				\"[^"]*\"
number				[0-9]+
button				[Bb][Uu][Tt][Tt][Oo][Nn]
tbutton				[Tt][Ii][Tt][Ll][Ee][Bb][Uu][Tt][Tt][Oo][Nn]
noraise				[Nn][Oo][Rr][Aa][Ii][Ss][Ee][Oo][Nn]
%%
"{"				{ return (LB); }
"}"				{ return (RB); }
"="				{ return (EQUALS); }
":"				{ return (COLON); }

{button}{number}		{ (void)sscanf(&yytext[6], "%d", &yylval.num);
				  return (BUTTON);
				}
{tbutton}{number}		{ (void)sscanf(&yytext[11], "%d", &yylval.num);
				  return (TBUTTON);
				}
[Mm][Ee][Nn][Uu]		{ return MENU; }
[Ff][Uu][Nn][Cc][Tt][Ii][Oo][Nn]  { return FUNCTION; }

m				{ return (META); }
s				{ return (SHIFT); }
c				{ return (CONTROL); }

window				{ return (WINDOW); }
icon				{ return (ICON); }
title				{ return (TITLE); }
root				{ return (ROOT); }
frame				{ return (FRAME); }
iconmgr				{ return (ICONMGR); }

f.nop				{ return F_NOP; }
f.beep				{ return F_BEEP; }
f.quit				{ return F_QUIT; }
f.title				{ return F_TITLE ; }
f.menu				{ return F_MENU; }
f.unfocus			{ return F_UNFOCUS; }
f.refresh			{ return F_REFRESH; }
f.winrefresh			{ return F_WINREFRESH; }
f.file				{ return F_FILE; }
f.twmrc				{ return F_TWMRC; }
f.version			{ return F_VERSION; }
f.circleup			{ return F_CIRCLEUP; }
f.circledown			{ return F_CIRCLEDOWN; }
f.source			{ return F_SOURCE; }
f.cutfile			{ return F_CUTFILE; }
f.function			{ return F_FUNCTION; }
f.showiconmgr			{ return F_SHOWLIST; }
f.hideiconmgr			{ return F_HIDELIST; }
"!"				{ return F_EXEC; }
"^"				{ return F_CUT; }
f.move				{ return F_MOVE; }
f.forcemove			{ return F_FORCEMOVE; }
f.iconify			{ return F_ICONIFY; }
f.deiconify			{ return F_DEICONIFY; }
f.focus				{ return F_FOCUS; }
f.resize			{ return F_RESIZE; }
f.raise				{ return F_RAISE; }
f.raiselower			{ return F_RAISELOWER; }
f.lower				{ return F_LOWER; }
f.destroy			{ return F_DESTROY; }
f.zoom				{ return F_ZOOM; }
f.fullzoom			{ return F_FULLZOOM; }
f.autoraise			{ return F_AUTORAISE; }


t.nop				{ return F_NOP; }
t.raise				{ return F_RAISE; }
t.lower				{ return F_LOWER; }
t.move				{ return F_MOVE; }

{noraise}[Mm][Oo][Vv][Ee]	{ return NO_RAISE_ON_MOVE; }
{noraise}[Dd][Ee][Ii][Cc][Oo][Nn][Ii][Ff][Yy] { return NO_RAISE_ON_DEICONIFY; }
{noraise}[Rr][Ee][Ss][Ii][Zz][Ee] { return NO_RAISE_ON_RESIZE; }
[Cc][Oo][Ll][Oo][Rr]		{ return (COLOR); }
[Mm][Oo][Nn][Oo][Cc][Hh][Rr][Oo][Mm][Ee] { return (MONOCHROME); }
[Nn][Oo][Tt][Ii][Tt][Ll][Ee][Ff][Oo][Cc][Uu][Ss] { return (NO_TITLE_FOCUS); }
[Nn][Oo][Hh][Ii][Gg][Hh][Ll][Ii][Gg][Hh][Tt] { return (NO_HILITE); }
[Zz][Oo][Oo][Mm]		{ return (ZOOM); }
[Uu][Nn][Kk][Nn][Oo][Ww][Nn][Ii][Cc][Oo][Nn] { return UNKNOWN_ICON; }
[Ww][Aa][Rr][Pp][Cc][Uu][Rr][Ss][Oo][Rr] { return WARPCURSOR; }
[Bb][Oo][Rr][Dd][Ee][Rr][Ww][Ii][Dd][Tt][Hh] { return BORDERWIDTH; }
[Tt][Ii][Tt][Ll][Ee][Ff][Oo][Nn][Tt] { return (TITLE_FONT); }
[Mm][Ee][Nn][Uu][Ff][Oo][Nn][Tt] { return (MENU_FONT); }
[Ii][Cc][Oo][Nn][Ff][Oo][Nn][Tt] { return (ICON_FONT); }
[Rr][Ee][Ss][Ii][Zz][Ee][Ff][Oo][Nn][Tt] { return (RESIZE_FONT); }
[Rr][Ee][Vv][Ee][Rr][Ss][Ee][Vv][Ii][Dd][Ee][Oo] { return (REVERSE_VIDEO); }
[Nn][Oo][Tt][Ii][Tt][Ll][Ee]	{ return (NO_TITLE); }
[Aa][Uu][Tt][Oo][Rr][Aa][Ii][Ss][Ee]	{ return (AUTO_RAISE); }
[Ff][Oo][Rr][Cc][Ee][Ii][Cc][Oo][Nn][Ss]	{ return (FORCE_ICON); }
[Dd][Ee][Ff][Aa][Uu][Ll][Tt][Ff][Uu][Nn][Cc][Tt][Ii][Oo][Nn] {
			       return (DEFAULT_FUNCTION); }
[Ww][Ii][Nn][Dd][Oo][Ww][Ff][Uu][Nn][Cc][Tt][Ii][Oo][Nn] {
			       return (WINDOW_FUNCTION); }
[Ii][Cc][Oo][Nn][Ss]			{ return (ICONS); }
[Ii][Cc][Oo][Nn][Dd][Ii][Rr][Ee][Cc][Tt][Oo][Rr][Yy] {return (ICON_DIRECTORY);}
[Bb][Oo][Rr][Dd][Ee][Rr][Cc][Oo][Ll][Oo][Rr] { return (BORDER_COLOR); }
[Tt][Ii][Tt][Ll][Ee][Ff][Oo][Rr][Ee][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (TITLE_FOREGROUND); }
[Tt][Ii][Tt][Ll][Ee][Bb][Aa][Cc][Kk][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (TITLE_BACKGROUND); }
[Bb][Oo][Rr][Dd][Ee][Rr][Tt][Ii][Ll][Ee][Ff][Oo][Rr][Ee][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (BORDER_TILE_FOREGROUND); }
[Bb][Oo][Rr][Dd][Ee][Rr][Tt][Ii][Ll][Ee][Bb][Aa][Cc][Kk][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (BORDER_TILE_BACKGROUND); }
[Mm][Ee][Nn][Uu][Ff][Oo][Rr][Ee][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (MENU_FOREGROUND); }
[Mm][Ee][Nn][Uu][Bb][Aa][Cc][Kk][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (MENU_BACKGROUND); }
[Mm][Ee][Nn][Uu][Tt][Ii][Tt][Ll][Ee][Ff][Oo][Rr][Ee][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (MENU_TITLE_FOREGROUND); }
[Mm][Ee][Nn][Uu][Tt][Ii][Tt][Ll][Ee][Bb][Aa][Cc][Kk][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (MENU_TITLE_BACKGROUND); }
[Mm][Ee][Nn][Uu][Ss][Hh][Aa][Dd][Oo][Ww][Cc][Oo][Ll][Oo][Rr] {
				return (MENU_SHADOW_COLOR); }
[Ii][Cc][Oo][Nn][Ff][Oo][Rr][Ee][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (ICON_FOREGROUND); }
[Ii][Cc][Oo][Nn][Bb][Aa][Cc][Kk][Gg][Rr][Oo][Uu][Nn][Dd] {
				return (ICON_BACKGROUND); }
[Ii][Cc][Oo][Nn][Bb][Oo][Rr][Dd][Ee][Rr][Cc][Oo][Ll][Oo][Rr] {
				return (ICON_BORDER_COLOR); }
[Dd][Oo][Nn][Tt][Mm][Oo][Vv][Ee][Oo][Ff][Ff] return (DONT_MOVE_OFF); }
[Dd][Ee][Cc][Oo][Rr][Aa][Tt][Ee][Tt][Rr][Aa][Nn][Ss][Ii][Ee][Nn][Tt][Ss] {
				return (DECORATE_TRANSIENTS); }
[Ii][Cc][Oo][Nn][Ii][Ff][Yy][Bb][Yy][Uu][Nn][Mm][Aa][Pp][Pp][Ii][Nn][Gg] { return (ICONIFY_BY_UNMAPPING); }
[Dd][Oo][Nn][Tt][Ii][Cc][Oo][Nn][Ii][Ff][Yy][Bb][Yy][Uu][Nn][Mm][Aa][Pp][Pp][Ii][Nn][Gg] { return (DONT_ICONIFY_BY_UNMAPPING); }
[Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr][Ff][Oo][Rr][Ee][Gg][Rr][Oo][Uu][Nn][Dd] { return (ICONMGR_FOREGROUND); }
[Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr][Bb][Aa][Cc][Kk][Gg][Rr][Oo][Uu][Nn][Dd] { return (ICONMGR_BACKGROUND); }
[Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr][Ff][Oo][Nn][Tt] { return (ICONMGR_FONT); }
[Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr][Gg][Ee][Oo][Mm][Ee][Tt][Rr][Yy] { return (ICONMGR_GEOMETRY); }
[Ss][Hh][Oo][Ww][Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr] { return (SHOW_ICONMGR); }
[Ii][Cc][Oo][Nn][Mm][Aa][Nn][Aa][Gg][Ee][Rr][Dd][Oo][Nn][Tt][Ss][Hh][Oo][Ww] { return ICONMGR_NOSHOW; }
[Nn][Oo][Bb][Aa][Cc][Kk][Ii][Nn][Gg][Ss][Tt][Oo][Rr][Ee] { return NO_BACKINGSTORE; }
[Nn][Oo][Ss][Aa][Vv][Ee][Uu][Nn][Dd][Ee][Rr] { return NO_SAVEUNDER; }
[Rr][Aa][Nn][Dd][Oo][Mm][Pp][Ll][Aa][Cc][Ee][Mm][Ee][Nn][Tt] { return RANDOM_PLACEMENT; }
{qstring}			{ yylval.ptr = yytext; return STRING; }
{number}			{ (void)sscanf(yytext, "%d", &yylval.num);
				  return (NUMBER);
				}
\#[^\n]*\n			{;}
[\n\t ]				{;}
.				{
				  fprintf(stderr,
					"twm: bad character `%s', line %d\n",
					yytext, yylineno);
				  ParseError = 1;
				}
%%
yywrap() { return(1);}

#define unput(c)	TwmUnput(c)
#define input()		TwmInput()
#define output(c)	TwmOutput(c)
