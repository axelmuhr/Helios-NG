/*{{{  Header */

/*
 * bsplit.c - binary version of the UNIX split command
 *
 * Copyright (c) 1994 Perihelion Software Ltd.
 * All rights reserved.
 *
 * Author:		N Clifton
 * Version:		$Revision: 1.4 $
 * Date:		$Date: 1994/02/24 10:48:57 $
 * Split Author:	John Ffitch
 *
 */

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/com/RCS/bsplit.c,v 1.4 1994/02/24 10:48:57 nickc Exp $";
#endif

/*}}}*/
/*{{{  Includes */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

/*}}}*/
/*{{{  Variables */

static long int	split_size = 1024L * 700L;	/* number of bytes per file */
static char *	ProgName   = NULL;		/* name of the program */
static int	dbg        = 0;			/* non-zero for debugging */

/*}}}*/
/*{{{  Code */

/*{{{  debug() */

void
debug( const char * format, ... )
/*
 * print a debugging message
 */
{
	va_list	args;
	

	va_start( args, format );

	if (dbg)
 	{	
		if (ProgName)
			fprintf( stderr, "%s: ", ProgName );
		
		vfprintf( stderr, format, args );
	
		fprintf( stderr, "\n" );
	
		fflush( stderr );
	}
	
	va_end( args );
	
	return;
	
} /* debug */

/*}}}*/
/*{{{  usage() */

void
usage( void )
/*
 * print a usage description
 */
{
	fprintf( stderr, "Usage: " );

	if (ProgName)
		fprintf( stderr, "%s ", ProgName );
	else
		fprintf( stderr, "bsplit " );
		
	fprintf( stderr, "\n" );
	fprintf( stderr, "    [-<number>[K/M]] (number of [kilo/mega]bytes per file)\n" );
	fprintf( stderr, "    [-d]             (enable debugging)\n" );
	fprintf( stderr, "    [-j]             (join files)\n" );
	fprintf( stderr, "    <file name>      (name of input (for split) or output (for join) file)\n" );
	fprintf( stderr, "    [<file name>]    (name of output (for split) or input (for join) file(s))\n" );
	
	return;
	
} /* usage */

/*}}}*/
/*{{{  open_file() */

FILE *
open_file(
	char *	name,
	int 	number,
	char *	mode )
/*
 * attempts to open the named file with the given extension
 * returns a file handle to the open file or NULL upon failure
 */
{
	char		newname[ 256 ];		/* XXX */
	register FILE *	ans;
	register int 	i;


	debug( "open_file: called with name = %s number = %d", name, number );

	strcpy( newname, name );

	i = strlen( name );

	newname[ i++ ] = ('a'+ (number / 26));
	newname[ i++ ] = ('a'+ (number % 26));
	newname[ i   ] = '\0';

	debug( "open_file: Opening file %s", newname );

	ans = fopen( newname, mode );

	if (ans == NULL)
	{
		debug( "open_file: Cannot open file %s", newname );
	}

	return ans;

} /* open_file */

/*}}}*/
/*{{{  main() */

int
main(
	int 	argc,
	char **	argv )
/*
 * the main body of the bsplit program
 */
{
	FILE *		inf;
	FILE *		outf;
	char *		arg;
	char *		in_file;
	char *		out_file;
	char *		split_name;
	int		join = 0;
	int		file_number;
	int 		ch;
 	long int 	byte_count;


	ProgName = argv[ 0 ];

	if ((arg = strrchr( ProgName, '/' )) != NULL)
	  {
	    ProgName = arg + 1;
	  }
	else if ((arg = strrchr( ProgName, '\\' )) != NULL)
	  {
	    ProgName = arg + 1;
	  }

	arg = *++argv;

	/* process command line arguments */
	
	while (arg != NULL && arg[ 0 ] == '-' && arg[ 1 ] != '\0')
	{
		if (arg[ 1 ] == 'd')
		{
			++dbg;
		}
		else if (arg[ 1 ] == 'j')
		{
			++join;
		}
		else if (arg[ 1 ] == 'h')
		{
			usage();
			
			return 0;
		}
		else if (isdigit( arg[ 1 ] ))
		{
			long int	save = split_size;
			char		unit;			
		
			/* Set new split rate */

			unit = arg[ strlen( arg ) - 1 ];
			
			sscanf( arg, "-%ld", &split_size);

			if (unit == 'k' || unit == 'K')
				split_size *= 1024L;
			else if (unit == 'm' || unit == 'M')
				split_size *= 1024L * 1024L;
 
			if (split_size == 0)
				split_size = save;
				
			debug( "Splitsize reset to %ld (from %s)",
				split_size, arg );
		}

		arg = *++argv;
		argc--;
	}

	if (argc < 2 || argc > 3)
	{
		usage();
		
		return 1;
	}
	
	in_file = arg;
		
	if (arg[ 0 ] == '-' && arg[ 1 ] == '\0')
	{
		/* Take current input */
		
		if (join)
			inf = stdout;
		else
    			inf = stdin;
  	}
	else
	{
  		if (join)
  			inf = fopen( arg, "wb" );
  		else
  			inf = fopen( arg, "rb" );
  		
    		if (inf == NULL)
    		{
      			debug( "Cannot open %s file %s",
      				join ? "output" : "input", arg );
      			
      			return 1;
		}
	}
	
	if (argc == 3)
	{
		/* set split name */
		
		out_file = split_name = *++argv;

		debug( "Split name reset to %s", split_name );	
  	}
	else
	{
		out_file = split_name = "x";
	}

	/*
	 * NB/ we do not check to see if the input and output names
	 * refer to the same file
	 */

	file_number = 0;
	byte_count  = 0;
	
	if (join)
	{
		/*
		 * NB/ when joining 'inf' is the ouput file
		 * and 'outf' itterates through the input files
		 */
		 
		while ((outf = open_file( split_name, file_number++, "rb" )) != NULL)
		{
			++byte_count;
			
			while ((ch = getc( outf )) != EOF)
			{
    				putc( ch, inf );
    			}

			fclose( outf );
		}

		if (byte_count == 0)
		{
			/* 
			 * no input files found - this usually means
			 * that the command line arguments were the
			 * wrong way around - try to (silently)
			 * correct this
			 */
			 
			if (in_file[ 0 ] != '-' && in_file[ 1 ] != '\0')
			{
				fclose( inf );
				remove( in_file );
				
				split_name  = in_file;
				file_number = 0;
				
				if (out_file[ 0 ] == '-' && out_file[ 1 ] == '\0')
				{
					outf = stdout;
				}
				else
				{
					outf = fopen( out_file, "wb" );
					
					if (outf == NULL)
					{
						debug( "Unable to open output file %s", out_file );
						
						return 1;
					}					
				}
				
				/* OK try again */
				
				while ((inf = open_file( split_name, file_number++, "rb" )) != NULL)
				{
					while ((ch = getc( inf )) != EOF)
					{
    						putc( ch, outf );
    					}

					fclose( inf );
				}
				
				inf = outf;
			}
			else
			{
				debug( "Cannot join multiple copies of stdin" );
				
				return 1;
			}
		}
		
		fclose( inf );
	}
	else
	{	
		outf = open_file( split_name, file_number, "wb" );
	
		while ((ch = getc( inf )) != EOF)
		{
    			putc( ch, outf );
    		
    			++byte_count;
    		
			if (byte_count == split_size)
			{
				fclose( outf );
			
				ch = getc( inf );
			
				if (ch == EOF)
				{
					break;
				}
			
				outf = open_file( split_name, ++file_number, "wb" );

				ungetc( ch, inf );
				
				byte_count = 0;
			}
		}
	
		fclose( outf );
	}
	
	return 0;

} /* main */

/*}}}*/

/*}}}*/

/* end of bsplit.c */
