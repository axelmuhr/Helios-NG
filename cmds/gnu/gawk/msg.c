/*
 * msg.c - routines for error messages
 */

/* 
 * Copyright (C) 1986, 1988, 1989, 1991 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Progamming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 1, or (at your option)
 * any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GAWK; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "awk.h"

int sourceline = 0;
char *source = NULL;

/* VARARGS2 */
static void
err(s, msg, argp)
char *s;
char *msg;
va_list *argp;
{
	char *file;

	(void) fflush(stdout);
	(void) fprintf(stderr, "%s: %s ", myname, s);
	vfprintf(stderr, msg, *argp);
	(void) fprintf(stderr, "\n");
	if (FNR) {
		(void) fprintf(stderr, "  input line number %d", FNR);
		file = FILENAME_node->var_value->stptr;
		if (file && !STREQ(file, "-"))
			(void) fprintf(stderr, ", file `%s'", file);
		(void) fprintf(stderr, "\n");
	}
	if (sourceline) {
		(void) fprintf(stderr, "  source line number %d", sourceline);
		if (source)
			(void) fprintf(stderr, ", file `%s'", source);
		(void) fprintf(stderr, "\n");
	}
	(void) fflush(stderr);
}

/*VARARGS0*/
void
msg(char * mesg, ... )
{
	va_list args;

	va_start(args, mesg);
	err("", mesg, &args);
	va_end(args);
}

/*VARARGS0*/
void
warning(char * mesg, ... )
{
	va_list args;

	va_start(args, mesg);

	err("warning:", mesg, &args);
	va_end(args);
}

/*VARARGS0*/
void
fatal(char * mesg, ... )
{
	va_list args;

	va_start(args, mesg);

	err("fatal error:", mesg, &args);
	va_end(args);
#ifdef DEBUG
	abort();
#endif
	exit(2);
}
