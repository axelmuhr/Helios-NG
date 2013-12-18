/*
 * $Header: /hsrc/cmds/public/stevie/RCS/stevie.h,v 1.2 1993/03/31 15:51:11 nickc Exp $
 *
 * Main header file included by all source files.
 */

#include "env.h"	/* defines to establish the compile-time environment */

#include <stdio.h>
#include <ctype.h>

#ifdef	BSD

#include <strings.h>
/* #define strchr index */

#else

#ifdef	MINIX

extern	char	*strchr();
extern	char	*strrchr();
extern	char	*strcpy();
extern	char	*strcat();
extern	int	strlen();

#else
#include <string.h>
#endif

#endif

#include "ascii.h"
#include "keymap.h"
#include "param.h"
#include "term.h"

extern	char	*strchr();

#define NORMAL 0
#define CMDLINE 1
#define INSERT 2
#define REPLACE 3
#define FORWARD 4
#define BACKWARD 5

/*
 * Boolean type definition and constants
 */
typedef	short	bool_t;

#ifndef	TRUE
#define	FALSE	(0)
#define	TRUE	(1)
#endif

/*
 * SLOP is the amount of extra space we get for text on a line during
 * editing operations that need more space. This keeps us from calling
 * malloc every time we get a character during insert mode. No extra
 * space is allocated when the file is initially read.
 */
#define	SLOP	10

/*
 * LINEINC is the gap we leave between the artificial line numbers. This
 * helps to avoid renumbering all the lines every time a new line is
 * inserted.
 */
#define	LINEINC	10

#define CHANGED		Changed=TRUE
#define UNCHANGED	Changed=FALSE

struct	line {
	struct	line	*prev, *next;	/* previous and next lines */
	char	*s;			/* text for this line */
	int	size;			/* actual size of space at 's' */
	unsigned long	num;		/* line "number" */
};

#define	LINEOF(x)	((x)->linep->num)

struct	lptr {
	struct	line	*linep;		/* line we're referencing */
	int	index;			/* position within that line */
};

typedef	struct line	LINE;
typedef	struct lptr	LPTR;

struct charinfo {
	char ch_size;
	char *ch_str;
};

extern struct charinfo chars[];

extern	int	State;
extern	int	Rows;
extern	int	Columns;
extern	char	*Realscreen;
extern	char	*Nextscreen;
extern	char	*Filename;
extern	LPTR	*Filemem;
extern	LPTR	*Filetop;
extern	LPTR	*Fileend;
extern	LPTR	*Topchar;
extern	LPTR	*Botchar;
extern	LPTR	*Curschar;
extern	LPTR	*Insstart;
extern	int	Cursrow, Curscol, Cursvcol, Curswant;
extern	bool_t	set_want_col;
extern	int	Prenum;
extern	bool_t	Changed;
extern	char	Redobuff[], Insbuff[];
extern	char	*Insptr;
extern	int	Ninsert;
extern	bool_t	got_int;

extern	char	*malloc(), *strcpy();

/*
 * alloc.c
 */
char	*alloc(), *strsave(), *mkstr();
int	screenalloc();
void	filealloc(), freeall();
LINE	*newline();
bool_t	bufempty(), buf1line(), lineempty(), endofline(), canincrease();

/*
 * cmdline.c
 */
void	docmdln(), msg(), emsg(), smsg(), gotocmd(), wait_return(), badcmd();
bool_t	doecmd();
char	*getcmdln();

/*
 * edit.c
 */
void	edit(), insertchar(), getout(), scrollup(), scrolldown(), beginline();
bool_t	oneright(), oneleft(), oneup(), onedown();

/*
 * fileio.c
 */
void	filemess(), renum();
bool_t	readfile(), writeit();

/*
 * help.c
 */
bool_t	help();

/*
 * linefunc.c
 */
LPTR	*nextline(), *prevline(), *coladvance(), *nextchar(), *prevchar();

/*
 * main.c
 */
void	stuffin(), stuffnum();
void	do_mlines();
int	vgetc();
bool_t	anyinput();

/*
 * mark.c
 */
void	setpcmark(), clrall(), clrmark();
bool_t	setmark();
LPTR	*getmark();

/*
 * misccmds.c
 */
void	opencmd(), fileinfo(), inschar(), delline();
bool_t	delchar();
int	cntllines(), plines();
LPTR	*gotoline();

/*
 * normal.c
 */
void	normal();

/*
 * param.c
 */
void	doset();

/*
 * ptrfunc.c
 */
int	inc(), dec();
int	gchar();
void	pchar(), pswap();
bool_t	lt(), equal(), ltoreq();
#if 0
/* not currently used */
bool_t	gtoreq(), gt();
#endif

/*
 * screen.c
 */
void	updatescreen(), updateline();
void	screenclear(), cursupdate();
void	s_ins(), s_del();
void	prt_line();

/*
 * search.c
 */
void	dosub(), doglob();
bool_t	searchc(), crepsearch(), findfunc(), dosearch(), repsearch();
LPTR	*showmatch();
LPTR	*fwd_word(), *bck_word(), *end_word();

/*
 * tagcmd.c
 */
void	dotag(), dountag();

/*
 * undo.c
 */
void	u_save(), u_saveline(), u_clear();
void	u_lcheck(), u_lundo();
void	u_undo();

/*
 * Machine-dependent routines.
 */
int	inchar();
void	flushbuf();
void	outchar(char c), outstr(), beep();
char	*fixname();
#ifndef	OS2
#ifndef	DOS
/*
void	remove(), rename();
*/
#endif
#endif
void	windinit(), windexit(), windgoto();
void	pause();
void	doshell();
