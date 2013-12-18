/*{{{  Comment */

/****************************************************************/
/* Helios Linker						*/
/*								*/
/* Copyright (c) 1989 - 1993 Perihelion Software Ltd.           */
/*  All Rights Reserved                                         */
/*                                                              */
/* File: link.c                                                 */
/*                                                              */
/*                                                              */
/* Author: NHG 26/9/88                                          */
/*                                                              */
/* Updates: CS/PAB/NC                                           */
/*                                                              */
/****************************************************************/
/* RcsId: $Id: link.c,v 1.44 1994/05/04 14:47:29 nickc Exp $ Copyright (C) Perihelion Software Ltd. */

/*}}}*/
/*{{{  Includes */

#include "link.h"
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
  
#ifdef __HELIOS
/*
 * XXX - the following is an amazing hack to get
 * the definition of Heliosno() without including
 * module.h
 */
  
typedef int Program;
#define __module_h
#include <nonansi.h>
#endif


#include "hash.c"

/*}}}*/
/*{{{  Constanst */

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0	/* sparc stdlib.h fails to define this! */
#define EXIT_FAILURE -1
#endif
#ifndef CLK_TCK
#define CLK_TCK 1000000	 /* and this! */
#endif

#define STDOUT	1
#define STDERR	2

/*}}}*/
/*{{{  Macros */

#define trace if (traceflags & db_files) _trace

/*}}}*/
/*{{{  Variables */

PUBLIC  FILE *		infd;
PUBLIC  int		outf;
PUBLIC  FILE *		verfd;
PRIVATE jmp_buf		error_level;
PUBLIC  WORD 		errors;
PRIVATE WORD 		phase      = 0;
PRIVATE WORD 		readfiles  = FALSE;
PUBLIC  WORD 		traceflags;
PRIVATE WORD 		maxerr     = 0;
PUBLIC  WORD 		verbose    = FALSE;
PUBLIC  WORD 		inlib      = FALSE;
#ifdef __SMT
PUBLIC	WORD 		smtopt     = TRUE; /* default to split module table operation */
#else
PUBLIC	WORD 		smtopt     = FALSE; /* default to original module table operation */
#endif
#if defined __ARM && (defined RS6000 || defined __SUN4)
PUBLIC WORD		bSharedLib    = FALSE;
PUBLIC WORD		bDeviceDriver = FALSE;
PUBLIC WORD		bTinyModel    = FALSE;
#endif
PUBLIC  WORD 		vmpagesize = 8 * 1024;
PUBLIC	WORD		gen_image_header = TRUE;

PRIVATE WORD		silent_running = FALSE;
PRIVATE WORD 		needobjed  = FALSE;
PRIVATE char *		progname   = NULL;	/* name of program we are linking */
PRIVATE char *		ProgName   = NULL;	/* name we are running under */
PRIVATE WORD 		stack_size = -1;
PRIVATE WORD 		heap_size  = -1;


PUBLIC  BYTE 		infile[  128 ];
PUBLIC	BYTE *		infile_duplicate = NULL;	/* malloced copy of the infile array */
PRIVATE BYTE 		outfile[ 128 ];

PUBLIC WORD uch = 0;
PUBLIC WORD unreadready = FALSE;

/*}}}*/
/*{{{  Forward declarations */

#if __STDC__
/* void 		unload( void ); */
PRIVATE WORD 		parsearg( char *** );
WORD 			max( word, word );
PRIVATE void 		tidy1(  void );
PRIVATE void 		tidyup( void );
#else
/* void 		unload(void); */
PRIVATE WORD 		parsearg();
WORD 			max();
PRIVATE void 		tidy1();
PRIVATE void 		tidyup();
PRIVATE void 		_fprintf();
#endif

/*}}}*/

/*{{{  Functions */

/*{{{  Housekeeping functions */

#ifndef __STDC__
# ifdef NOMEMCPY
/*{{{  memcpy() */

void
memcpy( to, from, count )
char *to, *from;
int count;
{
while(count--)
	*to++ = *from++;
}

/*}}}*/
# endif /* NOMEMCPY */

/* ANSI compatible clock() fn */
# ifdef NOCLOCKFN
/*{{{  clock() */

# include <sys/time.h>

# define CLK_TCK 1000000	 /* Usecs */
typedef unsigned int clock_t;

clock_t
clock()
{
  struct timeval t;
  struct timezone tz;

  
  gettimeofday( &t, &tz );
  
  return (t.tv_sec * 1000000 + t.tv_usec);
}

/*}}}*/
# endif /* no NOCLOCKFN */
#endif  /* no __STDC__ */

/*{{{  _strdup() */

PRIVATE BYTE *
_strdup( BYTE * str )
{
  BYTE *	copy;
  
  
  if (str == NULL)
    return NULL;

  copy = (BYTE *) malloc( strlen( str ) + 1 );

  if (copy == NULL)
    return NULL;
  
  return strcpy( copy, str );
  
} /* _strdup */

/*}}}*/

/*{{{  min() */

PUBLIC WORD
min(
    WORD a,
    WORD b )
{
  return a < b ? a : b;
}

/*}}}*/
/*{{{  max() */

PUBLIC WORD
max(
    WORD a,
    WORD b )
{
  return a > b ? a : b;
}

/*}}}*/

/*}}}*/
/*{{{  Message reporting functions */

#ifdef __STDC__
/*{{{  error() */

PUBLIC void error(BYTE *str,...)
{
	va_list ap;

	errors++;
	va_start(ap,str);
        if (readfiles) putc('\n',verfd);
#ifdef LINENO
        fprintf(verfd,"%s: Fatal Error: %s %ld : ",ProgName, infile, lineno );
#else
	if (infile && *infile != '\0')
	        fprintf(verfd,"%s: Fatal Error: %s : ", ProgName, infile );
       else
      	        fprintf(verfd,"%s: Fatal Error: ", ProgName);
#endif

	vfprintf(verfd,str,ap);
        putc('\n',verfd);
	va_end(ap);
        maxerr = max(maxerr,20);

	tidyup();
	exit((int)maxerr);
}

/*}}}*/
/*{{{  inform() */

PUBLIC void
inform( BYTE * str, ... )
{
  va_list ap;


  if (silent_running)
    return;
  
  va_start( ap, str );

  if (readfiles && verbose)
    putc( '\n', verfd );

#ifdef LINENO
  fprintf( verfd,",%s: Warning: %s %ld : ", ProgName, infile, lineno );
#else

  if (infile && *infile != '\0')
    fprintf( verfd,"%s: Warning: %s: ", ProgName, infile );
  else
    fprintf( verfd,"%s: Warning: ", ProgName );
#endif

  vfprintf( verfd, str, ap );

  putc( '\n', verfd );

  va_end( ap );

  return;	

} /* inform */

/*}}}*/
/*{{{  warn() */

PUBLIC void warn(BYTE *str, ...)
{
  va_list ap;


  va_start(ap,str);

  if (readfiles && verbose)
    putc('\n',verfd);
  
#ifdef LINENO
  fprintf(verfd,",%s: Warning: %s %ld : ",ProgName, infile,lineno);
#else
  if (infile && *infile != '\0')
    fprintf(verfd,"%s: Warning: %s : ", ProgName, infile);
  else
    fprintf(verfd,"%s: Warning: ", ProgName );
#endif
  vfprintf(verfd,str,ap);
  putc('\n',verfd);
  va_end(ap);
  errors++;
  maxerr = max(maxerr,10);
}

/*}}}*/
/*{{{  report() */

PUBLIC void
report( BYTE * str, ... )
{
   va_list ap;


   if (!verbose)
     return;

   va_start( ap, str );
   
   if (readfiles)
     putc( '\n', verfd );

   vfprintf( verfd, str, ap );

   putc( '\n', verfd );
   
   va_end( ap );

   return;
   
} /* report */

/*}}}*/
/*{{{  _trace() */

PUBLIC void _trace(BYTE *str,...)
{
   va_list ap;

   va_start(ap,str);
/*   if (readfiles) putc('\n',verfd); */
   fprintf(verfd,"%s: Trace: ", ProgName );
   vfprintf(verfd,str,ap);
   putc('\n',verfd);
   va_end(ap);
}

/*}}}*/
/*{{{  recover() */

PUBLIC void recover(BYTE *str,...)
{
	va_list ap;

        if (readfiles) putc('\n',verfd);
	va_start(ap,str);
#ifdef LINENO
        fprintf(verfd,"%s: Warning: %s %ld : ", ProgName, infile,lineno);
#else
	if (infile && *infile != '\0')
	        fprintf(verfd,"%s: Warning: %s : ", ProgName, infile);
	else
	        fprintf(verfd,"%s: Warning: ", ProgName );
#endif
        vfprintf(verfd,str,ap);
        va_end(ap);
        putc('\n',verfd);
        errors++;
        maxerr = max(maxerr,10);
        longjmp(error_level,1);
}

/*}}}*/

#else /* !__STDC__ */
/*{{{  error() */

PUBLIC void error(str,a,b,c,d,e,f)
BYTE *str;
INT a,b,c,d,e,f;
{
	errors++;
        if (readfiles) putc('\n',verfd);
#ifdef LINENO
        _fprintf(verfd,"Fatal Error: %s %ld : ",infile,lineno);
#else
	if(strlen(infile))
	        _fprintf(verfd,"Fatal Error: %s : ",infile);
       else
      	        _fprintf(verfd,"Fatal Error: ");
#endif
        _fprintf(verfd,str,a,b,c,d,e,f);
        putc('\n',verfd);
        maxerr = max(maxerr,20);

	tidyup();
	exit((int)maxerr);
}

/*}}}*/
/*{{{  warn() */

PUBLIC void warn(str,a,b,c,d,e,f)
BYTE *str;
INT a,b,c,d,e,f;
{
        if (readfiles) putc('\n',verfd);
#ifdef LINENO
        _fprintf(verfd,"Warning: %s %ld : ",infile,lineno);
#else
	if(strlen(infile))
	        _fprintf(verfd,"Warning: %s : ",infile);
	else
	        _fprintf(verfd,"Warning: ");
#endif
        _fprintf(verfd,str,a,b,c,d,e,f);
        putc('\n',verfd);
        errors++;
        maxerr = max(maxerr,10);
}

/*}}}*/
/*{{{  report() */

PUBLIC void report(str,a,b,c,d,e,f)
BYTE *str;
INT a,b,c,d,e,f;
{
	if( !verbose ) return;

        if (readfiles) putc('\n',verfd);
        _fprintf(verfd,str,a,b,c,d,e,f);
        putc('\n',verfd);
}

/*}}}*/
/*{{{  _trace() */

PUBLIC void _trace(str,a,b,c,d,e,f)
BYTE *str;
INT a,b,c,d,e,f;
{
       if (readfiles) putc('\n',verfd);
       _fprintf(verfd,"Trace: ");
       _fprintf(verfd,str,a,b,c,d,e,f);
       putc('\n',verfd);
}

/*}}}*/
/*{{{  recover() */

PUBLIC void recover(str,a,b,c,d,e,f)
BYTE *str;
INT a,b,c,d,e,f;
{
        warn(str,a,b,c,d,e,f);
        longjmp(error_level,1);
}

/*}}}*/
/*{{{  _fprintf() */

PRIVATE void _fprintf(fd,str,a,b,c,d,e,f)
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
         *t = *s;
         t++;
         s++;
         while( '0' <= *s && *s <= '9' ) 
         {
             *t = *s;
             t++;
             s++;
         }
         switch( *s ) 
         {
         case 'x': 
         case 'X': *t = 'l'; 
            t++;
            *t = 'x'; 
            t++;
            break;

         case 'd': 
         case 'D': *t = 'l'; 
            t++;
            *t = 'd'; 
            t++;
            break;

         default: *t = *s; 
            t++;
            break;
         }
         s++;
      }
      *t = *s;
      t++;
      s++;
   }
   *t = '\0';
   fprintf(fd,fbuf,a,b,c,d,e,f);
}

/*}}}*/
#endif /* __STDC__ */

/*}}}*/

/*{{{  tidyup() */

PRIVATE void
tidyup()
{
  tidy1();

  if (outf > STDERR)
    {
      close( outf );
      
      if (errors)
	{
#ifdef __STDC__
	  remove( outfile );
#else
	  unlink( outfile );
#endif
	  /* puts( "Image file removed\n" ); */
	}

      outf = STDOUT;
    }

  if (verfd != stderr &&
      verfd != NULL)
    {
      fclose( verfd );

      verfd = stderr;
    }
  
  VMTidy();

  return;
 
} /* tidyup */

/*}}}*/
/*{{{  tidy() */

PRIVATE void
tidy1()
{
  if (infd != stdin &&
      infd != NULL)
    {
      fclose( infd );
      
      infd = stdin;
    }

  return;
}

/*}}}*/
/*{{{  usage() */

PRIVATE void
usage( void )
{
  fprintf( stderr, "%s: $Revision: 1.44 $ command line arguments:\n", ProgName );
  fputs("\
<filename>\tinput\t\t- <filename> is a object file to be linked\n\
-\t\tstdin\t\t- link file read from stdin\n\
-l <filename>\tlibrary\t\t- <filename> is a library to be linked\n\
-o <filename>\toutput\t\t- specify the name for the output file\n\
-n <name>\tname\t\t- provide a name for the linked program\n\
-s <value>\tstack size\t- specify program's minimum stack size\n\
-h <value>\theap size\t- specify program's minimum heap size\n",
	stderr );
#ifdef __SMT
  fputs( "-SMTN\t\tdisable\t\t- disable split module table support\n", stderr );
#else
  fputs( "-SMTY\t\tenable\t\t- enable split module table support\n", stderr );
#endif
#ifdef VM
  fputs( "\
-m <filename>\tVM file\t\t- specify the name of the virtual memory file\n\
-z <value>\tVM size\t\t- specify the size of the virtual memory pages\n", stderr);
#endif
#ifdef __ARM
  fputs( "-A[lrt]\t\tARM Options\t- specify how to convert AOF files\n\
\t\t l\t\t-  build code for use in a shared library\n\
\t\t r\t\t-  build code for use in a device driver\n\
\t\t t\t\t-  build code using the tiny model\n", stderr );
#endif
  fputs( "\
-v [<file>]\tverbose\t\t- talk about what is going on [to <file>]\n\
-q\t\tquiet\t\t- suppress some warning messages\n\
-i\t\tno image header\t- suppress generation of image header for ROMming\n\
-?\t\thelp\t\t- display this information\n\
-t <letter(s)>\ttrace\t\t- enable tracing of link stage(s)\n\
\t\t c\t\t-  Code generation\n\
\t\t o\t\t-  image generation\n", stderr );
  fputs( "\
\t\t f\t\t-  File operations\n\
\t\t m\t\t-  Memory system\n\
\t\t y\t\t-  sYmbol manipulation\n\
\t\t d\t\t-  moDule generation\n\
\t\t s\t\t-  code Scanning\n", stderr );
#ifdef NEW_STUBS
fputs( "\t\t t\t\t-  sTub generation\n", stderr );
#endif
#if defined __ARM && (defined RS6000 || defined __SUN4)
fputs( "\t\t a\t\t-  AOF conversion\n", stderr );
#endif 
 
  return;
  
} /* usage */

/*}}}*/
/*{{{  parsearg() */

PRIVATE WORD
parsearg( char *** aargv )
{
  char **	argv = *aargv;
  char *	arg  = *argv;

  
#  define _ARG_ if (*++arg == '\0') { if ((arg = *(++argv)) == NULL) { usage(); exit( EXIT_FAILURE ); } }

  if ( *arg == '-' )
    {
      arg++;

      switch ( *arg )
	{
	case 'v':
	  verbose = !verbose;

	  if (*++arg != '\0')
	    {
	      FILE *	vfd;

	      trace( "opening verbose output file '%s'", arg );
	      
	      vfd = fopen( arg, "w" );

	      if (vfd == NULL)
		error( "Cannot open %s for output", arg );
	      else
		trace( "file opened" );
	      
	      verfd = vfd;
	    }
	  break;
	      
	case 't':
	  arg++;
	      
	  while (*arg != '\0')
	    {
	      switch ((int)locase( *arg ))
		{
		case 'c': traceflags ^= db_gencode;  break;
		case 'o': traceflags ^= db_genimage; break;
		case 'f': traceflags ^= db_files;    break;
		case 'm': traceflags ^= db_mem;      break;
		case 'y': traceflags ^= db_sym;      break;
		case 'd': traceflags ^= db_modules;  break;
		case 's': traceflags ^= db_scancode; break;
#ifdef NEW_STUBS
		case 't': traceflags ^= db_stubs;    break;
#endif
#if defined __ARM && (defined RS6000 || defined __SUN4)
		case 'a': traceflags ^= db_aof;  break;
#endif
		default:
		  report( "unknwon trace option '%c'", *arg );
		  break;
		}
	      arg++;
	    }
	  break;
	  
#ifdef __SMT
	case 'S':
	  /* disable split module table: option '-SMTN' */
	  
	  if (*(arg + 1) == 'M' &&
	      *(arg + 2) == 'T' &&
	      *(arg + 3) == 'N' &&
	      *(arg + 4) == '\0')
	    smtopt = FALSE;
	  else
	    error( "Unknown option -%s",arg );
	  break;
#else
	case 'S':
	  /* enable split module table: option '-SMTY' */
	  
	  if (*(arg + 1) == 'M' &&
	      *(arg + 2) == 'T' &&
	      *(arg + 3) == 'Y' &&
	      *(arg + 4) == '\0')
	    smtopt = TRUE;
	  else
	    error( "Unknown option -%s", arg );
	  break;
#endif
	case 'n':
	  /* objed module name setting option */
	  
	  _ARG_;
	    
	  needobjed = 1;
	  progname  = arg;
	  break;
	      
	case 's':
	  /* objed stack setting option */
	  
	  _ARG_;
	  
	  needobjed  = 1;
	  stack_size = (long)atol( arg );

	  break;

	default:
	  report( "unknown command line argument -%s", arg );
	  /* fall through */
	  
	case '?':
	  usage();
	  exit( EXIT_FAILURE );
	  
	case 'q':
	  silent_running = TRUE;
	  break;	  
	  
	case 'i':
	  /* disable the generation of a std Helios image header and the */
	  /* trailing zero word. */
	  gen_image_header = FALSE;
	  break;

	case 'h':
	  if (arg[ 1 ] == 'e' &&
	      arg[ 2 ] == 'l' &&
	      arg[ 3 ] == 'p' &&
	      arg[ 4 ] == '\0' )
	    {
	      usage();
	      exit( EXIT_SUCCESS );
	    }
	  
	  _ARG_;
	  
	  needobjed = 1;
	  heap_size = (long)atol(arg);
	  break;         
	      
#ifdef VM
	case 'm':
	  _ARG_;
	  VMfilename = arg;
	  break;
	      
	case 'z':
	  _ARG_;
	  vmpagesize = (long)atol( arg );
	  break;
#endif

#if defined __ARM && ( defined RS6000 || defined __SUN4 )
	case 'A':
	  arg++;

	  while (*arg != '\0')
	    {
	      switch ((int)locase( *arg ))
		{
		case 'l': bSharedLib    = TRUE; break;
		case 'r': bDeviceDriver = TRUE; break;
		case 't': bTinyModel    = TRUE; break;
		default:  report( "unknown ARM linker option '%c'", *arg ); break;
		}
	      arg ++;
	    }
	  break;
#endif
	  
	case 'o':
	  _ARG_;
	  
	  (void)strcpy( outfile, arg );

#ifdef __STDC__
	  remove( outfile );
#else
	  unlink( outfile );
#endif
	  /*
	   * BEWARE
	   *
	   * Under SunOS, opening a file with the truncate bit set,
	   * (for example by using fopen( ,"w+" )), will cause the
	   * file to be truncated every time the file pointer is
	   * moved backwards.  So, for example seeking to the beginning
	   * of the file will truncate the file to 0 length!  The objed()
	   * code does just this, so we cannot use fopen() to get hold
	   * of a file descriptor.  Hence the use of POSIX level I/O
	   * for the output file, and C level I/O for everything else.
	   */

	  errno = 0;
	  
	  trace( "opening output file '%s'", outfile );
	  
	  outf = open( outfile, O_CREAT | O_RDWR, 0666 );
	  
	  if (outf == -1)
	    error( "Cannot open %s for output, errno = %d", outfile, errno );
	  else
	    trace( "output file opened" );
	  
	  break;
	      
	case 'l':
	  _ARG_;
	  
	  inlib = TRUE;

	  trace( "opening library file '%s'", arg );
	  
	  goto inputfile;

	case '\0':
	  arg = "<stdin>";

#if defined __HELIOS /* && defined NEVER */
	  trace( "reopening stdin" );
	  
	  if ((fseek( stdin, 0L, SEEK_SET ) == 0) &&	/* only re-open ordinary files */
	      (fprintf( stderr, "reopening stdin\n" ),
	       infd = freopen( Heliosno( stdin )->Name, "rb", stdin )) == NULL)
	    {
	      error( "Failed to reopen stdin" );
	    }
#else
	  infd = stdin;
#endif
#if 0
	    {
	      int	c;

	      
	      c = getc( stdin );
	      
	      while (feof( stdin ) || ferror( stdin ))
		{
		  fprintf( stderr, "reset stdin\n" );
		  
		  clearerr( stdin );
		  
		  c = getc( stdin );
		}
	      
	      ungetc( c, stdin );

	      fprintf( stderr, "first char of stdin = %x\n", c );	      
	    }
#endif
	  trace( "opening stdin" );
	  
	  strcpy( infile, arg );
	  
	  goto readfile;	  
	}
	  
      *aargv = ++argv;
      
      return FALSE;
    }
  else
    {
      trace( "opening file '%s' for input", arg );
      
    inputfile:
      (void)strcpy( infile, arg );
      
#ifdef __STDC__
# ifdef TRIPOS
      infd = fopen( infile, "r" );
# else
      infd = fopen( infile, "rb" );
# endif
#else
      infd = fopen( infile, "r" );
#endif
      if (infd == NULL)
	{
	  error( "Cannot open %s for input", infile );
	}
      else
	trace( "opened input file" );
      
    readfile:      
      infile_duplicate = _strdup( arg );
      
      if (verbose) 
	{
	  int i = 80 - strlen( infile ) - sizeof "Reading ";

	  
	  fprintf( verfd, "Reading %*s\r", -i, infile );

	  fflush( verfd );
	}

      *aargv = ++argv;

      return TRUE;
    }
  
} /* parsearg */

/*}}}*/
/*{{{  main() */

/********************************************************/
/* main                                                 */
/*                                                      */
/* Initialise everything and call the linker            */
/*                                                      */
/********************************************************/

PUBLIC int
main(
     int     argc,
     BYTE ** argv )
{
  WORD 		infiles = 0;
  clock_t	start=0, inend=0, end=0;
  
  
  /* IOdebug( "Linker: starting" ); */
  
  infd  = stdin;
  outf  = STDOUT;
  verfd = stderr;
  
  infile[ 0 ] = 0;
  errors      = 0;
  
  ProgName = *argv++;
  argc--;
  
  if (strrchr( ProgName, '/' ))
    ProgName = strrchr( ProgName, '/' ) + 1;
  
  /* IOdebug( "Linker: progname = %s", ProgName ); */
  
  start = clock();

  /* IOdebug( "Linker: start = %ld", start ); */
  
  if (setjmp( error_level ) == 0)
    {
      /* IOdebug( "Linker: setjmp completed, initialising" ); */
      
      initmem();
      initcode();
      initmodules();
      initsym();

      /* IOdebug( "Linker: saying hello to world" ); */
      
      report( "Helios Linker V1.4" );
      report( "Copyright (C) 1989 - 1993, Perihelion Software Ltd." );

      phase     = 1;
      readfiles = TRUE;
      
      while (*argv != NULL)
	{
	  /* IOdebug( "Linker: parsing arg %s", *argv ); */
	  
	  if (parsearg( &argv ) == TRUE )
	    {
	      /* IOdebug( "Linker: processing arg" ); */
	      
	      infiles++;
	      
	      readfile();
	      
	      tidy1();
	      
	      inlib = FALSE;
	    }

	  /* IOdebug( "Linker: arg parsed" ); */
	}

      /* IOdebug( "Linker: args parsed" ); */
      
      strcpy( infile, "" );   /*  no longer know what input file to blame errors on */
      
      infile_duplicate = NULL;
      
      if (infiles == 0)   /* if no command line files, use stdin */
	{
	  /* IOdebug( "Linker: processing stdin" ); */
	  
	  readfile();
	}
      
      readfiles = FALSE; 

      /* IOdebug( "Linker: finished last module" ); */
      
      genend();   /* finish off last module */
      
      inend = clock();
      
      setmodules();

      if (errors == 0)
	{
	  phase = 2;
	  
	  scancode();
	}   

      if (errors == 0)
	{
	  phase = 3;
	  
	  genimage();
	  
	  report( "Image file generated                                     " );
	  
	  if (needobjed)
	    {
	      if (outf == STDOUT)
		{
		  error( "Cannot set stack size/heap size when writing to stdout" );
		}
	      else
		{
		  objed( outf, progname, stack_size, heap_size );
		}
	    }
	}

#ifdef NEW_STUBS
      build_stubs();
#endif
      
    }
  
  end = clock();

  /* IOdebug( "Linker: finished processing" ); */
  
  report( "Times (secs): Total %d Input %d Output %d",
	 (end   - start) / CLK_TCK,
	 (inend - start) / CLK_TCK,
	 (end   - inend) / CLK_TCK );
  
  report( "Memory used:   Code     %8ld Local Heap  %8ld", codesize, heapsize );
  report( "               Symbols  %8ld\n", symsize );
  
  VMStats();
  
  tidyup();
  
  exit( (int)maxerr );
  
  return EXIT_SUCCESS;
  
  use( phase );
} /* main */

/*}}}*/

/*}}}*/

/* link.c */
