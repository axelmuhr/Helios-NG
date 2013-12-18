/****************************************************************/
/* File: ampp.c                                                 */
/*                                                              */
/* Entry point for AMPP - Assembler Macro Pre-Preocessor for    */
/* the Helios assemblers. This preprocessor is based on that    */
/* given in Software Tools with a few syntactic changes and some*/
/* implementation improvements to make use of the facilities    */
/* available in C.                                              */
/*                                                              */
/* Author:   NHG  19-Feb-87                                     */
/* Modified: NOJC 25-Oct-91                                     */
/****************************************************************/

#ifdef NOT_USED
static char *RcsId = "$Id: ampp.c,v 1.11 1994/03/08 13:06:03 nickc Exp $";
#endif

#include "ampp.h"
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>


PUBLIC FILE *		verfd;
PUBLIC jmp_buf 		error_level;
PRIVATE jmp_buf 	default_level;
PRIVATE INT 		maxerr = 0;

PUBLIC BYTE *		infile  = "<stdin>";
PUBLIC BYTE *		outfile = "<stdout>";
PUBLIC BYTE *		incpaths[ MAX_INCLUDE_PATHS ] = { DEFAULT_INCLUDE_PATH };
PUBLIC INT		in_line = 0;

PUBLIC INT 		traceflags;

#ifdef MWC
long			_stksize = 40000;
#endif

#define trace if (traceflags & db_fileop) _trace

static void
tidyup()
{
  if (outfd != NULL)
    fflush( outfd );

  if (verfd != NULL)
   fflush( verfd );

  if (infd != stdin && infd != NULL)
    fclose( infd );

  infile  = "<unknown>";
  in_line = 0;
  
  if (outfd != stdout && outfd != NULL)
    fclose( outfd );

  if (verfd != stderr && verfd != NULL)
    fclose( verfd );

  /* unload(); */

  longjmp( default_level, 1 );

  return;

} /* tidyup */


PUBLIC INT
max(
    INT a,
    INT b )
{
   return a > b ? a : b;

} /* max */


#ifdef __STDC__

PUBLIC void
error( BYTE * str, ... )
{
  if (str != NULL && verfd != NULL)
    {
      va_list ap;

  
      va_start( ap, str );

      if (infile != NULL && *infile != '\0')
	fprintf( verfd, "AMPP: Error: %s %ld : ", infile, in_line );
      else
	fprintf( verfd, "AMPP: Error: " );

      vfprintf( verfd, str, ap );

      va_end( ap );

      putc( '\n', verfd );
    }
  
  maxerr = max( maxerr, 20 );

  tidyup();

  /* exit( (int) maxerr ); */
}


PUBLIC void
warn( BYTE * str, ... )
{
  va_list ap;


  if (str == NULL || verfd == NULL)
    return;
  
  va_start( ap, str );

  if (infile != NULL && *infile != '\0')
    fprintf( verfd, "AMPP: Warning: %s %ld : ", infile, in_line );
  else
    fprintf( verfd, "AMPP: Warning: " );

  vfprintf( verfd, str, ap );

  va_end( ap );

  putc( '\n', verfd );

  maxerr = max( maxerr, 10 );

  return;
  
} /* warn */


PUBLIC void
report( BYTE * str, ... )
{
  va_list	ap;


  if (str == NULL || verfd == NULL)
    return;

  va_start( ap, str );

  vfprintf( verfd, str, ap );

  putc( '\n', verfd );

  va_end( ap );

  return;
  
} /* report */


PUBLIC void
_trace( BYTE * str, ... )
{
  va_list ap;


  if (str == NULL || verfd == NULL)
    return;
  
  va_start( ap, str );

  fprintf( verfd, "Trace: " );

  vfprintf( verfd, str, ap );

  putc( '\n', verfd );

  va_end( ap );

  return;
  
} /* _trace */


PUBLIC void
recover( BYTE * str, ... )
{
  if (str != NULL && verfd != NULL)
    {
      va_list	ap;


      va_start( ap, str );

      if (infile != NULL && *infile != '\0')
	fprintf( verfd, "AMPP: Warning: %s %ld : ", infile, in_line );
      else
	fprintf( verfd, "AMPP: Warning: " );

      vfprintf( verfd, str, ap );

      va_end( ap );

      putc( '\n', verfd );
    }

  maxerr = max( maxerr, 10 );

  longjmp( reclev, 1 );

  /* NOTREACHED */

} /* recover */

#else /* !__STDC__ */

PUBLIC
error( str, a, b, c, d, e, f )
  BYTE *	str;
  INT 		a, b, c, d, e, f;
{
  _fprintf(verfd,"AMPP: Error: %s %d :", infile, in_line );

  _fprintf(verfd,str,a,b,c,d,e,f);

  putc('\n',verfd);

  maxerr = max(maxerr,20);

  tidyup();
}

PUBLIC
warn( str, a, b, c, d, e, f )
  BYTE *	str;
  INT 		a, b, c, d, e, f;
{
  _fprintf(verfd,"AMPP: Warning: %s %d :",infile, in_line );

  _fprintf(verfd,str,a,b,c,d,e,f);

  putc('\n',verfd);

  maxerr = max(maxerr,10);
}

PUBLIC void
recover( str, a, b, c, d, e, f )
  BYTE *	str;
  INT 		a, b, c, d, e, f;
{
  warn( str, a, b, c, d, e, f );

  longjmp( reclev, 1 );
}

PUBLIC
_trace( str, a, b, c, d, e, f )
  BYTE *	str;
  INT 		a, b, c, d, e, f;
{
  _fprintf(verfd,"Trace: ");

  _fprintf(verfd,str,a,b,c,d,e,f);

  putc('\n',verfd);
}

PUBLIC
report(str,a,b,c,d,e,f)
  BYTE *str;
  INT a,b,c,d,e,f;
{
  _fprintf(verfd,str,a,b,c,d,e,f);

  putc('\n',verfd);
}


PRIVATE
_fprintf(fd,str,a,b,c,d,e,f)
  FILE *fd;
  BYTE *str;
  INT a,b,c,d,e,f;
{
  static BYTE fbuf[128];
  BYTE *t = fbuf;
  BYTE *s = str;


  while( *s != '\0' )
    {
      if ( *s == '%' ) 
	{
	  *t++ = *s++;
	  while( '0' <= *s && *s <= '9' ) *t++ = *s++;
	  switch( *s )
	    {
	    case 'x': 
	    case 'X': *t++ = 'l'; *t++ = 'x'; break;
	      
	    case 'd': 
	    case 'D': *t++ = 'l'; *t++ = 'd'; break;
	      
	    default: *t++ = *s; break;
	    }
	  s++;
	}
      
      *t++ = *s++;	
    }
  
  *t = '\0';
  
  fprintf(fd,fbuf,a,b,c,d,e,f);
}
#endif /* not __STDC__ */


#define _ARG_ 		if ( *++arg == 0 ) arg = *++argv;

bool
parsearg(
	 BYTE ***	aargv,
	 int		pass )
{
  char **	argv = *aargv;
  char *	arg   = *argv;


  if (pass == 1)
    {
      if ( *arg == '-' )
	{
	  arg++;
	  
	  switch ( locase( *arg ) )
	    {
	    case 't':
	      while ( *++arg != 0 )
		{
		  switch ( locase( *arg ) )
		    {
		    case 'l': traceflags |= db_lex;     break;
		    case 'p': traceflags |= db_parse;   break;
		    case 'b': traceflags |= db_builtin; break;
		    case 's': traceflags |= db_sym;     break;
		    case 'k': traceflags |= db_putback; break;
		    case 'i': traceflags |= db_fileop;  break;
		    default:
		      error( "unknown trace option -t%c", *arg );
		      break;
		    }
		}
	      break;
	      
	    case 'v':
		{
		  FILE *vfd;
		  
		  
		  _ARG_;
		  
		  if (arg == NULL)
		    error( "filename must follow -v option" );
		  
		  vfd = fopen( arg, "w" );
		  
		  if ( vfd == NULL )
		    error( "Cannot open %s for output", arg );
		  
		  verfd = vfd;
		  
		  break;
		}
	      
	    case 'd':
		{
		  char *		name;
		  char *		defn;
		  struct Symbol *	sym;
		  struct Macro *	macro = New( struct Macro   );
		  struct Charbuf *	buf   = New( struct Charbuf );
		  
		  
		  if (macro == NULL || buf == NULL)
		    error( "ran out of memory parsing arguments" );
		  
		  _ARG_;
		  
		  if (arg == NULL)
		    error( "macro name must follow -d option" );
		  
		  name = arg;
		  defn = *++argv;
		  
		  if (defn == NULL)
		    error( "no definition following macro name" );
		  
		  sym = insert( name );
		  
		  strcpy( buf->text, defn );
		  
		  buf->size = strlen( defn );
		  
		  InitList( &macro->def );
		  
		  macro->nargs = 0;
		  
		  AddHead( &macro->def, &buf->node );
		  
		  adddef( (INT)s_macro, sym, (INT)macro );
		  
		  break;
		}
	      
	    case 'i':
		{
		  struct stat	stat_buf;
		  int		i;
		  
		  
		  _ARG_;
		  
		  if (arg == NULL)
		    error( "filename must follow -i option" );
		  
		  if (stat( arg, &stat_buf ))
		    {
		      int	len = strlen( arg );
		      
		      
		      if (arg[ len -1 ] == '/')
			{
			  arg[ len - 1 ] = '\0';
			  
			  if (stat( arg, &stat_buf ))
			    error( "cannot find include directory %s", arg );
			}
		      else
			error( "cannot find include directory %s", arg );
		    }
		  
		  if (!S_ISDIR( stat_buf.st_mode ))
		    error( "file %s is not a directory", arg );

		  for (i = 0; i < MAX_INCLUDE_PATHS; i++)
		    if (incpaths[ i ] == NULL)
		      {
			incpaths[ i ] = arg;
			break;
		      }
		  
		  if (i == MAX_INCLUDE_PATHS)
		    error( "Too many include directories" );
		}
	      
	      break;
	      
	    case 'o':
	      _ARG_;
	      
	      if ( outfd != stdout )
		error("Give only one output file");
	      
	      if (arg == NULL)
		error( "filename must follow -o option" );
	      
	      outfile = arg;

	      trace( "Opening output file %s", outfile );
	      
	      outfd = fopen( outfile, "w" );
	      
	      if ( outfd == NULL )
		error( "Cannot open %s for output", outfile );
	      
	      break;
	      
	    case 'h':
	    case '?':
	      report( "ampp V1.3 (RCS version $Revision: 1.11 $)" );
	      report( "command line options ..." );
	      report( "-t{l,p,b,s,k,i}  - enable tracing of lex, parse, builtins, symbols, putbacks and file operations" );
	      report( "-v <filename>    - name of file in which messages should be placed" );
	      report( "-d <name> <body> - define a macro called <name> with definition <body>" );
	      report( "-i <dirname>     - specify directory to search for include files" );
	      report( "-o <filename>    - name of output file" );
	      report( "-help            - provide this description" );
	      report( "-                - read from stdin" );
	      report( "<filename>       - name of file to read" );

	      tidyup();
	      
	      break;
	      
	    case '\0':
	      *aargv = argv + 1;
	      
	      return TRUE;
	      
	    default:
	      error( "unknown command line option -%s", arg );
	      break;
	    }
	  
	  *aargv = argv + 1;
	  
	  return FALSE;
	}
      else
	{
	  *aargv = argv + 1;
	  
	  return TRUE;
	}
    }
  else /* pass == 2 */
    {
      if ( *arg == '-' )
	{
	  arg++;
	  
	  switch ( locase( *arg ) )
	    {
	    case '\0':
	      infile  = "<stdin>";
	      infd    = stdin;
	      in_line = 1;
	      
	      *aargv = argv + 1;
	      
	      return TRUE;
	      
	    case 'd':
	      _ARG_;
		  
	      ++argv;

	      break;
	      
	    case 'i':
	    case 'v':
	    case 'o':
	      _ARG_;
	      
	      break;
	      
	    default:
	      break;
	    }
	  
	  *aargv = argv + 1;
	  
	  return FALSE;

	}
      else
	{
	  if ( infd != NULL && infd != stdin )
	    fclose( infd );
	  
	  infile  = arg;
	  in_line = 1;
	  
	  trace( "Opening input file %s", infile );
	  
	  infd    = fopen( infile, "r" );

	  if (infd == NULL)
	    {
	      int	i;

	      
	      for (i = MAX_INCLUDE_PATHS; i--;)
		{
		  char path[ 128 ];
	      

		  if (incpaths[ i ] == NULL)
		    continue;
		  
		  strcpy( path, incpaths[ i ] );
	      
		  if ( path[ strlen( path ) ] != '/' )
		    strcat( path, "/" );
	      
		  strcat( path, infile );
	      
		  trace( "Open failed, trying %s", path );
	      
		  infd = fopen( path, "r" );

		  if (infd != NULL)
		    break;
		}
	    }
	  
	  if ( infd == NULL )
	    error( "Cannot open %s for input", infile );
	  else
	    trace( "Open succeeded" );
	  
	  *aargv = argv + 1;
	  
	  return TRUE;
	}
    }
  
} /* parsearg */


PUBLIC char *
alloc(   INT size )
{
  char *	v  = (char *) malloc( (int) size );


  if (v == NULL )
    error( "Cannot allocate %d bytes\n", size );

  /*_trace("alloc %d -> %x Malloc(-1) = %d",size,v,Malloc(-1));*/

  return v;
}


/********************************************************/
/* main                                                 */
/*                                                      */
/* Initialise everything and call the macro processor   */
/*                                                      */
/********************************************************/

PUBLIC int
main(
     int 	argc,
     BYTE **	argv )
{
  int 		infiles = 0;
  BYTE **	copy_argv;
  
  
  infd    = stdin;
  in_line = 1;
  outfd   = stdout;
  verfd   = stderr;
  
  if ( setjmp( error_level ) == 0 )
    {
      memcpy( default_level, error_level, sizeof (jmp_buf) );
      
      argv++;
      argc--;
      
#ifdef VERBOSE
      report( "Assembler Macro PreProcessor V1.3" );
      report( "Copyright (C) 1987 - 1992 Perihelion Software Ltd." );
#endif
      
      initcs();
      
      initpb();

      initsym();
      
      initbuiltin();
      
      symb = 0;

      copy_argv = argv;
      
      while ( *argv != NULL ) 
	{
	  (void) parsearg( &argv, 1 );
	}

      argv = copy_argv;
      
      while ( *argv != NULL ) 
	{
	  if ( parsearg( &argv, 2 ) )
	    {
	      macro();
	      
	      infiles++;
	    }
	}
      
      if ( infiles == 0 )
	{
	  infd    = stdin;
	  in_line = 1;
	  
	  macro();
	}
      
#ifdef VERBOSE
      /* report( "AMPP Finished" ); */
#endif
      
      tidyup();
    }
  
  exit( (int) maxerr );

} /* main */
