/*

The routines in this file provide support for the Atari 520 or 1040ST
using VT52 emulation.  The I/O services are provided here as well.  It
compiles into nothing if not a 520ST style device.

 $Header: /usr/perihelion/Helios/cmds/emacs/RCS/st520.c,v 1.1 90/08/23 15:16:42 jon Exp $

*/

#define	termdef	1			/* don't define "term" external */

#include        <stdio.h>
#include        "estruct.h"
#include	"edef.h"

#if MEGAMAX
overlay "st520"
#endif

#if     ATARI & ST520 & MEGAMAX
#include	<osbind.h>
#include	<ctype.h>

#define LINEA_INIT 0xA000
#define V_CEL_WR   -0x28
#define V_CEL_MY   -0x2a
#define V_CEL_HT   -0x2e
#define V_FNT_AD   -0x16
#define V_OFF_AD   -0x0a
#define V_DISAB    -346

#define NROW    25                      /* Screen size.                 */
#define NCOL    80                      /* Edit if you want to.         */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define	NPAUSE	25			/* # times thru update to pause */
#define BIAS    0x20                    /* Origin 0 coordinate bias.    */
#define ESC     0x1B                    /* ESC character.               */
#define BEL     0x07                    /* ascii bell character         */

extern  int     ttopen();               /* Forward references.          */
extern  int     ttgetc();
extern  int     ttputc();
extern  int     ttflush();
extern  int     ttclose();
extern  int     st520move();
extern  int     st520eeol();
extern  int     st520eeop();
extern  int     st520beep();
extern  int     st520open();
extern	int	st520close();
extern	int	st520rev();
extern  int st520kopen();
extern  int st520kclose();
extern	int st520chgrez();

#if	COLOR
extern	int	st520fcol();
extern	int	st520bcol();

int		cfcolor = -1;		/* current fg (character) color */
int		cbcolor = -1;		/* current bg color */
int		oldpal[8];		/* pallette when emacs was invoked */
int		newpal[8] = {		/* default emacs pallette */
	0x000, 0x700, 0x070, 0x770, 0x007, 0x707, 0x077, 0x777};
#endif

int STncolors = 0;		/* number of colors  */
int STrez;			/* physical screen resolution */	

/*
 * Dispatch table. All the
 * hard fields just point into the
 * terminal I/O code.
 */
TERM    term    = {
        NROW-1,
        NCOL,
	MARGIN,
	MARGIN,
	SCRSIZ,
	NPAUSE,
        &st520open,
        &st520close,
	&st520kopen,
	&st520kclose,
        &ttgetc,
        &ttputc,
        &ttflush,
        &st520move,
        &st520eeol,
        &st520eeop,
        &st520beep,
        &st520rev
#if	MULTREZ
	, &st520chgrez
#endif
#if	COLOR
	, &st520fcol,
	&st520bcol
#endif
};
	struct KBDvecs {
		int (*midivec) ();
		int (*vkbderr) ();
		int (*vmiderr) ();
		int (*statvec) ();
		int (*mousevec) ();
		int (*clockvec) ();
		int (*joyvec) ();
		int (*midisys) ();
		int (*ikbdsys) ();
	};
	struct Param {
		char topmode;
		char buttons;
		char xparam;
		char yparam;
		int xmax,ymax;
		int xinitial,yinitial;
	};
	struct KBDvecs *kbdvecs;
	struct Param *paramp;
	char kbdcmds[25];

st520move(row, col)
{
        ttputc(ESC);
        ttputc('Y');
        ttputc(row+BIAS);
        ttputc(col+BIAS);
}

st520eeol()
{
        ttputc(ESC);
        ttputc('K');
}

st520eeop()
{

#if	COLOR
		st520fcol(gfcolor);
		st520bcol(gbcolor);
#endif
        ttputc(ESC);
        ttputc('J');
}

st520rev(status)	/* set the reverse video state */

int status;	/* TRUE = reverse video, FALSE = normal video */

{

	if(status) {
		ttputc(ESC);
		ttputc('p');
	}
	else {
		ttputc(ESC);
		ttputc('q');
	}
}

#if	COLOR
st520fcol(color)
int color;	
{
		if(color == cfcolor || !STncolors)
			return;
		else {

			ttputc(ESC);
			ttputc('b');
			ttputc(color & 0x0f);
			cfcolor = color;
		}
}

st520bcol(color)
int color;
{
		if(color == cbcolor || !STncolors)
			return;
		else {
			ttputc(ESC);
			ttputc('c');
			ttputc(color & 0x0f);
			cbcolor = color;
		}

}
#endif

st520beep()
{
#ifdef  BEL
        ttputc(BEL);
        ttflush();
#endif
}

st520open()
{
	int i,j,k;
	long phys, log;	/* screen bases */
	
/* IMPORTANT: it is ABSOLUTELY necessary that the default resolution be the
 *	largest possible so that display will allocate (malloc) the maximum
 *	size for the VIDEO arrray
 */
	STrez = Getrez();
	switch(STrez) {
		case 0: /* low res 25x40 16 colors */
			phys = Physbase();
			log  = Logbase();
			Setscreen(log, phys, 1);
			STrez = 1;
			/* fall thru to med res */

		case 1: /* med res 25x80 4 colors */
			term.t_nrow = 25 - 1;
			term.t_ncol  = 80;
			grez = 1;
#if	COLOR
			STncolors = 4;
			for(i=0;i<8;i++) {
				oldpal[i] = Setcolor(i,newpal[i]);
			}
#endif
			break;
		case 2: /* high res 25x80 no colors */
			term.t_nrow  = 40 - 1;
			term.t_ncol  = 80;
			grez = 2;
			make_8x10(); /* create a smaller font */
			set_40();    /* and go to 40 line mode */
#if	COLOR
			STncolors = 0;
#endif
			break;
	}

	revexist = TRUE;
	eolexist = TRUE;
	paramp = (struct Param *)malloc(sizeof(struct Param));
	kbdvecs = (struct KBDvecs *)Kbdvbase();
	paramp -> topmode = 0;
	paramp -> buttons = 4;
	paramp -> xparam = 8;
	paramp -> yparam = 10;
	paramp -> xmax = 79;
	paramp -> ymax = 23;
	paramp -> xinitial = 0;
	paramp -> yinitial = 0;
	Initmous(1,paramp,kbdvecs -> mousevec);

	i = 0;
	kbdcmds[i++] = 0x0a;	/*set mouse keycode mode */
	kbdcmds[i++] = 0x08;
	kbdcmds[i++] = 0x0a;
	Ikbdws(i-1,&kbdcmds[0]);
	Cursconf(1,0);
	Cursconf(3,0);
	Cconout(27);Cconout('E');
        ttopen();
}

st520close()

{
	int i,j,k;

	i = 0;
	kbdcmds[i++] = 0x80;	/*reset mouse keycode mode */
	kbdcmds[i++] = 0x01;
	Ikbdws(i-1,&kbdcmds[0]);
	if(grez == 2 && STrez == 2) /* b/w monitor in 40 row mode */
		restore();

#if		COLOR
	for(i=0;i<STncolors;i++)
		Setcolor(i,oldpal[i]);
#endif
	Cconout(27);Cconout('E');
	paramp -> buttons = 0;
	Initmous(2,paramp,kbdvecs -> mousevec);
	i = 0;
	kbdcmds[i++] = 0x80;	/*reset the keyboard*/
	kbdcmds[i++] = 0x01;
	Ikbdws(i-1,&kbdcmds[0]);
	Cursconf(1,0);
	ttclose();
}
st520kopen()
{

}
st520kclose()
{

}

st520chgrez(nurez)
int nurez;
{
	int ierr, i, j ,k;
	long phys, log;	/* screen bases */
	char dum[80]; /* for debugging only */
		
	if(grez == nurez)
		return(TRUE);
		
	if(STrez == 2) { /* b/w monitor-only allow hi | med rez */
		switch(nurez) {
			case 2: /* high res */
				term.t_nrow  = 40 - 1;
				term.t_ncol  = 80;
				make_8x10(); /* create a smaller font */
				set_40();    /* and go to 40 line mode */
				grez = 2;
				sgarbf = TRUE;
				onlywind(1,1);
				break;
			case 1: /* med res */
				term.t_nrow  = 25 - 1;
				term.t_ncol  = 80;
				restore();
				grez = 1;
				sgarbf = TRUE;
				onlywind(1,1);
				break;
			default:
				mlwrite("Invalid resolution");
				return(FALSE);
				break;
		}
	}
	else { /* color monitor-only allow low | medium resolution */
		phys = Physbase();
		log  = Logbase();
		switch(nurez) {
			case 1:
				term.t_nrow  = 25 - 1;
				term.t_ncol  = 80;
				Setscreen(log, phys, 1);
				STncolors = 4;
				grez = 1;
				sgarbf = TRUE;
				onlywind(1,1);
				break;
			case 0:
				term.t_nrow  = 25 - 1;
				term.t_ncol  = 40;
				Setscreen(log, phys, 0);
				STncolors = 8;
				grez = 0;
				sgarbf = TRUE;
				onlywind(1,1);
				break;
			default:
				mlwrite("%Invalid resolution");
				return(FALSE);
				break;
		}
	}
}			

STcurblink(onoff)
int onoff;
{
	if(onoff)
		Cursconf(2,0);
	else
		Cursconf(3,0);
}


char parm_save[28];
long fnt_8x10[640];

make_8x10()
{
	int i,j,k;
	long savea23[2];
	
	for(i=0;i<640;i++)
		fnt_8x10[i] = 0;
		
	asm {
	movem.l	A2-A3,savea23(A6)
	
	dc.w	LINEA_INIT		;A1 -> array of font headers

	lea	parm_save(A4),A2	;A2 -> parameters savearea
	move.l	V_OFF_AD(A0),(A2)+
	move.l	V_FNT_AD(A0),(A2)+
	move.w	V_CEL_HT(A0),(A2)+
	move.w	V_CEL_MY(A0),(A2)+
	move.w	V_CEL_WR(A0),(A2)+


	move.l	04(A1),A1		; A1 -> 8x8 font header
	move.l	76(A1),A2		; A2 -> 8x8 font data
	lea	fnt_8x10+0x100(A4),A3	; A3 -> 2nd line of font buffer
	move.w	#0x200-1,D0		; D0 <- longword counter for font xfer

fnt_loop:

	move.l	(A2)+,(A3)+
	dbf	D0,fnt_loop
		
	movem.l	savea23(A6),A2-A3
	}
	
}

set_40()
{
	long	savea23[2];
	
	asm {
	
;
;  use the 8x10 character set: 40 line mode
;

	movem.l	A2-A3,savea23(A6)
	
	dc.w	LINEA_INIT

	move.l	04(A1),A1		; A1 -> 8x8 font header
	move.l	72(A1),V_OFF_AD(A0)	; v_off_ad <- 8x8  offset table addr
	lea	fnt_8x10(A4),A2
	move.l	A2,V_FNT_AD(A0)		; v_fnt_ad <- 8x10 font data addr

	move.w	#10,V_CEL_HT(A0)	; v_cel_ht <- 10   8x10 cell height
	move.w	#39,V_CEL_MY(A0)	; v_cel_my <- 39   maximum cell "Y"
	move.w	#800,V_CEL_WR(A0)	; v_cel_wr <- 800  offset to cell Y+1

	movem.l savea23,A2-A3
	}
}

set_20()
{
	long	savea23[2];

	asm {
		
;
;  use the 8x10 character set: 20 line mode
;

	movem.l	A2-A3,savea23(A6)
	
	dc.w	LINEA_INIT		; A0 -> line A variables

	move.l	04(A1),A1		; A1 -> 8x8 font header
	move.l	72(A1),V_OFF_AD(A0)	; v_off_ad <- 8x8  offset table addr
	lea	fnt_8x10(A4),A2
	move.l	A2,V_FNT_AD(A0)		; v_fnt_ad <- 8x10 font data addr

	move.w	#10,V_CEL_HT(A0)	; v_cel_ht <- 10   8x10 cell height
	move.w	#19,V_CEL_MY(A0)	; v_cel_my <- 19   maximum cell "Y"
	move.w	#1600,V_CEL_WR(A0)	; v_cel_wr <- 800  offset to cell Y+1
	
	movem.l	savea23,A2-A3
	}
}


restore()
{
	long savea23[2];
	
	asm {
	
;  return what was saved in parameter save zone	

	movem.l	A2-A3,savea23(A6)

	dc.w	LINEA_INIT		; a0 -> line A variables

	lea	parm_save(A4),A2	; a2 -> parameter save area
	move.l	(A2)+,V_OFF_AD(A0)
	move.l	(A2)+,V_FNT_AD(A0)
	move.w	(A2)+,V_CEL_HT(A0)
	move.w	(A2)+,V_CEL_MY(A0)
	move.w	(A2)+,V_CEL_WR(A0)
	
	movem.l	savea23(A6),A2-A3
	}          
}
GetCurStat(onoff)
int	onoff;
{
	long savea23[2];

	asm {
	movem.l	A2-A3,savea23(A6)

	dc.w	LINEA_INIT		; a0 -> line A variables
	move.w	V_DISAB(A0),onoff(A6)	; 0 = cursor visible
	moveq	#0,D0
	move.w	V_DISAB(A0),D0	
	movem.l	savea23(A6),A2-A3
	}          
}
#else
#if	ATARI & ST520 & LATTICE

/*
	These routines provide support for the ATARI 1040ST using
the LATTICE compiler using the virtual VT52 Emulator

*/

#define NROW    40                      /* Screen size.                 */
#define NCOL    80                      /* Edit if you want to.         */
#define	MARGIN	8			/* size of minimim margin and	*/
#define	SCRSIZ	64			/* scroll size for extended lines */
#define	NPAUSE	300			/* # times thru update to pause */
#define BIAS    0x20                    /* Origin 0 coordinate bias.    */
#define ESC     0x1B                    /* ESC character.               */
#define BEL     0x07                    /* ASCII bell character         */

/****	ST Internals definitions		*****/

/*	BIOS calls */

#define	BCONSTAT	1	/* return input device status */
#define	CONIN		2	/* read character from device */
#define	BCONOUT		3	/* write character to device */

/*	XBIOS calls */

#define	INITMOUS	0	/* initialize the mouse */
#define	GETREZ		4	/* get current resolution */
#define	SETSCREEN	5	/* set screen resolution */
#define	SETPALETTE	6	/* set the color pallette */
#define	SETCOLOR	7	/* set or read a color */
#define	CURSCONF	21	/* set cursor configuration */
#define	IKBDWS		25	/* intelligent keyboard send command */
#define	KBDVBASE	34	/* get keyboard table base */

/*	GEMDOS calls */

#define	EXEC		0x4b	/* Exec off a process */

#define	CON		2	/* CON: Keyboard and screen device */

/*	LINE A variables	*/

#define LINEA_INIT 0xA000
#define V_CEL_WR   -0x28
#define V_CEL_MY   -0x2a
#define V_CEL_HT   -0x2e
#define V_FNT_AD   -0x16
#define V_OFF_AD   -0x0a
#define V_DISAB    -346

/*	Palette color definitions	*/

#define	LOWPAL	"000700070770007707077777"
#define	MEDPAL	"000700007777"
#define	HIGHPAL	"000111"

/*	ST Global definitions		*/

/* keyboard vector table */
struct KVT {
	long midivec;		/* midi input */
	long vkbderr;		/* keyboard error */
	long vmiderr;		/* MIDI error */
	long statvec;		/* IKBD status */
	int (*mousevec)();	/* mouse vector */
	long clockvec;		/* clock vector */
	long joyvec;		/* joystict vector */
} *ktable;

int (*sysmint)();			/* system mouse interupt handler */

/* mouse parameter table */
struct Param {
	char topmode;
	char buttons;
	char xparam;
	char yparam;
	int xmax,ymax;
	int xinitial,yinitial;
} mparam;

int currez;			/* current screen resolution */
char resname[][8] = {		/* screen resolution names */
	"LOW", "MEDIUM", "HIGH", "DENSE"
};
short spalette[16];			/* original color palette settings */
short palette[16];			/* current palette settings */

extern  int     ttopen();               /* Forward references.          */
extern  int     ttgetc();
extern  int     ttputc();
extern  int     ttflush();
extern  int     ttclose();
extern  int     stmove();
extern  int     steeol();
extern  int     steeop();
extern  int     stbeep();
extern  int     stopen();
extern	int	stclose();
extern	int	stgetc();
extern	int	stputc();
extern	int	strev();
extern	int	strez();
extern	int	stkopen();
extern	int	stkclose();

#if	COLOR
extern	int	stfcol();
extern	int	stbcol();
#endif

/*
 * Dispatch table. All the
 * hard fields just point into the
 * terminal I/O code.
 */
TERM    term    = {
	NROW-1,
        NROW-1,
        NCOL,
        NCOL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
        &stopen,
        &stclose,
	&stkopen,
	&stkclose,
        &stgetc,
	&stputc,
        &ttflush,
        &stmove,
        &steeol,
        &steeop,
        &stbeep,
        &strev,
	&strez
#if	COLOR
	, &stfcol,
	&stbcol
#endif
};

stmove(row, col)
{
        stputc(ESC);
        stputc('Y');
        stputc(row+BIAS);
        stputc(col+BIAS);
}

steeol()
{
        stputc(ESC);
        stputc('K');
}

steeop()
{
#if	COLOR
	stfcol(gfcolor);
	stbcol(gbcolor);
#endif
        stputc(ESC);
        stputc('J');
}

strev(status)	/* set the reverse video state */

int status;	/* TRUE = reverse video, FALSE = normal video */

{
	if (currez > 1) {
		stputc(ESC);
		stputc(status ? 'p' : 'q');
	}
}

#if	COLOR
mapcol(clr)	/* medium rez color translation */

int clr;	/* emacs color number to translate */

{
	static int mctable[] = {0, 1, 2, 3, 2, 1, 2, 3};

	if (currez != 1)
		return(clr);
	else
		return(mctable[clr]);
}

stfcol(color)	/* set the forground color */

int color;	/* color to set forground to */

{
	if (currez < 2) {
		stputc(ESC);
		stputc('b');
		stputc(mapcol(color));
	}
}

stbcol(color)	/* set the background color */

int color;	/* color to set background to */

{
	if (currez < 2) {
		stputc(ESC);
		stputc('c');
		stputc(mapcol(color));
	}
}
#endif

stbeep()
{
        stputc(BEL);
        ttflush();
}

domouse()	/* mouse interupt handler */

{
	return((*sysmint)());
}

stkopen()	/* open the keyboard (and mouse) */

{
	/* grab the keyboard vector table */
	ktable = (struct KVT *)xbios(KBDVBASE);
	sysmint = ktable->mousevec;	/* save mouse vector */

	/* initialize the mouse */
	mparam.topmode = 0;
	mparam.buttons = 4;
	mparam.xparam = 8;
	mparam.yparam = 10;
	mparam.xmax = 79;
	mparam.ymax = 23;
	mparam.xinitial = 0;
	mparam.yinitial = 0;
	xbios(INITMOUS, 4, &mparam, &domouse);
}

stopen()	/* open the screen */

{
	int i;

        ttopen();
	eolexist = TRUE;

	/* switch to a steady cursor */
	xbios(CURSCONF, 3);

	/* save the current color palette */
	for (i=0; i<16; i++)
		spalette[i] = xbios(SETCOLOR, i, -1);

	/* and find the current resolution */
	currez = xbios(GETREZ);
	strcpy(sres, resname[currez]);

	/* set up the screen size and palette */
	switch (currez) {
		case 0:	term.t_mrow = 25 - 1;
			term.t_nrow = 25 - 1;
			term.t_ncol = 40 - 1;
			strcpy(palstr, LOWPAL);
			break;

		case 1: term.t_mrow = 25 - 1;
			term.t_nrow = 25 - 1;
			strcpy(palstr, MEDPAL);
			break;

		case 2: term.t_mrow = 40 - 1;
			term.t_nrow = 25 - 1;
			strcpy(palstr, HIGHPAL);
	}

	/* and set up the default palette */
	spal(palstr);

	stputc(ESC);	/* automatic overflow off */
	stputc('w');
	stputc(ESC);	/* turn cursor on */
	stputc('e');
}

stkclose()	/* close the keyboard (and mouse) */

{
	static char resetcmd[] = {0x80, 0x01};	/* keyboard reset command */

	/* restore the mouse interupt routines */
	xbios(INITMOUS, 2, &mparam, (long)sysmint);

	/* and reset the keyboard controller */
	xbios(IKBDWS, 1, &resetcmd[0]);
}

stclose()

{
	stputc(ESC);	/* auto overflow on */
	stputc('v');

	/* switch to a flashing cursor */
	xbios(CURSCONF, 2);

	/* restore the original palette settings */
	xbios(SETPALETTE, spalette);

	ttclose();
}

/* 	spal(pstr):	reset the current palette according to a
			"palette string" of the form

	000111222333444555666777

	which contains the octal values for the palette registers
*/

spal(pstr)

char *pstr;	/* palette string */

{
	int pal;	/* current palette position */
	int clr;	/* current color value */
	int i;

	for (pal = 0; pal < 16; pal++) {
		if (*pstr== 0)
			break;

		/* parse off a color */
		clr = 0;
		for (i = 0; i < 3; i++)
			if (*pstr)
				clr = clr * 16 + (*pstr++ - '0');
		palette[pal] = clr;
	};

	/* and now set it */
	xbios(SETPALETTE, palette);
}

stgetc()	/* get a char from the keyboard */

{
	int rval;		/* return value from BIOS call */
	static int funkey = 0;	/* held fuction key scan code */

	/* if there is a pending function key, return it */
	if (funkey) {
		rval = funkey;
		funkey = 0;
	} else {
		/* waiting... flash the cursor */
		xbios(CURSCONF, 2);

		/* get the character */
		rval = bios(CONIN, CON);
		if ((rval & 255) == 0) {
			funkey = (rval >> 16) & 255;
			rval = 0;
		}

		/* and switch to a steady cursor */
		xbios(CURSCONF, 3);
	}

	return(rval & 255);
}

stputc(c)	/* output char c to the screen */

char c;		/* character to print out */

{
	bios(BCONOUT, CON, c);
}

strez(newrez)	/* change screen resolution */

char *newrez;	/* requested resolution */

{
	int nrez;	/* requested new resolution */

	/* first, decode the resolution name */
	for (nrez = 0; nrez < 4; nrez++)
		if (strcmp(newrez, resname[nrez]) == 0)
			break;
	if (nrez == 4) {
		mlwrite("%%No such resolution");
		return(FALSE);
	}

	/* next, make sure this resolution is legal for this monitor */
	if ((currez < 2 && nrez > 1) || (currez > 1 && nrez < 2)) {
		mlwrite("%%Resolution illegal for this monitor");
		return(FALSE);
	}

	/* eliminate non-changes */
	if (currez == nrez)
		return(TRUE);

	/* finally, make the change */
	switch (nrez) {
		case 0:	/* low resolution - 16 colors */
			newwidth(TRUE, 40);
			strcpy(palstr, LOWPAL);
			xbios(SETSCREEN, -1, -1, 0);
			break;

		case 1:	/* medium resolution - 4 colors */
			newwidth(TRUE, 80);
			strcpy(palstr, MEDPAL);
			xbios(SETSCREEN, -1, -1, 1);
			break;

		case 2:	/* High resolution - 2 colors - 25 lines */
			newsize(TRUE, 25);
			strcpy(palstr, HIGHPAL);
			/* change char set back to normal */
			break;

		case 3:	/* Dense resolution - 2 colors - 40 lines */
			/* newsize(TRUE, 40); */
			strcpy(palstr, HIGHPAL);
			/*change char set size */
			break;
	}

	/* and set up the default palette */
	spal(palstr);
	currez = nrez;
	strcpy(sres, resname[currez]);

	stputc(ESC);	/* automatic overflow off */
	stputc('w');
	stputc(ESC);	/* turn cursor on */
	stputc('e');

	return(TRUE);
}

system(cmd)	/* call the system to execute a new program */

char *cmd;	/* command to execute */

{
	char *pptr;			/* pointer into program name */
	char pname[NSTRING];		/* name of program to execute */
	char tail[NSTRING];		/* command tail */

	/* scan off program name.... */
	pptr = pname;
	while (*cmd && (*cmd != ' ' && *cmd != '\t'))
		*pptr++ = *cmd++;
	*pptr = 0;

	/* create program name length/string */
	tail[0] = strlen(cmd);
	strcpy(&tail[1], cmd);

	/* go do it! */
	return(gemdos(		(int)EXEC,
				(int)0,
				(char *)pname,
				(char *)tail,
				(char *)NULL));
}

#if	TYPEAH
typahead()

{
	int rval;	/* return value from BIOS call */

	/* get the status of the console */
	rval = bios(BCONSTAT, CON);

	/* end return the results */
	if (rval == 0)
		return(FALSE);
	else
		return(TRUE);
}
#endif

#if	FLABEL
fnclabel(f, n)		/* label a function key */

int f,n;	/* default flag, numeric argument [unused] */

{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif
#else
sthello()
{
}
#endif
#endif
