/*
 * Copyright (c) 1987 Regents of the University of California.
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

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1987 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef __HELIOS
#ifndef lint
static char sccsid[] = "@(#)yes.c	5.3 (Berkeley) 6/29/88";
#endif /* not lint */
#endif

#include <stdio.h>
  
int
main(
	int argc,
	char **argv )
{
	if (argc > 1)
		for(;;)
			puts(argv[1]);
	else for (;;)
		puts("y");
}
