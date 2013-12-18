/* SccsId : @(#)hash.c   1.3   1.3 Copyright (C) Perihelion Software Ltd.   */
/* RcsId: $Id: hash.c,v 1.4 1992/07/30 16:16:09 nickc Exp $ */
/* HASHSIZE must be a prime number */
/* some primes: 19 31 41 53 61 71 83 97 101 151 199  251 307 401 503 */


UWORD hash( char * s )
{
  char 		c;
  UWORD 	h = 0;


  while ((c = *s++) != 0)
    {
#ifdef NEVER
      UWORD	g;

      
      h = (h << 4) + c;

      if ( g = h & 0xf0000000 )
	{
	  h = h ^ (g >> 24);
	  h = h ^ g;
	}
#else
      h += (h << 5) + c;      
#endif
    }

  return h;
}
