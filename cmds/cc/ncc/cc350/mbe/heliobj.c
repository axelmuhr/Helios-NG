/*{{{  Comments */

/*
 * C compiler file heliobj.c
 * Copyright (C) Perihelion Software Ltd, 1988 - 1994
 * adapted by Codemist Ltd, 1989
 * tidied up by NC 1991-1993
 */

/* #define STIK_NOW_WORKS_ON_HARDWARE 1 */

/* AM Apr 89: change 'stublist' so that items are merely marked deleted */
/* (by having xr_defloc/xr_defext) instead of physical removeal.        */
/*
 * This was once called heliosobj.c, but to keep file component makes
 * short (e.g. for when we want to host on an MSDOS machine, yuk) it has
 * been contracted to heliobj.c
 */

/*}}}*/
/*{{{  #includes */

# ifdef __STDC__
# include <string.h>
# else
# include <strings.h>
# endif

#include <stddef.h>

#ifndef SEEK_SET
#define SEEK_SET  0
#endif

#ifndef __SMT
#define __SMT
#endif

#ifndef __C40
#define __C40
#endif

#include </hsrc/include/module.h>
#include </hsrc/include/helios.h>

#include <errno.h>
#include "globals.h"
#include "mcdep.h"
#include "mcdpriv.h"
#include "xrefs.h"
#include "store.h"
#include "codebuf.h"
#include "mipvsn.h"
#include "jopcode.h"
#include "ops.h"            /* For R_ADDR1 etc */
#include "regalloc.h"       /* for regmask */
#include "cg.h"             /* for procflags */
#include "errors.h"
#include "peep.h"
#include "builtin.h"		/* for mainsym */

/*}}}*/
/*{{{  Macros */

#ifndef offsetof
#define offsetof( type, member ) ((char *)&(((type *)0)->member) - (char *)0)
#endif

/*}}}*/
/*{{{  Constants */

#define OBJCODE      	0x01
#define OBJBSS        	0x02
#define OBJINIT       	0x03

#define OBJBYTE       	0x09	   	/* ls 3 bits = size      */
#define OBJSHORT      	0x0a
#define OBJWORD       	0x0c

#define OBJCODESYMB   	0x0d		/* was funcref */
#define OBJMODSIZE  	0x0e
#define OBJLABELREF   	0x0f
#define OBJDATASYMB   	0x10		/* was dataref */
#define OBJDATAMODULE 	0x11
#define OBJMODNUM     	0x12

#define OBJPATCH      	0x13   		/* PATCHES are 0x13 - 0x1f */
#define OBJPATCHMAX   	0x1f

#define OBJMODULE     	0x20
#define OBJBYTESEX	0x21

#define OBJGLOBAL     	0x22
#define OBJLABEL      	0x23
#define OBJDATA       	0x24
#define OBJCOMMON     	0x25
#define OBJCODETABLE  	0x26
#define OBJREF        	0x27		/* force reference to another library */
#define OBJCODESTUB	0x28		/* address of fn or stub */
#define OBJADDRSTUB	0x29		/* stub returning address of fn */

#define OBJNEWSEG	0x30
#define OBJEND		0x31
#define OBJLITERAL	0x32	   	/* OBJCODE of <= 4 bytes */

#define PATCHADD  	0x13 		/* patch = val1 +  val2 */
#define PATCHSHIFT	0x14 		/* patch = val2 << val1 */ /* right shift for -ve val1 */
#define PATCHSWAP	0x1E		/* patch = byte_swap( val1 ) */
#define PATCHOR		0x1f		/* patch = val1 | val2 */

#define PATCHC40DATAMODULE1	0x15
#define PATCHC40DATAMODULE2	0x16
#define PATCHC40DATAMODULE3	0x17
#define PATCHC40DATAMODULE4	0x18
#define PATCHC40DATAMODULE5	0x19
#define PATCHC40MASK24ADD	0x1a
#define PATCHC40MASK16ADD	0x1b
#define PATCHC40MASK8ADD	0x1c

/*}}}*/
/*{{{  Types */

typedef struct GSymlist
  {
    struct GSymlist *	cdr;
    Symstr *		sym;
  }
GSymlist;

typedef struct Stub
  {
    struct Stub *	stubcdr;
    Symstr *		stubsym;
  }
Stub;

/*}}}*/
/*{{{  Variables */

FILE *			objstream;
ExtRef *		obj_symlist;
CodeXref *		codexrefs;
DataXref *		dataxrefs;
int 			suppress_module;
ExtRef *		datasymbols;
LabelNumber *		exporting_routines = NULL;


static Stub *		stublist;
static GSymlist *	global_symbols;
static GSymlist *	stub_symbols;
static GSymlist *	addr_stub_symbols;
static int32 		obj_symcount;
static ExtRef **	obj_symlistend;
static int32     	codesize;
static int32     	codesizepos;
static int32     	datasize;              /* only for obj file use */
static int32     	datasizepos;
static int32     	maxcodep;
static int32     	maxcodeppos;

/*}}}*/
/*{{{  Functions */

/*{{{  objbyte() */

static void
objbyte( int32 b )
{
  if (objstream == NULL)
    {
      syserr( heliobj_no_output_stream );

      return;
    }
  
  putc( (char)b, objstream );

  return;
  
} /* objbyte */

/*}}}*/
/*{{{  objword() */

static void
objword( int32 x )
{
  unsigned char	val;

#ifdef TARGET_IS_LITTLE_ENDIAN
  val = (unsigned char)(x & 0xff);
  
  fwrite( &val, 1, sizeof (val), objstream );

  val = (unsigned char)((x >> 8) & 0xff);
  
  fwrite( &val, 1, sizeof (val), objstream );

  val = (unsigned char)((x >> 16) & 0xff);
  
  fwrite( &val, 1, sizeof (val), objstream );

  val = (unsigned char)((x >> 24) & 0xff);
  
  fwrite( &val, 1, sizeof (val), objstream );
  
#else /* ! TARGET_IS_LITTLE_ENDIAN */
  
  val = ((x >> 24) & 0xff);
  
  fwrite( &val, 1, sizeof (val), objstream );

  val = ((x >> 16) & 0xff);

  fwrite( &val, 1, sizeof (val), objstream );

  val = ((x >> 8) & 0xff);
  
  fwrite( &val, 1, sizeof (val), objstream );

  val = (x & 0xff);

  fwrite( &val, 1, sizeof (val), objstream );
   
#endif /* TARGET_IS_LITTLE_ENDIAN */

  return;
  
} /* objword */

/*}}}*/
/*{{{  objnum() */

#define bitmask( n ) ((1 << (n)) - 1)

static void
objnum( int32 n )
{
  int32 	i;
  int32 	nflag = (n < 0)? n = -n, 1 : 0;
  int32 	mask7 = bitmask( 7 );

#define NFLAG 0x40
#define MORE  0x80

  /* The prefix notation expressed in this function is described */
  /* in Nick Garnett's description of the Helios Link Format for */
  /* Non-Transputer Processors                                   */

  for (i = 28; i != 0; i -= 7)
    if ((n & (mask7 << i)) != 0)
      break;

  if ((n >> i) & NFLAG)
    i += 7;

  objbyte( (n >> i) | (nflag ? NFLAG : 0) | (i > 0 ? MORE : 0) );

  for (i -= 7; i >= 0; i -= 7)
    objbyte( ((n >> i) & mask7) | (i > 0 ? MORE : 0) );

  return;
  
} /* objnum */

/*}}}*/
/*{{{  objsymbol() */

static void
objsymbol(
	  char		prefix,
	  char *	s )
{
  objbyte( prefix );
  
  fputs( s, objstream );

  objbyte( '\0' );

  return;
  
} /* objsymbol */

/*}}}*/
/*{{{  globalise() */

static void
globalise( Symstr * s )
{
  GSymlist *	g;


  for (g = global_symbols; g != NULL; g = g->cdr)
    if (g->sym == s)
      return;
  
  global_symbols = (GSymlist *)global_cons2( SU_Other, global_symbols, s );

  return;
  
} /* globalise */

/*}}}*/
/*{{{  dumpglobals() */

static void
dumpglobals( void )
{
  GSymlist *	g;


  for (g = global_symbols; g != NULL; g = g->cdr)
    {
      objbyte( OBJGLOBAL );
      
      if (new_stubs && is_code( symext_( g->sym )))
	objsymbol( '.', symname_( g->sym ) );
      else
	objsymbol( '_', symname_( g->sym ) );
    }

  return;
  
} /* dumpglobals */

/*}}}*/
/*{{{  request_addr_stub() */

void
request_addr_stub( Symstr * s )
{
  GSymlist *	a;


  for (a = addr_stub_symbols; a != NULL; a = a->cdr)
    if (a->sym == s)
      return;
  
  addr_stub_symbols = (GSymlist *)global_cons2( SU_Other, addr_stub_symbols, s );

  return;
  
} /* request_addr_stub */

/*}}}*/
/*{{{  request_new_stub() */

void
request_new_stub( Symstr * s )
{
  GSymlist *	g;


  for (g = stub_symbols; g != NULL; g = g->cdr)
    if (g->sym == s)
      return;
  
  stub_symbols = (GSymlist *)global_cons2( SU_Other, stub_symbols, s );

  return;
  
} /* request_new_stub */

/*}}}*/
/*{{{  dump_new_stubs() */

static void
dump_new_stubs( void )
{
  GSymlist *	g;


  for (g = stub_symbols; g != NULL; g = g->cdr)
    {
      objbyte( OBJGLOBAL );
      
      objsymbol( '.', symname_( g->sym ) );

      objbyte( OBJREF );
      
      objsymbol( '_', symname_( g->sym ) );
    }

  for (g = addr_stub_symbols; g != NULL; g = g->cdr)
    {
      objbyte( OBJGLOBAL );
      
      fputs( ".addr", objstream );
      
      objsymbol( '.', symname_( g->sym ) );

      /*
       * We must generate a REF to the original function name
       * in order to ensure that the module containing the function
       * is linked, even if the function itself is never called.
       */
      
      objbyte( OBJREF );
      
      objsymbol( '.', symname_( g->sym ) );
    }

  return;
  
} /* dump_new_stubs */

/*}}}*/
/*{{{  objdirective() */

/*
 * this functions handles the cross reference directive 'xrtype'
 * and issues suitable linker instructions to ensure the correct
 * patching is performed.  The symbol 'sym' (if needed) is the
 * symbol refered to by the cross reference.  The word 'code'
 * is the instruction that is being patched.
 *
 * The function returns the number of bytes that have been altered
 * in the output file
 */

static int32
objdirective(
	     int32	xrtype,
	     Symstr *	sym,
	     int32	code )
{
  switch (xrtype)
    {
    case X_Modnum:				/* see load_static_data_ptr() in c40/gen.c */
      /*
       * take code word and add in module number,
       * shifted up to be a word offset.  (If we
       * are using split modules then there is a
       * two word structure for each module table
       * entry so we make module number be a two
       * word offset).  Note that for the 'C40
       * we are loading a WORD offset, not a BYTE
       * offset, so we may not need to shift at all
       */
      
      objbyte( OBJWORD );			/* patch the following word by ... */
      if (few_modules)
	objbyte( PATCHC40MASK8ADD );		/* ... adding into the bottom 8 bits of ... */
      else					/* or  */
	objbyte( PATCHC40MASK16ADD );		/* ... adding into the bottom 16 bits of ... */
      objnum( code );				/* ... the current word op code ... */
      if (split_module_table)
	{
	  objbyte( PATCHSHIFT );		/* ... the result of shifting ... */
	  objnum( 1 );				/* ... up by 1 bit ... */
	}
      objbyte( OBJMODNUM );			/* ... the current module number */
      break;
	
    case X_DataAddr:		/* see load_address_constant in c40/gen.c */
      objbyte( OBJWORD );			/* patch the following word by ... */
      objbyte( PATCHC40MASK16ADD );		/* ... adding into the bottom 16 bits of ... */
      objnum( code );				/* ... the current word of code ... */
      objbyte( PATCHSHIFT );			/* ... the result of shifting ... */
      objnum( -16 );				/* ... right by 16 bits ... */
      objbyte( PATCHSHIFT );			/* ... the result of shifting ... */
      objnum( 14 );				/* ... left by 14 bits ... */
      if (is_function( sym ))
	{
	  if (new_stubs)
	    {
	      objbyte( OBJCODESTUB );		/* ... the offset of ... */
	      objsymbol( '.', symname_( sym ) );/* ... the symbol */
	    }
	  else
	    {
	      objbyte( OBJLABELREF );		/* ... the offset of ... */
	      objsymbol( '#', symname_( sym ) );/* ... the symbol */
	    }
	}      
      else
	{
	  objbyte( OBJLABELREF );		/* ... the offset of ... */
	  objsymbol( '_', symname_( sym ) );	/* ... the symbol */
	}
      break;

    case X_DataAddr1:		/* high part of previous patch, see c40/gen.c */
      objbyte( OBJWORD );			/* patch the following word by ... */
      objbyte( PATCHC40MASK16ADD );		/* ... adding into the bottom 16 bits of ... */
      objnum( code );				/* ... the current word of code ... */
      objbyte( PATCHSHIFT );			/* ... the result of shifting ... */
      objnum( -18 );				/* ... right by 18 bits ... */
      objbyte( PATCHADD );			/* ... the result of adding ... */
      objnum( -3 * sizeof_int );		/* ... -3 * sizeof_int to ... */
      if (is_function( sym ))
	{
	  if (new_stubs)
	    {
	      objbyte( OBJCODESTUB );		/* ... the offset of ... */
	      objsymbol( '.', symname_( sym ) );/* ... the symbol */
	    }
	  else
	    {
	      objbyte( OBJLABELREF );		/* ... the offset of ... */
	      objsymbol( '#', symname_( sym ) );/* ... the symbol */
	    }
	}	  
      else
	{
	  objbyte( OBJLABELREF );		/* ... the offset of ... */
	  objsymbol( '_', symname_( sym ) );	/* ... the symbol */
	}	  
      break;
            
    case X_FuncAddr:		/* see load_address_constant in c40/gen.c */
      objbyte( OBJWORD );			/* patch the following word by ... */
      objbyte( PATCHC40MASK24ADD );		/* ... adding into the bottom 24 bits of ... */
      objnum( code );				/* ... the current word of code ... */
      objbyte( PATCHSHIFT );			/* ... the result of shifting right ... */
      objnum( -2 );				/* ... by two bits ... */
      objbyte( OBJADDRSTUB );			/* ... the offset of ... */
      fputs( ".addr", objstream );		/* ... the address obtaining function for ... */
      objsymbol( '.', symname_( sym ) );	/* ... the symbol */
      break;

    case X_PCreloc:		/* see call() in c40/gen.c */
      /*
       * This occurs when we have a call to a function stub
       */
       
      objbyte( OBJWORD );			/* patch the following word by ... */
      objbyte( PATCHC40MASK24ADD );		/* ... adding into the bottom 24 bits of ... */
      objnum( code );				/* ... the current word of code ... */
      objbyte( PATCHSHIFT );			/* ... the result of shifting ... */
      objnum( -2 );				/* ... right by two bits ... */
      if (new_stubs)
	{
	  objbyte( OBJCODESTUB );		/* ... the offset of ... */
	  objsymbol( '.', symname_( sym ) );	/* ... the symbol */
	}
      else
	{
	  objbyte( OBJLABELREF );		/* ... the offset of ... */
	  objsymbol( '#', symname_( sym ) );	/* ... the symbol */
	}
      break;
      
    case X_DataModule:				/* see load_address_constant in c40/gen.c */

      objbyte( OBJWORD );			/* patch the following word by ... */
      if (few_modules)
	objbyte( PATCHC40MASK8ADD );		/* ... adding into the bottom 8 bits of... */	
      else
	objbyte( PATCHC40MASK16ADD );		/* ... adding into the bottom 16 bits of... */
      objnum( code );				/* ... the current word op code ... */
      objbyte( PATCHSHIFT );			/* ... the result of shifting ... */
      objnum( -2 );				/* ... down by two bits ... */
      objbyte( OBJDATAMODULE );			/* ... the module number of ... */
      objsymbol( '_', symname_( sym ) );	/* ... the symbol */
      break;
      
    case X_DataModule1:
      objbyte( OBJWORD );			/* patch the following word by ... */
      objbyte( PATCHC40DATAMODULE1 );		/* ... applying 'C40 specific patch to ... */
      objnum( code );				/* ... a dummy value and ... */
      objbyte( PATCHSHIFT );			/* ... the result of shifting ... */
      objnum( -2 );				/* ... down by two bits ... */
      objbyte( OBJDATAMODULE );			/* ... the module number of ... */
      objsymbol( '_', symname_( sym ) );	/* ... the specified symbol */
      break;

    case X_DataModule2:
      objbyte( OBJWORD );			/* patch the following word by ... */
      objbyte( PATCHC40DATAMODULE2 );		/* ... applying 'C40 specific patch to ... */
      objnum( code );				/* ... a dummy value and ... */
      if (is_function( sym ))
	{
	  objbyte( PATCHSHIFT );		/* ... the result of shifting ... */
	  objnum( -2 );				/* ... down by 2 bits ... */
	}
      if (split_module_table)
	{
	  objbyte( OBJCODESYMB );		/* ... the offset into the codetable of ... */
	}
      else					/* ... or ... */
	{	  
	  objbyte( OBJDATASYMB );		/* ... the offset into the data area of ... */
	}
      objsymbol( '_', symname_( sym ) );	/* ... the specified symbol */
      break;

    case X_DataModule3:
      objbyte( OBJWORD );			/* patch the following word by ... */
      objbyte( PATCHC40DATAMODULE3 );		/* ... applying 'C40 specific patch to ... */
      objnum( code );				/* ... a dummy value and ... */
      objbyte( OBJMODNUM );			/* ... another dummy value */
      break;

    case X_DataModule4:
      objbyte( OBJWORD );			/* patch the following word by ... */
      objbyte( PATCHC40DATAMODULE4 );		/* ... applying 'C40 specific patch to ... */
      objnum( code );				/* ... a dummy value and ... */
      objbyte( OBJMODNUM );			/* ... another dummy value */
      return sizeof_int;

    case X_DataModule5:
      objbyte( OBJWORD );			/* patch the following word by ... */
      objbyte( PATCHC40DATAMODULE5 );		/* ... applying 'C40 specific patch to ... */
      objnum( code );				/* ... a dummy value and ... */
      objbyte( OBJMODNUM );			/* ... another dummy value */
      break;

    case X_DataSymbHi:
      objbyte( OBJWORD );			/* patch the following word by ... */
      objbyte( PATCHC40MASK16ADD );		/* ... adding into the bottom 16 bits of ... */
      objnum( code );				/* ... the current word op code ... */
      objbyte( PATCHSHIFT );			/* ... the result of shifting ... */
      if (split_module_table && is_function( sym ))
	{
	  objnum( -18 );			/* ... down by 18 bits ... */
	  objbyte( OBJCODESYMB );		/* ... the offset into the codetable of ... */
	}
      else					/* ... or ... */
	{	  
	  objnum( -16 );			/* ... down by 16 bits ... */
	  objbyte( OBJDATASYMB );		/* ... the offset into the data area of ... */
	}
      objsymbol( '_', symname_( sym ) );	/* ... the specified symbol */
      break;

    case X_DataSymbLo:
      objbyte( OBJWORD );			/* patch the following word by ... */
      objbyte( PATCHC40MASK16ADD );		/* ... adding into the bottom 16 bits of... */
      objnum( code );				/* ... the current word op code ... */
      objbyte( PATCHSHIFT );			/* ... the result of shifting ... */
      objnum( -16 );				/* ... down by 16 bits ... */
      objbyte( PATCHSHIFT );			/* ... the result of shifting ... */
      if (split_module_table && is_function( sym ))
	{
	  objnum( 14 );				/* ... up by 14 bits ... */
	  objbyte( OBJCODESYMB );		/* ... the offset into the codetable of ... */
	}
      else					/* ... or ... */
	{	  
	  objnum( 16 );				/* ... up by 16 bits ... */
	  objbyte( OBJDATASYMB );		/* ... the offset into the data area of ... */
	}
      objsymbol( '_', symname_( sym ) );	/* ... the specified symbol */
      break;

    case X_DataSymb:				/* see load_address_constant in c40/gen.c */
      objbyte( OBJWORD );			/* patch the following word by ... */
      objbyte( PATCHC40MASK16ADD );		/* ... adding into the bottom 16 bits of... */
      objnum( code );				/* ... the current word op code ... */
      objbyte( PATCHSHIFT );			/* ... the result of shifting ... */
      objnum( -2 );				/* ... down by 2 bits ... */
      if (split_module_table)
	{
	  objbyte( OBJCODESYMB );		/* ... the offset into the codetable of ... */
	}
      else					/* ... or ... */
	{	  
	  objbyte( OBJDATASYMB );		/* ... the offset into the data area of ... */
	}
      objsymbol( '_', symname_( sym ) );	/* ... the specified symbol */
      break;

    case X_PCreloc2:				/* see routine_entry() in mbe/gen.c */
      /*
       * This occurs when we have a conditional call to a function stub
       */
       
      objbyte( OBJWORD );			/* patch the following word by ... */
      objbyte( PATCHC40MASK16ADD );		/* ... adding into the bottom 16 bits of ... */
      objnum( code );				/* ... the current word of code ... */
      objbyte( PATCHSHIFT );			/* ... the result of shifting ... */
      objnum( -2 );				/* ... right by two bits ... */
      if (new_stubs)
	{
	  objbyte( OBJCODESTUB );		/* ... the offset of ... */
	  objsymbol( '.', symname_( sym ) );	/* ... the symbol */
	}
      else
	{
	  objbyte( OBJLABELREF );		/* ... the offset of ... */
	  objsymbol( '#', symname_( sym ) );	/* ... the symbol */
	}
      break;
      
#ifdef TARGET_HAS_DEBUGGER
    case X_Debug_Modnum:
      objbyte( OBJWORD );			/* patch the following word by inserting ... */
      objbyte( OBJMODNUM );			/* ... the current module number */
      break;
      
    case X_Debug_Offset:
      objbyte( OBJWORD );			/* patch the following word by inserting ... */
      if (split_module_table)
	{
	  objbyte( OBJCODESYMB );		/* ... the offset into the function table of ... */
	}
      else					/* ... or ... */
	{	  
	  objbyte( OBJDATASYMB );		/* ... the offset into the data table of ... */
	}
      objsymbol( '_', symname_( sym ) );	/* ... the function specified by symbol */
      break;
      
    case X_Debug_Ref:
      objbyte( OBJWORD );			/* patch the following word by inserting ... */
      objbyte( OBJLABELREF );			/* ... the offset of ... */
      objsymbol( '_', symname_( sym ) );	/* ... the specified function */
      break;
      
#endif /* TARGET_HAS_DEBUGGER */
      
    case X_Init:
      objbyte( OBJINIT );
      break;

    default:
      syserr( syserr_heliobj_bad_xref, sym, (long)xrtype );

    case X_absreloc:
      return 0;
    }

  return 4;
  
} /* objdirective */

/*}}}*/
/*{{{  startcode() */

static void
startcode( int32 n )
{
  if (n == 0 || objstream == NULL)
    return;

  objbyte( OBJCODE );
  
  objnum( n );

  return;
  
} /* startcode */

/*}}}*/
/*{{{  startdata() */

static void
startdata(
	  int32		n,
	  char *	name )
{
  if (objstream)
    {
      objbyte( OBJDATA );
      
      objnum( n );
      
      objsymbol( '_', name );
    }
  
  datasize += n;

  return;

} /* startdata */

/*}}}*/
/*{{{  startcodetable() */

static void
startcodetable(
	       int32	n,
	       char *	name )
{
  if (objstream)
    {
      objbyte( split_module_table ? OBJCODETABLE : OBJDATA );
      
      if (!split_module_table)
	objnum( n );
      
      objsymbol( '_', name );
    }
  
  maxcodep += n;

  return;
  
} /* startcodetable */

/*}}}*/
/*{{{  objlabel() */

static void
objlabel(
	 char	prefix,
	 char *	name )
{
  if (objstream)
    {
      /*
       * This guard seems necessary since objlabel is called from obj_symref
       * which itself gets called even when an object file is not needed.
       */
      
      objbyte( OBJLABEL );
      
      objsymbol( prefix, name );
    }

  return;
  
} /* objlabel */

/*}}}*/
/*{{{  obj_outcode() */

/* Since an object directive might take us over the end of     */
/* the buffer we are actually outputting this routine returns  */
/* the number of bytes it has encroached into the next buffer. */

static int32
obj_outcode(
	    char *		buff,
	    CodeFlag_t *	flag,
	    int32 		nbytes,
	    int32		segbase )
{
  static bool	inited  = FALSE;
  static long	big_end = FALSE;
  int32 	i;
  int32 	b = 0;
  int32		n;

  
  IGNORE( flag );               /* i.e. no cross-sex compilation */

  if (!inited)
    {
      int	a = 0x12345678;

	  
      if (((char *)&a)[ 0 ] == 0x12)
	{
	  big_end = TRUE;
	}

      inited = TRUE;
    }

  for (b = 0; b < nbytes; b += n)
    {
      bool	xrflag = 0;
      int32	xrtype = 0;
      int32	xcode  = 0;


      if (codexrefs)
        {
	  int32		x = codexrefs->codexroff & 0x00ffffffU;


	  if (x < segbase + nbytes)
            {
	      xrflag = 1;
	      xrtype = codexrefs->codexroff & 0xff000000U;
	      n      = x - segbase - b;
            }
	  else
	    {
	      n = nbytes - b;
	    }
        }
      else
	{
	  n = nbytes - b;
	}
      
      startcode( n );

      if (big_end)
	{
	  for (i = b; i < (b + n); i++)
	    {
	      objbyte( buff[ (i & ~3) + (3 - (i & 3)) ] );
	    }

	  xcode  =  (int32)buff[ i++ ] << 24;
	  xcode |= ((int32)buff[ i++ ] << 16) & 0x00ff0000;
	  xcode |= ((int32)buff[ i++ ] << 8)  & 0x0000ff00;
	  xcode |= ((int32)buff[ i++ ])       & 0x000000ff;
	}
      else
	{
	  /* little endian host */
	  
	  for (i = b; i < (b + n); i++)
	    objbyte( buff[ i ] );

	  xcode  = buff[ i++ ];
	  xcode |= (int32)buff[ i++ ] << 8;
	  xcode |= (int32)buff[ i++ ] << 16;
	  xcode |= (int32)buff[ i++ ] << 24;
	}
      
      if (xrflag)
        {
	  CodeXref *	c = codexrefs;


	  codexrefs = c->codexrcdr;

	  switch (xrtype)
            {
	    case X_absreloc:
	      xcode += c->codexrcode;
	      break;
	      
	    case X_DataSymb:
	    case X_PCreloc:
	    case X_PCreloc2:
	      break;

	    default:
	      syserr( heliobj_unknown_ref, xrtype );

	      break;
	      
	    case X_DataModule1:
	    case X_DataModule2:
	    case X_DataModule3:
	    case X_DataModule4:
	    case X_DataModule5:
	    case X_DataSymbHi:
	    case X_DataSymbLo:
	    case X_DataAddr1:
	    case X_DataAddr:
	    case X_FuncAddr:
	      
#ifdef TARGET_HAS_DEBUGGER
	    case X_Debug_Modnum:
	    case X_Debug_Offset:
	    case X_Debug_Ref:
#endif
	    case X_DataModule:
	    case X_Modnum:
	      break;
            }

	  n += objdirective( xrtype, c->codexrsym, xcode );
        }
    }

  return (b - nbytes);

} /* obj_outcode */

/*}}}*/
/*{{{  obj_codewrite() */

void
obj_codewrite( Symstr * name )
{
  int32 	i       = 0;
  int32 	overrun = 0;
  int32 	segbase = codebase;

  
  if (codep == 0)
    {
      return;
    }  

  if (codexrefs != NULL)
    {
      /* Odd how the list ALL gets reversed every routine? */
  
      codexrefs = (CodeXref *)dreverse( (List *)codexrefs );
    }
      
  while ((codep / sizeof (int)) - CODEVECSEGSIZE * i > CODEVECSEGSIZE)
    {
      overrun = obj_outcode( (char *)code_instvec_( i ) + overrun, code_flagvec_( i ),
                               CODEVECSEGSIZE * sizeof (int) - overrun, segbase );
      
      segbase += CODEVECSEGSIZE * sizeof (int) + overrun;
      
      i++;
    }
  
  obj_outcode( (char *)code_instvec_( i ) + overrun, code_flagvec_( i ),
                (codep - CODEVECSEGSIZE * i * sizeof (int)) - overrun, segbase );
  
  codesize += codep;

#if defined NEVER
  /*
   * XXX
   *
   * one day we would like to align the start of
   * functions (and the initialisation code) to 128
   * byte boundaries to optimize the use of the 'C40's
   * op code cache.  Unfortuanetly this would upset the
   * compiler's calculation of intra function offsets
   * so that these would have to be recoded.
   */
  
  if (name == NULL)
    {
      int32	x = codesize;

      
      /* ensure that code is padded to a 128 byte boundary */
      
      x &= (128 - 1);

      if (x)
	{
	  x = 128 - x;
	  
	  objbyte( OBJBSS );
      
	  objnum( x );

	  codesize += x;
	}
    }
#endif /* NEVER */
  
  return;

  use( name );
  
} /* obj_codewrite */

/*}}}*/
/*{{{  request_stub() */

void
request_stub( Symstr * name )
{
  Stub *	s;

  
  for (s = stublist; (s != NULL); s = s->stubcdr)
    {
      if (s->stubsym == name)
	return;
    }
  
  stublist = (Stub *)global_cons2( SU_Other, stublist, name );

  return;
  
} /* request_stub */

/*}}}*/
/*{{{  show_stubs() */

static void
show_stubs( void )
{
  Stub *	s;
  VRegInt 	vr1, vr2, vm;


  if (!new_stubs)
    {
      asmf( ";\n; Function Stubs\n" );
  
      in_stubs = 1;

      codebuf_reinit2();
  
      localcg_reinit();
  
      vr1.r = vr2.r = GAP; vm.i = 0;
  
      procflags = 0;
  
      show_instruction( J_ENTER, vr1, vr2, vm );
    }
  
  for (s = stublist; s != NULL; s = s->stubcdr)
   {
     Symstr *	sym = s->stubsym;


     if (!is_defined( symext_( sym ) ))
       {
	 if (debugging( DEBUG_OBJ ))
	   cc_msg( "Stub for $r\n", s );

	 if (new_stubs)
	   {
	     /*
	      * If 'sym' is not defined locally then make it global
	      */
	     
	     request_new_stub( sym );
	   }
	 else
	   {
	     /*
	      * CGS: Changed from show_entry because that makes the symbol defined.
	      * I do not want it defined because when we come to do the init sequence
	      * it will think that the label is defined and would put a pointer
	      * to the stub into the data area. (viz. X problem).
	      * All we do is define the label to the linker and let that worry about
	      * patching up the forward references from the code.
	      */
#if 0
	     show_entry( sym, xr_code | xr_defloc ); /* really a call to obj_symref */
#else
	     if (new_stubs)
	       {
		 objlabel( '.', symname_( sym ) );
	       }
	     else
	       {
		 objlabel( '#', symname_( sym ) );

		 /*
		  * force a reference to the .<name> so that the
		  * relevant scanned library will be linked in
		  */
		 
		 objbyte( OBJREF );
      
		 objsymbol( '.', symname_( sym ) );
	       }
#endif
	     
	     /*
	      * Note that the two values passed in the r1 and r2 fields here are a
	      * bit suspect, but something is certainly needed...
	      */
	     
	     /* Beware the sym on the next line -- it probably needs to be a      */
	     /* slighly different Symstr.                                         */
	     
	     vr1.rr = R_A1;
	     vr2.i  = 0;
	     vm.sym = sym;
	     
	     /*
	      * indicate that the following instruction marks the
	      * end of the stub, and so the NOPs that follow it
	      * are candidates for removal by the linker
	      * (This code is all to do with generating
	      * calling stubs, see load_address_constant() for
	      * more information)
	      */
	     
	     show_instruction( J_TAILCALLK, vr1, vr2, vm );
	     
	     vr1.r = vr2.r = GAP;
	     vm.i  = 0;
	     
	     show_instruction( J_ENDPROC, vr1, vr2, vm );
	     
	     show_code( sym );
	     
	     asm_lablist = NULL;
	   }
       }
   }
  
  in_stubs = 0;

  return;

} /* show_stubs */

/*}}}*/
/*{{{  export_routines() */

static void
export_routines( void )
{
  ExtRef *	x;

  
  for (x = obj_symlist; x != NULL; x = x->extcdr )
    {
      if (is_external_code( x ))
	{
	  if (objstream)
	    {
	      globalise( x->extsym );
	      
	      startcodetable( 4, symname_( x->extsym ) );
	    }

	  export_function( x->extsym, R_ATMP );
	}
    }

  return;
  
} /* export_routines */

/*}}}*/
/*{{{  output_symbol() */

static void
output_symbol(
	      Symstr *	sym,
	      int32	size )
{
  if (objstream)
    {
      if (sym != NULL)
        {
	  if (is_global( symext_( sym ) ))
	    globalise( sym );
        }
    }

  /* AM: NULL changed to "" next line Apr 89.  Why is sym ever 0? */

  startdata( size, sym ? symname_( sym ) : "" );

  return;
  
} /* output_symbol */

/*}}}*/
/*{{{  show_init_entry() */

static LabelNumber *	datainitlab;


static void
show_init_entry( void )
{
  VRegInt	vr1, vr2, vm;


  datainitlab = nextlabel();

  /* XXX - turn off stack checking and back tracing in initialisation rouitnes */
  
  var_no_stack_checks   = 1;
  var_backtrace_enabled = 0;
  
  codebuf_reinit2();                  /* needed before show_inst etc. */
  
  localcg_reinit();
  
  if (objstream != NULL)
    {
      objbyte( OBJINIT );

      codesize += sizeof_int;      
    }
  
  asmf( ";\n; Data / Function Table Initialisation\n" );
  
  codebase += sizeof_int;

  /*
   * register usage not thought out here yet ...
   */

  regmask  = 0;            /* regbit(R_ADDR1); ??? */

  /* no registers used by init code ! */
  
  memclr( (VoidStar)&usedmaskvec, sizeof(RealRegSet) );

  /* no debugging for init code ! */
  
  usrdbgmask = 0;
  
  vr1.r = vr2.r = GAP;
  vm.i  = 0;
  
  procflags = 0;

  show_instruction( J_ENTER, vr1, vr2, vm );

  /*  show_instruction( J_MOVR, R_A2, GAP, R_DP ); (DP seem not to get corrupted) */
  
  if (split_module_table)
    {
      DataInit * p;


      /* see if we are going to have to initialise any data */
      
      for (p = datainitp; p != NULL; p = p->datacdr)
	if (p->sort != LIT_LABEL)
	  break;
      
      prepare_for_initialisation( p != NULL, datainitlab );
    }

  return;
  
} /* show_init_entry */

/*}}}*/
/*{{{  show_init_return() */

static void
show_init_return( void )
{
  VRegInt vr1, vr2, vm;


  /*  show_instruction( J_MOVR, R_DP, GAP, R_A2 );          */

  vr1.r = vr2.r = GAP;
  vm.l  = RETLAB;

  show_instruction( J_B | Q_AL, vr1, vr2, vm );
  
  return;
  
} /* show_init_return */

/*}}}*/

#ifdef NOT_USED
/*{{{  add_data_stubs() */

/* We also have to generate stubs for the variables which are      */
/* initialised to point to functions not defined in this module.   */

static void
add_data_stubs( void )
{
  DataInit *	p;

  
  for (p = datainitp; p != 0; p = p->datacdr)
    {
      if (p->sort == LIT_ADCON)
	{
	  Symstr *	sym = (Symstr *)p->len;
	  ExtRef *	x   = symext_( sym );

	  
	  if (is_function( sym ) && /* redo */
	      !is_defined( x ))
	    request_stub( sym );
	}
    }

  return;
  
} /* add_data_stubs */

/*}}}*/
#endif /* NOT_USED */
/*{{{  IEEE_to_signle_float() */

/*
 * converts a 32 bit IEEE format floating point value into
 * a C40 format 32 bit floating point value
 */

int32
IEEE_to_single_float( int32 val )
{
  int32	e;
  int32	f;
  int32	s;

  
  /* extract components */
  
  e = (val >> 23) & 0xff;
  s = val & (1U << 31);
  f = val & ((1U << 23) - 1);

  /* decode - this algorithm is taken from the TMS320C4x User's Guide, page 4-12 */
  
  if (e == 0xff)
    {
      if (s == 0)
	{
	  e = 0x7f;
	  /* s = 0; */
	  f = 0x7fffff;
	}
      else
	{
	  e = 0x7f;
	  s = 1;
	  f = 0;
	}
    }
  else if (e == 0)
    {
      e = 0x80;
      s = 0;
      f = 0;
    }
  else
    {
      if (s == 0)
	{
	  e -= 0x7f;
	  /* s = 0; */
	  /* f = f; */
	}
      else
	{
	  if (f == 0)
	    {
	      e -= 0x80;
	      s  = 1;
	      /* f  = 0; */
	    }
	  else
	    {
	      e -= 0x7f;
	      s  = 1;
	      f  = ((~f) + 1);
	    }
	}
    }
  
  return (e << 24) | (s << 23) | (f & ((1 << 23) - 1));
  
} /* IEEE_to_single_float */

/*}}}*/
/*{{{  IEEE_to_extended_float() */

/*
 * converts a 64 bit IEEE format number into a 40 bit C40 number
 * returns the most signifcant 32 bits of the result
 */

int32
IEEE_to_extended_float(
		       int32		high,		/* most  significant 32 bits of 64 bit IEEE number */
		       unsigned32	low,		/* least significant 32 bits of 64 bit IEEE number */
		       int32 *		low_return )  	/* least significant 32 bits of the result         */
{
  int32		e;
  unsigned32	f;
  unsigned32	s;


  /*
   * extract components
   *
   * IEEE double precision format is
   *
   *
   *    63 62          52 51                          0
   *    __ ______________ _____________________________
   *   |  |              |                             |
   *   |s | exponent     | mantissa                    |
   *   |__|______________|_____________________________|
   * or                              |
   *    31 30          20 19        0|31              0
   *                                 |
   *
   * C40 extended precision format is
   *
   *              39        32 31 30                 0
   *              ____________ __ ____________________
   *             |            |  |                    |
   *             | exponent   |s | mantissa           |
   *             |____________|__|____________________|
   *   
   */
  
  e = (high >> 20) & 0x7ff;
  s = high & (1U << 31);
  f = (high << (31 - 19)) | (low >> 20);

  /* debug( "converting IEEE number %x (high) %x (low), e = %x, s = %x, f = %x", high, low, e, s, f ); */
  
  /* decode - this algorithm is based on the one in the TMS320C4x User's Guide, page 4-12 */

  e -= 0x3ff;

  if (e > 127)
    {
      if (e < 0x3ff || f != 0xffffffffU)
	{
	  cc_warn( "floating point accuracy completely lost!" );
	}
		
      e = 0x7f;
	      
      if (s == 0)
	{
	  /* largest positive value */
	  
	  f = 0x7fffffffU;
	}
      else
	{
	  /* largest negative value */
	  
	  f = 0;
	}
    }
  else if (e == -1023)
    {
      e = 0x80;
      s = 0;
      f = 0;
    }
  else if (e < -128)
    {
      if (e != 0xfffff400U || f != 0x0U)
	{
	  cc_warn( "floating point accuracy completely lost! (e = 0x%lx, s = 0x%lx, f = 0x%lx)",
		  e + 0x3ff, s, f );
	}
		
      if (s == 0)
	{
	  /* smallest positive value */
	  
	  e = 0x81;	/* beware of 0.0 */
	  f = 0x0;
	}
      else
	{
	  /* smallest negative value */
	  
	  e = 0x80;
	  f = 0x7fffffffU;
	}
    }
  else
    {
      if (s != 0)
	{
	  if (f == 0)
	    {
	      e -= 1;
	    }
	  else
	    {
	      f = ((~f) + 1);
	    }
	}
    }

  /* debug( "after conversion, e = %x, s = %x, f = %x", e, s, f ); */
  
  if (low_return != NULL)
    {
      if (s == 0)
	{
	  /* round towards nearest */

	  if (!(++f))
	    ++e;
	  
	  *low_return = (f >> 1);
	}
      else
	{
	  /* already rounded when converting from two's complement */
	  
	  *low_return = (1U << 31) | (f >> 1);
	}
      
      /* debug( "low return = %x", *low_return ); */
    }

  if (s == 0)
    {
      return (e << 24) | ((f >> 9) & ((1 << 23) - 1));
    }
  else
    {
      return (e << 24) | (1 << 23) | ((f >> 9) & ((1 << 23) - 1));
    }
  
} /* IEEE_to_extended_float */

/*}}}*/
/*{{{  Local Variables */

static int32		skip	     = 0;
static int32		block_size   = 0;
static unsigned32	curdsize     = 0;
static unsigned32	dataslot     = 0;
static unsigned32	old_dataslot = 0;
static unsigned32	words_out    = 0;
static ExtRef *		dsymb        = NULL;
static ExtRef *		curdsymb     = NULL;

/*}}}*/
/*{{{  init_phase() */

/*
 * initialise the static variables, (and possibly
 * generate some code), at the start of a phase
 */

static void
init_phase(
	   int 		first_phase,	/* non-0 if this is the first phase */
	   ExtRef *	datasymbols,	/* pointer to the start of the list of data symbols */
	   bool		need_label )	/* non-0 if code will be generated, only valid during first phase */
{
  static LabelNumber *	l;

  
  if (first_phase)
    {
      if (need_label)
	{
	  l = nextlabel();
      
	  prepare_for_block_copying( l );
	}
      else
	{
	  l = NULL;
	}
      
      curdsize     = 0;
      dataslot	   = 0;
      old_dataslot = 0;

      dsymb        = datasymbols;
      curdsymb	   = NULL;
    }
  else
    {
      show_init_return();

      if (l)
	{
	  setlabel( l );

	  words_out = 0;
	}
      else
	{
	  words_out = 1;
	}
    }

  return;

} /* init_phase */

/*}}}*/
/*{{{  phase_export_block() */

/*
 * Export a block of data.  The size of the
 * block is 'block_size' words.  The block's
 * source address is held in AR0 and the
 * block's destination address is in AR5
 */

static void
phase_export_block( void )
{
  if (block_size != 0)
    {
      block_copy_data( block_size );

      curdsize += block_size;
      
      block_size = 0;
    }

  return;
  
} /* phase_export_block */
  

/*}}}*/
/*{{{  phase_skip_block() */

/*
 * Skip a block of zero data
 */

static void
phase_skip_block( void )
{
  if (skip)
    {
      VRegInt	vr1;
      VRegInt	vr2;
      VRegInt	vm;

	      
      vr1.rr = vr2.rr = R_ATMP;
      vm.i   = skip;
      
      show_instruction( J_ADDK, vr1, vr2, vm );

      curdsize += skip;
      
      skip = 0;
    }

  return;
  
} /* phase_skip_block */

/*}}}*/
/*{{{  phase_export_symbols() */

/*
 * perform whatever actions are appropriate
 * so that the exported symbols are registered
 * with the linker
 */

static void
phase_export_symbols(
		     int 	first_phase,
		     int32	length,
		     int32	repeat )
{
  /*
   * Generate directives to allocate space
   * and a label for an exported data symbol
   */
	      
  if (first_phase)
    {
      /* if we have any exported data left */

      if (dsymb != NULL)
	{
	  int32	dataoff = dsymb->extoffset;
		  

	  /* have we reached the location of the next symbol to be exported  ? */
	  
	  if (dataslot == dataoff)
	    {
	      /* do we have a previous symbol to export ? */
	      
	      if (curdsymb != NULL)
		{
		  /*
		   * Export the previous symbol.
		   *
		   * The size of the symbol is not its own size, but
		   * distance between where it is defined and where
		   * the current symbol is defined.  This is because
		   * strings in the code segment must be copied into
		   * the data segment, but they do not have labels
		   * associated with them, and so no symbol is created
		   * for them.  Thus the space used by strings is
		   * amalgamated into the space used by the previous
		   * data symbol.
		   */

		  output_symbol( curdsymb->extsym, dataslot - old_dataslot );
		}

	      /* export all but the last symbol with this offset */
	      
	      for (; dsymb->extcdr && dsymb->extcdr->extoffset == dataoff; dsymb = dsymb->extcdr )
		{
		  output_symbol( dsymb->extsym, 0 );
		}

	      /* remember the last symbol at this offset, and the size of the data segment at this point */
	      
	      old_dataslot = dataslot;
	      curdsymb     = dsymb;

	      /* point to the next symbol in the list */
	      
	      dsymb = dsymb->extcdr;

	      if (dsymb != NULL && dsymb->extoffset < dataoff)
		{
		  syserr( heliobj_conflicting_offsets,
			 symname_( dsymb->extsym ), dsymb->extoffset,
			 symname_( curdsymb->extsym ), curdsymb->extoffset );
		}	      
	    }
	  else if (dataoff < dataslot)
	    {
	      /* oh dear - we have mislaid a symbol */
#ifdef DEBUG
	      debug( "dataoff = %lx, dataslot = %lx", dataoff, dataslot );
#endif  
	      syserr( syserr_heliobj_dataseggen );
	    }
	}

      /* keep track of how much data has been generated */
      
      dataslot += length * repeat;
    }

  return;
  
} /* phase_export_symbols */
  

/*}}}*/
/*{{{  phase_store() */

/*
 * perform whatever actions are appropriate for
 * storing the word 'value', 'repeat' times into
 * the data segment
 */

static void
phase_store(
	    int			first_phase,	/* non-0 iff this is the first phase 			*/
	    unsigned32		value,		/* the word to be stored in the data area 		*/
	    int32		repeat )	/* the number of times this word is to be stored	*/
{
  /* possibly export data symbols */
	      
  phase_export_symbols( first_phase, sizeof_long, repeat );
	      
  if (first_phase)
    {
      /* generate code to do the initialisation */

      if (value == 0)
	{
	  if (block_size != 0)
	    {
	      /*
	       * we have come to the end of a block of non-zero data,
	       * so generate the block
	       */
	      
	      phase_export_block();
	    }
	  	  
	  /* data area is pre-initialised to zero, so just skip over it */

	  skip += repeat;
	}
      else
	{
	  if (skip)
	    {
	      /*
	       * we are about to output some code, so adjust the destination
	       * pointer to allow for the blanks skipped
	       */

	      phase_skip_block();
	    }
	  
	  /*
	   * count the number of words to emit, and generate the code
	   * at the end of the block
	   */

	  block_size += repeat;
	}
    }
  else
    {
      if (value != 0)
	{
	  words_out = 1;
	  
	  while (repeat--)
	    {
	      outcodeword( value, LIT_NUMBER );
	    }
	}
    }

  return;
  
} /* phase_store */

/*}}}*/
/*{{{  phase_store_address_constant() */

/*
 * perform whatever actions are appropriate for
 * storing an address constant into the data segment
 */

static void
phase_store_address_constant(
			     int	first_phase,	/* non-0 if this is the first phase		   */
			     Symstr *	symbol,		/* the symbols whoes address is to be stored	   */
			     int32	offset )	/* the offset to be added to this symbol's address */
{
  /* possibly export data symbols */
  
  phase_export_symbols( first_phase, sizeof_ptr, 1 );
	      
  if (first_phase)
    {
      if (skip)
	{
	  phase_skip_block();
	}
      
      if (block_size)
	{
	  phase_export_block();
	}

      export_data_symbol( symbol, offset, curdsize );

      curdsize++;
    }
  else
    {
      /* pretend we are storing 0, so that non-0 blocks are flushed */
      
      phase_store( first_phase, 0, 1 );
    }  

  return;
  
} /* phase_store_address_constant */

/*}}}*/
/*{{{  finish_phase() */

static void
finish_phase(
	     int 	first_phase,	/* non-0 if this the end of the first phase */
	     bool	need_label )	/* non-0 if a label was needed */
{
  if (first_phase)
    {
      /* ignore any trailing skip */

      skip = 0;

      /* copy the last block, if there is one */
      
      if (block_size)
	{
	  phase_export_block();
	}

      /* if there is a remaining data symbol to export then do so */
      
      if (curdsymb)
	{
	  output_symbol( curdsymb->extsym, dataslot - old_dataslot );
	}
    }
  else
    {
      if (need_label && words_out == 0)
	{
	  /*
	   * we have generated a label, but we have
	   * not actually put any code to follow this
	   * label!  Hence we emit a dummy value, just
	   * to keep the object code formatter happy
	   */
	  
	  outcodeword( 0, LIT_NUMBER );
	}
    }  
  
  return;
  
} /* finish_phase */
  

/*}}}*/
/*{{{  insert_dlist() */

/*
 * inserts the cross refence for the given data item into
 * the data list, maintaining the ordering of the list.
 */

static void
insert_dlist( ExtRef * x )
{
  ExtRef *	ptr;
  ExtRef *	prev = NULL;
  

  prev = NULL;
  
  for (ptr = datasymbols; ptr != NULL; ptr = ptr->extcdr)
    {
      if (ptr->extoffset > x->extoffset)
	{
	  /* insert before ptr */

	  if (prev == NULL)
	    {
	      x->extcdr = datasymbols->extcdr;
	      
	      datasymbols = x;
	    }
	  else
	    {
	      prev->extcdr = x;
	      x->extcdr    = ptr;
	    }
	  
	  return;	  
	}

      prev = ptr;
    }

  /* append to end of list */
  
  if (prev == NULL)
    {
      datasymbols = x;
    }
  else
    {
      prev->extcdr = x;
    }

  x->extcdr = NULL;
  
  return;
  
} /* insert_dlist */
  

/*}}}*/
/*{{{  export_functions() */

static void
export_functions( void )
{
  ExtRef *	x;


  if (objstream == NULL)
    return;
  
  for (x = obj_symlist; x != NULL; x = x->extcdr )
    {
      if (is_external_code( x ))
	{
	  globalise( x->extsym );

	  if (!new_stubs || usrdbg( DBG_ANY ))
	    startcodetable( 4, symname_( x->extsym ) );
	}
    }

  return;
  
} /* export_functions */

/*}}}*/
/*{{{  dumpdata() */

/*
 * generate code to initialise the module's static data area
 */

static void
dumpdata( void )
{
  static bool	inited  = FALSE;
  static long	big_end = FALSE;
#if 0
  ExtRef **	dsend;
#endif
  DataInit *	p;
  int		phase;
  bool		need_label;
  

  if (datainitp == NULL && suppress_module)
    return;

  if (!inited)
    {
      int	a = 0x12345678;

	  
      if (((char *)&a)[ 0 ] == 0x12)
	{
	  big_end = TRUE;
	}

      inited = TRUE;
    }

  /* With any luck the symbols should be in obj_symlist in increasing */
  /* order so all I have to do is fetch the data symbols which are    */
  /* in obj_symlist and append them onto datasymbols to get them in   */
  /* the right order   (ascending order of extoffset)                 */
  
  /*
   * XXX - NC - 8/6/92
   *
   * Unfortunatelty this is not true.
   *
   * What can happen is that a symbol (X) is declared as external,
   * so it gains a place in the obj_symlist, with an offset of zero.
   * Other, normal symbols (Y, Z), are then declared, so they gain later
   * places in obj_symlist with increasing (non-zero) offsets.  Then the
   * external symbol (X) is in fact declared as being defined in the
   * current module!.  (Bad C practice, but legal).  This then gives
   * X a non-zero offset which is greater than Y or Z's, but with X
   * being declared before Y or Z in the obj_symlist.
   */

    {
      ExtRef **	flist;
      ExtRef *	x;
      
      
      datasymbols = NULL;
      flist       = &obj_symlist;

      /* scan linked list of symbols */
      
      for (x = obj_symlist; x != NULL;)
	{
	  ExtRef *	succ;

	  
	  succ = x->extcdr;
	  
	  if (is_data( x ) && is_defined( x ))
	    {
	      /* insert data items into data list */
	      
	      insert_dlist( x );	      
	    }
	  else
	    {
	      /* append function names to function list (these are always sorted) */
	      
	      *flist = x;
	      flist  = &x->extcdr;
	    }

	  x = succ;	  
	}
    }
  
  /* At this point we generate the code to initialise the static data */
  /* section. */

  codebuf_reinit2();
  
  show_init_entry();

  if (new_stubs && !usrdbg( DBG_ANY ))
    {
      export_functions();
    }
  else
    {
      if (!suppress_module)
	{
	  prepare_for_function_exporting();

	  /* generate a label for function address calculation */

	  if (asmstream != NULL)
	    {
	      flush_peepholer( DBG( "generating asm label" ) );
	      
	      exporting_routines = nextlabel();
	      
	      setlabel( exporting_routines );
	    }
	  
	  load_static_data_ptr( R_ATMP, split_module_table, NULL );

	  export_routines();
	}

      /*
       * XXX - beware of assumption of split_module_table == TRUE here
       */
  
      if (!suppress_module)
	{
	  VRegInt	vr1;
	  VRegInt	vr2;
	  VRegInt	vm;

      
	  vr1.r = vr2.r = GAP;
	  vm.l  = RETLAB;
      
	  show_instruction( J_B | Q_AL, vr1, vr2, vm );
	}
    }

  /*
   * scan the data list to see if we are going to have to produce
   * code for data initialisation
   */

  need_label = FALSE;
  phase      = -1;	/* flag used to inidicate if any data needs initialisation */
  
  for (p = datainitp; p != NULL; p = p->datacdr)
    {
      if (p->sort == LIT_LABEL)
	continue;

      phase = 0;	/* we are going to initialise data */
      
      if (p->sort == LIT_ADCON)
	continue;

      if (p->sort == LIT_FPNUM)
	{
	  if (p->len == sizeof_float && ((FloatCon *)p->val)->floatbin.fb.val == 0)
	    continue;

	  if (p->len == sizeof_double 		         &&
	      ((FloatCon *)p->val)->floatbin.db.msd == 0 &&
	      ((FloatCon *)p->val)->floatbin.db.lsd == 0  )
	    continue;

	  need_label = TRUE;
	  
	  break;
	}

      if (p->val != 0)
	{
	  need_label = TRUE;
	  
	  break;
	}
    }
      
  if (phase == 0)
    {
      setlabel( datainitlab );
      
      load_static_data_ptr( R_ATMP, NO, NULL );
	  
      prepare_for_data_exporting( R_ATMP );
  
      /*
       * scan the data list twice
       *
       * The first scan (or phase) generates code to block copy the data
       * from the code segment into the data segment, and to calculate
       * address constants.  The second phase generates word directives
       * to insert the data into the code segment.
       */
      
      for (phase = 2; phase--; )
	{
	  unsigned32	val;
	  
	  
	  init_phase( phase, datasymbols, need_label );
	  
	  if (big_end)
	    {
	      for (p = datainitp; p != NULL; p = p->datacdr)
		{
		  switch (p->sort)
		    {
		    case LIT_LABEL:
		      break;
		      
		    case LIT_ADCON:
		      if (p->rpt != 1)
			syserr( heliobj_repeated_export );
		      
		      phase_store_address_constant( phase, (Symstr *)p->len, p->val );
		      break;
		      
		    case LIT_BBBB:
		      val = p->val;
		      
		      val = ((val <<  8) & 0x00FF0000U) |
		        ((val >>  8) & 0x0000FF00U) |
			  (val >> 24)                |
			    (val << 24)                ;
		      
		      phase_store( phase, val, p->rpt );
		      
		      break;
		      
		    case LIT_BBH:
		      val = p->val;
		      phase_store( phase, (val << 16) | (val >> 24) | ((val >>  8) & 0x0000FF00U), p->rpt );
		      break;
		      
		    case LIT_HBB:
		      val = p->val;
		      phase_store( phase, (val >> 16) | (val << 24) | ((val <<  8) & 0x00FF0000U), p->rpt );
		      break;
		      
		    case LIT_HH:
		      val = p->val;		      
		      phase_store( phase, (val << 16) | (val >> 16), p->rpt );
		      break;
		      
		    case LIT_NUMBER:
		      phase_store( phase, p->val, p->rpt );
		      break;
		      
		    case LIT_FPNUM:
		      switch (p->len)
			{
			case sizeof_float:
			  phase_store( phase,
				      IEEE_to_single_float( ((FloatCon *)p->val)->floatbin.fb.val ),
				      p->rpt );
			  break;
			  
			case sizeof_double:		      
			  phase_store( phase,
				      IEEE_to_extended_float(
							     ((FloatCon *)p->val)->floatbin.db.msd,
							     ((FloatCon *)p->val)->floatbin.db.lsd,
							     (int32 *)&val ),
				      p->rpt );
			  phase_store( phase, val, p->rpt );
			  break;
			  
			default:
			  syserr( heliobj_bad_sized_FP, p->len );
			  break;
			}
		      
		      break;
		      
		    default:
		      syserr( heliobj_unknown_data_type, p->sort );
		      break;
		    }
		}
	    }
	  else
	    {
	      for (p = datainitp; p != NULL; p = p->datacdr)
		{
		  switch (p->sort)
		    {
		    case LIT_LABEL:
		      break;
		      
		    case LIT_ADCON:
		      if (p->rpt != 1)
			syserr( heliobj_repeated_export );
		      
		      phase_store_address_constant( phase, (Symstr *)p->len, p->val );
		      break;
		      
		    case LIT_BBBB:
		    case LIT_BBH:
		    case LIT_HBB:
		    case LIT_HH:
		      if (p->len != sizeof_long)
			syserr( heliobj_bad_length, p->len );
		      
		      phase_store( phase, p->val, p->rpt );
		      break;
		      
		    case LIT_NUMBER:
		      phase_store( phase, p->val, p->rpt );
		      break;
		      
		    case LIT_FPNUM:
		      switch (p->len)
			{
			case sizeof_float:
			  phase_store( phase,
				      IEEE_to_single_float( ((FloatCon *)p->val)->floatbin.fb.val ),
				      p->rpt );
			  break;
			  
			case sizeof_double:		      
			  phase_store( phase,
				      IEEE_to_extended_float(
							     ((FloatCon *)p->val)->floatbin.db.msd,
							     ((FloatCon *)p->val)->floatbin.db.lsd,
							     (int32 *)&val ),
				      p->rpt );
			  phase_store( phase, val, p->rpt );
			  break;
			  
			default:
			  syserr( heliobj_bad_sized_FP, p->len );
			  break;
			}
		      break;
		      
		    default:
		      syserr( heliobj_unknown_data_type, p->sort );
		      break;
		    }
		}
	    }
	  
	  finish_phase( phase, need_label );
	}
    }
  
  finished_exporting();

  return;

} /* dumpdata */

/*}}}*/
/*{{{  align() */

static void
align( void )
{
  int32		x = (int32)(sizeof_int - (codesize & (sizeof_int - 1)));
  

  /* Number of "bytes" to get to next multiple of word */

  if (x != sizeof_int) 
   {
     objbyte( OBJBSS );
     
     objnum( x );
     
     codesize += x;
   }

  return;
  
} /* align */

/*}}}*/
/*{{{  obj_symref() */

int32
obj_symref(
	   Symstr *	s,
	   int 		flags,
	   int32 	loc )
{
  ExtRef *		x;


  if ((x = symext_( s )) == NULL)     /* if not already defined */
    {
      x = (ExtRef *)GlobAlloc( SU_Other, sizeof (ExtRef) );

      
      x->extcdr       = NULL;
      x->extsym       = s,
      x->extindex     = obj_symcount++,
      x->extflags     = 0,
      x->extoffset    = 0;
      
      *obj_symlistend = symext_( s ) = x;
      obj_symlistend  = &x->extcdr;
    }
  
  /*
   * It is critical that for Helios when I define a symbol the stub that
   * had (maybe) been set up for it gets deleted.  The syserr here can
   * be made recoverable with -zqz
   */

  if (is_defined_( flags ) &&
      is_defined( x ))
    {
      /*
       * XXX - NC - made into a serious error becuase can happen if programmer
       * created two functions with the same name.  This is reported by the
       * compiler, and should not really generate a fatal error.
       */
	 
      cc_err( syserr_heliobj_2def, s );

      x->extflags &= ~xr_definition;
    }

  /* The next two lines cope with further ramifications of the abolition of */
  /* xr_refcode/refdata in favour of xr_code/data without xr_defloc/defext  */
  /* qualification.  This reduces the number of bits, but needs more        */
  /* checking in that a symbol defined as data, and then called via         */
  /* casting to a code pointer may acquire defloc+data and then get         */
  /* xr_code or'ed in.  Suffice it to say this causes confusion.            */
  /* AM wonders if gen.c ought to be more careful instead.                  */

  if (is_defined_( flags ))
    x->extflags &= ~(xr_code | xr_data);
  
  if (is_defined( x ))
    flags &= ~(xr_code | xr_data);

  /* end of fix.                                                             */
  
  x->extflags |= flags;
  
  if (is_defined_( flags ))
    {
      x->extoffset = loc;

      if (is_code( x ))          /* @@@ flags ...? */
	{
	  /* the setting of xr_defloc/defext marks sym as implicitly     */
	  /* deleted from stublist.                                      */

 	  objlabel( new_stubs ? '.' : '#', symname_( s ) );

	  /* ensure that there is a .main that can be found */
	  
	  if (s == mainsym && !new_stubs)
	    objlabel( '.', symname_( s ) );
	}
    }
  else if ((loc > 0) && !is_code_( flags ) && !is_defined( x ))
    {
      /* common data, not already defined */
      
      x->extoffset = loc;
    }

  /* The next line returns the offset of a function in the codesegment */
  /* if it has been previously defined -- this saves store on the arm  */
  /* and allows short branches on other machines.  Otherwise it        */
  /* returns -1 for undefined objects or data objects.                 */

  return (is_defined( x ) && is_code( x ) ? x->extoffset : -1);

} /* obj_symref */

/*}}}*/
/*{{{  obj_init() */

void
obj_init( void )
{
  obj_symcount      = 0;
  obj_symlist       = NULL;
  obj_symlistend    = &obj_symlist;
  dataxrefs         = NULL;
  codexrefs         = NULL;
  global_symbols    = NULL;
  stub_symbols      = NULL;
  addr_stub_symbols = NULL;
  stublist          = NULL;
  in_stubs          = 0;
  
  return;
  
} /* obj_init */

/*}}}*/
/*{{{  obj_header() */

void
obj_header( void )
{
#ifdef TARGET_HAS_DEBUGGER
  if (usrdbg( DBG_ANY ))
    db_init( sourcefile );
#endif
  
  if (!suppress_module)
    {
      char	sname[ 40 ];
      char *	ptr;      
      int 	len;


      if ((ptr = strrchr( sourcefile, '/' )) != NULL)	/* skip past path components in source name */
	++ptr;
      else
	ptr = sourcefile;
      
      len = strlen( ptr ) - 31;
      
      if (len < 0)
	len = 0;
      
      strcpy( sname, ptr + len );
      
      len = strlen( sname );

      objbyte( OBJMODULE );
      
      objnum( -1 );

      startcode( (int32)offsetof( Module, Name ) + len );
      
      objword( T_Module );

      codesizepos = ftell( objstream );
      
      objword( 0 );

      fputs( sname, objstream );

      objbyte( OBJBSS );
      
      objnum( 32L - len );

      objbyte( OBJWORD );
      
      objbyte( OBJMODNUM );

      startcode( offsetof( Module, Init ) - offsetof( Module, Version ) );

	{
	  int ver;

	  
	  /*
	   * The version number here is a bit dodgy, but I need a single number
	   * and this may be as good as any other that I can invent.
	   */

	  sscanf( MIP_VERSION, "%d", &ver );
	  
	  objword( (int32)ver );
	}
      
      datasizepos = ftell( objstream );
      
      objword( 0 );

      objbyte( OBJINIT );
      
      codesize = sizeof (Module);

      if (split_module_table)
	{
	  startcode( 4 );
	
	  maxcodeppos = ftell( objstream );
	
	  objword( 0 );	  
	}
      else
	{
	  codesize -= sizeof (int32);
	}      
    }

  return;
  
} /* obj_header */

/*}}}*/
/*{{{  obj_makestubs() */

void
obj_makestubs( void )          /* maybe this fn is part of flowgraf.c? */
{
  if (!suppress_module)
    {
      procflags = 0;          /* certainly these are owned elsewhere  */
      regmask   = 0;          /* I think the solution is connected to */
                              /* the fact that obj_symref is NOT      */
      /*      add_data_stubs();*/       /* really part of xxxobj.c              */
      
      show_stubs();
      
      /* AM: I now think that for each helios extern symbol we should have  */
      /* two Symstr's.  (Either with the same name by avoiding hashing or   */
      /* by prefixing the real extern with '.' or such like, which asm and  */
      /* obj then remove to make the world sweet.)                          */
      /* This forgeing at the JOPCODE level seems a bit fraught.            */
    }

  if (suppress_module != 1)
    {
      padstatic( 4 );       /* @@@ flush vg_wbuff, beware call in cg.c  */
                            /* @@@ rationalise soon.                    */
      dumpdata();

      asmf( ";       align 128\n" );
      asmf( "        init\n" );
      
      show_code( NULL );

      asm_lablist = NULL;
    }

  return;
  
} /* obj_makestubs */

/*}}}*/
/*{{{  obj_trailer() */

void
obj_trailer( void )
{
#ifdef TARGET_HAS_DEBUGGER
  if (usrdbg( DBG_ANY ))
    db_tidy();
#endif
  
  if (!suppress_module)
   {
     align();

     dumpglobals();

     if (new_stubs)
       dump_new_stubs();
     
     if (fseek( objstream, codesizepos, SEEK_SET ) == -1)
       {
	 cc_err( "Failed to seek to start of file to set code size, errno = %d", errno );

	 return;	 
       }

     objword( codesize );

     if (fseek( objstream, datasizepos, SEEK_SET ) == -1)
       {
	 cc_err( "Failed to seek to start of file to set data size, errno = %d", errno );

	 return;	 
       }     
     
     if (split_module_table)
       {
	 objword( datasize );

	 if (!new_stubs || usrdbg( DBG_ANY ))
	   {
	     if (fseek( objstream, maxcodeppos, SEEK_SET ) == -1)
	       {
		 cc_err( "Failed to seek to start of file to set max code size, errno = %d", errno );

		 return;
	       }

	     objword( maxcodep );
	   }	 
       }
     else
       {
	 objword( datasize + maxcodep );
       }
   }

  return;
  
} /* obj_trailer */

/*}}}*/

/*}}}*/

