/* $Id: hash.c,v 1.3 1991/06/07 07:30:14 nickc Exp $ */

UWORD hash(s)
  char *s;
{
  char 		c;
  UWORD 	h = 0;


  while (c = *s++)
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
