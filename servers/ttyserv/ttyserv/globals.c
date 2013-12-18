/*************************************************************************
**									**
**	       C O N S O L E  &  W I N D O W   S E R V E R		**
**	       -------------------------------------------		**
**									**
**		    Copyright (C) 1989, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** globals.c								**
**									**
**	- Global variables for the Window Server			**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	17/01/90 : C. Fleischer					**
** Changed   :	12/09/90 : G. Jodlauk	 for use with TTY		**
*************************************************************************/

#define __in_globals	1		/* flag that we are in this module */

#include "tty.h"

/*-------------------------  Global constants  -------------------------*/

tcap_def tcap_table[60] =		/* keys to look for in termcap	*/
{
  { "*1", "\x01b"    },			/* escape key (if not direct) */
  { "%5", "next"     },			/* next window key */
  { "%8", "prev"     },			/* previous window key */
  { "ku", "\x09b""A"   },		/* up arrow */
  { "kd", "\x09b""B"   },		/* down arrow */
  { "kr", "\x09b""C"   },		/* right arrow */
  { "kl", "\x09b""D"   },		/* left arrow */
  { "kR", "\x09b""T~"  },		/* shifted up arrow */
  { "kF", "\x09b""S~"  },		/* shifted down arrow */
  { "%i", "\x09b"" @~" },		/* shifted right arrow */
  { "#4", "\x09b"" A~" },		/* shifted left arrow */
  { "%1", "\x09b""?~"  },		/* help key */
  { "&8", "\x09b""1z"  },		/* undo */
  { "kh", "\x09b""H"   },		/* home */
  { "kP", "\x09b""3z"  },		/* Page up */
  { "@7", "\x09b""2z"  },		/* end */
  { "kN", "\x09b""4z"  },		/* Page down */
  { "kI", "\x09b""@"   },		/* insert */
  { "kD", "\x07f"      },		/* delete */
  { "k1", "\x09b""0~"  },		/* F1 */
  { "k2", "\x09b""1~"  },		/* F2 */
  { "k3", "\x09b""2~"  },		/* F3 */
  { "k4", "\x09b""3~"  },		/* F4 */
  { "k5", "\x09b""4~"  },		/* F5 */
  { "k6", "\x09b""5~"  },		/* F6 */
  { "k7", "\x09b""6~"  },		/* F7 */
  { "k8", "\x09b""7~"  },		/* F8 */
  { "k9", "\x09b""8~"  },		/* F9 */
  { "k;", "\x09b""9~"  },		/* F10 */

  { "F1", "\x09b""10~" },		/* F11 */
  { "F2", "\x09b""11~" },		/* F12 */
  { "F3", "\x09b""12~" },		/* F13 */
  { "F4", "\x09b""13~" },		/* F14 */
  { "F5", "\x09b""14~" },		/* F15 */
  { "F6", "\x09b""15~" },		/* F16 */
  { "F7", "\x09b""16~" },		/* F17 */
  { "F8", "\x09b""17~" },		/* F18 */
  { "F9", "\x09b""18~" },		/* F19 */
  { "FA", "\x09b""19~" },		/* F20 */

  { "FB", "\x09b""20~" },		/* F21 */
  { "FC", "\x09b""21~" },		/* F22 */
  { "FD", "\x09b""22~" },		/* F23 */
  { "FE", "\x09b""23~" },		/* F24 */
  { "FF", "\x09b""24~" },		/* F25 */
  { "FG", "\x09b""25~" },		/* F26 */
  { "FH", "\x09b""26~" },		/* F27 */
  { "FI", "\x09b""27~" },		/* F28 */
  { "FJ", "\x09b""28~" },		/* F29 */
  { "FK", "\x09b""29~" },		/* F30 */

  { "FL", "\x09b""30~" },		/* F31 */
  { "FM", "\x09b""31~" },		/* F32 */
  { "FN", "\x09b""32~" },		/* F33 */
  { "FO", "\x09b""33~" },		/* F34 */
  { "FP", "\x09b""34~" },		/* F35 */
  { "FQ", "\x09b""35~" },		/* F36 */
  { "FR", "\x09b""36~" },		/* F37 */
  { "FS", "\x09b""37~" },		/* F38 */
  { "FT", "\x09b""38~" },		/* F39 */
  { "FU", "\x09b""39~" },		/* F40 */

  { "", "" }
};

char		*tcap_null = "";

/*-------------------------  Global variables  -------------------------*/

char		term_name[ 32 ];	/* name of terminal		*/
char		my_nte[    32 ];	/* my actual name table entry	*/
DCB *		termdcb 	= NULL; /* DCB for pty device		*/
Stream *	line_in		= NULL;	/* terminal stream		*/
Stream *	line_out	= NULL;	/* screen stream		*/
Semaphore	Window_Lock;		/* lock for global window data	*/
Semaphore	Attr_Lock;		/* lock for Attributes		*/
int		Window_Count	= 0;	/* number of open windows	*/
int		WindowID	= 1;	/* id count for duplicate names	*/
Window *	Cur_Window	= NULL;	/* current window		*/
Screen *	Cur_Screen	= NULL;	/* current screen		*/
Keyboard *	Cur_Keyboard	= NULL;	/* current terminal		*/
Semaphore	Input_Lock;		/* stop input process		*/
bool		Input_Reset;		/* forget about matched input	*/
int		ospeed;			/* variables for termacp	*/
int		PC;

char		tcap_data[ 1024 ];	/* termcap data buffer		*/
char		tcap_buff[ 1024 ];	/* termcap string buffer	*/
char *		tcap_index;		/* pointer to string buffer	*/

char *		tcap_init;		/* init string			*/
char *		tcap_inif;		/* file containing init string	*/
char *		tcap_rest;		/* reset string			*/
char *		tcap_resf;		/* file containing reset string	*/

char *		tcap_clrs;		/* clear whole screen		*/
char *		tcap_ceos;		/* clear to end of screen	*/
char *		tcap_ceol;		/* clear to end of line		*/
char *		tcap_isln;		/* insert n lines above current	*/
char *		tcap_dlln;		/* delete n lines		*/
char *		tcap_insl;		/* insert line above current	*/
char *		tcap_dell;		/* delete current line		*/
char *		tcap_iscn;		/* insert n characters		*/
char *		tcap_dlcn;		/* delete n characters		*/
char *		tcap_insc;		/* insert character		*/
char *		tcap_delc;		/* delete current character	*/
char *		tcap_norm;		/* normal attributes		*/
char *		tcap_bld;		/* bold				*/
char *		tcap_dim;		/* dimmed			*/
char *		tcap_bli;		/* blinking			*/
char *		tcap_rev;		/* reversed			*/
char *		tcap_ulon;		/* underlined on		*/
char *		tcap_ulof;		/* underlined off		*/
char *		tcap_goto;		/* cursor to row [m] col [n]	*/
char *		tcap_bell;		/* audible or visible bell	*/

bool		term_wraps;		/* wrap on end of the line	*/
int		term_rows;		/* number of rows		*/
int		term_maxrow;		/* for quicker action		*/
int		term_cols;		/* number of coloumns		*/
int		term_maxcol;		/* for quicker action		*/

char *		term_output = NULL;	/* ansi output buffer		*/
int		term_ocount = 0;	/* ansi buffer counter		*/

key_def *	key_table = NULL;	/* table of defined keys	*/
int		key_tablen = 0;		/* length of that table		*/

int		optopt;			/* char checked for validity	*/
word		totalmem = 0;		/* amount of allocated memory	*/

/*--- end of globals.c ---*/
