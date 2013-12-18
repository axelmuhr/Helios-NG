#include "hydra.h"

main()
{
	unsigned long fail_addr;
	MemTestStruct MemResult;

	while( 1 )
		if( MemTest( 0x8d000000, 0x100000, &MemResult ) )
			exit( 0 );
}