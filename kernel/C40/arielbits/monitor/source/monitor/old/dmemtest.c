unsigned long DMemTest( unsigned long *base1, unsigned long *base2, unsigned long length1, unsigned long length2 )
{
	unsigned long i, maxlength, minlength, times, fail_addr;

	maxlength = length1>length2?length1:length2;
	minlength = length1<length2?length1:length2;
	times = maxlength / minlength;

	for( i=0 ; i < times ; i++ )
		if( fail_addr = DualMem( base1+(i*minlength), base2+(i*minlength), minlength ))
			return( fail_addr );

	if( fail_addr = DualMem( base1+(times*minlength), base2+(times*minlength),  maxlength%minlength ) )
		return( fail_addr );
}