#include "hydra.h"
#include <math.h>

#define done     7             /* a flag for termination of a test */
#define nu_patn  4             /* number of data pattens */
#define nu_rout  6             /* number of routes being tested */
#define numb     3             /* number of connected ports in DSP1 */


int test( hydra_conf config )
{
	unsigned long *mem_ptr;
	unsigned long i, j, k, sram_size;
	unsigned long fail_addr, failed=FALSE;
	unsigned long parms[3], dram, g_sram=0xc0000000, length=0x4000;
	MemTestStruct MemTestResults;


	dram = config.l_dram_base;

	c40_printf( "\n\n" );

	LED( RED, OFF, config );
	LED( GREEN, OFF, config );

	/* Test DRAM */
	c40_printf( "Testing DRAM .... " );
	if( !MemTest( 0x8d000000, config.dram_size*0x100000, &MemTestResults ) )
	{
		c40_printf( "Failed at address %x\n\n\n", MemTestResults.FailAddress );
		LED( RED, ON, config );
		writeVIC( 0x7b, readVIC(0x7b) | 0x40 );  /* Assert SYSFAIL */
		return( 0 );
	}
	else
		c40_printf( "Passed\n" );

	c40_printf( "Testing processor 1 ....\n" );
	/* Test outer SRAM */
	c40_printf( "   Global SRAM ... " );
	parms[0] = g_sram;
	parms[1] = dram;
	parms[2] = length;
	copy( parms );
	if( !MemTest( 0xc0000000, config.sram1_size*1024, &MemTestResults ) )
	{
		parms[0] = dram;
		parms[1] = g_sram;
		parms[2] = length;
		copy( parms );
		c40_printf( "Failed at address %x\n\n\n", MemTestResults.FailAddress );
		LED( RED, ON, config );
		writeVIC( 0x7b, readVIC(0x7b) | 0x40 );  /* Assert SYSFAIL */
		failed = TRUE;
	}
	else
	{
		parms[0] = dram;
		parms[1] = g_sram;
		parms[2] = length;
		copy( parms );

		c40_printf( "Passed\n" );
	}


	/* Test other 3 DSP's */

	if( !CommFlush( config, DSP_2 ) )
	{
		c40_printf( "Error booting DSP 2.\nAborting testing.\n" );
		writeVIC( 0x7b, readVIC(0x7b) | 0x40 );  /* Assert SYSFAIL */
		return( 0 );
	}
	if( config.daughter )
	{
		if( !CommFlush( config, DSP_3 ) )
		{
			c40_printf( "Error booting DSP 3.\nAborting testing.\n" );
			writeVIC( 0x7b, readVIC(0x7b) | 0x40 );  /* Assert SYSFAIL */
			return( 0 );
		}
		if( !CommFlush( config, DSP_4 ) )
		{
			c40_printf( "Error booting DSP 4.\nAborting testing.\n" );
			writeVIC( 0x7b, readVIC(0x7b) | 0x40 );  /* Assert SYSFAIL */
			return( 0 );
		}
	}

	reset_others( config, DSP_2 );
	if( config.daughter )
	{
		reset_others( config, DSP_3 );
		reset_others( config, DSP_4 );
	}

	/* Boot other processor(s) with boot code from EPROM */
	c40_printf( "Booting other processor%s ... ", config.daughter?"s":"" );
	if( i = BootOthers( DSP_2 ) )
	{
		c40_printf( "Failed booting processor 2\n\n\n", i );
		LED( RED, ON, config );
		failed = TRUE;
		writeVIC( 0x7b, readVIC(0x7b) | 0x40 );  /* Assert SYSFAIL */
		return(0);
	}
	else
	{
		/* Indicate that test will be performed */
		comm_sen( DSP_2, YES, NumTries );
	}
	
	if( config.daughter )
	{
		if( i = BootOthers( DSP_3 ) )
		{
			c40_printf( "Failed booting processor 3\n\n\n" );
			LED( RED, ON, config );
			failed = TRUE;
			writeVIC( 0x7b, readVIC(0x7b) | 0x40 );  /* Assert SYSFAIL */
			return(0);
		}
		else
		{
			/* Indicate that test will be performed */
			comm_sen( DSP_3, YES, NumTries );
		}

		if( i = BootOthers( DSP_4 ) )
		{
			c40_printf( "Failed booting processor 4\n\n\n" );
			LED( RED, ON, config );
			failed = TRUE;
			writeVIC( 0x7b, readVIC(0x7b) | 0x40 );  /* Assert SYSFAIL */
			return(0);
		}
		else
		{
			/* Indicate that test will be performed */
			comm_sen( DSP_4, YES, NumTries );
		}
	}
	c40_printf( "Successful\n" );	

	/* Test Comm port connections on the Hydra card */
/*
	c40_printf( "Testing Comm Ports ... " );
	if( !CommTest( config ) )
	{
		c40_printf( "Failed\n" );
		LED( RED, ON, config );
		writeVIC( 0x7b, readVIC(0x7b) | 0x40 );  /* Assert SYSFAIL */
/*		failed = TRUE;
	}
	else
		c40_printf( "Passed\n" );
*/

	if( config.daughter )
		k = 3;
	else
		k = 1;
	/* Test memory on other processor(s) */
	for( i=0 ; i < k ; i++ )
	{
		/* Send DRAM size and base address */
		if( !comm_sen( i, config.l_dram_base, NumTries ) )
			return( FAILURE );
		if( !comm_sen( i, config.dram_size, NumTries ) )
			return( FAILURE );

		c40_printf( "Testing DRAM from DSP %d ... ", 2+i );
		if( !comm_rec( i, &fail_addr, NumTries ) )
		{
			c40_printf( "\n   DSP %d communication time out.\n", 2+i );
			fail_addr = 0;
		}
		if( fail_addr )
		{
			failed = TRUE;
			c40_printf( "Failed\n     Failed at address %x.\n", fail_addr );
			writeVIC( 0x7b, readVIC(0x7b) | 0x40 );  /* Assert SYSFAIL */
		}
		else
			c40_printf( "Passed\n" );

		for( j=0 ; j < 2 ; j++ )
		{
			c40_printf( "Testing %s SRAM on DSP %d ... ", j?"local":"global", 2+i );
			switch( i )
			{
				case 0 :
					comm_sen( 0, config.sram2_size, NumTries );
					break;
				case 1 :
					comm_sen( 1, config.sram3_size, NumTries );
					break;
				case 2 :
					comm_sen( 2, config.sram4_size, NumTries );
					break;
			}
			if( !comm_rec( i, &fail_addr, NumTries ) )
			{
				c40_printf( "\n   DSP %d communication time out.\n", 2+i );
				fail_addr = 0;
			}
			if( fail_addr )
			{
				failed = TRUE;
				c40_printf( "Failed\n     Failed at address %x.\n", fail_addr );
				writeVIC( 0x7b, readVIC(0x7b) | 0x40 );  /* Assert SYSFAIL */
			}
			else
				c40_printf( "Passed\n" );
		}
	}

	c40_printf( "\n\n" );

	if( failed )
	{
		/* Indicate failure */
		LED( GREEN, OFF, config );
		LED( RED, ON, config );		
		c40_printf( "\n\n" );
		writeVIC( 0x7b, readVIC(0x7b) | 0x40 );  /* Assert SYSFAIL */
		return( 0 );
	}
	else
	{
		/* Indicate success */
		LED( GREEN, ON, config );
		LED( RED, OFF, config );
		return( 1 );
	}
}




int CommTest( hydra_conf config )
{
	unsigned long data[nu_patn]={0x0,0xFFFFFFFF,0xAAAAAAAA,0x55555555};   /* data to be sent */

	unsigned long  out_2=3;         /* the route ID when without the daughter card attached */
	unsigned long   rout[nu_rout]={0053,0143,043,003,003,043};   /* route IDs in octal */

	unsigned long   i_mesg;         /* the received message */
	int  port[numb]={0,1,2};        /* the connected port numbers in DSP1 */
	int  out_port=0;

	int i,j,k=0;

	if (config.daughter)             /* Test All Four Processors */
	{
    
	 	for (i=0;i<nu_rout;i++)
 		{  
			if (!fmod(i,2))               /* if it is a forword or backword test ? */
				out_port=0;
			else 
				out_port=k;
			for ( j=0;j<nu_patn;j++)
			{
				if (!comm_sen(out_port,rout[i], NumTries))   /* send the control word out */
					return(0);
				while (!comm_rec(port[k],&i_mesg, NumTries))   /* wait for the control word coming back */
				{
					k++;
					if (k==numb)
						k=0;
				}
  				if (!comm_sen(out_port,data[j], NumTries))   /* send the data out */	
					return(0);
			
				while (!comm_rec(port[k],&i_mesg, NumTries));   /* dump the control word and get the data coming back */

				if (i_mesg!=data[j])       /* is there any error in the received data */
					return(0);
			}
		}
 	 
		for (i=0;i<numb;i++)
			if (!comm_sen(i,done, NumTries))      /* send the "done" flag to all other DSPs */
				return(0);

		return(1);  
	}
	else            /* Only test DSP 2 */
	{
		for ( j=0;j<nu_patn;j++)
		{
			if (!comm_sen(out_port,out_2, NumTries))   /* send the control word out */
				return( 0 );

	                if( !comm_rec(out_port,&i_mesg, NumTries) )
				return( 0 );   /* Get the control word coming back */

			if( !comm_sen(out_port,data[j], NumTries))   /* send the data out */
				return( 0 );

			if( !comm_rec(out_port,&i_mesg, NumTries) )
				return( 0 );   /* dump the control word and get the data coming back */
       
			if (i_mesg!=data[j])       /* is there any error in the received data */
			{
				if (!comm_sen(out_port,done, NumTries))      /* send the "done" flag to DSP2 */
					return( 0 );
				return( 0 );
			}
		}
		if (!comm_sen(out_port,done, NumTries))      /* send the "done" flag to DSP2 */
			return( 0 );
		
		return( 1 );
	}						  	
}




