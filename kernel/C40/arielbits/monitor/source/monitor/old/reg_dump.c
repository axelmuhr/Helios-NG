#include "hydra.h"

extern reg_set monitor_set;


void reg_dump( reg_set monitor_set )
{
	c40_printf( "\n" );
	c40_printf( "R0 => " );
	print_reg( monitor_set.DSP_ir0, monitor_set.DSP_fr0 );
	c40_printf( "     " );
	c40_printf( "R1 => " );
	print_reg( monitor_set.DSP_ir1, monitor_set.DSP_fr1 );
	c40_printf( "\n" );
	c40_printf( "R2 => " );
	print_reg( monitor_set.DSP_ir2, monitor_set.DSP_fr2 );
	c40_printf( "     " );
	c40_printf( "R3 => " );
	print_reg( monitor_set.DSP_ir3, monitor_set.DSP_fr3 );
	c40_printf( "\n" );
	c40_printf( "R4 => " );
	print_reg( monitor_set.DSP_ir4, monitor_set.DSP_fr4 );
	c40_printf( "     " );
	c40_printf( "R5 => " );
	print_reg( monitor_set.DSP_ir5, monitor_set.DSP_fr5 );
	c40_printf( "\n" );
	c40_printf( "R6 => " );
	print_reg( monitor_set.DSP_ir6, monitor_set.DSP_fr6 );
	c40_printf( "     " );
	c40_printf( "R7 => " );
	print_reg( monitor_set.DSP_ir7, monitor_set.DSP_fr7 );
	c40_printf( "\n" );
	c40_printf( "R8 => " );
	print_reg( monitor_set.DSP_ir8, monitor_set.DSP_fr8 );
	c40_printf( "     " );
	c40_printf( "R9 => " );
	print_reg( monitor_set.DSP_ir9, monitor_set.DSP_fr9 );
	c40_printf( "\n" );
	c40_printf( "R10 => " );
	print_reg( monitor_set.DSP_ir10, monitor_set. DSP_fr10 );
	c40_printf( "    " );
	c40_printf( "R11 => " );
	print_reg( monitor_set.DSP_ir11,  monitor_set.DSP_fr11 );
	c40_printf( "\n\n" );

	c40_printf( "ar0 = %x     ar1 = %x     ar2 = %x     ar3 = %x\n",
			  monitor_set.DSP_ar0, monitor_set.DSP_ar1,
			  monitor_set.DSP_ar2, monitor_set.DSP_ar3 );
	c40_printf( "ar4 = %x     ar5 = %x     ar6 = %x     ar7 = %x\n",
			  monitor_set.DSP_ar4, monitor_set.DSP_ar5,
			  monitor_set.DSP_ar6, monitor_set.DSP_ar7 );
	c40_printf( "\nDP = %x      IR0 = %x     IR1 = %x     SP = %x\n",
			  monitor_set.DSP_DP, monitor_set.DSP_IR0,
			  monitor_set.DSP_IR1, monitor_set.DSP_SP );
	c40_printf( "ST = %x      IIE = %x     IIF = %x     DIE = %x\n",
			  monitor_set.DSP_ST, monitor_set.DSP_IIE,
			  monitor_set.DSP_IIF, monitor_set.DSP_DIE );
	c40_printf( "IVTP = %x   TVTP = %x\n\n",
			  monitor_set.DSP_IVTP, monitor_set.DSP_TVTP );
}




void print_reg( unsigned long lower, unsigned long upper )
{
	char buf[9];

	c40_printf( "0x" );
	xtoa( upper, buf );
	buf[2] = '\0';
	putstr( buf );
	c40_printf( " %x = %f", lower, upper );
}
