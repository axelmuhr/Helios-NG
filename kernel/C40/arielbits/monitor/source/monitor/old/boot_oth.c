#include "hydra.h"


#define EPROM_ADDR (0x340000) /* EPROM starts at 0x300000, and is 512 Kbytes */
                              /* The boot program for the other processors */
                              /* starts at the second half of the EPROM */
                              /* and therefore starts at address */
                              /* 0x300000 + 256k=0x340000 */


int BootOthers( int Which )
{
	int blk_cnt, i, done;
	unsigned long eprom_ptr, val;


	eprom_ptr = (unsigned long)(1); /* Skip boot width */		

	/* Send Header */
	for( i=0 ; i < 2 ; i++ )
		if( !comm_sen( Which, val=ReadEprom(eprom_ptr++), NumTries) )
			return( Which + 2 );

	/* Copy all data blocks */
	for( done=FALSE ; !done ; )
	{
		/* Get the block length and check for end of data stream */
		if( !comm_sen(Which, blk_cnt=ReadEprom(eprom_ptr++), NumTries) )
			return( Which + 2 );
		if( blk_cnt == 0 )
		{
			done = TRUE;
			continue;
		}

		/* Send destination address */
		if( !comm_sen(Which, ReadEprom(eprom_ptr++), NumTries) )
			return( Which + 2 );

		/* Send data block */
		while( blk_cnt-- )
		{
			if( !comm_sen(Which, ReadEprom(eprom_ptr++), NumTries) )
				return( Which + 2 );
		}
	}

	/* Send Trailer */
	for( i=0 ; i < 3 ; i++ )
		if( !comm_sen(Which, ReadEprom(eprom_ptr++), NumTries) )
			return( Which + 2 );

	/* Send Processor ID's, and wait for acknowledgement */
	if( !comm_sen( Which, Which+2, NumTries ) )
		return( Which + 2 );
	if( !comm_rec( Which, &val, NumTries ) )
		return( Which + 2 );
	if( val != Which+2 )
		return( Which + 2 );

	return( 0 );
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