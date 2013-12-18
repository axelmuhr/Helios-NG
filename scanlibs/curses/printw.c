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
static char sccsid[] = "@(#)printw.c	5.4 (Berkeley) 6/30/88";
#endif /* not lint */

/*
 * printw and friends
 *
 */

# include	"curses.ext"
# ifdef __STDC__
# include	<stdarg.h>
# endif

/*
 *	This routine implements a printf on the standard screen.
 */
#ifdef __STDC__
int	printw(char *fmt, ...)
#else
int	printw(fmt, args)
char	*fmt;
int	args;
#endif
{

	char	buf[512];
# ifdef __STDC__
	va_list ap;

	va_start(ap,fmt);
	(void) vsprintf(buf, fmt, ap);
	va_end(ap);
# else
	(void) vsprintf(buf, fmt, &args);
# endif
	return waddstr(stdscr, buf);
}

/*
 *	This routine implements a printf on the given window.
 */
#ifdef __STDC__
int	wprintw(WINDOW *win, char *fmt, ...)
#else
int	wprintw(win, fmt, args)
WINDOW	*win;
char	*fmt;
int	args;
#endif
{

	char	buf[512];
# ifdef __STDC__
	va_list ap;

	va_start(ap,fmt);
	(void) vsprintf(buf, fmt, ap);
	va_end(ap);
# else
	(void) vsprintf(buf, fmt, &args);
# endif
	return waddstr(win, buf);
}
