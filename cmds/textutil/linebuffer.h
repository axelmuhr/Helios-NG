/* linebuffer.h -- declarations for reading arbitrarily long lines
   Copyright (C) 1986, 1991 Free Software Foundation, Inc.

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

/* Written by Richard Stallman. */

/* A `struct linebuffer' holds a line of text. */

struct linebuffer
{
  long size;			/* Allocated. */
  long length;			/* Used. */
  char *buffer;
};

#ifdef __STDC__
/* Initialize a linebuffer for use. */
void initbuffer (struct linebuffer *);

/* Read a line from a stream into a linebuffer;
   works regardless of the length of the line.  */
struct linebuffer *readline (struct linebuffer *, FILE *);
#else
void initbuffer ();
struct linebuffer *readline ();
#endif
