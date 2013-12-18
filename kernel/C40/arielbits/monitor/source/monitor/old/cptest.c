#include "hydra.h"


#define DSP2	(0)


void CopyTest()
{
	int i;

	for( i=0 ; i < 100 ; i++ )
		*(unsigned long *)(0x8d000000 + i) = i;

	comm_sen( DSP2, CopyStuff, 0 );
	comm_sen( DSP2, 0x8d000000, 0 );
	comm_sen( DSP2, 0xc0000000, 0 );
	comm_sen( DSP2, 100, 0 );
}