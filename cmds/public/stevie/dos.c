/* $Header:
 *
 * MSDOS support for Stevie.
 * Many of the functions in this file have two versions:
 *   -	The original version, using ANSI escape sequences
 *	(by Tim Thompson and/or Tony Andrews).
 *	It requires a non-IBM ANSI driver, such as the shareware NANSI.SYS.
 *   -	The BIOS-function version (by Larry A. Shurr).  The BIOS version
 *	doesn't require an enhanced console driver such as NANSI.SYS.
 *	Invoke it by #defining BIOS in ENV.H.
 * Dave Tutelman has incorporated many features of Larry Shurr's BIOS
 * version (such as colors and 43-line mode) into the ANSI version.
 */

#include "stevie.h"
#include <stdio.h>
#include <dos.h>
#include <signal.h>

char	*getenv();

static	char	getswitch();
static	void	setswitch();

#ifdef BIOS
void bios_t_ed();
void bios_t_el();
#endif

enum hostval_e {hIBMPC, hTIPRO};
typedef enum hostval_e hostval;
static	hostval	host_type   = 0;	/* Gets host computer type */

static	char	bgn_color   = 0x7;	/* For saving orig color */
static	char	quitting_now= 0;	/* Set for windexit() */
static	int	crt_int     = 0;	/* Gets CRT BIOS interrupt */
static	char	bgn_page    = 0;	/* For saving current page (IBM PC) */
static	char	bgn_mode    = 0;	/* For saving video mode (IBM PC) */

#ifdef BIOS
static	int	sav_curattr = 0;	/* For saving cursor attributes */
static	int	sav_curpos  = 0;	/* For saving cursor position */
#endif

/*
 * inchar() - get a character from the keyboard
 */
int
inchar()
{
	int	c;

	got_int = FALSE;

	for (;;beep()) {	/* loop until we get a valid character */

		flushbuf();	/* flush any pending output */

		switch (c = getch()) {
		case 0x1e:
			return K_CCIRCM;
		case 0:				/* special key */
			if (State != NORMAL) {
				c = getch();	/* throw away next char */
				continue;	/* and loop for another char */
			}
			switch (c = getch()) {
			case 0x50:
				return K_DARROW;
			case 0x48:
				return K_UARROW;
			case 0x4b:
				return K_LARROW;
			case 0x4d:
				return K_RARROW;
			case 0x47:		/* Home key */
				stuffin("1G");
				return -1;
			case 0x4f:		/* End key */
				stuffin("G");
				return -1;
			case 0x51:		/* PgDn key */
				stuffin(mkstr(CTRL('F')));
				return -1;
			case 0x49:		/* PgUp key */
				stuffin(mkstr(CTRL('B')));
				return -1;
			case 0x52:		/* insert key */
				return K_INSERT;
			case 0x53:		/* delete key */
				stuffin("x");
				return -1;
			/*
			 * Hard-code some useful function key macros.
			 */
			case 0x3b: /* F1 */
				stuffin(":help\n");
				return -1;
			case 0x3c: /* F2 */
				stuffin(":n\n");
				return -1;
			case 0x55: /* SF2 */
				stuffin(":n!\n");
				return -1;
			case 0x3d: /* F3 */
				stuffin(":N\n");
				return -1;
			case 0x56: /* SF3 */
				stuffin(":N!\n");
				return -1;
			case 0x3e: /* F4 */
				stuffin(":e #\n");
				return -1;
			case 0x57: /* SF4 */
				stuffin(":e! #\n");
				return -1;
			case 0x3f: /* F5 */
				stuffin(":rew\n");
				return -1;
			case 0x58: /* SF5 */
				stuffin(":rew!\n");
				return -1;
			case 0x40: /* F6 */
				stuffin("]]");
				return -1;
			case 0x59: /* SF6 */
				stuffin("[[");
				return -1;
			case 0x42: /* F8 - Set up global substitute */
				stuffin(":1,$s/");
				return -1;
			case 0x43: /* F9 - declare C variable */
				stuffin("yyp!!cdecl\n");
				return -1;
			case 0x5C: /* SF9 - explain C declaration */
				stuffin("yyp^iexplain \033!!cdecl\n");
				return -1;
			case 0x44: /* F10 - save & quit */
				stuffin(":x\n");
				return -1;
			case 0x5D: /* F10 - quit without saving */
				stuffin(":q!\n");
				return -1;
			default:
				break;
			}
			break;

		default:
			return c;
		}
	}
}


static	int	bpos = 0;
#ifdef BIOS

#define	BSIZE	256
static	char	outbuf[BSIZE];

/* Flushbuf() is used a little differently here in the BIOS-only interface
 * than in the case of other systems.  In general, the other systems buffer
 * large amounts of text and screen management data (escape sequences).
 * Here, only text is buffered, screen management is performed using BIOS
 * calls.  Hence, the buffer is much smaller since no more than one line of
 * text is buffered.  Also, screen management calls must assure that the
 * buffered text is output before performing the requested function.
 *
 * O.K.  Now I had better explain the tricky code sequences for IBM PC and
 * TI Pro.  In both cases, the tricks involve: 1) getting the text written
 * to the display as quickly as possible in the desired color and 2) assur-
 * ing that the cursor is positioned immediately following the latest text
 * output.
 *
 * On the IBM PC, we output the first character using the "write character
 * with attribute" function followed by code which outputs the buffer, a
 * character at a time, using the "write tty" function.  The first write
 * sets the display attributes, which are then reused by the "write tty"
 * function.  The "write tty" is then used to quickly write the data while
 * advancing the cursor.  The "write character with attribute" function
 * does not advance the cursor and so cannot be used to write the entire
 * buffer without additional code to advance the cursor in a separate oper-
 * ation.  Even though the first character in each buffer gets written
 * twice, the result is still substantially faster than it would be using a
 * "write character with attribute" - "[re]position cursor" sequence.
 *
 * On the TI Pro, we output the entire buffer using the "write character
 * string with attribute" function which is fast and convenient.  Unfortun-
 * ately, it does not advance the cursor.  Therefore, we include code which
 * determines the current location of the cursor, writes the buffer, then
 * positions the cursor at the end of the new data.
 *
 * I admit it, this is tricky, but it makes display updates much faster
 * than the would be using a more straightforward approach.
 */


void
flushbuf()				/* Flush buffered output to display */
{
	union	REGS	inregs, curregs, outregs;

	if (bpos != 0) {
		char	*bptr = outbuf;

		switch (host_type) {
		case hIBMPC:
			inregs.h.ah = 0x09;
			inregs.h.al = *bptr;
			inregs.h.bh = bgn_page;
			inregs.h.bl = P(P_CO);
			inregs.x.cx = 1;
			int86(crt_int, &inregs, &outregs);
			inregs.h.ah = 0x0E;
			while (bpos-- > 0) {
				inregs.h.al = *bptr++;
				int86(crt_int, &inregs, &outregs);
			}
			break;
		case hTIPRO:
			curregs.h.ah = 0x03;
			int86(crt_int, &curregs, &curregs);
			inregs.h.ah = 0x10;
			inregs.h.al = P(P_CO);
			inregs.x.bx = FP_OFF(outbuf);
			inregs.x.cx = bpos;
			inregs.x.dx = FP_SEG(outbuf);
			int86(crt_int, &inregs, &outregs);
			curregs.h.ah = 0x02;
			curregs.h.dh += bpos;
			int86(crt_int, &curregs, &outregs);
			break;
		}
	}
	bpos = 0;
}

void
write_tty(c)				/* Used to execute control chars */
char	c;
{
	int	curcol;

	union	REGS	inregs, curregs, outregs;

	flushbuf(); \

	switch (c) {
	case '\t':
		inregs.h.ah = 0x09;
		inregs.h.al = ' ';
		inregs.h.bh = bgn_page;
		inregs.h.bl = P(P_CO);
		inregs.x.cx = 1;
		int86(crt_int, &inregs, &outregs);
		inregs.h.ah = 0x0E;
		int86(crt_int, &inregs, &outregs);
		curregs.h.ah = 0x03;
		curregs.h.bh = bgn_page;
		int86(crt_int, &curregs, &outregs);
		curcol = host_type == hIBMPC ? outregs.h.dl : outregs.h.dh;
		while (curcol++ % P(P_TS)) int86(crt_int, &inregs, &outregs);
		break;
	case '\n':
		if (host_type == hTIPRO) bios_t_el();
		/* No break, fall through to default action. */
	default:
		inregs.h.ah = 0x0E;
		inregs.h.bh = bgn_page;
		inregs.h.al = c;
		int86(crt_int, &inregs, &outregs);
		break;
	}
}

#else		/* Not BIOS */

#define	BSIZE	2048
static	char	outbuf[BSIZE];

void
flushbuf()
{
	if (bpos != 0)
		write(1, outbuf, bpos);
	bpos = 0;
}

#endif


/*
 * Macro to output a character. Used within this file for speed.
 *
 * This macro had to be upgraded for the BIOS-only version because we
 * cannot count on flushbuf() to execute control characters such as
 * end-of-line or tab.  Therefore, when we encounter one, we flush
 * the buffer and call a routine which executes the character.
 */

#ifdef BIOS

#define	outone(cc) {                 \
  register char ch = cc;             \
  if (ch >= ' ') {                   \
    outbuf[bpos++] = ch;             \
    if (bpos >= BSIZE) flushbuf();   \
  } else write_tty(ch);              \
}

#else

#define	outone(c)	outbuf[bpos++] = c; if (bpos >= BSIZE) flushbuf()

#endif

/*
 * Function version for use outside this file.
 */

void
outchar(c)
char	c;
{
	outone(c);
}

/*
 * outstr(s) - write a string to the console
 */

void
outstr(s)
char	*s;
{
	while (*s) {
		outone(*s++);
	}
}

void
beep()
{
	if ( P(P_VB) )
		vbeep();
	else
		outchar('\007');
}

vbeep()		/* "Visual Bell" - reverse color flash */
{
	unsigned char oldcolor, revcolor, temp;

	oldcolor = P(P_CO);

	/* put reverse color in revcolor */
	revcolor = (P(P_CO) & 0x07) << 4;	/* foregnd -> bkgnd */
	temp     = (P(P_CO) & 0x70) >> 4;	/* bkgnd -> foregnd */
	revcolor  |= temp;

	/* Flash revcolor, then back */
	setcolor (revcolor);
	flushbuf();
#ifdef TURBOC
	delay (100);
#endif
	setcolor (oldcolor);
	windgoto (Cursrow, Curscol);
	flushbuf();
}


#ifndef TURBOC
sleep(n)
int	n;
{
	/*
	 * Should do something reasonable here.
	 */
}
#endif

void
pause()
{
	long	l;

	flushbuf();

#ifdef TURBOC
	delay (600);	/* "stop" for a fraction of a second */
#else
	/*
	 * Should do something better here...
	 */
	for (l=0; l < 5000 ;l++)
		;
#endif
}

void
sig()
{
	signal(SIGINT, sig);

	got_int = TRUE;
}

static	char	schar;		/* save original switch character */

/*	While Larry Shurr's addition of color and mode support was
 *	dependent on #define BIOS, there's no reason it needs to be.
 *	The BIOS is always there, even if NANSI.SYS isn't.  We'll
 *	use the BIOS where appropriate, and extend support to
 *	all cases of #define DOS.  This is especially true of the
 *	setup in windinit() and the termination in windexit().
 */

void
windinit()
{
	union	REGS	regs;
	struct	SREGS	sregs;

	/* The "SYSROM..." string is a signature in the TI Pro's ROM which
	 * which we can look for to determine whether or not we're running
	 * on a TI Pro.  If we don't find it at F400:800D,
	 * we assume we're running on an IBM PC or clone.
	 * Unfortunately, the signature is actually
	 * the system ROM's copyright notice though you will note that the
	 * year is omitted.  Still, placing it in this program might
	 * inadvertantly make it appear to be an official copyright notice
	 * for THIS program.  Hence, I have surrounded the signature
	 * string with disclaimers.
	 */

        static	char far *disclaimer1 =
	  "The following is *NOT* a copyright notice for this program: ";

	static	char far *ti_sig =
          "SYSROM (c) Copyright Texas Instruments Inc.";

	static	char far *disclaimer2[] = {
	  "\nInstead, it is a signature string we look for ",
	  "to distinguish the TI Pro computer.\n",
	  "Actually, this program is in the public domain."
	};

	static char far	*ti_sig_addr = (char far *)0xF400800D;
	static int ti_sig_len = sizeof(ti_sig) - 1;

	/* Identify the host type.  Look for signature in TI Pro ROM.  If */
	/* found, set host type to TI Pro, else assume host is an IBM PC. */

	host_type = strncmp(ti_sig, ti_sig_addr, ti_sig_len) ? hIBMPC : hTIPRO;

	/* Next, perform host-dependent initialization. */

	switch (host_type) {
	case hIBMPC:
		/* Get the video mode info */
		crt_int = 0x10;
		regs.h.ah = 0x0F;
		int86(crt_int, &regs, &regs);
		bgn_page = regs.h.bh;
		bgn_mode = regs.h.al;
		Columns = regs.h.ah;
		/*  Find the starting color, and save to restore later */
		regs.h.ah = 8;		/* Read char/attr BIOS fn */
		regs.h.bh = bgn_page;
		int86(crt_int, &regs, &regs);
		bgn_color = (int) regs.h.ah;
		P(P_CO) = bgn_color;
		break;
	case hTIPRO:
		Columns = 80;
		crt_int = 0x49;
		P(P_CO) = 0x0F;
		break;

	default:
		Columns = 80;
		break;
	}

	P(P_LI) = Rows = 25;

	schar = getswitch();
	setswitch('/');

	signal(SIGINT, sig);
#ifndef BIOS
	setraw (1);
#endif
}

void
windexit(r)
int r;
{

	union	REGS	regs;

	quitting_now = 1;

	/* Restore original color */
	setcolor (bgn_color);

	if (host_type == hIBMPC) {
		/* If we've changed any of the setup, reset the mode.
		 * Otherwise, leave stuff on the screen.
		 */
		regs.h.ah = 0x0F;	/* "Get-mode" BIOS fn */
		int86(0x10, &regs, &regs);
		if (bgn_mode != regs.h.al)
			set_mode (bgn_mode);
	}

	flushbuf();
	setswitch(schar);
#ifndef BIOS
	setraw(0);
#endif
	exit(r);
}


#ifndef BIOS
/*	Setraw sets the console driver into raw mode, which makes it run
 *	somewhat faster.  Details of the function:
 *	If r=1, remember current mode, and set into raw mode.
 *	   r=0, return to the original mode.
 */

setraw (r)
  int	r;
{
	static int origr=0;	/* save the original r */
	union REGS rr;

	/* Do IOCTL call to get current control info */
	rr.x.ax = 0x4400;	/* Read IOCTL info - DOS fn */
	rr.x.bx = 1;		/* Handle for stdout */
	intdos (&rr, &rr);

	/* Save relevant info, and modify for "set" call */
	if (r) {
		origr = rr.h.dl & 0x20;		/* save current "raw" bit */
		rr.h.dl = rr.h.dl | 0x20;	/* set "raw" bit */
	}
	else
		rr.h.dl = (rr.h.dl & (~0x20)) | (origr & 0x20);

	/* Do IOCTL call to set control info */
	rr.x.ax = 0x4401;	/* Set IOCTL function call */
	rr.x.bx = 1;		/* Handle for stdout */
	rr.h.dh = 0;		/* DL already set up */
	intdos (&rr, &rr);
}
#endif


void
windgoto(r, c)				/* Move cursor to r'ow & c'olumn */
register int	r, c;
{
#ifdef BIOS
	union	REGS	inregs, outregs;

	if (bpos > 0) flushbuf();

	inregs.h.ah = 0x02;

	switch (host_type) {
	case hIBMPC :
		inregs.h.bh = bgn_page;
		inregs.h.dh = r;
		inregs.h.dl = c;
		break;
	case hTIPRO:
		inregs.h.dh = c;
		inregs.h.dl = r;
		break;
	}

	int86(crt_int, &inregs, &outregs);

#else		/* Not BIOS */

	r += 1;
	c += 1;

	/*
	 * Check for overflow once, to save time.
	 */
	if (bpos + 8 >= BSIZE)
		flushbuf();

	outbuf[bpos++] = '\033';
	outbuf[bpos++] = '[';
	if (r >= 10)
		outbuf[bpos++] = r/10 + '0';
	outbuf[bpos++] = r%10 + '0';
	outbuf[bpos++] = ';';
	if (c >= 10)
		outbuf[bpos++] = c/10 + '0';
	outbuf[bpos++] = c%10 + '0';
	outbuf[bpos++] = 'H';

#endif
}

FILE *
fopenb(fname, mode)
char	*fname;
char	*mode;
{
	FILE	*fopen();
	char	modestr[16];

	sprintf(modestr, "%sb", mode);
	return fopen(fname, modestr);
}

static	char
getswitch()
{
	union	REGS	inregs, outregs;

	inregs.h.ah = 0x37;
	inregs.h.al = 0;

	intdos(&inregs, &outregs);

	return outregs.h.dl;
}

static	void
setswitch(c)
char	c;
{
	union	REGS	inregs, outregs;

	inregs.h.ah = 0x37;
	inregs.h.al = 1;
	inregs.h.dl = c;

	intdos(&inregs, &outregs);
}

#define	PSIZE	128

/*
 * fixname(s) - fix up a dos name
 *
 * Takes a name like:
 *
 *	\x\y\z\base.ext
 *
 * and trims 'base' to 8 characters, and 'ext' to 3.
 */
char *
fixname(s)
char	*s;
{
	char	*strchr(), *strrchr();
	static	char	f[PSIZE];
	char	base[32];
	char	ext[32];
	char	*p;
	int	i;

	strcpy(f, s);

	for (i=0; i < PSIZE ;i++)
		if (f[i] == '/')
			f[i] = '\\';

	/*
	 * Split the name into directory, base, extension.
	 */
	if ((p = strrchr(f, '\\')) != NULL) {
		strcpy(base, p+1);
		p[1] = '\0';
	} else {
		strcpy(base, f);
		f[0] = '\0';
	}

	if ((p = strchr(base, '.')) != NULL) {
		strcpy(ext, p+1);
		*p = '\0';
	} else
		ext[0] = '\0';

	/*
	 * Trim the base name if necessary.
	 */
	if (strlen(base) > 8)
		base[8] = '\0';

	if (strlen(ext) > 3)
		ext[3] = '\0';

	/*
	 * Paste it all back together
	 */
	strcat(f, base);
	strcat(f, ".");
	strcat(f, ext);

	return f;
}

void
doshell(cmd)
char	*cmd;
{
	if (cmd == NULL)
		if ((cmd = getenv ("COMSPEC")) == NULL)
			cmd = "command.com";

	system(cmd);
	wait_return();
}


/*
 *	setcolor (color)
 *	Set the screen attributes (basically, color) to value co.
 *	The color attributes for a DOS machine are the BIOS colors
 *	for text.  Where BIOS is not defined, we map the Escape
 *	sequences to the NANSI.SYS equivalents of the BIOS colors.
 */

setcolor (color)
  int	color;
{
#ifdef BIOS
	P(P_CO) = host_type == hIBMPC ? color : ((color & 0x17) | 0x08);
#else
	unsigned char work;

	/* Send the ANSI define-attribute sequence */
	outone('\033');
	outone('[');
	outone('0');		/* Normal color */
	outone(';');
	/* BIOS-to-NANSI color conversion may look a little bizarre.
	 * They have different bit orderings to represent the
	 * color (BIOS=RGB, NANSI=BGR).
	 *
	 * First put the foreground color.
	 */
	work = 0;
	if (color & 1)		work += 4;	/* Blue */
	if (color & 2)		work += 2;	/* Green */
	if (color & 4)		work += 1;	/* Red */
	outone('3');		/* NANSI foreground starts at 30 */
	outone(work + '0');
	outone(';');
	/*  Now the background color */
	work = 0;
	if (color & 0x10)	work += 4;	/* Blue */
	if (color & 0x20)	work += 2;	/* Green */
	if (color & 0x40)	work += 1;	/* Red */
	outone('4');		/* NANSI background starts at 40 */
	outone(work + '0');
	/*  Do the intensity and blinking, if any  */
	if (color & 8) {	/* intensity */
		outone(';');
		outone('1');
	}
	if (color & 0x80) {	/* blink */
		outone(';');
		outone('5');
	}
	/*  The 'm' suffix means "set graphic rendition"  */
	outone('m');
	P(P_CO) = color;

#endif		/* Not BIOS */

	if (!quitting_now) {
		screenclear();
		updatescreen();
	}
}


/*	setrows (r)
 *	Sets the screen to "r" rows, or lines, where "r" is a feasible
 *	value for the IBM PC with some common display.  In this function:
 *   -	We set the mode to 25-line or 43-line mode, assuming the display
 *	supports the requested mode.
 *   -	We set the logical number of lines that Stevie uses to "r",
 *	so that the screen USED may not be the same as the physical screen.
 *
 *	The function returns the number of rows set.
 */
setrows (r)
  int r;
{
	int	rphys, rlog;	/* physical and logical "r" */

	rphys = (r <= 25) ? 25 : 43 ;
	rlog  = (r <= 50) ? r : 50;

	/* Set the mode to correspond to the number of lines */
	set_mode (rphys);

	return (rlog);
}


set_mode (m)
  int m;
{
	set_25 ();
	if (m == 43)
		set_43 ();
}

#ifdef BIOS

int
set_25(lines)			/* Set display to 25 line mode */
int	lines;
{
	union	REGS	inregs, outregs;

	switch (host_type) {
	case hIBMPC:
		inregs.h.ah = 0x00;
		inregs.h.al = bgn_mode;
		int86(crt_int, &inregs, &outregs);
		break;
	case hTIPRO:
		windgoto(0, 0);
		inregs.h.ah = 0x09;
		inregs.h.al = ' ';
		inregs.h.bl = P(P_CO);
		inregs.x.cx = 80 * 25;
		int86(crt_int, &inregs, &outregs);
		if (lines > 25) lines = 25;
		break;
	}

	return(lines);
}

int
set_43(lines)			/* Set display to 43/50 line mode */
int	lines;
{
	union	REGS	inregs, outregs;

	switch (host_type) {
	case hIBMPC:
		inregs.x.ax = 0x1112;
		inregs.h.bl = 0;
		int86(crt_int, &inregs, &outregs);
		inregs.x.ax = 0x1200;
		inregs.h.bl = 0x20;
		int86(crt_int, &inregs, &outregs);
		inregs.h.ah = 0x01;
		inregs.x.cx = 0x0707;
		int86(crt_int, &inregs, &outregs);
		break;
	case hTIPRO:
		if (lines > 25) lines = 25;
		break;
	}

	return(lines);
}

#else		/* Not BIOS */

set_25 ()
{
	send_setmode (bgn_mode);
}

set_43 ()
{
	send_setmode (43);
}

send_setmode (m)
{
	outone('\033');
	outone('[');

	/* Convert 2-digit decimal to ASCII */
	if (m >= 10)
		outone( m/10 + '0' );
	outone( m%10 + '0' );
	outone ('h');
}

#endif		/* Not BIOS */

#ifdef BIOS
/*
 *	The rest of the file is BIOS-specific
 */

void
bios_t_ci()				/* Make cursor invisible */
{
	union	REGS	inregs, outregs;

	if (sav_curattr == 0) {
		inregs.h.ah = 0x03;
		inregs.h.bh = bgn_page;
		int86(crt_int, &inregs, &outregs);
		sav_curattr = outregs.x.cx;
		inregs.h.ah = 0x01;
		inregs.x.cx = 0x2000;
		int86(crt_int, &inregs, &outregs);
	}
}

void
bios_t_cv()				/* Make cursor visible */
{
	union	REGS	inregs, outregs;

	if (sav_curattr != 0) {
		inregs.h.ah = 0x01;
		inregs.h.bh = bgn_page;
		inregs.x.cx = sav_curattr;
		int86(crt_int, &inregs, &outregs);
		sav_curattr = 0;
	}
}

/*
 * O.K., I have tried to keep bios.c as "pure" as possible. I.e., I have used
 * BIOS calls for everything instead of going for all-out speed by using
 * direct-video access for updating the display - after all, I named it this
 * module bios.c.  There is one area, however, where using the BIOS is just
 * too much of a compromise... the TI Pro's "scroll display" functions are so
 * slow and ugly that I hate them.  True, they are very flexible, but their
 * poor on-screen appearance and low performance are a liability - you prob-
 * ably think I'm exaggerating, but you're wrong - it is truly bad.  There-
 * fore, I am bypassing them and scrolling the screen myself; something I
 * nearly always do.  From a purist like me, that really says something.
 */

void
bios_t_dl(r,l)				/* Delete lines */
int r, l;
{
	char	far	*end;		/* End ptr for TI Pro screen */
	char	far	*dst;		/* Dest ptr for scrolling TI Pro scrn */
	char	far	*src;		/* Src  ptr for scrolling TI Pro scrn */

	union	REGS	inregs, outregs;

	switch (host_type) {
	case hIBMPC:
		inregs.h.ah = 0x06;
		inregs.h.al = l;
		inregs.h.bh = P(P_CO);
		inregs.h.ch = r;
		inregs.h.cl = 0;
		inregs.h.dh = Rows - 1;
		inregs.h.dl = Columns - 1;
		int86(crt_int, &inregs, &outregs);
		break;
	case hTIPRO:
		inregs.h.ah = 0x17;
		int86(crt_int, &inregs, &outregs);
		dst = MK_FP(0xDE00, outregs.x.dx + (r * Columns));
		src = dst + (l * Columns);
		end = MK_FP(0xDE00, outregs.x.dx + ((Rows - 1) * Columns));
		while (src < end) *dst++ = *src++;
		while (dst < end) *dst++ = ' ';
		break;
	}
}

void
bios_t_ed()				/* Home cursor, erase display */
{
	union	REGS	inregs, outregs;

	windgoto(0, 0);

	inregs.h.ah = 0x09;
	inregs.h.al = ' ';
	inregs.h.bh = bgn_page;
	inregs.h.bl = P(P_CO);
	inregs.x.cx = Columns * Rows;
	int86(crt_int, &inregs, &outregs);
}

void
bios_t_el()				/* Erase to end-of-line */
{
	short	ccol;

	union	REGS	inregs, outregs;

	inregs.h.ah = 0x03;
	inregs.h.bh = bgn_page;
	int86(crt_int, &inregs, &outregs);

	inregs.h.ah = 0x09;
	inregs.h.al = ' ';
	inregs.h.bl = P(P_CO);

	ccol = host_type == hIBMPC ? outregs.h.dl : outregs.h.dh;

	inregs.x.cx = Columns - ccol;
	int86(crt_int, &inregs, &outregs);
}

/* As in the delete-line function, we scroll the TI display ourselves
 * rather than the use the slow-and-ugly software scroll in the BIOS.  See
 * the remarks for bios_t_dl() additional information.
 */

void
bios_t_il(r,l)				/* Insert lines */
int r, l;
{
	char	far	*end;		/* End ptr for TI Pro screen */
	char	far	*dst;		/* For scrolling TI Pro screen */
	char	far	*src;		/* For scrolling TI Pro screen */

	union	REGS	inregs, outregs;

	switch (host_type) {
	case hIBMPC:
		inregs.h.ah = 0x07;
		inregs.h.al = l;
		inregs.h.bh = P(P_CO);
		inregs.h.ch = r;
		inregs.h.cl = 0;
		inregs.h.dh = Rows - 1;
		inregs.h.dl = Columns - 1;
		int86(crt_int, &inregs, &outregs);
		break;
	case hTIPRO:
		inregs.h.ah = 0x17;
		int86(crt_int, &inregs, &outregs);
		dst = MK_FP(0xDE00, outregs.x.dx + (Columns * (Rows - 1)) - 1);
		src = dst - (Columns * l);
		end = MK_FP(0xDE00, outregs.x.dx + (Columns * r));
		while (src >= end) *dst-- = *src--;
		src = MK_FP(0xDE00, outregs.x.dx + (r * Columns));
		end = src + (l * Columns);
		while (src < end) *src++ = ' ';
		break;
	}
}

void
bios_t_rc()				/* Restore saved cursor position */
{
	union	REGS	inregs, outregs;

	inregs.h.ah = 0x02;
	inregs.h.bh = bgn_page;
	inregs.x.dx = sav_curpos;
	int86(crt_int, &inregs, &outregs);
}

void
bios_t_sc()				/* Save cursor position */
{
	union	REGS	inregs, outregs;

	inregs.h.ah = 0x03;
	inregs.h.bh = bgn_page;
	int86(crt_int, &inregs, &outregs);
	sav_curpos = outregs.x.dx;
}

#endif

