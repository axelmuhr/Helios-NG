/* $Header: /dsl/HeliosRoot/Helios/cmds/public/stevie/RCS/os2.c,v 1.1 1993/08/06 15:17:14 nickc Exp tony $
 *
 * OS/2 System-dependent routines.
 */

#define	INCL_BASE
#include <os2.h>
#include <signal.h>
#include "stevie.h"

/*
 * inchar() - get a character from the keyboard
 */
int
inchar()
{
	register int	c;

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
			case 0x52:
				return K_INSERT;
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

#define	BSIZE	2048
static	char	outbuf[BSIZE];
static	int	bpos = 0;

void
flushbuf()
{
	if (bpos != 0)
		write(1, outbuf, bpos);
	bpos = 0;
}

/*
 * Macro to output a character. Used within this file for speed.
 */
#define	outone(c)	outbuf[bpos++] = c; if (bpos >= BSIZE) flushbuf()

/*
 * Function version for use outside this file.
 */
void
outchar(c)
register char	c;
{
	outbuf[bpos++] = c;
	if (bpos >= BSIZE)
		flushbuf();
}

static	char	cell[2] = { 0, 7 };

/*
 * outstr(s) - write a string to the console
 *
 * We implement insert/delete line escape sequences here. This is kind
 * of a kludge, but at least it's localized to a single point.
 */
void
outstr(s)
register char	*s;
{
	if (strcmp(s, T_DL) == 0) {		/* delete line */
		int	r, c;

		flushbuf();
		VioGetCurPos(&r, &c, 0);
		VioScrollUp(r, 0, 100, 100, 1, cell, 0);
		return;
	}
	if (strcmp(s, T_IL) == 0) {		/* insert line */
		int	r, c;

		flushbuf();
		VioGetCurPos(&r, &c, 0);
		VioScrollDn(r, 0, 100, 100, 1, cell, 0);
		return;
	}

	while (*s) {
		outone(*s++);
	}
}

void
beep()
{
	in ( P(P_VB) )
		vbeep();
	else
		outone('\007');
}

sleep(n)
int	n;
{
	DosSleep(1000L * n);
}

void
pause()
{
	flushbuf();
	DosSleep(300L);
}

void
sig()
{
	signal(SIGINT, sig);

	got_int = TRUE;
}

void
windinit()
{
	Columns = 80;
	P(P_LI) = Rows = 25;

	signal(SIGINT, sig);
}

void
windexit(r)
int r;
{
	flushbuf();
	exit(r);
}

void
windgoto(r, c)
register int	r, c;
{
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
		cmd = "cmd.exe";

	system(cmd);
	wait_return();
}

/*
 *	FILL IT IN, FOR YOUR SYSTEM, AND SHARE IT!
 *
 *	The next couple of functions do system-specific stuff.
 *	They currently do nothing; I'm not familiar enough with
 *	system-specific programming on this system.
 *	If you fill it in for your system, please post the results
 *	and share with the rest of us.
 */


setcolor (c)
/*
 * Set the color to c, using the local system convention for numbering
 * colors or video attributes.
 *
 * If you implement this, remember to note the original color in
 * windinit(), before you do any setcolor() commands, and
 * do a setcolor() back to the original as part of windexit().
 */
  int c:
{
}


setrows (r)
/*
 * Set the number of lines to r, if possible.  Otherwise
 * "do the right thing".  Return the number of lines actually set.
 *
 * If you implement this, remember to note the original number of rows
 * in windinit(), before you do any setrows() commands, and
 * do a setrows() back to the original as part of windexit().
 */
  int r;
{
	/* Since we do nothing, just return the current number of lines */
	return ( P(P_LI) );
}


vbeep ()
/*
 * Do a "visual bell".  This generally consists of flashing the screen
 * once in inverse video.
 */
{
	int	color, revco;

	color = P( P_CO );		/* get current color */
	revco = reverse_color (color);	/* system-specific */
	setcolor (revco);
	flushbuf ();
	pause ();
	setcolor (color);
	windgoto (Cursrow, Curscol);
	flushbuf ();
}

reverse_color (co)
/*
 * Returns the inverse video attribute or color of co.
 * The existing code below is VERY simple-minded.
 * Replace it with proper code for your system.
 */
 int co;
{
	if (co)		return (0);
	else		return (1);
}


/********** End of do-it-yourself kit **********************/

