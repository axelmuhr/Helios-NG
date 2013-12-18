/*************************************************************************
**									**
**	       C O N S O L E  &  W I N D O W   S E R V E R		**
**	       -------------------------------------------		**
**									**
**		    Copyright (C) 1989, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** ansi.c								**
**									**
**	- Ansi Emulator 						**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	20/11/89 : C. Fleischer					**
** Changed   :	18/01/90 : C. Fleischer	 terminal reinitialisation	**
** Changed   :	12/09/90 : G. Jodlauk	 for use with TTY		**
*************************************************************************/

#define __in_ansi	1		/* flag that we are in this module */

#include "tty.h"

/*************************************************************************
 * ANSI WORK ROUTINES
 *
 * - These routines really do the work : they maintain the window's
 *   screen buffer and pass output to the terminal if the window they
 *   are just working on is the currently displayed one.
 *
 ************************************************************************/


static void
AnsiBell (Screen *s)
{
#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiBell");
#endif
    if (ThisScreen)
	TermBell ();
}

static void
AnsiBackSpace (Screen *s)
{
    int		*cur_x	= &s->Cur_x;

#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiBackSpace");
#endif
    if (*cur_x > 0)
    {
	(*cur_x)--;
	if (ThisScreen)
	    TermChar ('\x08');
    }
}

static void
AnsiCarriageReturn (Screen *s)
{
#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiCarriageReturn");
#endif
    s->Cur_x = 0;
    if (ThisScreen)
	TermChar ('\x0d');
}

static void
AnsiLineFeed (Screen *s)
{
    char	**map = s->map;
    char	*line;
    int		*cur_y = &s->Cur_y;
    int		i;

#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiLineFeed in line %d", *cur_y);
#endif
    if (*cur_y >= term_maxrow)		/* last line of screen ?	*/
    {
    	line = map[0];
	for (i = 0; i < term_maxrow; i++)	/* scroll map		*/
	    map[i] = map[i + 1];
	memset (line, ' ', term_cols);
	map[term_maxrow] = line;
	*cur_y = term_maxrow;
        if (ThisScreen)
	    TermLineFeed (TRUE);	/* Linefeed with scroll 	*/
    }
    else
    {
	(*cur_y)++;			/* next line			*/
        if (ThisScreen)
	    TermLineFeed (FALSE);	/* Linefeed without scroll 	*/
    }
}

static void
AnsiCursorUp (Screen *s, int count)
{
    int		*cur_y	= &s->Cur_y;

#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiCursorUp %d in line %d", count, *cur_y);
#endif
    *cur_y -= count;
    if (*cur_y < 0)
	*cur_y = 0;
    if (ThisScreen)
	TermGoto (s->Cur_x, *cur_y);
}

static void
AnsiCursorDown (Screen *s, int count)
{
    int		*cur_y	= &s->Cur_y;

#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiCursorDown %d in line %d", count, *cur_y);
#endif
    *cur_y += count;
    if (*cur_y > term_maxrow)
	*cur_y = term_maxrow;
    if (ThisScreen)
	TermGoto (s->Cur_x, *cur_y);
}

static void
AnsiCursorRight (Screen *s, int count)
{
    int		*cur_x	= &s->Cur_x;

#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiCursorRight %d", count);
#endif
    *cur_x += count;
    if (*cur_x > term_maxcol)
	*cur_x = term_maxcol;
    if (ThisScreen)
	TermGoto (*cur_x, Cur_Screen->Cur_y);
}

static void
AnsiCursorLeft (Screen *s, int count)
{
    int		*cur_x	= &s->Cur_x;

#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiCursorLeft %d", count);
#endif
    *cur_x -= count;
    if (*cur_x < 0)
	*cur_x = 0;
    if (ThisScreen)
	TermGoto (*cur_x, Cur_Screen->Cur_y);
}

static void
AnsiCursorNext (Screen *s, int count)
{
#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiCursorNext %d", count);
#endif
    AnsiCarriageReturn (s);
    while (count-- > 0)
    	AnsiLineFeed (s);
}

static void
AnsiCursorPrev (Screen *s, int count)
{
    int		*cur_y	= &s->Cur_y;

#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiCursorPrev %d", count);
#endif
    AnsiCarriageReturn (s);
    *cur_y -= count;
    if (*cur_y < 0)
	*cur_y = 0;
    if (ThisScreen)
	TermGoto (Cur_Screen->Cur_x, *cur_y);
}

static void
AnsiGoto (Screen *s, int x, int y)
{
    int		*cur_x	= &s->Cur_x;
    int		*cur_y	= &s->Cur_y;

    if (x < 0)
	x = 0;
    elif (x > term_maxcol)
      {
	x = term_maxcol;
      }
    
    *cur_x = x;

    if (y < 0)
	y = 0;
    elif (y > term_maxrow)
      {
	y = term_maxrow;
      }
      
    *cur_y = y;
#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiGoto %d %d => %d %d", x, y, *cur_x, *cur_y);
#endif
    if (ThisScreen)
	TermGoto (x, y);
}

static void
AnsiClearScreen (Screen *s)
{
    int		i;

#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiClearScreen");
#endif
    for (i = 0; i < term_rows; i++)
	memset (s->map[i], ' ', term_cols);
    s->Cur_x = 0;
    s->Cur_y = 0;
    if (ThisScreen)
	TermClearScreen ();
}

static void
AnsiClearEos (Screen *s)
{
    int		x = s->Cur_x;
    int		y = s->Cur_y;

#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiClearEos");
#endif
    memset (&s->map[y][x], ' ', term_cols - x);
    for (y++; y < term_rows; y++)
	memset (s->map[y], ' ', term_cols);
    if (ThisScreen)
	TermClearEos ();
}

static void
AnsiClearEol (Screen *s)
{
    int		x = s->Cur_x;

#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiClearEol");
#endif
    memset (&s->map[s->Cur_y][x], ' ', term_cols - x);
    if (ThisScreen)
	TermClearEol ();
}


static void
AnsiInsertLines (Screen *s, int count, bool scroll)
{
    char	**smap	= s->map;	/* source map			*/
    char	**dmap	= s->tempmap;	/* destination map		*/
    int		rows	= s->Rows;	/* number of map rows		*/
    int		cols	= s->Cols;	/* number of map columns	*/
    int		cur_y	= s->Cur_y;
    int		i, j;
        
#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiInsertLines %d before line %d", count, cur_y);
#endif

    if ( count <= 0)
    	return;

    for (i = 0, j = 0; i < cur_y; i++, j++)
	dmap[i] = smap[j];		/* copy unchanged lines		*/
    for (i += count; i < rows; i++, j++)
	dmap[i] = smap[j];		/* copy unchanged lines down	*/
    for (i = cur_y; j < rows; i++, j++)
    {			
	dmap[i] = smap[j];		/* copy rest of lines up	*/
	memset (dmap[i], ' ', cols);	/* and clear them		*/
    }

    s->map = dmap;			/* exchange maps		*/
    s->tempmap = smap;

    if ( !scroll )
    	AnsiGoto (s, 0, cur_y);

    if (ThisScreen)
	TermInsertLines (count);
}

static void
AnsiDeleteLines (Screen *s, int count, bool scroll)
{
    char	**smap	= s->map;	/* source map			*/
    char	**dmap	= s->tempmap;	/* destination map		*/
    int		rows	= s->Rows;	/* number of map rows		*/
    int		cols	= s->Cols;	/* number of map columns	*/
    int		cur_y	= s->Cur_y;
    int		i, j;
        
#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiDeleteLines %d from line %d", count, cur_y);
#endif

    if ( count <= 0)
    	return;

    for (i = 0, j = 0; i < cur_y; i++, j++)
	dmap[i] = smap[j];		/* copy unchanged lines		*/
    for (j += count; j < rows; i++, j++)
	dmap[i] = smap[j];		/* copy unchanged lines up	*/
    for (j = cur_y; i < rows; i++, j++)
    {			
	dmap[i] = smap[j];		/* copy rest of lines down	*/
	memset (dmap[i], ' ', cols);	/* and clear them		*/
    }

    s->map = dmap;			/* exchange maps		*/
    s->tempmap = smap;

    if ( !scroll )
    	AnsiGoto (s, 0, cur_y);
    if (ThisScreen)
	TermDeleteLines (count);
}

static void
AnsiChar (Screen *s, char ch)
{
#ifdef	DEBUG
    static char	*fname	= "AnsiChar";
#endif
    int		*cur_x = &s->Cur_x;
    int		cur_y = s->Cur_y;

#ifdef	DEBUG
    Debug (ANSI_ALL) ((ch >= ' ' && ch < 127) ? "%s '%c' @ %d,%d" : "%s <%02x> @ %d,%d", 
    	fname, ch, *cur_x, cur_y);
#endif

    if (*cur_x > term_maxcol)		/* implicit scroll		*/
    {
        AnsiCarriageReturn (s);
        AnsiLineFeed (s);
    }

    s->map[cur_y][*cur_x] = ch;	/* store char in map		*/

    if (ThisScreen)
        TermChar (ch);			/* put char to screen		*/

/*
    if (*cur_x == term_maxcol &&	/ * end of line and not		* /
      cur_y < term_maxrow)		/ * in the last line ?		* /
	AnsiGoto (s, term_maxcol, cur_y);	/ * yes, goto last char	* /
*/
    (*cur_x)++;				/* x++, even behind maxcol !	*/
}

static void
AnsiInsertChars (Screen *s, int count)
{
    int		cur_x	= s->Cur_x;
    int		cur_y	= s->Cur_y;
    char	*line	= s->map[cur_y];
    int		i;

#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiInsertChars %d", count);
#endif
    if (cur_x + count > term_maxcol)
    {
	memset (&line[cur_x], ' ', term_cols - cur_x);
	count = term_cols - cur_x;
    }
    else
    {
	for (i = term_maxcol; i >= cur_x + count; i--)
	    line[i] = line[i - count];
	memset (&line[cur_x], ' ', count);
    }
    if (ThisScreen)
	TermInsertChars (count);
}

static void
AnsiDeleteChars (Screen *s, int count)
{
    int		cur_x	= s->Cur_x;
    int		cur_y	= s->Cur_y;
    char	*line	= s->map[cur_y];
    int		i;

#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiDeleteChars %d", count);
#endif
    if (cur_x + count > term_cols)
	count = term_cols - cur_x;
    for (i = cur_x; i < term_cols - count; i++)
	line[i] = line[i + count];
    memset (&line[i], ' ', count);
    if (ThisScreen)
        TermDeleteChars (count);
}


static void
AnsiSetRendition (Screen *s)
{
    int		*mode = &s->mode;
    int		arg;
    int		i = 0;

    if( s->args == 0 ) s->argv[0] = 0, s->args = 1;
    
    for (i = 0; i < s->args; i++)
    {
	arg = (i == s->args) ? 0 : s->argv[i];
#ifdef	DEBUG
	Debug (ANSI) ("\t\t\tAnsiSetRendition %d", arg);
#endif

	switch (arg)
	{
	case 1 :			/* bold (md)			*/
	    *mode |= ANSI_mode_bold;
	    break;
	case 2 :			/* dimmed (mh)			*/
	    *mode |= ANSI_mode_dimmed;
	    break;
	case 4 :			/* underlined (us & !ul)	*/
	    *mode |= ANSI_mode_underline;
	    break;
	case 5 :			/* blinking (mb)		*/
	    *mode |= ANSI_mode_blink;
	    break;
	case 7 :			/* reverse (mr)			*/
	    *mode |= ANSI_mode_reverse;
	    break;
	case 0 :			/* normal (me)			*/
	    *mode = ANSI_mode_normal;
	    break;
	default :
	    continue;
	}
	if (ThisScreen)
	   TermSetRendition (*mode);
    }
}

static void
AnsiScrollUp (Screen *s, int count)
{
#if 0
    int		*cur_x	= &s->Cur_x;
    int		*cur_y	= &s->Cur_y;
    int		save_x	= *cur_x;
    int		save_y	= *cur_y;
    int		i;

#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiScrollUp %d in line %d", count, *cur_y);
#endif
    if (count >= term_rows)
    {
	AnsiClearScreen (s);
    }
    else
    {
    	AnsiGoto (s, 0, 0);
	*cur_x = 0; *cur_y = 0;
	for (i = 0; i < count; i++)
	    AnsiDeleteLine (s);
    }
    *cur_x = save_x; *cur_y = save_y;
    AnsiGoto (s, save_x, save_y);
#else
    int		cur_x	= s->Cur_x;
    int		cur_y	= s->Cur_y;
    
#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiScrollUp %d", count);
#endif

    if (count >= term_rows)
    {
	AnsiClearScreen (s);
    	AnsiGoto (s, cur_x, cur_y);
    }
    else
    {
    	AnsiGoto (s, 0, 0);
    	AnsiDeleteLines (s, count, TRUE);
    	AnsiGoto (s, cur_x, cur_y);
    }
#endif
}

static void
AnsiScrollDown (Screen *s, int count)
{
#if 0
    int		*cur_x	= &s->Cur_x;
    int		*cur_y	= &s->Cur_y;
    int		save_x	= *cur_x;
    int		save_y	= *cur_y;
    int		i;

#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiScrollDown %d in line %d", count, *cur_y);
#endif
    if (count >= term_rows)
    {
	AnsiClearScreen (s);
    }
    else
    {
    	AnsiGoto (s, 0, 0);
	*cur_x = 0; *cur_y = 0;
	for (i = 0; i < count; i++)
	    AnsiInsertLine (s);
    }
    *cur_x = save_x; *cur_y = save_y;
    AnsiGoto (s, save_x, save_y);
#else
    int		cur_x	= s->Cur_x;
    int		cur_y	= s->Cur_y;
    
#ifdef	DEBUG
    Debug (ANSI) ("\t\t\tAnsiScrollDown %d", count);
#endif

    if (count >= term_rows)
    {
	AnsiClearScreen (s);
    	AnsiGoto (s, cur_x, cur_y);
    }
    else
    {
    	AnsiGoto (s, 0, 0);
    	AnsiInsertLines (s, count, TRUE);
    	AnsiGoto (s, cur_x, cur_y);
    }
#endif
}

void
AnsiFlush (Screen *s)
{
#ifdef	DEBUG
    Debug (ANSI_ALL) ("AnsiFlush");
#endif
    if (ThisScreen)
	TermFlush ();
}

/*************************************************************************
 * RESET ESCAPE SEQUENCE PROCESSING
 *
 * - Clear all the escape flags.
 * - Reset the argument counters.
 *
 * Parameter  :	s	= screen to work on
 * Return     :	- nothing -
 *
 ************************************************************************/

static void
AnsiResetEscape (Screen *s)
{
#ifdef	DEBUG
    Debug (ANSI_ALL) ("AnsiResetEscape");
#endif
    s->flags &= ~(ANSI_escape_found | ANSI_in_escape);
    s->argc = 0;
    s->args = 0;
}

/*************************************************************************
 * WRITE ANSI DATA TO A WINDOW
 *
 * - Detect escape sequences using the screen flags,
 *   call sequence handlers if necessary,
 *   else write the char to the screen.
 *
 * Parameter  :	s	= screen where the data goes
 *		ch	= character to be written
 * Return     :	- nothing -
 *
 ************************************************************************/

void
AnsiWrite (Screen *s, char ch)
{
#ifdef	DEBUG
    static char	*fname	= "AnsiWrite";
#endif
    int 	*args	= &s->args;
    int		*argc	= &s->argc;
    int 	*flags	= &s->flags;
    int 	i;

#ifdef	DEBUG
    Debug (ANSI) ((ch >= ' ' && ch < 127) ? "%s '%c' " : "%s <%02x> ",
	fname, ch);
#endif

    if (*flags & ANSI_in_escape)	/* currently in an escape seq.	*/
    {
#ifdef	DEBUG
	Debug (ANSI_ALL) ("in escape ");
#endif

	switch (ch)
	{
	case '0' : case '1' : case '2' : case '3' : case '4' :
	case '5' : case '6' : case '7' : case '8' : case '9' :
					/* digit, part of an argument	*/
	    if (*args == *argc)
	    {
		if (*args < MaxArgs)
		{
		    (*args)++;		/* first one, count it		*/
		}
		s->argv[*argc] = ch - '0';	/* set new argument	*/
	    }
	    else				/* add to arg		*/
		s->argv[*argc] = 10 * s->argv[*argc] + ch - '0';
	    return;			/* process next char		*/
	case ';' :
					/* semicolon marks end of arg	*/
	    *argc = *args;
	    return;			/* process next char		*/
	case '@' :
	    AnsiInsertChars (s, (*args) ? s->argv[0] : 1);
	    break;
	case 'A' :
	    AnsiCursorUp (s, (*args) ? s->argv[0] : 1);
	    break;
	case 'B' :
	    AnsiCursorDown (s, (*args) ? s->argv[0] : 1);
	    break;
	case 'C' :
	    AnsiCursorRight (s, (*args) ? s->argv[0] : 1);
	    break;
	case 'D' :
	    AnsiCursorLeft (s, (*args) ? s->argv[0] : 1);
	    break;
	case 'E' :
	    AnsiCursorNext (s, (*args) ? s->argv[0] : 1);
	    break;
	case 'F' :
	    AnsiCursorPrev (s, (*args) ? s->argv[0] : 1);
	    break;
	case 'H' :
	    AnsiGoto (s, (*args > 1) ? s->argv[1] - 1 : 1,
			  (*args > 0) ? s->argv[0] - 1 : 1);
	    break;
	case 'J' :
	    AnsiClearEos (s);		  break;
	case 'K' :
	    AnsiClearEol (s);		  break;
	case 'L' :
	    AnsiInsertLines (s, (*args) ? s->argv[0] : 1, FALSE);
	    break;
	case 'M' :
	    AnsiDeleteLines (s, (*args) ? s->argv[0] : 1, FALSE);
	    break;
	case 'P' :
	    AnsiDeleteChars (s, (*args) ? s->argv[0] : 1);
	    break;
	case 'S' :
	    AnsiScrollUp (s, (*args) ? s->argv[0] : 1);
	    break;
	case 'T' :
	    AnsiScrollDown (s, (*args) ? s->argv[0] : 1);
	    break;
	case 'm' :
	    AnsiSetRendition (s);
	}
	AnsiResetEscape (s);
    }
    else
    {
#ifdef	DEBUG
	Debug (ANSI_ALL) ("normal ");
#endif
	if ((*flags & ANSI_escape_found) && ch == '[')
	{
	    *flags |= ANSI_in_escape;
	    *flags &= ~ANSI_escape_found;
#ifdef	DEBUG
	    Debug (ANSI_ALL) ("in escape ");
#endif
	    return;
	}
	*flags &= ~ANSI_escape_found;
	switch (ch)
	{
	case '\n' :
	    AnsiLineFeed (s);		   break;
	case '\r' :
	    AnsiCarriageReturn (s);	   break;
	case '\x07' :
	    AnsiBell (s);		   break;
	case '\x08' :
	    AnsiBackSpace (s);		   break;
	case '\x09' :
	    i = 8 - (s->Cur_x % 8);
	    if (s->Cur_x + i > term_maxcol)
	    {
		AnsiCarriageReturn (s);
		AnsiLineFeed (s);
	    }
	    else
		AnsiGoto (s, s->Cur_x + i, s->Cur_y);
	    break;
	case '\x0b' :
	    AnsiCursorUp (s, 1);	  break;
	case '\x0c' :
	    AnsiClearScreen (s);	  break;
	case '\x1b' :
	    *flags |= ANSI_escape_found;
	    break;
	case '\x9b' :
	    *flags |= ANSI_in_escape;
	    break;
	default :
	    if (ch >= ' ')
		AnsiChar (s, ch);
	}
    }
}


/*************************************************************************
 * WRITE ANSI DATA TO A WINDOW
 *
 * - Detect escape sequences using the screen flags,
 *   call sequence handlers if necessary,
 *   else write the char to the screen.
 *
 * Parameter  :	s	= screen where the data goes
 *		data	= characters to be written
 *		count	= number of characters
 * Return     :	- nothing -
 *
 ************************************************************************/

void
AnsiWriteData (Screen *s, char *data, int count)
{
#ifdef	DEBUG
    static char	*fname	= "AnsiWriteData";
#endif
    int		pos	= 0;
    int 	*args	= &s->args;
    int		*argc	= &s->argc;
    int 	*flags	= &s->flags;
    int 	i;
    char	ch;

    while (pos < count)
    {
	ch = data[pos++];

#ifdef	DEBUG
	Debug (ANSI) ((ch >= ' ' && ch < 127) ? "%s '%c' " : "%s <%02x> ",
	    fname, ch);
#endif

	if (*flags & ANSI_in_escape)	/* currently in an escape seq.	*/
	{
#ifdef	DEBUG
	    Debug (ANSI_ALL) ("in escape ");
#endif

	    switch (ch)
	    {
	    case '0' : case '1' : case '2' : case '3' : case '4' :
	    case '5' : case '6' : case '7' : case '8' : case '9' :
					/* digit, part of an argument	*/
		if (*args == *argc)
		{
		    if (*args < MaxArgs)
		    {
			(*args)++;	/* first one, count it		*/
		    }
		    s->argv[*argc] = ch - '0';	/* set new argument	*/
		}
		else				/* add to arg		*/
		    s->argv[*argc] = 10 * s->argv[*argc] + ch - '0';
		continue;		/* process next char		*/
	    case ';' :
					/* semicolon marks end of arg	*/
		*argc = *args;
		continue;		/* process next char		*/
	    case '@' :
		AnsiInsertChars (s, (*args) ? s->argv[0] : 1);
		break;
	    case 'A' :
		AnsiCursorUp (s, (*args) ? s->argv[0] : 1);
		break;
	    case 'B' :
		AnsiCursorDown (s, (*args) ? s->argv[0] : 1);
		break;
	    case 'C' :
		AnsiCursorRight (s, (*args) ? s->argv[0] : 1);
		break;
	    case 'D' :
		AnsiCursorLeft (s, (*args) ? s->argv[0] : 1);
		break;
	    case 'E' :
		AnsiCursorNext (s, (*args) ? s->argv[0] : 1);
		break;
	    case 'F' :
		AnsiCursorPrev (s, (*args) ? s->argv[0] : 1);
		break;
	    case 'H' :
		AnsiGoto (s, (*args > 1) ? s->argv[1] - 1 : 1,
			      (*args > 0) ? s->argv[0] - 1 : 1);
		break;
	    case 'J' :
		AnsiClearEos (s);	      break;
	    case 'K' :
		AnsiClearEol (s);	      break;
	    case 'L' :
	    	AnsiInsertLines (s, (*args) ? s->argv[0] : 1, FALSE);
	    	break;
	    case 'M' :
	    	AnsiDeleteLines (s, (*args) ? s->argv[0] : 1, FALSE);
	    	break;
	    case 'P' :
		AnsiDeleteChars (s, (*args) ? s->argv[0] : 1);
		break;
	    case 'S' :
		AnsiScrollUp (s, (*args) ? s->argv[0] : 1);
		break;
	    case 'T' :
		AnsiScrollDown (s, (*args) ? s->argv[0] : 1);
		break;
	    case 'm' :
		AnsiSetRendition (s);
	    }
	    AnsiResetEscape (s);
	}
	else
	{
#ifdef	DEBUG
	    Debug (ANSI_ALL) ("normal ");
#endif
	    if ((*flags & ANSI_escape_found) && ch == '[')
	    {
		*flags |= ANSI_in_escape;
		*flags &= ~ANSI_escape_found;
#ifdef	DEBUG
		Debug (ANSI_ALL) ("in escape ");
#endif
		continue;
	    }
	    *flags &= ~ANSI_escape_found;
	    switch (ch)
	    {
	    case '\n' :
		AnsiLineFeed (s);	       break;
	    case '\r' :
		AnsiCarriageReturn (s);        break;
	    case '\x07' :
		AnsiBell (s);		       break;
	    case '\x08' :
		AnsiBackSpace (s);	       break;
	    case '\x09' :
		i = 8 - (s->Cur_x % 8);
		if (s->Cur_x + i > term_maxcol)
		{
		    AnsiCarriageReturn (s);
		    AnsiLineFeed (s);
		}
		else
		    AnsiGoto (s, s->Cur_x + i, s->Cur_y);
		break;
	    case '\x0b' :
		AnsiCursorUp (s, 1);	      break;
	    case '\x0c' :
		AnsiClearScreen (s);	      break;
	    case '\x1b' :
		*flags |= ANSI_escape_found;	
		break;
	    case '\x9b' :
		*flags |= ANSI_in_escape;	
		break;
	    default :
		if (ch >= ' ')
		    AnsiChar (s, ch);
	    }
	}
    }
}


/*************************************************************************
 * INITIALISE THE ANSI EMULATOR
 *
 * - Get the termcap entry for the specified terminal.
 * - If found, get the used strings and values from the entry
 *   else initialise to defaults.
 * - Initialise the terminal.
 * - Initialise the input cooker.
 *
 * Parameter  :	term	= terminal name
 * Return     :	TRUE	if terminal was found.
 *
 ************************************************************************/

bool
AnsiInit (char *term)
{
#ifdef DEBUG
    static char	*fname	= "AnsiInit";
#endif

    if (*term)
    {
    	switch (tgetent (tcap_data, term))	/* get termcap entry	*/
    	{
   	case -1 :
#ifdef	DEBUG
	    Debug (ERROR) ("%s : unable to open termcap database.", fname );
#endif
	    return FALSE;
    	case  0 :
#ifdef	DEBUG
	    Debug (ERROR) ("%s : Terminal %s not found in termcap database.",
	    	fname, term);
#endif
	    return FALSE;
	}
    }

    TermInit ();			/* initialise terminal output	*/
    InputInit ();			/* initialise terminal input	*/
    return TRUE;
}


/*************************************************************************
 * REINITIALISE THE ANSI EMULATOR
 *
 * - Get the termcap entry for the specified terminal.
 * - If found, get the used strings and values from the entry
 *   else initialise to defaults.
 * - Initialise the terminal.
 * - Initialise the input cooker.
 *
 * Parameter  :	term	= terminal name
 * Return     :	TRUE	if terminal was found.
 *
 ************************************************************************/

bool
AnsiReinit (char *term)
{
#ifdef	DEBUG
    static char	*fname	= "AnsiReinit";
#endif
    Window *	wp;


#ifdef	DEBUG
    Debug (SETTERM) ("%s (%s) started.", fname, term);
#endif

    if (*term)
    {
      switch (tgetent( tcap_data, term ))	/* get termcap entry	*/
    	{
   	case -1 :
#ifdef	DEBUG
	    Debug (ERROR) ("%s : unable to open termcap database.", fname);
#endif
	    return FALSE;
    	case  0 :
#ifdef	DEBUG
	    Debug (ERROR) ("%s : Terminal %s not found in termcap database.",
	    	fname, term);
#endif
	    return FALSE;
	}
    }

    TermInit();				/* initialise terminal output	*/
    InputInit();			/* initialise terminal input	*/

    wp = Cur_Window;

    do
    {
    	AnsiReinitScreen( &wp->Screen );
	
    	wp->Attribs.Min  = wp->Screen.Rows;
    	wp->Attribs.Time = wp->Screen.Cols;
	
#ifdef	DEBUG
    	Debug (ATTR | SETTERM) ("New window size Rows : %d, Cols : %d.", wp->Attribs.Min, wp->Attribs.Time);
#endif
    	wp = wp->Next;
    }
    while (wp != Cur_Window);
    
#ifdef	DEBUG
    Debug (SETTERM) ("%s (%s) ready.", fname, term);
#endif

    return TRUE;
}


/*************************************************************************
 * INITIALISE A SCREEN STRUCTURE
 *
 * - Allocate and initialise the screen map.
 * - Clear the map.
 * - Initialise the screen variables.
 *
 * Parameter  :	screen	= pointer to screen structure
 * Return     :	TRUE if everything was ok
 *		FALSE if an error occured
 *
 ************************************************************************/

word
AnsiInitScreen (Screen *s)
{
    char **	map;
    char **	tempmap;
    char *	screen;
    int		i;

    
    if ((map = (char **) Malloc ((word) term_rows * sizeof (char *))) == NULL)
	return FALSE;

    if ((tempmap = (char **) Malloc ((word) term_rows * sizeof (char *))) == NULL)
    {
    	Free (map);
	
	return FALSE;
    }

    if ((screen = (char *) Malloc ((word) term_rows * (word) term_cols)) == NULL)
    {
	Free (map);
	Free (tempmap);
	
	return FALSE;
    }

    memset (screen, ' ', term_rows * term_cols);
    
    map[0] = screen;
    
    for (i = 1; i < term_rows; i++)
	map[i] = map[i - 1] + term_cols;

    s->map     = map;
    s->screen  = screen;
    s->tempmap = tempmap;
    s->Cur_x   = 0;
    s->Cur_y   = 0;
    s->flags   = 0;
    s->mode    = ANSI_mode_normal;
    s->Rows    = term_rows;
    s->Cols    = term_cols;
    
    AnsiResetEscape (s);
    
    return TRUE;
}


/*************************************************************************
 * TIDY UP A SCREEN STRUCTURE
 *
 * - Free the screen maps.
 *
 * Parameter  :	screen	= pointer to screen structure
 * Return     :	- nothing -
 *
 ************************************************************************/

void
AnsiTidyScreen (Screen *s)
{
    Free (s->map);
    Free (s->tempmap);
    Free (s->screen);
}

/*************************************************************************
 * REINITIALISE A SCREEN STRUCTURE
 *
 * - Allocate and initialise a new screen map, 
 *   copy the old screen contents and free the old map.
 * - Initialise the screen variables.
 *
 * Parameter  :	screen	= pointer to screen structure
 * Return     :	TRUE if everything was ok
 *		FALSE if an error occured
 *
 ************************************************************************/

word
AnsiReinitScreen (Screen *s)
{
    char **	map;
    char **	tempmap;
    char *	screen;
    int		i, crows, ccols;


					/* Did the screen size change ?	*/
    if (s->Rows == term_rows && s->Cols == term_cols)
    	return TRUE;
    	
					/* Allocate a new line map,	*/
    if ((map = (char **) Malloc( (word) term_rows * sizeof (char *) )) == NULL)
	return FALSE;
					/* a new temp map		*/
    if ((tempmap = (char **) Malloc( (word) term_rows * sizeof (char *) )) == NULL)
    {
    	Free( map );
	
	return FALSE;
    }
					/* and a new screen buffer.	*/
    if ((screen = (char *) Malloc( (word) term_rows * (word) term_cols )) == NULL)
    {
	Free( map );
	Free( tempmap );
	
	return FALSE;
    }
					/* Clear the new screen buffer.	*/
    memset( screen, ' ', term_rows * term_cols );

    map[ 0 ] = screen;			/* Initialise the line map.	*/

    for (i = 1; i < term_rows; i++)
	map[ i ] = map[ i - 1 ] + term_cols;
    
					/* Set the copy area.		*/
    crows = (term_rows > s->Rows) ? s->Rows : term_rows;
    ccols = (term_cols > s->Cols) ? s->Cols : term_cols;

    					/* Copy the used lines to the	*/
    for (i = 0; i < crows; i++)		/* new screen.			*/
    	memcpy( map[ i ], s->map[ i ], ccols );

    Free( s->map );			/* Free the old line map	*/
    Free( s->tempmap );			/* the old tempmap		*/
    Free( s->screen );			/* and the old screen buffer,	*/
    
    s->map     = map;			/* then put the new line map,	*/
    s->tempmap = tempmap;		/* the new tempmap		*/
    s->screen  = screen;		/* and the new buffer into the 	*/
    s->Rows    = term_rows;		/* screen structure.		*/
    s->Cols    = term_cols;

    return TRUE;
}

/*--- end of ansi.c ---*/




