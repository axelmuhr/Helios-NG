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
*************************************************************************/

#define __in_globals	1		/* flag that we are in this module */

#include "window.h"

/*-------------------------  Global constants  -------------------------*/

tcap_def tcap_table[60] =		/* keys to look for in termcap	*/
{
  { "*1", "\x01b"    },			/* escape key (if not direct) */
  { "%5", "next"     },			/* next window key */
  { "%8", "prev"     },			/* previous window key */
  { "ku", "\x09bA"   },			/* up arrow */
  { "kd", "\x09bB"   },			/* down arrow */
  { "kr", "\x09bC"   },			/* right arrow */
  { "kl", "\x09bD"   },			/* left arrow */
  { "kR", "\x09bT~"  },			/* shifted up arrow */
  { "kF", "\x09bS~"  },			/* shifted down arrow */
  { "%i", "\x09b @~" },			/* shifted right arrow */
  { "#4", "\x09b A~" },			/* shifted left arrow */
  { "%1", "\x09b?~"  },			/* help key */
  { "&8", "\x09b1z"  },			/* undo */
  { "kh", "\x09bH"   },			/* home */
  { "kP", "\x09b3z"  },			/* Page up */
  { "@7", "\x09b2z"  },			/* end */
  { "kN", "\x09b4z"  },			/* Page down */
  { "kI", "\x09b@"   },			/* insert */
  { "kD", "\x07f"    },			/* delete */
  { "k1", "\x09b0~"  },			/* F1 */
  { "k2", "\x09b1~"  },			/* F2 */
  { "k3", "\x09b2~"  },			/* F3 */
  { "k4", "\x09b3~"  },			/* F4 */
  { "k5", "\x09b4~"  },			/* F5 */
  { "k6", "\x09b5~"  },			/* F6 */
  { "k7", "\x09b6~"  },			/* F7 */
  { "k8", "\x09b7~"  },			/* F8 */
  { "k9", "\x09b8~"  },			/* F9 */
  { "k;", "\x09b9~"  },			/* F10 */

  { "F1", "\x09b10~" },			/* F11 */
  { "F2", "\x09b11~" },			/* F12 */
  { "F3", "\x09b12~" },			/* F13 */
  { "F4", "\x09b13~" },			/* F14 */
  { "F5", "\x09b14~" },			/* F15 */
  { "F6", "\x09b15~" },			/* F16 */
  { "F7", "\x09b16~" },			/* F17 */
  { "F8", "\x09b17~" },			/* F18 */
  { "F9", "\x09b18~" },			/* F19 */
  { "FA", "\x09b19~" },			/* F20 */

  { "FB", "\x09b20~" },			/* F21 */
  { "FC", "\x09b21~" },			/* F22 */
  { "FD", "\x09b22~" },			/* F23 */
  { "FE", "\x09b23~" },			/* F24 */
  { "FF", "\x09b24~" },			/* F25 */
  { "FG", "\x09b25~" },			/* F26 */
  { "FH", "\x09b26~" },			/* F27 */
  { "FI", "\x09b27~" },			/* F28 */
  { "FJ", "\x09b28~" },			/* F29 */
  { "FK", "\x09b29~" },			/* F30 */

  { "FL", "\x09b30~" },			/* F31 */
  { "FM", "\x09b31~" },			/* F32 */
  { "FN", "\x09b32~" },			/* F33 */
  { "FO", "\x09b33~" },			/* F34 */
  { "FP", "\x09b34~" },			/* F35 */
  { "FQ", "\x09b35~" },			/* F36 */
  { "FR", "\x09b36~" },			/* F37 */
  { "FS", "\x09b37~" },			/* F38 */
  { "FT", "\x09b38~" },			/* F39 */
  { "FU", "\x09b39~" },			/* F40 */

  { "", "" }
};

char		*tcap_null = "";

/*-------------------------  Global variables  -------------------------*/

char		**environ;		/* environment pointer for tcap	*/
char		term_name[32];		/* name of terminal		*/
Stream		*line_in	= NULL;	/* terminal stream		*/
Stream		*line_out	= NULL;	/* screen stream		*/
Semaphore	Window_Lock;		/* lock for global window data	*/
int		Window_Count = 0;	/* number of open windows	*/
int		WindowID = 1;		/* id count for duplicate names	*/
Window		*Cur_Window = NULL;	/* current window		*/
Screen		*Cur_Screen = NULL;	/* current screen		*/
Keyboard	*Cur_Keyboard = NULL;	/* current terminal		*/
Semaphore	Input_Lock;		/* stop input process		*/
bool		Input_Reset;		/* forget about matched input	*/
int		ospeed;			/* variables for termacp	*/
int		PC;

char		tcap_data[1024];	/* termcap data buffer		*/
char		tcap_buff[1024];	/* termcap string buffer	*/
char		*tcap_index;		/* pointer to string buffer	*/

char		*tcap_init;		/* init string			*/
char		*tcap_inif;		/* file containing init string	*/
char		*tcap_rest;		/* reset string			*/
char		*tcap_resf;		/* file containing reset string	*/

char		*tcap_clrs;		/* clear whole screen		*/
char		*tcap_ceos;		/* clear to end of screen	*/
char		*tcap_ceol;		/* clear to end of line		*/
char		*tcap_isln;		/* insert n lines above current	*/
char		*tcap_dlln;		/* delete n lines		*/
char		*tcap_insl;		/* insert line above current	*/
char		*tcap_dell;		/* delete current line		*/
char		*tcap_iscn;		/* insert n characters		*/
char		*tcap_dlcn;		/* delete n characters		*/
char		*tcap_insc;		/* insert character		*/
char		*tcap_delc;		/* delete current character	*/
char		*tcap_norm;		/* normal attributes		*/
char		*tcap_bld;		/* bold				*/
char		*tcap_dim;		/* dimmed			*/
char		*tcap_bli;		/* blinking			*/
char		*tcap_rev;		/* reversed			*/
char		*tcap_ulon;		/* underlined on		*/
char		*tcap_ulof;		/* underlined off		*/
char		*tcap_goto;		/* cursor to row [m] col [n]	*/
char		*tcap_bell;		/* audible or visible bell	*/

bool		term_wraps;		/* wrap on end of the line	*/
int		term_rows;		/* number of rows		*/
int		term_maxrow;		/* for quicker action		*/
int		term_cols;		/* number of coloumns		*/
int		term_maxcol;		/* for quicker action		*/

char		*term_output = NULL;	/* ansi output buffer		*/
int		term_ocount = 0;	/* ansi buffer counter		*/

key_def 	*key_table = NULL;	/* table of defined keys	*/
int		key_tablen = 0;		/* length of that table		*/

int		opterr = 1;		/* enable error messages	*/
int		optind = 1;		/* index into argv vector	*/
int		optopt;			/* char checked for validity	*/
char		*optarg;		/* option's associated argument */
word		totalmem = 0;		/* amount of allocated memory	*/

/*--- end of globals.c ---*/
