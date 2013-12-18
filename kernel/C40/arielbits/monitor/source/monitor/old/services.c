#include "hydra.h"



void compare( unsigned long parms[] )
{
	if( parms[2] == 0xFFFFFFFF )
		return;
	
	while( parms[2]-- )
		if( *((unsigned long *)parms[0]++) != *((unsigned long *)parms[1]++) )
		{
			c40_printf( "Compare fault at address [%x].\n", --parms[0] );
			return;
		}

	c40_printf( "Compare successfull.\n" );
}




void dump( unsigned long parms[] )
{
	int i;

	if( parms[1] == 0xFFFFFFFF )
		return;

	while( parms[1] )
	{
		c40_printf( "\n[%x]   ", parms[0] );
		for( i=0 ; i < 4 ; i++ )
		{
			c40_printf( "%x  ", *((unsigned long *) parms[0]++) );
			if( !(--parms[1]) )
				break;
		}
	}
	c40_printf( "\n\n" );
}




void fill( unsigned long parms[] )
{
	if( parms[1] == 0xFFFFFFFF )
		return;

	while( parms[1]-- )
		*((unsigned long *) parms[0]++) = parms[2];
}





void enter( unsigned long parms[] )
{
	char inval[9];
	int i, j, ok;
	unsigned long xval;


	if( parms[0] == 0xFFFFFFFF )
		return;

	while( 1 )
	{
		c40_printf( "[%x]  %x : ", parms[0], *((unsigned long *)parms[0]) );


		j=0;
		while( (j < 8) && ((c40_putchar(inval[j]=c40_getchar()))!='\n') )
		{
			if( inval[j++] == '\b' )
				if( j == 1 )
					j -= 1;
				else 
					j -= 2;
		}

		if( inval[1] == '\n' )
		{
			break;
		}

		if( j == 8 )
			c40_printf( "\n" );

		inval[j] = '\0';

		*((unsigned int *)parms[0]++) = atox( inval, &ok );

		if( ok == FAILURE )
		{
			c40_printf( "Not a valid number %s.\n", inval );
			break;
		}

		if( !(--parms[1]) )
			break;
	}

	c40_printf( "\n\n" );
}





void search( unsigned long parms[] )
{
	if( parms[1] == 0xFFFFFFFF )
		return;

	while( parms[0] != parms[1] )
		if( *((unsigned long *)parms[0]++) == parms[2] )
		{
			c40_printf( "%x found at address %x.\n", parms[2], --parms[0] );
			return;
		}

	c40_printf( "%x not found.\n", parms[2] );
}


void version( void )
{
	c40_printf( "\n\n     Hydra_mon version %s\n\n", VERSION );
}



void help()
{
	c40_printf( "\n\nHydra_Mon Command summary :\n"
					"  Test =>              t\n"
					"  Configure =>         cf\n"
					"  Dump =>              d  start_addr #(of addresses to dump)\n"
					"  Enter =>             e  address\n"
					"  Fill =>              f  start_addr #(of addresses to fill) value\n"
					"  Copy =>              cp start1 start2 #(of addresses to copy)\n"
					"  Compare =>           c  start1 start2 #(of addresses to compare)\n"
					"  Search =>            s  from_address to_address for_data\n"
					"  Version =>           v\n"  );
/*					"  Set Breakpoint =>    sbp address\n"
					"  Delete Breakpoint => dbp address\n"
					"  List Breakpoints  => lbp\n"
					"  Single Step =>       ss\n"
					"  Go =>                g   (address, blank to continue)\n" 
					"  Reg Dump =>          rd\n\n" );
*/
}
