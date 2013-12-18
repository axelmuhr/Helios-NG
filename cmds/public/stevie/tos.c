/* $Header: /dsl/HeliosRoot/Helios/cmds/public/stevie/RCS/tos.c,v 1.1 1993/08/06 15:17:14 nickc Exp tony $
 *
 * System-dependent routines for the Atari ST.
 */

#include "stevie.h"

#include <osbind.h>

/*
 * inchar() - get a character from the keyboard
 *
 * Certain special keys are mapped to values above 0x80. These
 * mappings are defined in keymap.h. If the key has a non-zero
 * ascii value, it is simply returned. Otherwise it may be a
 * special key we want to map.
 *
 * The ST has a bug involving keyboard input that seems to occur
 * when typing quickly, especially typing capital letters. Sometimes
 * a value of 0x02540000 is read. This doesn't correspond to anything
 * on the keyboard, according to my documentation. My solution is to
 * loop when any unknown key is seen. Normally, the bell is rung to
 * indicate the error. If the "bug" value is seen, we ignore it completely.
 */
int
inchar()
{
	register long	c;

	for (;;) {
		c = Bconin(2);
	
		if ((c & 0xff) != 0)
			return ((int) c);
	
		switch ((int) (c >> 16) & 0xff) {
	
		case 0x62: return K_HELP;
		case 0x61: return K_UNDO;
		case 0x52: return K_INSERT;
		case 0x47: return K_HOME;
		case 0x48: return K_UARROW;
		case 0x50: return K_DARROW;
		case 0x4b: return K_LARROW;
		case 0x4d: return K_RARROW;
		case 0x29: return K_CCIRCM;	/* control-circumflex */
		
		/*
		 * Occurs due to a bug in TOS.
		 */
		case 0x54:
			break;
		/*
		 * Add the function keys here later if we put in support
		 * for macros.
		 */
	
		default:
			beep();
			break;
	
		}
	}
}

void
outchar(c)
char	c;
{
	if (c < ' ')
		Bconout(2, c);
	else
		Bconout(5, c);
}

void
outstr(s)
register char	*s;
{
	while (*s)
		Bconout(2, *s++);
}

/*
 * flushbuf() - a no-op for TOS
 */
void
flushbuf()
{
}

#define	BGND	0
#define	TEXT	3

/*
 * vbeep() - visual bell
 */
static void
vbeep()
{
	int	text, bgnd;		/* text and background colors */
	long	l;

	text = Setcolor(TEXT, -1);
	bgnd = Setcolor(BGND, -1);

	Setcolor(TEXT, bgnd);		/* swap colors */
	Setcolor(BGND, text);

	for (l=0; l < 5000 ;l++)	/* short pause */
		;

	Setcolor(TEXT, text);		/* restore colors */
	Setcolor(BGND, bgnd);
}

void
beep()
{
	if (P(P_VB))
		vbeep();
	else
		outchar('\007');
}

/*
 * remove(file) - remove a file
 */
void
remove(file)
char	*file;
{
	Fdelete(file);
}

/*
 * rename(of, nf) - rename existing file 'of' to 'nf'
 */
void
rename(of, nf)
char	*of, *nf;
{
	Fdelete(nf);		/* if 'nf' exists, remove it */
	Frename(0, of, nf);
}

void
windinit()
{
	if (Getrez() == 0)
		Columns = 40;		/* low resolution */
	else
		Columns = 80;		/* medium or high */

	P(P_LI) = Rows = 25;

	Cursconf(1,NULL);
}

void
windexit(r)
int	r;
{
	exit(r);
}

static	char	gobuf[5] = { '\033', 'Y', '\0', '\0', '\0' };

void
windgoto(r, c)
int	r, c;
{
	gobuf[2] = r + 040;
	gobuf[3] = c + 040;
	outstr(gobuf);
}

/*
 * System calls or library routines missing in TOS.
 */

void
sleep(n)
int	n;
{
	int	k;

	k = Tgettime();
	while ( Tgettime() <= k+n )
		;
}

void
pause()
{
	long	n;

	for (n = 0; n < 8000 ;n++)
		;
}

int
system(cmd)
char	*cmd;
{
	char	arg[1];

	arg[0] = (char) 0;	/* no arguments passed to the shell */

	if (Pexec(0, cmd, arg, 0L) < 0)
		return -1;
	else
		return 0;
}

#ifdef	SOZOBON

FILE *
fopenb(fname, mode)
char	*fname;
char	*mode;
{
	char	modestr[10];

	sprintf(modestr, "%sb", mode);

	return fopen(fname, modestr);
}

#endif

#ifndef	SOZOBON
/*
 * getenv() - get a string from the environment
 *
 * Both Alcyon and Megamax are missing getenv(). This routine works for
 * both compilers and with the Beckemeyer and Gulam shells. With gulam,
 * the env_style variable should be set to either "mw" or "gu".
 */
char *
getenv(name)
char	*name;
{
	extern long	_base;
	char	*envp, *p;

	envp = *((char **) (_base + 0x2c));

	for (; *envp ;envp += strlen(envp)+1) {
		if (strncmp(envp, name, strlen(name)) == 0) {
			p = envp + strlen(name);
			if (*p++ == '=')
				return p;
		}
	}
	return (char *) 0;
}
#endif

/*
 * mktemp() - quick hack since there isn't one here
 */
char *
mktemp(name)
char	*name;
{
	int	num;		/* pasted into the string to make it unique */
	char	cbuf[7];
	char	*s;		/* where the X's start in name */
	int	fd;

	if ((s = strchr(name, 'X')) == NULL)	/* needs to be an X */
		return (char *) NULL;

	if (strlen(s) != 6)			/* should be 6 X's */
		return (char *) NULL;

	for (num = 0; num < 1000 ;num++) {
		sprintf(cbuf, "%06d", num);
		strcpy(s, cbuf);
		if ((fd = open(name, 0)) < 0)
			return name;
		close(fd);
	}
	return (char *) NULL;
}

void
doshell(cmd)
char	*cmd;
{
	if (cmd == NULL) {
		shell();
		return;
	}
	system(cmd);
	wait_return();
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

