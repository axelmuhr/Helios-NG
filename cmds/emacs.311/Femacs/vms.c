/*
 *  Advanced VMS terminal driver
 *
 *  Consults the system's terminal tables for both DEC terminals,
 *  foreign and user defined terminals.
 *
 *  Author:  Curtis Smith
 *  Update history:
 *	14-Jul-1987, first revision.
 *	02-Jan-1988, by J. A. Lomicka: Code addition for interpreting
 *		LK201 function keys, application keypad and cursor
 *		position reports.
 *	09-Apr-1988, second revision.
 *		Major changes:
 *		1) H files removed; replaced with globalvalues.
 *		2) Terminal now left in ECHO mode. Read call
 *		disables echo.
 *		3) TYPAHD macro now performs correctly.
 *		4) $sres variable now accepts NORMAL and WIDE.
 *		5) Writes to screen now use QIO without waiting.
 *		This gives a slight increase in performance.
 *		Had to make some QIOW a different event flag number.
 *		6) Function keys, special keys and arrow keys
 *		are now bound using the following table:
 *			FNa-DOWN FNn-5    FN0-     FNA-F10  FNN-E1
 *			FNb-LEFT FNo-6    FN1-F1   FNB-F11  FNO-E2
 *			FNc-RITE FNp-7    FN2-F2   FNC-F12  FNP-E3
 *			FNd-UP   FNq-8    FN3-F3   FND-F13  FNQ-E4
 *			FNe-PF1  FNr-9    FN4-F4   FNE-F14  FNR-E5
 *			FNf-PF2  FNs-.    FN5-F5   FNF-F15  FNS-E6
 *			FNg-PF3  FNt-ENT  FN6-F6   FNG-F16  FNT-
 *			FNh-PF4  FNu-,    FN7-F7   FNH-F17  FNU-
 *			FNi-0    FNv--    FN8-F8   FNI-F18  FNV-
 *			FNj-1    FNw-     FN9-F9   FNJ-F19  FNW-
 *			FNk-2    FNx-              FNK-F20  FNX-
 *			FNl-3    FNy-              FNL-     FNY-
 *			FNm-4    FNz-              FNM-     FNZ-
 *		See ebind.h for key bindings.
 *	2-dec-88 Curtis Smith
 *	- These have been rebound to the new machine independant bindings
 */
 
/** Standard include files **/
#include <stdio.h>			/* Standard I/O package		*/
#include "estruct.h"			/* Emacs' structures		*/
#include "etype.h"
#include "edef.h"			/* Emacs' definitions		*/
#include	"elang.h"

#if	VMS
/***
 *  nothing  -  The nothing function.
 *
 *  This function is used as a placeholder for unimplemented functions,
 *  or to compile an empty module (which VMS doesn't like).
 *
 *  Nothing returned.
 ***/
nothing()
{
	/* Nothing */
}

#if VMSVT

/** Parameters **/
#define NKEYENT		128		/* Number of keymap entries	*/

/** VMS's include files **/
#include <descrip.h>			/* Descriptor definitions	*/
#include <smgtrmptr.h>			/* SMG stuff                    */
#include <ssdef.h>
#include <iodef.h>
#include <tt2def.h>

/** I/O information block definitions **/
struct iosb {				/* I/O status block		*/
	short i_cond;			/* Condition value		*/
	short i_xfer;			/* Transfer count		*/
	long i_info;			/* Device information		*/
};
struct termchar {			/* Terminal characteristics	*/
	char t_class;			/* Terminal class		*/
	char t_type;			/* Terminal type		*/
	short t_width;			/* Terminal width in characters	*/
	long t_mandl;			/* Terminal's mode and length	*/
	long t_extend;			/* Extended characteristics	*/
};
struct tahd {				/* Typeahead count		*/
	short t_count;			/* Number of character waiting	*/
	char t_first;			/* Next character available	*/
	char t_resv1;			/* Reserved by DEC		*/
	long t_resv2;			/* Reserved by DEC		*/
};

/** Type definitions **/
struct keyent {				/* Key mapping entry		*/
	struct keyent * samlvl;		/* Character on same level	*/
	struct keyent * nxtlvl;		/* Character on next level	*/
	char ch;			/* Character			*/
	int code;			/* Resulting keycode		*/
};

/** Command success **/
#define SUCCESS(x)	((x)&1)		/* TRUE if successful		*/
#define FAILURE(x)	(((x)&1)==0)	/* TRUE if unsuccessful		*/

/** Values to manage the screen **/
static int termtype;			/* Handle to pass to SMG	*/
static short channel;			/* Terminal I/O channel		*/
static struct termchar oldmode;		/* Old terminal modes		*/
static struct termchar newmode;		/* New terminal modes		*/
static char * begin_reverse;		/* Begin reverse video		*/
static char * end_reverse;		/* End reverse video		*/
static char * erase_to_end_line;	/* Erase to end of line		*/
static char * erase_whole_display;	/* Erase whole display		*/
static char * width_narrow;		/* Set narrow size screen	*/
static char * width_wide;		/* Set wide size screen		*/
static int narrow_char;			/* Number of characters narrow	*/
static int wide_char;			/* Number of characters wide	*/
static char inbuf[64];			/* Input buffer			*/
static char * inbufh = inbuf;		/* Head of input buffer		*/
static char * inbuft = inbuf;		/* Tail of input buffer		*/
static char outbuf[1024];		/* Output buffer		*/
static char * outbuft = outbuf;		/* Tail of output buffer	*/
static char keyseq[256];		/* Prefix escape sequence table	*/
static struct keyent keymap[NKEYENT];	/* Key map			*/
static struct keyent * nxtkey = keymap;	/* Next free key entry		*/

/** Forward references **/
int vmsopen(), vmsclose(), vmsgetc(), vmsputc(), vmsflush();
int vmsmove(), vmseeol(), vmseeop(), vmsbeep(), vmsrev(), vmscres();

/** Terminal dispatch table **/
TERM term = {
	72 - 1,				/* Max number of rows allowable */
	/* Filled in */ - 1,		/* Current number of rows used	*/
	160,				/* Max number of columns	*/
	/* Filled in */ 0,		/* Current number of columns	*/
	64,				/* Min margin for extended lines*/
	8,				/* Size of scroll region	*/
	100,				/* # times thru update to pause */
	vmsopen,			/* Open terminal at the start	*/
	vmsclose,			/* Close terminal at end	*/
	nothing,			/* Open keyboard		*/
	nothing,			/* Close keyboard		*/
	vmsgetc,			/* Get character from keyboard	*/
	vmsputc,			/* Put character to display	*/
	vmsflush,			/* Flush output buffers		*/
	vmsmove,			/* Move cursor, origin 0	*/
	vmseeol,			/* Erase to end of line		*/
	vmseeop,			/* Erase to end of page		*/
	vmsbeep,			/* Beep				*/
	vmsrev,				/* Set reverse video state	*/
	vmscres				/* Change screen resolution	*/
#if COLOR
	,
	nothing,			/* Set forground color		*/
	nothing				/* Set background color		*/
#endif /* COLOR */
};

/***
 *  vmsmove  -  Move the cursor (0 origin)
 *
 *  vmsmove calls to the SMG run-time library to produce a character
 *  sequence to position the cursor.  If the sequence cannot be made,
 *  a string "OOPS" is produced instead, much like the termcap library
 *  under UNIX.  In the case of "OOPS", the user will soon know that
 *  his terminal entry is incorrect.
 *
 *  Nothing returned.
 ***/
vmsmove(row, column)
int row;				/* Row position			*/
int column;				/* Column position		*/
{
	char buffer[32];
	int rlen, status;
	
	static int code = SMG$K_SET_CURSOR_ABS;
	static int len = sizeof(buffer);
	static int arg[3] = { 2 };

	/* SMG assumes the row/column positions	are 1 based. */
	arg[1] = row + 1;
	arg[2] = column + 1;

	/* Call to SMG for the sequence */
	status = SMG$GET_TERM_DATA(&termtype, &code, &len, &rlen, buffer, arg);
	if (SUCCESS(status)) {
		buffer[rlen] = '\0';
		vmsputs(buffer);
	} else
		vmsputs("OOPS");
}

/***
 *  vmscres  -  Change screen resolution
 *
 *  vmscres changes the screen resolution of the current window.
 *  Allowable sizes are NORMAL and WIDE.
 *
 *  Nothing returned
 ***/
vmscres(value)
char * value;				/* Value to set			*/
{
	int width;

	/* Skip if not supported */
	if (width_wide == NULL || width_narrow == NULL)
		return;

	/* Check value */
	if (strcmp(value, "WIDE") == 0) {
		width = wide_char;
		vmsputs(width_wide);
	} else if (strcmp(value, "NORMAL") == 0) {
		width = narrow_char;
		vmsputs(width_narrow);
	}

	/* Change width */
	oldmode.t_width = newmode.t_width = width;
	newwidth(TRUE, width);

	/* Set resolution variable */
	strcpy(sres, value);
}

/***
 *  vmsrev  -  Set the reverse video status
 *
 *  vmsrev either sets or resets the reverse video state, based on the
 *  boolean argument.  This function is only called if the revexist
 *  boolean variable is set to TRUE.  Otherwise there is no reverse
 *  video available.
 *
 *  Nothing returned.
 ***/
vmsrev(status)
int status;				/* TRUE if setting reverse	*/
{
	vmsputs(status ? begin_reverse : end_reverse);
}

/***
 *  vmseeol  -  Erase to end of line
 *
 *  When this function is called, the lines worth of text after the
 *  cursor is erased.  This function is only called if the eolexist
 *  boolean variable is set to TRUE.  Otherwise the display manager
 *  will produce enough spaces to erase the line.
 *
 *  Nothing returned.
 ***/
vmseeol()
{
	vmsputs(erase_to_end_line);
}

/***
 *  vmseeop  -  Erase to end of page (clear screen)
 *
 *  vmseeop really should be called vmsclear because it really should
 *  be an erase screen function.  When called, this routine will send
 *  the erase entire screen sequence to the output.
 *
 *  Nothing returned.
 ***/
vmseeop()
{
	vmsputs(erase_whole_display);
}

/***
 *  vmsbeep  -  Ring the bell
 *
 *  vmsbeep send a bell character to the output.  It might be possible
 *  in the future to include the NOISY definition and attempt to flash
 *  the screen, perhaps using LIGHT_SCREEN and DARK_SCREEN.
 *
 *  Nothing returned.
 ***/
vmsbeep()
{
	vmsputc('\007');
}

/***
 *  vmsgetstr  -  Get an SMG string capability by name
 *
 *  vmsgetstr attempts to obtain the escape sequence for a particular
 *  job from the SMG library.  Most sequences do not require a parameter
 *  with the sequence, others do.  In order to obtain the definition
 *  without knowing ahead of time whether ornot the definition has a
 *  parameter, we call SMG once with a parameter and if that fails, we
 *  try again without one.  If both attempts fail, we will return the
 *  NULL string.
 *
 *  Storage for the sequence comes from a local pool.
 *
 *  Returns:	Escape sequence
 *		NULL	No escape sequence available
 ***/ 
char * vmsgetstr(code)
int code;				/* Request code			*/
{
	char * result;
	int rlen, status;
	
	static char seq[1024];
	static char * buffer = seq;
	static int len = sizeof(seq);
	static int arg[2] = { 1, 1 };

	/* Get sequence with one parameter */
	status = SMG$GET_TERM_DATA(&termtype, &code, &len, &rlen, buffer, arg);
	if (FAILURE(status)) {
		/* Try again with zero parameters */
		status = SMG$GET_TERM_DATA(&termtype, &code, &len, &rlen, buffer);
		if (FAILURE(status))
			return NULL;
	}

	/* Check for empty result */
	if (rlen == 0)
		return NULL;
	
	/* Save current position so we can return it to caller */
	result = buffer;
	buffer[rlen++] = '\0';
	buffer += rlen;

	/* Return capability to user */
	return result;
}

/***
 *  vmsgetnum  -  Get numerical constant from SMG
 *
 *  vmsgetnum attempts to get a numerical constant from the SMG package.
 *  If the constant cannot be found, -1 is returned.
 ***/
int vmsgetnum(code)
int code;				/* SMG code			*/
{
	int status, result;

	/* Call SMG for code translation */
	status = SMG$GET_NUMERIC_DATA(&termtype, &code, &result);
	return FAILURE(status) ? -1 : result;
}

/***
 *  vmsaddkey  -  Add key to key map
 *
 *  vmsaddkey adds a new escape sequence to the sequence table.
 *  I am not going to try to explain this table to you in detail.
 *  However, in short, it creates a tree which can easily be transversed
 *  to see if input is in a sequence which can be translated to a
 *  function key (arrows and find/select/do etc. are treated like
 *  function keys).  If the sequence is ambiguous or duplicated,
 *  it is silently ignored.
 *
 *  Nothing returned
 ***/
vmsaddkey(code, fn)
int code;				/* SMG key code			*/
int fn;				/* Resulting keycode		*/
{
	char * seq;
	int first;
	struct keyent * cur, * nxtcur;

	/* Skip on NULL sequence */
	seq = vmsgetstr(code);
	if (seq == NULL)
		return;
	
	/* If no keys defined, go directly to insert mode */
	first = 1;
	if (nxtkey != keymap) {
		
		/* Start at top of key map */
		cur = keymap;
		
		/* Loop until matches exhast */
		while (*seq) {
			
			/* Do we match current character */
			if (*seq == cur->ch) {
				
				/* Advance to next level */
				seq++;
				cur = cur->nxtlvl;
				first = 0;
			} else {
				
				/* Try next character on same level */
				nxtcur = cur->samlvl;
				
				/* Stop if no more */
				if (nxtcur)
					cur = nxtcur;
				else
					break;
			}
		}
	}
	
	/* Check for room in keymap */
	if (strlen(seq) > NKEYENT - (nxtkey - keymap))
		return;
		
	/* If first character if sequence is inserted, add to prefix table */
	if (first)
		keyseq[(unsigned char) *seq] = 1;
		
	/* If characters are left over, insert them into list */
	for (first = 1; *seq; first = 0) {
		
		/* Make new entry */
		nxtkey->ch = *seq++;
		nxtkey->code = fn;
		
		/* If root, nothing to do */
		if (nxtkey != keymap) {
			
			/* Set first to samlvl, others to nxtlvl */
			if (first)
				cur->samlvl = nxtkey;
			else
				cur->nxtlvl = nxtkey;
		}

		/* Advance to next key */
		cur = nxtkey++;
	}
}

/***
 *  vmscap  -  Get capabilities from VMS's SMG library
 *
 *  vmscap retrives all the necessary capabilities from the SMG
 *  library to operate microEmacs.  If an insufficent number of
 *  capabilities are found for the particular terminal, an error
 *  status is returned.
 *
 *  Returns:	0 if okay, <>0 if error
 ***/
int vmscap()
{
	char * set_cursor_abs;
	int status;
	
	/* Start SMG package */
	status = SMG$INIT_TERM_TABLE_BY_TYPE(&oldmode.t_type, &termtype);
	if (FAILURE(status)) {
		printf(TEXT189);
/*                     "Cannot find entry for terminal type.\n" */
		printf(TEXT190);
/*                     "Check terminal type with \"SHOW TERMINAL\" or\n" */
		printf(TEXT191);
/*                     "try setting with \"SET TERMINAL/INQUIRE\"\n" */
		return 1;
	}
		
	/* Get reverse video */
	begin_reverse = vmsgetstr(SMG$K_BEGIN_REVERSE);
	end_reverse = vmsgetstr(SMG$K_END_REVERSE);
	revexist = begin_reverse != NULL && end_reverse != NULL;
	
	/* Get erase to end of line */
	erase_to_end_line = vmsgetstr(SMG$K_ERASE_TO_END_LINE);
	eolexist = erase_to_end_line != NULL;
	
	/* Get more neat stuff */
	erase_whole_display = vmsgetstr(SMG$K_ERASE_WHOLE_DISPLAY);
	width_wide = vmsgetstr(SMG$K_WIDTH_WIDE);
	width_narrow = vmsgetstr(SMG$K_WIDTH_NARROW);
	narrow_char = vmsgetnum(SMG$K_COLUMNS);
	wide_char = vmsgetnum(SMG$K_WIDE_SCREEN_COLUMNS);
	set_cursor_abs = vmsgetstr(SMG$K_SET_CURSOR_ABS);

	/* Disable resoultion if unreasonable */
	if (narrow_char < 10 || wide_char < 10) {
		width_wide = width_narrow = NULL;
		strcpy(sres, "NORMAL");
	} else
		/* Kludge resolution */
		strcpy(sres, oldmode.t_width == wide_char ? "WIDE" : "NORMAL");

	/* Check for minimal operations */
	if (set_cursor_abs == NULL || erase_whole_display == NULL) {
		printf(TEXT192);
/*                     "The terminal type does not have enough power to run\n" */
		printf(TEXT193);
/*                     "MicroEMACS.  Try a different terminal or check\n" */
		printf(TEXT194);
/*                     "type with \"SHOW TERMINAL\".\n" */
		return 1;
	}
	
	/* Add function keys to keymapping table */
	vmsaddkey(SMG$K_KEY_DOWN_ARROW, CTRL | 'N');
	vmsaddkey(SMG$K_KEY_LEFT_ARROW, CTRL | 'B');
	vmsaddkey(SMG$K_KEY_RIGHT_ARROW, CTRL | 'F');
	vmsaddkey(SMG$K_KEY_UP_ARROW, CTRL | 'P');
	vmsaddkey(SMG$K_KEY_PF1, META | '<');
	vmsaddkey(SMG$K_KEY_PF2, META | '>');
	vmsaddkey(SMG$K_KEY_PF3, CTRL | 'S');
	vmsaddkey(SMG$K_KEY_PF4, CTRL | 'Z');
	vmsaddkey(SMG$K_KEY_0, ALTD | '0');
	vmsaddkey(SMG$K_KEY_1, ALTD | '1');
	vmsaddkey(SMG$K_KEY_2, ALTD | '2');
	vmsaddkey(SMG$K_KEY_3, ALTD | '3');
	vmsaddkey(SMG$K_KEY_4, ALTD | '4');
	vmsaddkey(SMG$K_KEY_5, ALTD | '5');
	vmsaddkey(SMG$K_KEY_6, ALTD | '6');
	vmsaddkey(SMG$K_KEY_7, ALTD | '7');
	vmsaddkey(SMG$K_KEY_8, ALTD | '8');
	vmsaddkey(SMG$K_KEY_9, ALTD | '9');
	vmsaddkey(SMG$K_KEY_PERIOD, SHFT|'.');
	vmsaddkey(SMG$K_KEY_ENTER, SHFT|CTRL|'M');
	vmsaddkey(SMG$K_KEY_COMMA, SHFT|',');
	vmsaddkey(SMG$K_KEY_MINUS, SHFT|'-');
	vmsaddkey(SMG$K_KEY_F1, '1');
	vmsaddkey(SMG$K_KEY_F2, '2');
	vmsaddkey(SMG$K_KEY_F3, '3');
	vmsaddkey(SMG$K_KEY_F4, '4');
	vmsaddkey(SMG$K_KEY_F5, '5');
	vmsaddkey(SMG$K_KEY_F6, '6');
	vmsaddkey(SMG$K_KEY_F7, '7');
	vmsaddkey(SMG$K_KEY_F8, '8');
	vmsaddkey(SMG$K_KEY_F9, '9');
	vmsaddkey(SMG$K_KEY_F10, '0');
	vmsaddkey(SMG$K_KEY_F11, SHFT | '1');
	vmsaddkey(SMG$K_KEY_F12, SHFT | '2');
	vmsaddkey(SMG$K_KEY_F13, SHFT | '3');
	vmsaddkey(SMG$K_KEY_F14, SHFT | '4');
	vmsaddkey(SMG$K_KEY_F15, SHFT | '5');
	vmsaddkey(SMG$K_KEY_F16, SHFT | '6');
	vmsaddkey(SMG$K_KEY_F17, SHFT | '7');
	vmsaddkey(SMG$K_KEY_F18, SHFT | '8');
	vmsaddkey(SMG$K_KEY_F19, SHFT | '9');
	vmsaddkey(SMG$K_KEY_F20, SHFT | '0');
	vmsaddkey(SMG$K_KEY_E1, META | ' ');
	vmsaddkey(SMG$K_KEY_E2, META | 'I');
	vmsaddkey(SMG$K_KEY_E3, META | CTRL | 'I');
	vmsaddkey(SMG$K_KEY_E4, META | 'J');
	vmsaddkey(SMG$K_KEY_E5, META | 'O');
	vmsaddkey(SMG$K_KEY_E6, META | CTRL | 'O'); 

	/* Everything okay */
	return 0;
}

/***
 *  vmsgtty - Get terminal type from system control block
 *
 *  vmsgtty obtains the terminal's information such as flags modes,
 *  baud rate, size etc. and stores it in the structure block.
 *  If the block cannot be obtainedm, an error condition is returned.
 *
 *  Returns:	Status
 ***/
int vmsgtty(tc)
struct termchar * tc;			/* Terminal characteristics	*/
{
	int status;
	struct iosb io;
	
	/* Get terminal characteristics */
	status = SYS$QIOW(1, channel, IO$_SENSEMODE, &io, 0, 0,
		tc, sizeof(*tc), 0, 0, 0, 0);
	
	/* Check status */
	return FAILURE(status) && FAILURE(io.i_cond);
}

/***
 *  vmsstty - Set terminal control block
 *
 *  vmsstty takes a previous vmsgtty with modifications, and stores
 *  this as the current terminal control block.
 *
 *  Returns:	Status
 ***/
int vmsstty(tc)
struct termchar * tc;			/* Terminal characteristics	*/
{
	int status;
	struct iosb io;
	
	/* Set terminal characteristics */
	status = SYS$QIOW(1, channel, IO$_SETMODE, &io, 0, 0,
		tc, sizeof(*tc), 0, 0, 0, 0);
	
	/* Check status */
	return FAILURE(status) && FAILURE(io.i_cond);
}

/***
 *  vmsclose  -  Close the connection to the terminal
 *
 *  vmsclose resets the terminal to the original state and cuts the
 *  connection.  No further operations should be done after closing.
 *
 *  Nothing returned.
 ***/
vmsclose()
{
	/* Flush pending output */
	vmsflush();

	/* Do this stupid thing for synchronization */
	SYS$QIOW(1, channel, IO$_WRITELBLK | IO$M_NOFORMAT,
		0, 0, 0, " \b", 2, 0, 0, 0, 0);

	/* Reset terminal to original modes */
	vmsstty(&oldmode);
	
	/* Release channel */
	SYS$DASSGN(channel);
}

/***
 *  vmsopen  -  Get terminal type and open terminal
 *
 *  Nothing returned
 ***/
vmsopen()
{
	$DESCRIPTOR(name, "TT");
	int status;
	
	/* Open channel to terminal */
	status = SYS$ASSIGN(&name, &channel, 0, 0);
	if (FAILURE(status)) {
		printf(TEXT195);
/*                     "Cannot open channel to terminal.\n" */
		exit(1);
	}
	
	/* Get terminal type */
	if (vmsgtty(&oldmode)) {
		printf(TEXT196);
/*                     "Cannot obtain terminal settings.\n" */
		goto error;
	}
	
	/* Get SMG */
	if (vmscap())
		goto error;
	
	/* Set sizes */
	term.t_nrow = ((unsigned int) oldmode.t_mandl >> 24) - 1;
	term.t_ncol = oldmode.t_width;

	/* Set new terminal modes */
	newmode = oldmode;
	newmode.t_extend |= TT2$M_PASTHRU;
	if (vmsstty(&newmode)) {
		printf(TEXT197);
/*                     "Cannot modify terminal settings.\n" */
		goto error;
	}

	/* Finished! */
	return;

	/* Make sure terminal deassigned */	
error:
	/* Release channel */
	SYS$DASSGN(channel);
	exit(1);
}

/***
 *  vmsflush  -  Flush output buffer
 *
 *  vmsflush causes all queued output characters to be written to the
 *  terminal's screen.  We will use SYS$QIO because we don't need to
 *  wait and because it dramaticly increases performance.
 *
 *  Nothing returned.
 ***/
vmsflush()
{
	int len;
	
	/* Compute size of request */
	len = outbuft - outbuf;
	
	/* Skip if zero */
	if (len) {
		SYS$QIO(0, channel, IO$_WRITELBLK | IO$M_NOFORMAT,
			0, 0, 0, outbuf, len, 0, 0, 0, 0);
			
		/* Reset buffer positions */
		outbuft = outbuf;
	}
}

/***
 *  vmsputc  -  Send a character to the screen
 *
 *  vmsputc queues character into a buffer for screen output.  When the
 *  buffer is full the flush routine is called.  This is help speed
 *  things up by avoiding millions of system calls.
 *
 *  Nothing returned.
 ***/
vmsputc(ch)
char ch;				/* Character to add		*/
{
	/* Check for overflow */
	if (outbuft == &outbuf[sizeof(outbuf)])
		vmsflush();
		
	/* Add character to buffer */
	*outbuft++ = ch;
}

/***
 *  vmsputs  -  Send a string to vmsputc
 *
 *  vmsputs is a short-cut routine to handle sending a string of characters
 *  to the character output routine.  A check is made for a NULL string,
 *  while is considered valid.  A NULL string will produce no output.
 *
 *  Nothing returned.
 ***/
vmsputs(string)
char * string;				/* String to write		*/
{
	if (string)
		while (*string)
			vmsputc(*string++);
}

/***
 *  vmsgchar  -  Get character directly from VMS
 *
 *  vmsgchar is the lowest level of retrieving character from VMS.
 *  The argument timed specifies where to wait for a character for
 *  a short period (1 > wait >= 2 seconds) or indefinately.  The short
 *  period version is used when obtaining a escape sequence.
 *
 *  Returns:  character or 0 if no character is available.
 ***/
char vmsgchar(timed)
int timed;				/* TRUE if being timed		*/
{
	char ch;
	int status, op;
	struct iosb io;
	
	/* Make operation */
	op = (timed ? IO$M_TIMED : 0) | IO$_READLBLK | IO$M_NOECHO;
	
	/* Get next character */
	status = SYS$QIOW(1, channel, op, &io, 0, 0, &ch, 1, 2, 0, 0, 0);
		
	/* Fatal error */
	if (FAILURE(status) || FAILURE(io.i_cond)) {

		/* Check for time-out */
		if (io.i_cond == SS$_TIMEOUT)
			return 0;

		/* Real I/O error occured */
		printf(TEXT198, status, io.i_cond);
/*                     "I/O error (%d,%d)\n" */
		SYS$DASSGN(channel);
		exit(1);
	}
	
	/* Return next character */
	return ch;
}

/***
 *  vmsqin  -  Queue character for input
 *
 *  vmsqin queues the character into the input buffer for later
 *  reading.  This routine will mostly be used by mouse support
 *  and other escape sequence processing.
 *
 *  Nothing returned.
 ***/
vmsqin(ch)
char ch;				/* Character to add		*/
{
	/* Check for overflow */
	if (inbuft == &inbuf[sizeof(inbuf)]) {
		
		/* Annoy user */
		vmsbeep();
		return;
	}
	
	/* Add character */
	*inbuft++ = ch;
}

/***
 *  vmsgcook  -  Get characters from input device
 *
 *  vmsgcook "cooks" input from the input device and places them into
 *  the input queue.
 *
 *  Nothing returned.
 ***/
vmsgcook()
{
	char ch;
	struct keyent * cur;
	
	/* Get first character untimed */
	ch = vmsgchar(0);
	vmsqin(ch);
	
	/* Skip if the key isn't a special leading escape sequence */
	if (keyseq[(unsigned char) ch] == 0)
		return;
		
	/* Start translating */
	cur = keymap;
	while (cur) {
		if (cur->ch == ch) {
			/* Is this the end */
			if (cur->nxtlvl == NULL) {
				/* Replace all character with new sequence */
				inbuft = inbuf;
				vmsqin(0);
                                vmsqin((cur->code)>>8);
				vmsqin(cur->code);
				return;
			} else {
				/* Advance to next level */
				cur = cur->nxtlvl;
			
				/* Get next character, timed */
				ch = vmsgchar(1);
				if (ch == '\0')
					return;

				/* Queue character */
				vmsqin(ch);
			}
		} else
			/* Try next character on same level */
			cur = cur->samlvl;
	}
}

/***
 *  vmsgetc  -  Get a character
 *
 *  vmsgetc obtains input from the character input queue.  If the queue
 *  is empty, a call to vmsgcook() is called to fill the input queue.
 *
 *  Returns:	character
 ***/
int vmsgetc()
{
	char ch;

	/* Loop until character found */
	while (1) {
	
		/* Get input from buffer, if available */
		if (inbufh != inbuft) {
			ch = *inbufh++;
			if (inbufh == inbuft)
				inbufh = inbuft = inbuf;
			break;
		} else
	
			/* Fill input buffer */
			vmsgcook();
	}
	
	/* Return next character */
	return (int) ch;
}

#if FLABEL
/***
 *  fnclabel  -  Label function keys
 *
 *  Currently, VMS does not have function key labeling.
 *
 *  Returns:	status.
 ***/
int fnclabel(flag, n)
int flag;				/* TRUE if default		*/
int num;				/* Numerical argument		*/
{
	/* On machines with no function keys...don't bother */
	return TRUE;
}
#endif /* FLABEL */

/***
 *  spal  -  Set palette type
 *
 *  spal sets the palette colors for the 8 colors available.  Currently,
 *  there is nothing here, but some DEC terminals, (VT240 and VT340) have
 *  a color palette which is available under the graphics modes.
 *  Further, a foreign terminal could also change color registers.
 *
 *  Nothing returned
 ***/
spal()
{
	/* Nothing */
}

#endif /* VMSVT */

#if TYPEAH
/***
 *  typahead  -  Check for pending input
 *
 *  typahead check the input buffer for pending input.  If input exists
 *  TRUE is returned.  This routine is used mostly by the display
 *  update routine to avoid redrawing the entire display when it
 *  doesn't need to do so.
 *
 *  Returns:  boolean.
 ***/
int typahead()
{
	int status;
	struct iosb io;
	struct tahd tbuf;

	/* Check for buffered characters */
	if (inbufh != inbuft)
		return TRUE;

	/* Call to system for character count */
	status = SYS$QIOW(1, channel, IO$_SENSEMODE | IO$M_TYPEAHDCNT,
		&io, 0, 0, &tbuf, sizeof(tbuf), 0, 0, 0, 0);
	if (FAILURE(status) || FAILURE(io.i_cond))
		return FALSE;
	return tbuf.t_count;
}
#endif /* TYPEAH */

/***
 *  vmsdcl  -  Execute or invoke a DCL
 *
 *  vmsdcl without an argument will invoke a new DCL as a new process
 *  and attach to it.  When the DCL returns, emacs will regain control.
 *  With an argument, the DCL starts and begins executing the command
 *  line.
 *
 *  Returns:  status.
 ***/
int vmsdcl(command)
char * command;				/* Command to execute		*/
{
	int result, status;
	struct dsc$descriptor desc, * dp;
	
	/* Set up descriptor */
	if (command) {
		desc.dsc$a_pointer = command;
		desc.dsc$w_length = strlen(command);
		desc.dsc$b_dtype = DSC$K_DTYPE_T;
		desc.dsc$b_class = DSC$K_CLASS_S;
		dp = &desc;
	} else 
		dp = NULL;
	
	/* Turn off raw input */
	vmsstty(&oldmode);

	/* Call DCL */
	status = LIB$SPAWN(dp, 0, 0, 0, 0, 0, &result, 0, 0, 0);

	/* Return to raw input */
	vmsstty(&newmode);
	
	/* Return status */
	return SUCCESS(status) && SUCCESS(result);
}		

/***
 *  spawncli  -  Spawn a new DCL
 *
 *  spawncli reliquishes control of the terminal and passes it on to
 *  a newly created DCL process.  When the DCL process finishes, the
 *  screen is redrawen and emacs regains control.
 *
 *  Returns:  status
 ***/
int spawncli(flag, num)
int flag;				/* TRUE if default		*/
int num;				/* Numerical argument		*/
{
	/* Restrict usage */
	if (restflag)
		return resterr();
		
	/* Move to last line and announce */
	vmsmove(term.t_nrow, 0);
	vmsputs(TEXT199);
/*              "[Starting DCL]\r\n" */
	vmsflush();

	/* Redraw screen on return */	
	sgarbf = TRUE;
	
	/* Start command */
	return vmsdcl(NULL);
}

/***
 *  spawn  -  Spawn a command
 *
 *  spawncli reliquishes control of the terminal and passes it on to
 *  a newly created DCL process.  When the DCL process finishes, the
 *  screen is redrawen and emacs regains control.
 *
 *  Returns:  status
 ***/
int spawn(flag, num)
int flag;				/* TRUE if default		*/
int num;				/* Numerical argument		*/
{
	char line[NLINE];
	int status;
	
	/* Restrict usage */
	if (restflag)
		return resterr();
		
	/* Ask for command */
	status = mlreply("!", line, sizeof(line));
	if (status != TRUE)
		return status;
		
	/* Move to last line and announce */
	vmsmove(term.t_nrow, 0);
	vmsputs(TEXT200);
/*              "[Calling DCL]\r\n" */
	vmsflush();
	
	/* Run command */
	status = vmsdcl(line);
	
	/* Wait for conformation */
	vmsputs(TEXT6);
/*              "\r\n\n[END]" */
	vmsflush();
	vmsgchar();

	/* Redraw screen on return */	
	sgarbf = TRUE;
	return status;
}

/***
 *  execprg  -  Execute a program without using command interpretter
 *
 *  execprg is the same as spawn because there is no reasonable way
 *  of avoiding the command interpretter.  This routine is mostly for
 *  smaller systems.
 *
 *  Returns:  status
 ***/
int execprg(flag, num)
int flag;				/* TRUE if default		*/
int num;				/* Numerical argument		*/
{
	return spawn(flag, num);
}

/***
 *  pipecmd   -  Pipe a one line command into a window
 *
 *  pipecmd take a single command and places output into a buffer.
 *  This is accomplished on non-UNIX machines by sending the result
 *  of the command interpretter into a temporary file, then reading
 *  the file into the buffer.
 *
 *  This command is not implemented current, but could be implemented
 *  and will be once I get some manuals.
 *
 *  Returns:  status
 ***/
int pipecmd(flag, num)
int flag;				/* TRUE if default		*/
int num;				/* Numerical argument		*/
{
	mlwrite(TEXT201);
/*              "[Not available yet under VMS]" */
	return FALSE;
}

/***
 *  filter  -  Filter buffer through an external command
 *
 *  filter take a buffer and pipes into a program.  This is accomplished
 *  under non-UNIX systems by writing the buffer to a temporary  file
 *  and running the command with the temporary file as input and another
 *  temporary file as output.  Once the command completes, the buffer
 *  is replaced by the contents of the second temporary file and
 *  all temporary files are removed.
 *
 *  This command is currently not implemented, but can be implemented
 *  and will be once I get the manuals.
 *
 *  Returns:  status.
 ***/
int filter(flag, num)
int flag;				/* TRUE if default		*/
int num;				/* Numerical argument		*/
{
	mlwrite(TEXT201);
/*              "[Not available yet under VMS]" */
	return FALSE;
}

/* return a system dependant string with the current time */

char *PASCAL NEAR timeset()

{
	register char *sp;	/* temp string pointer */
	char buf[16];		/* time data buffer */
	extern char *ctime();

	time(buf);
	sp = ctime(buf);
	sp[strlen(sp)-1] = 0;
	return(sp);
}


/* couple of dummy routine for VMS. MJB: 09-Nov-89 */
char *PASCAL NEAR getffile(fspec)
char * fspec;
{
	return((char *)NULL);
}

char *PASCAL NEAR getnfile()
{
	return((char *)NULL);
}
#else
vmshello()
{
}
#endif
