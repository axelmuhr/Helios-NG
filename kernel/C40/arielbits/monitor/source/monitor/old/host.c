#include "hydra.h"
#include "host.h"

static HostMessage *msg;
static HostDataBuff HostBuff;

HostIntStructure VMEInt;

hydra_conf config;
extern unsigned long *_stack;
extern int EEPROM_Status;

void c_int14( void )
{
	int i;

	writeVIC( 0x5F, 0 );	/* Deassert interrupt */

	IACK( 0x20 );

	if( i = readVIC( 0x63 ) )	/* Check for GetProperty or SetProperty first */
	{
		writeVIC( 0x63, 0 );  /* Acknowledge data received */

		switch( i )
		{
			case SetProperty:
				while( !(i=readVIC(0x63)) );   /* Wait for property code */
				writeVIC( 0x63, 0 );  /* Acknowledge data received */
				switch( i & 0xF0 )
				{
					case UartABaud:
						config.uartA.baud = ReadHostWord() & 0xFFFF;
						write_config( config );
						break;
					case UartAParity:
						config.uartA.parity = ReadHostWord();
						write_config( config );
						break;
					case UartABits:
						config.uartA.bits = ReadHostWord() & 0xFF;
						write_config( config );
						break;
					case UartBBaud:
						config.uartB.baud = ReadHostWord() & 0xFFFF;
						write_config( config );
						break;
					case UartBParity:
						config.uartB.parity = ReadHostWord();
						write_config( config );
						break;
					case UartBBits:
						config.uartB.bits = ReadHostWord() & 0xFF;
						write_config( config );
						break;
					case DRAMSize:
						config.dram_size = ReadHostWord() & 0xFF;
						write_config( config );
						break;
					case CpuClock:
						config.cpu_clock = ReadHostWord() & 0xFF;
						write_config( config );
						break;
					case SRAM1Size:
						config.sram1_size = ReadHostWord() & 0xFFFF;
						write_config( config );
						break;
					case SRAM2Size:
						config.sram2_size = ReadHostWord() & 0xFFFF;
						write_config( config );
						break;
					case SRAM3Size:
						config.sram3_size = ReadHostWord() & 0xFFFF;
						write_config( config );
						break;
					case SRAM4Size:
						config.sram4_size = ReadHostWord() & 0xFFFF;
						write_config( config );
						break;
					case DRAMBase:
						config.l_dram_base = ReadHostWord();
						write_config( config );
						break;
					case HostJTAGBase:
						WriteEepromWord( 13, ReadHostWord(), config );
						write_config( config );
						break;
					case LocalJTAGBase:
						WriteEepromWord( 25, ReadHostWord(), config );
						write_config( config );
						break;
					case Daughter:
						config.daughter = ReadHostWord() & 0xFF;
						write_config( config );
						break;
					case IPCRBase:
						WriteEepromWord( 0x19, ReadHostWord() & 0xFFFF, config );
						break;
					case Hardware:
						WriteEeprom( 79, config.revision=ReadHostWord(), config.l_jtag_base + 8 );
						break;
				}
				break;
			case GetProperty:
				while( !(i=readVIC(0x63)) );   /* Wait for property code */
				switch( i )
				{
					case UartABaud:
						WriteHostWord( config.uartA.baud );
						writeVIC( 0x63, SUCCESS );
						break;
					case UartAParity:
						WriteHostWord( config.uartA.parity );
						writeVIC( 0x63, SUCCESS );
						break;
					case UartABits:
						WriteHostWord( config.uartA.bits );
						writeVIC( 0x63, SUCCESS );
						break;
					case UartBBaud:
						WriteHostWord( config.uartB.baud );
						writeVIC( 0x63, SUCCESS );
						break;
					case UartBParity:
						WriteHostWord( config.uartB.parity );
						writeVIC( 0x63, SUCCESS );
						break;
					case UartBBits:
						WriteHostWord( config.uartB.bits );
						writeVIC( 0x63, SUCCESS );
						break;
					case DRAMSize:
						WriteHostWord( config.dram_size );
						writeVIC( 0x63, SUCCESS );
						break;
					case CpuClock:
						WriteHostWord( config.cpu_clock );
						writeVIC( 0x63, SUCCESS );
						break;
					case SRAM1Size:
						WriteHostWord( config.sram1_size );
						writeVIC( 0x63, SUCCESS );
						break;
					case SRAM2Size:
						WriteHostWord( config.sram2_size );
						writeVIC( 0x63, SUCCESS );
						break;
					case SRAM3Size:
						WriteHostWord( config.sram3_size );
						writeVIC( 0x63, SUCCESS );
						break;
					case SRAM4Size:
						WriteHostWord( config.sram4_size );
						writeVIC( 0x63, SUCCESS );
						break;
					case DRAMBase:
						WriteHostWord( config.l_dram_base );
						writeVIC( 0x63, SUCCESS );
						break;
					case HostJTAGBase:
						WriteHostWord( (readVAC(0x1)<<16)&0xFFFF0000 );
						writeVIC( 0x63, SUCCESS );
						break;
					case LocalJTAGBase:
						WriteHostWord( (readVAC(7)<<16)&0xFFFF0000 );
						writeVIC( 0x63, SUCCESS );
						break;
					case Daughter:
						WriteHostWord( config.daughter );
						writeVIC( 0x63, SUCCESS );
						break;
					case IPCRBase:
						WriteHostWord( readVAC(0x04) );
						writeVIC( 0x63, SUCCESS );
						break;
					case BoardName:
						WriteHostWord( BOARD );
						writeVIC( 0x63, SUCCESS );
						break;
					case Firmware:
						WriteHostWord( FIRMWARE_REV );
						writeVIC( 0x63, SUCCESS );
						break;
					case Hardware:
						WriteHostWord( config.revision );
						writeVIC( 0x63, SUCCESS );
						break;
				}
				break;
			default:
				return;
		}

		writeVIC( 0x63, 0 );  /* Clear handshake register */
	}
	else
	{
		switch( msg->WhatToDo )
		{
			case BootADsp:
				if( !CommFlush( config, msg->Parameters[0] ) )
				{
					msg->WhatToDo = FAILURE;
					break;
				}
				reset_others( config, msg->Parameters[0] );
				if( !BootOthers( msg->Parameters[0] ) )
					msg->WhatToDo = SUCCESS;
				else
				{
					msg->WhatToDo = FAILURE;
					break;
				}
				/* Indicate that testing will not be performed */
				comm_sen( msg->Parameters[0], NO, NumTries );
				comm_sen( msg->Parameters[0], config.l_dram_base, NumTries );
				comm_sen( msg->Parameters[0], config.dram_size, NumTries );
				break;
			case CopyStuff:
				copy( msg->Parameters );
				msg->WhatToDo = SUCCESS;
				break;			
			case Run:
				msg->WhatToDo = SUCCESS;
				RunForHost( msg->Parameters[0] );
				break;
			case Halt:
				msg->WhatToDo = SUCCESS;
				halt( &_stack );
				break;					
			case HostIntNumber:
				VMEInt.IntNum = msg->Parameters[0];
				msg->WhatToDo = SUCCESS;
				break;
			case HostIntVector:
				VMEInt.IntVector = msg->Parameters[0];
				msg->WhatToDo = SUCCESS;
				break;
			default:
				return;
		}
	}
}




int InitHost( hydra_conf config )
{

	/* Set host message pointer */
	msg = (HostMessage *)(config.l_dram_base + (config.dram_size*0x100000) - sizeof(HostMessage) );

	/* Zero interrupt semaphore */
	msg->InterruptSemaphore = 1;

	/* Clear out IPCR's */
	writeVIC( 0x63, 0 );
	writeVIC( 0x67, 0 );
	writeVIC( 0x6B, 0 );
	writeVIC( 0x6F, 0 );
	writeVIC( 0x73, 0 );

}



unsigned long ReadHostWord( void )
{
	unsigned long val, i;

	for( i=0, val=0 ; i < 4 ; i++ )
	{
		while( !(readVIC(0x63)) );   /* Wait for data */
		
		val |= readVIC( 0x67 );

		writeVIC( 0x63, 0 );  /* Acknowledge data received */
	}

	return( val );
}



void WriteHostWord( unsigned long val )
{
	unsigned long i;

	for( i=0 ; i < 4 ; i++ )
	{
		writeVIC( 0x67, (val >> (8*i)) & 0xFF );  /* Write data */

		writeVIC( 0x63, 0 );  /* Write send acknowledge */

		while( !(readVIC(0x63)) );   /* Wait for receive acknowledgement */
	}
}



void copy( unsigned long parms[] )
{
	register unsigned long *source, *destination, count;

	source = (unsigned long *)parms[0];
	destination = (unsigned long *)parms[1];
	count = parms[2];

	while( count--)
		*destination++ = *source++;
}

void writeVIC( unsigned long add, unsigned long data )
{
	*((unsigned long *)(((0xFFFC0000 | add) >> 2) | 0xB0000000)) = data;
}

unsigned long readVIC( unsigned long add )
{
	return( (*((unsigned long *)(((0xFFFC0000 | add) >> 2) | 0xB0000000))) &0xFF );
}

unsigned long readVAC( unsigned long add )
{
	return( (*((unsigned long *)(( ((0xFFFD0000 | (add << 8)) >> 2) | 0xB0000000))) >> 16) & 0xFFFF );
}



void IACK( unsigned long ipl )
{
	unsigned long tcr;
	char dummy;

	tcr = readTCR();		/* Save old TCR */
	writeTCR( ipl );		/* Set LA1 high */
	dummy = *((char *) 0xbfffffc1 );	/* IACK */
	writeTCR( tcr );			/* restore old TCR */
}





int write_config( hydra_conf config )
{
	unsigned long val;


	val = ReadEepromWord( 61, config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	switch( config.uartA.baud )
	{
		case 150:
			val &= 0xE3FF;
			val |= 0x0000;
			break;
		case 300:
			val &= 0xE3FF;
			val |= 0x0400;
			break;
		case 600:
			val &= 0xE3FF;
			val |= 0x0800;
			break;
		case 1200:
			val &= 0xE3FF;
			val |= 0x0C00;
			break;
		case 2400:
			val &= 0xE3FF;
			val |= 0x1000;
			break;
		case 4800:
			val &= 0xE3FF;
			val |= 0x1400;
			break;
		case 9600:
			val &= 0xE3FF;
			val |= 0x1800;
			break;
		case 19200:
			val &= 0xE3FF;
			val |= 0x1c00;
			break;
	}

	if( config.uartA.parity > 0 )
		if( config.uartA.parity % 2 )
		{
			val |= 0x8000;
			val &= 0xBFFF;
		}
		else
		{
			val |= 0xC000;
		}
	else
		if( config.uartA.parity % 2 )
		{
			val &= 0x3FFF;
		}
		else
		{
			val |= 0x4000;
			val &= 0x7FFF;
		}
	if( config.uartA.bits == 7 )
		val &= 0xDFFF;
	else
		val |= 0x2000;
	WriteEepromWord( 61, val, config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}

	val = ReadEepromWord( 63, config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	switch( config.uartB.baud )
	{
		case 150:
			val &= 0xE3FF;
			val |= 0x0000;
			break;
		case 300:
			val &= 0xE3FF;
			val |= 0x0400;
			break;
		case 600:
			val &= 0xE3FF;
			val |= 0x0800;
			break;
		case 1200:
			val &= 0xE3FF;
			val |= 0x0C00;
			break;
		case 2400:
			val &= 0xE3FF;
			val |= 0x1000;
			break;
		case 4800:
			val &= 0xE3FF;
			val |= 0x1400;
			break;
		case 9600:
			val &= 0xE3FF;
			val |= 0x1800;
			break;
		case 19200:
			val &= 0xE3FF;
			val |= 0x1c00;
			break;
	}

	if( config.uartB.parity > 0 )
		if( config.uartB.parity % 2 )
		{
			val |= 0x8000;
			val &= 0xBFFF;
		}
		else
		{
			val |= 0xC000;
		}
	else
		if( config.uartB.parity % 2 )
		{
			val &= 0x3FFF;
		}
		else
		{
			val |= 0x4000;
			val &= 0x7FFF;
		}
	if( config.uartB.bits == 7 )
		val &= 0xDFFF;
	else
		val |= 0x2000;
	WriteEepromWord( 63, val, config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}

	WriteEepromWord( 67, config.dram_size, config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	WriteEepromWord( 69, config.daughter, config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	WriteEepromWord( 71, config.sram1_size, config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	WriteEepromWord( 73, config.sram2_size, config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	WriteEepromWord( 75, config.sram3_size, config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
	WriteEepromWord( 77, config.sram4_size, config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}

	switch( config.cpu_clock )
	{
		case 40:
			WriteEepromWord( 59, 66 << 8, config );
			if( !EEPROM_Status )
			{
				return( 0 );
			}
			break;
		case 50:
			WriteEepromWord( 59, 82 << 8, config );
			if( !EEPROM_Status )
			{
				return( 0 );
			}
			break;
		default:
			WriteEepromWord( 59, 66 << 8, config );
			if( !EEPROM_Status )
			{
				return( 0 );
			}
			break;
	}


	val = (config.l_dram_base & 0x7FFFFFFF) >> 14;
	WriteEepromWord( 17, val, config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}

	val = (config.l_jtag_base & 0x7FFFFFFF) >> 14;
	WriteEepromWord( 25, val, config );
	if( !EEPROM_Status )
	{
		return( 0 );
	}

	WriteEeprom( 79, config.revision, config.l_jtag_base + 8 );
	if( !EEPROM_Status )
	{
		return( 0 );
	}
}




void WriteEepromWord( int WordAddress, unsigned long data, hydra_conf config )
{
	int i;
	
	for( i=0 ; i < 2 ; i++, data >>= 8 )
	{
		WriteEeprom( WordAddress++, data & 0xff, config.l_jtag_base + 8 ); 
		if( !EEPROM_Status )
		{
			return;
		}
	}
}




unsigned long ReadEepromWord( int WordAddress, hydra_conf config )
{
	unsigned long word;
	int i, j;

	for( i=0, word=0L ; i < 2 ; i++ )
	{
		word |= ((j=ReadEeprom( WordAddress++, config.l_jtag_base + 8 )) << (i*8));
		if( !EEPROM_Status )
		{
			return( 0 );
		}
	}

	return( word );
}





void reset_others( hydra_conf config, int Which )
{
	unsigned long i;

	/* Assert processor's reset */
	*(unsigned long *)(config.l_jtag_base + 8) &= ~( 0x10000000 << Which );

	/* Kill time */
	for( i=0 ; i < 100 ; )
		i++;

	/* Setup everything but reset */
 	/* Set ROM enable and boot vector */
	*(unsigned long *)(config.l_jtag_base + 8) |= ( 0xE01 << (Which*3) );
	*(unsigned long *)(config.l_jtag_base + 8) |= ( 0x40000 << Which );

	/* Kill time */
	for( i=0 ; i < 100 ; )
		i++;

	/* Now everything */
	/* Set the reset line high */
	*(unsigned long *)(config.l_jtag_base + 8) |= (0x10000000 << Which );

	/* Kill time */
	for( i=0 ; i < 100 ; )
		i++;
}





int CommFlush( hydra_conf config, int Which )
{
	int i, TimeOut;

	/*
	 *	The idea here is to put the processor on the other end of the comm port into
	 *	reset, and then take it out of reset with the NMI asserted.  The processor wil
	 *	immediately take the interrupt and will end up in a dead loop in the internal
	 *	ROM.  while all this is going on, its comm port will flush the data from our 
	 *	comm port.
	 */

	/* Halt input channel */
	*(unsigned long *)(0x100040+(Which*0x10)) = 0x8;
	/* Flush input section */
	while( *(unsigned long *)(0x100040+(Which*0x10)) & 0x00001E00 )
		i = *(int *)(0x100041+(Which*0x10));

	/* There is a danger here of loosing the token.  At reset, DSP 1 is in output mode, */
	/* and the other DSP's are in input mode.  If we reset one of the other DSP's while */
	/* it is in output mode, then the token is lost forever, or until board reset, whichever  */
	/* comes first.  To avoid this catastrofic event, we will put garbage data into the */
	/* output fifo to force arbitration for the token. */

	/* Do this only if the other DSP is running */
	if( *(unsigned long *)(config.l_jtag_base + 8) & ( 0x10000000 << Which ) )
	{
		/* Put garbage data into output fifo (if it is not full) to insure arbitration.  */
		if( !( *(unsigned long *)(0x100040+(0x10*Which)) & 0x100 ) )
			*(unsigned long *)(0x100042+(Which*0x10)) = 0;
		/* Now wait for the token to show up */
		for( TimeOut=0 ; (TimeOut < COMM_FLUSH_TIMEOUT) && ((*(unsigned long *)(0x100040+(0x10*Which))) & 0x4) ; TimeOut++ );
		if( TimeOut == COMM_FLUSH_TIMEOUT )
			return( FAILURE ); 
/*		while( (*(unsigned long *)(0x100040+(0x10*Which))) & 0x4 ); */
	}

	/* Assert the other processor's reset */
	*(unsigned long *)(config.l_jtag_base + 8) &= ~( 0x10000000 << Which );

	/* Setup everything but reset */
 	/* Set ROM enable and boot vector */
	*(unsigned long *)(config.l_jtag_base + 8) |= ( 0xE01 << (Which*3) );

	/* Deassert NMI */
	*(unsigned long *)(config.l_jtag_base + 8) |=  (0x40000 << Which );

	/* Kill time */
	for( i=0 ; i < 100 ; )
		i++;

	/* Deassert the other processor's reset line */
	*(unsigned long *)(config.l_jtag_base + 8) |= (0x10000000 << Which );

	/* Kill time */
	for( i=0 ; i < 100 ; )
		i++;

	/* Assert NMI */
	*(unsigned long *)(config.l_jtag_base + 8) &= ~( 0x40000 << Which );

	/* Kill time */
	for( i=0 ; i < 100 ; )
		i++;

	/* Deassert NMI */
	*(unsigned long *)(config.l_jtag_base + 8) |=  (0x40000 << Which );

	/* Put garbage data into output fifo (if it is not full) to insure arbitration */
	if( !( *(unsigned long *)(0x100040+(0x10*Which)) & 0x100 ) )
		*(unsigned long *)(0x100042+(Which*0x10)) = 0;

	/* Wait until the FIFO is empty */
	for( TimeOut=0 ; (TimeOut < COMM_FLUSH_TIMEOUT) && ((*(unsigned long *)(0x100040+(0x10*Which))) & 0x4) ; TimeOut++ );
	if( TimeOut == COMM_FLUSH_TIMEOUT )
		return( FAILURE ); 
/*	while( *(unsigned long *)(0x100040+(Which*0x10)) & 0x000001E0 ); */

	/* Reassert the other processor's reset */
	*(unsigned long *)(config.l_jtag_base + 8) &= ~( 0x10000000 << Which );

	/* Unhalt input channel */
	*(unsigned long *)(0x100040+(Which*0x10)) = 0x0;

	return( SUCCESS );
}




void c_int15( void )
{
	int i;

	i = 4;				/* Silicon bug kludge */

	if( VMEInt.IntNum == 0 )	/* Check if host interrupts are enabled */
		return;

	wait( &msg->InterruptSemaphore );

	while( readVIC(0x83) & (1<<VMEInt.IntNum) );	/* Wait if interrupt is pending */

	writeVIC( 0x87 + ((VMEInt.IntNum-1) * 4), VMEInt.IntVector );	/* Set interrupt vector */

	writeVIC(  0x83, 1 | (1<<VMEInt.IntNum) );	/* Trigger interrupt */

	signal( &msg->InterruptSemaphore );
}
