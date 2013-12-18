#include	<stdio.h>
#include	<syslib.h>
#include	<stdlib.h>
#include	<link.h>
#include 	<nonansi.h>
#include	"b422cons.h"
#include	"b422err.h"
#include	"b422pcol.h"
#include	"b422fns.h"

#define DSIZE	0x10000


static int   	count;
static BYTE	data[DSIZE];
static BYTE	message[MAX_SCSI_MESSAGE_SIZE];
static BYTE	sense_data[MAX_SCSI_MESSAGE_SIZE];
static BYTE	msglen, result;
static INT16	exestat;
static INT32	logical_block_address, block_length;



static void msel()
{
	INT32 blen;
	
	printf("Block length? ");
	fflush(stdout);
	scanf("%d", &blen);
	printf("\n");

	msglen   = 0;
	
	data[0]  = 0;
	data[1]  = 0;
	data[2]  = 0;
	data[3]  = 0x08;
	data[4]  = 0;
	data[5]  = 0;
	data[6]  = 0;
	data[7]  = 0;
	data[8]  = 0;
	data[9]  = getbyte(blen, 3);
	data[10] = getbyte(blen, 2);
	data[11] = getbyte(blen, 1);

	scsi_mode_select_6(2,
			   2,
			   0,
			   0,
			   0x0c,
			   0,
			   data,
            	           message,
	    	           &msglen,
	    	           &result,
	    	           &exestat);

	printf("mode select: execute = %d, scsi = %d\n",
	       exestat, result);
}
			   
			   
			   
static void msen()
{
	BYTE i;
	
	msglen   = 0;
	
	scsi_mode_sense_6(2,
			  2,
			  0,
			  0,
			  0x3f,
			  0x5c,
			  0,
			  data,
            	          message,
	    	          &msglen,
	    	          &result,
	    	          &exestat);

	printf("mode sense: execute = %d, scsi = %d\n",
	       exestat, result);
	
	for (i = 0; i < data[0]; i++) 
		printf("%2x%c", data[i], 
		       ((i % 10 == 9) || (i == data[0])) ? '\n' : ' ');
}
			   
			   
			   
static void inq()
{
	msglen = 0;

	scsi_inquiry_6(2, 
	    	       2,
	    	       0,
	    	       0,
	    	       0,
	    	       0,
	    	       0,
	    	       0x24,
	    	       0,
	   	       data,
            	       message,
	    	       &msglen,
	    	       &result,
	    	       &exestat);

	printf("inquiry: execute = %d, scsi = %d\n",
	       exestat, result);

	printf("revision: %d\n", data[2]);
	printf("vendor  : %c%c%c%c%c%c%c%c\n", data[8], data[9], data[10], 
		data[11], data[12], data[13], data[14], data[15]);
	printf("product : %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
		data[16], data[17], data[18], data[19], data[20],
		data[21], data[22], data[23], data[24], data[25], 
		data[26], data[27], data[28], data[29], data[30], data[31]);
	printf("h/w rev : %d\n", data[32]);
	printf("f/w rev : %d\n", data[33]);
	printf("rom rev : %d\n", data[34]);
}



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


static void fmt()
{
		msglen = 0;	

		scsi_format_6(2,
			      2,
			      0,
			      0,
			      0,
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

	printf("scsi_format_6: execute = %d, scsi = %d\n",
	       exestat, result);
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


static void r6()
{
	msglen = 0;	

	scsi_read_6(2,
		    2,
		    0,
		    0,
		    0x001,
		    0x008,
		    block_length,
		    0,
		    data,
		    message,
		    &msglen,
		    &result,
		    &exestat);		

	printf("scsi_read_6: execute = %d, scsi = %d\n",
       	exestat, result);

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



static void w6()
{
	msglen = 0;	

	scsi_write_6(2,
		     2,
		     0,
		     0,
		     0x001,
		     0x008,
		     block_length,
		     0,
		     data,
		     message,
		     &msglen,
		     &result,
		     &exestat);		

	printf("scsi_write_6: execute = %d, scsi = %d\n",
	       exestat, result);
}



int main()
{
	int 		res, i;
	
	printf("\n\nB422 SCSI TRAM test program\n\n");
	
	/* Initialise link to TRAM */
	if ((res = b422_init(2)) != TRUE) {
		printf("b422_init returned 0x%x\n", res);
		return(FALSE);
	}
	printf("*** TRAM link initialised ***\n");

	/* Reset the SCSI bus */
	b422_reset(2, B422_SCSI_ID);
	printf("*** Reset SCSI bus ***\n");

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
		printf("0.  Exit\n");
		printf("1.  Inquiry\n");
		printf("2.  Capacity\n");
		printf("3.  Mode_select\n");
		printf("4.  Mode_sense\n");
		printf("10. Read_10\n");
		printf("11. Read_6\n");
		printf("12. Write_10\n");
		printf("13. Write_6\n");
		printf("90. Format\n? ");
		fflush(stdout);
		scanf("%d", &res);
		printf("\n");
	
		if (res == 1)
			inq();
		else if (res == 2)
			cap();
		else if (res == 3)
			msel();
		else if (res == 4)
			msen();
		else if (res == 10)
			r10();
		else if (res == 11)
			r6();
		else if (res == 12)
			w10();
		else if (res == 13)
			w6();
		else if (res == 90)
			fmt();
			
	} while (res != 0);
	
		
	/* Terminate link to TRAM					*/
	b422_term(2);
	printf("*** Closed link to TRAM ***\n");
		
	/* Successful completion 					*/
	printf("*** Successful program completion ***\n");
	return(TRUE);
}


