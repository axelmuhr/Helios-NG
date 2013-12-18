#include <helios.h>

#define TargetBytesPerWord 4

#define TRUE 1
#define FALSE 0

#define f_pfix 0x20
#define f_nfix 0x60

/********************************************************/
/* pfsize                                               */
/*                                                      */
/* Calculates how many bytes the supplied value can be  */
/* encoded in.                                          */
/********************************************************/

WORD pfsize(n)
WORD n;
{
        WORD pfx = 0;

        if( n < 0 )
        {
                pfx = 1;
                n = (~n) >> 4;
        }

        for(; pfx <= TargetBytesPerWord*2; pfx++)
        {
                n = n>>4;
                if( n == 0 ) return pfx+1;
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

#define pbyte(x) (*pbytefn)(x)

encode(op,arg,pbytefn)
UBYTE op;
WORD arg;
void (*pbytefn)();
{
        if ( arg < 0 ) encodestep( (~arg)>>4, TRUE, pbytefn);
        else if ( arg > 15 ) encodestep( arg>>4, FALSE, pbytefn);
        pbyte( op | ( arg & 0xf ) );
}

encodestep( arg, negative, pbytefn)
WORD arg, negative;
void (*pbytefn)();
{
        if( arg > 15 )
        {
                encodestep( arg>>4, negative, pbytefn);
                pbyte( f_pfix | ((negative ? ~arg : arg) & 0xf) );
        }
        else pbyte((negative ? f_nfix : f_pfix ) | (arg & 0xf) );
}
