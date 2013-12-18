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
static char sccsid[] = "@(#)mvscanw.c	5.3 (Berkeley) 6/30/88";
#endif /* not lint */

# include	"curses.ext"
#ifdef __STDC__
#include <stdarg.h>
#endif

/*
 * implement the mvscanw commands.  Due to the variable number of
 * arguments, they cannot be macros.  Another sigh....
 *
 */

#ifdef __STDC__
int	mvscanw(int y, int x, char *fmt, ...)
#else
int	mvscanw(y, x, fmt, args)
reg int		y, x;
char		*fmt;
int		args;
#endif
{
#ifdef __STDC__
        va_list ap;
        int result;
        
        va_start(ap,fmt);
        result = move(y, x) == OK ? _sscans(stdscr, fmt, ap) : ERR;
        va_end(ap);
        return (result);
#else
	return move(y, x) == OK ? _sscans(stdscr, fmt, &args) : ERR;
#endif
}

#ifdef __STDC__
int	mvwscanw(WINDOW *win, int y, int x, char *fmt, ...)
#else
int	mvwscanw(win, y, x, fmt, args)
reg WINDOW	*win;
reg int		y, x;
char		*fmt;
int		args;
#endif
{
#ifdef __STDC__
        va_list ap;
        int result;

        va_start(ap,fmt);
        result = wmove(win, y, x) == OK ? _sscans(win, fmt, ap) : ERR;
        va_end(ap);
        return (result);
#else
	return wmove(win, y, x) == OK ? _sscans(win, fmt, &args) : ERR;
#endif
}
