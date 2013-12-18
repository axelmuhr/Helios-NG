/*
 * The routines in this file provide support for the TI-PC and other
 * compatible terminals. It goes directly to the graphics RAM to do
 * screen output. It compiles into nothing if not a TI-PC driver
 */

#define termdef 1                       /* don't define "term" external */

#include        <stdio.h>
#include        "estruct.h"
#include	"etype.h"
#include        "edef.h"
#include	"elang.h"

#if     TIPC

#define NROW    25                      /* Screen size.                 */
#define NCOL    80                      /* Edit if you want to.         */
#define MARGIN  8                       /* size of minimim margin and   */
#define SCRSIZ  64                      /* scroll size for extended lines */
#define NPAUSE  200                     /* # times thru update to pause */
#define BEL     0x07                    /* BEL character.               */
#define ESC     0x1B                    /* ESC character.               */
#define SPACE   32                      /* space character              */
#define SCADD   0xDE000L                /* address of screen RAM        */

#define CHAR_ENABLE     0x08            /* TI attribute to show char    */
#define TI_REVERSE      0x10            /* TI attribute to reverse char */
#define BLACK   0+CHAR_ENABLE           /* TI attribute for Black       */
#define BLUE    1+CHAR_ENABLE           /* TI attribute for Blue        */
#define RED     2+CHAR_ENABLE           /* TI attribute for Red         */
#define MAGENTA 3+CHAR_ENABLE           /* TI attribute for Magenta     */
#define GREEN   4+CHAR_ENABLE           /* TI attribute for Green       */
#define CYAN    5+CHAR_ENABLE           /* TI attribute for Cyan        */
#define YELLOW  6+CHAR_ENABLE           /* TI attribute for Yellow      */
#define WHITE   7+CHAR_ENABLE           /* TI attribute for White       */


extern PASCAL NEAR ttopen();               /* Forward references.          */
extern PASCAL NEAR ttgetc();
extern PASCAL NEAR ttputc();
extern PASCAL NEAR ttflush();
extern PASCAL NEAR ttclose();
extern PASCAL NEAR timove();
extern PASCAL NEAR tieeol();
extern PASCAL NEAR tieeop();
extern PASCAL NEAR tibeep();
extern PASCAL NEAR tiopen();
extern PASCAL NEAR tikopen();
extern PASCAL NEAR tirev();
extern PASCAL NEAR ticres();
extern PASCAL NEAR ticlose();
extern PASCAL NEAR tikclose();
extern PASCAL NEAR tiputc();

#if     COLOR
extern PASCAL NEAR tifcol();
extern PASCAL NEAR tibcol();

int     cfcolor = -1;           /* current forground color */
int     cbcolor = -1;           /* current background color */
int     ctrans[] =              /* ansi to ti color translation table */
        {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE};
#endif

/*
 * Standard terminal interface dispatch table. Most of the fields point into
 * "termio" code.
 */
TERM    term    = {
	NROW-1,
        NROW-1,
        NCOL,
        NCOL,
        MARGIN,
        SCRSIZ,
        NPAUSE,
        tiopen,
        ticlose,
        tikopen,
        tikclose,
        ttgetc,
        tiputc,
        ttflush,
        timove,
        tieeol,
        tieeop,
        tibeep,
        tirev,
        ticres
#if     COLOR
        , tifcol,
        tibcol
#endif
};

extern union REGS rg;

#if     COLOR
PASCAL NEAR setatt(attr)
int attr;
{
        rg.h.ah = 0x16;         /* set the forground character attribute */
        rg.h.bl = attr;
        int86(0x49, &rg, &rg);
}

PASCAL NEAR tifcol(color)           /* set the current output color */

int color;      /* color to set */

{
        cfcolor = ctrans[color];
        setatt (cfcolor);
}

PASCAL NEAR tibcol(color)           /* set the current background color */

int color;      /* color to set */

{
        cbcolor = ctrans[color];
}
#endif

PASCAL NEAR timove(row, col)
{
        rg.h.ah = 2;            /* set cursor position function code */
        rg.h.dh = col;
        rg.h.dl = row;
        int86(0x49, &rg, &rg);
}

PASCAL NEAR tieeol()        /* erase to the end of the line */

{
        int ccol;       /* current column cursor lives */
        int crow;       /*         row  */

        /* find the current cursor position */
        rg.h.ah = 3;            /* read cursor position function code */
        int86(0x49, &rg, &rg);
        ccol = rg.h.dh;         /* record current column */
        crow = rg.h.dl;         /* and row */

        rg.h.ah = 0x09;         /* Write character at cursor position */
        rg.h.al = ' ';          /* Space */
        rg.h.bl = cfcolor;
        rg.x.cx = NCOL-ccol;    /* Number of characters to write */
        int86(0x49, &rg, &rg);

}

PASCAL NEAR tiputc(ch)      /* put a character at the current position in the
	                   current colors */

int ch;

{
        rg.h.ah = 0x0E;         /* write char to screen with current attrs */
        rg.h.al = ch;
        int86(0x49, &rg, &rg);
}

PASCAL NEAR tieeop()                        /* Actually a clear screen */
{

        rg.h.ah = 0x13;         /* Clear Text Screen and Home Cursor */
        int86(0x49, &rg, &rg);
}

PASCAL NEAR tirev(state)            /* change reverse video state */

int state;      /* TRUE = reverse, FALSE = normal */

{
        setatt(state ? cbcolor : cfcolor);
}

PASCAL NEAR ticres()	/* change screen resolution */

{
	return(TRUE);
}

PASCAL NEAR spal()		/* change palette string */

{
	/*	Does nothing here	*/
}

PASCAL NEAR tibeep()
{
        bdos(6, BEL, 0);
}

PASCAL NEAR tiopen()
{
	strcpy(sres, "NORMAL");
        revexist = TRUE;
        ttopen();
}

PASCAL NEAR tikopen()

{
}

PASCAL NEAR ticlose()

{
#if     COLOR
        tifcol(7);
        tibcol(0);
#endif
        ttclose();
}

PASCAL NEAR tikclose()

{
}
#else
tihello()
{
}
#endif

