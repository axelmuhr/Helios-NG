#include <stdio.h>
#include <stdlib.h>
#include <root.h>
#include <link.h>

int
main(
     int 	argc,
     char **	argv )
{
#ifdef __C40
  int 	ln = 0;
#else
  int 	ln = 3;
#endif
  char	x[ 2 ];
	

  if (argc >1)
    ln = atoi(argv[ 1 ] );

  if (AllocLink( ln ) <0)
    {
      extern int oserr;

      
      puts( "Error in AllocLink()" );
      
      fprintf( stderr, "oserr = %x\n", oserr );
      
      return EXIT_FAILURE;
    }
  
  
  for (;;)
    {
      LinkIn( 1, ln, x, -1 );
      
      putchar( *x );
    }

  return EXIT_SUCCESS;
}
