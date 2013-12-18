main( void )
{
	unsigned long fail_addr;


	if( fail_addr = DMemTest(0x40000000,0xc0000000,16000,16000) )
		return( 0 );
	else
		return( 1 );
}

