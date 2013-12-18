#include "hydra.h"


extern int EEPROM_Status;

extern hydra_conf config;


void writeVAC( unsigned long add, unsigned long data )
{
	*((unsigned long *)(((0xFFFD0000 | (add << 8)) >> 2) | 0xB0000000)) = (data << 16);
}

void readVACEPROM( void )
{
	unsigned long i;

	i = *((unsigned long *)(((unsigned long)0xff000000 >> 2) | 0xb0000000));
}



void setupVICVAC( hydra_conf config )
{
	int i=0, j;


	readVACEPROM();	

	writeVIC( 0x27, j=ReadEeprom( i++, config.l_jtag_base +8 ) );	/* i = 0 */
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVIC( 0x47, j=ReadEeprom( i++, config.l_jtag_base +8 ) );	/* i = 1 */
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVIC( 0x57, j=ReadEeprom( i++, config.l_jtag_base +8 ) );	/* i = 2 */
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVIC( 0xA7, j=ReadEeprom( i++, config.l_jtag_base +8 ) );	/* i = 3 */
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVIC( 0xB3, j=ReadEeprom( i++, config.l_jtag_base +8 ) );	/* i = 4 */
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVIC( 0xB7, j=ReadEeprom( i++, config.l_jtag_base +8 ) );	/* i = 5 */
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVIC( 0xC3, j=ReadEeprom( i++, config.l_jtag_base +8 ) );	/* i = 6 */
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVIC( 0xC7, j=ReadEeprom( i++, config.l_jtag_base +8 ) );	/* i = 7 */
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVIC( 0xCB, j=ReadEeprom( i++, config.l_jtag_base +8 ) );	/* i = 8 */
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVIC( 0xCF, j=ReadEeprom( i++, config.l_jtag_base +8 ) );	/* i = 9 */
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	i++;

	writeVAC( 0x0, j=ReadEepromWord(i, config) );	/* i = 11 */
	i += 2;	
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x1, j=ReadEepromWord(i, config) );	/* i = 13 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x2, j=ReadEepromWord(i, config) );	/* i = 15 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x3, j=ReadEepromWord(i, config) );	/* i = 17 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x4, j=ReadEepromWord(i, config) );	/* i = 19 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x5, j=ReadEepromWord(i, config) );	/* i = 21 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x6, j=ReadEepromWord(i, config) );	/* i = 23 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x7, j=ReadEepromWord(i, config) );	/* i = 25 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x8, j=ReadEepromWord(i, config) );	/* i = 27 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x9, j=ReadEepromWord(i, config) );	/* i = 29 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0xA, j=ReadEepromWord(i, config) );	/* i = 31 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0xB, j=ReadEepromWord(i, config) );	/* i = 33 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0xC, j=ReadEepromWord(i, config) );	/* i = 35 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0xD, j=ReadEepromWord(i, config) );	/* i = 37 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0xE, j=ReadEepromWord(i, config) );	/* i = 39 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0xF, j=ReadEepromWord(i, config) );	/* i = 41 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x10, j=ReadEepromWord(i, config) );	/* i = 43 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x11, j=ReadEepromWord(i, config) );	/* i = 45 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x12, j=ReadEepromWord(i, config) );	/* i = 47 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x13, j=ReadEepromWord(i, config) );	/* i = 49 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x14, j=ReadEepromWord(i, config) );	/* i = 51 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x16, j=ReadEepromWord(i, config) );	/* i = 53 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x1a, j=ReadEepromWord(i, config) );	/* i = 55 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x1b, j=ReadEepromWord(i, config) );	/* i = 57 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x1c, j=ReadEepromWord(i, config) );	/* i = 59 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x1d, j=ReadEepromWord(i, config) );	/* i = 61 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x1f, j=ReadEepromWord(i, config) );	/* i = 63 */ 
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x23, j=ReadEepromWord(i, config) );	/* i = 65 */
	i += 2;
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
	writeVAC( 0x29, readVAC( 0x29 ) );	/* Initialize VAC with read and write of the VAC id */
	if( !EEPROM_Status )
	{
		ConfigError();
		return;
	}
}


void ConfigError( void )
{
	c40_printf( "Error reading EEPROM.\nSYSTEM NOT FULLY CONFIGURED !!!\n" );
	LED( RED, ON, config );
	LED( GREEN, OFF, config );
}




void SetupVICVACDefault( void )
{
	readVACEPROM();	

	writeVIC( 0xA7, 0x56 );
	writeVIC( 0xB3, 0x70 );
	writeVIC( 0xB7, 0x0D );
	writeVIC( 0xC3, 0x10 );
	writeVIC( 0xC7, 0xFF );
	writeVIC( 0xCB, 0x14 );
	writeVIC( 0xCF, 0xFF );

	writeVAC( 0x0, 0xFFFF );
	writeVAC( 0x1, 0xFFFE );
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
	writeVAC( 0xE, 0xF110 );
	writeVAC( 0xF, 0xF810 );
	writeVAC( 0x10, 0xE014 );
	writeVAC( 0x11, 0xF810 );
	writeVAC( 0x12, 0xF810 );
	writeVAC( 0x13, 0xF810 );
	writeVAC( 0x14, 0xEBFE );
	writeVAC( 0x1b, 0x1FFF );
	writeVAC( 0x1c, 0x4200 );
	writeVAC( 0x1d, 0x77C0 );

	writeVAC( 0x29, readVAC( 0x29 ) );	/* Initialize VAC with read and write of the VAC id */
}