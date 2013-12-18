/*
 * Copyright (c) 1981 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifdef lint
static char sccsid[] = "@(#)scanw.c	5.3 (Berkeley) 6/30/88";
#endif /* not lint */

/*
 * scanw and friends
 *
 */

# include	"curses.ext"
# ifdef __STDC__
# include	<stdarg.h>
int  _sscans(WINDOW *win, char *fmt, ...);
# endif

#include <string.h> /* for strlen() */
		       
/*
 *	This routine implements a scanf on the standard screen.
 */
# ifdef __STDC__
int 	scanw(char *fmt, ...) {
        va_list ap;
	int nargs;

        va_start(ap,fmt);
	nargs = _sscans(stdscr, fmt, ap);
        va_end(ap);
	return (nargs);
}
# else
scanw(fmt, args)
char	*fmt;
int	args; {
	return _sscans(stdscr, fmt, &args);
}
# endif

/*
 *	This routine implements a scanf on the given window.
 */
 
#ifdef __STDC__
int	wscanw(WINDOW *win, char *fmt, ...) {
        va_list ap;
        int nargs;

        va_start(ap,fmt);
        nargs = _sscans(win, fmt, ap);
        va_end(ap);
        return (nargs);
}
#else
int wscanw(win, fmt, args)
WINDOW	*win;
char	*fmt;
int	args; {
	return _sscans(win, fmt, &args);
}
# endif

/*
 *	This routine actually executes the scanf from the window.
 *
 *	This is really a modified version of "sscanf".  As such,
 * it assumes that sscanf interfaces with the other scanf functions
 * in a certain way.  If this is not how your system works, you
 * will have to modify this routine to use the interface that your
 * "sscanf" uses.
 */
#ifdef __STDC__
int  _sscans(WINDOW *win, char *fmt, ...)
#else
int _sscans(win, fmt, args)
WINDOW	*win;
char	*fmt;
int	*args;
#endif
{
	char	buf[100];
	FILE	junk;
#ifdef __STDC__
        va_list ap;
        int nargs;
#endif

	junk._flag = _IOREAD|_IOSTRG;
	junk._base = junk._ptr = (unsigned char *)buf;
	if (wgetstr(win, buf) == ERR)
		return ERR;
	junk._icnt = strlen(buf);
	junk._ocnt = strlen(buf);
#ifdef __HELIOS
#ifdef __STDC__
        va_start(ap,fmt);
        nargs = fscanf(&junk, fmt, ap);
        va_end(ap);
        return (nargs);
#else
	return fscanf(&junk, fmt, args);
#endif
#else
#ifdef __STDC__
        va_start(ap,fmt);
        nargs = _doscan(&junk, fmt, ap);
        va_end(ap);
        return (nargs);
# else
	return _doscan(&junk, fmt, args);
#endif
#endif
}
