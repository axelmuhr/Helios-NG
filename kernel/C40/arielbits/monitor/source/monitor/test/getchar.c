#define BUF_SIZE 20

static char key_buf[BUF_SIZE];
int key_ptr=0;

extern int key_mode;

#define BEEP 7

void c_int01( void )      /* Keyboard Handler */
{
	char dummy;
	unsigned long tcr;

	
	writeVIC( 0x27, 0xa1 );	/* Disable this interrupt */

/*	tcr = readTCR();		/* Save old TCR */
/*	writeTCR( 0x40 );		/* Set LA1 high */
/*	dummy = *((char *) 0xbfffffc0 );	/* IACK */
/*	writeTCR( tcr );			/* restore old TCR */
	
	if( key_ptr == BUF_SIZE - 1 )
	{
		/**** START WORKAROUND ****/
		while( !(readVAC( 0x25 ) & 0x80 ) )
			dummy = (char)readVAC( 0x20 );
		/**** END WORKAROUND ****/

		dummy = (char)readVAC( 0x20 );
		c40_putchar( BEEP );
	}
	else
	{
		/**** START WORKAROUND ****/
		while( !(readVAC( 0x25 ) & 0x80 ) )
			dummy = (char)readVAC( 0x20 );
		/**** END WORKAROUND ****/

		key_buf[++key_ptr] = readVAC( 0x20 );
	}

	writeVIC( 0x27, 0x01 );		/* Re-enable this interrupt */
}



int c40_getchar( void )
{
	char next_ch;
	int i;

	while( !key_ptr );

	next_ch = key_buf[key_ptr];

	for( i=1 ; i < key_ptr ; i++ )
		key_buf[i] = key_buf[i+1];

	key_ptr--;

	return( next_ch );
}



int c40_putchar( char ch )
{

	writeVIC( 0x27, 0xa1 );	/* Disable this interrupt */


	/* Wait until the transmitter is ready */
	while( !(readVAC(0x25) & (unsigned long)0x100) );

	/* Wait until the transmitter is ready again due to VAC bug */
	while( !(readVAC(0x25) & (unsigned long)0x100) );

	/* Write character to transmitter */
	writeVAC( 0x1E, ch << 8 );

	writeVIC( 0x27, 0x01 );		/* Re-enable this interrupt */

	return( ch );
}
