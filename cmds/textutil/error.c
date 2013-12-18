/* error.c -- error handler for noninteractive utilities
   Copyright (C) 1990, 1991 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* David MacKenzie */

#include <stdio.h>
#include <stdlib.h>

#ifndef VPRINTF_MISSING

#ifdef __STDC__
#include <stdarg.h>
#define VA_START(args, lastarg) va_start(args, lastarg)
#else
#include <varargs.h>
#define VA_START(args, lastarg) va_start(args)
#endif

#else

#ifndef DOPRNT_MISSING
#define va_alist args
#define va_dcl int args;
#else
#define va_alist a1, a2, a3, a4, a5, a6, a7, a8
#define va_dcl char *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8;
#endif

#endif

#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#else

static char *
strerror (int errnum )
{
  extern char *sys_errlist[];
  extern int sys_nerr;

  if (errnum > 0 && errnum < sys_nerr)
    return sys_errlist[errnum];
  return "Unknown system error";
}
#endif

/* Print the program name and error message MESSAGE, which is a printf-style
   format string with optional args.
   If ERRNUM is nonzero, print its corresponding system error message.
   Exit with status STATUS if it is nonzero. */
/* VARARGS */
void
#if !defined (VPRINTF_MISSING) && defined (__STDC__)
error (int status, int errnum, char *message, ...)
#else
error (status, errnum, message, va_alist)
     int status;
     int errnum;
     char *message;
     va_dcl
#endif
{
  extern char *program_name;
#ifndef VPRINTF_MISSING
  va_list args;
#endif

  fprintf (stderr, "%s: ", program_name);
#ifndef VPRINTF_MISSING
  VA_START (args, message);
  vfprintf (stderr, message, args);
  va_end (args);
#else
#ifndef DOPRNT_MISSING
  _doprnt (message, &args, stderr);
#else
  fprintf (stderr, message, a1, a2, a3, a4, a5, a6, a7, a8);
#endif
#endif
  if (errnum)
    fprintf (stderr, ": %s", strerror (errnum));
  putc ('\n', stderr);
  fflush (stderr);
  if (status)
    exit (status);
}
