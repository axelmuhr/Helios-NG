/*
 * Name:	MG 2a
 *		Amiga console device virtual terminal display
 * Last Edit:	29-Nov-87 mic@emx.cc.utexas.edu
 * Created:	19-Apr-86 mic@emx.cc.utexas.edu
 *
 * Drives the Amiga console device display.  The code is basically
 * like the termcap driver in that it uses the console device
 * scrolling region.  It also has some hacks to manage the console
 * device colors.  The latest hack is to inform the terminal I/O
 * driver when we intend to do an escape sequence; this allows the
 * terminal I/O driver to turn off the cursor without breaking up
 * the sequences, leading to a garbled screen.
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/tasks.h>
#include <exec/ports.h>
#include <exec/io.h>
#include <devices/console.h>
#include <libraries/dos.h>
#include <graphics/clip.h>
#include <graphics/view.h>
#include <graphics/rastport.h>
#include <graphics/layers.h>
#include <graphics/text.h>
#include <graphics/gfxbase.h>
#ifndef	MANX
#include <intuition/intuitionbase.h>
#endif
#include <intuition/intuition.h>

#undef	TRUE
#undef	FALSE
#include	"def.h"

#define	BEL	0x07			/* BEL character.		*/
#define	ESC	0x1B			/* ESC character.		*/
#define	LF	0x0A			/* Linefeed character		*/
#define	CSI	0x9B			/* Command Sequence Introducer	*/

extern	int	ttrow;
extern	int	ttcol;
extern	int	tttop;
extern	int	ttbot;
extern	int	tthue;

int	tceeol	=	3;		/* Costs, ANSI display.		*/
int	tcinsl	= 	17;
int	tcdell	=	16;


#ifdef	CHANGE_COLOR
short	mode_rendition = MODE_RENDITION,	/* set standard colors */
	text_rendition = TEXT_RENDITION,
	text_fg = TEXT_FG + 30,
	text_bg = TEXT_BG + 40,
	mode_fg = MODE_FG + 30,
	mode_bg = MODE_BG + 40;
#else				/* colors are hard-coded		*/
#define mode_rendition MODE_RENDITION
#define	text_rendition TEXT_RENDITION
#define text_fg (TEXT_FG + 30)
#define text_bg (TEXT_BG + 40)
#define mode_fg (MODE_FG + 30)
#define mode_bg (MODE_BG + 40)
#endif

#ifdef	LATTICE
VOID	asciiparm(int) ;
#else
VOID	asciiparm() ;
#endif
VOID	ttnowindow() ;
VOID	ttwindow() ;

/*
 * Initialize the terminal when the editor
 * Initialize the virtual terminal.
 * Set the console device's top edge below
 * the front-to-back gadgets, to avoid
 * garbage when scrolling.
 */
VOID
ttinit()
{
	ttputc(CSI);
	asciiparm(TOP_OFFSET);
	ttputc('y');
}

/*
 * Clean up the terminal, in anticipation of
 * a return to the command interpreter. This
 * is a no-op on the Amiga, since the window
 * is deleted anyway.
 */
VOID
tttidy()
{
}

/*
 * Move the cursor to the specified origin 0 row and column position. Try to
 * optimize out extra moves; redisplay may have left the cursor in the right
 * location last time!
 */
VOID
ttmove(row, col)
{
	if (ttrow!=row || ttcol!=col) {
		ttnflush(8);		/* flush if buffer too full 	*/
		ttputc(CSI);
		asciiparm(row+1);
		ttputc(';');
		asciiparm(col+1);
		ttputc('H');
		ttrow = row;
		ttcol = col;
	}
}

/*
 * Erase to end of line.
 */
VOID
tteeol()
{
	ttnflush(2);		/* flush if not enough room to fit in buffer */
	ttputc(CSI);
	ttputc('K');
}

/*
 * Erase to end of page.
 */
VOID
tteeop()
{
	ttnflush(12);		/* flush (but only if not enough room for seq */
	ttputc(CSI);
	asciiparm((tthue == CTEXT) ? text_rendition : mode_rendition);
	ttputc(';');
	asciiparm(text_fg);
	ttputc(';');
	asciiparm(text_bg);
	ttputc('m');
	ttputc(CSI);	/* clear to end of display */
	ttputc('J');
}

/*
 * Make a noise.
 */
VOID
ttbeep()
{
	ttputc(BEL);
	ttflush();
}

/*
 * Convert a number to decimal
 * ascii, and write it out. Used to
 * deal with numeric arguments.
 */
VOID
asciiparm(n)
register int	n;
{
	if (n > 9)
		asciiparm(n/10);
	ttputc((n%10) + '0');
}

/*
 * Insert a block of blank lines onto the
 * screen, using a scrolling region that starts at row
 * "row" and extends down to row "bot".  Deal with the one
 * line case, which is a little bit special, with special
 * case code.
 */
VOID
ttinsl(row, bot, nchunk)
{
	if (row == bot) {			/* Funny case.		*/
		if (nchunk != 1)
			panic("ttinsl: nchunk != 1");
		ttmove(row, 0);
		tteeol();
		return;
	} 
	ttmove(1+bot-nchunk, 0);
	if (nchunk > 0) {
		ttwindow(row, bot);
		ttnflush(4);		/* don't break the sequence  */
		ttputc(CSI);
  		asciiparm(nchunk);
		ttputc('T');		/* Scroll scrolling region down	*/
		ttnowindow();
	}
}

/*
 * Delete a block of lines, with the uppermost
 * line at row "row", in a screen slice that extends to
 * row "bot". The "nchunk" is the number of lines that have
 * to be deleted.  It's really easy with the console
 * device scrolling region.
 */
VOID
ttdell(row, bot, nchunk)
{
	if (row == bot) {		/* One line special case	*/
		ttmove(row, 0);
		tteeol();
		return;
	}
	if (nchunk > 0) {
		ttwindow(row, bot);
		ttnflush(4);		/* don't break esc. sequence	*/
		ttputc(CSI);
  		asciiparm(nchunk);
		ttputc('S');		/* Scroll scrolling region up	*/
		ttnowindow();
	}
	ttrow = HUGE;
	ttcol = HUGE;
	ttmove(bot-nchunk,0);
}

/*
 * This routine sets the scrolling window on the display to go from line
 * "top" to line "bot" (origin 0, inclusive). The caller checks for the
 * pathalogical 1 line scroll window that doesn't work right on all
 * systems, and avoids it. The "ttrow" and "ttcol" variables are set
 * to a crazy value to ensure that ttmove() actually does something.
 */

extern	struct Window	*EmW;			/* The window MG uses */

VOID
ttwindow(top,bot)
{
	if (tttop != top || ttbot != bot) {
		ttnflush(10);			/* Flush if necessary	*/
		ttputc(CSI);			/* Home cursor		*/
		ttputc('H');

		ttputc(CSI);			/* Set top offset	*/
		asciiparm(TOP_OFFSET + top * FontHeight(EmW));
		ttputc('y');

		ttputc(CSI);
		asciiparm(bot - top + 1);	/* Set page length	*/
		ttputc('t');

		ttrow = HUGE;			/* Force cursor reset	*/
		ttcol = HUGE;
		tttop = top;			/* Save region state	*/
		ttbot = bot;
	}
}

/*
 * Switch to full screen scrolling
 */
VOID
ttnowindow()
{
	ttnflush(10);			/* Flush if necessary		*/
	ttputc(CSI);			/* Home cursor			*/
	ttputc('H');

	ttputc(CSI);			/* Set top offset to normal	*/
	asciiparm(TOP_OFFSET);
	ttputc('y');

	ttputc(CSI);			/* Set page length to nrow	*/
	asciiparm(nrow);
	ttputc('t');

	ttrow = HUGE;			/* Make cursor unknown.		*/
	ttcol = HUGE;
	tttop = HUGE;
	ttbot = HUGE;
}

#ifdef	CHANGE_COLOR
/*
 * Set the rendition of the mode line by
 * selecting colors from the following:
 *	0 -- plain text
 *	1 -- bold-face
 *	3 -- italic
 *	4 -- underscore
 *	7 -- inverse video
 * Certain of these selections may be less than
 * appealing :-)
 */

ttmode(f, n)
{
	register int	s;
	char		buf[2];

	if (!(f & FFARG)) {
		if ((s = ereply("Set mode line rendition (0-7): ",
				buf, sizeof(buf))) != TRUE)
			return (s);
		n = atoi(buf);
	}
	if (n < 0 || n > 7)
		return (FALSE);

	mode_rendition = n;		/* store the color	*/
	sgarbf = TRUE;
	return (TRUE);
}

/*
 * Set the rendition of the text area.
 * Most of these selections will be
 * less than appealing :-]
 */

tttext(f, n)
{
	register int	s;
	char		buf[2];

	if (!(f & FFARG)) {
		if ((s = ereply("Set text rendition (0-7): ",
				buf, sizeof(buf))) != TRUE)
			return (s);
		n = atoi(buf);
	}
	if (n < 0 || n > 7)
		return (FALSE);

	text_rendition = n;		/* store the color	*/
	sgarbf = TRUE;
	return (TRUE);
}

/*
 * Set foreground color for entire window
 * to a value between 30 and 37, which
 * corresponds to the arguments 0-7.
 * This requires a total refresh, which
 * sets up the screen.
 */

textforeground(f, n)
{
	register int	s;
	char		buf[2];

	if (!(f & FFARG)) {
		if ((s = ereply("Text foreground color (0-7): ",
				buf, sizeof(buf))) != TRUE)
			return (s);
		n = atoi(buf);
	}
	if (n < 0 || n > 7)
		return (FALSE);

	text_fg = n + 30;
	sgarbf = TRUE;
	return (TRUE);
}

/*
 * Set background color for entire window
 * to a value between 40 and 47 inclusive.
 */

textbackground(f, n)
{
	register int	s;
	char		buf[2];

	if (!(f & FFARG)) {
		if ((s = ereply("Text background color (0-7): ",
				buf, sizeof(buf))) != TRUE)
			return (s);
		n = atoi(buf);
	}
	if (n < 0 || n > 7)
		return (FALSE);

	text_bg = n + 40;
	sgarbf = TRUE;
	return (TRUE);
}

/*
 * Set foreground color for entire the mode line
 */

modeforeground(f, n)
{
	register int	s;
	char		buf[2];

	if (!(f & FFARG)) {
		if ((s = ereply("Mode line foreground color (0-7): ",
				buf, sizeof(buf))) != TRUE)
			return (s);
		n = atoi(buf);
	}
	if (n < 0 || n > 7)
		return (FALSE);

	mode_fg = n + 30;
	sgarbf = TRUE;
	return (TRUE);
}

/*
 * Set background color for the mode line
 */

modebackground(f, n)
{
	register int	s;
	char		buf[2];

	if (!(f & FFARG)) {
		if ((s = ereply("Mode line background color (0-7): ",
				buf, sizeof(buf))) != TRUE)
			return (s);
		n = atoi(buf);
	}
	if (n < 0 || n > 7)
		return (FALSE);

	mode_bg = n + 40;
	sgarbf = TRUE;
	return (TRUE);
}
#endif

/*
 * Set the current writing color to the
 * specified color. Watch for color changes that are
 * not going to do anything (the color is already right)
 * and don't send anything to the display.
 */

VOID
ttcolor(color)
register int	color;
{
	if (color != tthue) {
		ttnflush(12);			/* Flush if necessary	*/
		if (color == CTEXT) {		/* Normal video.	*/
			ttputc(CSI);		/* Reset to 0		*/
			ttputc('m');
			ttputc(CSI);		/* Set text style	*/
			asciiparm(text_rendition);
			ttputc(';');
			asciiparm(text_fg);
			ttputc(';');
			asciiparm(text_bg);
			ttputc('m');
		} else if (color == CMODE) {	/* Standout mode	*/
			ttputc(CSI);		/* Reset to 0		*/
			ttputc('m');
			ttputc(CSI);		/* Set standout mode	*/
			asciiparm(mode_rendition);
			ttputc(';');
			asciiparm(mode_fg);	/* Use mode line colors	*/
			ttputc(';');
			asciiparm(mode_bg);
			ttputc('m');
		}
		tthue = color;			/* Save the color.	*/
	}
}

/*
 * This routine is called by the "refresh the screen" command to try and resize
 * the display. The new size, which must be deadstopped to not exceed the NROW
 * and NCOL limits, is stored back into "nrow" and "ncol". Display can always
 * deal with a screen NROW by NCOL. Look in "window.c" to see how the caller
 * deals with a change. On the Amiga, we make the Intuition terminal driver
 * do all the work.
 */

VOID
ttresize()
{
 	setttysize();
}
