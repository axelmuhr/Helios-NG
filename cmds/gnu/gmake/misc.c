/* Copyright (C) 1988, 1989 Free Software Foundation, Inc.
This file is part of GNU Make.

GNU Make is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GNU Make is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Make; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "make.h"
#include "dep.h"

static char *rcsid = "$Header: /dsl/HeliosRoot/Helios/cmds/gnu/gmake/RCS/misc.c,v 1.2 1992/06/25 10:58:33 nickc Exp $";

/* Compare strings *S1 and *S2.
   Return negative if the first is less, positive if it is greater,
   zero if they are equal.  */

int
alpha_compare (s1, s2)
     char **s1, **s2;
{
  if (**s1 != **s2)
    return **s1 - **s2;
  return strcmp (*s1, *s2);
}

/* Discard each backslash-newline combination from LINE.
   Backslash-backslash-newline combinations become backslash-newlines.
   This is done by copying the text at LINE into itself.  */

void
collapse_continuations (line)
     char *line;
{
  register char *in, *out, *p;
  register int backslash;
  register unsigned int bs_write;

  in = index (line, '\n');
  if (in == 0)
    return;

  out = in;
  if (out > line)
    while (out[-1] == '\\')
      --out;

  while (*in != '\0')
    {
      /* BS_WRITE gets the number of quoted backslashes at
	 the end just before IN, and BACKSLASH gets nonzero
	 if the next character is quoted.  */
      backslash = 0;
      bs_write = 0;
      for (p = in - 1; p >= line && *p == '\\'; --p)
	{
	  if (backslash)
	    ++bs_write;
	  backslash = !backslash;

	  /* It should be impossible to go back this far without exiting,
	     but if we do, we can't get the right answer.  */
	  if (in == out - 1)
	    abort ();
	}

      /* Output the appropriate number of backslashes.  */
      while (bs_write-- > 0)
	*out++ = '\\';

      /* Skip the newline.  */
      ++in;

      /* If the newline is quoted, discard following whitespace
	 and any preceding whitespace; leave just one space.  */
      if (backslash)
	{
	  in = next_token (in);
	  while (out > line && (out[-1] == ' ' || out[-1] == '\t'))
	    --out;
	  *out++ = ' ';
	}
      else
	/* If the newline isn't quoted, put it in the output.  */
	*out++ = '\n';

      /* Now copy the following line to the output.
	 Stop when we find backslashes followed by a newline.  */
      while (*in != '\0')
	if (*in == '\\')
	  {
	    p = in + 1;
	    while (*p == '\\')
	      ++p;
	    if (*p == '\n')
	      {
		in = p;
		break;
	      }
	    while (in < p)
	      *out++ = *in++;
	  }
	else
	  *out++ = *in++;
    }

  *out = '\0';
}


/* Remove comments, backslash-newline combinations and whitespace
   following such combinations from LINE.
   This is done by copying the text at LINE into itself.  */

void
collapse_line (line)
     char *line;
{
  register char *p, *p2;
  register int backslash;
  register unsigned int bs_write;

  /* Collapse backslash-newline continuations.  */
  collapse_continuations (line);

  while (1)
    {
      p = index (line, '#');
      if (p == 0)
	break;

      backslash = 0;
      bs_write = 0;
      for (p2 = p - 1; p2 > line && *p2 == '\\'; --p2)
	{
	  if (backslash)
	    ++bs_write;
	  backslash = !backslash;
	}

      if (!backslash)
	{
	  /* Cut off the line at the #.  */
	  *p = '\0';
	  break;
	}

      /* strcpy better copy left to right.  */
      line = p;
      strcpy (p2 + 1 + bs_write, line);
    }
}

/* Print N spaces (used by DEBUGPR for target-depth).  */

void
print_spaces (n)
     register unsigned int n;
{
  while (n-- > 0)
    putchar (' ');
}


/* Return a newly-allocated string whose contents
   concatenate those of s1, s2, s3.  */

char *
concat (s1, s2, s3)
     register char *s1, *s2, *s3;
{
  register unsigned int len1, len2, len3;
  register char *result;

  len1 = *s1 != '\0' ? strlen (s1) : 0;
  len2 = *s2 != '\0' ? strlen (s2) : 0;
  len3 = *s3 != '\0' ? strlen (s3) : 0;

  result = (char *) xmalloc (len1 + len2 + len3 + 1);

  if (*s1 != '\0')
    bcopy (s1, result, len1);
  if (*s2 != '\0')
    bcopy (s2, result + len1, len2);
  if (*s3 != '\0')
    bcopy (s3, result + len1 + len2, len3);
  *(result + len1 + len2 + len3) = '\0';

  return result;
}

/* Print an error message and exit.  */

#ifndef __STDC__
/* VARARGS1 */
void
fatal (s1, s2, s3, s4, s5, s6)
     char *s1, *s2, *s3, *s4, *s5, *s6;
{
  if (makelevel == 0)
    fprintf (stderr, "%s: ", program);
  else
    fprintf (stderr, "%s[%u]: ", program, makelevel);
  fprintf (stderr, s1, s2, s3, s4, s5, s6);
  fputs (".  Stop.\n", stderr);

  die (1);
}

/* Print error message.  `s1' is printf control string, `s2' is arg for it. */

/* VARARGS1 */

void
error (s1, s2, s3, s4, s5, s6)
     char *s1, *s2, *s3, *s4, *s5, *s6;
{
  if (makelevel == 0)
    fprintf (stderr, "%s: ", program);
  else
    fprintf (stderr, "%s[%u]: ", program, makelevel);
  fprintf (stderr, s1, s2, s3, s4, s5);
  putc ('\n', stderr);
  fflush (stderr);
}
#else
#include <stdarg.h>
void
fatal (char * s1, ... )
{ va_list ap;
  va_start(ap,s1);
  if (makelevel == 0)
    fprintf (stderr, "%s: ", program);
  else
    fprintf (stderr, "%s[%u]: ", program, makelevel);
  vfprintf (stderr, s1, ap);
  fputs (".  Stop.\n", stderr);

  die (1);
}

void
error (char * s1, ... )
{ va_list ap;
  va_start(ap,s1);
  if (makelevel == 0)
    fprintf (stderr, "%s: ", program);
  else
    fprintf (stderr, "%s[%u]: ", program, makelevel);
  vfprintf (stderr, s1, ap);
  putc ('\n', stderr);
  fflush (stderr);
  va_end(ap);
}
#endif
/* Print an error message from errno.  */

void
perror_with_name (str, name)
     char *str, *name;
{
#ifndef HELIOS
  extern int errno, sys_nerr;
  extern char *sys_errlist[];

  if (errno < sys_nerr)
    error ("%s%s: %s", str, name, sys_errlist[errno]);
  else
    error ("%s%s: Unknown error %d", str, name, errno);
#else
    error ("%s%s: posix error no. %d", str, name, errno);
#endif
}

/* Print an error message from errno and exit.  */

void
pfatal_with_name (name)
     char *name;
{
#ifndef HELIOS
  extern int errno, sys_nerr;
  extern char *sys_errlist[];

  if (errno < sys_nerr)
    fatal ("%s: %s", name, sys_errlist[errno]);
  else
    fatal ("%s: Unknown error %d", name, errno);
#else
    fatal ("%s: posix error no. %d", name, errno);
#endif

  /* NOTREACHED */
}

/* Like malloc but get fatal error if memory is exhausted.  */
extern char *malloc (), *realloc ();

char *
xmalloc (size)
     unsigned int size;
{
  char *result = malloc (size);
  if (result == 0)
    fatal ("virtual memory exhausted");
  return result;
}


char *
xrealloc (ptr, size)
     char *ptr;
     unsigned int size;
{
  char *result = realloc (ptr, size);
  if (result == 0)
    fatal ("virtual memory exhausted");
  return result;
}

char *
savestring (str, length)
     char *str;
     unsigned int length;
{
  register char *out = (char *) xmalloc (length + 1);
  if (length > 0)
    bcopy (str, out, length);
  out[length] = '\0';
  return out;
}

/* Search string BIG (length BLEN) for an occurrence of
   string SMALL (length SLEN).  Return a pointer to the
   beginning of the first occurrence, or return nil if none found.  */

char *
sindex (big, blen, small, slen)
     char *big;
     unsigned int blen;
     char *small;
     unsigned int slen;
{
  register unsigned int b;

  if (blen < 1)
    blen = strlen (big);
  if (slen < 1)
    slen = strlen (small);

  for (b = 0; b < blen; ++b)
    if (big[b] == *small && !strncmp (&big[b + 1], small + 1, slen - 1))
      return (&big[b]);

  return 0;
}

/* Limited INDEX:
   Search through the string STRING, which ends at LIMIT, for the character C.
   Returns a pointer to the first occurrence, or nil if none is found.
   Like INDEX except that the string searched ends where specified
   instead of at the first null.  */

char *
lindex (s, limit, c)
     register char *s, *limit;
     int c;
{
  while (s < limit)
    if (*s++ == c)
      return s - 1;

  return 0;
}

/* Return the address of the first whitespace or null in the string S.  */

char *
end_of_token (s)
     char *s;
{
  register char *p = s;
  register int backslash = 0;

  while (*p != '\0' && (backslash || (*p != ' ' && *p != '\t' && *p != '\f')))
    {
      if (*p++ == '\\')
	{
	  backslash = !backslash;
	  while (*p == '\\')
	    {
	      backslash = !backslash;
	      ++p;
	    }
	}
      else
	backslash = 0;
    }

  return p;
}

/* Return the address of the first nonwhitespace or null in the string S.  */

char *
next_token (s)
     char *s;
{
  register char *p = s;

  while (*p == ' ' || *p == '\t' || *p == '\f')
    ++p;
  return p;
}

/* Find the next token in PTR; return the address of it, and store the
   length of the token into *LENGTHPTR if LENGTHPTR is not nil.  */

char *
find_next_token (ptr, lengthptr)
     char **ptr;
     unsigned int *lengthptr;
{
  char *p = next_token (*ptr);
  char *end;

  if (*p == '\0')
    return 0;

  *ptr = end = end_of_token (p);
  if (lengthptr != 0)
    *lengthptr = end - p;
  return p;
}

/* Copy a chain of `struct dep', making a new chain
   with the same contents as the old one.  */

struct dep *
copy_dep_chain (d)
     register struct dep *d;
{
  register struct dep *c;
  struct dep *firstnew = 0;
  struct dep *lastnew;

  while (d != 0)
    {
      c = (struct dep *) xmalloc (sizeof (struct dep));
      bcopy ((char *) d, (char *) c, sizeof (struct dep));
      if (c->name != 0)
	c->name = savestring (c->name, strlen (c->name));
      c->next = 0;
      if (firstnew == 0)
	firstnew = lastnew = c;
      else
	lastnew = lastnew->next = c;

      d = d->next;
    }

  return firstnew;
}

#ifdef	iAPX286
/* The losing compiler on this machine can't handle this macro.  */

char *
dep_name (dep)
     struct dep *dep;
{
  return dep->name == 0 ? dep->file->name : dep->name;
}
#endif
