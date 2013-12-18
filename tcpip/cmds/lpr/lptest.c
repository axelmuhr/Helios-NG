/*
 * Copyright (c) 1983 Regents of the University of California.
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

#ifndef __HELIOS
#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)lptest.c  5.3 (Berkeley) 6/30/88";
#endif /* not lint */
#else
static char *rcsid = "$Header: /hsrc/tcpip/cmds/lpr/RCS/lptest.c,v 1.1 1992/01/16 17:35:32 craig Exp $";
#endif

/*
 * lptest -- line printer test program (and other devices).
 */

#include <stdio.h>

#ifdef __HELIOS
#include <stdlib.h>
#endif

int main (int argc, char **argv)
{
  int len, count;
  register i, j, fc, nc;
  char outbuf[BUFSIZ];

  setbuf(stdout, outbuf);
  if (argc >= 2)
    len = atoi(argv[1]);
  else
    len = 79;
  if (argc >= 3)
    count = atoi(argv[2]);
  else
    count = 200;
  fc = ' ';
  for (i = 0; i < count; i++) {
    if (++fc == 0177)
      fc = ' ';
    nc = fc;
    for (j = 0; j < len; j++) {
      putchar(nc);
      if (++nc == 0177)
        nc = ' ';
    }
    putchar('\n');
  }
  (void) fflush(stdout);
}
