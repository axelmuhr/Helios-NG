/* $Id: encode.c,v 1.5 1992/09/25 10:41:21 paul Exp $ */

#ifdef __HELIOS
#include <helios.h>
#define TargetBytesPerWord 4
#else
#include "ttypes.h"
#endif

#ifdef FALSE
#undef FALSE
#endif

#define FALSE 0l

#define f_pfix 0x20
#define f_nfix 0x60

/********************************************************/
/* pfsize                                               */
/*                                                      */
/* Calculates how many bytes the supplied value can be  */
/* encoded in.                                          */
/********************************************************/

PUBLIC WORD
pfsize( n )
  WORD n;
{
  WORD pfx = 0;

  
  if ( n < 0 )
    {
      pfx = 1;
      n   = (~n) >> 4;
    }

  for (; pfx <= TargetBytesPerWord * 2; pfx++)
    {
      n = n >> 4;
      
      if ( n == 0 )
	return pfx + 1;
    }

  return pfx;
}

/********************************************************/
/* encode                                               */
/*                                                      */
/* Generates a prefix encoded version of the given op   */
/* and argument, outputting the bytes via the supplied  */
/* function.                                            */
/********************************************************/

#define pbyte( x ) (*pbytefn)( x )

PRIVATE void
encodestep( arg, negative, pbytefn )
  WORD 		arg, negative;
  void (*	pbytefn)();
{
  if( arg > 15 )
    {
      encodestep( (WORD)(arg >> 4), negative, pbytefn );
      
      pbyte( (WORD)(f_pfix | ((negative ? ~arg : arg) & 0xf)) );
    }
  else
    pbyte( (WORD)((negative ? f_nfix : f_pfix ) | (arg & 0xf)) );
}


PUBLIC void
encode( op, arg, pbytefn )
  WORD 		op;
  WORD 		arg;
  void (*	pbytefn)();
{
  if ( arg < 0 )
    encodestep( (WORD)((~arg) >> 4), TRUE, pbytefn );
  else if ( arg > 15 )
    encodestep( (WORD)(arg >> 4), FALSE, pbytefn );

  pbyte( (WORD)(op | ( arg & 0xf )) );

  return;
}

/* End of encode.c */
