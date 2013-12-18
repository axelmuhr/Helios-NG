/* term.c -- screen output code for Atari ST terminal
 *
 * author :  Sandra Loosemore (based on an earlier version by dec-rec!conroy)
 * date   :  24 Oct 1987
 * changes:  Marion Hakanson -- Jan 1988
 *
 * This code simulates scrolling regions by using the
 * insert line and delete line functions. Should display
 * handling be taught about this. Based on Rich's code
 * for the Heath H19.
 */

#include	"..\..\def.h"
 
#define	BEL	0x07			/* BEL character.		*/
#define	ESC	0x1B			/* ESC character.		*/
#define	LF	0x0A			/* Line feed.			*/
 
extern int ttrow;
extern int ttcol;
extern int tttop;
extern int ttbot;
extern int tthue;

int	tceeol	=	2;		/* Costs.			*/
int	tcinsl, tcdell;			/* Set in ttyio.c:ttopen()	*/

 
/* Move the cursor to the specified origin 0 row and column position.  Try
 *	to optimize out extra moves; redisplay may have left the cursor in 
 *	the right location on the screen last time.
 */

VOID ttmove(row, col)
{   if (ttrow!=row || ttcol!=col) {
	if (row > nrow)
	    row = nrow;
	if (col > ncol)
	    col = ncol;
	ttputc(ESC);
	ttputc('Y');
	ttputc(row+' ');
	ttputc(col+' ');
	ttrow = row;
	ttcol = col;
	}
    }

 
/* Erase to end of line. */

VOID tteeol()
{   ttputc(ESC);
    ttputc('K');
    }

 
/* Erase to end of page. */

VOID tteeop()
{   ttputc(ESC);
    ttputc('J');
    }

 
/* Make a noise.  */

VOID ttbeep()
{   ttputc(BEL);
    ttflush();
    }

 
/* Insert nchunk blank line(s) onto the screen, scrolling the last line on
 *	the screen off the bottom. This is done with a cluster of clever 
 *	insert and delete commands, because there are no scroll regions.
 */

VOID ttinsl(row, bot, nchunk)
{   register int	i;
    if (row == bot) {
	ttmove(row, 0);
	tteeol();
	return;
    }
    ttmove(1+bot-nchunk, 0);
    for (i=0; i<nchunk; i++) {
	ttputc(ESC);
	ttputc('M');
    }
    ttmove(row, 0);
    for (i=0; i<nchunk; i++) {
	ttputc(ESC);
	ttputc('L');
    }
    ttrow = row;
    ttcol = 0;
    }

 
/* Delete nchunk line(s) at "row", replacing the bottom line on the screen
 *	with a blank line. This is done with a crafty sequences of insert 
 *	and delete line sequences since there is no scroll region.  The
 * 	presence of the echo area makes a boundary condition go away.
 */

VOID ttdell(row, bot, nchunk)
{   register int	i;
    if (row == bot) {
	ttmove(row, 0);
	tteeol();
	return;
    }
    ttmove(row, 0);
    for (i=0; i<nchunk; i++) {
	ttputc(ESC);
	ttputc('M');
    }
    ttmove(1+bot-nchunk,0);
    for (i=0; i<nchunk; i++) {
	ttputc(ESC);
	ttputc('L');
    }
    ttrow = 1+bot-nchunk;
    ttcol = 0;
    }

 
/* Set display color.  Normal video is text color.  Reverse video is used
 * 	for the mode line.
 */

VOID ttcolor(color)
    register int color;
{   if (color != tthue) {
	if (color == CTEXT) {		/* Normal video.	*/
	    ttputc(ESC);
	    ttputc('q');
	    } 
	else if (color == CMODE) {	/* Reverse video.	*/
	    ttputc(ESC);
	    ttputc('p');
	    }
	tthue = color;
	}
    }
