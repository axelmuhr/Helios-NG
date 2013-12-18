#define RED	1
#define GREEN	2
#define ON	1
#define OFF	0

unsigned long delay=0x80000;

main()
{
	int i;

	GIEOn();

	while( 1 )
	{
		LED( RED, ON );
		LED( GREEN, OFF );

		for( i=0 ; i < delay ; i++ );

		LED( RED, OFF );
		LED( GREEN, ON );

		for( i=0 ; i < delay ; i++ );

	}
}