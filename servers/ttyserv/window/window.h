/*************************************************************************
**									**
**	       C O N S O L E  &  W I N D O W   S E R V E R		**
**	       -------------------------------------------		**
**									**
**		    Copyright (C) 1989, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** window.h								**
**									**
**	- Definitions and prototyping for the Window Server		**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	16/11/89 : C. Fleischer					**
** Changed   :	17/01/90 : C. Fleischer	 global variables -> globals.c	**
** Changed   :	20/04/90 : C. Fleischer	 inserted window.ddf, termcap,h	**
** Changed   :  08/08/90 : C. Fleischer  tempmap variable for scrolling	**
*************************************************************************/

#ifndef	__window_h
#define	__window_h

#include <helios.h>			/* standard header		*/
#include <stdio.h>
#include <stdlib.h>
#include <syslib.h>
#include <servlib.h>
#include <string.h>
#include <root.h>
#include <message.h>
#include <link.h>
#include <task.h>
#include <sem.h>
#include <protect.h>
#include <codes.h>
#include <gsp.h>
#include <process.h>
#include <attrib.h>
#include <ioevents.h>
#include <nonansi.h>

#include "../debug/debug.h"

/*---------------------------  Debug areas  ----------------------------*/

#define	TCAP		0x00000003
#define	TCAP_ALL	0x00000002
#define INPUT		0x0000000C
#define INPUT_ALL	0x00000008
#define TERM		0x00000030
#define TERM_ALL	0x00000020
#define	ANSI		0x000000C0
#define	ANSI_ALL	0x00000080
#define READ		0x00000300
#define	READ_ALL	0x00000200
#define	WRITE		0x00000C00
#define	WRITE_ALL	0x00000800
#define	OPEN		0x00003000
#define	CREATE		0x0000C000
#define	DELETE		0x00030000
#define	ATTR		0x000C0000
#define	SETTERM		0x00300000
#define	CBREAK		0x00C00000
#define	MAIN		0x03000000
#define	MAIN_ALL	0x02000000

/*---------------------  Definitions for typedefs  ---------------------*/

#define OutBufSize	256		/* size of output buffer	*/
#define InBufSize	256		/* size of input buffer 	*/
#define InBufMask	255		/* mask for buffer pointers	*/
#define CookSize	512		/* size of cooker buffer	*/

/*----------------  Definitions for the Ansi Emulator  -----------------*/

#define	MaxBufSize	4096		/* size limit for write buffer	*/

/* flag values								*/

#define	ANSI_in_escape		0x01	/* an escape seen (CSI?)	*/
#define	ANSI_escape_found	0x02	/* a CSI found			*/
#define	ANSI_dirty		0x04	/* last screen char pending	*/

/* switch window codes							*/

#define ANSI_last		0	/* switch to last window	*/
#define ANSI_next		1	/* switch to next window	*/

/* attribute codes							*/

#define ANSI_mode_normal	0x00
#define ANSI_mode_bold		0x01
#define ANSI_mode_dimmed	0x02
#define ANSI_mode_underline	0x04
#define ANSI_mode_blink		0x08
#define ANSI_mode_reverse	0x10

#define NoTimeout	-1		/* wait forever			*/
#define ShortTimeout	OneSec / 2	/* timeout for a CSI		*/
#define TermBufSize	4096		/* size of Ansi terminal buffer */

#define ThisScreen	( s == Cur_Screen )

/*----------------  Definitions for the Window Server  -----------------*/

#define	StackSize	2000		/* default stack size for Fork	*/
#define MaxArgs		5		/* maximum number of Ansi args	*/
#define	Cooked_EOF	1		/* ctrl-D detected		*/
#define	Cooked_Done	2		/* Read has been satisfied	*/
#define Cooked_CtrlC	4		/* ctrl-C event sent		*/

#define FG_Reinit	0x00001fd0	/* Reinitialise Terminal tables	*/

					/* Comparison arg for qsort ()	*/
typedef int	( *CompFnPtr ) ( const void *, const void );

typedef struct tcap_def			/* termcap entry definition	*/
{
    char     *tcap_name;
    char     *helios_sequence;
}
tcap_def;

typedef struct key_def			/* local key definition		*/
{
    char    *local_sequence;
    char    *helios_sequence;
}
key_def;

typedef struct key_node			/* key definition node		*/
{
    Node    node;
    char    *local_sequence;
    char    *helios_sequence;
}
key_node;

typedef struct Screen
{
    char	**map;			/* screen map lines		*/
    char	*screen;		/* whole screen			*/
    int		Cur_x;			/* current x position		*/
    int		Cur_y;			/* current y position		*/
    int 	flags;			/* Ansi status flags		*/
    int		mode;			/* screen mode: inverse, bold..	*/
    int 	argv[MaxArgs + 1];	/* escape sequence arguments	*/
    int		args;			/* started arguments counter	*/
    int 	argc;			/* complete arguments counter	*/
    int		Rows;			/* number of screen rows	*/
    int		Cols;			/* number of screen cols	*/
    char	**tempmap;		/* 2nd screen map for scrolling	*/
}
Screen;

typedef struct Keyboard
{
    Semaphore	raw_lock;		/* lock raw input buffer	*/
    char	in_raw[InBufSize];	/* circular raw input buffer	*/
    int 	in_head;		/* buffer head			*/
    int 	in_tail;		/* buffer tail			*/
    char	in_cook[CookSize];	/* buffer to cook input 	*/
    int 	in_count;		/* cook buffer counter		*/
    int 	in_flags;		/* Ansi status flags		*/
}
Keyboard;

typedef struct Window
{
    ObjNode		ObjNode;	/* directory node		*/
    struct Window	*Next;		/* window chaining pointers	*/
    struct Window	*Prev;
    Attributes		Attribs;	/* current attributes		*/
    bool		echo;		/* TRUE for echo mode		*/
    bool		raw_in;		/* TRUE for raw input mode	*/
    bool		raw_out;	/* TRUE for raw output mode	*/
    bool		Xoff;		/* window processing suspended	*/
    Port		EventPort;	/* port to send events to	*/
    int			EventCount;	/* count field for stream ident	*/
    Semaphore		ReadLock;	/* prevent multiple Read reqs.	*/
    Semaphore		WriteLock;	/* prevent multiple Write reqs.	*/
    Screen		Screen;		/* window output data		*/
    Keyboard		Keyboard;	/* window input data		*/
}
Window;

/*-------------------------  Global constants  -------------------------*/

extern tcap_def tcap_table [];		/* keys to look for in termcap	*/
extern char	*tcap_null;

extern key_def	empty_table [];		/* empty table			*/

/*-------------------------  Global variables  -------------------------*/

extern char		**environ;	/* Environment pointer		*/
extern char		term_name[];	/* name of terminal		*/
extern Stream		*line_in;	/* terminal stream		*/
extern Stream		*line_out;	/* screen stream		*/
extern Semaphore	Window_Lock;	/* lock for global window data	*/
extern int		Window_Count;	/* number of open windows	*/
extern int		WindowID;	/* id count for duplicate names	*/
extern Window		*Cur_Window;	/* current window		*/
extern Screen		*Cur_Screen;	/* current screen		*/
extern Keyboard		*Cur_Keyboard;	/* current terminal		*/
extern Semaphore	Input_Lock;	/* stop input process		*/
extern bool		Input_Reset;	/* forget about matched input	*/
extern char		tcap_data[];	/* termcap data buffer		*/
extern char		tcap_buff[];	/* termcap string buffer	*/
extern char		*tcap_index;	/* pointer to string buffer	*/

extern char		*tcap_init;	/* init string			*/
extern char		*tcap_inif;	/* file containing init string	*/
extern char		*tcap_rest;	/* reset string			*/
extern char		*tcap_resf;	/* file containing reset string	*/

extern char		*tcap_clrs;	/* clear whole screen		*/
extern char		*tcap_ceos;	/* clear to end of screen	*/
extern char		*tcap_ceol;	/* clear to end of line		*/
extern char		*tcap_isln;	/* insert n lines above current	*/
extern char		*tcap_dlln;	/* delete n lines		*/
extern char		*tcap_insl;	/* insert line above current	*/
extern char		*tcap_dell;	/* delete current line		*/
extern char		*tcap_iscn;	/* insert n characters		*/
extern char		*tcap_dlcn;	/* delete n characters		*/
extern char		*tcap_insc;	/* insert character		*/
extern char		*tcap_delc;	/* delete current character	*/
extern char		*tcap_norm;	/* normal attributes		*/
extern char		*tcap_bld;	/* bold				*/
extern char		*tcap_dim;	/* dimmed			*/
extern char		*tcap_bli;	/* blinking			*/
extern char		*tcap_rev;	/* reversed			*/
extern char		*tcap_ulon;	/* underlined on		*/
extern char		*tcap_ulof;	/* underlined off		*/
extern char		*tcap_goto;	/* cursor to row [m] col [n]	*/
extern char		*tcap_bell;	/* audible or visible bell	*/

extern bool		term_wraps;	/* wrap on end of the line	*/
extern int		term_rows;	/* number of rows		*/
extern int		term_maxrow;	/* for quicker action		*/
extern int		term_cols;	/* number of coloumns		*/
extern int		term_maxcol;	/* for quicker action		*/

extern char		*term_output;	/* ansi output buffer		*/
extern int		term_ocount;	/* ansi buffer counter		*/

extern key_def 		*key_table;	/* table of defined keys	*/
extern int		key_tablen;	/* length of that table		*/

extern int		opterr;		/* enable error messages	*/
extern int		optind;		/* index into argv vector	*/
extern int		optopt;		/* char checked for validity	*/
extern char		*optarg;	/* option's associated argument */
extern word		totalmem;	/* amount of allocated memory	*/

/*------------------------  additional functions  ----------------------*/

extern int		getopt		( int argc, char **argv, char *optmask );
extern int		getint		( char *nptr );
extern void *		newmem		( word size );
extern void		freemem		( void *mem );

/*-------------------------  termcap functions  ------------------------*/

extern int		tgetent		( char *bp, char *name );
extern int		tgetnum		( char *id );
extern int		tgetflag	( char *id );
extern char *		tgetstr		( char *id, char **area );
extern char *		tgoto		( char *cm, int destcol, int destlin );

/*---------------------  Terminal output functions  --------------------*/

extern void		TermFlush	( void );
extern void		TermGoto	( int x, int y );
extern void		TermClearScreen	( void );
extern void		TermClearEol	( void );
extern void		TermClearEos	( void );
extern void		TermRefresh	( int start_y, int end_y );
extern void		TermLineFeed	( bool scroll );
extern void		TermSetRendition ( int mode );
extern void		TermBell	( void );
extern void		TermInsertLines	( int count );
extern void		TermDeleteLines	( int count );
extern void		TermInsertChars	( int count );
extern void		TermDeleteChars	( int count );
extern void		TermChar	( char ch );
extern void		TermInit	( void );

extern void		RedrawScreen	( Window *w );

/*---------------------  Terminal input functions  ---------------------*/

extern void		HandleInput	( Stream *line );
extern void		InputInit	( void );

/*----------------------  Ansi emulator functions  ---------------------*/

extern bool		AnsiInit	( char *terminal );
extern bool		AnsiReinit	( char *terminal );
extern word		AnsiInitScreen	( Screen *s );
extern void		AnsiTidyScreen	( Screen *s );
extern word		AnsiReinitScreen ( Screen *s );
extern void		AnsiWrite	( Screen *s, char ch );
extern void		AnsiWriteData	( Screen *s, char *data, int count );
extern void		AnsiFlush	( Screen *s );

/*----------------------  Window server functions  ---------------------*/

extern void		SetCurrentWindow ( Window *w );

extern void		Do_Private	( ServInfo *servinfo );

extern void		Do_Open		( ServInfo *servinfo );
extern void		Do_Create	( ServInfo *servinfo );
extern void		Do_Delete	( ServInfo *servinfo );

extern void		WindowRead	( MCB *m, Window *w, word *sflags );
extern void		WindowWrite	( MCB *m, Window *w );
extern void		WindowGetInfo	( MCB *m, Window *w );
extern void		WindowSetInfo	( MCB *m, Window *w );
extern void		WindowEnableEvents ( MCB *m, Window *w, int *id );


#endif

/*--- end of window.h ---*/
