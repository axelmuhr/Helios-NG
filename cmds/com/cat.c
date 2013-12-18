/***************************************************************************
 ***************************************************************************
 **	CAT COMMAND for HELIOS                                            **
 **	Russell Bradford - University of Bath				  **
 **	2 March 1988	                                                  **
 ***************************************************************************
 ***************************************************************************/

/* Revision history:
 * Started somtime in the past
 * Brought up to spec of SUN man page 2 March 88
 * Helios-ized by John Fitch 19 April 1988
 * Stdin to work with redirection, and use flags  PAB 19/5/88
 * Opened file in text mode to stop appending of ^m on end of line JMP 07/11/89
 * Checked equality of input and output to give warning, BLV 14/4/91
 */

/* Notes:
 *   Extras are multiple flags, eg 'cat -sv foo', rather than 'cat -s -v foo';
 *   also, eg 'cat foo -n foo' will cat two copies of foo, the second with line
 *   numbers.
 *   A bit more fussy about catting non-files
 */

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/cat.c,v 1.6 1992/10/12 15:39:34 bart Exp $";

#include <stdio.h>
#include <unistd.h>
#include <syslib.h>
#include <gsp.h>
#include <string.h>
#include <nonansi.h>
#include <servlib.h>

long uflag = FALSE;
long nflag = FALSE;
long sflag = FALSE;
long vflag = FALSE;
long bflag = FALSE;
long tflag = FALSE;
long eflag = FALSE;
long new_line = FALSE;


/* output a char, taking into account the -s, -n, -b, or -e flags */

void
out_char( char ch )
{
  static int 	squish_count = 0;
  static int 	line_count   = 1;
  char 		num[ 10 ];
  int 		i;

  
  if (sflag) 			/* squish multiple newlines */
    {
      if (ch == '\n')
	{
	  squish_count++;
	  
	  if (squish_count > 2)
	    return;
	}
      else
	{
	  squish_count = 0;
	}
    }

  if (nflag && new_line) 		/* number a new line */
    {
      if (!bflag || ch != '\n') 	/* but not if it is blank */
	{
	  sprintf( num, "%6d  ", line_count++ );

	  for (i = 0; i < 8; i++)
	    putchar( num[ i ] );
	}

      new_line = FALSE;
    }

  if (ch == '\n')
    {
      if (eflag)
	putchar( '$' );	/* put a $ at EOL */

      new_line = TRUE;
    }
  
  putchar( ch );

  return;
  
} /* out_char */


/* output an ASCII character, prefacing it with ^ or M- as necessary */

void
out_escaped_char( char ch )
{
  if (ch & 0200)
    {
      out_char( 'M' );
      out_char( '-' );
      
      ch &= 0177;			/* clear top bit */
    }

  if ((ch >= ' ' && ch < 0177) || ch == '\n')
    {
      out_char( ch );
    }
  else if (ch == 0177) 	/* ch = delete */
    {
      out_char( '^' );
      out_char( '?' );
    }
  else 			/* control char */
    {
      if (ch == '\t' && !tflag)
	{
	  out_char( ch );
	}
      else
	{
	  out_char( '^' );
	  out_char( ch + '@' );
	}
    }

  return;
  
} /* out_escaped */

    
/* output a buffer, determining whether to translate control chars */

void
out(
    char *	buf,
    long 	nbytes )
{
  long 		i;


  if (vflag)			/* translate output */
    for (i = 0; i < nbytes; i++)
      out_escaped_char( buf[ i ] );
  else
    for (i = 0; i < nbytes; i++)
      out_char( buf[ i ] );

  return;
  
} /* out */


/* open, copy, and close a file */

void
copy_file( char * f )
{
  byte 		buf[ BUFSIZ ];
  word 		bytes;
  word		i;
  FILE   *	fd;
  Object *	o;


  if ((o = Locate( CurrentDir, f )) == NULL)
    {
      fprintf( stderr, "cat: Can't find '%s'\n", f );

      return;
    }

  unless (o->Type & Type_File)
    {
      fprintf( stderr, "cat: Can't cat '%s': not a file\n", f );
      
      return;
    }

  /* BLV - check for cat'ing a file to itself.	*/
  
  if (!isatty( 1 ))
    if (!strcmp( o->Name, Heliosno( stdout )->Name ))
      {
	fprintf( stderr, "cat: input %s is output\n", objname( o->Name ) );

	return;
      }
    
  if ((fd = fopen( f, "r" )) == NULL)
    {
      fprintf( stderr, "cat: Can't access '%s'\n", f );

      return;
    }

  if (uflag || nflag || sflag || vflag || bflag || tflag || eflag)
    {
      do
	{
	  bytes = fread( buf, 1, BUFSIZ, fd );
	  out( buf, bytes );
	}
      while (bytes > 0);
    }
  else
    {
      while ((bytes = fread( buf, 1, BUFSIZ, fd )) > 0)
	{
	  for (i = 0; i < bytes; i++)
	    {	      
	      putchar( buf[ i ] );
	    }
	}
    }

  fclose( fd );

  Close( o );

  return;
  
} /* out_file */


/* copy across stdin as a special case - we don't want to close it on EOF */
/* PAB fixed redirection and now uses flags correctly */
/* BLV - check for the case "cat < xyz > xyz" */

void
copy_stdin( void )
{
  byte buff[ BUFSIZ ];


  if (!isatty( 0 ))
    if (!strcmp( Heliosno( stdin )->Name, Heliosno( stdout )->Name))
      {
	fputs("cat: input - is output\n", stderr);

	return;
      }
       
  while (fgets( buff, BUFSIZ, stdin ))
    out( buff, strlen( buff ) );

  return;
  
} /* copy_stdin */


void
bad_arg( char a )
{
  fprintf( stderr, "cat: Bad arg -'%c'\n", a );

  return;
  
} /* bad_arg */


int
main(
  int 		argc,
  char **	argv )
{
  long i, file;

  
  file = FALSE;			/* true if a file in the arglist */

  for (i = 1; i < argc; i++)
    if (argv[ i ][ 0 ] == '-')
      {
	switch (argv[ i ][ 1 ])
	  {
	  case 'u':
	    uflag = TRUE;
	    break;
	    
	  case 'n':
	    nflag    = TRUE;
	    new_line = TRUE;
	    break;
	    
	  case 's':
	    sflag = TRUE;
	    break;
	    
	  case 'v':
	    vflag = TRUE;
	    break;
	    
	  case 'b':
	    bflag    = TRUE;
	    nflag    = TRUE;
	    new_line = TRUE;
	    break;

	  case 't':
	    tflag = TRUE;
	    vflag = TRUE;
	    break;

	  case 'e':
	    eflag = TRUE;
	    vflag = TRUE;
	    break;

	  case 0:			/* lone '-' */
	    copy_stdin();
	    file = TRUE;
	    break;

	  default:
	    bad_arg( argv[ i ][ 1 ] );
	    break;
	  }
	
	if (argv[ i ][ 1 ] != 0 &&
	    argv[ i ][ 2 ] != 0)
	  {
	    /* multiple arg */

	    argv[i]++;
	    argv[i][0] = '-';	/* nudge it along */
	    i--;			/* defeat loop counter */
	  }
      }
    else
      {
	copy_file( argv[ i ] );
	
	file = TRUE;
      }

  if (!file)
    copy_stdin();

  return 0;

} /* main */
