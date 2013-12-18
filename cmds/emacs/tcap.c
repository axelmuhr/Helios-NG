/*	tcap:	Unix V5, V7 and BS4.2 Termcap video driver
		for MicroEMACS

 $Header: /usr/perihelion/Helios/cmds/emacs/RCS/tcap.c,v 1.1 90/08/23 15:16:50 jon Exp $

*/

#define	termdef	1			/* don't define "term" external */

#include <stdio.h>
#include	"estruct.h"
#include        "edef.h"

#if TERMCAP

#define	MARGIN	8
#define	SCRSIZ	48			/* scroll for long lines */
#define	NPAUSE	10			/* # times thru update to pause */
#define BEL     0x07
#define ESC     0x1B

extern int      ttopen();
extern int      ttgetc();
extern int      ttputc();
extern int	tgetnum();
extern int      ttflush();
extern int      ttclose();
extern int	tcapkopen();
extern int	tcapkclose();
extern int      tcapmove();
extern int      tcapeeol();
extern int      tcapeeop();
extern int      tcapbeep();
extern int	tcaprev();
extern int	tcapcres();
extern int      tcapopen();
extern int      tput();
extern char     *tgoto();
#if	COLOR
extern	int	tcapfcol();
extern	int	tcapbcol();
#endif

#define TCAPSLEN 315
char tcapbuf[TCAPSLEN];
char *UP, PC, *CM, *CE, *CL, *SO, *SE;

TERM term = {
	NULL,	/* these four values are set dynamically at open time */
	NULL,
	NULL,
	NULL,
	MARGIN,
	SCRSIZ,
	NPAUSE,
        tcapopen,
        ttclose,
        tcapkopen,
        tcapkclose,
        ttgetc,
        ttputc,
        ttflush,
        tcapmove,
        tcapeeol,
        tcapeeop,
        tcapbeep,
        tcaprev,
        tcapcres
#if	COLOR
	, tcapfcol,
	tcapbcol
#endif
};

tcapopen()

{
        char *getenv();
        char *t, *p, *tgetstr();
        char tcbuf[1024];
        char *tv_stype;
        char err_str[72];

        if ((tv_stype = getenv("TERM")) == NULL)
        {
                puts("Environment variable TERM not defined!");
                exit(1);
        }

        if ((tgetent(tcbuf, tv_stype)) != 1)
        {
                sprintf(err_str, "Unknown terminal type %s!", tv_stype);
                puts(err_str);
                exit(1);
        }

 
       if ((term.t_nrow=(short)tgetnum("li")-1) == -1){
               puts("termcap entry incomplete (lines)");
               exit(1);
       }
	term.t_mrow =  term.t_nrow;

       if ((term.t_ncol=(short)tgetnum("co")) == -1){
               puts("Termcap entry incomplete (columns)");
               exit(1);
       }
	term.t_mcol = term.t_ncol;

        p = tcapbuf;
        t = tgetstr("pc", &p);
        if(t)
                PC = *t;

        CL = tgetstr("cl", &p);
        CM = tgetstr("cm", &p);
        CE = tgetstr("ce", &p);
        UP = tgetstr("up", &p);
	SE = tgetstr("se", &p);
	SO = tgetstr("so", &p);
	if (SO != NULL)
		revexist = TRUE;
#if	FIXSG & REVSTA
	SG = tgetnum("sg");	/* standout glitch 	*/
#endif

        if(CL == NULL || CM == NULL || UP == NULL)
        {
                puts("Incomplete termcap entry\n");
                exit(1);
        }

	if (CE == NULL)		/* will we be able to use clear to EOL? */
		eolexist = FALSE;
		
        if (p >= &tcapbuf[TCAPSLEN])
        {
                puts("Terminal description too big!\n");
                exit(1);
        }
        ttopen();
}

tcapkopen()

{
	strcpy(sres, "NORMAL");
}

tcapkclose()

{
}

tcapmove(row, col)
register int row, col;
{
        putpad(tgoto(CM, col, row));
}

tcapeeol()
{
        putpad(CE);
}

tcapeeop()
{
        putpad(CL);
}

tcaprev(state)		/* change reverse video status */

int state;		/* FALSE = normal video, TRUE = reverse video */

{
	static int oldstate = TRUE + TRUE;
	if (state == oldstate) return;
	oldstate = state;
	if (state) {
		if (SO != NULL)
			putpad(SO);
	} else
		if (SE != NULL)
			putpad(SE);
}

tcapcres()	/* change screen resolution */

{
	return(TRUE);
}

spal(dummy)	/* change palette string */

{
	/*	Does nothing here	*/
}

#if	COLOR
tcapfcol()	/* no colors here, ignore this */
{
}

tcapbcol()	/* no colors here, ignore this */
{
}
#endif

tcapbeep()
{
	ttputc(BEL);
}

putpad(str)
char    *str;
{
	tputs(str, 1, ttputc);
}

putnpad(str, n)
char    *str;
{
	tputs(str, n, ttputc);
}


#if	FLABEL
fnclabel(f, n)		/* label a function key */

int f,n;	/* default flag, numeric argument [unused] */

{
	/* on machines with no function keys...don't bother */
	return(TRUE);
}
#endif
#else

hello()
{
}

#endif TERMCAP
