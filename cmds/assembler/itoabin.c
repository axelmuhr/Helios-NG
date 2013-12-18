/*
 * File:	itoabin.c
 * Author:	P.A.Beskeen
 * Date:	Aug '91
 *
 * Description: Function to convert an integer to a textual binary
 *		representation.
 *
 *
 * RcsId: $Id: itoabin.c,v 1.1 1992/03/12 21:16:01 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */

#ifndef NULL
# define NULL 0
#endif


/*
 * itoabin
 *
 * Converts 'nbits' least significant bits of integer argument 'b' to a
 * textual ASCII binary representation.
 *
 * The text is returned in '*user_str' which is assumed to contain at least
 * nbits+1 characters. If rs is NULL, itoabin returns a pointer to an internal
 * static string. This string will of course be corrupted by the next call to
 * itoabin and can only cope with up to 64 bits - you got a bigger int?!
 *
 */

char *itoabin(char *user_str, int nbits, int b)
{
	static char static_str[65];
	char *bs;
	int i = 0;

	if (user_str == NULL)
		bs = static_str;
	else
		bs = user_str;

	for (i = 0; i < nbits; i++) {
		if (b & (1 << (nbits-1)))
			*bs++ = '1';
		else
			*bs++ = '0';
		b <<= 1;
	}

	*bs = '\0';

	if (user_str == NULL)
		return static_str;
	else
		return user_str;

}


/* end of itoabin.c */
