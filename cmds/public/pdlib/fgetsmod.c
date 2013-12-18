#ifndef lint
static char *RCSid = "$Header: fgetsmod.c,v 1.2 87/08/21 16:43:10 rnovak Exp $";
#endif

/*
 *------------------------------------------------------------------
 *
 * $Source: /u3/syseng/rnovak/src/lib/RCS/fgetsmod.c,v $
 * $Revision: 1.2 $
 * $Date: 87/08/21 16:43:10 $
 * $State: Exp $
 * $Author: rnovak $
 * $Locker:  $
 *
 *------------------------------------------------------------------
 * $Log:	fgetsmod.c,v $
 * Revision 1.2  87/08/21  16:43:10  rnovak
 * ran this through cb
 * 
 * Revision 1.1  87/08/21  16:33:02  rnovak
 * Initial revision
 * 
 *------------------------------------------------------------------
 */

#ifndef lint
static char rcsid[] = "$Header: fgetsmod.c,v 1.2 87/08/21 16:43:10 rnovak Exp $";
#endif

#include	<stdio.h>

#define TOOLONG -2

/* New and improved version of fgets.  Unlike original, eats up extra chars.
 * fgets1 will read n - 1 characters, or up to a new line, whichever
 * comes first, from stream iop into string s.  The last character read
 * into s is followed by a null character.
 *
 * It deals with all possibilities.  If line ends with newline or have
 * isolated EOF, no problem.  Otherwise, it will insert a newline and eat
 * any excess characters.  Hence guarantees line ending with newline
 * followed by null.
 *
 * It returns:
 *   1.  NULL at end of file, for compatible with fgets.
 *   2.  TOOLONG if line is too long.
 *       This is usable as a warning.
 *   3.  Length of line, excluding null (like strlen), otherwise.
 *       This is useful in the usual case when line is read uneventfully.
 */

fgetsmod(s, n, iop)
char *s;
register FILE *iop;
{
	register c;
	register char *cs;

	cs = s;

	while (--n > 0 && (c = getc(iop)) != EOF) {
		*cs++ = c;
		if (c == '\n')
			break;
	}

	if (c == '\n') {	/* normal ending, commonest case */
		*cs = '\0';
		return (cs - s);
	}

	if ((c == EOF) && (cs == s))  /* isolated EOF, second commonest case */
		return (NULL);

	if (n == 0) {	/* line too long */
		*cs = '\0';
		*(--cs) = '\n';	/* put in missing newline */
		while ((c = getc(iop)) != EOF && c != '\n')	/* eat up extra chars */
			;
		return (TOOLONG);
	}

	if (c == EOF) {	/* final line has no newline -- rare */
		*cs++ = '\n';
		*cs = '\0';
		return (cs - s);	/* pretend all was OK */
	}
	/* NOTREACHED */
}
