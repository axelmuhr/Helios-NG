void writeVIC( unsigned long add, unsigned long data )
{
	*((unsigned long *)(((0xFFFC0000 | add) >> 2) | 0xB0000000)) = data;
}

unsigned long readVIC( unsigned long add )
{
	return( *((unsigned long *)(((0xFFFC0000 | add) >> 2) | 0xB0000000)) );
}

void writeVAC( unsigned long add, unsigned long data )
{
	*((unsigned long *)(((0xFFFD0000 | (add << 8)) >> 2) | 0xB0000000)) = (data << 16);
}

unsigned long readVAC( unsigned long add )
{
	return( (*((unsigned long *)(( ((0xFFFD0000 | (add << 8)) >> 2) | 0xB0000000))) >> 16) );
}

void readVACEPROM( void )
{
	unsigned long i;

	i = *((unsigned long *)(((unsigned long)0xff000000 >> 2) | 0xb0000000));
}



void SetupUART( void )
{
/*	writeVAC( 0x16, 0x0010 );	/* Interrupt control reg, Map SIO ints to PIO7 */
/*	writeVAC( 0x1b, 0x0cff );	/* PIO function reg, configure PIO's for SIO and such */
/*	writeVAC( 0x1c, 0x4200 );	/* CPU clock divisor reg, Set for 33 MHz */
/*	writeVAC( 0x1d, 0x77c0 );	/* Configure UART channel A */
/*	writeVAC( 0x23, 0x8000 );	/* Enable interrupts on single character */
/*	writeVAC( 0x1A, 0x4000 );	/* HIACKEN enable */
/*	writeVIC( 0x27, 0x09 );	/* Setup LIRQ1's IPLx value, sensitivity, and polarity */
/*	writeVIC( 0x57, 0x0f );		/* Set interrupt vector */

/*	readVAC( 0x29 );		/* Read VAC Id */

/*	readVAC( 0x20 );		/* Prime the FIFO with a dummy read */

	/****START WORKAROUND ****/
	/* Synchronize UART */
	writeVAC( 0x1d, 0x77d0 );
	writeVAC( 0x1e, 7 );
	writeVAC( 0x1e, 7 );
	writeVAC( 0x1d, 0x77c0 );
	/**** END WORKAROUND ****/

}


void setupVICVAC( void )
{
	int i=0, j;


	readVACEPROM();	

	writeVIC( 0xA7, 0x56 );
	writeVIC( 0xB3, 0x70 );
	writeVIC( 0xB7, 0x0D );
	writeVIC( 0xC3, 0x10 );
	writeVIC( 0xC7, 0xFF );
	writeVIC( 0xCB, 0x10 );
	writeVIC( 0xCF, 0xFF );

	writeVAC( 0x0, 0x00F8 );
	writeVAC( 0x1, 0x0040 );
	writeVAC( 0x2, 0xFF00 );
	writeVAC( 0x3, 0x3400 );
	writeVAC( 0x4, 0xF0F1 );
	writeVAC( 0x5, 0x0001 );
	writeVAC( 0x6, 0x0002 );
	writeVAC( 0x7, 0xFDFF );
	writeVAC( 0x8, 0x7F00 );
	writeVAC( 0x9, 0x2C00 );
	writeVAC( 0xA, 0x4C00 );
	writeVAC( 0xB, 0x0400 );
	writeVAC( 0xC, 0xF810 );
	writeVAC( 0xD, 0xF810 );
	writeVAC( 0xE, 0xE010 );
	writeVAC( 0xF, 0xF810 );
	writeVAC( 0x10, 0xE014 );
	writeVAC( 0x11, 0xF810 );
	writeVAC( 0x12, 0xF810 );
	writeVAC( 0x13, 0xF810 );
	writeVAC( 0x14, 0xEBFE );
	writeVAC( 0x1b, 0x1FFF );

	writeVAC( 0x29, readVAC( 0x29 ) );	/* Initialize VAC with read and write of the VAC id */


	writeVIC( 0x27, j=ReadEeprom(i++) );	/* i = 0 */
	writeVIC( 0x57, j=ReadEeprom(i++) );	/* i = 1 */
	writeVIC( 0xA7, j=ReadEeprom(i++) );	/* i = 2 */
	writeVIC( 0xB3, j=ReadEeprom(i++) );	/* i = 3 */
	writeVIC( 0xB7, j=ReadEeprom(i++) );	/* i = 4 */
	writeVIC( 0xC3, j=ReadEeprom(i++) );	/* i = 5 */
	writeVIC( 0xC7, j=ReadEeprom(i++) );	/* i = 6 */
	writeVIC( 0xCB, j=ReadEeprom(i++) );	/* i = 7 */
	writeVIC( 0xCF, j=ReadEeprom(i++) );	/* i = 8 */
	i++;

	writeVAC( 0x0, j=ReadEepromWord(i) );	/* i = 10 */
	i += 2;	
	writeVAC( 0x1, j=ReadEepromWord(i) );	/* i = 12 */
	i += 2;
	writeVAC( 0x2, j=ReadEepromWord(i) );	/* i = 14 */
	i += 2;
	writeVAC( 0x3, j=ReadEepromWord(i) );	/* i = 16 */
	i += 2;
	writeVAC( 0x4, j=ReadEepromWord(i) );	/* i = 18 */
	i += 2;
	writeVAC( 0x5, j=ReadEepromWord(i) );	/* i = 20 */
	i += 2;
	writeVAC( 0x6, j=ReadEepromWord(i) );	/* i = 22 */
	i += 2;
	writeVAC( 0x7, j=ReadEepromWord(i) );	/* i = 24 */
	i += 2;
	writeVAC( 0x8, j=ReadEepromWord(i) );	/* i = 26 */
	i += 2;
	writeVAC( 0x9, j=ReadEepromWord(i) );	/* i = 28 */
	i += 2;
	writeVAC( 0xA, j=ReadEepromWord(i) );	/* i = 30 */
	i += 2;
	writeVAC( 0xB, j=ReadEepromWord(i) );	/* i = 32 */
	i += 2;
	writeVAC( 0xC, j=ReadEepromWord(i) );	/* i = 34 */
	i += 2;
	writeVAC( 0xD, j=ReadEepromWord(i) );	/* i = 36 */
	i += 2;
	writeVAC( 0xE, j=ReadEepromWord(i) );	/* i = 38 */
	i += 2;
	writeVAC( 0xF, j=ReadEepromWord(i) );	/* i = 40 */
	i += 2;
	writeVAC( 0x10, j=ReadEepromWord(i) );	/* i = 42 */
	i += 2;
	writeVAC( 0x11, j=ReadEepromWord(i) );	/* i = 44 */
	i += 2;
	writeVAC( 0x12, j=ReadEepromWord(i) );	/* i = 46 */
	i += 2;
	writeVAC( 0x13, j=ReadEepromWord(i) );	/* i = 48 */
	i += 2;
	writeVAC( 0x14, j=ReadEepromWord(i) );	/* i = 50 */
	i += 2;
	writeVAC( 0x16, j=ReadEepromWord(i) );	/* i = 52 */
	i += 2;
	writeVAC( 0x1a, j=ReadEepromWord(i) );	/* i = 54 */
	i += 2;
	writeVAC( 0x1b, j=ReadEepromWord(i) );	/* i = 56 */
	i += 2;
	writeVAC( 0x1c, j=ReadEepromWord(i) );	/* i = 58 */
	i += 2;
	writeVAC( 0x1d, j=ReadEepromWord(i) );	/* i = 60 */
	i += 2;
/*	writeVAC( 0x1f, j=ReadEepromWord(i) );	/* i = 62 */ 
	i += 2;
	writeVAC( 0x23, j=ReadEepromWord(i) );	/* i = 64 */
	i += 2;

	writeVAC( 0x29, readVAC( 0x29 ) );	/* Initialize VAC with read and write of the VAC id */

}




unsigned long ReadEepromWord( int WordAddress )
{
	unsigned long word;
	int i, j;

	for( i=0, word=0L ; i < 2 ; i++ )
		word |= ((j=ReadEeprom( WordAddress++ )) << (i*8));

	return( word );
}
