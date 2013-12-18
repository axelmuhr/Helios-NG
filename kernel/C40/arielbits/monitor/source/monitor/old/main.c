#include "hydra.h"

extern hydra_conf config;      /* Holds the EEPROM configuration information */
reg_set monitor_set, call_set;  /* These structures holds all CPU registers */
										  /* while calling user programs              */
unsigned long breaks[MAX_BREAKS]; /* Holds break point insrtuctions */
unsigned long brk_addrs[MAX_BREAKS]; /* Holds break point addresses */

extern int key_ptr;     /* Pointer into the keyboard buffer */
extern void c_int01( void );
extern void c_int14( void );
extern void c_int15( void );

unsigned short crctab[256];

main( void )
{
	unsigned long val;
	int i, j;
	

	GIEOff();

	SetIntTable( 0x40000c00 );
	SetIntVect( NMI, c_int01 );

	SetIntVect( IIOF2, c_int14 );
	EnableInt( IIOF2 );

	SetTrapTable( 0x40000c00 );
	SetTrapVect( 0x7, c_int15 );

	ClearIIOF();

	GIEOn();

	SetupVICVACDefault();

	c40_printf( "\n\n\n\n\n" );

	config.l_jtag_base = 0xbf7fc000;

	if( !(readVAC( 0x19 ) & 0x200) )   /* Check if we need to restore factory parameters */
	{
		for( j=0 ; j < 2 ; j++ )
		{
			LED( GREEN, ON, config );
			LED( RED, ON, config );
			for( i=0 ; i < 0x80000 ; i++ );

			LED( GREEN, OFF, config );
			LED( RED, OFF, config );
			for( i=0 ; i < 0x80000 ; i++ );
		}
		
		c40_printf( "     Restoring factory default parameters.\n     Release switch to proceed.\n" );
		while( !(readVAC( 0x19 ) & 0x200) );  /* Wait for button release */
		restore( config );
		for( j=0 ; j < 2 ; j++ )
		{
			LED( GREEN, ON, config );
			LED( RED, ON, config );
			for( i=0 ; i < 0x80000 ; i++ );

			LED( GREEN, OFF, config );
			LED( RED, OFF, config );
			for( i=0 ; i < 0x80000 ; i++ );
		}
		c40_printf( "\n\nParameters restored\n\n" );
	}

	read_config( &config ); 
	setupVICVAC( config );

	for( j=0 ; j < 2 ; j++ )
	{
		LED( GREEN, ON, config );
		LED( RED, OFF, config );
		for( i=0 ; i < 0x80000 ; i++ );		

		LED( GREEN, OFF, config );
		LED( RED, ON, config );
		for( i=0 ; i < 0x80000 ; i++ );
	}
	
	LED( GREEN, OFF, config );
	LED( RED, OFF, config );

	mk_crctbl( CRC16, crchware );

	key_ptr = 0;

	zero_regs( &monitor_set );
	zero_regs( &call_set );

	for( i=0 ; i < MAX_BREAKS ; i++ ) /* Reset all breakpoints */
	{
		breaks[i] = 0;
		brk_addrs[i] = 0;
	}

	init();    /* Turns Cache on */

	writeVIC( 0x7b, readVIC(0x7b) & ~(0x40) );  /* Deassert SYSFAIL */
	test( config );

	InitHost( config );

	c40_printf( "%c", BEEP );

	version();

	c40_printf( "     Ariel_ Hydra revision %c\n\n\n", config.revision );

	monitor( &config );
}





void zero_regs( reg_set *regs )
{
	regs->DSP_ir0 = 0;
	regs->DSP_ir1 = 0;
	regs->DSP_ir2 = 0;
	regs->DSP_ir3 = 0;
	regs->DSP_ir4 = 0;
	regs->DSP_ir5 = 0;
	regs->DSP_ir6 = 0;
	regs->DSP_ir7 = 0;
	regs->DSP_ir8 = 0;
	regs->DSP_ir9 = 0;
	regs->DSP_ir10 = 0;
	regs->DSP_ir11 = 0;

	regs->DSP_fr0 = 0x7FFF0000;
	regs->DSP_fr1 = 0x7FFF0000;
	regs->DSP_fr2 = 0x7FFF0000;
	regs->DSP_fr3 = 0x7FFF0000;
	regs->DSP_fr4 = 0x7FFF0000;
	regs->DSP_fr5 = 0x7FFF0000;
	regs->DSP_fr6 = 0x7FFF0000;
	regs->DSP_fr7 = 0x7FFF0000;
	regs->DSP_fr8 = 0x7FFF0000;
	regs->DSP_fr9 = 0x7FFF0000;
	regs->DSP_fr10 = 0x7FFF0000;
	regs->DSP_fr11 = 0x7FFF0000;

	regs->DSP_ar0 = 0;
	regs->DSP_ar1 = 0;
	regs->DSP_ar2 = 0;
	regs->DSP_ar3 = 0;
	regs->DSP_ar4 = 0;
	regs->DSP_ar5 = 0;
	regs->DSP_ar6 = 0;
	regs->DSP_ar7 = 0;

	regs->DSP_DP = 0;
	regs->DSP_IR0 = 0;
	regs->DSP_IR1 = 0;
	regs->DSP_BK = 0;
	regs->DSP_SP = 0;

	regs->DSP_ST = 0;
	regs->DSP_DIE = 0;
	regs->DSP_IIE = 0;
	regs->DSP_IIF = 0;

	regs->DSP_RS = 0;
	regs->DSP_RE = 0;
	regs->DSP_RC = 0;

	regs->DSP_IVTP = 0;
	regs->DSP_TVTP = 0;

	regs->ret_add = 0;
}



void SetMICR( unsigned long global, unsigned long local )
{
	*(unsigned long *)(0x100000) = global;
	*(unsigned long *)(0x100004) = local;
}
