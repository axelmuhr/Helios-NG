#include "hydra.h"

#define EPROM_ADDR (0x30d000) /* EPROM starts at 0x300000, and is 512 Kbytes */
                              /* The boot program for the other processors */
                              /* starts at the second half of the EPROM */
                              /* and therefore starts at address */
                              /* 0x300000 + 256k=0x340000 */
/*#define EPROM_ADDR (0x340000) /* EPROM starts at 0x300000, and is 512 Kbytes */
                              /* The boot program for the other processors */
                              /* starts at the second half of the EPROM */
                              /* and therefore starts at address */
                              /* 0x300000 + 256k=0x340000 */
#define DSP_2 (0)
#define DSP_3 (1)
#define DSP_4 (2)


int BootOthers( hydra_conf config, int booted[] )
{
	int blk_cnt, i, done;
	unsigned long eprom_ptr, val;


	eprom_ptr = (unsigned long)(1); /* Skip boot width */		

	/* Send Header */
	for( i=0 ; i < 2 ; i++ )
	{
		if( !comm_sen(DSP_2, val=ReadEprom(eprom_ptr++), NumTries) )
			return( 2 );
		if( config.daughter )
		{
			if( !comm_sen( DSP_3, val, NumTries ) )
				return( 3 ); 
			if( !comm_sen( DSP_4, val, NumTries ) )
				return( 4 );
		}
	}

	/* Copy all data blocks */
	for( done=FALSE ; !done ; )
	{
		/* Get the block length and check for end of data stream */
		if( !comm_sen(DSP_2, blk_cnt=ReadEprom(eprom_ptr++), NumTries) )
			return( 2 );
		if( blk_cnt == 0 )
		{
			done = TRUE;
			continue;
		}

		/* Send destination address */
		if( !comm_sen(DSP_2, val=ReadEprom(eprom_ptr++), NumTries) )
			return( 2 );
		if( config.daughter )
		{
			if( !comm_sen( DSP_3, val, NumTries ) )
				return( 3 ); 
			if( !comm_sen( DSP_4, val, NumTries ) )
				return( 4 );
		}

		/* Send data block */
		while( blk_cnt-- )
		{
			if( !comm_sen(DSP_2, val=ReadEprom(eprom_ptr++), NumTries) )
				return( 2 );
			if( config.daughter )
			{
				if( !comm_sen( DSP_3, val, NumTries ) )
					return( 3 ); 
				if( !comm_sen( DSP_4, val, NumTries ) )
					return( 4 );
			}
		}
	}

	/* Send Trailer */
	for( i=0 ; i < 3 ; i++ )
	{
		if( !comm_sen(DSP_2, val=ReadEprom(eprom_ptr++), NumTries) )
			return( 2 );
		if( config.daughter )
		{
			if( !comm_sen( DSP_3, val, NumTries ) )
				return( 3 ); 
			if( !comm_sen( DSP_4, val, NumTries ) )
				return( 4 );
		}
	}

	/* Send Processor ID's, and wait for acknowledgement */
	if( !comm_sen( DSP_2, 2, NumTries ) )
		return( 2 );
	if( !comm_rec( DSP_2, &val, NumTries ) )
		return( 2 );
	if( val != 2 )
		return( 2 );
	booted[0] = TRUE;
	
	if( config.daughter )
	{
		if( !comm_sen( DSP_3, 3, NumTries ) )
			return( 3 ); 
		if( !comm_rec( DSP_3, &val, NumTries ) )
			return( 3 );
		if( val != 3 )
			return( 3 );
		booted[1] = TRUE;

		if( !comm_sen( DSP_4, 4, NumTries ) )
			return( 4 );
		if( !comm_rec( DSP_4, &val, NumTries ) )
			return( 4 );
		if( val != 4 )
			return( 4 );
		booted[2] = TRUE;
	}
}


static unsigned long ReadEprom( unsigned long WordAddr )
{
	unsigned long data, i;

	for( i=0, data=0, WordAddr*=4 ; i < 4 ; i++, WordAddr++ )
		data |= (*(unsigned long *)(EPROM_ADDR + WordAddr ) & 0xFF)
		        << (i * 8);

	return( data );
}
