#include "hydra.h"
#include "host.h"

#define done    7            /* a flag for termination of a test */
#define field   3            /* the length of each field of a control data */
#define numb   6           /* the number of ports in a DSP */
#define DSP1  3
#define NumTries 0


extern unsigned long *_stack;
unsigned long IAm, ISREntry=0;
static HostMessage *msg;
static int *IntSemaphore;
static HostIntStructure VMEInt;

void c_int01( void );
void c_int02( void );



main()
{
	unsigned long failed, mem_size, DramAddr, DramLength, test;
	MemTestStruct MemTestResults;


	/* Get processor ID, and acknowledge it */
	comm_rec( DSP1, &IAm, NumTries );
	comm_sen( DSP1, IAm, NumTries );

	comm_rec( DSP1, &test, NumTries );

	comm_rec( DSP1, &DramAddr, NumTries );
	comm_rec( DSP1, &DramLength, NumTries );

	if( test )
	{
/*
		if( !CommTestOthers() )
			exit();
*/

		/* Test DRAM */
		if( !MemTest( DramAddr, DramLength*0x100000, &MemTestResults ) )
		{
			comm_sen( DSP1, (unsigned long)MemTestResults.FailAddress, NumTries );
		}
		else
			comm_sen( DSP1, 0, NumTries );

		/* Wait for global SRAM size */
		comm_rec( DSP1, &mem_size, NumTries );

		/* Test global SRAM */
		if( !MemTest( 0xc0000000, mem_size*1024, &MemTestResults ) )
		{
			comm_sen( DSP1, (unsigned long)MemTestResults.FailAddress, NumTries );
		}
		else
			comm_sen( DSP1, 0, NumTries );

		/* Wait for local SRAM size */
		comm_rec( DSP1, &mem_size, NumTries );

		/* Test local SRAM */
		if( !MemTest( 0x40000800, (mem_size*1024) - 0x800, &MemTestResults ) )
		{
			comm_sen( DSP1, (unsigned long)MemTestResults.FailAddress, NumTries );
		}
		else
			comm_sen( DSP1, 0, NumTries );
	}

	InitHostOthers( IAm, DramAddr, DramLength );

	SetIntTable( 0x40000600 );
	SetIntVect( NMI, c_int01 );

	SetTrapTable( 0x40000600 );
	SetTrapVect( 0x7, c_int02 );
}


int CommTestOthers(void)
{
	unsigned long i_mesg, o_mesg;
	int out_port;
	int i=0;
	
	do
	{
		i = 0;
		while(!comm_rec( i, &i_mesg, 10))    /* waiting for the control word */
			if( ++i == numb )
				i=0;
		
		out_port = i_mesg & 0x7;         /* get the outgoing port number */
		if( out_port == done)           /* if the current test done ? */
			return( 1 );
		o_mesg = i_mesg >> field;       /* strip off the used port number */

		comm_sen( out_port, o_mesg, 0 );	/* send the control word */

		comm_rec( i, &i_mesg, 0 );	/* recieve the data */
		
		o_mesg=i_mesg;
		comm_sen( out_port, o_mesg, 0 );
	} while( out_port != done );      /* if the current test done ? */

	return( 1 );
}





void c_int01( void )
{
	switch( msg->WhatToDo )
	{
		case CopyStuff:
			copy( msg->Parameters );
			msg->WhatToDo = SUCCESS;
			break;
		case Run:
			msg->WhatToDo = SUCCESS;
			RunForHost( msg->Parameters[0] );
			break;
		case Halt:
			msg->WhatToDo = SUCCESS;
			halt( &_stack );
			break;					
		case HostIntNumber:
			VMEInt.IntNum = msg->Parameters[0];
			msg->WhatToDo = SUCCESS;
			break;
		case HostIntVector:
			VMEInt.IntVector = msg->Parameters[0];
			msg->WhatToDo = SUCCESS;
			break;
	}
}


void InitHostOthers( int WhoAmI, unsigned long DramAddr, unsigned long DramLength )
{
	msg = (HostMessage *)(DramAddr + (DramLength*0x100000) - (WhoAmI * sizeof(HostMessage)));

	IntSemaphore = (int *)(DramAddr + (DramLength*0x100000) - 1);
}



void c_int02( void )
{
	int i;

	i = 0;		/* Silicon bug kludge */

	wait( IntSemaphore );

	while( readVIC(0x83) & (1<<VMEInt.IntNum) );	/* Wait if interrupt is pending */

	writeVIC( 0x87 + ((VMEInt.IntNum-1) * 4), VMEInt.IntVector );	/* Set interrupt vector */

	writeVIC(  0x83, 1 | (1<<VMEInt.IntNum) );	/* Trigger interrupt */

	signal( IntSemaphore );
}


void writeVIC( unsigned long add, unsigned long data )
{
	*((unsigned long *)(((0xFFFC0000 | add) >> 2) | 0xB0000000)) = data;
}

unsigned long readVIC( unsigned long add )
{
	return( (*((unsigned long *)(((0xFFFC0000 | add) >> 2) | 0xB0000000))) &0xFF );
}



void copy( unsigned long parms[] )
{
	register unsigned long *source, *destination, count;

	source = (unsigned long *)parms[0];
	destination = (unsigned long *)parms[1];
	count = parms[2];

	while( count--)
		*destination++ = *source++;
}
