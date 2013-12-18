#include "hydra.h"

extern int EEPROM_Status;
extern hydra_conf config;


int read_config( hydra_conf *config )
{
	unsigned long chksum=0, val;
	unsigned int temp;


	val = ReadEepromWord( 61, *config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}

	switch( (val&0x1C00) >> 10 )
	{
		case 0:
			config->uartA.baud = 150;
			break;
		case 1:
			config->uartA.baud = 300;
			break;
		case 2:
			config->uartA.baud = 600;
			break;
		case 3:
			config->uartA.baud = 1200;
			break;
		case 4:
			config->uartA.baud = 2400;
			break;
		case 5:
			config->uartA.baud = 4800;
			break;
		case 6:
			config->uartA.baud = 9600;
			break;
		case 7:
			config->uartA.baud = 19200;
			break;
	}

	if( val & 0x8000 )
		if( val & 0x4000 )
			config->uartA.parity = 2;
		else
			config->uartA.parity = 1;
	else
		if( val & 0x4000 )
			config->uartA.parity = -2;
		else
			config->uartA.parity = -1;

	if( val & 0x2000 )
		config->uartA.bits = 8;
	else
		config->uartA.bits = 7;


	val = ReadEepromWord( 63, *config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	switch( (val&0x1C00) >> 10 )
	{
		case 0:
			config->uartB.baud = 150;
			break;
		case 1:
			config->uartB.baud = 300;
			break;
		case 2:
			config->uartB.baud = 600;
			break;
		case 3:
			config->uartB.baud = 1200;
			break;
		case 4:
			config->uartB.baud = 2400;
			break;
		case 5:
			config->uartB.baud = 4800;
			break;
		case 6:
			config->uartB.baud = 9600;
			break;
		case 7:
			config->uartB.baud = 19200;
			break;
	}

	if( val & 0x8000 )
		if( val & 0x4000 )
			config->uartB.parity = 2;
		else
			config->uartB.parity = 1;
	else
		if( val & 0x4000 )
			config->uartB.parity = -2;
		else
			config->uartB.parity = -1;
	 
	if( val & 0x2000 )
		config->uartB.bits = 8;
	else
		config->uartB.bits = 7;


	config->dram_size = ReadEepromWord( 67, *config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	config->daughter = ReadEepromWord( 69, *config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	config->sram1_size = ReadEepromWord( 71, *config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	config->sram2_size = ReadEepromWord( 73, *config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	config->sram3_size = ReadEepromWord( 75, *config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	config->sram4_size = ReadEepromWord( 77, *config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}

	switch( ReadEepromWord( 59, *config ) >> 8 )
	{
		case 66:
			config->cpu_clock = 40;
			break;
		case 82:
			config->cpu_clock = 50;
			break;
		default:
			config->cpu_clock = 40;
			break;
	}
	if( !EEPROM_Status )
	{
		return( 0 );
	}

	val = ReadEepromWord( 17, *config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	config->l_dram_base = (val << 14) | 0x80000000;

	val = ReadEepromWord( 25, *config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	config->l_jtag_base = (val << 14) | 0x80000000;

	config->revision = ((' '<<24)|(' '<<16)|(' '<<8)|ReadEeprom( 79, config->l_jtag_base + 8 ));
	if( !EEPROM_Status )
	{
		return( 0 );
	}
}





void ConfigError( void )
{
	LED( RED, ON, config );
	LED( GREEN, OFF, config );
}


void writeVAC( unsigned long add, unsigned long data )
{
	*((unsigned long *)(((0xFFFD0000 | (add << 8)) >> 2) | 0xB0000000)) = (data << 16);
}

unsigned long readVAC( unsigned long add )
{
	return( (*((unsigned long *)(( ((0xFFFD0000 | (add << 8)) >> 2) | 0xB0000000))) >> 16) & 0xFFFF );
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