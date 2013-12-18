#include "ctype.h"
#include "hydra.h"


extern int EEPROM_Status;

void configure( hydra_conf *config )
{
	unsigned long i, temp_addr, flag;
	char inchar;


	while( 1 )
	{
		display_conf( *config );

		switch( menu() )
		{
			case '1':
				configure_uart( config, 'a' );
				update_chksum( config );
				break;
			case '2':
				configure_uart( config, 'b' );
				update_chksum( config );
				break;
			case '3':
				config->cpu_clock = get_clock();
				update_chksum( config );
				break;
			case '4':
				get_sram( config );
				update_chksum( config );
				break;
			case '5':
				config->dram_size = get_dram();
				update_chksum( config );
				break;
			case '6':
				for( flag=FALSE ; !flag ; )
				{
					temp_addr = get_addr( "DRAM base" );
					if( !temp_addr )
						flag = TRUE;
					else if( (temp_addr>>16) <= 0x8000 )
						c40_printf( "Not a valid base address for DRAM\n" );
					else
					{
						config->l_dram_base = temp_addr;
						flag = TRUE;
					}
				}
				update_chksum( config );
				break;
			case '7':
				c40_printf( "\n\nVME or Local Address (V/L) ?" );
				for( inchar=tolower(c40_getchar()) ; (inchar!='v') && (inchar!='l') ; inchar=tolower(c40_getchar()), c40_printf( "%c", BEEP ) );
				switch( inchar )
				{
					case 'v':
						temp_addr = get_addr( "VME JTAG/Hydra control register base" );
						if( temp_addr )
						{
							WriteEepromWord(13, temp_addr >> 16, *config);
							if( !EEPROM_Status )
							{
								ConfigError();
								return;
							}
							config->l_jtag_space=get_space();
						}
						break;
					case 'l':
						temp_addr = get_addr( "Local JTAG/Hydra control register base" );
						config->l_jtag_base = temp_addr ? temp_addr : config->l_jtag_base;
						update_chksum( config );
						break;
				}
				break;
			case '8' :
				WriteEepromWord( 19, (get_addr( "IPCR base" ) & 0x7FFFFFFF) >> 14, *config );
				update_chksum( config );
				break;
			case '9' :
				config->daughter = get_daughter();
				update_chksum( config );
				break;
			case 'V' :
			case 'v' :
				c40_printf( "\n\n     Enter hardware revision " );
				while( !isalpha(inchar=c40_getchar()) )
					c40_printf( "%c", BEEP );
				c40_printf( "%c\n\n", config->revision = toupper(inchar) );
				update_chksum( config );
				break;
			case 'R' :
			case 'r' :
				if( !write_config( *config ) )
				{
					c40_printf( "Error writting EEPROM.\n" );
				}
				c40_printf( "\n\n" );
				return;
				break;
		}
	}
}



unsigned long menu( void )
{
	int i;

	c40_printf( "\n" );
	c40_printf(  "1) RS-232 channel A configuration\n"
	             "2) RS-232 channel B configuration\n"
	             "3) CPU clock rate\n"
	             "4) SRAM size(s)\n"
	             "5) DRAM size\n"
	             "6) DRAM base address\n"
	             "7) JTAG / Hydra control register address(s)\n"
	             "8) IPCR base address\n"
	             "9) Daughter card attached\n"
	             "R) Return\n" );

	c40_printf( "Selection => " );
	while( 1 )
	{
		i = c40_getchar();
		if( (i=='R') || (i=='r') )
			return( i );
		else if( (i=='V') || (i=='v') )
			return( i );
		else if( isdigit(i) )
		{
			if( (i >= '1') && (i <= '9') )
				return( i );
			else
				c40_printf( "%c", 7 );

		}
		else
			c40_printf( "%c", 7 );
	}
}



void configure_uart( hydra_conf *config, char which )
{
	unsigned long val;
	char inchar;


	c40_printf( "\n\n1) Baud rate\n2) Parity\n3) Bits/character\nR) Return\n\nSelection => " );

	while( 1 )
	{
		inchar = c40_getchar();
		if( inchar == '1' ) 
		{
			switch( which )
			{
				case 'a':
					config->uartA.baud = (val=get_baud())?val:config->uartA.baud;
					break;
				case 'b':
					config->uartB.baud = (val=get_baud())?val:config->uartB.baud;
					break;
			}
			c40_printf( "\n\n1) Baud rate\n2) Parity\n3) Bits/character\nR) Return\n\nSelection => " );
		}
		else if( inchar == '2' )
		{
			get_parity( config, which );
			c40_printf( "\n\n1) Baud rate\n2) Parity\n3) Bits/character\nR) Return\n\nSelection => " );
		}
		else if( inchar == '3' )
		{
			get_bits( config, which );
			c40_printf( "\n\n1) Baud rate\n2) Parity\n3) Bits/character\nR) Return\n\nSelection => " );
		}
		else if( (inchar == 'r') || (inchar == 'R') )
			return;
		else
			c40_printf( "%c", 7 );
	}
}



unsigned long get_baud( void )
{
	char inchar;

	c40_printf( "\n\nBaud Rates :\n" );
	c40_printf( "  1) 150\n  2) 300\n  3) 600\n  4) 1200\n  5) 2400\n"
	            "  6) 4800\n  7) 9600\n  8) 19200\n  R) Return" );
	c40_printf( "\n" );
	c40_printf( "Selection => " );

	while( 1 )
	{
		inchar = c40_getchar();
		if( (inchar >= '1') && ( inchar <= '7') )
			switch( inchar )
			{
				case '1':
					return( 150 );
				case '2':
					return( 300 );
				case '3':
					return( 600 );
				case '4':
					return( 1200 );
				case '5':
					return( 2400 );
				case '6':
					return( 4800 );
				case '7':
					return( 9600 );
				case '8':
					return( 19200 );
			}
		else if( (inchar == 'r') || (inchar == 'R') )
			return( 0 );
		else
			c40_printf( "%c", 7 );
	}
}





void get_parity( hydra_conf *config, char which )
{
	char inchar;

	c40_printf( "\n\n1) Parity enabled/disabled\n2) Even/odd parity\nR) Return\n\nSelection => " );

	while( 1 )
	{
		inchar = c40_getchar();
		if( inchar == '1' ) 
		{
			switch( which )
			{
				case 'a':
					config->uartA.parity = parity_enable()?abs(config->uartA.parity):-abs(config->uartA.parity);
					break;
				case 'b':
					config->uartB.parity = parity_enable()?abs(config->uartB.parity):-abs(config->uartA.parity);
					break;
			}
			c40_printf( "\n\n1) Parity enabled/disabled\n2) Even/odd parity\nR) Return\n\nSelection => " );
		}
		else if( inchar == '2' )
		{
			switch( which )
			{
				case 'a':
					if( config->uartA.parity > 0 )
						config->uartA.parity = parity_type();
					else
						config->uartA.parity = -parity_type();
					break;
				case 'b':
					if( config->uartB.parity > 0 )
						config->uartB.parity = parity_type();
					else
						config->uartB.parity = -parity_type();
					break;
			}
			c40_printf( "\n\n1) Parity enabled/disabled\n2) Even/odd parity\nR) Return\n\nSelection => " );
		}
		else if( (inchar == 'r') || (inchar == 'R') )
			return;
		else
			c40_printf( "%c", 7 );
	}
}




unsigned long parity_enable( void )
{
	c40_printf( "\n\nEnable parity (y/n)?" );
	while( 1 )
	{
		switch( c40_getchar() )
		{
			case 'Y' :
			case 'y' :
				c40_printf( "\n" );
				return( 1 );
			case 'N' :
			case 'n' :
				c40_printf( "\n" );
				return( 0 );
			default :
				c40_printf( "%c", 7 );
				break;
		}
	}
}



	
unsigned long parity_type( void )
{
	c40_printf( "\n\nEven or odd parity (E/O)?" );
	while( 1 )
	{
		switch( c40_getchar() )
		{
			case 'E' :
			case 'e' :
				c40_printf( "\n" );
				return( 2 );
			case 'O' :
			case 'o' :
				c40_printf( "\n" );
				return( 1 );
			default :
				c40_printf( "%c", 7 );
				break;
		}
	}
}







void get_bits( hydra_conf *config, char which )
{
	c40_printf( "\n\nBits per character (7/8)?" );
	while( 1 )
	{
		switch( c40_getchar() )
		{
			case '7' :
				switch( which )
				{
					case 'a':
						config->uartA.bits = 7;
						return;
					case 'b':
						config->uartB.bits = 7;
						return;
				}
			case '8' :
				switch( which )
				{
					case 'a':
						config->uartA.bits = 8;
						return;
					case 'b':
						config->uartB.bits = 8;
						return;
				}
			default :
				c40_printf( "%c", 7 );
				break;
		}
	}
}



	

unsigned long get_addr( char *str )
{
	char in_char, in_line[9]={ '\0' };
	int i, ok=FAILURE;
	unsigned long addr;

	while( !ok )
	{
		c40_printf( "\n" );
		c40_printf( "%s Address => ", str );

		ok = SUCCESS;
		i=0;
		while( (c40_putchar((in_char = c40_getchar()))) != '\n' )
		{
			if( in_char == '\b' )
			{
				if( i )
					i--;
			}
			else if( in_char == 0xd ) /* 0xd is a carriage return */
				continue;
			else
				in_line[i++] = in_char;

			if( i > 8 )
			{
				c40_printf( "\nLine too long.\n" );
				ok = FAILURE;
				break;
			}
		}
		in_line[i] = '\0';

		if( !ok )
			continue;

		if( i == 0 )
			return( 0 );	

		ok = SUCCESS;
		addr = atox( in_line, &ok );
		if( ok == FAILURE )
			c40_printf( "Not a valid number : %s\n", in_line );
		else
			return( addr );
	}
}




unsigned long get_clock( void )
{
	c40_printf( "\n\n" );
	c40_printf( "1) 40 MHz\n2) 50 MHz\n" );
	c40_printf( "Selection => " );

	while( 1 )
	{
		switch( c40_getchar() )
		{
			case '1' :
				c40_printf( "\n" );
				return( (unsigned long) 40 );
			case '2' :
				c40_printf( "\n" );
				return( (unsigned long) 50 );
			default :
				c40_printf( "%c", 7 );
		}
	}
}





void get_sram( hydra_conf *config )
{
	int flag=FALSE;
	char inchar;
	SramSize *ptr;
        int i;

	c40_printf( "\nFor which processor (1..4) " );
	while( !flag )
	{
		inchar = c40_getchar();
		if( (inchar >= '1') && (inchar <= '4') )
		{
			switch( inchar )
			{
				case '1':
					ptr = &config->sram1_size;
					break;
				case '2':
					ptr = &config->sram2_size;
					break;
				case '3':
					ptr = &config->sram3_size;
					break;
				case '4':
					ptr = &config->sram4_size;
					break;
			}
			flag = TRUE;
		}
		else
			c40_printf( "%c", 7 );
	}
	c40_printf( "\n" );

	for( i=0 ; i < 2 ; i++, ptr++ )
	{
		c40_printf( "\n\n%s SRAM size:\n", i?"Global":"Local" );
		c40_printf( "1) 16 Kwords\n2) 64 Kwords\n3) 256 Kwords\n" );
		c40_printf( "Selection => " );

		while( 1 )
		{
			switch( c40_getchar() )
			{
				case '1' :
					c40_printf( "\n" );
					*(int *)ptr = 16;
					return;
				case '2' :
					c40_printf( "\n" );
					*(int *)ptr = 64;
					return;
				case '3' :
					c40_printf( "\n" );
					*(int *)ptr = 256;
					return;
				default :
					c40_printf( "%c", 7 );
			}
		}
	}
}



unsigned long get_daughter( void )
{
	c40_printf( "\n\nIs a daughter card attached (y/n)?" );
	while( 1 )
	{
		switch( c40_getchar() )
		{
			case 'Y' :
			case 'y' :
				c40_printf( "\n" );
				return( 1 );
			case 'N' :
			case 'n' :
				c40_printf( "\n" );
				return( 0 );
			default :
				c40_printf( "%c", 7 );
				break;
		}
	}
}



unsigned int get_dram( void )
{
	c40_printf( "\n" );
	c40_printf( "1) 1 MWords\n2) 4 MWords\n3) 16 MWords\n\n" );
	c40_printf( "Selection => " );

	while( 1 )
	{
		switch( c40_getchar() )
		{
			case '1' :
				c40_printf( "\n" );
				return( (unsigned long) 1 );
				break;
			case '2' :
				c40_printf( "\n" );
				return( (unsigned long) 4 );
				break;
			case '3' :
				c40_printf( "\n" );
				return( (unsigned long) 16 );
				break;
			default :
				c40_printf( "%c", 7 );
				break;
		}
	}
}




unsigned long get_space( void )
{
   	char inchar;


	c40_printf( "\n" );
	c40_printf( "1) A32\n2) A24\n3) A16\n\n" );
	c40_printf( "Selection => " );

	while( 1 )
	{
   		inchar = c40_getchar();

		if( (inchar >=1) && (inchar <=3) )
		{
			return( inchar - '1' );
			c40_printf( "\n" );
		}
		else
			c40_printf( "%c", 7 );
	}
}






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
	config->sram1_size.local = ReadEeprom( 71, config->l_jtag_base+8 );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	config->sram1_size.global = ReadEeprom( 72, config->l_jtag_base+8 );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	config->sram2_size.local = ReadEeprom( 73, config->l_jtag_base+8);
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	config->sram2_size.global = ReadEeprom( 74, config->l_jtag_base+8 );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	config->sram3_size.local  = ReadEeprom( 75, config->l_jtag_base+8 );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	config->sram3_size.global = ReadEeprom( 76, config->l_jtag_base+8 );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	config->sram4_size.local  = ReadEeprom( 77, config->l_jtag_base+8 );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	config->sram4_size.global = ReadEeprom( 78, config->l_jtag_base+8 );
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
 
        config->l_jtag_space = (ReadEeprom( 0x8, config->l_jtag_base + 8 ) >>2) & 0x3;

	config->revision = ((' '<<24)|(' '<<16)|(' '<<8)|ReadEeprom( 79, config->l_jtag_base + 8 ));
	if( !EEPROM_Status )
	{
		return( 0 );
	}
}





void display_conf( hydra_conf config )
{
	int i, j, tempA, tempB, Dtemp;
	SramSize temparry[4];
	char	*ptrA, *ptrB;


	c40_printf( "\n\n" );

	c40_printf( "                           Channel A              Channel B\n" );
	c40_printf( "Baud rate                     %d                   %d\n", config.uartA.baud, config.uartB.baud );

	i = config.uartA.parity;
	if( i > 0 )
		ptrA = " En";
	else
		ptrA = "Dis";

	i = config.uartB.parity;
	if( i > 0 )
		ptrB = " En";
	else
		ptrB = "Dis";

	c40_printf( "Parity                    %sabled/%s           %sabled/%s\n"
	          , ptrA, abs(config.uartA.parity)==1?"Odd":"Even"
	          , ptrB, abs(config.uartB.parity)==1?"Odd":"Even" );
	c40_printf( "Bits per character             %d                      %d\n"
	          , config.uartA.bits, config.uartB.bits );

	c40_printf( "CPU clock rate = %d MHz.\n", config.cpu_clock );

	temparry[0] = config.sram1_size;
	temparry[1] = config.sram2_size;
	temparry[2] = config.sram3_size;
	temparry[3] = config.sram4_size;
	if( config.daughter )
		j = 4;
	else
		j = 2;
	for( i=0 ; i < j ; i++ )
	{
		c40_printf( "DSP %d SRAM size = %d KWords(Local), %d KWords(Global).", i+1, temparry[i].local, temparry[i].global );
		if( !(i % 2) )
			c40_printf( "  " );
		else
			c40_printf( "\n" );
	}

	switch( config.dram_size )
	{
		case 1 :
		case 4 :
		case 16 :
			c40_printf( "DRAM size = %d MWords.\n", config.dram_size );
			break;
		default:
			c40_printf( "Invalid DRAM size.\n" );
			break;
	}
	
	c40_printf( "DRAM base address = %xH\n", config.l_dram_base );

	c40_printf( "JTAG / Hydra control register address = %xH(Local), %xH in A%d(VME)\n", config.l_jtag_base, ReadEepromWord(13, config) << 16, 32-(config.l_jtag_space*8) );

	c40_printf( "IPCR base address = %xH\n", (ReadEepromWord(19, config) << 14) | 0x80000000 );

	c40_printf( "Daughter card %spresent\n", config.daughter?"":"not " );
}




void update_chksum( hydra_conf *config )
{
	unsigned long chksum=0;
	int i;
	unsigned int temp;


	crcupdate( config->uartA.baud & 0xFF, &chksum );

	crcupdate( config->uartB.baud & 0xFF, &chksum );

	for( i=0 ; i < 4 ; i++ )
		crcupdate( (config->uartA.parity>>i) & 0xFF, &chksum );

	for( i=0 ; i < 4 ; i++ )
		crcupdate( (config->uartB.parity>>i) & 0xFF, &chksum );

	crcupdate( config->uartA.bits & 0xFF, &chksum );

	crcupdate( config->uartB.bits & 0xFF, &chksum );

	crcupdate( config->dram_size & 0xFF, &chksum );

	crcupdate( config->cpu_clock & 0xFF, &chksum );

	for( i=0 ; i < 4 ; i++ )
		crcupdate( (config->l_dram_base>>i) & 0xFF, &chksum );

	for( i=0 ; i < 4 ; i++ )
		crcupdate( (config->l_jtag_base>>i) & 0xFF, &chksum );

	crcupdate( config->daughter &= 0xFF, &chksum );

	config->checksum = chksum;
}
