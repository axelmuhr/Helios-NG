/*
 * OS2NONPM.C
 *
 * The routines in this file provide video and keyboard support using the
 * OS/2 Vio and Kbd functions (not the presentation manager).
 *
 * The os2putc, os2eeol and os2eeop routines modify the logical video
 * buffer.  Os2flush calls VioShowBuf to update the physical video buffer.
 * An earlier version used VioWrtTTy with ANSI processing (easy to do, but
 * sloooow).  A later version using VioWrtNCell was better, but not as
 * good as manipulating the logical buffer.
 */

#define	termdef	1			/* don't define "term" external */

#include	<stdio.h>
#include	"estruct.h"
#include	"etype.h"
#include	"edef.h"
#include	"elang.h"

#if     OS2NPM
/*
 * Os2def.h defines COLOR, but no MSC stuff needs it.
 * We need COLOR as defined in estruct.h, so edit it out of os2def.h.
 */
#define INCL_BASE
#include	<os2.h>
#include	<conio.h>

#define	NROW    50		/* Screen size.                 */
#define	NCOL    80		/* Edit if you want to.         */
#define	MARGIN	8		/* size of minimim margin and	*/
#define	SCRSIZ	64		/* scroll size for extended lines */
#define	NPAUSE	100		/* # times thru update to pause */

#define CDCGA	0		/* color graphics adapter	*/
#define CDMONO	1		/* monochrome display adapter	*/
#define CDEGA	2		/* EGA				*/
#define CDVGA	3		/* VGA				*/

#define NDRIVE	4		/* number of video modes	*/

int dtype = -1;				/* current video mode	*/
char drvname[][8] = {			/* names of video modes	*/
	"CGA", "MONO", "EGA", "VGA"
};

/* Forward references.          */
os2move();
os2eeol();
os2eeop();
os2beep();
os2open();
os2close();
os2getc();
os2putc();
int os2flush(void);
os2rev();
os2kclose();
os2kopen();
os2cres();
os2parm();
#if	COLOR
os2fcol();
os2bcol();
#endif

struct {			/* Current screen attribute for ORing	*/
	BYTE	filler;		/* with character to be displayed.	*/
	BYTE	attr;
} os2cell = {0, 0x07};

static struct {				/* initial states		*/
	USHORT		ansiState;	/* ANSI translation		*/
	VIOCONFIGINFO	vioConfigInfo;	/* video configuration		*/
	VIOMODEINFO	vioModeInfo;	/* video mode			*/
	KBDINFO		kbdInfo;	/* keyboard info		*/
} initial;

static int	cfcolor = -1;		/* current foreground color	*/
static int	cbcolor = -1;		/* current background color	*/
static int	ctrans[] =	/* ansi to ibm color translation table	*/
	{0, 4, 2, 6, 1, 5, 3, 7,
	 8, 12, 10, 14, 9, 13, 11, 15};

static short	 os2row;		/* current cursor row	*/
static short	 os2col;		/* current cursor col	*/

/*
 * To minimize the amount of buffer that VioShowBuf has to update, we
 * keep track of the lowest and highest bytes in the logical video
 * buffer which have been modified.
 */
static USHORT	*lvb;			/* logical video buffer	*/
static USHORT	 lvbLen;		/* length of buffer	*/
static USHORT	 lvbMin;		/* min index of modified byte	*/
static USHORT	 lvbMax;		/* max index of modified byte	*/

/*
 * Standard terminal interface dispatch table.
 */
TERM    term    = {
	NROW-1,
	NROW-1,
	NCOL,
	NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
	os2open,
	os2close,
	os2kopen,
	os2kclose,
	os2getc,
	os2putc,
	os2flush,
	os2move,
	os2eeol,
	os2eeop,
	os2beep,
	os2rev,
	os2cres
#if	COLOR
	, os2fcol,
	os2bcol
#endif
};


#if	COLOR
/*----------------------------------------------------------------------*/
/*	os2fcol()							*/
/* Set the current foreground color.					*/
/*----------------------------------------------------------------------*/

os2fcol(
	int color)			/* color to set */
{
	if (dtype != CDMONO)
		cfcolor = ctrans[color];
	else
		cfcolor = 7;
	os2cell.attr &= 0xF0;
	os2cell.attr |= cfcolor;
	
}


/*----------------------------------------------------------------------*/
/*	os2bcol()							*/
/* Set the current background color.					*/
/*----------------------------------------------------------------------*/

os2bcol(
	int color)		/* color to set */
{
	if (dtype != CDMONO)
		cbcolor = ctrans[color];
	else
		cbcolor = 0;
	os2cell.attr &= 0x0F;
	os2cell.attr |= cbcolor << 4;
}
#endif


/*----------------------------------------------------------------------*/
/*	os2move()							*/
/* Move the cursor.							*/
/*----------------------------------------------------------------------*/

os2move(
	int row,
	int col)
{
	os2row = row;
	os2col = col;
	VioSetCurPos(os2row, os2col, 0);
}


/*----------------------------------------------------------------------*/
/*	os2flush()							*/
/* Update the physical video buffer from the logical video buffer.	*/
/*----------------------------------------------------------------------*/

int os2flush(void)
{
	if (lvbMin <= lvbMax) {		/* did anything change?	*/
		VioShowBuf(lvbMin * 2, (lvbMax - lvbMin + 1) * 2, 0);
		VioSetCurPos(os2row, os2col, 0);
	}
	lvbMin = lvbLen;
	lvbMax = 0;
}


/*----------------------------------------------------------------------*/
/*	os2getc()							*/
/* Get a character from the keyboard.					*/
/* Function keys, editing keys and alt- keys take two consecutive calls	*/
/* to os2getc().  The first returns zero and the second returns the	*/
/* key's scan code.							*/
/* Nextc holds the scan code until we are called again.			*/
/*----------------------------------------------------------------------*/

os2getc()
{
	KBDKEYINFO	keyInfo;
	static int	nextc = -1;  /* -1 when not holding a scan code	*/

	if (nextc != -1) {
		keyInfo.chChar = (char)nextc;
		nextc = -1;
		return keyInfo.chChar;		/* return the scan code	*/
	}
	
	KbdCharIn(&keyInfo, IO_WAIT, 0);	/* get a character	*/

	/* Function, edit or alt- key?	*/
	if (keyInfo.chChar == 0  ||  keyInfo.chChar == 0xE0) {
		nextc = keyInfo.chScan;		/* hold on to scan code	*/
		return 0;
	}
	return keyInfo.chChar;
}


#if	TYPEAH
/*----------------------------------------------------------------------*/
/*	typahead()							*/
/* Returns true if a key has been pressed.				*/
/*----------------------------------------------------------------------*/

typahead()

{
	return kbhit();
}
#endif


/*----------------------------------------------------------------------*/
/*	os2putc()							*/
/* Put a character at the current position in the current colors.	*/
/* Note that this does not behave the same as putc() or VioWrtTTy().	*/
/* This routine does nothing with returns and linefeeds.  For backspace */
/* it puts a space in the previous column and moves the cursor to the	*/
/* previous column.  For all other characters, it will display the	*/
/* graphic representation of the character and put the cursor in the	*/
/* next column (even if that is off the screen.  In practice this isn't	*/
/* a problem.								*/
/*----------------------------------------------------------------------*/

os2putc(int c)
{
	USHORT	cell;
	USHORT	i;

	if (c == '\n' || c == '\r') {		/* returns and linefeeds */
		return;
	}
	if (c == '\b') {			/* backspace		*/
		cell = ' ' | *(USHORT *)&os2cell;
		--os2col;			/* move cursor back	*/
		i = os2row * term.t_ncol + os2col;
	}
	else {
		cell = (0x00ff & c) | *(USHORT *)&os2cell;
		i = os2row * term.t_ncol + os2col;
		++os2col;			/* move cursor forward	*/
	}
	lvb[i] = cell;
	if (i < lvbMin)
		lvbMin = i;
	if (i > lvbMax)
		lvbMax = i;
}


/*----------------------------------------------------------------------*/
/*	os2eeol()							*/
/* Erase to end of line.						*/
/*----------------------------------------------------------------------*/

os2eeol()
{
	USHORT	cell = ' ';
	USHORT  i;

	cell |= *(USHORT *)&os2cell;
	
	i = os2row * term.t_ncol + os2col;  /* current cursor position	*/
	if (i < lvbMin)
		lvbMin = i;
	while (i < os2row * term.t_ncol + term.t_ncol)
		lvb[ i++] = cell;
	if (--i > lvbMax)
		lvbMax = i;
}


/*----------------------------------------------------------------------*/
/*	os2eeop()							*/
/* Erase to end of page.						*/
/*----------------------------------------------------------------------*/

os2eeop()
{
	USHORT	cell = ' ';
	USHORT  i;

#if COLOR
	if (dtype != CDMONO)
		cell |= (ctrans[gbcolor] << 4 | ctrans[gfcolor]) << 8;
	else
		cell |= 0x0700;
#else
	cell |= 0x0700;
#endif

	i = os2row * term.t_ncol + os2col;  /* current cursor position	*/
	if (i < lvbMin)
		lvbMin = i;
	while (i < term.t_nrow * term.t_ncol + term.t_ncol)
		lvb[ i++] = cell;
	if (--i > lvbMax)
		lvbMax = i;
}

/*----------------------------------------------------------------------*/
/*	os2rev()							*/
/* Change reverse video state.  (Does nothing.)				*/
/*----------------------------------------------------------------------*/

os2rev(int state)
{
}

/*----------------------------------------------------------------------*/
/*	os2cres()							*/
/* Change the screen resolution.					*/
/*----------------------------------------------------------------------*/

os2cres(char *res)		/* name of desired video mode	*/
{
	USHORT	err;
	int	type;			/* video mode type	*/
	VIOMODEINFO vioModeInfo;

	vioModeInfo = initial.vioModeInfo;
	
	/* From the name, find the type of video mode.	*/
	for (type = 0; type < NDRIVE; type++) {
		if (strcmp(res, drvname[type]) == 0)
			break;
	}
	if (type == NDRIVE)
		return(FALSE);	/* not a mode we know about	*/

		
	switch (type) {
		case CDMONO:
		case CDCGA:
			vioModeInfo.row = 25;
			break;
		case CDEGA:
			vioModeInfo.row = 43;
			break;
		case CDVGA:
			vioModeInfo.row = 50;
			break;
	}
	
	if (VioSetMode(&vioModeInfo, 0))	/* change modes 	*/
		return(FALSE);			/* couldn't do it	*/

	newsize(TRUE, vioModeInfo.row);

	/* reset the $sres environment variable */
	strcpy(sres, drvname[type]);
	dtype = type;				/* set the current mode	*/
	
	return TRUE;
}


/*----------------------------------------------------------------------*/
/*	spal()								*/
/* Change pallette settings.  (Does nothing.)				*/
/*----------------------------------------------------------------------*/

spal(dummy)
{
}


/*----------------------------------------------------------------------*/
/*	os2beep()							*/
/*----------------------------------------------------------------------*/

os2beep()
{
	DosBeep(1200, 175);
}


/*----------------------------------------------------------------------*/
/*	os2open()							*/
/* Find out what kind of video adapter we have and the current video	*/
/* mode.  Even if the adapter supports a higher resolution mode than is */
/* in use, we still use the current mode.				*/
/*----------------------------------------------------------------------*/

os2open()
{
	initial.vioConfigInfo.cb = 0x0A;
	VioGetConfig(0, &initial.vioConfigInfo, 0);
	switch (initial.vioConfigInfo.adapter) {
		case 3:
			dtype = CDVGA;
			break;
		case 2:
			dtype = CDEGA;
			break;
		case 1:
			dtype = CDCGA;
			break;
		case 0:
		default:
			dtype = CDMONO;
	}
	strcpy(sres, drvname[dtype]);

	initial.vioModeInfo.cb = 0x0E;
	VioGetMode(&initial.vioModeInfo, 0);
	newsize(TRUE, initial.vioModeInfo.row);
			
	VioGetAnsi(&initial.ansiState, 0);
	VioGetBuf(&lvb, &lvbLen, 0);
	lvbMin = lvbLen;
	lvbMax = 0;
}


/*----------------------------------------------------------------------*/
/*	os2close()							*/
/* Restore the original video settings.					*/
/*----------------------------------------------------------------------*/

os2close()
{
	VioSetAnsi(initial.ansiState, 0);
	VioSetMode(&initial.vioModeInfo, 0);
	VioSetCurPos(initial.vioModeInfo.row - 1,
			 initial.vioModeInfo.col - 1, 0);
}

/*----------------------------------------------------------------------*/
/*	os2kopen()							*/
/* Open the keyboard.							*/
/*----------------------------------------------------------------------*/

os2kopen()
{
	KBDINFO	kbdInfo;

	initial.kbdInfo.cb = 0x000A;
	KbdGetStatus(&initial.kbdInfo, 0);	
	kbdInfo = initial.kbdInfo;
	kbdInfo.fsMask &= ~0x0001;		/* not echo on		*/
	kbdInfo.fsMask |= 0x0002;		/* echo off		*/
	kbdInfo.fsMask &= ~0x0008;		/* cooked mode off	*/
	kbdInfo.fsMask |= 0x0004;		/* raw mode		*/
	kbdInfo.fsMask &= ~0x0100;		/* shift report	off	*/
	KbdSetStatus(&kbdInfo, 0);
}


/*----------------------------------------------------------------------*/
/*	os2kclose()							*/
/* Close the keyboard.							*/
/*----------------------------------------------------------------------*/

os2kclose()
{
	KbdSetStatus(&initial.kbdInfo, 0); /* restore original state	*/
}


#if 0
/*----------------------------------------------------------------------*/
/*	scwrite()							*/
/* Write a line to the screen.
/* I tried using this routine with MEMMAP = 1, but there were too many	*/
/* problems with the cursor and flushing the buffer.			*/
/*----------------------------------------------------------------------*/

scwrite(
	int	 row,	/* row of screen to place outstr on */
	char	*outstr,/* string to write out (must be term.t_ncol long) */
	int	 forg,	/* foreground color of string to write */
	int	 bacg,	/* background color */
{
	USHORT	 attr;
	int	 i;

	attr = (((ctrans[bacg] & 15) << 4) | (forg & 15)) << 8;

	for (i = row * term.t_ncol;  i < (row + 1) * term.t_ncol;  i++)
		lvb[i] = attr | *(outstr++);

	if (i < lvbMin)
		lvbMin = i;
	if (i > lvbMax)
		lvbMax = i;
}
#endif
#endif

