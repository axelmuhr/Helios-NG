/*{{{  Header */

/*
 * C => C & FORTRAN & Modula-2 compilers and assembler front end for Helios.
 *
 * Original version by Tim Glauert.
 *
 * Tidied up and extended by NC, THG, NHG, and PLC
 *
 * This code is Copyright (c) 1990, 1991, 1992, 1993, 1994 Perihelion Software Ltd.
 *   All Rights Reserved.
 *
 * RCS version: $Revision: 1.2 $
 * RCS date:    $Date: 1994/06/13 13:36:23 $
 */

#define VERSION		1
#define REVISION	64
#define DATE		05/10/93

/*}}}*/
/*{{{  Revision Log */

/*
 * 12 - added -q<> option to enable compiler debugging options
 *
 * 13 - allowed objed commands to be used even with a -B option
 *
 * 14 - added -DATW to automatic defines
 *
 * 15 - added -n<string> option to allow name of objed to be defined
 *
 * 16 - added multiple, comma seperated paths in -I
 *
 * 17 - added -R option to prevent loading RAM disk
 *
 * 18 - added environment variables
 *
 * 19 - FORTRAN 77 support added; uses posix execvp to execute programs 
 *
 * 20 - added -Fg option to remove procedure names
 *
 * 21 - made sure c fails if compiler(s) could not be found
 *
 * 22 - added automatic detection of RAM disk and reversed sense of -R option
 *
 * 23 - added -a, -z and -Z options to pass on flags to assembler and compilers respectivly
 *
 * -- missing versions 24 to 26 due to hard disk collapse -- I have tried to recreate them below
 *
 * 24 - added -Fn to turn off the vector stack
 *
 * 25 - added -r to allow compilation of device drivers
 *
 * 26 - added "-" as an output filename, meaning stdout
 *
 * -- back on track (I hope)
 *
 * 27 - added control-C handling - but not very good
 *
 * 28 - added -f option to generate name for virtual memory file
 *
 * 29 - prevented the inclusion of ',' in the search path for <> type includes
 *
 * 30 - allowed -s option to have a space before the number
 *
 * 31 - added file support for Rowley Modula-2 compiler.
 *	fixed some spelling and punctuation mistakes in the usage message.
 *
 * 32 - made -L option the same as the -l option (for backwards compatability)
 *      tidied up AddLibrary()
 *      added -wA and -FA options
 *      changed MACHINENAME to B008
 *
 * 33 - changed LIBDIR to C_LIBDIR and INCDIR to C_INCDIR to avoid conflicts 
 *      with the Imake utility
 *
 * 34 - added '*.lib' and 'lib*.a' to the library search pattern, and made an error messgae
 *      be produced if the library could not be found
 *
 * 35 - fixed AddLibrary() to distinguish between scanned and resident libraries
 *
 * 36 - added -E option and allowed .i as .c files
 *
 * 37 - recognise '.lib' as a valid extension for joining libraries
 *
 * 38 - recognises '-lm' as a UNIX abbreviation for the maths libraries
 *
 * 39 - added function AddLibraryName() to add names to the library list whilst
 *      preventing duplicate entries
 *
 * 40 - changed linking for C programs to use c0.o c.lib and helios.lib
 *
 * 41 - added -y option to pass text on to the macro assembler
 *
 * 42 - added -T5 option (for the T425),
 *      and sorted out multiple '-t' options in C compiler command line
 *      and added C_NONSTANDRD environment flag to enable various non-standard features
 *      and removed signal handling as Posix now does this automatically
 *
 * 43 - added support for compiling under UNIX
 *      and the use of pipes in non-standard mode
 *
 * 44 - added -T9 option (for the T810)
 *
 * 45 - fixed bug with pipe usage casuing C to return -1
 *
 * 46 - replaced use of access() with stat() so that we can detect non-file
 *      type names.
 *
 * 47 - changed -L option to match UNIX version and added -Wc,arg[,arg ..]
 *      compilation phase processing
 *
 * 48 - unknown!
 *
 * 49 - fixed AddLibrary, so that all libraries are linked with the -l option
 *    - fixed generation of FORTRAN temporary file names
 *
 * 50 - tried to fix bugs with return codes and library names
 *
 * 51 - Added /include to default search path for includes (this is Bart's include service)
 *
 * 52 - Added '-u' option to prevent files from being deleted
 *      Separated libraries into scanned and resident
 *      Added '-Fu' to enable the linker reporting unreferenced labels
 *
 * 53 - can now be used as a host utility on Sun4 etc.
 *
 * 54 - moved all machine dependent defines to makefile
 *
 * 55 - removed automatic detection of endian-ism of host as this is no longer needed
 *
 * 56 - fixed return codes to conform to POSIX spec
 *
 * 57 - added code to cope with being a front end to the new transputer compiler.
 *
 * 58 - added code to cope with the C40 compiler
 *
 * 59 - improved compiler type switching code
 *
 * 60 - added -J option to build libraries incrementally
 *
 * 61 - added OLD_TRAN to NEW_NCC options to allow old style transputer builds using environment
 *
 * 62 - cleaned up help text to reduce code and data sizes
 *
 * 63 - Added 68K support and folding in the sources
 *
 * 64 - Added RemoveLibraryPath function to cope with c_libdir being changed by command line switches
 */

/*}}}*/
/*{{{  Header Files */

#include <stdio.h>

#ifdef UNIX

#include "queue.h"

#ifndef true

#define bool 		int
#define false 		0
#define true  		1
#define Null( a )	((a *) NULL)
#define TRUE  		1
#define FALSE 		0

#endif

#else /* ! UNIX */

#include <helios.h>
#include <queue.h>

#endif /* UNIX */

#ifndef R140
#include <stdlib.h>
#else
#include <sys/types.h>
#endif

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <string.h>

#ifndef UNIX
#include <nonansi.h>
#include <posix.h>
#endif

#include <signal.h>

#ifndef R140

#include <unistd.h>

#ifndef UNIX
#include <syslib.h>
#endif

#endif /* R140 */

#ifndef UNIX
#include <syslib.h>
#endif

#ifdef RS6000
#define _XOPEN_SOURCE
#endif

#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

/*}}}*/
/*{{{  Constants */

#ifdef UNIX
#  define const
#endif

#ifdef R140
#  define SEEK_END 2
#  define L_tmpnam 25
typedef	int	pid_t;
extern int	errno;
#endif /* R140 */

#ifdef RS6000
#define vfork	fork
#endif

/* NB defines below are default values; should be defined in m. dep makefile. */

#ifdef UNIX

#ifndef SHELL
#define SHELL			"/bin/sh"
#endif

#ifndef PRE_PROCESSOR
#define PRE_PROCESSOR		"/lib/cpp"
#endif

#ifndef REMOVER
#define REMOVER			"/bin/rm"
#endif

#else /* ! UNIX */

#ifndef SHELL
#define SHELL			"/helios/bin/shell"
#endif

#ifndef PRE_PROCESSOR
#define PRE_PROCESSOR		"/helios/bin/ncc"
#endif

#ifndef REMOVER
#define REMOVER			"/helios/bin/rm"
#endif

#endif /* UNIX */

#ifndef TRANSPUTER_CC
#define TRANSPUTER_CC		"/helios/bin/cc"
#endif

#ifndef C40_CC
#define C40_CC			"/helios/bin/ncc"
#endif

#ifndef M68K_CC
#define M68K_CC			"/helios/bin/ncc"
#endif

#ifndef ARM_CC
#define ARM_CC			"/helios/bin/ncc"
#endif

#ifndef ARM_LTD_CC
#define ARM_LTD_CC		"armcc150"
#endif

#ifndef I860_CC
#define I860_CC			"/helios/bin/ncc"
#endif

#ifndef C_COMPILER
 #ifdef __TRAN
  #define C_COMPILER		TRANSPUTER_CC
 #else
  #ifdef __C40
  #define C_COMPILER		C40_CC
  #else
   #ifdef __ARM
   #define C_COMPILER		ARM_CC
   #else
    #ifdef __M68K
     #define C_COMPILER		M68K_CC
    #endif
   #endif
  #endif
 #endif
#endif

#ifndef F77_PASS1
# define F77_PASS1		"/helios/bin/f77p1"
#endif

#ifndef F77_PASS2
# define F77_PASS2		"/helios/bin/f77p2"
#endif

#ifndef M2_COMPILER
# define M2_COMPILER		"/helios/bin/mc"
#endif

#ifndef TRANSPUTER_ASSEMBLER
 #define TRANSPUTER_ASSEMBLER	"/helios/bin/asm"
#endif

#ifndef C40_ASSEMBLER
# define C40_ASSEMBLER		"/helios/bin/as"
#endif

#ifndef M68K_ASSEMBLER
# define M68K_ASSEMBLER		"/helios/bin/as"
#endif

#ifndef ARM_ASSEMBLER
 #define ARM_ASSEMBLER		"/helios/bin/as"
#endif

#ifndef ASSEMBLER
# ifdef __TRAN
#  define ASSEMBLER		TRANSPUTER_ASSEMBLER
# else
#  ifdef __C40
#   define ASSEMBLER		C40_ASSEMBLER
#  else
#   ifdef __ARM
#    define ASSEMBLER		ARM_ASSEMBLER
#   else
#    ifdef __M68K
#     define ASSEMBLER		M68K_ASSEMBLER
#    endif
#   endif
#  endif
# endif
#endif

#ifndef TRANSPUTER_LINKER
# define TRANSPUTER_LINKER	"/helios/bin/asm"
#endif

#ifndef C40_LINKER
# define C40_LINKER		"/helios/bin/ld"
#endif

#ifndef M68K_LINKER
# define M68K_LINKER		"/helios/bin/ld"
#endif

#ifndef ARM_LINKER
# define ARM_LINKER		"/helios/bin/ld"
#endif

#ifndef I860_LINKER
# define I860_LINKER		"/helios/bin/ld"
#endif

#ifndef LINKER
# ifdef __TRAN
#  define LINKER			TRANSPUTER_LINKER
# else
#  ifdef __C40
#   define LINKER			C40_LINKER
#  else
#   ifdef __ARM
#    define LINKER			ARM_LINKER
#   else
#    ifdef __M68K
#     define LINKER			M68K_LINKER
#    endif
#   endif
#  endif
# endif
#endif

#ifndef MACRO
# define MACRO			"/helios/bin/ampp"
#endif

#ifndef C40_C_LIBDIR
# define C40_C_LIBDIR		"/helios/lib/"
#endif

#ifndef M68K_C_LIBDIR
# define M68K_C_LIBDIR		"/helios/lib/"
#endif

#ifndef ARM_C_LIBDIR
# define ARM_C_LIBDIR		"/helios/lib/"
#endif

#ifndef TRANSPUTER_C_LIBDIR
# define TRANSPUTER_C_LIBDIR	"/helios/lib/"
#endif

#ifndef C_LIBDIR
# ifdef __TRAN
#  define C_LIBDIR		TRANSPUTER_C_LIBDIR
# else
#  ifdef __C40
#   define C_LIBDIR		C40_C_LIBDIR
#  else
#   ifdef __ARM
#    define C_LIBDIR		ARM_C_LIBDIR
#   else
#    ifdef __M68K
#     define C_LIBDIR		M68K_C_LIBDIR
#    endif
#   endif
#  endif
# endif
#endif

#ifndef C_INCDIR
# define C_INCDIR		"/helios/include/"
#endif

#ifndef MACDIR
# define MACDIR			"/helios/include/ampp/"
#endif

#ifndef SYMDIR
# define SYMDIR			"/helios/symbols/"
#endif

#define LINK_HELIOS		"helios.lib"
#define LINK_CSTART		"c0.o"
#define LINK_CLIB		"c.lib"
#define LINK_FSTART		"fstart.o"
#define LINK_M2START		"m2start.o"
#define BASIC			"basic.m"
#define OBJNAME			"a.out"

#ifndef ARM_MACHINENAME
#define ARM_MACHINENAME		"ARM"
#endif

#ifndef I860_MACHINENAME
#define I860_MACHINENAME	"i860"
#endif

#ifndef TRANSPUTER_MACHINENAME
#define TRANSPUTER_MACHINENAME	"TRANSPUTER"
#endif

#ifndef C40_MACHINENAME
#define C40_MACHINENAME		"TMS320C40"
#endif

#ifndef M68K_MACHINENAME
#define M68K_MACHINENAME	"M68000"
#endif

#ifndef MACHINENAME
# ifdef __ARM
#  define MACHINENAME		ARM_MACHINENAME
# else
#  ifdef __I860
#   define MACHINENAME		I860_MACHINENAME
#  else
#   ifdef __C40
#    define MACHINENAME		C40_MACHINENAME
#   else
#    ifdef __M68K
#     define MACHINENAME	M68K_MACHINENAME
#    else
#     ifdef __TRAN
#      define MACHINENAME	TRANSPUTER_MACHINENAME
#     endif
#    endif
#   endif
#  endif
# endif
#endif

#ifndef MACHINENAME
# define MACHINENAME		"Unknown_CPU_type"
#endif

#define ARGBUFSIZE	1024

/*}}}*/
/*{{{  Macros */

#define _is_norcroft_compiler	(compiler == ARM_COMPILER ||  \
				 compiler == ARM_LTD_COMPILER ||  \
				 compiler == C40_COMPILER ||  \
				 compiler == M68K_COMPILER || \
				 compiler == I860_COMPILER )

#define _is_transputer_compiler	(compiler == TRANSPUTER_COMPILER || compiler == UNKNOWN_COMPILER)

#define NEW( _type )	(_type *)SafeAlloc( sizeof( _type ) )


#define streq(  s1, s2 )	(strcmp(  s1, s2      ) == 0)
#define strneq( s1, s2, len )	(strncmp( s1, s2, len ) == 0)
#define strend( a, b )	        (streq( a + strlen( a ) - strlen( b ), b ))

/*}}}*/
/*{{{  Types */

typedef struct	_NameNode
  {
    Node 	node;		/* link into list */
    char *	name;		/* the name */
  }
NameNode;

typedef List 	NameList;

typedef struct	_FileNode
  {
    Node 	node;		/* link into list */
    char *	ext;		/* extension */
    char *	oext;		/* original extension */
    char *	root;		/* name of file less extension */
    char *	base;		/* name of file less directory and extension */
  }
FileNode;

typedef enum	_State
  {
    normal,
    objed_suppressed,
    objed_enabled
  }
State;

typedef List 	FileList;


typedef enum
  {
    UNKNOWN_COMPILER,
    TRANSPUTER_COMPILER,
    ARM_COMPILER,
    ARM_LTD_COMPILER,
    C40_COMPILER,
    M68K_COMPILER,
    I860_COMPILER
  }
compiler_type;

/*}}}*/
/*{{{  Local Variables */

static NameList		IncludeDirs;
static NameList		SymbolDirs;
static NameList		LibraryDirs;
static NameList		ScanLibraryNames;
static NameList		ResLibraryNames;
static NameList		Defines;

static NameList		CompileOpts;
static NameList		PreProcOpts;
static NameList		AssembleOpts;
static NameList		MacroOpts;
static NameList		LinkOpts;
static NameList		ModulaOpts;

static char 		FortranP1Opts[ 256 ];
static char		FortranP2Opts[ 256 ];
static char *		FortranMapFile;

static FileList		SourceFiles;

static int		NSources;
static int		NOfiles;
static int		NCfiles;
static int		NSfiles;
static int		NAfiles;
static int		NFfiles;
static int		NMfiles;
static int		NDfiles;

static int		NCompiles;

static FileNode	*	LastCompile;

static int		HeapSize;
static int		StackSize;

static char *		ObjectName;

static State		state = normal;

static bool		ram_disk_present;
static bool		suppress_removes = false;

static char *		vfile = NULL;

static char *		shell;
static char *		c_compiler;
static char *		pre_processor;
static char *		f77_pass1;
static char *		f77_pass2;
static char *		m2_compiler;
static char *		assembler;
static char *		linker;
static char *		remover;
static char *		macro;
static char *		link_helios;
static char *		c_libdir;
static char *		link_clib;
static char *		c_incdir;
static char *		macdir;
static char *		symdir;
static char * 		link_cstart;
static char * 		link_fstart;
static char *		link_m2start;
static char *		machinename;

static compiler_type	compiler = UNKNOWN_COMPILER;

static int		Flags;
#define FLAG_B		(1 << 0)
#define FLAG_J		(1 << 1)
#define FLAG_O		(1 << 2)
#define FLAG_P		(1 << 3)
#define FLAG_S		(1 << 4)
#define FLAG_V		(1 << 5)
#define FLAG_b		(1 << 6)
#define FLAG_c		(1 << 7)
#define FLAG_j		(1 << 8)
#define FLAG_n		(1 << 9)
#define FLAG_o		(1 << 10)
#define FLAG_p		(1 << 11)
#define FLAG_v		(1 << 12)

#define STDLIBS		((Flags & (FLAG_b | FLAG_B)) == 0)
#define VERIFY		((Flags & FLAG_v) != 0)
#define OPTIMISE	((Flags & FLAG_O) != 0)
#define ASSEMBLE	((Flags & (FLAG_S | FLAG_p)) == 0)
#define ASSEMBLE_ONLY	((Flags & FLAG_S) != 0)
#define LINK		((Flags & (FLAG_S | FLAG_p | FLAG_c)) == 0)
#define EXECUTE		((Flags & FLAG_n) == 0)
#define VERBOSE		((Flags & FLAG_V) != 0)
#define REDIRECT	((Flags & FLAG_o) != 0)
#define JOIN		((Flags & FLAG_j) != 0)
#define APPEND		((Flags & FLAG_J) != 0)
#define NOLIBS		((Flags & FLAG_B) != 0)
#define PRE_PROC_ONLY	((Flags & FLAG_p) != 0)
#define USE_PIPE	((Flags & FLAG_P) != 0)
#define COMPILE_ONLY	((Flags & FLAG_c) != 0)


static int		Warns;

#define WARN_a		(1 << 0)
#define WARN_d		(1 << 1)
#define WARN_f		(1 << 2)
#define WARN_i		(1 << 3)
#define WARN_n		(1 << 4)
#define WARN_o		(1 << 5)
#define WARN_p		(1 << 6)
#define WARN_s		(1 << 7)
#define WARN_u		(1 << 8)
#define WARN_v		(1 << 9)


static int		Errs;

#define ERR_c		(1 << 0)
#define ERR_p		(1 << 1)
#define ERR_z		(1 << 2)


static int		Feats;

#define FEAT_b		(1 <<  0)
#define FEAT_d		(1 <<  1)
#define FEAT_f		(1 <<  2)
#define FEAT_g		(1 <<  3)
#define FEAT_h		(1 <<  4)
#define FEAT_l		(1 <<  5)
#define FEAT_m		(1 <<  6)
#define FEAT_n		(1 <<  7)
#define FEAT_p		(1 <<  8)
#define FEAT_s		(1 <<  9)
#define FEAT_u		(1 << 10)
#define FEAT_v		(1 << 11)
			 
static char *		ObjName;
static char *		ProgName = NULL;

static bool		use_non_standard_features = false;

/*}}}*/
/*{{{  Forward Declarations */

#ifdef __STDC__
static int ExecuteCommandLine( void );
#else
static int ExecuteCommandLine();
#endif

/*}}}*/
/*{{{  Functions */

/*{{{  Utility Routines */

/* #define DEBUG */

/*{{{  debug */

#ifdef DEBUG

static void
#ifdef __STDC__
debug( char * format, ... )
#else
debug( format, va_alist )
  char * format;
  va_dcl
#endif
  /*
   * print an debugging message
   */
{
  va_list	args;
  
  
#ifdef __STDC__    
  va_start( args, format );
#else
  va_start( args );
#endif
  
  fflush( stderr );
  
  fseek( stderr, 0L, SEEK_END );
  
  if (ProgName)
    fprintf( stderr, "%s: ", ProgName);
  
  vfprintf( stderr, format, args );
  
  fprintf( stderr, "\n" );
  
  fflush( stderr );
  
  va_end( args );
  
  return;
  
} /* debug */
    
#endif /* DEBUG */

/*}}}*/
/*{{{  error */

static void
#ifdef __STDC__
error( char * format, ... )
#else
error( format, va_alist )
  char *	format;
  va_dcl
#endif
  /*
   * print an error message
   */
{
  va_list	args;
  
  
#ifdef __STDC__    
  va_start( args, format );
#else
  va_start( args );
#endif
  
  fflush( stderr );
  
  fseek( stderr, 0L, SEEK_END );
  
  vfprintf( stderr, format, args );
  
  fprintf( stderr, "\n" );
  
  fflush( stderr );
  
  va_end( args );
  
  return;
  
} /* error */

/*}}}*/
/*{{{  warning */

  
static void
#ifdef __STDC__
warning( char * format, ... )
#else
warning( format, va_alist )
  char *	format;
  va_dcl
#endif
  /*
   * print a warning message
   */
{
  va_list	args;
  
  
#ifdef __STDC__    
  va_start( args, format );
#else
  va_start( args );
#endif
  
  fflush( stderr );
  
  fseek( stderr, 0L, SEEK_END );
  
  if (ProgName)
    fprintf( stderr, "%s: warning: ", ProgName);
  
  vfprintf( stderr, format, args );
  
  fprintf( stderr, "\n" );
  
  fflush( stderr );
  
  va_end( args );
  
  return;
  
} /* warning */

/*}}}*/
/*{{{  fail */

  
static void
#ifdef __STDC__
fail( char * format, ... )
#else
fail( format, va_alist )
  char *	format;
  va_dcl
#endif
  /*
   * print a reason message and terminate
   */
{
  va_list	args;
  
  
  signal( SIGINT, SIG_DFL );
  
#ifdef __STDC__    
  va_start( args, format );
#else
  va_start( args );
#endif
  
  fflush( stderr );
  
  fseek( stderr, 0L, SEEK_END );
  
  fprintf( stderr, "%s:- ", ProgName );
  
  vfprintf( stderr, format,  args );
  
  fprintf( stderr, "\n" );
  
  fflush( stderr );
  
  va_end( args );
  
  exit( 1 );
  
} /* fail */

/*}}}*/
/*{{{  SafeAlloc */

static char *
#ifdef __STDC__
SafeAlloc( unsigned long size )
#else
SafeAlloc( size )
  unsigned long size;
#endif
  /*
   * allocate 'size' bytes of memory or fail
   */
{
  char *	ptr = (char *)malloc( (size_t)size );
  
  
  if (ptr == NULL)
    {
      fail( "malloc failed (for %lu bytes)", size );
    }
  
  return ptr;
  
} /* SafeAlloc */

/*}}}*/

/*}}}*/
/*{{{  List Routines */

/*{{{  InitNode() */

static void
#ifdef __STDC__
InitNode( Node * pNode )
#else
InitNode( pNode )
  Node * pNode;
#endif
  /*
   * Initialises a node so that it is self-referential
   */
{
  if (pNode != Null( Node ))
    {
      pNode->Next = pNode->Prev = pNode;
    }
  
  return;
      
} /* InitNode */

/*}}}*/
/*{{{  First Node */

static Node *
#ifdef __STDC__
FirstNode( List * plist )
#else
FirstNode( plist )
  List * plist;
#endif
  /*
   * returns the first node in the given list or NULL if the list is empty
   */
{
  if (plist 		== Null( List ) ||
      plist->Head 	== Null( Node ) ||
      plist->Head->Next == Null( Node ) )
    {
      return Null( Node );
    }
  
  return plist->Head;
  
} /* FirstNode */

/*}}}*/
/*{{{  NextNode */

static Node *
#ifdef __STDC__
NextNode( Node * pnode )
#else
NextNode( pnode )
  Node * pnode;
#endif
  /*
   * returns the next node in the list containing 'pnode' or NULL if this
   * is the last node in the list or the node is not attached to a list
   */
{
  if (pnode 		== Null( Node ) ||
      pnode->Next 	== Null( Node ) ||
      pnode->Next->Next == Null( Node ) )
    {
      return Null( Node );
    }
  
  return pnode->Next;
  
} /* NextNode */

/*}}}*/
/*{{{  ListSize */

  
static int
#ifdef __STDC__
ListSize( List * plist )
#else
ListSize( plist )
  List * plist;
#endif
  /*
   * returns the number of elements in the list provided
   */
{
  int		n = 0;
  Node *	pnode;
  
  
  for (pnode = FirstNode( plist ); pnode; pnode = NextNode( pnode ))
    n++;
  
  return n;
  
} /* ListSize */

/*}}}*/
/*{{{  DupName */

  
static char *
#ifdef __STDC__
DupName( char * pname )
#else
DupName( pname )
  char * pname;
#endif
  /*
   * returns a copy of the string provided
   */
{
  char *	ptr = SafeAlloc( (unsigned long)strlen( pname ) + 1 );
    
    
  strcpy( ptr, pname );
    
  return ptr;
    
} /* DupName */

/*}}}*/
/*{{{  AddName */

  
static void
#ifdef __STDC__
AddName(
	NameList *	plist,
	char *		pname )
#else
AddName( plist, pname )
  NameList *	plist;  
  char *	pname;
#endif
  /*
   * add a new node to the list containing the string 'name'
   */	
{
  NameNode *	pnew = NEW( NameNode );
  
  pnew->name = DupName( pname );

  InitNode( &pnew->node );
  
  AddTail( (List *)plist, &pnew->node );
  
  return;
  
} /* AddName */

/*}}}*/
/*{{{  RemoveName */

  
static bool
#ifdef __STDC__
RemoveName(
	   NameList *	plist,
	   char * 	pname )
#else
RemoveName( plist, pname )
  NameList *	plist;  
  char * 	pname;
#endif
  /*
   * remove a name from the given list
   * returns TRUE upon success, FALSE otherwise
   * only removes the first occurance of pname
   */
{
  NameNode *	pnode;
  
  
  for (pnode  = (NameNode *)FirstNode( (List *)plist );
       pnode != NULL;
       pnode  = (NameNode *)NextNode( (Node *)pnode ))
    {
      if (streq( pnode->name, pname ))
	{
	  (void) Remove( (Node *)pnode );
	  
	  free( pnode );
	  
	  return TRUE;
	}
    }
  
  return FALSE;
  
} /* RemoveName */

/*}}}*/
/*{{{  AddNameToFront */

  
static void
#ifdef __STDC__
AddNameToFront(
	       NameList *	plist,
	       char *		pname )
#else
AddNameToFront( plist, pname )
  NameList *	plist;
  char *	pname;
#endif
  /*
   * add a new node to the list containing the string 'name'
   * name is added to the head of the list
   */	
{
  NameNode *	pnew = NEW( NameNode );
  
  
  pnew->name = DupName( pname );
  
  if (strchr( pname, ' ' ))
    {
      warning( "blank(s) found in argument '%s'", pname );
    }

  InitNode( &pnew->node );
  
  AddHead( (List *)plist, &pnew->node );
  
  return;
  
} /* AddNameToFront */

/*}}}*/
/*{{{  AddNamef */

static void
#ifdef __STDC__
AddNamef(
	 NameList *	plist,
	 char *		format,
	 ...		)
#else
AddNamef( plist, format, va_alist )
  NameList *	plist;
  char *	format;
  va_dcl
#endif
  /*
   * adds a node to a list containing a constructed string
   */	
{
  static char	buffer[ ARGBUFSIZE ];
  va_list	args;
  
  
#ifdef __STDC__    
  va_start( args, format );
#else
  va_start( args );
#endif
  
  vsprintf( buffer, format, args );
  
  va_end(   args );
  
  AddName( plist, buffer );
  
  return;
  
} /* AddNamef */

/*}}}*/
/*{{{  AddLibraryName */

  
static void
#ifdef __STDC__
AddLibraryName(
	       bool	resident,
	       char *	format,
	       ...	)
#else
AddLibraryName( resident, format, va_alist )
  bool		resident;
  char *	format;
  va_dcl
#endif
  /*
   * adds a name to the Library Name list
   */	
{
  static char	buffer[ ARGBUFSIZE ];
  va_list	args;
  NameList *	plibs;
  NameNode *	pname;
  
  
  if (resident)
    {
      plibs  = &ResLibraryNames;
    }
  else
    {
      plibs  = &ScanLibraryNames;
    }
  
  /* format the name */
  
#ifdef __STDC__    
  va_start( args, format );
#else
  va_start( args );
#endif
  
  vsprintf( buffer, format, args );
  
  va_end(   args );
  
  /* check to see if this name already exists */
  
  for (pname  = (NameNode *)FirstNode( (List *)plibs );
       pname != NULL;
       pname  = (NameNode *)NextNode( (Node *)pname ) )
    {
      if (streq( pname->name, buffer ))
	break;
    }
  
  if (pname == NULL)
    {
      /* add the name to the end of the list */
      
      AddName( plibs, buffer );
    }
  
  return;
  
} /* AddLibraryName */

/*}}}*/
/*{{{  AddFile */

static void
#ifdef __STDC__
AddFile(
	FileList *	plist,
	char *		file )
#else
AddFile( plist, file )
  FileList *	plist;
  char *	file;
#endif
  /*
   * adds a file name to the the file name list
   * seperates the filename and extension
   */	
{
  FileNode *	New = NEW( FileNode );
  char *	tmp = DupName( file );
  char *	p   = strrchr( tmp, '.' );
  
  
  if (p == NULL)
    {
      New->ext = DupName( "" );
    }
  else
    {
      *p++     = '\0';
      New->ext = DupName( p );
    }
  
  p = strrchr( tmp, '/' );
  
  if (p == NULL)
    {
      p = tmp;
    }
  else
    {
      p++;
    }
  
  New->base = DupName( p );
  New->oext = New->ext;
  New->root = DupName( tmp );

  InitNode( &New->node );
  
  AddTail( (List *)plist, &New->node );
  
  free( tmp );
  
  return;
  
} /* AddFile */

/*}}}*/
/*{{{  AddNames */

static void
#ifdef __STDC__
AddNames(
	 NameList *	nlist,
	 char *		format,
	 NameList *	plist )
#else
AddNames( nlist, format, plist )
  NameList *	nlist;
  char *	format;
  NameList *	plist;
#endif
  /*
   * merge two name lists
   */	
{
  NameNode *	This;
  
  
  for (This = (NameNode *)FirstNode( (List *)plist );
       This;
       This = (NameNode *)NextNode( (Node *)This ))
    {
      AddNamef( nlist, format, This->name );
    }
  
  return;
  
} /* AddNames */

/*}}}*/
/*{{{  AddFiles */

  
static void
#ifdef __STDC__
AddFiles(
	 NameList *	nlist,
	 char *		format,
	 FileList *	plist )
#else
AddFiles( nlist, format, plist )
  NameList *	nlist;
  char *	format;
  FileList *	plist;
#endif  
  /*
   * merge two file name lists
   */	
{
  FileNode *	This;
  
  
  for (This = (FileNode *)FirstNode( (List *)plist );
       This;
       This = (FileNode *)NextNode( (Node *)This ))
    {
      AddNamef( nlist, format, This->root, This->ext );
    }
  
  return;
  
} /* AddFiles */

/*}}}*/
/*{{{  AddLibrary */

/*
 * NB/ It is not sufficient to use access() here, as it is possible
 * to have a library name that matches a directory name, eg libX11.a vs X11/
 */

#define	file_exists( name )	(stat( name, &status ) == 0 && (status.st_mode & S_IFREG) == S_IFREG)
  
  
static bool
#ifdef __STDC__
AddLibrary(
	   char *	dir,
	   char *	stem )
#else
AddLibrary( dir, stem )
  char *	dir;
  char *	stem;
#endif
  /*
   * search for a library called 'stem' in directory 'dir'
   * if the library is found, add its name to the library list
   * returns true iff the library was found, false otherwise
   */
{
  static char	buf[ 100 ];	/* XXX */
  struct stat	status;  
  bool		resident  = false;
  char *	seperator = dir[ strlen(dir) - 1 ] == '/' ? "" : "/";
  

  if (strchr( stem, '.' ))
    {
      /* stem already has an extension - do not try adding one */

      sprintf( buf, "%s%s%s", dir, seperator, stem );
      
      if (file_exists( buf ))
	{
	  char * ptr = strchr( stem, '.' ) + 1;
	  
	  
	  if (streq( ptr, "def" ))
	    {
	      goto got_resident;
	    }
	  else
	    {
	      goto got_scanned;
	    }
	}
    }
  else
    {
      sprintf( buf, "%s%s%slib.def", dir, seperator, stem );
      
      if (file_exists( buf ))
	goto got_resident;
      
      sprintf( buf, "%s%s%s.def", dir, seperator, stem );
      
      if (file_exists( buf ))
	goto got_resident;
      
      sprintf( buf, "%s%s%slib", dir, seperator, stem );
      
      if (file_exists( buf ))
	goto got_scanned;
      
      sprintf( buf, "%s%s%s.lib", dir, seperator, stem );
      
      if (file_exists( buf ))
	goto got_scanned;
      
      sprintf( buf, "%s%s%s", dir, seperator, stem );
      
      if (file_exists( buf ))
	goto got_scanned;
      
      sprintf( buf, "%s%slib%s.a", dir, seperator, stem );
      
      if (file_exists( buf ))
	goto got_scanned;
      
      if (streq( stem, "m" ))
	{
	  /*
	   * UNIX style maths libraries
	   */
	  
	  if (!STDLIBS)
	    {
	      if (c_libdir[ strlen( c_libdir ) - 1 ] == '/')
		{
		  AddLibraryName( true, "%sfplib.def",  c_libdir );
		  AddLibraryName( true, "%sfpclib.def", c_libdir );
		}
	      else
		{
		  AddLibraryName( true, "%s/fplib.def",  c_libdir );
		  AddLibraryName( true, "%s/fpclib.def", c_libdir );
		}
	    }
	  
	  return TRUE;
	}
    }
  
  return FALSE;
  
 got_resident:
  
  if (streq( stem, "X" ))
    {
      /*
       * adding in the resident X library make sure that there is
       * sufficient stack space for the program.  (What, hack, us !?)
       */
      
      if (StackSize < 10000)
	StackSize = 10000;
    }
  
  resident = true;
  
 got_scanned:
  if (streq( stem, link_helios ) || streq( stem, link_clib ))
    {
      /* these two are actually resident libraries, despite their extension */
      
      resident = true;
    }
  
  AddLibraryName( resident, "-l%s", buf );
  
  return TRUE;
  
} /* AddLibrary */

/*}}}*/
/*{{{  FindAndAddLibrary */

static void
#ifdef __STDC__
FindAndAddLibrary( char * pfile_name )
#else
FindAndAddLibrary( pfile_name )
  char * pfile_name;
#endif
/*
 * attempts to locate the correct directory and name of the indicated
 * library, and then adds it to the library name list
 */
{
  NameList *	pdirs = &LibraryDirs;
  NameNode *	pdir_name;


  if (strrchr( pfile_name, '/' ))
    {
      struct stat	status;


      if (file_exists( pfile_name ))
	{
	  AddLibraryName( false, "-l%s", pfile_name );

	  return;
	}
    }
  else
    {
      for (pdir_name  = (NameNode *)FirstNode( (List *)pdirs );
	   pdir_name != NULL;
	   pdir_name  = (NameNode *)NextNode( (Node *)pdir_name ))
	{
	  if (AddLibrary( pdir_name->name, pfile_name ))
	    {
	      return;
	    }
	}
    }
  
  warning( "failed to locate library '%s'", pfile_name );
  
  return;
  
} /* FindAndAddLibrary */

/*}}}*/

/*}}}*/
/*{{{  Execution */

static char 	command_line[  1024 ];		/* XXX */
static char 	command_line2[ 1024 ];		/* XXX */

/*{{{  ExecuteList */

static int
#ifdef __STDC__
ExecuteList( NameList * nlist )
#else
ExecuteList( nlist )
  NameList * nlist;
#endif
/*
 * execute the indicate command with the provided arguments
 * wait for the command to terminate and return its exit code
 */
{
  pid_t		pid;
  int		i;
  int		result 	= -1;
  int		argc   	= ListSize( (List *)nlist );
  char **	argv 	= (char **)SafeAlloc( ((unsigned long)argc + 1) * sizeof( char * ) );
  NameNode *	pnode;
  char *	cmdline;
  

#ifdef DEBUG
  debug( "ExecuteList() called" );
#endif
  
  if (USE_PIPE && command_line[ 0 ] != '\0')
    {
      cmdline = command_line2;

      cmdline[ 0 ] = '\0';
    }
  else
    {
      cmdline = command_line;
    }
    
  i = 0;
    
  for (pnode = (NameNode *)FirstNode( (List *)nlist );
       pnode;
       pnode = (NameNode *)NextNode( (Node *)pnode ) )
    {
      if (USE_PIPE)
	{
	  strcat( cmdline, pnode->name );
	  strcat( cmdline, " " );
	    
	  continue;
	}
      
      argv[ i ] = pnode->name;

      while (isspace( argv[ i ][ 0 ]))
	argv[ i ] += 1;

      if (VERIFY)
	printf( "%s ", argv[ i ] );
      
      ++i;
    }
    
  argv[ i ] = NULL;
    
  if (!USE_PIPE && VERIFY)
    {
      printf( "\n" );
    }
    
  if (!EXECUTE || USE_PIPE)
    {
      free( argv );
      
      return 0;
    }
    
  if ((pid = vfork()) == 0)
    {
      int	return_code;
      
	
      return_code = execvp( argv[ 0 ], argv );
	
      _exit( return_code );
    }
    
  if (pid == -1)
    {
      fail( "Failed to execute %s", argv[ 0 ] );
    }
    
  while (wait( &result ) == -1 && errno == EINTR)
    ;
    
  if (result != 0)
    {
      warning( "Failed because %s exited with return code of %d", argv[ 0 ], result );
    }
    
  return result;
    
} /* ExecuteList */

/*}}}*/
/*{{{  command_line_to_argv */

static char **
#ifdef __STDC__
command_line_to_argv( char * command_line )
#else
command_line_to_argv( command_line )
  char *  command_line;
#endif
{
  static char *	argv[ 200 ];		/* XXX */
  char *	ptr;
  int		index;
  char		c;
  bool		found_quote;
  

  if (command_line == NULL)
    return NULL;

  ptr   = command_line;
  index = 0;
  found_quote = false;
  
  
  while ((c = *ptr++) != '\0')
    {
      /* skip past character after a backslash */
      
      if (c == '\\')
	if ((c = *ptr++) == '\0')
	  break;

      /* skip characters between double quotes */
      
      if (c == '"')
	{
	  if (found_quote)
	    found_quote = false;
	  else
	    found_quote = true;
	}

      /* break arguments at white space */
      
      if (isspace( c ) && !found_quote)
	{
	  /* point to start of command line argument */
	  
	  argv[ index++ ] = command_line;

	  /* terminate the argument */
	  
	  ptr[ -1 ] = '\0';

	  /* advance pointer to next non white space character */
	  
	  while ((c = *ptr) != '\0' && isspace( c ))
	    ptr++;

	  /* remember start of next bit of command line */
	  
	  command_line = ptr;
	}
    }

  /* terminate argv array */
  
  argv[ index ] = NULL;

  /* finished */

  return argv;
  
} /* command_line_to_argv */

/*}}}*/
/*{{{  signal_handler */

static pid_t	proc1 = -1;	/* process ID of first  process */
static pid_t	proc2 = -1;	/* process ID of second process */

void
#ifdef __STDC__
signal_handler( int sig )
#else
signal_handler( sig )
  int	sig;
#endif
{
  /* broadcast signal to child processes */
  
  if (proc1 != -1)
    kill( proc1, sig );

  if (proc2 != -1)
    kill( proc2, sig );

  /* finished */
  
  return;
  
} /* signal_handler */

/*}}}*/
/*{{{  ExecuteCommandLine */

static int
#ifdef __STDC__
ExecuteCommandLine( void )
#else
ExecuteCommandLine()
#endif
/*
 * execute the commands built up in the command line
 * wait for the commands to terminate and return the exit code
 */
{
  int		result 	= -1;
  pid_t		pid;
  char **	argv;
  void (*	old_handler)();

      
#ifdef DEBUG
  debug( "ExecuteCommandLine() called" );
#endif
      
  if (!USE_PIPE || command_line[ 0 ] == '\0')
    {
      return 0;
    }

  if (command_line2[ 0 ] != '\0')
    {
      int	fds[ 2 ];
      int	result2 = -1;

      
      if (VERIFY)
	{
	  printf( "%s |\n", command_line );
	  printf( "%s\n",   command_line2 );
	}
    
      if (!EXECUTE)
	{
	  command_line[  0 ] = '\0';
	  command_line2[ 0 ] = '\0';
      
	  return 0;
	}

      /* create a pipe to connect the two processes */
      
      if (pipe( fds ) != 0)
	{
	  fail( "Failed to create a pipe" );
	}

      /* convert command line to argv format */
      
      argv = command_line_to_argv( command_line );
	  
      /* set up a signal handler to process interrupts */
      
      if ((old_handler = signal( SIGINT, signal_handler )) == SIG_ERR)
	{
	  fail( "failed to set up signal handler" );
	}

      /* create first command line processes */
      
      if ((proc1 = vfork()) == 0)
	{
	  /* close stdout */
	  
	  close( 1 );

	  /* duplicate write channel of pipe as stdout */
	  
	  dup2( fds[ 1 ], 1 );

	  /* close unused file descriptors */
	  
	  close( fds[ 0 ] );
	  close( fds[ 1 ] );
	  
	  /* and run the first command line */
	  
	  _exit( execvp( argv[ 0 ], argv ) );
	}

      /* check to see if vfork worked */
      
      if (proc1 == -1)
	{
	  fail( "Failed to create child process for %s", command_line );
	}
      
      /* convert second command line to argv */
	  
      argv = command_line_to_argv( command_line2 );
	  
      /* create second command line processes */
      
      if ((proc2 = vfork()) == 0)
	{
	  /* close stdin */
	  
	  close( 0 );

	  /* duplicate read channel of pipe as stdin */
	  
	  dup2( fds[ 0 ], 0 );

	  /* close unused file descriptors */
	  
	  close( fds[ 0 ] );
	  close( fds[ 1 ] );

	  /* and run the second command line */
	  
	  _exit( execvp( argv[ 0 ], argv ) );
	}
      
      /* check to see if second vfork failed */
      
      if (proc2 == -1)
	{
	  fail( "Failed to create child process for %s", command_line2 );
	}

      /* close unused file descriptors */
      
      close( fds[ 0 ] );
      close( fds[ 1 ] );
	  
      /* wait for one child to finish */
      
      while ((pid = wait( &result )) == -1 && errno == EINTR)
	;

      /* mark the child process as having been terminated */
      
      if (pid == proc1)
	proc1 = -1;
      else if (pid == proc2)
	proc2 = -1;
      else
	warning( "unknown child process %d has terminated", pid );
	
      /* wait for other child to finish */
      
      while ((pid = wait( &result2 )) == -1 && errno == EINTR)
	;

      /* mark the other process as having been terminated */
      
      if (pid == proc1)
	proc1 = -1;
      else if (pid == proc2)
	proc2 = -1;
      else
	warning( "unknown child process %d has terminated", pid );
      
      if (result != 0)
	{
	  warning( "Failed because %s exited with return code of %d", command_line, result );
	}
      
      if (result2 != 0)
	{
	  warning( "Failed because %s exited with return code of %d", command_line2, result2 );
	}
      
      /* merge results */
      
      if (result2 != 0)
	result = result2;
    }
  else
    {
      if (VERIFY)
	{
	  printf( "%s\n", command_line );
	}
    
      if (!EXECUTE)
	{
	  command_line[ 0 ] = '\0';
      
	  return 0;
	}
    
      argv = command_line_to_argv( command_line );
	  
      if ((old_handler = signal( SIGINT, signal_handler )) == SIG_ERR)
	{
	  fail( "failed to set up signal handler" );
	}
      
      if ((proc1 = vfork()) == 0)
	{
	  _exit( execvp( argv[ 0 ], argv ) );
	}
      
      if (proc1 == -1)
	{
	  fail( "Failed to create child process for %s", command_line );
	}
    
      while ((pid = wait( &result )) == -1 && errno == EINTR)
	;
      
      if (pid == proc1)
	proc1 = -1;
      else
	warning( "unknown child process %d has terminated", pid );
	
      if (result != 0)
	{
	  warning( "Failed because %s exited with return code of %d", command_line, result );
	}
    }
  
  /* reset signal handler */

  (void) signal( SIGINT, old_handler );
  
  /* mark command lines as used */
      
  command_line[ 0 ]  = '\0';
  command_line2[ 0 ] = '\0';

  return result;
    
} /* ExecuteCommandLine */

/*}}}*/
/*{{{  RemoveFile */

static void
#ifdef __STDC__
RemoveFile( FileNode * pfile )
#else
RemoveFile( pfile )
  FileNode * pfile;
#endif
/*
 * deleted the indicated file
 */
{
  NameList	args;
  List *	plist = (List *)&args;
    

#ifdef DEBUG
  debug( "RemoveFile() called" );
#endif
  
  if (suppress_removes)
    return;
  
  InitList( plist );
    
  AddNamef( plist, "%s",    remover );
  AddNamef( plist, "%s.%s", pfile->root, pfile->ext );
  
  (void) ExecuteList( plist );

  return;
  
} /* RemoveFile */

/*}}}*/
/*{{{  C_Compile */

static int
#ifdef __STDC__
C_Compile( FileNode * fnode )
#else
C_Compile( fnode )
  FileNode * fnode;
#endif
/*
 * compile the indicated C file 
 * returns exit code of compilation
 */
{
  NameList	args;
  List *	plist  = (List *)&args;
  int		result = 0;
    
    
#ifdef DEBUG
  debug( "C_Compile() called" );
#endif
  
  InitList( plist );
    
  AddNamef( plist, "%s",   c_compiler   );
  AddNames( plist, "%s",   &CompileOpts );
  AddNamef( plist, "%s.%s", fnode->root, fnode->oext );

  if (REDIRECT && !_is_transputer_compiler && COMPILE_ONLY)
    {
      /*
       * XXX beware of compromising
       *
       * c fred.c -o fred
       * c -c fred.c -o fred
       */
      
      if (ObjName != NULL)
	{
	  if (_is_norcroft_compiler)
	    AddName( plist, "-o" );
	  else
	    AddName( plist, "-s" );

	  AddName(  plist, ObjName );
	}
    }
  else if (!USE_PIPE)
    {
      if (_is_norcroft_compiler)
	{
	  char *	name;


	  AddName( plist, "-o" );

	  name = fnode->root;

	  if ((name = strrchr( name, '/' )) != NULL)
	    {
	      name++;
	    }
	  else
	    {
	      name = fnode->root;	      
	    }
	  
	  if (ASSEMBLE_ONLY)
	    {
	      AddNamef( plist, "%s.s", name );
	    }
	  else
	    {
	      AddNamef( plist, "%s.o", name );
	    }
	}
      else
	{
	  AddName(  plist, "-s"                );
	  AddNamef( plist, "%s.s", fnode->root );
	}
    }
  else if (_is_norcroft_compiler && !COMPILE_ONLY && !ASSEMBLE_ONLY)
    {
      /*
       * By default Norcroft compiler's will create their own output
       * file name, so we must explicity override it
       */

      AddName( plist, "-o" );

      AddName( plist, "-" );
    }
  else if ((COMPILE_ONLY || ASSEMBLE_ONLY)
	   && ObjName != NULL
	   && ObjName != ((FileNode *)FirstNode( &SourceFiles ))->base
	   && !_is_transputer_compiler)
    {
      /*
       * force rename of output of compiler
       */

      AddName( plist, "-o" );

      AddName( plist, ObjName );
    }
  
    
  result = ExecuteList( plist );
  
  NCompiles++;
    
  LastCompile = fnode;
  
  if (_is_norcroft_compiler)
    {
      if (ASSEMBLE_ONLY)
	{
	  fnode->ext  = DupName( "s" );
	}
      else
	{
	  fnode->ext  = DupName( "o" );
	}
    }
  else
    {
      fnode->ext  = DupName( "s" );
    }
  
  return result;
    
} /* C_Compile */

/*}}}*/
/*{{{  Pre_Process */

static int
#ifdef __STDC__
Pre_Process( FileNode * fnode )
#else
Pre_Process( fnode )
  FileNode * fnode;
#endif
/*
 * pre_process the indicated file
 * returns exit code of compilation
 */
{
  NameList	args;
  List *	plist  = (List *)&args;
  int		result = 0;
  bool		using_cpp;
    
#ifdef DEBUG
  debug( "PreProcess() called" );
#endif
  
  using_cpp = strend( pre_processor, "cpp" );
  
  InitList( plist );

  AddNamef( plist, "%s",    pre_processor );
  if (!using_cpp)
    AddName( plist, "-E" );
  AddNames( plist, "%s",    &PreProcOpts  );
  AddNames( plist, "-D%s",  &Defines      );
  AddNames( plist, "-I%s",  &IncludeDirs  );
  AddNamef( plist, "%s.%s", fnode->root, fnode->oext );
  if (REDIRECT)
    {
      if (using_cpp)
	AddName( plist, ObjectName );
      else
	warning( "Unable to redirect output of pre-processor" );
    }
  else if (using_cpp)
    AddNamef( plist, "%s.i",  fnode->root   );
    
  result = ExecuteList( plist );
    
  NCompiles++;
    
  LastCompile = fnode;
    
  fnode->ext  = DupName( "i" );
    
  return result;
    
} /* pre_process */

/*}}}*/
/*{{{  F77_Compile */

#ifndef __ARM
static int
#ifdef __STDC__
F77_Compile( FileNode * fnode )
#else
F77_Compile( fnode )
  FileNode * fnode;
#endif
/*
 * compile the indicated FORTRAN file 
 * returns exit code of compilation
 */
{
  NameList	args;
  List *	plist    = (List *)&args;
  bool		pipe_off = false;
  int		result   = 0;
  char		buffer[ L_tmpnam + 1 ];
  char *	ptmp;
  char *	pname;
    
    
  if (USE_PIPE)
    {
      /* switch off piping for FORTRAN compiles */
      
      pipe_off = true;
      
      Flags &= ~FLAG_P;
    }
    
  InitList( plist );
    
  AddNamef( plist, "%s",      f77_pass1   );
  AddNamef( plist, "%s.f",    fnode->root );

  /* generate a temporary file name */

  /*
   * first get a temporary name, then extract the file name part, then add in our own root
   * This is highly grungy, but will actually work as the C library version of tmpnam()
   * does not check to see if the name it creates is truely unique with respect to the
   * directory it has chosen
   */

  (void) tmpnam( buffer );

  if ((ptmp = strrchr( buffer, '/' )) == NULL)
    {
      ptmp = buffer;
    }
  else
    {
      ++ptmp;
    }

  if ((pname = getenv( "TMPDIR" )) == NULL)
    {
      if (ram_disk_present)
	{
	  pname = "/ram/tmp";
	}
      else
	{
	  pname = "/helios/tmp";
	}
    }
  
  AddNamef( plist, "%s/%s", pname, ptmp );
      
  if (FortranP1Opts[ 0 ])
    {
      AddName( plist, ","           );
      AddName( plist, FortranP1Opts );
    }
    
  if ((result = ExecuteList( plist )) == 0)
    {
      InitList( plist );
      
      AddNamef( plist, "%s", f77_pass2 );
      
      AddNamef( plist, "%s/%s", pname, ptmp );
      
      AddNamef( plist, "%s.s", fnode->root );
      
      if (FortranP2Opts[ 0 ] ||
	  FortranMapFile)
	{
	  AddName( plist, "," );
	  AddName( plist, "," );
	  
	  if (FortranP2Opts[ 0 ])
	    {
	      AddName( plist, FortranP2Opts );
	    }
	  else
	    {
	      AddName( plist, "," );
	    }
	  
	  if (FortranMapFile)
	    {
	      AddName( plist, FortranMapFile );
	    }
	}
      
      result = ExecuteList( plist );
    }
      
  NCompiles++;
    
  LastCompile = fnode;
    
  fnode->ext  = DupName( "s" );
    
  if (pipe_off)
    {
      Flags |= FLAG_P;
    }
  
  return result;
    
} /* F77_Compile */
#endif /* not __ARM */

/*}}}*/
/*{{{  M2_CompileDef */

#ifndef __ARM

static int
#ifdef __STDC__
M2_CompileDef( FileNode * fnode )
#else
M2_CompileDef( fnode )
  FileNode * fnode;
#endif
/*
 * compile the indicated Modula-2 definition file
 * returns exit code of compilation
 */
{
  NameList	args;
  List *	plist  = (List *)&args;
  int		result = 0;
    
    
  InitList( plist );
    
  AddNamef( plist, "%s",     m2_compiler );
  AddNames( plist, "%s",     &ModulaOpts );
  AddNamef( plist, "%s.def", fnode->root );
  
  result = ExecuteList( plist );
  
  return result;
    
} /* M2_CompileDef */

#endif /*not __ARM */

/*}}}*/
/*{{{  M2_CompileMod */

#ifndef __ARM

static int
#ifdef __STDC__
M2_CompileMod( FileNode * fnode )
#else
M2_CompileMod( fnode )
  FileNode * fnode;
#endif
/*
 * compile the indicated Modula-2 program file
 * returns exit code of compilation
 */
{
  NameList	args;
  List *	plist     = (List *)&args;
  int		result    = 0;
  char *	extension = Null( char );
    
    
  InitList( plist );
  
  AddNamef( plist, "%s",     m2_compiler );
  AddNames( plist, "%s",     &ModulaOpts );
  AddNamef( plist, "%s.mod", fnode->root );
  
  AddName(  plist, "-w" );
  
  if (ram_disk_present)
    {
      AddName( plist, "/ram/tmp" );
    }
  else
    {
      AddName( plist, "/helios/tmp" );
    }
    
  LastCompile = fnode;
  
  if (REDIRECT && !ASSEMBLE)
    {
      if (ObjName != NULL)
	{
	  AddName(  plist, "-s"    );
	  AddName(  plist, ObjName );
	  
	  extension = "";
	}
    }
  else if (!ASSEMBLE)
    {
      AddName(  plist, "-s"                );
      AddNamef( plist, "%s.s", fnode->root );
      
      extension = "s";
    }
  else
    {
      AddName(  plist, "-o"                );
      AddNamef( plist, "%s.o", fnode->root );
      
      extension   = "o";
      LastCompile = NULL;
    }
    
  result = ExecuteList( plist );
  
  NCompiles++;
  
  fnode->ext  = DupName( extension );
  
  return result;
  
} /* M2_CompileMod */

#endif /* not __ARM */

/*}}}*/
/*{{{  MacroAssemble */

static int
#ifdef __STDC__
MacroAssemble( FileNode * fnode )
#else
MacroAssemble( fnode )
  FileNode * fnode;
#endif
/*
 * macro assemble the indicated file
 * returns exit code of operation
 */
{
  NameList	args;
  List *	plist  = (List *)&args;
  int		result = 0;
  
  
#ifdef DEBUG
  debug( "MacroAssemble() called" );
#endif
  
  InitList( plist );
  
  AddNamef( plist, "%s", macro );
  
  if (REDIRECT && !ASSEMBLE)
    {
      if (ObjName != NULL)
	{
	  AddNamef( plist, "-o%s", ObjName );
	}
    }
  else if (!USE_PIPE)
    {
      AddNamef( plist, "-o%s.s", fnode->root );
    }
  
  AddNamef( plist, "-i%s", macdir );
  
  AddNames( plist, "%s",   &MacroOpts );
  
  AddNamef( plist, "%s%s", macdir, BASIC );
  
  AddNamef( plist, "%s.a", fnode->root );
  
  result = ExecuteList( plist );
  
  NCompiles++;
      
  fnode->ext = DupName( "s" );

  return result;
  
} /* MacroAssemble */

/*}}}*/
/*{{{  Assemble */

static int
#ifdef __STDC__
Assemble( FileNode * fnode )
#else
Assemble( fnode )
  FileNode * fnode;
#endif
/*
 * assemble the indicated file
 * returns the result of the assemble operation
 */
{
  NameList	args;
  List *	plist  = (List *)&args;
  int		result = 0;
    
    
#ifdef DEBUG
  debug( "Assemble() called" );
#endif
  
  InitList( plist );
    
  AddNamef( plist, "%s", assembler );

  if (_is_transputer_compiler)
    AddName(  plist, "-p"          ); /* only assemble */

  AddNames( plist, "%s", &AssembleOpts );
  
#ifndef __ARM
  if (vfile != NULL)
    {
      AddNamef( plist, "-m%s", vfile );
    }
#endif
  
  if (REDIRECT && !LINK)
    {
      if (ObjName != NULL)
	{
	  AddName( plist, "-o"    );
	  AddName( plist, ObjName );
	}
    }
  else
    {
      FileNode *	This;

      
      /*
       * check to see if we are about to create an object
       * file whose name matches the name of an object file
       * already created...
       */

      for (This  = (FileNode *)FirstNode( (List *)&SourceFiles );
	   This != NULL;
	   This  = (FileNode *)NextNode( (Node *)This ) )
	{
	  if (This == fnode)
	    break;
	  
	  if (streq( This->ext, "o" ) && streq( This->root, fnode->root ))
	    {
	      warning( "source files %s.%s and %s.%s produce same named object files!",
		    This->root, This->oext, fnode->root, fnode->oext );

	      return -1;
	    }
	}
      
      AddName(  plist, "-o"                );
      AddNamef( plist, "%s.o", fnode->root );
    }
  
  if (!USE_PIPE)
    {
      AddNamef( plist, "%s.s", fnode->root );
    }
  
  result = ExecuteList( plist );
  
  if (streq( fnode->oext, "s" ))
    {
      NCompiles++;
      
      LastCompile = fnode;
    }
  else if (USE_PIPE && (streq( fnode->oext, "c" ) || streq( fnode->oext, "a" )))
    {
      (void) ExecuteCommandLine();
    }
  else
    {
      RemoveFile( fnode );
    }
  
  fnode->ext = DupName( "o" );
  
  return result;
  
} /* Assemble */

/*}}}*/
/*{{{  DoCompiles */

static bool
DoCompiles(
#ifdef __STDC__
	   void
#endif
	   )
/*
 * perform all the necessary compilations
 * returns TRUE if all the compilations complete successfully, FALSE otherwise
 */
{
  FileNode *	This;
  FileNode *	next;
  bool		result = TRUE;
  
  
#ifdef DEBUG
  debug( "DoCompiles() called" );
#endif
  
  for (This = (FileNode *)FirstNode( (List *)&SourceFiles );
       (This != NULL) && result;
       This = next)
    {
      /* initialize next so Remove() works safely */
      
      next = (FileNode *)NextNode( (Node *)This );
      
      switch (*This->ext)
	{
	case 'c':
	  if (PRE_PROC_ONLY)
	    {
	      if (Pre_Process( This ) != 0)
		result = FALSE;
	      
	      break;
	    }
	  
	  /* drop through */
	  
	case 'i':
	  if (C_Compile( This ) != 0)
	    {
	      result = FALSE;
	    }
	  
	  if (!_is_norcroft_compiler &&	/* these compiler produce object code output, not assembler */
	      (NCfiles > 1 || NAfiles > 0 || NOfiles > 0))
	    {
	      if (Assemble( This ) != 0)
		result = FALSE;
	    }
	  break;
	  
#ifndef __ARM	  
	case 'f':
	  if (F77_Compile( This ) != 0)
	    result = FALSE;
	  break;
#endif
	case 'a':
	  if (MacroAssemble( This ) != 0)
	    result = FALSE;

	  if (! _is_transputer_compiler)
	    {
	      if (Assemble( This ) != 0)
		result = FALSE;
	    }
	  
	  if ((NAfiles > 1 || NCfiles > 0 || NOfiles > 0))
	    {
	      if (Assemble( This ) != 0)
		result = FALSE;
	    }
	  
	  break;
	  
	default:
	  /* Modula-2 extensions are three characters wide */
	  
#ifdef __ARM
	  result = TRUE;
#else
	  if (streq( This->ext, "mod" ))
	    {
	      if (M2_CompileMod( This ) != 0)
		result = FALSE;
	    }
	  else if (streq( This->ext, "def" ))
	    {
	      if (M2_CompileDef( This ) != 0)
		result = FALSE;
	      
	      /* delete This node from the list of source  */
	      /* files as it should not be included in the */
	      /* file list passed to the linker.	     */
	      
	      Remove( (Node *)This );
	    }
#endif /* __ARM */	  
	  break;
	}
    }
    
  return result;
    
} /* DoCompiles */

/*}}}*/
/*{{{  DoAssembles */

static bool
DoAssembles(
#ifdef __STDC__
	    void
#endif
	    )
/*
 * perform all necessary assembles
 * returns TRUE if all the assembles complete successfuly, FALSE otherwise
 */
{
  FileNode *	This;
  bool		result = TRUE;
    
    
#ifdef DEBUG
  debug( "DoAssembles() called" );
#endif
  
  for (This = (FileNode *)FirstNode( (List *)&SourceFiles );
       This;
       This = (FileNode *)NextNode( (Node *)This ))
    {
      if (streq( This->ext, "s" ))
	{
	  if (Assemble( This ) != 0)
	    {
	      result = FALSE;
	      
	      break;
	    }
	}
    }
    
  return result;
  
} /* DoAssembles */

/*}}}*/
/*{{{  DoLink */

static bool
DoLink(
#ifdef __STDC__
       void
#endif
       )
/*
 * perform a final link if necessary
 * returns TRUE if this succeeds, FALSE otherwise
 */
{
  NameList	args;
  List *	plist  = (List *)&args;
  int		result = 0;
    
    
#ifdef DEBUG
  debug( "DoLink() called" );
#endif
  
  InitList( plist );

  if (JOIN || APPEND)
    {
      if (_is_transputer_compiler)
	{
	  if (APPEND)
	    {
	      error( "Cannot append to transputer scanned libraries" );

	      return FALSE;	      
	    }
	  
	  AddNamef( plist, "%s", linker    );
	  AddNames( plist, "%s", &LinkOpts );      
	  AddName(  plist, "-p" );
	
	  if (!USE_PIPE || NCfiles == 0) 
	    {
	      AddFiles( plist, "%s.%s", &SourceFiles );
	    }
      
	  if (vfile != NULL)
	    {
	      AddNamef( plist, "-m%s", vfile );
	    }
      
	  if (ObjName != NULL)
	    {
	      AddName(  plist, "-o" 	     );
	      AddNamef( plist, "%s", ObjName );
	    }
	}
      else
	{
	  FILE *	output = Null( FILE );
	  FileNode *	This;
	  
	  
	  /*
	   * we want to concatenate together the files in 'SourceFiles'
	   * into a file called 'ObjName'.  Unfortunately we cannot call
	   * 'cat' as a) it does not have a -o option to redirect its
	   * output and b) it does not understand '-' to read from stdin
	   */

	  result = 0;

	  if (EXECUTE)
	    {
	      /* open output file */
	  
	      if (ObjName == Null( char ))
		{
		  output = stdout;
		}
	      else
		{
		  output = fopen( ObjName, APPEND ? "ab" : "wb" );
	      
		  if (output == Null( FILE ))
		    {
		      warning( "Failed to open output file '%s' for  concatenation", ObjName );

		      return FALSE;	      
		    }
		}
	    }
	  
	  /* tell the world we are about to start */
	  
	  if (VERBOSE || VERIFY)
	    printf( "'cat' " );

	  /* read each of the source files */
	  
	  for (This  = (FileNode *)FirstNode( (List *)&SourceFiles );
	       This != NULL;
	       This  = (FileNode *)NextNode( (Node *)This ) )
	    {
	      FILE *	input;
	      char	name[ ARGBUFSIZE ];


	      /* build the source file's name */
	      
	      sprintf( name, "%s.%s", This->root, This->ext );

	      /* tell the world */
	      
	      if (VERBOSE || VERIFY)
		printf( "%s ", name );

	      if (EXECUTE)
		{
		  /* open the source file */
		  
		  input = fopen( name, "rb" );

		  /* check it */
		  
		  if (input == Null( FILE ))
		    {
		      warning( "Failed to open input file '%s' for concatenation", name );

		      result = 1;
		    }
		  else
		    {
		      /* why bother being clever - the C library will buffer for us anyway */

		      /* copy the source file to the output file */
		      
		      for (;;)
			{
			  int	c;

			  
			  c = fgetc( input );

			  if (feof( input ))
			    break;
			  
			  fputc( c, output );		      
			}

		      /* close the source file */
		      
		      fclose( input );

		      input = Null( FILE );		  /* be paranoid */
		    }
		}
	    }

	  if (VERBOSE || VERIFY)
	    {
	      if (ObjName != NULL)
		printf( "> %s", ObjName );

	      putchar( '\n' );	      
	    }
	  
	  /* close output file */

	  if (EXECUTE)
	    {
	      fclose( output );

	      output = Null( FILE );
	    }	  
	}
    }
  else
    {
      AddNamef( plist, "%s", linker    );
      AddNames( plist, "%s", &LinkOpts );
  
      if (vfile != NULL)
	{
	  AddNamef( plist, "-m%s", vfile );
	}
      
      if (!NOLIBS)
	{
	  if (NFfiles > 0)
	    {
	      if (link_fstart[ 0 ] == '/')
		AddName( plist, link_fstart );
	      else
		AddNamef( plist, "%s%s", c_libdir, link_fstart );
	    }
	  else if (NMfiles > 0)
	    {
	      if (link_m2start[ 0 ] == '/')
		AddName( plist, link_m2start );
	      else
		AddNamef( plist, "%s%s", c_libdir, link_m2start );
	    }
	  else
	    {
	      if (link_cstart[ 0 ] == '/')
		AddName( plist, link_cstart );
	      else
		AddNamef( plist, "%s%s", c_libdir, link_cstart );
	      
	      if (link_clib[ 0 ] == '/')
		{
		  AddLibraryName( TRUE, "-l%s", link_clib );
		}
	      else if (!AddLibrary( c_libdir, link_clib ))
		{
		  warning( "Could not find C library '%s%s'", c_libdir, link_clib );

		  return FALSE;
		}
	    }
	}
	
      if (NSources == 1 && (NCfiles != 0 || NAfiles != 0) && USE_PIPE && !_is_transputer_compiler)
	{
	  AddName( plist, "-" );
	}
      else
	{
	  AddFiles( plist, "%s.%s", &SourceFiles );
	}

      AddNames( plist, "%s", &ScanLibraryNames );

      AddNames( plist, "%s", &ResLibraryNames );
      
      if (state != objed_suppressed)
	{
	  AddNamef( plist, "-n%s", ObjectName );
	  AddNamef( plist, "-s%d", StackSize  );
	  AddNamef( plist, "-h%d", HeapSize   );
	}
      
      if (ObjName != NULL)
	{		
	  AddName(  plist, "-o"		 );
	  AddNamef( plist, "%s", ObjName );
	}
    }

  if (!JOIN && !APPEND || _is_transputer_compiler)
    result = ExecuteList( plist );
  
  /*
   * When one source is compiled and linked, the intermediate file is removed 
   */
  
  if (!JOIN              &&
      !USE_PIPE          &&
      NCompiles   == 1   &&
      LastCompile != NULL )
    {
      RemoveFile( LastCompile );
    }
    
  return (result == 0);
    
} /* DoLink */

/*}}}*/

/*}}}*/
/*{{{  Command Line Construction */

/*{{{  ParseWarnings */

static void
#ifdef __STDC__
ParseWarnings( char * s )
#else
ParseWarnings( s )
  char * s;
#endif
/*
 * parse the warning / error suppression options
 */
{
  for (; *s; s++)
    {
      switch (*s)
	{
	case 'A':
	  if (compiler != UNKNOWN_COMPILER)
	    {
	      Warns = WARN_a | WARN_d | WARN_f | WARN_i | WARN_n | WARN_o | WARN_p | WARN_u | WARN_v ;
	      Errs  = 0;
	    }
	  else
	    {
	      Warns = WARN_a | WARN_d | WARN_f | WARN_s | WARN_v;
	      Errs  = ERR_c  | ERR_p  | ERR_z;
	    }
	  break;
	  
	case 'a':
	  Warns |= WARN_a;
	  break;
	  
	case 'c':
	  Errs  |= ERR_c;
	  break;
	  
	case 'd':
	  Warns |= WARN_d;
	  break;
	  
 	case 'f':
	  Warns |= WARN_f;
	  break;

	case 'i':
	  if (_is_norcroft_compiler)
	    Warns |= WARN_i;
	  break;

	case 'n':
	  if (_is_norcroft_compiler)
	    Warns |= WARN_n;
	  break;

	case 'o':
	  if (_is_norcroft_compiler)
	    Warns |= WARN_o;
	  break;

	case 'p':
	  if (compiler != UNKNOWN_COMPILER)
	    {
	      Warns |= WARN_p;
	    }
	  else
	    {
	      Errs  |= ERR_p;
	    }
	  break;
	  
	case 's':
	  Warns |= WARN_s;
	  break;
	  
	case 'u':
	  if (_is_norcroft_compiler)
	    Warns |= WARN_u;
	  break;

	case 'v':
	  Warns |= WARN_v;
	  break;
	  
	case 'z':
	  Errs  |= ERR_z;
	  break;
	  
	default:
	  warning( "Unknown warning suppression option -w%c, ignored", *s );
	}
    }

  return;
  
} /* ParseWarnings */

/*}}}*/
/*{{{  ParseFeatures */

static void
#ifdef __STDC__
ParseFeatures( char * s )
#else
ParseFeatures( s )
  char * s;
#endif
/*
 * parse the features options
 */
{
  for (; *s; s++)
    {
      switch (*s)
	{
	case 'A':
	  switch (compiler)
	    {
	    default:
	    case C40_COMPILER:
	    case TRANSPUTER_COMPILER:
	      Feats |= FEAT_s | FEAT_g;
	      break;

	    case M68K_COMPILER:
	    case ARM_COMPILER:
	    case ARM_LTD_COMPILER:
	    case I860_COMPILER:
	      break;
	    }
	  break;
	  
	case 'b':
	  if (compiler == C40_COMPILER)
	    Feats |= FEAT_b;
	  break;
	  
	case 'd':
	  if (compiler == C40_COMPILER)
	    Feats |= FEAT_d;
	  break;
	  
	case 'f':
	  if (_is_transputer_compiler)
	    Feats |= FEAT_f;
	  break;
	  
	case 'g':
	  Feats |= FEAT_g;
	  break;
	  
	case 'h':
	  Feats |= FEAT_h;
	  break;
	  
	case 'l':
	  Feats |= FEAT_l;
	  break;
	  
	case 'm':
	  Feats |= FEAT_m;
	  break;
	  
	case 'n':
	  Feats |= FEAT_n;
	  break;
	  
	case 'p':
	  if (compiler == C40_COMPILER)
	    Feats |= FEAT_p;
	  break;
	  
	case 's':
	  Feats |= FEAT_s;
	  break;
	  
	case 'u':
	  Feats |= FEAT_u;
	  break;
	  
	case 'v':
	  Feats |= FEAT_v;
	  break;
	  
	default:
	  warning( "Unknown feature -F%c, ignored", *s );
	}
    }

  return;
  
} /* ParseFeatures */

/*}}}*/
/*{{{  AddWarningsAndFeatures */

static void
AddWarningsAndFeatures(
#ifdef __STDC__
		       void
#endif
		       )
/*
 * adds the parsed warning / error suppressions and the features
 * to the argument list
 */
{
  static char buf[ ARGBUFSIZE ];
  
  
  if (Warns != 0)
    {
      strcpy( buf, "-w" );
      
      if ((Warns & WARN_a) != 0) strcat( buf, "a" );
      if ((Warns & WARN_d) != 0) strcat( buf, "d" );
      if ((Warns & WARN_f) != 0) strcat( buf, "f" );
      if ((Warns & WARN_n) != 0) strcat( buf, "n" );
      if ((Warns & WARN_p) != 0) strcat( buf, "p" );
      if ((Warns & WARN_s) != 0) strcat( buf, "s" );
      if ((Warns & WARN_v) != 0) strcat( buf, "v" );

      AddName( &CompileOpts, buf );
    }
  
  if (Errs != 0)
    {
      if (_is_norcroft_compiler)
	strcpy( buf, "-W" );
      else
	strcpy( buf, "-e" );

      if ((Errs & ERR_c) != 0) strcat( buf, "c" );
      if ((Errs & ERR_p) != 0) strcat( buf, "p" );
      if ((Errs & ERR_z) != 0) strcat( buf, "z" );
      
      AddName( &CompileOpts, buf );
    }
  
  if (Feats != 0)
    {
      strcpy( buf, "-f" );
      
      if ((Feats & FEAT_d) != 0) strcat( buf, "d" );
      if ((Feats & FEAT_h) != 0) strcat( buf, "h" );
      if ((Feats & FEAT_m) != 0) strcat( buf, "m" );
      if ((Feats & FEAT_v) != 0) strcat( buf, "v" );

      if (VERBOSE && _is_norcroft_compiler)
	strcat( buf, "s" );
	
      if ((Warns & WARN_o) != 0) strcat( buf, "o" );
      if ((Warns & WARN_u) != 0) strcat( buf, "a" );
      if ((Warns & WARN_i) != 0) strcat( buf, "p" );      
      
      if ((Feats & FEAT_f) != 0)
	{
	  if (compiler == UNKNOWN_COMPILER)
	    strcat( buf, "f" );
	  else if (compiler == TRANSPUTER_COMPILER)
	    AddName( &CompileOpts, "-pf0" );
	  else
	    warning( "f feature not supported for non-transputer compilers" );
	}
      
      if ((Feats & FEAT_b) != 0)
	{
	  AddName( &CompileOpts, "-Zpm1" );
	}
      
      if ((Feats & FEAT_g) != 0)
	{
	  if (compiler == UNKNOWN_COMPILER)
	    AddName( &CompileOpts, "-pg0" );	/* old transputer compiler uses a pragma */
	  else
	    strcat( buf, "f" );			/* Norcroft compilers use a feature */
	}
      
      if ((Feats & FEAT_n) != 0)
	{
	  if (compiler != UNKNOWN_COMPILER)
	    AddName( &CompileOpts, "-Zpf0" );
	  else
	    AddName( &CompileOpts, "-pf0" );
	}	
      
      if ((Feats & FEAT_p) != 0)
	{
	  AddName( &CompileOpts, "-Zpn1" );
	}

      if ((Feats & FEAT_l) != 0 && compiler == C40_COMPILER)
	{
	  AddName( &CompileOpts, "-Zpl1" );
	}

      if ((Feats & FEAT_s) != 0)
	{
	  if (compiler != UNKNOWN_COMPILER)
	    AddName( &CompileOpts, "-Zps1" );
	  else
	    AddName( &CompileOpts, "-ps1" );
	}

      if ((Feats & FEAT_u) != 0)
	{
	  AddName( &LinkOpts, "-wru" );
	}
      
      AddName( &CompileOpts, buf );
    }

  return;
  
} /* AddWarningsAndFeatures */

/*}}}*/
/*{{{  AddDefines */

static void
AddDefines(
#ifdef __STDC__
	   void
#endif
	   )
/*
 * add the define list to the argument list
 */
{
  if (_is_norcroft_compiler)
    AddNames( &CompileOpts, "-D%s", &Defines );
  else
    AddNames( &CompileOpts, "-d%s", &Defines );

  return;
  
} /* AddDefines */

/*}}}*/
/*{{{  AddInclude */

static void
#ifdef __STDC__
AddInclude( char * path )
#else
AddInclude( path )
  char * path;
#endif
/*
 * add the indicated path(s) to the include argument list
 */
{
  static char	buffer[ ARGBUFSIZE ];
  NameNode *	This;
  char *	end;
  char *	start = path;
  
  
  while (path != NULL)
    {
      end = strchr( start, ',' );
      
      if (end != NULL)
	{
	  *end = '\0';
	}
      else
	{
	  path = NULL;
	}
      
      strcpy( buffer, start );
      
      start = end + 1;
      
      /*
       * add trailing / if none present and name is not blank
       */
      
      if (buffer[ strlen( buffer ) - 1 ] != '/' && strlen( buffer ) > 0)
	{
	  strcat( buffer, "/" );
	}
      
      /*
       * check for and ignore duplicates, for faster compilation
       */
      
      for ( This = (NameNode *)FirstNode( (List *)&IncludeDirs );
	   This;
	   This = (NameNode *)NextNode( (Node *)This) )
	{
	  if (streq( This->name, buffer ))
	    return;
	}
      
      AddName( &IncludeDirs, buffer );
    }

  return;
  
} /* AddInclude */

/*}}}*/
/*{{{  AddSymbol */

static void
#ifdef __STDC__
AddSymbol( char * path )
#else
AddSymbol( path )
  char * path;
#endif
/*
 * add the indicated path(s) to the symbol search argument list (M2)
 */
{
  static char	buffer[ ARGBUFSIZE ];
  NameNode *	This;
  char *	end;
  char *	start = path;
  
  
  while (path != NULL)
    {
      end = strchr( start, ',' );
      
      if (end != NULL)
	{
	  *end = '\0';
	}
      else
	{
	  path = NULL;
	}
      
      strcpy( buffer, start );
      
      start = end + 1;
      
      /*
       * add trailing / if none present and name is not blank
       */
      
      if (buffer[ strlen( buffer ) - 1 ] != '/' && strlen( buffer ) > 0)
	{
	  strcat( buffer, "/" );
	}
      
      /*
       * check for and ignore duplicates, for faster compilation
       */
      
      for ( This = (NameNode *)FirstNode( (List *)&SymbolDirs );
	   This;
	   This = (NameNode *)NextNode( (Node *)This) )
	{
	  if (streq( This->name, buffer ))
	    return;
	}
      
      AddName( &SymbolDirs, buffer );
    }

  return;
  
} /* AddSymbol */

/*}}}*/
/*{{{  AddLibraryPath */

static void
#ifdef __STDC__
AddLibraryPath( char * path )
#else
AddLibraryPath( path )
  char * path;
#endif
/*
 * add the indicated path(s) to the library path list
 */
{
  static char	buffer[ ARGBUFSIZE ];
  NameNode *	This;
  char *	end;
  char *	start = path;
  

  while (path != NULL)
    {
      end = strchr( start, ',' );
      
      if (end != NULL)
	{
	  *end = '\0';
	}
      else
	{
	  path = NULL;
	}
      
      strcpy( buffer, start );
      
      start = end + 1;
      
      /*
       * add trailing / if none present and name is not blank
       */
      
      if (buffer[ strlen( buffer ) - 1 ] != '/' && strlen( buffer ) > 0)
	{
	  strcat( buffer, "/" );
	}
      
      /*
       * check for and ignore duplicates, for faster compilation
       */
      
      for (This = (NameNode *)FirstNode( (List *)&LibraryDirs );
	   This;
	   This = (NameNode *)NextNode( (Node *)This) )
	{
	  if (streq( This->name, buffer ))
	    {
	      return;
	    }
	}

      AddNameToFront( &LibraryDirs, buffer );
    }

  return;
  
} /* AddLibraryPath */

/*}}}*/
/*{{{  RemoveLibraryPath */

static void
#ifdef __STDC__
RemoveLibraryPath( char * pPath )
#else
RemoveLibraryPath( pPath )
  char * pPath;
#endif
/*
 * Remove the indicated path(s) from the library path list
 */
{
  static char	aBuffer[ ARGBUFSIZE ];
  NameNode *	pThis;
  char *	pEnd;
  char *	pStart = pPath;

  
  while (pPath != NULL)
    {
      pEnd = strchr( pStart, ',' );
      
      if (pEnd != NULL)
	{
	  *pEnd = '\0';
	}
      else
	{
	  pPath = NULL;
	}
      
      strcpy( aBuffer, pStart );
      
      pStart = pEnd + 1;
      
      /* add trailing / if none present and name is not blank */
      
      if (aBuffer[ strlen( aBuffer ) - 1 ] != '/' && strlen( aBuffer ) > 0)
	{
	  strcat( aBuffer, "/" );
	}

      /* find name in list and remove */
      
      for (pThis  = (NameNode *)FirstNode( (List *)&LibraryDirs );
	   pThis != NULL;
	   pThis  = (NameNode *)NextNode( (Node *)pThis) )
	{
	  if (streq( pThis->name, aBuffer ))
	    {
	      Remove( (Node *) pThis );
	      break;
	    }
	}

      if (pThis == NULL)
	warning( "Unable to remove library path '%s'", aBuffer );
    }

  return;
  
} /* RemoveLibraryPath */

/*}}}*/
/*{{{  AddIncludeOpts */

static void
AddIncludeOpts(
#ifdef __STDC__
	       void
#endif
	       )
/*
 * add the include path(s) to the C compiler's search list
 */
{
  static char	buffer[ ARGBUFSIZE ];
  NameNode *	This = (NameNode *)FirstNode( (List *)&IncludeDirs );
  
  
  if (This == NULL)
    return;
  
  strcpy( buffer, "-i" );
  
  for ( ;
       This;
       This = (NameNode *)NextNode( (Node *)This) )
    {
      strcat( buffer, This->name );
      strcat( buffer, "," );
    }
  
  buffer[ strlen( buffer ) - 1 ] = '\0'; 	/* remove trailing ',' */
  
  AddName( &CompileOpts, buffer ); 		/* -i version */
  
  /*
   * start -j option 3 places futher on so that we miss the "./,"
   * at the start of the -i option
   */
  
  buffer[ 3 ] = '-';
  buffer[ 4 ] = 'j';
  
  AddName( &CompileOpts, buffer + 3 ); 		/* -j version */

  return;
  
} /* AddIncludeOpts */

/*}}}*/
/*{{{  AddSymbolOpts */

static void
AddSymbolOpts(
#ifdef __STDC__
	      void
#endif
	      )
/*
 * compile a symbol list for passing to the modula compiler
 */
{
  static char	buffer[ ARGBUFSIZE ];
  NameNode *	This = (NameNode *)FirstNode( (List *)&SymbolDirs );
  
  
  if (This == NULL)
    return;
  
  strcpy( buffer, "-i" );
  
  for (;
       This;
       This = (NameNode *)NextNode( (Node *)This ))
    {
      strcat( buffer, This->name );
      strcat( buffer, "," );
    }
  
  buffer[ strlen( buffer ) - 1 ] = '\0'; 	/* remove trailing ',' */
  
  AddName( &ModulaOpts, buffer ); 		/* -i version */

  return;
  
} /* AddSymbolOpts */

/*}}}*/

/*}}}*/
/*{{{  Utilities for main */

/*{{{  CountSources */

static void
CountSources(
#ifdef __STDC__
	     void
#endif
	     )
/*
 * returns number of source files to be compiled
 */
{
  FileNode *	This;
  NameNode *	pname;
  
  
  NOfiles = NAfiles = NCfiles = NFfiles = NSfiles = NDfiles = NMfiles = NSources = 0;
  
  for (This = (FileNode *)FirstNode( (List *)&SourceFiles );
       This;
       This = (FileNode *)NextNode( (Node *)This ))
    {
      NSources++;
      
      switch (*This->ext)
	{
	case 'i':		/* pre-processed C source */
	case 'c':
	  NCfiles++;
	  break;
	  
	case 'f':
	  NFfiles++;
	  break;
	  
	case 's':
	  NSfiles++;
	  break;
	  
	case 'a':
	  break;
	  
	case 'o':
	   /* 
	    * catch the case where someone includes /helios/lib/c0.o
	    * in their object list
	    */
	  
	  if (strneq( This->root, link_cstart, strlen( link_cstart ) - 2 ))
	    link_cstart = NULL;
	  else
	    NOfiles++;

	  break;
	  
	case 'p':
	  break;
	  
	default:
	  if (streq( This->ext, "mod" ))
	    NMfiles++;
	  else if (streq( This->ext, "def" ))
	    NDfiles++;
	  else if (streq( This->ext, "lib" ))
	    break;
	  else
	    fail( "unknown type of source file => %s.%s", This->root, This->ext );
	  break;
	}
    }
  
  for (pname  = (NameNode *)FirstNode( &ScanLibraryNames );
       pname != NULL;
       pname  = (NameNode *)NextNode( (Node *)pname ) )
    {
      /*
       * scanned libraries placed here without a -l at
       * the start are intended to be included in their
       * entirity, and hence count as source files.
       */
      
      if (*pname->name != '-')
	++NSources;
    }

  return;
  
} /* CountSources */

/*}}}*/
/*{{{  usage */

static void
usage(
#ifdef __STDC__
      void
#endif
      )
  /*
   * describe the options to this utility
   */
{
  int	err = dup( 2 );
  int	pid;
  int	pfds[ 2 ];
  
  
  pipe( pfds );
  
  pid = vfork();
  
  if (pid == 0)
    {
      dup2( pfds[ 0 ], 0 );
      
      close( pfds[ 0 ] );
      close( pfds[ 1 ] );
      
      execlp( "more", "more", 0 );
      
      pid = -1;
      
      _exit( 0 );
    }
  
  if (pid == -1)
    close( err );
  
  else dup2( pfds[ 1 ], 2 );
  
  close( pfds[ 0 ] );
  close( pfds[ 1 ] );
  
  error( "Helios compiler driver, version %d.%d (RCS $Revision: 1.2 $)", VERSION, REVISION );
  error( "Usage: %s [options] files", ProgName );

{ static char *strings[] = {
"*.c\t\tC source file.\n\
*.i\t\tPre-processed C source file.\n",
#ifndef __ARM
"*.f\t\tFORTRAN source file.",
#endif
"*.s\t\tAssembler source file.\n\
*.a\t\tMacro Assembler source file.",
#ifndef __ARM
"*.def\t\tModula-2 definition source file.\n\
*.mod\t\tModula-2 program source file.",
#endif
"*.o\t\tAssembled file - ready for linking.\n\
-a<text>\tPass <text> as an option to assembler.\n\
-b\t\tDon't link with standard library (helios.lib).\n\
-c\t\tCompile/Assemble only, don't link.\n\
-c40\t\tCompile code for a C40.\n\
-d<name>\tSpecify output file name for library .def compilations.",
#ifndef __ARM
"-e[6|7]\t\tEnforce FORTRAN standard.",
#endif
"-f <name>\tSpecify file name for the assembler's virtual memory system.\n\
-g\t\tCompile for debugging.\n\
-h<val>\t\tSpecify heap size of program.\n\
-j\t\tCreate scanned library.\n\
-l<name>\tLink with standard library <name>\n\
-m\t\tCompile code for shared libraries.\n\
-n\t\tDon't actually execute commands (implies -v).",
"-n<string>\tSpecify object name of program.\n\
-o <name>\tSpecify output name (default *.o or \"a.out\") (\"-\" is stdout)\n\
-p\t\tCompile code for profiling (Transputer only).\n\
-q<fgilpz>\tEnable internal compiler debugging features:\n\
\t\t 'f' Function names.\n\
\t\t 'g' Code generation.\n\
\t\t 'i' Include files.\n\
\t\t 'l' Lexical analysis.\n\
\t\t 'p' Pre-processor.\n\
\t\t 'z' Fatal error handling.\n\
-r\t\tCompile code for device drivers.",
"-s<val>\t\tSpecify stack size of program.\n\
-t\t\tCompile code for (back) tracing.\n\
-tran\t\tCompile code for Transputers.\n\
-u\t\tPrevents removal of intermediary files.\n\
-v\t\tDisplay command(s) being executed.\n\
-w[Aacdfpsvz]\tSuppress warnings and error messages from the C compiler when:",
"\t\t 'a' '=' occurs in a condition context,\n\
\t\t 'c' ANSI disallowed casts are used,\n\
\t\t 'd' (some)deprecated features are used,\n\
\t\t 'f' functions are implicity declared as 'extern int()',\n\
\t\t 'i' pointers are explicitly cast to integers,\n\
\t\t 'n' implicit casting between types,\n\
\t\t 'o' old K&R style function headers are used,",
"\t\t 'p' junk occurs after #else and #endif,\n\
\t\t 's' shorts are used for the first time, (Transputer)\n\
\t\t 'u' unused variables,\n\
\t\t 'v' void functions are written without 'void',\n\
\t\t 'z' zero sized arrays occur,\n\
\t\t 'A' all of the above.\n\
-x<proctype>\tCompile code for <proctype> (arm/ARM/c40/m68k/tran).\n\
-y<text>\tPass <text> as an option to the Macro Assembler.",
"-z<text>\tPass <text> as an option to the C compiler.\n\
-A<text>\tPass <text> as an option to the linker.\n\
-B\t\tDo not link with any libraries.  Do not perform objed.\n\
-C\t\tPerform array bound (F77,M2) or memory access checking (C)\n\
-C40\t\tCompile code for C40s.\n\
-D <name>\t#define <name>",
"-D <name>=<val> #define <name> to be <val> (default <val> is \"1\").\n\
-E\t\tPre-process only - do not compile\n\
-F[Afghmnpsuv]  Enable C  compiler features:\n\
\t\t 'b' enables back trace support (C40)\n\
\t\t 'd' disables new stub generation (C40)\n\
\t\t 'f' disables the vector stack (Transputer)\n\
\t\t 'g' removes procedure names from the code\n\
\t\t 'h' warns of discrepencies in function declarations\n\
\t\t 'l' small model (C40)",
"\t\t 'm' warns of unused macros\n\
\t\t 'n' do not put arrays or structs on vector stack (Transputer)\n\
\t\t 'p' disables peepholing by the code generator (C40)\n\
\t\t 's' turns off stack checking\n\
\t\t 'u' warns of unused functions and variables during linking\n\
\t\t 'v' warns of unused global functions and variables in a file\n\
\t\t 'A' turns on selected useful features\n\
-I<dir>\t\tSpecify a directory to be searched for #include or Modula-2\n\
\t\tsymbol files.\n\
-J\t\tAppend to scanned library.",
#ifndef __ARM
"-K<text>\tPass <text> as an option to Modula-2 compiler.",
#endif
"-L<dir>\t\tSpecifies directories to be searched for libraries.",
#ifndef __ARM
"-M<name>\tProduce map file <name> (F77).",
"-M68K\t\tCompile code for M68K processors.",
#endif
"-O\t\tOptimise code, perform full link.\n\
-P<text>\tPass <text> as an option to the pre-processor.",
#ifndef __ARM
"-R\t\tForce use of RAM disk for temporary FORTRAN and Modula-2 files,\n\
\t\teven if RAM disk is not loaded",
#endif
"-S\t\tProduce textual assembler output from *.c in *.s, don't link.",
#ifndef __ARM
"-T[4|5|8|9]\tSpecify Transputer type.(5 => T425)",
#endif
"-V\t\tPass on verbose flag to executed commands.\n\
-U <name>\tRemoves any initial definition of <name>.",
#ifndef __ARM
"-W<val>\t\tSpecify warning level (F77).",
#endif
"-Wc,arg[,args]  Pass on arguments to specific parts of the compilation.\n\
\t\twhere c is one of :-\n\
\t\t 'p' C pre-processor (cf -P),\n\
\t\t '0' C compiler (cf -z),\n\
\t\t '2' C compiler (optimising section),\n\
\t\t 'a' assembler (cf -a),\n\
\t\t 'l' linker (cf -A),\n\
\t\t 'M' Macro Assembler (cf -y),",
#ifndef __ARM
"\t\t 'm' Modula-2 compiler (cf -M).\n\
-X<val>\t\tSpecify cross reference width (F77).\n\
-Z<text>\tPass <text> as an option to FORTRAN compiler.",
#endif
"-help\t\tThis message.",
"",
"The driver uses the following environment variables if present:-\n\
SHELL\t\tPathname of the command line interpreter.\n\
C_COMPILER\tPathname of the C compiler.\n\
PRE_PROCESSOR\tPathname of the C pre-processor.",
#ifndef __ARM
"F77_PASS1\tPathname of the first  pass of the FORTRAN compiler.\n\
F77_PASS2\tPathname of the second pass of the FORTRAN compiler.\n\
M2_COMPILER\tPathname of the Modula-2 compiler.",
#endif
"ASSEMBLER\tPathname of the assembler.\n\
LINKER\t\tPathname of the linker.\n\
REMOVER\t\tPathname of the file removal program.\n\
MACRO\t\tPathname of the assembler macro pre-processor.\n\
MACDIR\t\tPathname of the standard AMPP macro directory.\n\
C_LIBDIR\tPathname of the library directory.\n\
C_INCDIR\tPathname of the standard C header directory.",
#ifndef __ARM
"TMPDIR\t\tPathname of the directory for holding temporary FORTRAN files",
#endif
"LINK_HELIOS\tName\t of the standard Helios link library.\n\
LINK_CSTART\tName\t of the standard C startup file.\n\
LINK_CLIB\tName\t of the standard C link library.",
#ifndef __ARM
"LINK_FSTART\tName\t of the standard FORTRAN startup file.\n\
LINK_M2START\tName\t of the standard Modula-2 startup file.",
#endif
"MACHINENAME\tName\t of the destination hardware, passed on by -D.\n\
OBJNAME\t\tDefault name of output file.\n\
C_NONSTANDARD\tEnables non-standard features.\n\
\t\t (uses pipes to connect processes and\n\
\t\t does not use 'a.out' unless really necessary)",
"NEW_NCC\t\tDescribes the type of Norcroft C compiler being used, could be:\n\
\t\t ARM\t  For the ARM Ltd C compiler\n\
\t\t arm\t  For the Helios ARM C compiler\n\
\t\t C40\t  For the Helios TMS320C40 C compiler\n\
\t\t I860\t  For the Helios i860 C compiler\n\
\t\t M68K\t  For the Helios m68K C compiler\n\
\t\t TRAN\t  For the new Helios Transputer C compiler\n\
\t\t OLD_TRAN For the old Helios Transputer C compiler (default)",
NULL
};
  int i;
  for (i = 0; strings[i] != NULL; i++)
	error(strings[i]);
}
  
  if (pid >= 0) 
    {
      int 	stat;
      int 	p;
      
      
      close( 2 );
      
      p = wait( &stat );
      
      dup2( err, 2 );
      
      close( err );
    }
  
  return;
  
} /* usage */

/*}}}*/

/*}}}*/
/*{{{  Main */

#define	get_var( var, val, default ) 	\
  if ((var = getenv( val )) == NULL) 	\
    {					\
      var = default;			\
    }
  

int
#ifdef __STDC__
main(
     int	argc,
     char **	argv )
#else
main( argc, argv )
  int		argc;
  char **	argv;
#endif
{
#ifndef UNIX
  Object *	o;
#endif
  int		i 	 = 0x01020304;
  bool		succeeded = TRUE;
  char *	ld;
  char *	cc;
  char *	cld;
  
    
  if ((ProgName = strrchr( argv[ 0 ], '/')) != NULL)
    {
      ProgName++;
    }
  else
    {
      ProgName  = argv[ 0 ];
    }

  get_var( shell,         "SHELL",         SHELL         );
  get_var( pre_processor, "PRE_PROCESSOR", PRE_PROCESSOR );
  get_var( f77_pass1,     "F77_PASS1",     F77_PASS1     );
  get_var( f77_pass2,     "F77_PASS2",     F77_PASS2     );
  get_var( m2_compiler,   "M2_COMPILER",   M2_COMPILER   );
  get_var( assembler,     "ASSEMBLER",     ASSEMBLER     );
  get_var( remover,       "REMOVER",       REMOVER       );
  get_var( macro,         "MACRO",         MACRO         );
  get_var( macdir,        "MACDIR",        MACDIR        );
  get_var( c_incdir,      "C_INCDIR",      C_INCDIR      );
  get_var( symdir,        "SYMDIR",        SYMDIR        );
  get_var( link_helios,   "LINK_HELIOS",   LINK_HELIOS   );
  get_var( link_cstart,   "LINK_CSTART",   LINK_CSTART   );
  get_var( link_clib,     "LINK_CLIB",     LINK_CLIB     );
  get_var( link_fstart,   "LINK_FSTART",   LINK_FSTART   );
  get_var( link_m2start,  "LINK_M2START",  LINK_M2START  );
  get_var( ObjectName,    "OBJNAME",       OBJNAME       );
  get_var( machinename,   "MACHINENAME",   MACHINENAME   );
      
  if (getenv( "C_NONSTANDARD" ))
    {
      use_non_standard_features = true;
    }

#if   defined __ARM
  
  compiler = ARM_COMPILER;

#elif defined __C40

  compiler = C40_COMPILER;
  
#elif defined __I860

  compiler = I860_COMPILER;
  
#elif defined __M68K

  compiler = M68K_COMPILER;
  
#endif
  
  if (getenv( "NEW_NCC" ))
    {
      char *	ptr = getenv( "NEW_NCC" );
      char *	mc  = getenv( "MACHINENAME" );
      
      
      compiler = TRANSPUTER_COMPILER;
      ld       = TRANSPUTER_LINKER;
      cc       = TRANSPUTER_CC;
      cld      = TRANSPUTER_C_LIBDIR;
      
      if (streq( ptr, "C40" ))
	{
	  compiler = C40_COMPILER;
	  ld	   = C40_LINKER;
	  cc	   = C40_CC;
	  cld      = C40_C_LIBDIR;
	  
	  if (mc == NULL)
	    machinename = C40_MACHINENAME;
	}
      else if (streq( ptr, "ARM" ))
	{
	  compiler = ARM_LTD_COMPILER;
	  ld       = ARM_LINKER;
	  cc	   = ARM_LTD_CC;
	  
	  if (mc == NULL)
	    machinename = ARM_MACHINENAME;
	}
      else if (streq( ptr, "arm" ))
	{
	  compiler = ARM_COMPILER;
	  ld       = ARM_LINKER;
	  cc	   = ARM_CC;
	  
	  if (mc == NULL)
	    machinename = ARM_MACHINENAME;
	}
      else if (streq( ptr, "I860" ))
	{
	  compiler = I860_COMPILER;
	  ld	   = I860_LINKER;
	  cc	   = I860_CC;
      
	  if (mc == NULL)
	    machinename = I860_MACHINENAME;
	}
      else if (streq( ptr, "M68K" ))
	{
	  compiler = M68K_COMPILER;
	  ld       = M68K_LINKER;
	  cc	   = M68K_CC;
	  
	  if (mc == NULL)
	    machinename = M68K_MACHINENAME;
	}
      else if (streq( ptr, "OLD_TRAN" ))
	{
	  compiler = UNKNOWN_COMPILER;
	  
	  if (mc == NULL)
	    machinename = TRANSPUTER_MACHINENAME;
	}
      else
	{
	  if (!streq( ptr, "TRANSPUTER" ) && !streq( ptr, "TRAN" ))
	    warning( "unknown NEW_NCC environment option '%s'", ptr );
	  
	  if (mc == NULL)
	    machinename = TRANSPUTER_MACHINENAME;
	}
    }
  else
    {
      ld  = LINKER;
      cc  = C_COMPILER;
      cld = C_LIBDIR;
    }  

  get_var( linker,     "LINKER",     ld  );
  get_var( c_compiler, "C_COMPILER", cc  );
  get_var( c_libdir,   "C_LIBDIR",   cld );
  
  InitList( &IncludeDirs      );
  InitList( &SymbolDirs       );
  InitList( &LibraryDirs      );
  InitList( &ScanLibraryNames );
  InitList( &ResLibraryNames  );
  InitList( &Defines          );
  InitList( &SourceFiles      );
  InitList( &CompileOpts      );
  InitList( &PreProcOpts      );
  InitList( &ModulaOpts       );
  InitList( &AssembleOpts     );
  InitList( &MacroOpts        );
  InitList( &LinkOpts         );

  AddLibraryPath( c_libdir );
  
  FortranP1Opts[ 0 ] = '\0';
  FortranP2Opts[ 0 ] = '\0';
  FortranMapFile     = NULL;
  
  NCompiles = 0;
  Flags     = Warns = Errs = 0;
  ObjName   = ObjectName;
  
  HeapSize  = StackSize = 5000;

  /* There does not seem to be an easy way to define the following */
  /* automatically without introducing some circularity		   */
  
  AddInclude( "." );
  AddSymbol ( "." );
  
#ifdef UNIX
  ram_disk_present = false;
#else

  if ((o = Locate( NULL, "/loader/ram" )) == NULL)
    {
      ram_disk_present = false;
    }
  else
    {
      ram_disk_present = true;
      
      Close( o );
      
      AddInclude( "/ram/include/" );
      AddSymbol ( "/ram/symbols/" );
    }
#endif

  for (i = 1; i < argc; i++ )
    {
      char *	arg = argv[ i ];
      

      if (arg[ 0 ] == '-')
	{
	  switch (arg[ 1 ])
	    {
	    case '\0': /* lone "-" on command line */
	      
	      usage();
	      
	      break;
	      
	    case 'A': /* pass option to linker */

	      if (arg[ 2 ] != '\0')
	  	{
		  AddName( &LinkOpts, &arg[ 2 ] );
		}
	      else
		{
		  if (++i >= argc)
		    {
		      fail( "No text following -A option" );
		    }
		
		  AddName( &LinkOpts, &arg[ 2 ] );		  
		}
	      break;
	      
	    case 'B': /* don't link with any libraries */
	      
	      Flags |= FLAG_B;
	      
	      if (state == normal)
		state = objed_suppressed;
	      
	      break;
	      
	    case 'C': /* perform array bound checking */

	      if (streq( arg, "-C40" ))
		{
		  RemoveLibraryPath( c_libdir );
		  
		  compiler    = C40_COMPILER;
		  linker      = C40_LINKER;
		  c_compiler  = C40_CC;
		  c_libdir    = C40_C_LIBDIR;
		  assembler   = C40_ASSEMBLER;
		  machinename = C40_MACHINENAME;
		  
		  AddLibraryPath( c_libdir );
		}
	      else
		{
		  strcat( FortranP2Opts, "+B" );

		  if (_is_norcroft_compiler)
		    {
		      AddName( &CompileOpts, "-Zpc1" );		  
		    }
		}
	      
	      break;
	      
	    case 'D': /* macro definitions */

	      if (arg[ 2 ] == '\0')
		{
		  if (++i >= argc)
		    {
		      fail( "No text following -D option" );
		    }

		  AddName( &Defines, argv[ i ] );
		}
	      else
		{
		  AddName( &Defines, &arg[ 2 ] );
		}
	      
	      break;
	      
	    case 'E': /* pre-process only */
	      
	      Flags |= FLAG_p;
	      
	      break;
	      
	    case 'F': /* compiler features */
	      if (arg[ 2 ] != '\0')
		{
		  ParseFeatures( &arg[ 2 ] );
		}
	      else
		{
		  warning( "no characters after -F option" );
		}
	      break;
	      
	    case 'I': /* include directory */
	      
	      if (arg[ 2 ] != '\0')
		{
		  AddInclude( &arg[ 2 ] );
		  AddSymbol ( &arg[ 2 ] );
		}
	      else
		{
		  if (++i >= argc)
		    {
		      fail( "No directory name following -I option" );
		    }
		  
		  AddInclude( argv[ i ] );
		  AddSymbol ( argv[ i ] );
		}
	      
	      break;
	      
	    case 'J': /* append to library */
	      Flags |= FLAG_J;
	      
	      break;
	      
#ifndef __ARM	      
	    case 'K': /* pass option to Modula-2 compiler */

	      if (arg[ 2 ] != '\0')
		AddName( &ModulaOpts, &arg[ 2 ] );
	      
	      break;
#endif /* __ARM */
	      
	    case 'L': /* add name to library path */

	      if (arg[ 2 ] == '\0')
		{
		  if (++i >= argc)
		    {
		      fail( "No directory name following -L option" );
		    }
		  
		  AddLibraryPath( argv[ i ] );
		}
	      else
		{
		  AddLibraryPath( &arg[ 2 ] );
		}
	      
	      break;
	      
	    case 'M': /* produce map file */

	      if (streq( arg, "-M68K" ))
		{
		  RemoveLibraryPath( c_libdir );
		  
		  compiler    = M68K_COMPILER;
		  linker      = M68K_LINKER;
		  c_compiler  = M68K_CC;
		  c_libdir    = M68K_C_LIBDIR;
		  assembler   = M68K_ASSEMBLER;
		  machinename = M68K_MACHINENAME;
		  
		  AddLibraryPath( c_libdir );
		}
	      else if (arg[ 2 ] != '\0')
		{
		  FortranMapFile = &arg[ 2 ];
		}	      
	      
	      break;
	      
	    case 'O': /* Optimise */
	      
	      Flags |= FLAG_O;
	      break;
	      
	    case 'P': /* pass on option to pre-processor */

	      if (arg[ 2 ] == 0)
		{
		  if (++i >= argc)
		    {
		      fail( "No text following -P option" );
		    }
		  
		  AddName( &PreProcOpts, argv[ i ] );
		}
	      else
		{
		  AddName( &PreProcOpts, &arg[ 2 ] );
		}
	      
	      break;
	      
	    case 'R': /* use RAM disk for temporary files even if ram disk is not present */
	      
	      ram_disk_present = true;
	      break;
	      
	    case 'S': /* Assemble only */
	      
	      Flags |= FLAG_S;
	      break;
	      
	    case 'T': /* Transputer type */
	      if (compiler != TRANSPUTER_COMPILER && compiler != UNKNOWN_COMPILER)
		warning( "-Tn option not applicable to non transputer processors" );
	      else
		switch (arg[ 2 ])
		  {
		  case '9':
		    AddName( &ModulaOpts,  "-t9" );
		    AddName( &CompileOpts, "-t9" );
		    AddName( &Defines,    "T9000" );		    
		    strcat( FortranP2Opts, "+F" );
		    break;
		    
		  case '8':
		    AddName( &ModulaOpts,  "-t8" );
		    AddName( &CompileOpts, "-t8" );
		    AddName( &Defines,    "T800" );
		    strcat( FortranP2Opts, "+F" );
		    break;
		    
		  case '5':
		    AddName( &ModulaOpts,  "-t5" );
		    AddName( &CompileOpts, "-t5" );
		    AddName( &Defines,    "T400" );
		    break;
		    
		  case '4':
		    AddName( &ModulaOpts,  "-t4" );
		    AddName( &CompileOpts, "-t4" );
		    AddName( &Defines,    "T400" );
		    break;
		    
		  case '\0':
		    warning( "No number after -T option, ignored" );
		    
		    break;
		    
		  default:
		    warning( "Unknown transputer type -T'%c', ignored", arg[ 2 ] );
		    
		    break;
		  }

	      break;
	      
	    case 'U': /* remove macro definition */

	      if (arg[ 2 ] == '\0')
		{
		  if (++i >= argc)
		    {
		      fail( "No text following -U option" );
		    }
		  
		  (void) RemoveName( &Defines, argv[ i ] );
	      
		  if (streq( argv[ i ], "_BSD" ))
		    (void) RemoveName( &ScanLibraryNames, "/helios/libbsd.a" );
		}
	      else
		{
		  (void)RemoveName( &Defines, &arg[ 2 ] );
	      
		  if (streq( &arg[ 2 ], "_BSD" ))
		    (void) RemoveName( &ScanLibraryNames, "/helios/libbsd.a" );
		}
	      
	      break;
	      
	    case 'V': /* verbose */
	      
	      Flags |= FLAG_V;
	      break;
	      
	    case 'W': /* warning level */
	      if (arg[ 2 ] != '\0' && arg[ 3 ] == ',')
		{
		  NameList *	plist;

		  
		  /* we have an argument of the form -Wc,arg[,arg...] */

		  switch (arg[ 2 ])
		    {
		    case 'p':	/* pre - processor */
		      plist = &PreProcOpts;
		      break;
		      
		    case '2':	/* optimising C compiler */
		      
		      /* drop through */
		      
		    case '0':	/* C compiler */
		      plist = &CompileOpts;
		      break;
		      
		    case 'a':	/* assembler */
		      plist = &AssembleOpts;
		      break;
		      
		    case 'l':	/* linker */
		      plist = &LinkOpts;
		      break;

		    case 'M':	/* MacroAssembler */
		      plist = &MacroOpts;
		      break;

		    case 'm':	/* Modula-2 */
		      plist = &ModulaOpts;
		      break;

		    default:
		      warning( "unknown compiler phase '%c', -W option has been ignored", arg[ 2 ] );
		      
		      plist = NULL;
		      
		      break;
		    }

		  if (plist)
		    {
		      char *	end;

		      
		      /* advance past start of argument */

		      arg += 4;	/* "-Wc," */

		      do
			{
			  end = strchr( arg, ',' );

			  if (end)
			    {
			      *end++ = '\0';
			    }

			  AddName( plist, arg );

			  arg = end;
			}
		      while (end);
		    }
		}
	      else
		{
		  strcat( FortranP1Opts, "+W" );

		  if (arg[ 2 ] != '\0')
		    strcat( FortranP1Opts, &arg[ 2 ] );
		}

	      break;
	      
	    case 'X': /* Cross reference width */
	      
	      strcat( FortranP1Opts, "+X" );

	      if (arg[ 2 ] != '\0')
		strcat( FortranP1Opts, &arg[ 2 ] );
	      
	      break;
	      
	    case 'Z': /* pass on option to FORTRAN compiler */
	      
	      if (arg[ 2 ] == '\0')
		{
		  if (++i >= argc)
		    {
		      fail( "No text following -Z option" );
		    }
		  
		  strcat( FortranP1Opts, argv[ i ] );
		}
	      else
		{
		  strcat( FortranP1Opts, &arg[ 2 ] );
		}
	      
	      break;
	      
	    case 'a': /* pass on option to assembler */
	      
	      if (arg[ 2 ] == '\0')
		{
		  if (++i >= argc)
		    {
		      fail( "No text following -a option" );
		    }
		  
		  AddName( &AssembleOpts, argv[ i ] );
		}
	      else
		{
		  AddName( &AssembleOpts, &arg[ 2 ] );
		}
	      
	      break;
	      
	    case 'b': /* don't link with standard maths libraries */
	      
	      Flags |= FLAG_b;
	      
	      break;
	      
	    case 'c': /* compile only */
	      
	      if (streq( arg, "-c40" ))
		{
		  RemoveLibraryPath( c_libdir );
		  
		  compiler    = C40_COMPILER;
		  linker      = C40_LINKER;
		  c_compiler  = C40_CC;
		  assembler   = C40_ASSEMBLER;
		  c_libdir    = C40_C_LIBDIR;
		  machinename = C40_MACHINENAME;
		  
		  AddLibraryPath( c_libdir );
		}
	      else
		{
		  Flags |= FLAG_c;
		}

	      break;
	      
	    case 'd': /* .def output file */

	      if (arg[ 2 ] != '\0')
		AddName( &LinkOpts, &arg[ 0 ] );
	      
	      break;
#ifndef __ARM	      
	    case 'e': /* enforce standards */
	      
	      switch (arg[ 2 ])
		{
		case '6':
		  strcat( FortranP1Opts, "+6" );
		  break;
		  
		case '7':
		  strcat( FortranP1Opts, "+7" );
		  break;

		case '\0':
		  warning( "No number following -e option, ignored" );

		  break;
		  
		default:
		  warning( "Unknown -e option %c, ignored", arg[ 2 ] );
		  
		  break;
		}
	      
	      break;
#endif /* __ARM */	      
	    case 'f': /* virtual memory file name */
	      
	      if (arg[ 2 ] != '\0')
		{
		  vfile = &arg[ 2 ];
		}
	      else
		{
		  if (++i >= argc)
		    {
		      fail( "No filename following -f" );
		    }
		  
		  vfile = argv[ i ];
		}
	      
	      break;
	      
	    case 'g': /* compile for debugging */
	      
	      AddName( &CompileOpts, "-g" );
	      
	      AddName( &ModulaOpts,  "-g" );
	      
	      strcat( FortranP2Opts, "+G" );

	      (void) AddLibrary( c_libdir, "d" );	/* debugging library */
	      
	      break;
	      
	    case 'h': /* specify heap size */
	      
	      if (!strncmp( &arg[ 1 ], "help", 4 ))
		{
		  usage();
		  
		  return 0;
		}
	      else if ((HeapSize = atoi( &arg[ 2 ] )) < 200)
		{				
		  warning( "a heap size of %d is too small, adjusting to 200", HeapSize );

		  HeapSize = 200;
		}
	      
	      state = objed_enabled;
	      
	      break;
	      
	    case 'j': /* join object files */
	      
	      Flags |= FLAG_j;
	      
	      break;
	      
	    case 'l': /* specify library */

	      if (arg[ 2 ] == '\0')
		{
		  if (++i >= argc)
		    {
		      fail( "No library name following -l option" );
		    }

		  FindAndAddLibrary( argv[ i ] );
		}
	      else
		{
		  FindAndAddLibrary( &arg[ 2 ] );
		}
	      
	      break;
	      
	    case 'm': /* compile for library */

	      switch (compiler)
		{
		case UNKNOWN_COMPILER:
		default:
		  AddName( &CompileOpts, "-l" );
		  break;

		case TRANSPUTER_COMPILER:
		  fail( "New Compiler does not support resident libraries (yet)" );    
		  break;

		case ARM_COMPILER:
		case C40_COMPILER:
		  AddName( &CompileOpts, "-Zl");
		  AddName( &AssembleOpts, "-d");
		  break;
		  
		case ARM_LTD_COMPILER:
		  AddName( &LinkOpts, "-Al" );
		  break;
		}
	      
	      AddName( &ModulaOpts,  "-m" );
	      
	      strcat( FortranP2Opts, "+M" );

	      break;
	      
	    case 'n': /* report commands executed OR specify object name */
	      
	      if (arg[ 2 ] != '\0')
		{
		  ObjectName = &arg[ 2 ];
		}
	      else
		{
		  Flags |= FLAG_n;
		  Flags |= FLAG_v; 	/* must be verbose too */
		}
	      
	      break;
	      
	    case 'o': /* output file name */
	      
	      Flags |= FLAG_o;
	      
	      if (arg[ 2 ] != '\0')
		{
		  ObjName = &arg[ 2 ];
		  
		  if (streq( ObjectName, OBJNAME ))
		    {
		      ObjectName = ObjName;
		    }
		  
		  if (streq( ObjName, "-" ))
		    {
		      ObjName    = NULL;
		      ObjectName = OBJNAME;
		    }
		}
	      else
		{
		  i++;
		  
		  if (i >= argc)
		    {
		      fail( "No filename following -o option" );
		    }
		  
		  ObjName = argv[ i ];
		  
		  if (streq( ObjectName, OBJNAME ))
		    {
		      ObjectName = ObjName;
		    }
		  
		  if (streq( ObjName, "-" ))
		    {
		      ObjName    = NULL;
		      ObjectName = OBJNAME;
		    }
		}
	      
	      break;
	      
	    case 'p': /* compile for profiling */
	      if (_is_norcroft_compiler)
		{
		  if (_is_transputer_compiler)
		    AddName( &CompileOpts, "-p" );
		  else
		    warning( "-p option is currently only supported for transputers" );
		}
	      else
		AddName( &CompileOpts, "-k" );		/* old Transputer compiler uses -k for profiling */

	      AddName( &ModulaOpts,  "-p" );
	      
	      strcat( FortranP2Opts, "+P" );
	      
	      break;
	      
	    case 'q': /* compiler debugging features */

	      if (_is_norcroft_compiler)
		{
		  AddNamef( &CompileOpts, "-Zq%s", &arg[ 2 ] );
		}
	      else
		{
		  AddNamef( &CompileOpts, "%s", arg );
		}
	      
	      break;
	      
	    case 'r': /* compile for device driver */

	      switch (compiler)
		{
		case UNKNOWN_COMPILER:
		default:
		  AddName( &CompileOpts, "-r" );
		  break;

		case TRANSPUTER_COMPILER:
		  fail( "New Compiler does not support device drivers (yet)" );    
		  break;

		case C40_COMPILER:
		case ARM_COMPILER:
		  AddName( &CompileOpts, "-Zr" );  
		  AddName( &AssembleOpts, "-d" );	/* do not generate module header/trailer */
		  break;

		case ARM_LTD_COMPILER:
		  AddName( &LinkOpts, "-Ar" );
		  break;
		}

	      AddName( &ModulaOpts,  "-r" );
	      
	      strcat( FortranP2Opts, "+R" );
	      
	      break;
	      
	    case 's': /* specify stack size */
	      
	      if (arg[ 2 ] != '\0')
		{
		  StackSize = atoi( &arg[ 2 ] );
		}
	      else
		{
		  if (++i >= argc)
		    {
		      fail( "no number following -s option" );
		    }
		  else
		    {
		      StackSize = atoi( argv[ i ] );
		    }
		}
	      
	      if (StackSize < 200)
		{
		  warning( "a stack size of %d is too small, adjusting to 200", StackSize );

		  StackSize = 200;
		}
	      
	      state = objed_enabled;
	      
	      break;
	      
	    case 't': /* compile tracing code */

	      if (streq( arg, "-tran" ) || streq( arg, "-transputer" ))
		{
		  RemoveLibraryPath( c_libdir );
		  
		  compiler    = TRANSPUTER_COMPILER;
		  linker      = TRANSPUTER_LINKER;
		  c_compiler  = TRANSPUTER_CC;
		  machinename = TRANSPUTER_MACHINENAME;
		  assembler   = TRANSPUTER_ASSEMBLER;
		  c_libdir    = TRANSPUTER_C_LIBDIR;
		  
		  AddLibraryPath( c_libdir );
		}
	      else
		{
		  AddName( &ModulaOpts, "-t" );
	      
		  strcat( FortranP1Opts, "+T" );

		  if (_is_norcroft_compiler)
		    {
		      AddName( &CompileOpts, "-Zpm1" );
		    }
		}
	      
	      break;
	      
	    case 'u': /* do not delete temporary files */
	      
	      suppress_removes = true;
	      
	      break;
	      
	    case 'v': /* verify */
	      
	      strcat( FortranP1Opts, "+V" );
	      strcat( FortranP2Opts, "+V" );
	      
	      Flags |= FLAG_v;
	      
	      break;

	    case 'w': /* warning flags */
             if (arg[ 2 ] != '\0')
               ParseWarnings( &arg[ 2 ] );
	      
	      break;
	  
	    case 'x': /* processor type */

	      if (i != 1)
		warning( "-x<name> option should be first in command list" );
	      
	      RemoveLibraryPath( c_libdir );
	      
	      if (streq( arg, "-xarm"))
		{
		  compiler    = ARM_COMPILER;
		  linker      = ARM_LINKER;
		  c_compiler  = ARM_CC;
		  c_libdir    = ARM_C_LIBDIR;
		  assembler   = ARM_ASSEMBLER;
		  machinename = ARM_MACHINENAME;
		}
	      else if (streq( arg, "-xARM"))
		{
		  compiler    = ARM_LTD_COMPILER;
		  linker      = ARM_LINKER;
		  c_compiler  = ARM_LTD_CC;
		  c_libdir    = ARM_C_LIBDIR;
		  assembler   = ARM_ASSEMBLER;
		  machinename = ARM_MACHINENAME;
		}
	      else if (streq( arg, "-xc40") || streq( arg, "-xC40"))
		{
		  compiler    = C40_COMPILER;
		  linker      = C40_LINKER;
		  c_compiler  = C40_CC;
		  assembler   = C40_ASSEMBLER;
		  c_libdir    = C40_C_LIBDIR;
		  machinename = C40_MACHINENAME;
		}
	      else if (streq( arg, "-xm68k") || streq( arg, "-xM68K"))
		{
		  compiler    = M68K_COMPILER;
		  linker      = M68K_LINKER;
		  c_compiler  = M68K_CC;
		  assembler   = M68K_ASSEMBLER;
		  c_libdir    = M68K_C_LIBDIR;
		  machinename = M68K_MACHINENAME;		  
		}
	      else if (streq( arg, "-xtransputer" ) ||
		       streq( arg, "-xTransputer" ) ||
		       streq( arg, "-xtran" )        )
		{
		  compiler    = TRANSPUTER_COMPILER;
		  linker      = TRANSPUTER_LINKER;
		  c_compiler  = TRANSPUTER_CC;
		  assembler   = TRANSPUTER_ASSEMBLER;
		  c_libdir    = TRANSPUTER_C_LIBDIR;
		  machinename = TRANSPUTER_MACHINENAME;
		}
	      else
		{
		  fail( "No processor type following -x option" );
		}
	      
	      AddLibraryPath( c_libdir );
	      break;

	    case 'y': /* pass on option to macro assembler */
	      
	      if (arg[ 2 ] == '\0')
		{
		  if (++i >= argc)
		    {
		      fail( "No text following -y option" );
		    }
		  
		  AddName( &MacroOpts, argv[ i ] );
		}
	      else
		{
		  AddName( &MacroOpts, &arg[ 2 ] );
		}
	      
	      break;
	      
	    case 'z': /* pass on option to C compiler */
	      
	      if (arg[ 2 ] == '\0')
		{
		  if (++i >= argc)
		    {
		      fail( "No text following -z option" );
		    }
		  
		  AddName( &CompileOpts, &arg[ 2 ] );
		}
	      else
		{
		  AddName( &CompileOpts, &arg[ 2 ] );
		}
	      
	      break;
	      
	    case 'G': /* unused */
	    case 'H': /* unused */
	    case 'N': /* unused */
	    case 'Q': /* unused */
	    case 'Y': /* unused */
	    case 'i': /* unused */
	    case 'k': /* unused */
	    default:
	      
	      warning( "Unknown option '%s' passed to linker", &arg[ 0 ] );
	      
	      AddName( &LinkOpts, &arg[ 0 ] );
	      
	      break;
	    }
	}
      else
	{
	  char *	ptmp;
	  int		len = strlen( arg );


	  /*
	   * catch the library named [<path>]lib<name>.a
	   */
	  
	  if ((ptmp = strrchr( arg, '/' )) == NULL)
	    ptmp = arg;
	  else
	    ++ptmp;

	  if (strncmp( ptmp, "lib", 3 )           == 0 &&
	      strncmp( arg + (len - 2), ".a", 2 ) == 0)
	    {

	      /*
	       * Beware of this horrible hack.
	       *
	       * In CountSources() scanned libraries are counted
	       * as being source files if they do not start with
	       * a -l.  The only candidates for such files are
	       * caught here, but I must prepend the -l option or
	       * otherwise the linker will not know about the
	       * file being a scanned library.  Hence the space
	       * before the -l in the string below.
	       */
	      
	      AddLibraryName( false, " -l%s", arg );
	    }
	  else
	    {
	      AddFile( &SourceFiles, arg );
	    }
	}
    }

  if (_is_norcroft_compiler)
    {
      /* Norcroft compilers try to link automatically ... */

      if (ASSEMBLE_ONLY)
	AddName( &CompileOpts, "-S" );
      else
	AddName( &CompileOpts, "-c" );
    }

  /* catch _BSD being defined and add BSD library to library list, if necessary */
  
    {
      NameNode *	pName;
      

      for (pName  = (NameNode *)FirstNode( (List *) &Defines );
	   pName != NULL;
	   pName  = (NameNode *)NextNode( (Node *)pName ))
	{
	  if (streq( pName->name, "_BSD" ))
	    (void) AddLibrary( c_libdir, "bsd" );
	}
    }

  /* Cope with C_COMPILER environment variables which have parameters included */

  if ((cc = strchr( c_compiler, ' ' )) != NULL)
    {
      char *	copy = (char *) malloc( strlen( c_compiler ) + 1 );

      
      if (copy == NULL) fail( "out of memory copying string" );
      
      strcpy( copy, c_compiler );
      
      cc = strchr( copy, ' ' );

      *cc = '\0';

      c_compiler = copy;

      do
	{
	  while (*cc++ == ' ')
	    ;		     			/* point to start of next word */

	  if (*cc == '\0') break;		/* catch end of string */
	  
	  copy = strchr( cc, ' ' );		/* point to end of next word */

	  if (copy != NULL) *copy = '\0';	/* isolate next word */
	  
	  AddName( &CompileOpts, cc );		/* install next word */

	  cc = copy;				/* advance pointer */
	}
      while (cc != NULL);
    }
  
  /* O/S identification */
  
  AddName( &Defines, "helios" );	/* this is archaic and should be allowed to die out */
  AddName( &Defines, "__HELIOS" );
  
  /* Processor identification */

  AddName( &Defines, machinename );

  /* Compiler and Host identification */

  AddName( &MacroOpts, "-d__HELIOS" );
  AddName( &MacroOpts, "1" );
  
  switch (compiler)
    {
    default:
    case TRANSPUTER_COMPILER:
      AddName( &Defines, "transputer" );
      AddName( &Defines, "__TRAN" );
      AddName( &Defines, "__HELIOSTRAN" );
      AddName( &MacroOpts, "-dhelios.TRAN" );
      AddName( &MacroOpts, "1" );
      AddName( &MacroOpts, "-d__TRAN" );
      AddName( &MacroOpts, "1" );      
      AddName( &MacroOpts, "-d__HELIOSTRAN" );
      AddName( &MacroOpts, "1" );      
      break;

    case ARM_LTD_COMPILER:
    case ARM_COMPILER:
      AddName( &Defines, "__ARM" );
      AddName( &Defines, "__HELIOSARM" );
      AddName( &Defines, "__SMT" );		/* split module table */
      AddName( &MacroOpts, "-dhelios.arm" );
      AddName( &MacroOpts, "1" );
      AddName( &MacroOpts, "-d__ARM" );
      AddName( &MacroOpts, "1" );      
      AddName( &MacroOpts, "-d__HELIOSARM" );
      AddName( &MacroOpts, "1" );      
      AddName( &MacroOpts, "-d__SMT" );
      AddName( &MacroOpts, "1" );      
      break;

    case C40_COMPILER:
      AddName( &Defines, "__C40" );
      AddName( &Defines, "__HELIOSC40" );
      AddName( &Defines, "__SMT" );
      AddName( &MacroOpts, "-dhelios.C40" );
      AddName( &MacroOpts, "1" );
      AddName( &MacroOpts, "-d__C40" );
      AddName( &MacroOpts, "1" );      
      AddName( &MacroOpts, "-d__HELIOSC40" );
      AddName( &MacroOpts, "1" );      
      AddName( &MacroOpts, "-d__SMT" );
      AddName( &MacroOpts, "1" );      
      break;
      
    case I860_COMPILER:
      AddName( &Defines, "__I860" );
      AddName( &Defines, "__HELIOSI860" );
      AddName( &MacroOpts, "-dhelios.I860" );
      AddName( &MacroOpts, "1" );
      AddName( &MacroOpts, "-d__I860" );
      AddName( &MacroOpts, "1" );      
      AddName( &MacroOpts, "-d__HELIOSI860" );
      AddName( &MacroOpts, "1" );      
      break;
      
    case M68K_COMPILER:
      AddName( &Defines, "__M68K" );
      AddName( &Defines, "__HELIOSM68K" );
      AddName( &MacroOpts, "-dhelios.M68K" );
      AddName( &MacroOpts, "1" );
      AddName( &MacroOpts, "-d__M68K" );
      AddName( &MacroOpts, "1" );      
      AddName( &MacroOpts, "-d__HELIOSM68K" );
      AddName( &MacroOpts, "1" );      
      break;      
    }
  
#ifndef UNIX
  AddInclude( "/include" );
#endif
  AddInclude( c_incdir );
  AddSymbol ( symdir );
  
  CountSources();

  if (NSources == 0)
    {
      fail( "nothing to compile" );
    }
  else if (NSources == 1 || NCfiles == 1)
    {
      if (use_non_standard_features && streq( ObjectName, OBJNAME ))
	{
	  char * 	path;


	  /* use the source file name as the output name */
	  
	  ObjName = ((FileNode *)FirstNode( &SourceFiles ))->base;

	  /* but only the file name, not the path */
	     
	  if ((path = strrchr( ObjName, '/' )) != NULL)
	    {
	      ObjName = path + 1;
	    }

	  ObjectName = ObjName;
	}
    }
  else if (NSources > 1 && !LINK && REDIRECT )
    {
      fail( "Invalid -o option: multiple output files" );
    }

  if (use_non_standard_features &&
      (_is_transputer_compiler			/* other compilers seek back to the start of their output */
       && !JOIN					/* do not use pipes if joining together library files */
       && !APPEND				/* or concatentating library files */
       && NSources == 1				/* more than one source file means temp files must be used */
       && (NCfiles > 0 || NAfiles > 0)		/* do not use pipes when just assembling */
       ))
    {
      Flags |= FLAG_P;
    }
  
  AddWarningsAndFeatures();
  AddDefines();
  AddIncludeOpts();
  AddSymbolOpts();
  
  if (OPTIMISE)
    {
      if (_is_transputer_compiler)
	{
	  AddName( &AssembleOpts, "-f" ); /* full link */
	  AddName( &LinkOpts,     "-f" );
	}      
    }
  
  if (VERBOSE)
    {
      AddName( &AssembleOpts, "-v" );
      AddName( &ModulaOpts,   "-v" );
      AddName( &LinkOpts,     "-v" );
    }
  
  if (STDLIBS)
    {
      /* add standard libraries */

      if (link_helios[ 0 ] == '/')
	{
	  AddLibraryName( TRUE, "-l%s", link_helios );
	}
      else if (!AddLibrary( c_libdir, link_helios ) && !COMPILE_ONLY)
	{
	  warning( "Could not find standard Helios library '%s%s'!", c_libdir, link_helios );
	}
    }
  
#ifdef __ARM
  if  (ASSEMBLE_ONLY)
    {
      AddName( &CompileOpts, "-S" );
    }
  else
    {
      AddName( &CompileOpts, "-c" );
    }
  
  /* following line is a fudge! */

  AddName( &CompileOpts, "-WANPVDF" );

  if (compiler == ARM_LTD_COMPILER)
    {
      AddName( &CompileOpts, "-li" );
      AddName( &CompileOpts, "-apcs" );
      AddName( &CompileOpts, "3/reentrant" );
    }
#endif /* __ARM */
  
  succeeded = DoCompiles();

  if (succeeded && ASSEMBLE)
    {
      /* > 0 because if NSfiles > 0 we must assemble */
      
      if (!LINK || NCfiles + NFfiles + NSfiles + NAfiles + NMfiles > 0)
	succeeded |= DoAssembles();
      
      /* attempt to link only if not all Modula-2 .def files given */
      
      if (NSources != NDfiles && LINK && succeeded)
	{
	  succeeded = DoLink();
	}
    }
  
  if (succeeded && USE_PIPE)
    {
      succeeded = (ExecuteCommandLine() == 0);
    }

  return (succeeded ? EXIT_SUCCESS : EXIT_FAILURE);
  
} /* main */

/*}}}*/

/*}}}*/

/* end of c.c */
