/*************************************************************************
**									**
**	       C O N S O L E  &  W I N D O W   S E R V E R		**
**	       -------------------------------------------		**
**									**
**		    Copyright (C) 1989, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** terminal.c								**
**									**
**	- termcap-related Terminal output routines			**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	20/11/89 : C. Fleischer					**
** Changed   :	18/01/90 : C. Fleischer	 terminal reinitialisation	**
** Changed   :	12/09/90 : G. Jodlauk	 for use with TTY		**
*************************************************************************/

#define __in_term	1		/* flag that we are in this module */

#include "tty.h"
extern int device_write(char *buf, int size, word timeout);

/*************************************************************************
 * FLUSH THE TERMINAL BUFFER
 *
 * - Send a buffer to the terminal line.
 * - Reset the buffer pointer.
 *
 * Parameter  :	- nothing -
 * Return     :	- nothing -
 *
 ************************************************************************/

void
TermFlush (void)
{
#ifdef	DEBUG
    static char	*fname = "TermFlush";
#endif
    char	*output = term_output;
    int		count	= term_ocount;
    int 	actual;

#ifdef	DEBUG
    Debug (TERM_ALL) ("%s %d chars.", fname, count);
#endif

    while (count > 0)			/* flush complete terminal buf	*/
    {
	actual = device_write (output, count, IOCTimeout);

#ifdef	DEBUG
	Debug (TERM_ALL) ("%s %d chars written.", fname, actual);
#endif

	if (actual > 0)			/* something written :		*/
	{
	    output += actual;		/* adjust buffer pointer	*/
	    count -= actual;		/* and buffer length		*/
	}
    }
    term_ocount = 0;			/* reset the Ansi counter	*/
#ifdef	DEBUG
    Debug (TERM_ALL) ("%s ready.", fname);
#endif
}

/*************************************************************************
 * WRITE A CHARACTER TO THE TERMINAL BUFFER
 *
 * - Copy the character to the ansi_buffer,
 *   flush the buffer if full.
 *
 * Parameter  :	ch	= character to be written
 * Return     :	- nothing -
 *
 ************************************************************************/

static void
TermPutChar (char ch)
{
#ifdef	DEBUG
    static char	*fname	= "TermPutChar";

    Debug (TERM_ALL) ((ch >= ' ' && ch < 127) ? "%s '%c'" : "%s <%02x>", fname, ch);
#endif

    term_output[term_ocount++] = ch;
    if (term_ocount >= TermBufSize)
	TermFlush ();
}


/*************************************************************************
 * WRITE A STRING TO THE TERMINAL BUFFER
 *
 * - Copy the string to the ansi_buffer,
 *   flush the buffer if full.
 *
 * Parameter  :	str	= data to be written
 * Return     :	- nothing -
 *
 ************************************************************************/

static void
TermPutString (char *str)
{
#ifdef	DEBUG
    Debug (TERM_ALL) ("TermPutString '%L'", str);
#endif
    while (*str)
    {
	term_output[term_ocount++] = *str++;
	if (term_ocount >= TermBufSize)
	    TermFlush ();
    }
}


/*************************************************************************
 * WRITE SOME DATA TO THE TERMINAL BUFFER
 *
 * - Copy the specified amount of data to the ansi_buffer,
 *   flush the buffer if full.
 *
 * Parameter  :	data	= data to be written
 *		count	= number of data bytes
 * Return     :	- nothing -
 *
 ************************************************************************/

static void
TermPutData (char *data, int count)
{
#ifdef	DEBUG
    Debug (TERM_ALL) ("TermPutData %d chars", count);
#endif
    while (count > 0)
    {
	term_output[term_ocount++] = *data++;
	count--;
	if (term_ocount >= TermBufSize)
	    TermFlush ();
    }
}


/*************************************************************************
 * TERMINAL CONTROL ROUTINES
 *
 * - These routines do the terminal-specific action on the screen.
 * - If some termcap facilities are missing, the routines will try to
 *   emulate them.
 *
 ************************************************************************/

/*************************************************************************
 * POSITION CURSOR
 ************************************************************************/

void
TermGoto (int x, int y)
{
#ifdef	DEBUG
    Debug (TERM) ("\t\t\tTermGoto %d %d", x, y);
#endif

    TermPutString (tgoto (tcap_goto, x, y));
}

/*************************************************************************
 * CLEAR SCREEN AND HOME CURSOR
 ************************************************************************/

void
TermClearScreen (void)
{
#ifdef	DEBUG
    Debug (TERM) ("\t\t\tTermClearScreen");
#endif

    if (*tcap_clrs)			/* clear screen...		*/
	TermPutString (tcap_clrs);
    elif (*tcap_ceos)			/* clear to end of screen...	*/
    {
	TermGoto (0, 0);
	TermPutString (tcap_ceos);
    }
    else				/* else use linefeed		*/
    {
	int	i;
	TermGoto (0, term_maxrow);
	for (i = 0; i <= term_rows + 1; i++)
	    TermPutChar ('\n');
	TermGoto (0, 0);
    }
/*
-- crf: 28/10/92 - illegal memory access
-- When TermClearScreen() is called from RedrawScreen(), Cur_Screen is NULL
-- (set in SetCurrentWindow()). As far as I can tell, it is not actually
-- necessary to modify the flags in this case because the ANSI_dirty flag is
-- in fact cleared in RedrawScreen() before this routine is called.
-- Consequently, it should be O.K. simply to test if Cur_Screen is NULL prior
-- to clearing the flag.
*/
    if (Cur_Screen)
	    Cur_Screen->flags &= ~ANSI_dirty;
}

/*************************************************************************
 * REDRAW REST OF A SINGLE LINE (WITH DIRTY HANDLING)
 * THE CURSOR IS EXPECTED TO BE AT THE SPECIFIED POSITION
 * !!! CHANGES CURSOR POSITION !!!
 ************************************************************************/

void
TermRedrawLine (Screen *s, int cur_x, int cur_y)
{
    char	*line	= s->map[cur_y];
    
#ifdef	DEBUG
    Debug (TERM) ("\t\t\tTermRedrawLine at %d %d", cur_x, cur_y);
#endif
    if (term_wraps && cur_y == term_maxrow)
    {
	char	*ins;

    	s->flags &= ~ANSI_dirty;	/* assume it cleaned	*/
    	
	if (*tcap_insc)			/* is an 'insert char' sequence	*/
	    ins = tcap_insc;		/* defined ? 			*/
	elif (*tcap_iscn)
	    ins = tgoto (tcap_iscn, 0, 1);
	else
	    ins = NULL;

	if (ins)			/* Sequence exists, use it	*/
	{
	    				/* update except last 2 cols	*/
	    TermGoto (cur_x, cur_y);
	    TermPutData (&line[cur_x], term_maxcol - cur_x - 2);
	    TermPutChar (line[term_maxcol]);	/* add last char	*/
	    TermGoto (term_maxcol - 1, cur_y);	/* step back one char	*/
	    TermPutString (ins);	/* insert 1 space		*/
					/* and write prelast char	*/
	    TermPutChar (line[term_maxcol - 1]);
	    return;
	}

	if (*tcap_insl)			/* is an 'insert char' sequence	*/
	    ins = tcap_insl;		/* defined ? 			*/
	elif (*tcap_isln)
	    ins = tgoto (tcap_isln, 0, 1);
	else
	    ins = NULL;

	if (ins)			/* Sequence exists, use it	*/
	{
	    TermGoto (0, cur_y - 1);	/* go one line back		*/
	    TermPutData (line, term_cols);	/* write last line	*/
	    TermPutString (ins);	/* insert a new line		*/
	    				/* and write prelast line	*/
	    TermPutData (s->map[cur_y - 1], term_cols);
	    return;
	}
	TermGoto (cur_x, cur_y);
					/* no insert char available:	*/
					/* update all chars except last	*/
	TermPutData (&line[cur_x], term_maxcol - cur_x);
	s->flags |= ANSI_dirty;	/* mark as dirty	*/
    }
    else
	TermPutData (&line[cur_x], term_cols - cur_x);
}

/*************************************************************************
 * CLEAR FROM CURRENT CURSOR POSITION TO END OF LINE
 ************************************************************************/

void
TermClearEol (void)
{
#ifdef	DEBUG
    Debug (TERM) ("\t\t\tTermClearEol");
#endif

    if (*tcap_ceol)			/* clear to end of line...	*/
	TermPutString (tcap_ceol);
    else
    {
    	Screen	*s	= Cur_Screen;
	int	cur_x	= s->Cur_x;
	int	cur_y	= s->Cur_y;

					/* else redraw rest of line	*/
	TermRedrawLine (s, cur_x, cur_y);
	TermGoto (cur_x, cur_y);	/* and reposition cursor.	*/

    }
}

/*************************************************************************
 * CLEAR FROM CURRENT CURSOR POSITION TO END OF SCREEN
 ************************************************************************/

void
TermClearEos (void)
{
#ifdef	DEBUG
    Debug (TERM) ("\t\t\tTermClearEos");
#endif

    if (*tcap_ceos)			/* clear to end of screen...	*/
	TermPutString (tcap_ceos);
    else				/* else emulate with		*/
    {					/* clear to end of line		*/
#if 0
	int	save_x, save_y;
	int	*cur_x	= &Cur_Screen->Cur_x;
	int	*cur_y	= &Cur_Screen->Cur_y;
	int	i;
	
	save_x = *cur_x;
	save_y = *cur_y;
	TermClearEol ();		/* clear rest of current line	*/
	*cur_x = 0;
	for (i = Cur_Screen->Cur_y + 1; i < term_rows; i++)
	{
	    *cur_y = i;
	    TermClearEol ();		/* clear all other lines	*/
	}
	*cur_x = save_x;
	*cur_y = save_y;
	TermGoto (save_x, save_y);
#else
	Screen	*s	= Cur_Screen;
	int	cur_x	= s->Cur_x;
	int	cur_y	= s->Cur_y;
	int	i;
	
    	if (*tcap_ceol)			/* clear to end of line...	*/
	    TermPutString (tcap_ceol);
        else				/* else redraw rest of line	*/
	    TermRedrawLine (s, cur_x, cur_y);
	for (i = cur_y + 1; i < term_rows; i++)
	{				/* clear all other lines	*/
	    TermGoto (0, i);
    	    if (*tcap_ceol)		/* clear whole line		*/
	    	TermPutString (tcap_ceol);
            else			/* else redraw rest of line	*/
	    	TermRedrawLine (s, 0, i);
	}
	TermGoto (cur_x, cur_y);
#endif
    }
}

/*************************************************************************
 * REDRAW A PART OF THE SCREEN
 * THE CURSOR IS EXPECTED TO BE AT THE START OF start_y LINE
 ************************************************************************/

void
TermRedrawScreen (int start_y, int end_y)
{
    Screen	*s	= Cur_Screen;
    char	*line;
    int 	i, j;

#ifdef	DEBUG
    Debug (TERM) ("\t\t\tTermRedrawScreen  from %d to %d", start_y, end_y);
#endif

#if 0
    if (*tcap_ceol)			/* use clear to end of line..	*/
    {
	for (i = start_y; i <= end_y; i++)	/* for each line	*/
	{
	    line = s->map[i];			/* set map pointer	*/
	    for (j = term_maxcol; j >= 0; j--)
		if (line[j] != ' ')	/* scan for trailing blanks	*/
		    break;
	    if (j < 0)			/* empty line : clear it	*/
	    {
		TermPutString (tcap_ceol);
		if (i < term_maxrow)	/* if not last, go to next line	*/
		    TermPutString ("\r");
	    }
	    elif (j == term_maxcol && term_wraps)	/* special case	*/
	    {
		if (i == term_maxrow)	/* more special case: last line	*/
		{
		    TermPutData (line, term_cols - 1);	/* write text	*/
		    s->flags |= ANSI_dirty;		/* without last	*/
		}
		else
		{			/* write full line,		*/
		    TermPutData (line, term_cols);	/* reposition	*/
		    TermGoto (0, i + 1);		/* the cursor.	*/
		}
	    }
	    else			/* nothing serious : write the	*/
	    {				/* non_blank part of the line	*/
		TermPutData (line, j + 1);	/* and clear the rest.	*/
		TermPutString (tcap_ceol);
		if (i < term_maxrow)	/* if not last, go to next line	*/
		    TermPutString ("\r");
	    }
	}
    }
    else				/* simulation needed for clear	*/
    {					/* to end of line.		*/
	for (i = start_y; i <= end_y; i++)
	{
	    line = s->map[i];			/* set map pointer	*/
	    if (i == term_maxrow && term_wraps)	/* special case : last	*/
	    {				/* line. write text without	*/
		TermPutData (line, term_cols - 1);	/* last char,	*/
		s->flags |= ANSI_dirty;			/* set flag	*/
	    }
	    else
		TermPutData (line, term_cols);	/* write whole text	*/
	    if (i < term_maxrow)
	    {
		if (term_wraps)		/* wrap : reposition cursor	*/
		    TermGoto (0, i + 1);
		else			/* else next line		*/
		    TermPutString ("\r");
	    }
	}
    }
#else
    TermGoto (0, start_y);
    for (i = start_y; i <= end_y; i++)	/* for each line		*/
    {
	line = s->map[i];		/* set map pointer		*/
    	if (*tcap_ceol)			/* use clear to end of line..	*/
        {
	    for (j = term_maxcol; j >= 0; j--)
		if (line[j] != ' ')	/* scan for trailing blanks	*/
		    break;

	    if (j == term_maxcol)	/* full line, write it all out	*/
	    {
	    	if (i == term_maxrow && term_wraps)	/* special case	*/
	    	    TermRedrawLine (s, 0, i);	/* call special func.	*/
	    	else
	    	    TermPutData (line, term_cols);	/* write text	*/
	    }
	    else
	    {
	    	TermPutData (line, j + 1);	/* write text		*/
		TermPutString (tcap_ceol);	/* and clear to EOL	*/
	    }
	}
	else
	    TermRedrawLine (s, 0, i);	/* call special func.		*/
	TermPutChar ('\r');		/* goto start of line		*/
	if (i < term_maxrow)		/* and to next line, if not in	*/
	    TermPutChar ('\n');		/* last line			*/
    }
#endif
    TermGoto (s->Cur_x, s->Cur_y);	/* restore cursor position	*/
}

/*************************************************************************
 * HANDLE LINEFEED
 ************************************************************************/

void
TermLineFeed (bool scroll)
{
    Screen	*s	= Cur_Screen;
    int		*flags	= &s->flags;
    int		cur_x	= s->Cur_x;
    int		cur_y	= s->Cur_y;
#ifdef	DEBUG
    Debug (TERM) ("\t\t\tTermLineFeed @ %d,%d flags are %x", cur_x, cur_y, *flags);
#endif
					/* something to clean up	*/
    if (scroll && (*flags & ANSI_dirty))
    {
#ifdef	DEBUG
	Debug (TERM) ("\t\t\tTermLineFeed with tidyup");
#endif

	TermGoto (0, term_maxrow);		/* force a scroll	*/
	TermPutChar ('\x0a');
	TermGoto (term_maxcol, term_maxrow - 1);	/* update char	*/
	TermPutChar (s->map[term_maxrow - 1][term_maxcol]);
	TermGoto (cur_x, cur_y);
	*flags &= ~ANSI_dirty;			/* clear flag		*/
    }
    else				/* everything fine		*/
	TermPutChar ('\x0a');
}

/*************************************************************************
 * SET SCREEN RENDITION
 ************************************************************************/

void
TermSetRendition (int mode)
{
#ifdef	DEBUG
    Debug (TERM) ("\t\t\tTermSetRendition %d", mode);
#endif

    TermPutString (tcap_norm);		/* reset old modes		*/
    TermPutString (tcap_ulof);		/* reset underline		*/
    if (mode)				/* any modes to be set ?	*/
    {
	if (mode & ANSI_mode_bold)	/* check mode bits		*/
	    TermPutString (tcap_bld);
	if (mode & ANSI_mode_dimmed)
	    TermPutString (tcap_dim);
	if (mode & ANSI_mode_underline)
	    TermPutString (tcap_ulon);
	if (mode & ANSI_mode_blink)
	    TermPutString (tcap_bli);
	if (mode & ANSI_mode_reverse)
	    TermPutString (tcap_rev);
    }
}

/*************************************************************************
 * RING TERMINAL BELL
 ************************************************************************/

void
TermBell (void)
{ 
#ifdef	DEBUG
    Debug (TERM) ("\t\t\tTermBell");
#endif

    TermPutString (tcap_bell);
}

/*************************************************************************
 * INSERT A LINE ABOVE THE CURRENT LINE
 ************************************************************************/

void
TermInsertLines (int count)
{
    Screen	*s	= Cur_Screen;
    
#ifdef	DEBUG
    Debug (TERM) ("\t\t\tTermInsertLines %d", count);
#endif

    if (count <= 0)
    	return;

    if (*tcap_isln)
    {
	TermPutString (tgoto (tcap_isln, 0, count));
	s->flags &= ~ANSI_dirty;	/* cleans dirty corner	*/
    }
    elif (*tcap_insl)
    {
    	while (count--)
	    TermPutString (tcap_insl);
	s->flags &= ~ANSI_dirty;	/* cleans dirty corner	*/
    }
    else
	TermRedrawScreen (s->Cur_y, term_maxrow);
}

/*************************************************************************
 * DELETE THE CURRENT LINE
 ************************************************************************/

void
TermDeleteLines (int count)
{
    Screen	*s	= Cur_Screen;
    int		*flags	= &s->flags;
        
#ifdef	DEBUG
    Debug (TERM) ("\t\t\tTermDeleteLines %d", count);
#endif

    if (count <= 0)
    	return;

    if (*tcap_dlln)
    {
	TermPutString (tgoto (tcap_dlln, 0, count));
	if (*flags & ANSI_dirty)
	{
	    if (term_maxrow - count >= s->Cur_y)
	    {				/* no, tidy up			*/
	    	TermGoto (term_maxcol, term_maxrow - count);
	    	TermPutChar (s->map[term_maxrow - count][term_maxcol]);
	    	TermGoto (s->Cur_x, s->Cur_y);
	    }
	    *flags &= ~ANSI_dirty;	/* dirty corner cleaned...	*/
	}
    }
    elif (*tcap_dell)
    {
	TermPutString (tcap_dell);
	if (*flags & ANSI_dirty)
	{
	    if (term_maxcol > s->Cur_y)
	    {				/* not deleting the last line	*/
	    	TermGoto (term_maxcol, term_maxrow - 1);
	    	TermPutChar (s->map[term_maxrow - 1][term_maxcol]);
	    	TermGoto (s->Cur_x, s->Cur_y);
	    }
	    *flags &= ~ANSI_dirty;	/* dirty corner cleaned...	*/
	}
	while (--count)
	    TermPutString (tcap_dell);
    }
    else
	TermRedrawScreen (s->Cur_y, term_maxrow);
}

/*************************************************************************
 * INSERT count CHARACTERS
 ************************************************************************/

void
TermInsertChars (int count)
{
    Screen	*s	= Cur_Screen;
    int		cur_y	= s->Cur_y;

#ifdef	DEBUG
    Debug (TERM) ("\t\t\tTermInsertChars %d", count);
#endif

    if (count <= 0)
    	return;

    if (cur_y == term_maxrow)		/* insert in last line		*/
    	s->flags &= ~ANSI_dirty;	/* cleans by shifting 		*/
    	
    if (*tcap_iscn)
	TermPutString (tgoto (tcap_iscn, 0, count));
    elif (*tcap_insc)
	while (count--)
	    TermPutString (tcap_insc);
    else
    {
    	int	cur_x	= s->Cur_x;
    	
	TermRedrawLine (s, cur_x, cur_y);
	TermGoto (cur_x, cur_y);
    }
}

/*************************************************************************
 * DELETE count CHARACTERS
 ************************************************************************/

void
TermDeleteChars (int count)
{
    Screen	*s	= Cur_Screen;
    
#ifdef	DEBUG
    Debug (TERM) ("\t\t\tTermDeleteChars %d", count);
#endif

    if (count <= 0)
    	return;

    if (*tcap_dlcn)
    {
	TermPutString (tgoto (tcap_dlcn, 0, count));
	if (s->Cur_y == term_maxrow && s->flags & ANSI_dirty)
	{				
					/* is the dirty place deleted ?	*/
	    if (term_maxcol - count >= s->Cur_x)
	    {				/* no, tidy up			*/
	    	TermGoto (term_maxcol - count, term_maxrow);
	    	TermPutChar (s->map[term_maxrow][term_maxcol - count]);
	    	TermGoto (s->Cur_x, s->Cur_y);
	    }
	    s->flags &= ~ANSI_dirty;
	}
    }
    elif (*tcap_delc)
    {
	TermPutString (tcap_delc);
	if (s->Cur_y == term_maxrow && s->flags & ANSI_dirty)
	{
	    if (term_maxcol - count >= s->Cur_x)
	    {
	    	TermGoto (term_maxcol - 1, term_maxrow);
	    	TermPutChar (s->map[term_maxrow][term_maxcol - 1]);
	    	TermGoto (s->Cur_x, s->Cur_y);
	    }
	    s->flags &= ~ANSI_dirty;
	}
	while (--count)
	    TermPutString (tcap_delc);
    }
    else
    {
	TermRedrawLine (s, s->Cur_x, s->Cur_y);
	TermGoto (s->Cur_x, s->Cur_y);
    }
}

/*************************************************************************
 * WRITE A CHARACTER
 ************************************************************************/

void
TermChar (char ch)
{
    Screen	*s	= Cur_Screen;
#ifdef	DEBUG
    static char	*fname	= "TermChar";
    Debug (TERM) ((ch >= ' ' && ch < 127) ? "%s '%c' @ %d %d" : "%s <%02x> @ %d %d", fname, ch, s->Cur_x, s->Cur_y);
#endif

    if (term_wraps &&
      s->Cur_y == term_maxrow &&
      s->Cur_x == term_maxcol)
    {  
	TermRedrawLine (s, term_maxcol - 1, term_maxrow);
#ifdef	DEBUG
	Debug (TERM_ALL) ("%s set flags to %x", fname, s->flags);
#endif
	TermGoto (s->Cur_x, s->Cur_y);
    }
    else
	TermPutChar (ch);
}

/*************************************************************************
 * REDRAW THE CURRENT SCREEN
 *
 * - Put the whole buffered screen contents to the terminal.
 * - Handle the term_wrap problem.
 * - Print the Window name into upper right corner.
 * - Set right rendition.
 *
 * Parameter  :	- nothing -
 * Return     :	- nothing -
 *
 ************************************************************************/

void
RedrawScreen (Window *w)
{
    char	*name	= w->ObjNode.Name;
    Screen	*s;
    byte	**map;
    int 	rows;
    int 	cols;
    int 	i, j;

#ifdef	DEBUG
    Debug (TERM) ("\t\t\tRedrawScreen...");
#endif

    Wait (&w->WriteLock);		/* prevent window write access	*/
    s = &w->Screen;			/* copy values to locals	*/
    map = s->map;
    rows = s->Rows;
    cols = s->Cols;
    s->flags &= ~ANSI_dirty;		/* screen is not dirty		*/
    TermPutString (tcap_norm);
    TermClearScreen ();

    for (i = 0; i < (term_wraps ? rows - 1 : rows); i++)
    {
	for (j = cols - 1; j >= 0; j--)
	    if (map[i][j] != ' ')
		break;
	if (j >= 0)
	    TermPutData (map[i], j + 1);

	if (i < rows - 1)
	    TermPutString ("\r\n");
    }

    if (term_wraps)
    {
	for (j = cols - 1; j >= 0; j--)
	    if (map[i][j] != ' ')
		break;
	    if (j >= 0)
	    {
		if (j == cols - 1)
		{
		    TermPutData (map[rows - 1], j - 1);
		    TermRedrawLine (s, cols - 2, rows - 1);
		}
		else
		    TermPutData (map[rows - 1], j + 1);
	    }
    }
    TermPutString (tgoto (tcap_goto, cols - strlen (name), 0));
    TermPutString (tcap_rev);
    TermPutString (name);
    TermPutString (tcap_norm);
    TermPutString (tgoto (tcap_goto, s->Cur_x, s->Cur_y));
    TermSetRendition (s->mode);
    TermFlush ();
    Signal (&w->WriteLock);		/* prevent window write access	*/
}


/*************************************************************************
 * READ A CONTROL STRING
 *
 * - Get the termcap entry for the control string.
 * - If no termcap entry exists, set the string pointer
 *   to tcap_null.
 * - Delete trailing padding information.
 *
 * Parameter  :	code	= termcap entry code
 *		t_str	= pointer to tcap string
 * Return     :	TRUE	= an entry was found
 *		FALSE	= no entry available
 *
 ************************************************************************/

static int
GetControl (char *code, char **t_str)
{
    *t_str = tgetstr (code, &tcap_index);
    if (*t_str == NULL)
    {
	*t_str = tcap_null;
	return FALSE;
    }
    else
    {
	while ((**t_str >= '0' && **t_str <= '9') || **t_str == '.')
	    (*t_str)++;
	return TRUE;
    }
}


/*************************************************************************
 * INITIALISE THE TERMINAL
 *
 * - Get the termcap entry for the initialisation string
 *   and send it to the terminal (if any).
 * - Get the termcap entry for the initialisation file,
 *   open it and copy the contents to the terminal.
 *
 * Parameter  :	- nothing -
 * Return     :	- nothing -
 *
 ************************************************************************/

void
TermInit (void)
{
#ifdef	DEBUG
    static char	*fname = "TermInit";
#endif
    char	buffer[256];
    int 	count = 0;
    Object	*o;
    Stream	*s;

#ifdef	DEBUG
    Debug (SETTERM) ("%s started.", fname);
#endif
    
					/* get a Terminal buffer	*/
    while (term_output == NULL)		/* if not yet allocated.	*/
    {
    	term_output = (char *) Malloc (TermBufSize);
    	if (term_output == NULL)
    	    Delay (OneSec / 10);	/* failed: wait, then try again	*/
    }
    term_ocount = 0;

    tcap_index = tcap_buff;

    GetControl ("cl", &tcap_clrs);	/* clear whole screen		*/
    GetControl ("cd", &tcap_ceos);	/* clear to end of screen	*/
    GetControl ("ce", &tcap_ceol);	/* clear to end of line		*/
    GetControl ("AL", &tcap_isln);	/* insert n lines above current	*/
    GetControl ("DL", &tcap_dlln);	/* delete n lines		*/
    GetControl ("al", &tcap_insl);	/* insert line above current	*/
    GetControl ("dl", &tcap_dell);	/* delete current line		*/
    GetControl ("IC", &tcap_iscn);	/* insert n characters		*/
    GetControl ("DC", &tcap_dlcn);	/* delete n characters		*/
    GetControl ("ic", &tcap_insc);	/* insert character		*/
    GetControl ("dc", &tcap_delc);	/* delete current character	*/
    GetControl ("cm", &tcap_goto);	/* cursor to row [m] col [n]	*/

    if (!GetControl ("bl", &tcap_bell))	/* audible bell		*/
	if (!GetControl ("vb", &tcap_bell))	/* else visible bell	*/
	    tcap_bell = "\007\0";		/* else ^G (default)	*/

    if (GetControl ("me", &tcap_norm))	/* normal attributes		*/
    {					/* if normal, then check others	*/
	GetControl ("md", &tcap_bld);	/* bold				*/
	GetControl ("mh", &tcap_dim);	/* dimmed			*/
	GetControl ("mb", &tcap_bli);	/* blinking			*/
	GetControl ("mr", &tcap_rev);	/* reversed			*/
    }
    elif (GetControl ("se", &tcap_norm))	/* else try standout	*/
	GetControl ("so", &tcap_rev);
    else
    {
    	tcap_norm = tcap_null;		/* reset old values to null	*/
    	tcap_bld = tcap_null;
    	tcap_dim = tcap_null;
    	tcap_bli = tcap_null;
    	tcap_rev = tcap_null;
    }
    if (GetControl ("ue", &tcap_ulof))	/* if underlined off, then try	*/
	GetControl ("us", &tcap_ulon);	/* underlined on		*/
    else
    	tcap_ulon = tcap_null;


    term_wraps = tgetflag ("am");	/* does the terminal wrap ?	*/
    term_rows = tgetnum ("li");		/* get number of rows		*/
    if (term_rows <= 0) term_rows = 24;	/* default is 24		*/
    term_cols = tgetnum ("co");		/* get number of coloumns	*/
    if (term_cols <= 0) term_cols = 80;	/* default is 80		*/
    term_maxrow = term_rows - 1;
    term_maxcol = term_cols - 1;

    if (GetControl ("is", &tcap_init))
    {
	TermPutData (tcap_init, strlen (tcap_init));
    }

    if (GetControl ("if", &tcap_inif))
    {
	o = Locate (NULL, tcap_inif);
	if (o == NULL)
	    return;
	s = Open (o, NULL, O_ReadOnly);
	if (s == NULL)
	{
	    Close (o);
	    return;
	}
	while ((count = (int) Read (s, buffer, sizeof (buffer), 10 * OneSec)) >= 0)
	    if (count > 0)
		TermPutData (buffer, count);
	if (count < -1)
#ifdef	DEBUG
	    Debug (ERROR) ("%s : error %E while reading init file %s.",
		fname, count, tcap_inif);
#endif
	Close (s);
	Close (o);
    }
    TermFlush ();

#ifdef	DEBUG
    Debug (SETTERM) ("%s ready.", fname);
#endif
}

/*************************************************************************
 * INITIALISE THE TERMINAL
 *
 * - Get the termcap entry for the termination string
 *   and send it to the terminal (if any).
 * - Get the termcap entry for the termination file,
 *   open it and copy the contents to the terminal.
 *
 * Parameter  :	- nothing -
 * Return     :	- nothing -
 *
 ************************************************************************/

void
TermTidy (void)
{
#ifdef	DEBUG
    static char	*fname = "TermTidy";
#endif
    char	buffer[256];
    int 	count = 0;
    Object	*o;
    Stream	*s;

#ifdef	DEBUG
    Debug (SETTERM) ("%s started.", fname);
#endif
    
    if (GetControl ("rs", &tcap_init))
    {
	TermPutData (tcap_init, strlen (tcap_init));
    }

    if (GetControl ("rf", &tcap_inif))
    {
	o = Locate (NULL, tcap_inif);
	if (o == NULL)
	    return;
	s = Open (o, NULL, O_ReadOnly);
	if (s == NULL)
	{
	    Close (o);
	    return;
	}
	while ((count = (int) Read (s, buffer, sizeof (buffer), 10 * OneSec)) >= 0)
	    if (count > 0)
		TermPutData (buffer, count);
	if (count < -1)
#ifdef	DEBUG
	    Debug (ERROR) ("%s : error %E while reading init file %s.",
		fname, count, tcap_inif);
#endif
	Close (s);
	Close (o);
    }
    TermFlush ();

#ifdef	DEBUG
    Debug (SETTERM) ("%s ready.", fname);
#endif
    if (term_output != NULL)
    	Free (term_output);
    	
}
/*--- end of terminal.c ---*/
