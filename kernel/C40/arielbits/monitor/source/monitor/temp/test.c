#include "hydra.h"
#include <math.h>

#define done     7             /* a flag for termination of a test */
#define nu_patn  4             /* number of data pattens */
#define nu_rout  6             /* number of routes being tested */
#define numb     3             /* number of connected ports in DSP1 */


int booted[3] = { FALSE, FALSE, FALSE };


int test( hydra_conf config, char out_where )
{
	unsigned long *mem_ptr;
	unsigned long i, j, k, sram_size;
	unsigned long fail_addr, failed=FALSE;
	MemTestStruct MemResults;


	c40_printf( "\n\n" );

	LED( RED, OFF );
	LED( GREEN, OFF );

	/* Test DRAM */
	c40_printf( "Testing DRAM .... " );
	if( MemTest( 0x8d000000, config.dram_size*0x100000, &MemResults ) )
	{
		c40_printf( "Failed at address %x during the %s test.\n", MemResults.FailAddress, MemResults.TestFailed?"Write Address":"Walking Bits" );
		c40_printf( "Written value  = %x, Read value = %x\n", MemResults.Written, MemResults.Read );
		LED( RED, ON );
		return( 0 );
	}
	else
		c40_printf( "Passed\n" );

	c40_printf( "Testing processor 1 ....\n" );
	/* Test outer SRAM */
	c40_printf( "   Global SRAM ... " );
	if( MemTest( 0xc0000000, config.sram1_size*1024, &MemResults ) )
	{
		c40_printf( "Failed at address %x during the %s test.\n", MemResults.FailAddress, MemResults.TestFailed?"Write Address":"Walking Bits" );
		c40_printf( "Written value  = %x, Read value = %x\n", MemResults.Written, MemResults.Read );
		LED( RED, ON );
		failed = TRUE;
	}
	else
		c40_printf( "Passed\n" );

return( 1 );

	/* Test other 3 DSP's */

	CommFlush( config );

	if( !reset_others() )
	{
		c40_printf( "Error Writing Hydra control register.\n" );
		LED( RED, ON );
		failed = TRUE;
	}

	/* Boot other processor(s) with boot code from EPROM */
	c40_printf( "Booting other processor%s ... ", config.daughter?"s":"" );
	if( i = BootOthers( config, booted ) )
	{
		c40_printf( "Failed booting processor %d\n", i );
		LED( RED, ON );
		failed = TRUE;
		return(0);
	}
	else
		c40_printf( "Successful\n" );	

	if( config.daughter )
		k = 3;
	else
		k = 1;
	/* Test memory on other processor(s) */
	for( i=0 ; i < k ; i++ )
	{
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
		comm_sen( i, config.dram_size, NumTries );
		comm_sen( i, config.l_dram_base, NumTries );

		for( j=0 ; j < 2 ; j++ )
		{
			c40_printf( "Testing %s SRAM on DSP %d ... ", j?"local":"global", 2+i );
			comm_rec( i, &fail_addr, (int)1e6 );
			if( fail_addr )
			{
				failed = TRUE;
				c40_printf( "Failed\n     Failed at address %x.\n", fail_addr );
			}
			else
				c40_printf( "Passed\n" );
		}
	}


	c40_printf( "\n\n" );

	if( failed )
	{
		/* Indicate failure */
		LED( GREEN, OFF );
		LED( RED, ON );		
		return( 0 );
	}
	else
	{
		/* Indicate success */
		LED( GREEN, ON );
		LED( RED, OFF );
		return( 1 );
	}
}



int reset_others( void )
{
	unsigned long leds, i;

	leds = (*(unsigned long *)0xbf7fc008) & 0x80200000;

	/* Setup everything but reset */
	*(unsigned long *)(0xbf7fc008) = 0x005FFE49 | leds;
	if( *(unsigned long *)(0xbf7fc008) != (0x005FFE49|leds))
		return( 0 );

	for( i=0 ; i < 1000 ; )
		i++;

	/* Now everything */
	*(unsigned long *)(0xbf7fc008) = 0x705FFE49 | leds;
	if( *(unsigned long *)(0xbf7fc008) != (0x705FFE49|leds))
		return( 0 );

	return( 1 );	
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
				return( 0 );
		}
		if (!comm_sen(out_port,done, NumTries))      /* send the "done" flag to DSP2 */
			return( 0 );
		
		return( 1 );
	}						  	
}




void CommFlush( hydra_conf config )
{
	unsigned long ContRegVal, i, val;

	ContRegVal = (*(unsigned long *)(config.l_jtag_base + 8));


	/* Wait until the Comm Ports are empty */
	if( config.daughter )
		i = 3;
	else
		i = 1;
	while( i --)
	{
		if( !booted[i] )
			continue;

		/* Set the NMI high */
		*(unsigned long *)(config.l_jtag_base + 8) = ContRegVal | (0x00040000<<i);

		/* Now set them low, thusly triggering an interrupt */
		*(unsigned long *)(config.l_jtag_base + 8) = ContRegVal & ~(0x00040000<<i);

		/* Set the NMI high again */
		*(unsigned long *)(config.l_jtag_base + 8) = ContRegVal | (0x00040000<<i);

		/* Flush input section */
		while( *(unsigned long *)(0x100040+(i*0x10)) & 0x00001E00 )
			val = *(unsigned long *)(0x100041+(i*0x10));

		/* Output garbage to insure that DSP 1 owns the token */
		*(unsigned long *)(0x100042+(i*0x10)) = 0;

		/* Wait until the FIFO is empty */
		while( *(unsigned long *)(0x100040+(i*0x10)) & 0x000001E0 );

		/* Now set them low, thusly triggering an interrupt */
		*(unsigned long *)(config.l_jtag_base + 8) = ContRegVal & ~(0x00040000<<i);

		/* Set the NMI high again */
		*(unsigned long *)(config.l_jtag_base + 8) = ContRegVal | (0x00040000<<i);
	}


}
