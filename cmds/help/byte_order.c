/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Margo Seltzer.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1990 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)byte_order.c	5.1 (Berkeley) 1/31/91";
#endif /* not lint */

#include <stdio.h>
main ( argc, argv )
int	argc;
char	**argv;
{
    int		num = 0x41424344;
    if ( ((((char *)&num)[0]) == 0x41) && ((((char *)&num)[1]) == 0x42) &&
	 ((((char *)&num)[2]) == 0x43) && ((((char *)&num)[3]) == 0x44) ) {
	    printf ( "BIG ENDIAN\n" );
    } else if ( ((((char *)&num)[3]) == 0x41) && 
		((((char *)&num)[2]) == 0x42) && 
		((((char *)&num)[1]) == 0x43) && 
		((((char *)&num)[0]) == 0x44) ) {
	    printf ( "LITTLE ENDIAN\n" );
    } else {
	printf ( "Odd Endian -- are you running on a PDP-11?\n");
    }

}
