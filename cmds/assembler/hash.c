/*
 * File:	symbtab.c
 * Author:	P.A.Beskeen
 * Date:	Aug '91
 *
 * Description: hash generator function
 *
 *		This is effectively the same function as used in transputer
 *		assembler and generic linker i.e. I stole it!
 *
 *		CAUTION: HASHSIZE must be a prime number!
 *		Some primes are:
 *
 *			19 31 41 53 61 71 83 97 101 151 199  251 307 401 503
 *			509 541 557 599 653 701 751 809 853 907 953 1009
 *			1499 1999 2503 3001 4001 4999 6007 7001 8009 9001
 *			10007 15013 30011 40009 50021 60013 65521
 *			1000231!
 *
 * RcsId: $Id: hash.c,v 1.1 1992/03/12 21:16:01 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */

#include "gasm.h"


unsigned int Hash(char *s)
{
	char 		c;
	unsigned int	h = 0;


	while ((c = *s++) != '\0') {
#if 1
		h += (h << 5) + c;
#else
		unsigned int g;

		h = (h << 4) + c;

		if ( g = h & 0xf0000000 ) {
			h = h ^ (g >> 24);
			h = h ^ g;
		}
#endif
	}

	return (h % HASHSIZE);
}


/* end of hash.c */

