/* Context-format output routines for GNU DIFF.
   Copyright (C) 1988, 1989 Free Software Foundation, Inc.

This file is part of GNU DIFF.

GNU DIFF is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

GNU DIFF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU DIFF; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "diff.h"
#include "regex.h"

static char *rcsid = "$Header: /usr/perihelion/Helios/cmds/gnu/diff/RCS/context.c,v 1.1 90/08/28 12:27:57 james Exp $";

static void pr_context_hunk ();
static struct change *find_hunk ();
static void mark_ignorable ();
static int find_function ();

/* Last place find_function started searching from.  */
static int find_function_last_search;

/* The value find_function returned when it started searching there.  */
static int find_function_last_match;

/* Print a header for a context diff, with the file names and dates.  */

void
print_context_header (inf)
     struct file_data *inf;
{
  fprintf (outfile, "*** %s\t%s", inf[0].name,
	   ctime (&inf[0].stat.st_mtime));
  fprintf (outfile, "--- %s\t%s", inf[1].name,
	   ctime (&inf[1].stat.st_mtime));
}

/* Print an edit script in context format.  */

void
print_context_script (script)
     struct change *script;
{
  if (ignore_blank_lines_flag || ignore_regexp)
    mark_ignorable (script);
  else
    {
      struct change *e;
      for (e = script; e; e = e->link)
	e->ignore = 0;
    }

  find_function_last_search = 0;
  find_function_last_match = -1;

  print_script (script, find_hunk, pr_context_hunk);
}

/* Print a pair of line numbers with a comma, translated for file FILE.
   If the second number is smaller, use the first in place of it.

   Args A and B are internal line numbers.
   We print the translated (real) line numbers.  */

static void
print_context_number_range (file, a, b)
     struct file_data *file;
     int a, b;
{
  int trans_a, trans_b;
  translate_range (file, a, b, &trans_a, &trans_b);

  /* Note: we can have B < A in the case of a range of no lines.
     In this case, we should print the line number before the range,
     which is B.  */
  if (trans_b >= trans_a)
    fprintf (outfile, "%d,%d", trans_a, trans_b);
  else
    fprintf (outfile, "%d", trans_b);
}

/* Print a portion of an edit script in context format.
   HUNK is the beginning of the portion to be printed.
   The end is marked by a `link' that has been nulled out.

   Prints out lines from both files, and precedes each
   line with the appropriate flag-character.  */

static void
pr_context_hunk (hunk)
     struct change *hunk;
{
  int first0, last0, first1, last1, show_from, show_to, i;
  struct change *next;
  char *prefix;
  int lastline;
  char *function;
  int function_length;

  /* Determine range of line numbers involved in each file.  */

  analyze_hunk (hunk, &first0, &last0, &first1, &last1, &show_from, &show_to);

  if (!show_from && !show_to)
    return;

  /* Include a context's width before and after.  */

  first0 = max (first0 - context, 0);
  first1 = max (first1 - context, 0);
  last0 = min (last0 + context, files[0].buffered_lines - 1);
  last1 = min (last1 + context, files[1].buffered_lines - 1);

  /* If desired, find the preceding function definition line in file 0.  */
  function = 0;
  if (function_regexp)
    find_function (&files[0], first0, &function, &function_length);

  /* If we looked for and found a function this is part of,
     include its name in the header of the diff section.  */
  fprintf (outfile, "***************");

  if (function)
    {
      fprintf (outfile, " ");
      fwrite (function, 1, min (function_length - 1, 40), outfile);
    }

  fprintf (outfile, "\n*** ");
  print_context_number_range (&files[0], first0, last0);
  fprintf (outfile, " ****\n");

  if (show_from)
    {
      next = hunk;

      for (i = first0; i <= last0; i++)
	{
	  /* Skip past changes that apply (in file 0)
	     only to lines before line I.  */

	  while (next && next->line0 + next->deleted <= i)
	    next = next->link;

	  /* Compute the marking for line I.  */

	  prefix = " ";
	  if (next && next->line0 <= i)
	    /* The change NEXT covers this line.
	       If lines were inserted here in file 1, this is "changed".
	       Otherwise it is "deleted".  */
	    prefix = (next->inserted > 0 ? "!" : "-");

	  print_1_line (prefix, &files[0].linbuf[i]);
	}
    }

  fprintf (outfile, "--- ");
  print_context_number_range (&files[1], first1, last1);
  fprintf (outfile, " ----\n");

  if (show_to)
    {
      next = hunk;

      for (i = first1; i <= last1; i++)
	{
	  /* Skip past changes that apply (in file 1)
	     only to lines before line I.  */

	  while (next && next->line1 + next->inserted <= i)
	    next = next->link;

	  /* Compute the marking for line I.  */

	  prefix = " ";
	  if (next && next->line1 <= i)
	    /* The change NEXT covers this line.
	       If lines were deleted here in file 0, this is "changed".
	       Otherwise it is "inserted".  */
	    prefix = (next->deleted > 0 ? "!" : "+");

	  print_1_line (prefix, &files[1].linbuf[i]);
	}
    }
}

/* Scan a (forward-ordered) edit script for the first place that at least
   2*CONTEXT unchanged lines appear, and return a pointer
   to the `struct change' for the last change before those lines.  */

static struct change *
find_hunk (start)
     struct change *start;
{
  struct change *prev;
  int top0, top1;
  int thresh;

  do
    {
      /* Computer number of first line in each file beyond this changed.  */
      top0 = start->line0 + start->deleted;
      top1 = start->line1 + start->inserted;
      prev = start;
      start = start->link;
      /* Threshold distance is 2*CONTEXT between two non-ignorable changes,
	 but only CONTEXT if one is ignorable.  */
      thresh = ((prev->ignore || (start && start->ignore))
		? context
		: 2 * context);
      /* It is not supposed to matter which file we check in the end-test.
	 If it would matter, crash.  */
      if (start && start->line0 - top0 != start->line1 - top1)
	abort ();
    } while (start
	     /* Keep going if less than THRESH lines
		elapse before the affected line.  */
	     && start->line0 < top0 + thresh);

  return prev;
}

/* Set the `ignore' flag properly in each change in SCRIPT.
   It should be 1 if all the lines inserted or deleted in that change
   are ignorable lines.  */

static void
mark_ignorable (script)
     struct change *script;
{
  while (script)
    {
      struct change *next = script->link;
      int first0, last0, first1, last1, deletes, inserts;

      /* Turn this change into a hunk: detach it from the others.  */
      script->link = 0;

      /* Determine whether this change is ignorable.  */
      analyze_hunk (script, &first0, &last0, &first1, &last1, &deletes, &inserts);
      /* Reconnect the chain as before.  */
      script->link = next;

      /* If the change is ignorable, mark it.  */
      script->ignore = (!deletes && !inserts);

      /* Advance to the following change.  */
      script = next;
    }
}

/* Find the last function-header line in FILE prior to line number LINENUM.
   This is a line containing a match for the regexp in `function_regexp'.
   Store the address of the line text into LINEP and the length of the
   line into LENP.
   Do not store anything if no function-header is found.  */

static int
find_function (file, linenum, linep, lenp)
     struct file_data *file;
     int linenum;
     char **linep;
     int *lenp;
{
  int i = linenum;
  int last = find_function_last_search;
  find_function_last_search = i;

  while (--i >= last)
    {
      /* See if this line is what we want.  */

      if (0 <= re_search (&function_regexp_compiled,
			  files[0].linbuf[i].text,
			  files[0].linbuf[i].length,
			  0, files[0].linbuf[i].length,
			  0))
	{
	  *linep = files[0].linbuf[i].text;
	  *lenp = files[0].linbuf[i].length;
	  find_function_last_match = i;
	  return 1;
	}
    }
  /* If we search back to where we started searching the previous time,
     find the line we found last time.  */
  if (find_function_last_match >= 0)
    {
      i = find_function_last_match;
      *linep = files[0].linbuf[i].text;
      *lenp = files[0].linbuf[i].length;
      return 1;
    }
  return 0;
}
