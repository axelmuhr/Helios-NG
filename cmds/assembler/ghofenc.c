/*
 * File:	ghofenc.c
 * Author:	P.A.Beskeen
 * Date:	Aug '91
 *
 * Description: GHOF number encoding function
 *
 * RcsId: $Id: ghofenc.c,v 1.1 1992/03/12 21:16:01 paul Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */

#include "ghof.h"

static int Enc(unsigned n, int nflag);


/*
 * GHOFEncode
 *
 * Write a number out in GHOF encoded format.
 *
 * This is similar to the encoding of the transputer instruction set.
 * It uses the minimum number of bytes necessary to describe a number.
 *
 * Algorithm:
 *	Set top bit of byte if another follows it, set penultimate top bit of
 *	first byte if number is negative, continue setting top bit of byte
 *	ad infinitum until number complete.
 *
 * The function ObjWriteByte() supplied by the GHOF object code formatter
 * is used to output the encoded byte.
 */

void GHOFEncode(int n)
{
	ObjWriteByte((n < 0) ? Enc(-n, ENC_NEG) : Enc(n, 0));
}


static int Enc(unsigned n, int nflag)
{
	if (n >> 6 == 0)
		return ((n & BOT6BITS) | nflag);

	ObjWriteByte(Enc(n >> 7, nflag) | ENC_MORE);

	return(n & BOT7BITS);
}


#if 0
/* Read in and decode A GHOF encoded number. */
/* Returns the decoded number. */

int GHOFDecode(void)
{
   unsigned ch = ObjReadByte();
   int nflag  = (ch & ENC_NEG) != 0;
   unsigned r = ch & 0x3f;

   while( (ch & ENC_MORE) != 0 )
   {
      ch = ObjReadByte();
      r  = (r << 7) | (unsigned)(ch & 0x7f);
   }

   return nflag? -r: r;
}
#endif
