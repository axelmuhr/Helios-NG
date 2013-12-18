#include "hydra.h"

extern unsigned long breaks[MAX_BREAKS];
extern unsigned long brk_addrs[MAX_BREAKS];


int set_brk( unsigned long parms[] )
{
	int i;

	for( i=0 ; (i < MAX_BREAKS) && breaks[i] ; i++ );

	if( i == MAX_BREAKS )
	{
		c40_printf( "Exceeded maximum number of breakpoints, no breakpoint set.\n" );
		return( FAILURE );
	}

	/* Check if a break point is being set within three instructions
		of a delayed program control instruction */
	for( i=0 ; i < 3 ; i++ )
	{
		if( ((*(unsigned long *)(parms[0]-i) & BcondAF_MASK) == BcondAF)
		 || ((*(unsigned long *)(parms[0]-i) & BcondAT_MASK) == BcondAT)
		 || ((*(unsigned long *)(parms[0]-i) & BcondD_MASK) == BcondD)
		 || ((*(unsigned long *)(parms[0]-i) & BRD_MASK) == BRD)
		 || ((*(unsigned long *)(parms[0]-i) & DBcondD_MASK) == DBcondD)
		 || ((*(unsigned long *)(parms[0]-i) & RETIcondD_MASK) == RETIcondD)
		 || ((*(unsigned long *)(parms[0]-i) & RPTBDreg_MASK) == RPTBDreg)
		 || ((*(unsigned long *)(parms[0]-i) & RPTBDim_MASK) == RPTBDim)
		 )
		{
			c40_printf( "Can't insert a breakpoint at an address that is\n"
							"one of the three instructions following a delayed\n"
							"program control instruction.\n"
							"No breakpoint set.\n" );
			return( FAILURE );
		}
	}

	brk_addrs[i] = parms[0];
	breaks[i] = *(unsigned long *)brk_addrs[i];
	*(unsigned long *)brk_addrs[i] = BREAK_TRAP;

	return( SUCCESS );
}



int del_brk( unsigned long parms[] )
{
	int i;

	for( i=0 ; (i < MAX_BREAKS) && (brk_addrs[i] != parms[0]) ; i++ );

	if( i == MAX_BREAKS )
	{
		c40_printf( "Breakpoint not set at address %xh.\n", parms[0] );
		return( FAILURE );
	}

	for( ; i < MAX_BREAKS-1 ; i++ )
	{
		brk_addrs[i] = brk_addrs[i+1];
		breaks[i] = breaks[i+1];
	}

	brk_addrs[MAX_BREAKS-1] = 0;
	breaks[MAX_BREAKS-1] = 0;

	return( SUCCESS );
}




void list_brks( void )
{
	int i;

	c40_printf( "Break points at addresses:\n" );

	for( i=0 ; (i < MAX_BREAKS) && brk_addrs[i] ; i++ )
		c40_printf( "     %xh\n", brk_addrs[i] );
}
