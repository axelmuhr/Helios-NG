/* $Header: /dsl/HeliosRoot/Helios/cmds/public/stevie/RCS/help.c,v 1.1 1993/08/06 15:17:14 nickc Exp tony $
 *
 * Routine to display a command summary.
 * (Dave Tutelman note:
 *	I added the ability to page backwards and forwards through help.
 *	In order to minimize the abuse to the existing code, I used
 *	"goto"s and labeled each screen.  It's not the way I would have
 *	done help from scratch, but it's not TOO ugly.
 * )
 */

#include <ctype.h>
#include "stevie.h"
#include "ascii.h"
#include "keymap.h"

/* Macro to show help screen 'n'.
 * If C supported label types, it'd be cleaner to do it that way. */
#define	SHOWHELP( n )	switch(n) {		\
			case 0: goto Screen0;   \
			case 1: goto Screen1;	\
			case 2: goto Screen2;	\
			case 3: goto Screen3;	\
			case 4: goto Screen4;	\
			case 5: goto Screen5;	\
			case 6: goto Screen6;	\
			case 7: goto Screen7;	\
			case 8: goto Screen8;   \
			default: return (TRUE);	}

extern	char	*Version;

static	int	helprow;
static	int	lastscreen = 0;		/* return to help in previous screen */

#ifdef	HELP

static	void	longline();

bool_t
help()
{
	int k;

	SHOWHELP( lastscreen );		/* where did we quit help last ? */

/***********************************************************************
 * Zeroth Screen : Index to the help screens.
 ***********************************************************************/

Screen0:
	CLS;
	windgoto(helprow = 0, 0);

longline("\
   Index to HELP Screens\n\
   =====================\n\n");
longline("\
      0    Help index  (this screen)\n\
      1    Positioning within file, adjusting the screen\n\
      2    Character positioning\n\
      3    Line positioning, marking & returning, undo & redo\n");
longline("\
      4    Insert & replace, words, sentences, paragraphs\n\
      5    Operators, miscellaneous operations, yank & put\n\
      6    \"Ex\" command line operations\n\
      7    Set parameters\n\
      8    System-specific features\n");

	windgoto(0, 52);
	longline(Version);

	SHOWHELP( helpkey (0) );


/***********************************************************************
 * First Screen:   Positioning within file, Adjusting the Screen
 ***********************************************************************/

Screen1:
	CLS;
	windgoto(helprow = 0, 0);

longline("\
   Positioning within file\n\
   =======================\n\
      ^F             Forward screenfull\n\
      ^B             Backward screenfull\n");
longline("\
      ^D             scroll down half screen\n\
      ^U             scroll up half screen\n");
longline("\
      G              Goto line (end default)\n\
      ]]             next function\n\
      [[             previous function\n\
      /re            next occurence of regular expression 're'\n");
longline("\
      ?re            prior occurence of regular expression 're'\n\
      n              repeat last / or ?\n\
      N              reverse last / or ?\n\
      %              find matching (, ), {, }, [, or ]\n");
longline("\
\n\
   Adjusting the screen\n\
   ====================\n\
      ^L             Redraw the screen\n\
      ^E             scroll window down 1 line\n\
      ^Y             scroll window up 1 line\n");
longline("\
      z<RETURN>      redraw, current line at top\n\
      z-             ... at bottom\n\
      z.             ... at center\n");

	SHOWHELP( helpkey (1) );


/***********************************************************************
 * Second Screen:   Character positioning
 ***********************************************************************/

Screen2:
	CLS;
	windgoto(helprow = 0, 0);

longline("\
   Character Positioning\n\
   =====================\n\
      ^              first non-white\n\
      0              beginning of line\n\
      $              end of line\n\
      h              backward\n");
longline("\
      l              forward\n\
      ^H             same as h\n\
      space          same as l\n\
      fx             find 'x' forward\n");
longline("\
      Fx             find 'x' backward\n\
      tx             upto 'x' forward\n\
      Tx             upto 'x' backward\n\
      ;              Repeat last f, F, t, or T\n");
longline("\
      ,              inverse of ;\n\
      |              to specified column\n\
      %              find matching (, ), {, }, [, or ]\n");

	SHOWHELP( helpkey (2) );


/***********************************************************************
 * Third Screen:   Line Positioning, Marking and Returning
 ***********************************************************************/

Screen3:
	CLS;
	windgoto(helprow = 0, 0);

longline("\
    Line Positioning\n\
    ================\n\
    H           home window line\n\
    L           last window line\n\
    M           middle window line\n");
longline("\
    +           next line, at first non-white\n\
    -           previous line, at first non-white\n\
    CR          return, same as +\n\
    j           next line, same column\n\
    k           previous line, same column\n");

longline("\
\n\
    Marking and Returning\n\
    =====================\n\
    ``          previous context\n\
    ''          ... at first non-white in line\n");
longline("\
    mx          mark position with letter 'x'\n\
    `x          to mark 'x'\n\
    'x          ... at first non-white in line\n");

longline("\n\
    Undo  &  Redo\n\
    =============\n\
    u           undo last change\n\
    U           restore current line\n\
    .           repeat last change\n");

	SHOWHELP( helpkey (3) );


/***********************************************************************
 * Fourth Screen:   Insert & Replace,
 ***********************************************************************/

Screen4:
	CLS;
	windgoto(helprow = 0, 0);

longline("\
    Insert and Replace\n\
    ==================\n\
    a           append after cursor\n\
    i           insert before cursor\n\
    A           append at end of line\n\
    I           insert before first non-blank\n");
longline("\
    o           open line below\n\
    O           open line above\n\
    rx          replace single char with 'x'\n\
    R           replace characters\n");
if (! P(P_TO))
longline("\
    ~           change case (upper/lower) of single char\n");

longline("\
\n\
    Words, sentences, paragraphs\n\
    ============================\n\
    w           word forward\n\
    b           back word\n\
    e           end of word\n\
    )           to next sentence\n\
    }           to next paragraph\n");
longline("\
    (           back sentence\n\
    {           back paragraph\n\
    W           blank delimited word\n\
    B           back W\n\
    E           to end of W\n");

	SHOWHELP( helpkey (4) );


/***********************************************************************
 * Fifth Screen:   Operators, Misc. operations, Yank & Put
 ***********************************************************************/

Screen5:
	CLS;
	windgoto(helprow = 0, 0);

longline("\
    Operators (double to affect lines)\n\
    ==================================\n\
    d           delete\n\
    c           change\n");
longline("\
    <           left shift\n\
    >           right shift\n\
    y           yank to buffer\n\
    !           filter lines (command name follows)\n");
if (P(P_TO))
longline("\
    ~           reverse case (upper/lower)\n");

longline("\n\
    Miscellaneous operations\n\
    ========================\n\
    C           change rest of line\n\
    D           delete rest of line\n\
    s           substitute chars\n");
longline("\
    S           substitute lines (not yet)\n\
    J           join lines\n\
    x           delete characters\n\
    X           ... before cursor\n");

longline("\n\
    Yank and Put\n\
    ============\n\
    p           put back text\n\
    P           put before\n\
    Y           yank lines");

	SHOWHELP( helpkey (5) );


/***********************************************************************
 * Sixth Screen:   Command-line operations
 ***********************************************************************/

Screen6:
	CLS;
	windgoto(helprow = 0, 0);

longline("\
    EX command-line operations\n\
    ==========================\n");
longline("\
    :w          write back changes\n\
    :wq         write and quit\n\
    :x          write if modified, and quit\n\
    :q          quit\n\
    :q!         quit, discard changes\n\
    :e name     edit file 'name'\n\
    :e!         reedit, discard changes\n");
if (P(P_TG))
longline("\
    :e #        edit alternate file\n");
else
longline("\
    :e #        edit alternate file (also ctrl-^)\n");
longline("\
    :w name     write file 'name'\n\
    :n          edit next file in arglist\n\
    :N          edit prior file in arglist\n\
    :rew        rewind arglist\n\
    :f          show current file and lines\n");
longline("\
    :f file     change current file name\n\
    :g/pat/p|d  global command (print or delete only)\n\
    :s/p1/p2/   text substitution (trailing 'g' optional)\n\
    :ta tag     to tag file entry 'tag'\n\
    ^]          :ta, current word is tag\n");
if (P(P_TG))
longline("\
    :untag      back to before last ':ta' (also ctrl-^)\n");
longline("\
    :sh         run an interactive shell\n\
    :!cmd       execute a shell command\n\
");

	SHOWHELP( helpkey (6) );


/***********************************************************************
 * Seventh Screen:   Set parameters
 ***********************************************************************/

Screen7:
	CLS;
	windgoto(helprow = 0, 0);

longline("\
    Set Parameters\n\
    ==============\n");
longline("\
    :set param-name[=param-value]   to set\n\
    :set sm, :set nosm, :set co=23  examples\n\
    :set all    display all values\n\
    :set        display non-default values\n\n");
longline("Abbrev, name, and current value:\n");
hparm(P_AI); hparm(P_SM); longline("\n");
hparm(P_BK); hparm(P_CO); longline("\n");
hparm(P_TS); hparm(P_MO); longline("\n");
hparm(P_IC); hparm(P_ML); longline("\n");
hparm(P_TG); hparm(P_TO); longline("\n");
hparm(P_EB); hparm(P_VB); longline("\n");
hparm(P_LI); hparm(P_NU); longline("\n");
hparm(P_SS); longline(" (# of lines for ^D, ^U)\n");
hparm(P_LS); longline(" (show tabs, newlines graphically)\n");
hparm(P_RP); longline(" (min # of lines to report on oper)\n");
hparm(P_WS); longline(" (search wraps around end of file)\n");
hparm(P_CR); longline(" (write newline to file as CR-LF)\n");

	SHOWHELP( helpkey (7) );


/***********************************************************************
 * Eighth Screen:   System-Specific Features for DOS and OS/2
 ***********************************************************************/

Screen8:
	CLS;
	windgoto(helprow = 0, 0);

longline("\
    MSDOS & OS/2 Special Keys\n\
    =========================\n");
longline("\
    The cursor keypad does pretty much what you'd expect,\n");
longline("\
    as long as you're not in text entry mode:\n\
\n\
    Home, End, PgUp, PgDn, and the arrow keys navigate.\n\
    Insert    enter text before cursor.\n\
    Delete    delete character at the cursor.\n\n");

longline("\
    Function Keys\n\
    =============\n\
    F1      Help\n\
    F2      Next file (:n)             Shift-F2  discard changes (:n!)\n\
    F3      Previous file (:N)         Shift-F3  discard changes (:N!)\n");
longline("\
    F4      Alternate file (:e #)      Shift-F4  discard changes (:e! #)\n\
    F5      Rewind file list (:rew)    Shift-F5  discard changes (:rew!)\n\
    F6      Next function (]])         Shift-F6  Prev. function ([[)\n\
    F8      Global subst. (:1,$s/)\n\
    F10     Save & quit (:x)           Shift-F10 discard changes (:q!)");

	SHOWHELP( helpkey (8) );

}


/*	longline (p)
 *	Put the string p into the buffer, expanding newlines.
 */
static void
longline(p)
char	*p;
{
	register char	*s;

	for ( s = p; *s ;s++ ) {
		if ( *s == '\n' )
			windgoto(++helprow, 0);
		else
			outchar(*s);
	}
}

/*	hparm (n)
 *	Put the help info for param #n into the buffer.
 */
hparm (p)
  int p;
{
	char	buf[25];
	char	*bp;

	sprintf(buf, "     %6s  %-10s  ",
			params[p].shortname, params[p].fullname);
	bp = buf + strlen (buf);
	if (params[p].flags & P_NUM)	/* numeric param */
		sprintf(bp, "%-3d", params[p].value);
	else {				/* Boolean param */
		if (params[p].value)
			strcpy (bp, "yes");
		else
			strcpy (bp, "no ");
	}
	longline (buf);
}

/* Get keystroke and return instructions on what to do next.
 * Argument is current help screen.
 * Return value is target help screen, or -1 to quit help.
 */

#ifdef DOS
#  define	NSCREEN	8
#else
#ifdef OS2
#  define	NSCREEN 8
#else
#  define	NSCREEN 7
#endif
#endif

int
helpkey (n)
  int n;
{
	static int	c = '\0';
	int	prevkey;
	char	banner [16];

	/* Start with instructions on navigating Help */
	strcpy (banner, "PAGE 0 OF HELP");
	banner [5] = (char)n + '0';
	windgoto(helprow = Rows-4, 63);
	longline(banner);
	windgoto(helprow = Rows-3, 63);
	longline("^^^^^^^^^^^^^^");
	windgoto(helprow = Rows-2, 54);
	longline("<Press Esc to quit Help>");
	windgoto(helprow = Rows-1, 44);
	longline("<Other keys navigate Help screens>\n");

	/* Now get keystrokes till we get a valid one */
	while (1) {
		prevkey = c;
		c = vgetc();
		switch (c) {
		  /* cases for Next Screen */
		  case ' ':	  case '\t':
		  case '\n':	  case '\r':	case '+':
		  case K_DARROW:
		  case 'f':	  case CTRL('F'):
		  case CTRL('D'): case CTRL('Y'):
		  case 'n':	  case 'N':	case CTRL('N'):
		  case 'j':
			if (n < NSCREEN)	return (n+1);
			break;

		  /* cases for Previous Screen */
		  case BS:	  case '-':
		  case K_UARROW:
		  case 'b':	  case CTRL('B'):
		  case CTRL('U'): case CTRL('E'):
		  case 'p':	  case 'P':	case CTRL('P'):
		  case 'k':
			if (n > 0) return (n-1);
			break;

		  /* cases for Quit Help */
		  case ESC:
		  case 'Q':
		  case 'q':
		  case 'X':
		  case 'x':
			lastscreen = n;		/* remember where we quit */
			return (-1);

		  /* "G" is presumed to be a "vi-style" go-to-line,
		   * except that we interpret it as go-to-screen.
		   */
		  case 'G':
			/* If previous key was a number,
			 * we're already there.  Otherwise, go to
			 * last screen.
			 */
			if (prevkey<(int)'0' || prevkey>NSCREEN+(int)'0')
				return (NSCREEN);
			break;

		  /* Default is screen number or invalid code */
		  default:
			if (c>=(int)'0' && c<=NSCREEN+(int)'0')
				return ( c - (int)'0' );
			break;
		}
	}
}


#else

bool_t
help()
{
	msg("Sorry, help not configured");
	return FALSE;
}
#endif
