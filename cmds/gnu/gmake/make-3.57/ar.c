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
#include "file.h"

static char *rcsid = "$Header: /usr/perihelion/Helios/cmds/gnu/gmake/make-3.57/RCS/ar.c,v 1.1 90/08/28 14:29:40 james Exp $";

/* Defined in arscan.c.  */
extern long int ar_scan ();
extern int ar_member_touch ();


/* Return nonzero if NAME is an archive-member reference, zero if not.
   An archive-member reference is a name like `lib(member)'.
   If a name like `lib((entry))' is used, a fatal error is signaled at
   the attempt to use this unsupported feature.  */

int
ar_name (name)
     char *name;
{
  char *p = index (name, '('), *end = name + strlen (name) - 1;
  
  if (p == 0 || p == name || *end != ')')
    return 0;

  if (p[1] == '(' && end[-1] == ')')
    fatal ("attempt to use unsupported feature: `%s'", name);

  return 1;
}

static long int ar_member_date_1 ();

/* Return the modtime of NAME.  */

time_t
ar_member_date (name)
     char *name;
{
  char *arname;
  char *memname;
  char *p;
  long int val;

  /* This "file" is an archive member.  */
  p = index (name, '(');
  arname = savestring (name, p - name);
  val = strlen (p) - 2;
  if (val > 15)
    val = 15;
  memname = savestring (p + 1, val);
  p = rindex (memname, ')');
  if (p != 0)
    *p = '\0';

  /* Make sure we know the modtime of the archive itself because
     we are likely to be called just before commands to remake a
     member are run, and they will change the archive itself.  */
  (void) f_mtime (enter_file (arname));

  val = ar_scan (arname, ar_member_date_1, (long int) memname);

  free (arname);
  free (memname);
  return (val <= 0 ? (time_t) -1 : (time_t) val);
}

/* This function is called by `ar_scan' to find which member to look at.  */

/* ARGSUSED */
static long int
ar_member_date_1 (desc, name, hdrpos, datapos, size, date, uid, gid, mode, mem)
     int desc;
     char *name;
     long int hdrpos, datapos, size, date;
     int uid, gid, mode;
     char *mem;
{
  return ar_name_equal (name, mem) ? date : 0;
}

/* Set the archive-member NAME's modtime to now.  */

int
ar_touch (name)
     char *name;
{
  register char *p, *arname, *memname;
  register int val;

  p = index (name, '(');
  arname = savestring (name, p - name);
  val = strlen (p) - 2;
  if (val > 15)
    val = 15;
  memname = savestring (p + 1, val);
  p = rindex (memname, ')');
  if (p != 0)
    *p = '\0';

  /* Make sure we know the modtime of the archive itself before we
     touch the member, since this will change the archive itself.  */
  (void) f_mtime (enter_file (arname));

  val = 1;
  switch (ar_member_touch (arname, memname))
    {
    case -1:
      error ("touch: Archive `%s' does not exist", arname);
      break;
    case -2:
      error ("touch: `%s' is not a valid archive", arname);
    case -3:
      perror_with_name ("touch: ", arname);
      break;
    case 1:
      error ("touch: Member `%s' does not exist in `%s'", memname, arname);
      break;
    case 0:
      val = 0;
      break;
    default:
      error ("touch: Bad return code from ar_member_touch on `%s'", name);
    }

  free (arname);
  free (memname);

  return val;
}
