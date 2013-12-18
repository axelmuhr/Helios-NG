#include "hydra.h"
#include "vicvac.h"

extern void c_int01( void );

main()
{
	unsigned long i;


	SetBCR( 0x1dea4000, 0x19cc4000 );

	readVACEPROM();

	SetKeyHandler( c_int01 );

	SetupUART();

	writeVAC( 0x29, readVAC( 0x29 ) );	/* Initialize VAC with read and write of the VAC id */

	c40_putchar( 7 );

	while( 1 )
		c40_putchar( c40_getchar() );

	while( 1 )
	{
		c40_printf( "Bob Dillon Rules !!!!!!!\n" );
		c40_putchar( 7 );
	}

}



void SetBCR( unsigned long global, unsigned long local )
{
	*(unsigned long *)(0x100000) = global;
	*(unsigned long *)(0x100004) = local;
}


void SetupUART( void )
{
	writeVAC( 0x16, 0x0010 );	/* Interrupt control reg, Map SIO ints to PIO7 */
	writeVAC( 0x1b, 0x0cff );	/* PIO function reg, configure PIO's for SIO and such */
	writeVAC( 0x1c, 0x4200 );	/* CPU clock divisor reg, Set for 33 MHz */
	writeVAC( 0x1d, 0x7fc0 );	/* Configure UART channel A */
	writeVAC( 0x23, 0x8000 );	/* Enable interrupts on single character */
	writeVAC( 0x1A, 0x4000 );	/* HIACKEN enable */
	writeVIC( 0x27, 0x09 );	/* Setup LIRQ1's IPLx value, sensitivity, and polarity */
	writeVIC( 0x57, 0x0f );		/* Set interrupt vector */
	readVAC( 0x29 );		/* Read VAC Id */

	readVAC( 0x20 );		/* Prime the FIFO with a dummy read */

	/****START WORKAROUND ****/
	/* Synchronize UART */
	writeVAC( 0x1d, 0x77d0 );
	writeVAC( 0x1e, 7 );
	writeVAC( 0x1e, 7 );
	writeVAC( 0x1d, 0x77c0 );
	/**** END WORKAROUND ****/

}
