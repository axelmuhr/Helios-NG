#include "hydra.h"


extern unsigned short crctab[];


void crcupdate( unsigned long data, unsigned long *accum )
{
	unsigned long comb_val;

	comb_val = (*accum >> 8) ^ (data & 0xFF);
	*accum = (*accum << 8) ^ crctab[comb_val];
}


unsigned long x_rcvcrc( unsigned long *data, int data_size )
{
	unsigned long i, accum;

	for( accum=i=0 ; i < data_size+2 ; i++ )
		crcupdate( *data++, &accum );
	return( accum );
}



unsigned long x_sndcrc( unsigned long *buff, int data_size )
{
	unsigned long i, accum;

	for( accum=i=0 ; i < data_size ; ++i )
		crcupdate( *buff++, &accum );

	/* return( accum ); */
	return( (accum >> 8) + (accum << 8) );
}





unsigned long crchware( unsigned long data, unsigned long genpoly, unsigned long accum )
{
	static int i;

	data <<= 8;
	for( i=8 ; i > 0 ; i-- )
	{
		if( (data^accum) & 0x8000 )
			accum = (accum << 1) ^ genpoly;
		else
			accum <<= 1;

		data <<= 1;
	}

	return( accum );
}





void mk_crctbl( unsigned long poly, unsigned long (*crcfn)(unsigned long,unsigned long,unsigned long) )
{
	int i;

	for( i=0 ; i < 256 ; i++ )
		crctab[i] = (*crcfn)( i, poly, 0 );
}
