#include "hydra.h"


#define 	VAL	0xAAAAAAAA

main()
{
	unsigned long val;


	SetupUART();

	c40_putchar( 0x7 );

	WriteEEPROM( VAL, 0 );

	if( ReadEEPROM( 0 ) != VAL )
		c40_printf( "Failed\n\n" );
	else
		c40_printf( "Succeeded\n\n" );

}