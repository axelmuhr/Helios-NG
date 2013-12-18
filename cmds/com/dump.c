
static char *rcsid = "$Header: /hsrc/cmds/com/RCS/dump.c,v 1.3 1992/07/31 09:16:55 nickc Exp $"; 

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <nonansi.h>


void
printhex( char c )
{
  if (c < 10)
    putchar( c + '0' );
  else
    putchar( c - 10 + 'A' );
}


void
dump( FILE * f )
{
  int		i;
  int		buffer[ 16 ];
  unsigned	offset = 0;


  for (;;)
    {
      for (i = 0; i < 16; i++)
	buffer[ i ] = fgetc( f );

      if (buffer[ 0 ] == -1)
	break;	/* done				*/

      printf( "%04x: ", offset );

      for (i = 0; i < 16; i++)
	if (buffer[ i ] != -1)
	  {
	    char c = buffer[ i ];


	    printhex( c >> 4 );
	    printhex( c & 0xf);
	    
	    putchar( ' ' );
	  }
	else
	  printf( "   " );

      printf( "   " );

      for (i = 0; i < 16; i++)
	{
	  if (buffer[ i ] != -1)
	    {
	      if (!isprint( buffer[ i ] ))
		buffer[ i ] = '.';
	      
	      putchar( buffer[ i ] );
	    }
	  else
	    putchar( ' ' );
	}
      
      putchar( '\n' );

      offset += 16;
    }

  return;
  
} /* dump */
  

int
main(
     int 	argc,
     char *	argv[] )
{
  FILE *	f;


  if (argc < 2)
    {
      f = freopen( Heliosno( stdin )->Name, "rb", stdin );

      if (f == NULL)
	{
	  fprintf( stderr, "dump: failed to reopen stdin\n" );

	  return EXIT_FAILURE;	  
	}
      
      dump( f );
    }
  else
    {
      int	i;

      
      for (i = 1; i < argc; i++)
	{
	  f = fopen(argv[ i ], "rb" );		/* open file for binary read	*/

	  if (f == NULL)
	    {
	      fprintf( stderr, "dump: Can't open file '%s'\n", argv[ i ] );

	      return EXIT_FAILURE;
	    }

	  dump( f );

	  fclose( f );	  
	}
    }

  return EXIT_SUCCESS;

  rcsid = rcsid;
  
} /* main */

