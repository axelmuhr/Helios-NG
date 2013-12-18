#include <stdio.h>

int
main( void )
{
	double a = 6.0;
	double b = 3.0;
	double c = a / b;

	printf( "a = %f, b = %f, c = %f\n", a, b, c );
	printf( "a = %x %x, b = %x %x, c = %x %x\n", a, b, c );

	return 0;
}
