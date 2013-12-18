#include <helios.h>

WORD link[4] = { 1, 2, 4, 8 };
static WORD	*reset = 0x000000C0;

int main( WORD	argc,
	  char	**argv )
/*
 * reset a Parsytec link
 */
{
  if (argc < 2)
    {
      printf("usage: reset <link number>\n");

      exit(1);
    }

  while( argc > 1)
    {
      *reset = 0;
      *reset = 1;
      *reset = 2;
      *reset = 3;
	
      *reset = link[ atoi(argv[ --argc ]) ];
	
      Delay( 1000 );
	
      *reset = 0;
    }

  return (0);
}
