/**
*
* Title:  Helios Debugger - Memory dumping.
*
* Author: Andy England
*
* Date:   September 1988
*
*         (c) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/dump.c,v 1.3 1992/10/27 13:51:50 nickc Exp $";
#endif

#include "tla.h"

PUBLIC void dump(DEBUG *debug, word *addr) 
/* -- crf : 07/08/91 - "format", "limit" not used */
/* , UBYTE *limit, int format) */
{
  DISPLAY *display = debug->display;
#ifndef OLDCODE
  int i, j;
#endif

  dstart(display);
#ifndef OLDCODE
  for (i = 0; i < 16; i++)
  {
    dprintf(display, "%08x:  ", addr);
    for (j = 0; j < 4; j++)
      dprintf(display, " %08x", peekword(debug, addr));
    dputc(display, '\n');
  }
#else
  while (addr < limit)
  {
    dprintf(display, "%08x:  ", addr);
    switch (format)
    {
      case 'i':
      addr = disasm(addr);
      break;

      case 'd':
      dprintf(display, "%d", peekshort(debug, addr));
      break;

      case 'D':
      dprintf(display, "%ld", peekword(debug, addr));
      break;

      case 'o':
      dprintf(display, "%o", peekshort(debug, addr));
      break;

      case 'O':
      dprintf(display, "%lo", peekword(debug, addr));
      break;

      case 'x':
      dprintf(display, "%04x", peekshort(debug, addr));
      break;

      case 'X':
      dprintf(display, "%08lx", peekword(debug, addr));
      break;

      case 'b':
      dprintf(display, "\\%o", peekbyte(debug, addr));
      break;

      case 'c':
      dprintf(display, "'%c'", peekbyte(debug, addr));
      break;

      case 's':
      dprintf(display, "\"%s\"", addr);
      addr += strlen(addr) + 1;
      break;

#ifdef FLOAT
      case 'f':
      dprintf(display, "%f", peekfloat(debug, addr));
      break;

      case 'g':
      dprintf(display, "%g", peekdouble(debug, addr));
      break;
#endif

      default:
      cmderr(debug, "bad print format \"%c\"", format);
      return;
    }
    dputc(display, '\n');
  }
#endif
  dend(display, TRUE);
}
