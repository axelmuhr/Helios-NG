#define DSP1  3
#define NumTries 0

int IAm, ISREntry=0;

/* Global control variables used to communicate between the main program */
/* and the comm port interrupt service routine */
int MemTestType=SRAMTest, 
     MemTestFailure=FALSE, 
     EndTesting=FALSE;

void c_int01( void );
void c_int02( void );



main()
{
	int i;
	unsigned long failed,fail_addr, SRAMsize, DRAMsize, DRAMaddr;

	SetIntTable( 0x2ffc00 );	/* Set NMI for Comm Port Flush service */
	SetIntVect( 1, c_int01 );

	/* Get processor ID, and acknowledge it */
	comm_rec( DSP1, &IAm, NumTries );
	comm_sen( DSP1, IAm, NumTries );

	/* Wait for SRAM size */
	comm_rec( DSP1, &SRAMsize, NumTries );

	/* Wait for DRAM size */
	comm_rec( DSP1, &DRAMsize, NumTries );

	/* Wait for DRAM address */
	comm_rec( DSP1, &DRAMaddr, NumTries );

	/* Test global SRAM */
	if( fail_addr = MemTest( 0xc0000000, SRAMsize*1024 ) )
	{
		comm_sen( DSP1, fail_addr, NumTries );
	}
	else
		comm_sen( DSP1, 0, NumTries );		

	/* Test local SRAM */
	if( fail_addr = MemTest( 0x40000000, SRAMsize*1024 ) )
	{
		comm_sen( DSP1, fail_addr, NumTries );
	}
	else
		comm_sen( DSP1, 0, NumTries );	


	for( i=0 ; i < 6 ; i++ )	/* Set up all Comm Port ICRDY vectors for Comm Port test */
		SetIntVect( (0xe + (i*4)),  c_int02 );
	OrIntMask( 0x444444 );   /* Enable all ICRDY interrupts */

	while( !EndTesting )
	{
		if( MemTestType == SRAMTest )
		{
			/* Test SRAM */
			if( fail_addr = DMemTest( 0xc0000000, 0x40000000, SRAMsize*1024, SRAMsize*1024 ) )
			{
				comm_sen( DSP1, CONTROL, NumTries );
				comm_sen( DSP1, MemoryTestFailure, NumTries );
			}
		}
		else
		{
			/* Test DRAM */
			if( fail_addr = MemTest( DRAMaddr, DRAMsize*1024 ) )
			{
				comm_sen( DSP1, CONTROL, NumTries );
				comm_sen( DSP1, MemoryTestFailure, NumTries );
			}
		}
	}
}


void c_int02( void )
{
	CommMessage Message;
	int out_port;
	int i=0;
	
	for( i=0 ; i < 6 ; i++ )
		if( comm_rec( i, &Message.header, 1) )    /* Get the first available control word */
		{
			comm_rec( i, &Message.data, 0 ); /* Get message data */
			break;    /* Exit for loop */
		}
	if( i == 6 )
		return;

	if( Message.header == CONTROL )
	{
		switch( Message.data )
		{
			case MemTestSwitch :
				MemTestType =
				    MemTestType==SRAMTEST?DRAMTest:DRAMTest;
				break;
			case End :
				EndTesting = TRUE;
				break;
		}
	}
	else
	{  
		/* Send the new header */
		comm_sen( (Message.header&0x7), Message.header>>3, 0 );
		/* Send the data */
		comm_sen( (Message.header&0x7), Message.data, 0 ); 
	}	

}



void c_int01( void )
{
	unsigned long val;


	if( ISREntry == 0 )
	{
		ISREntry = 1;

		while( ISREntry == 1 )
			val = *(unsigned long *)0x100071;
		
	}
	else
		ISREntry = 0;
}