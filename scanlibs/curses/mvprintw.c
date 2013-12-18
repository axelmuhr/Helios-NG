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
static char sccsid[] = "@(#)mvprintw.c	5.4 (Berkeley) 6/30/88";
#endif /* not lint */

# include	"curses.ext"
# ifdef __STDC__
# include	<stdarg.h>
# endif

/*
 * implement the mvprintw commands.  Due to the variable number of
 * arguments, they cannot be macros.  Sigh....
 *
 */
#ifdef __STDC__
int	mvprintw(int y, int x, char *fmt, ...)
#else
int mvprintw(y, x, fmt, args)
reg int		y, x;
char		*fmt;
int		args; 
#endif
{

	char	buf[512];
#ifdef __STDC__
	va_list	ap;
#endif

	if (move(y, x) != OK)
		return ERR;
# ifdef __STDC__
	va_start(ap,fmt);
	(void) vsprintf(buf, fmt, ap);
	va_end(ap);
# else
	(void) vsprintf(buf, fmt, &args);
# endif
	return waddstr(stdscr, buf);
}

#ifdef __STDC__
int	mvwprintw(WINDOW *win, int y, int x, char *fmt, ...)
#else
int mvwprintw(win, y, x, fmt, args)
reg WINDOW	*win;
reg int		y, x;
char		*fmt;
int		args;
#endif
{

	char	buf[512];
# ifdef __STDC__
	va_list	ap;
# endif

	if (move(y, x) != OK)
		return ERR;
# ifdef __STDC__
	va_start(ap,fmt);
	(void) vsprintf(buf, fmt, ap);
	va_end(ap);
# else
	(void) vsprintf(buf, fmt, &args);
# endif
	return waddstr(win, buf);
}
