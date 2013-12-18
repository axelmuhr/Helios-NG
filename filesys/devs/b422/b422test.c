#include	<stdio.h>
#include	<syslib.h>
#include	<stdlib.h>
#include	<link.h>
#include 	<nonansi.h>
#include	"b422def.h"
#include	"b422fns.h"

#define DSIZE	0x10000


static int   	count;
static BYTE	data[DSIZE];
static BYTE	message[MAX_SCSI_MESSAGE_SIZE];
static BYTE	sense_data[MAX_SCSI_MESSAGE_SIZE];
static BYTE	msglen, result;
static INT16	exestat;
static INT32	logical_block_address, block_length;


static void cap()
{
	msglen = 0;

	scsi_read_capacity_10(
		2,
		2,
		0,
		0,
		0,
		FALSE,
		0,
		0,
		0,
		0,
		0,
		0,
		data,
		message,
		&msglen,
		&result,
		&exestat);
	
	printf("scsi_read_capacity_10: execute = %d, scsi = %d\n",
	       exestat, result);
	
	if (exestat == 0) {
		logical_block_address = data[0] * 0x1000000 +
		      			data[1] * 0x10000   +
		      			data[2] * 0x100     +
		      			data[3];
			      
		block_length  = data[4] * 0x1000000 +
		      		data[5] * 0x10000   +
		      		data[6] * 0x100     +
		      		data[7];
			      
		printf("%d blocks of %d bytes = capacity of %d bytes\n",
			logical_block_address, block_length,
			logical_block_address * block_length);
	}
}	



static void r10()
{
	INT32	blks, badd, i;
	
	printf("Blocks & address (Hex)? ");
	fflush(stdout);
	scanf("%x %x", &blks, &badd);
	printf("\n");

	for (i = 0; i < (blks * block_length); i++)
		data[i] = 0xff;

	msglen = 0;	

	scsi_read_10(2,
		     2,
		     0,
		     0,
		     badd,
		     blks,
		     block_length,
		     0,
		     data,
		     message,
		     &msglen,
		     &result,
		     &exestat);		

	printf("scsi_read_10: execute = %d, scsi = %d\n",
       	exestat, result);

	for (i = 0; i < (blks * block_length); i++)
		printf("%2x%c", data[i], (i % 16 == 15) ? '\n' : ' ');
}



static void w10()
{
	INT32	blks, badd, i;
	
	printf("Blocks & address (Hex)? ");
	fflush(stdout);
	scanf("%x %x", &blks, &badd);
	printf("\n");

	for (i = 0; i < (blks * block_length); i+=4) {
		data[i]     = i / block_length;
		data[i + 1] = i / 256;
		data[i + 2] = i % 256;
		data[i + 3] = badd % 256;
	}
		
	msglen = 0;	

	scsi_write_10(2,
		      2,
		      0,
		      0,
		      (INT64)badd,
		      /*0x008,*/blks,
		      block_length,
		      0,
		      data,
		      message,
		      &msglen,
		      &result,
		      &exestat);		

	printf("scsi_write_10: execute = %d, scsi = %d\n",
	       exestat, result);
}



static void w10xn()
{
	INT32	blks, badd, i, count;
	
	printf("Blocks, address & count (Hex)? ");
	fflush(stdout);
	scanf("%x %x %x", &blks, &badd, &count);
	printf("\n");

	for (i = 0; i < (blks * block_length); i+=4) {
		data[i]     = i / block_length;
		data[i + 1] = i / 256;
		data[i + 2] = i % 256;
		data[i + 3] = badd % 256;
	}

	i = 1;
	
	do {
				
		msglen = 0;	
	
		scsi_write_10(2,
			      2,
			      0,
			      0,
			      (INT64)badd,
			      /*0x008,*/blks,
			      block_length,
			      0,
			      data,
			      message,
			      &msglen,
			      &result,
			      &exestat);		

		printf("scsi_write_10 x %d: execute = %d, scsi = %d\n",
	       		i, exestat, result);
	} while ((i++ < count) && (exestat == 0));
	
	if (exestat != 0) {
	
		msglen = 0;
		scsi_request_sense_6(
			2,
			2,
			0,
			0,
			18,
			0,
			sense_data,
			message,
			&msglen,
			&result,
			&exestat);

		printf("scsi_request_sense: execute = %d, scsi = %d\n",
		       exestat, result);
	       
		msglen = 0;
		scsi_test_unit_ready_6(
			2,
			2,
			0,
			0,
			0,
			message,
			&msglen,
			&result,
			&exestat);

		printf("scsi_test_unit_ready: execute = %d, scsi = %d\n",
	       	       exestat, result);
				       
		/* Reset the SCSI bus */
		b422_reset(
			2, 
			ENABLE_PARITY_CHECKING,
			ENABLE_PARITY_GENERATION,
			/* FAST_CABLE_MODE, */
			SLOW_CABLE_MODE,
			DEFAULT_DATA_PHASE_TIME_OUT,
			DEFAULT_INTERRUPT_TIME_OUT,
			B422_SCSI_ID,
			/* SCSI_FAST_DMA_MODE); */
			SCSI_SLOW_DMA_MODE);
		printf("*** Reset SCSI bus ***\n");

		/* Wait until the unit is ready */
		do { 
			msglen = 0;
			scsi_request_sense_6(
				2,
				2,
				0,
				0,
				18,
				0,
				sense_data,
				message,
				&msglen,
				&result,
				&exestat);

			printf("scsi_request_sense: execute = %d, scsi = %d\n",
			       exestat, result);
	       
			msglen = 0;
			scsi_test_unit_ready_6(
				2,
				2,
				0,
				0,
				0,
				message,
				&msglen,
				&result,
				&exestat);

			printf("scsi_test_unit_ready: execute = %d, scsi = %d\n",
	       	    	   exestat, result);
				       
		} while (exestat != 0);

	}
}



int main()
{
	int	res, i;
	
	printf("\n\nB422 SCSI TRAM test program\n\n");
	
	/* Initialise link to TRAM */
	if ((res = b422_init(2, TRUE)) != TRUE) {
		printf("b422_init returned 0x%x\n", res);
		return(FALSE);
	}
	printf("*** TRAM link initialised ***\n");

	/* Reset the SCSI bus */
	b422_reset(
		2, 
		ENABLE_PARITY_CHECKING,
		ENABLE_PARITY_GENERATION,
		/* FAST_CABLE_MODE, */
		SLOW_CABLE_MODE,
		DEFAULT_DATA_PHASE_TIME_OUT,
		DEFAULT_INTERRUPT_TIME_OUT,
		B422_SCSI_ID,
		/* SCSI_FAST_DMA_MODE); */
		SCSI_SLOW_DMA_MODE);
	printf("*** Reset SCSI bus ***\n");

	/* Wait until the unit is ready */
	do { 
		msglen = 0;
		scsi_request_sense_6(
			2,
			2,
			0,
			0,
			18,
			0,
			sense_data,
			message,
			&msglen,
			&result,
			&exestat);

		printf("scsi_request_sense: execute = %d, scsi = %d\n",
		       exestat, result);
	       
		msglen = 0;
		scsi_test_unit_ready_6(
			2,
			2,
			0,
			0,
			0,
			message,
			&msglen,
			&result,
			&exestat);

		printf("scsi_test_unit_ready: execute = %d, scsi = %d\n",
	       	       exestat, result);
				       
	} while (exestat != 0);

	printf("\n*** SCSI device ready for use ***\n");

	cap(); /* This sets block length */
	
	do { /* SCSI commands */
			
		/* Initialise block count */
		count = 0;

		/* Initialise data */	
		for (i = 0; i < DSIZE; i++)
			data[i] = 0xaa;

		/* Find out which type of operation to perform */
		printf("\nCommands:\n");
		printf("0. Exit\n");
		printf("1. Read_10\n");
		printf("2. Write_10\n");
		printf("3. Write_10 x n\n? ");
		fflush(stdout);
		scanf("%d", &res);
		printf("\n");
	
		if (res == 1)
			r10();
 		else if (res == 2)
			w10();
		else if (res == 3)
			w10xn();
			
	} while (res != 0);
	
		
	/* Terminate link to TRAM					*/
	b422_term(2);
	printf("*** Closed link to TRAM ***\n");
		
	/* Successful completion 					*/
	printf("*** Successful program completion ***\n");
	return(TRUE);
}


