/*{{{  Comments */

/****************************************************************/
/* Helios Linker					     	*/
/*								*/
/* File: genimage.c 						*/
/*                                                              */
/* Copyright (c) 1988, 1989, 1990, 1991, 1992, 1993, 1994 to	*/
/*               Perihelion Software Ltd.			*/
/*                                                              */
/* Author: NHG 26/9/88                                          */
/*                                                              */
/* Updates: CS/PAB                                              */
/*      Shift patch						*/
/*	SWAP patch                                              */
/*	ARM patches						*/
/*	Recursive patches					*/
/*         NC                                                   */
/*      'C40 patches                                            */
/*                                                              */
/****************************************************************/
/* RcsId: $Id: genimage.c,v 1.53 1994/04/07 10:56:11 nickc Exp $ */

/*}}}*/
/*{{{  #includes */

#include <module.h>
#include <unistd.h>	/* for write() */
#include "link.h"

/*}}}*/
/*{{{  #defines */

#define trace if (traceflags & db_genimage) _trace

#define fits_in_16_bits( val )		(((val) & 0xffff8000U) == 0 || ((val) & 0xffff8000U) == 0xffff8000U)
#define fits_in_16_bits_unsigned( val )	(((val) & 0xffff0000U) == 0 )

#ifdef __C40
#define fits_in_8_bits( val )		(((val) & 0xffffff80U) == 0 || ((val) & 0xffffff80U) == 0xffffff80U)
#define fits_in_24_bits( val )		(((val) & 0xff800000U) == 0 || ((val) & 0xff800000U) == 0xff800000U)
#endif

/*}}}*/
/*{{{  Forward references */

#ifdef __STDC__
word 		patchit( word vtype, long value, word size );
void 		putpatch( Code * c );
word 		mcpatch( word type, VMRef v, word size );
word 		stdpatch( word type, long value );
static void 	outbyte( UBYTE );
static void 	outbyte1( WORD );
static void 	imageheader( WORD );
long 		swapw( long x );
unsigned 	swaps( unsigned x );
void 		flush_output_buffer(void);
#else
word 		patchit();
void 		putpatch();
word 		mcpatch();
word 		stdpatch();
static void 	outbyte();
static void 	outbyte1();
static void 	imageheader();
long 		swapw();
unsigned	swaps();
void 		flush_output_buffer();
#endif

/*}}}*/
/*{{{  Variables */

word 		codepos;

extern FILE *	verfd;

word		nchars;

#ifdef __ARM
static word 	restofdppatch;		/* rest of value to be pathed by armdprest */
static word 	patchNeg;		/* if patch value need a sub rather than an add instr. */
static bool	AOFSymbolIsData;
#endif

#define		BUFFER_SIZE	4096

UBYTE		output_buffer[BUFFER_SIZE];	/* buffered output */
int		output_buffer_pointer = 0;	/* and pointer */

/*}}}*/
/*{{{  Functions */

/*{{{  genimage() */

void
#ifdef __STDC__
genimage( void )
#else
genimage()
#endif
{
  Code *	c;
  asm_Module *	m;
  VMRef		curblock;


  curmod   = module0;
  m        = VMAddr( asm_Module, curmod );
  curblock = m->start;
  
  VMlock( curblock );
   
  c = VMAddr( Code, curblock );
  
  VMDirty( curblock );
   
  codepos = 0;

  imageheader( totalcodesize + sizeof (int) );
      
  for (;;)
    {
      word	tag = c->type;

      
      /* trace( "image op %#x", tag ); */
      
      switch (tag)
	{
	case OBJNEWSEG:	/* go to new code page            */
	  
	  trace( "NEWSEG %#x", c->value.v );
	  
	  VMunlock( curblock );
	  
	  curblock = c->value.v;
	  
	  VMlock( curblock );
	  
	  c = VMAddr( Code, curblock );
	  
	  VMDirty( curblock );
	  
	  continue;
         
	  
      case OBJEND: 	/* start new module            */
	  
         VMunlock( curblock );
	  
         m = VMAddr( asm_Module, curmod );
	  
         curmod = m->next;
	  
         if (NullRef( curmod ))
	   {
	     if (gen_image_header)
		/* only generate tailer if we gen header */
		outword( 0L );

	     flush_output_buffer();

	     return;
	   }

	  m = VMAddr( asm_Module, curmod );

	  curblock = m->start;
	  
	  VMlock( curblock );
	  
	  c = VMAddr( Code, curblock );
	  
	  VMDirty( curblock );
	  
	  codepos = 0;

	  continue;

	case OBJMODULE:
	case OBJDATA:
	case OBJCOMMON:
	case OBJCODETABLE:
	  break;
      
	case OBJCODE:
	    {
	      int	i;
	      int	size = c->size;
	      UBYTE *	v    = VMAddr( UBYTE, c->value.v );
	
	      
	      for (i = 0; i < size ; outbyte( v[ i++ ] ))
		;
	      
	      codepos += size;

	      break;
	    }
            
	case OBJBSS:
	    {
	      int	i;
	      int	size = c->size;

	      
	      for (i = 0; i < size ; outbyte( 0 ))
		;
	      
	      codepos += size;

	      break;
	    }
      
	case OBJLITERAL:
	    {
	      int	i;
	      int	size = c->size;
	      UBYTE *	v    = (UBYTE *)&c->value.w;

	      
	      for (i = 0; i < size ; outbyte( v[ i++ ]))
		;
	      
	      codepos += size;
	      
	      break;
	    }

	case OBJWORD:
	case OBJSHORT:
	case OBJBYTE:
	  putpatch( c );
	  
	  codepos += tag & 0x07;
	  
	  break;
         
	case OBJINIT:
	  if (NullRef( c->value.v ))
	    {
	      outword( 0L );
	    }
	  else 
	    {
	      word	next = VMAddr( Code, c->value.v )->loc;

	      
	      outword( (WORD)(next - c->loc) );
	    }
	  
	  codepos += 4;

	  break;
                  
	default:
	  error( "Unknown tag %#x (@%#x)", tag, codepos );
	  break;
	}
      
      c++;
    }

  flush_output_buffer();

  return;
  
} /* genimage */

/*}}}*/
/*{{{  putpatch() */

void
#ifdef __STDC__
putpatch( Code * c )
#else
putpatch( c )
  Code *	c;
#endif
{
  word	value = patchit( c->vtype, c->value.fill, c->type );
   
  trace( "Final patch value = %#x", value );
  
  switch (c->type)
    {
    case OBJWORD:
      outword( value );
      break;
      
    case OBJSHORT:
      if (!fits_in_16_bits( value ))
	warn( "Patch value %#x too large for short", value );      

#ifdef __BIGENDIAN
      outbyte1( value >> 8 );
      outbyte1( value );
#else
      outbyte1( value );
      outbyte1( value >> 8 );
#endif
      
      break;
      
    case OBJBYTE:
      if (value < -128 || value > 255)
	warn( "Patch value %#x too large for byte (@%#x)", value, codepos );
      
      outbyte1( value );
      
      break;

    default:
      warn( "unknown patch result %d\n", c->type );
      break;
   }

  return;
  
} /* putpatch */

/*}}}*/
/*{{{  patchit() */

word
#ifdef __STDC__
patchit(
	word 	vtype,	/* patch type */
	long 	value, 	/* ptr to another patch, or patch symbol */
	word 	size )   /* patch size */
#else
patchit( vtype, value, size )
  word 	vtype;	/* patch type */
  long 	value; 	/* ptr to another patch, or patch symbol */
  word 	size;   /* patch size */
#endif
{
  if (OBJPATCH <= vtype && vtype <= OBJPATCHMAX )
   {
     Value	val;

     
     val.fill = value;

     return (mcpatch( vtype, val.v, size ));
   }

   return (stdpatch( vtype, value ));

} /* patchit */

/*}}}*/
/*{{{  FindSymbolOffset() */

/*
 * Calculate the offset from the start of the current
 * module of the indicated symbol.
 */

WORD
FindSymbolOffset( VMRef vSymbol )
{
  Symbol *	pSymbol;

  
  pSymbol = VMAddr( Symbol, vSymbol );

  if (NullRef( pSymbol ))
    return 0;
  
  if (pSymbol->module != curmod)
    {
      WORD	iSize;
      VMRef 	vMod;
      VMRef	vOurMod = pSymbol->module;
      

      /* The symbol is not in the current module */
      
      vMod = module0;

      /* discover which comes first 'curmod' or 'vOurMod' */

      while (!NullRef( vMod ) &&
	     vMod != vOurMod  &&
	     vMod != curmod    )
	{
	  vMod = VMAddr( asm_Module, vMod )->next;
	}
      
      if (NullRef( vMod ))
	{
	  pSymbol = VMAddr( Symbol, vSymbol );
	  
	  warn( "Stub Generation: could not find module containing symbol '%s'!",
	       pSymbol->name );
	  
	  return codepos;
	}
      
      iSize = 0;
      
      if (vMod == curmod)
	{
	  /* search for forwards reference */

	  while (!NullRef( vMod ) &&
		 vMod != vOurMod   )
	    {
	      asm_Module * 	pModule = VMAddr( asm_Module, vMod );
	      
	      iSize += pModule->length;
	      vMod   = pModule->next;
	    }
	  
	  pSymbol = VMAddr( Symbol, vSymbol );
	  
	  if (NullRef( vMod ))
	    {
	      warn( "Stubs: unable to locate module %s containing symbol '%s'",
		   VMAddr( asm_Module, vOurMod )->file_name,
		   pSymbol->name );
	      
	      return codepos;
	    }
	  
	  return VMAddr( Code, pSymbol->value.v )->loc + iSize;
	}
      else
	{
	  /* search for backwards reference */
	  
	  while (!NullRef( vMod ) &&
		 vMod != curmod    )
	    {
	      asm_Module *	pModule = VMAddr( asm_Module, vMod );
	      
	      
	      iSize += pModule->length;
	      vMod   = pModule->next;
	    }
	  
	  pSymbol = VMAddr( Symbol, vSymbol );
	  
	  if (NullRef( vMod ))
	    {
	      warn( "Stub Generation: unable to locate module refering to symbol '%s'",
		   pSymbol->name );
	      
	      return codepos;
	    }
	  
	  return VMAddr( Code, pSymbol->value.v )->loc - iSize;
	}
    }

  /* symbol is inside the current module - just return the offset */
  
  return VMAddr( Code, pSymbol->value.v )->loc;
  
} /* FindSymbolOffset */

/*}}}*/
/*{{{  stdpatch() */

word
#ifdef __STDC__
stdpatch(
	 word 	vtype,
	 long	xvalue )
#else
stdpatch( vtype, xvalue )
  word 	vtype;
  long	xvalue;
#endif
{
  Value 	value;
  asm_Module *	m;
  Symbol *	s;


  value.fill = xvalue;

  m = VMAddr( asm_Module, curmod );
      
  switch (vtype)
    {
    default:
      error( "Unknown standard patch type in stdpatch: %#x (@%#x)", vtype, codepos );
      
      return 0;
      
    case OBJMODSIZE:
      return m->length;
      
    case OBJMODNUM:
      trace( "Stdpatch: returning module %08x, number %d", curmod, m->id );

      return m->id;

#ifdef NEW_STUBS
    case OBJCODESTUB:
    case OBJADDRSTUB:
	{
	  WORD	r;

	  
	  s = VMAddr( Symbol, value.v );

	  if (s->type != S_CODESYMB)
	    {
	      VMRef	ref;
	      VMRef	mod;
	      

	      if (vtype == OBJCODESTUB)
		{
		  char	name[ 128 ];		/* XXX */
		  
		  /* catch unlikely case of a CODESTUB for something other than .<name> */
		  
		  if (*s->name != '.')
		    {
		      warn( "Function \"%s\" used in module '%s', but not defined",
			   s->name + 1, m->file_name );

		      /* prevent future warnings */
		      
		      s->type   = S_CODESYMB;
		      s->module = curmod;
	  
		      return 0;
		    }

		  /* duplicate name */
	      
		  strcpy( name, s->name );

		  /* change first character */
	      
		  name[ 0 ] = '_';

		  /* find the associated shared library symbol */
	      
		  ref = lookup( name );

		  if (ref == NullVMRef)
		    {
		      warn( "Function \"%s\" is called from module '%s', but it is not defined",
			   name + 1, m->file_name );
		      
		      /* prevent future warnings */
		  
		      s->type   = S_CODESYMB;
		      s->module = curmod;
		  
		      return 0;		  
		    }
		  
		  /* check the type of the symbol */
	      
		  s = VMAddr( Symbol, ref );

		  if (s->type == S_UNBOUND)
		    {
		      warn( "Function \"%s\" is called in '%s', but it is not defined",
			   name + 1, m->file_name );
		      
		      /* prevent futher warnings */
		  
		      s->type   = S_FUNCDONE;
		      s->module = curmod;

		      return 0;
		    }
	
		  if (s->type != S_FUNCDONE)
		    {
		      if (s->type == S_FUNCSYMB)
			{
			  warn( "Function \"%s\" is called in '%s', but it has not been linked",
			       name + 1, m->file_name );
			}
		      else
			{
			  warn( "Function \"%s\" is called in '%s', but defined in '%s' as something else.",
			       name + 1, m->file_name, s->file_name );
			}

		      /* prevent futher warnings */
		  
		      s->type   = S_FUNCDONE;
		      s->module = curmod;

		      return 0;
		    }
	
		  s->referenced = 1;
		}
	      else
		{
		  ref = value.v;
		}
	      
	      /* request a new stub */
	      
	      r = new_stub( ref, vtype == OBJADDRSTUB );
	      
	      /* adjust 'r' to be relative to current module */
	      
	      mod = module0;
	      
	      while (!NullRef( mod ) &&
		     mod != curmod)
		{
		  asm_Module *	module;
		  
		  
		  module = VMAddr( asm_Module, mod );
		  
		  r -= module->length;
		  
		  mod = module->next;
		}
	    }
	  else
	    {
	      r = FindSymbolOffset( value.v );
	    }
	  
	  trace( "Stdpatch: returning label offset of %s = %d", s->name, r - codepos );
	  
	  return r - codepos;
	}
#endif /* NEW_STUBS */
      
    case OBJLABELREF:
	{
	  word	r;

	  
	  s = VMAddr( Symbol, value.v );
	  
	  if (s->type != S_CODESYMB)
	    {
	      warn( "Label \"%s\" used in module '%s', but not defined", s->name, m->file_name );
	      
	      s->type   = S_CODESYMB;
	      s->module = curmod;
	  
	      return 0;
	    }
	  
	  r = FindSymbolOffset( value.v );
	  
	  trace( "Stdpatch: returning label offset of %s = %d", s->name, r - codepos );

	  return r - codepos;
	}

    case OBJDATASYMB:
      s = VMAddr( Symbol, value.v );
      
      trace( "Stdpatch: datasymb of symbol %s", s->name );
      
      if (s->type != S_DATADONE)
	{
	  if (s->type == S_FUNCDONE)
	    warn( "Alas, function %s was assumed to be data, please link %s before %s",
		 s->name + 1, VMAddr( asm_Module, s->module)->file_name, m->file_name );
	  else
	    warn( "Data Symbol \"%s\" used in module '%s', but not defined",
		 s->name + 1, m->file_name );

	  s->type   = S_DATADONE;
	  s->module = curmod;
	  
	  return 0;
	}

      return s->value.w;
      
    case OBJCODESYMB:
      s = VMAddr( Symbol, value.v );
      
      trace( "Stdpatch: codesymb of symbol %s", s->name );
      
      if (s->type != S_FUNCDONE)
	{
	  warn( "Function Symbol \"%s\" used in module '%s', but not defined", s->name + 1, m->file_name );
	  
	  s->type   = S_FUNCDONE;
	  s->module = curmod;
	  
	  return 0;
	}

      return s->value.w;
      
    case OBJDATAMODULE:
      s = VMAddr( Symbol, value.v );
      
      trace( "Stdpatch: datamodule of symb %s", s->name );
      
      if (s->type != S_DATADONE && s->type != S_FUNCDONE)
	{
#ifdef NEW_STUBS
	  char	name[ 128 ];	/* XXX */
	  VMRef	ref;
	  
	  /* catch unlikely case of a DATAMODULE for something other than _<name> */
	  
	  if (*s->name != '_')
	    {
	      warn( "Function \"%s\" called in module '%s', but not defined",
		   s->name + 1, m->file_name );
	      
	      /* prevent future warnings */
		      
	      s->type   = S_FUNCDONE;
	      s->module = curmod;
	  
	      return 0;
	    }
	  
	  /* duplicate name */
	  
	  strcpy( name, s->name );
	  
	  /* change first character */
	      
	  name[ 0 ] = '.';
	  
	  /* find the associated shared library symbol */
	      
	  ref = lookup( name );

	  if (ref == NullVMRef)
	    {
	      warn( "Function \"%s\" was called from module '%s', but it is not defined",
		   name + 1, m->file_name );
	      
	      /* prevent future warnings */
	      
	      s->type   = S_FUNCDONE;
	      s->module = curmod;
	      
	      return 0;		  
	    }
	  
	  /* check the type of the symbol */
	  
	  s = VMAddr( Symbol, ref );
	  
	  if (s->type == S_UNBOUND)
	    {
	      warn( "Function \"%s\" was called in '%s', but it is not defined",
		   name + 1, m->file_name );
	      
	      /* prevent futher warnings */
	      
	      s->type   = S_FUNCDONE;
	      s->module = curmod;
	      
	      return 0;
	    }
	  
	  if (s->type != S_CODESYMB)
	    {
	      if (s->type == S_FUNCSYMB)
		{
		  warn( "Function \"%s\" was called in '%s', but it has not been linked",
		       name + 1, m->file_name );
		}
	      else
		{
		  warn( "Function \"%s\" was called in '%s', but defined in '%s' as type %x.",
		       name + 1, m->file_name, s->file_name, s->type );
		}
	      
	      /* prevent further warnings */
	      
	      s->type   = S_CODESYMB;
	      s->module = curmod;
	      
	      return 0;
	    }
	  
	  /* indicate that the .<name> is being used */
	  
	  s->referenced = 1;

	  trace( "branch ref to %s in %s", s->name, VMAddr( asm_Module, s->module )->file_name );
	  
	  /* generate a branch stub called _<name> */
	  
	  RequestBranchStub( value.v );

	  /* locate this branch stub */
	  
	  s = VMAddr( Symbol, value.v );
	  
#else /* not NEW_STUBS */

	  if (s->AOFassumedData)
	    {
	      if (!NullRef( s->module ))
		{ 
		  asm_Module *	pDef; 

	      
		  pDef = VMAddr( asm_Module, s->module );	  

		  if (m->file_name == pDef->file_name)
		    warn( "Alas, function '%s' was assumed to be data, please link %s before %s",
			 s->name + 1, s->file_name, m->file_name );
		  else
		    warn( "Alas, function '%s' was assumed to be data, please link %s before %s",
			 s->name + 1, pDef->file_name, m->file_name );
		}
	      else
		warn( "Beware, function '%s' was assumed to be data", s->name + 1 );
	      
	      s->type   = S_DATADONE;
	      s->module = curmod;
	    }
	  else
	    {
	      warn( "Symbol \"%s\" used in module '%s', but not defined",
		   s->name + 1, m->file_name );
	  
	      s->type = S_FUNCDONE;
	      s->module = curmod;
	    }
	  
	  return 0;
#endif /* NEW_STUBS */
	}

      m = VMAddr( asm_Module, s->module );
      
      trace( "DATAMODULE returns module id %d for symbol %s", m->id, s->name /* XXX */ );
      
      if (smtopt)
	return (m->id * 8 + (s->type == S_FUNCDONE ? 4 : 0));
      else
	return (m->id * 4);
    }
  
  return 0;
  
} /* stdpatch */

/*}}}*/
/*{{{  imageheader() */

/****************************************************************/
/* imageheader                                                  */
/*                                                              */
/* Generate image file header                                   */
/*                                                              */
/****************************************************************/

PRIVATE void
#ifdef __STDC__
imageheader( WORD imagesize )
#else
imageheader( imagesize )
  WORD	imagesize;
#endif
{
	if (gen_image_header) {
		outword( Image_Magic );
		outword( 0L );			/* Flags */
		outword( imagesize );
	}

	nchars = 0;

	return;

} /* imageheader */

/*}}}*/
/*{{{  outword() */

/****************************************************************/
/* outword                                                      */
/*                                                              */
/* output a word to image file                                  */
/*                                                              */
/****************************************************************/

PUBLIC void
#ifdef __STDC__
outword( WORD val )
#else
outword( val )
  WORD	val;
#endif
{
  int i;


#ifdef __BIGENDIAN /* 68k etc */
  
  for (i = 24 ; i >= 0 ; i -= 8)
    outbyte( (UBYTE)(val >> i) );
  
#else

  for (i = 0 ; i < 32 ; i += 8)
    outbyte( (UBYTE)(val >> i) );
  
#endif

  return;
  
} /* outword */

/*}}}*/
/*{{{  outbyte1() */

/****************************************************************/
/* Procedure: outbyte1                                           */
/*                                                              */
/*      output a byte to image file                             */
/*                                                              */
/****************************************************************/

PRIVATE void
#ifdef __STDC__
outbyte1( WORD b )
#else
outbyte1( b )
  WORD	b;
#endif
{
  outbyte( (UBYTE)b );

  return;
  
} /* outbyte1 */

/*}}}*/
/*{{{  outbyte() */

PRIVATE void
#ifdef __STDC__
outbyte( UBYTE b )
#else
outbyte( b )
  UBYTE	b;
#endif
{
  output_buffer[output_buffer_pointer++] = b;

  if (output_buffer_pointer == BUFFER_SIZE)
    flush_output_buffer();

  nchars++;

  return;
  
} /* outbyte */

/*}}}*/
/*{{{  flush_output_buffer() */

PUBLIC void
#ifdef __STDC__
flush_output_buffer( void )
#else
flush_output_buffer( )
#endif
{
  int size = 0;
  int size1;

  while (size != output_buffer_pointer)
    {
      size1 = write(outf, (char *)(output_buffer + size),
		    output_buffer_pointer - size);

      if (size1 < 0)
	error( "image file write error");
      
      size += size1;
    }

  output_buffer_pointer = 0;
}

/*}}}*/
/*{{{  swapw() */

/* swap words */

long
#ifdef __STDC__
swapw( long x )
#else
swapw( x )
  long	x;
#endif
{

  return ((x >> 24) & 0x000000ffU) |
         ((x >>  8) & 0x0000ff00U) |
         ((x <<  8) & 0x00ff0000U) |
	 ((x << 24) & 0xff000000U) ;

} /* swapw */

/*}}}*/
/*{{{  swaps() */

/* swap shorts */

unsigned int
#ifdef __STDC__
swaps( unsigned int x )
#else
swaps( x )
  unsigned int x;
#endif
{

  return ((x >> 8) & 0xff) | ((x << 8) & 0xff00);
  
} /* swaps */

/*}}}*/

#if defined __C40
/*{{{  mask_and_sign_extend() */

/*
 * extracts the bits specified by 'mask' from the word 'value'
 * if necessary the resulting word is sign extended.
 * 'mask' must be a contigous set of bits starting from
 * the least significant bit
 */

static signed long
mask_and_sign_extend_word(
			  unsigned long	value,
			  unsigned long	mask )
{
  value &= mask;

  if (value & ((mask + 1) >> 1))
    {
      value |= ~mask;
    }

  return (signed long)value;
  
} /* mask_and_sign_extend_word */

/*}}}*/
#endif /* __C40 */

/*{{{  mcpatch() */

/* Machine specific patches defined for ARM, M68K and PGC1 */

word
#ifdef __STDC__
mcpatch(
	word 	type,
	VMRef	v,
	word 	size )
#else
mcpatch( type, v, size )
  word 		type;
  VMRef	 	v;
  word 		size;
#endif
{
#ifdef __ARM
  long 		data;
#endif
  Patch *	p     = VMAddr( Patch, v );
  long		value; 
#ifdef __C40
  static word	saved_modnum = 0;
  static word	saved_offset = 0;
#endif

#ifdef __ARM
  if (size != OBJWORD && (type == PATCHARMDT ||
			  type == PATCHARMDP ||
			  type == PATCHARMJP ) )
    error( "ARM specific patch used to patch a short or byte value (@%#x)", codepos );

  if (type < PATCHARMAOFLSB || type > PATCHARMAOFREST)
#endif /* __ARM */

    value = patchit( p->type, p->value.fill, size );
  
  switch (type)
    {
    case PATCHADD:
      return p->word + value;

   case PATCHSHIFT:
      if (p->word < 0)
	{
	  return ((signed long) value) >> (- p->word);
	}
      else
	{
	  return value << p->word;
	}

    case PATCHSWAP:
      if (size == OBJWORD)
	return swapw( value ); /* swap word */
      else
	return swaps( (unsigned) value ); /* swap short */

    case PATCHOR:
      return p->word | value;

#ifdef __C40
      /*
       * XXX - NC - 2/10/91
       *
       * WARNING: These patches a highly dependent upon the
       * register scheme defined in c40/target.h and the
       * code in load_address_constant() in c40/gen.c
       *
       * There are four cases:
       *
       *       modnum < 256               modnum < 256                 modnum > 255             modnum > 255
       *       offset < 256               offset > 255                 offset < 256             offset > 255
       *
       *  LDI *+AR4( modnum ), AR5    LDI  *+AR4( modnum ), AR5    LDI  AR4, AR5                LDI  AR4, AR5
       *  LDI *+AR5( offset ), AR5    ADDI offset, AR5             ADDI modnum, AR5             ADDI modnum, AR5
       *  Bu  AR5                     LDI  *AR5, AR5               LDI  *AR5, AR5               LDI  *AR5, AR5
       *  -                           Bu   AR5                     LDI  *+AR5( offset ), AR5    ADDI offset, AR5
       *  -                           -                            Bu   AR5                     LDI  *AR5, AR5
       *  -                           -                            -                            Bu   AR5
       */
      
    case PATCHC40DATAMODULE1:					/* value is DataModule */
      if (fits_in_8_bits( value ))
	{
	  saved_modnum = -2;
	  
	  return 0x084d0400U + (value & 0xff);			/* LDI *+AR4( modnum ), AR5 */
	}
      else if (!fits_in_16_bits( value ))
	{
	  error( "linker: module number does not fit in 16 bits - code will not link" );
	}
      else
	{
	  saved_modnum = value;

	  return 0x080d000cU;					/* LDI AR4, AR5 */
	}
      
    case PATCHC40DATAMODULE2:					/* value is DataSymb */
      if (saved_modnum == -2)
	{
	  saved_offset = -2;
	  
	  if (fits_in_8_bits( value ))
	    {
	      return 0x084d0500U + (value & 0xff);		/* LDI *+AR5( offset ), AR5 */
	    }
	  else if (!fits_in_16_bits( value ))
	    {
	      error( "linker: function offset in module table does not fit in 16 bits - code will not link" );
	    }
	  else
	    {
	      saved_offset = -1;				/* trigger LDI *AR5, AR5 */
	      
	      return 0x026d0000U + (value & 0xffff);		/* ADDI offset, AR5 */
	    }	  
	}
      else
	{
	  saved_offset = value;
      
	  return 0x026d0000U + saved_modnum;			/* ADDI modnum, AR5 */
	}	      
      
    case PATCHC40DATAMODULE3:
      if (saved_modnum == -2 && saved_offset == -2)
	{
	  return 0x6800000dU;					/* Bu Ar5 */
	}
      else
	{
	  return 0x084dc500U;					/* LDI *AR5, AR5 */
	}

    case PATCHC40DATAMODULE4:
      if (saved_modnum == -2 && saved_offset != -2)
	{
	  return 0x6800000dU;					/* Bu AR5 */
	}
      else if (saved_modnum != -2)
	{
	  if (fits_in_8_bits( saved_offset ))
	    {
	      saved_modnum = -2;				/* special case, picked up DataModule5 */
	      
	      return 0x084d0500U + (saved_offset & 0xff);	/* LDI *+AR5( offset ), AR5 */
	    }
	  else
	    {
	      return 0x026d0000U + (saved_offset & 0xffff);	/* ADDI offset, AR5 */
	    }
	}

    case PATCHC40DATAMODULE5:
      if (saved_modnum == -2)
	{
	  return 0x6800000dU;					/* Bu Ar5 */
	}
      else
	{
	  return 0x084dc500U;					/* LDI *AR5, AR5 */
	}

      /*
       * special addition routines that take a mask as well
       */
      
    case PATCHC40MASK24ADD:
	{
	  word	mask = 0x00FFFFFFU;
	  
	  
	  value += mask_and_sign_extend_word( p->word, mask );

	  if (!fits_in_24_bits( value ))
	    {
	      warn( "calculated offset (%08x) too big to fit in 24 bit instruction field (%08x)!",
		   value, p->word );
	    }
	  
	  return (p->word & (~mask)) | (value & mask);
	}      
      
    case PATCHC40MASK16ADD:
	{
	  word	mask = 0x0000FFFFU;

	  
	  value += mask_and_sign_extend_word( p->word, mask );

	  if (!fits_in_16_bits( value ))
	    {
	      warn( "calculated offset (%08x) too big to fit in 16 bit instruction field (%08x)!",
		   value, p->word );
	      warn( "This is probably because the source file was compiled with the -Zpl1 option" );	      
	    }
	  
	  return (p->word & (~mask)) | (value & mask);
	}      
      
    case PATCHC40MASK8ADD:
	{
	  word		mask = 0x000000FFU;
	  

	  /* NB/ use unsigned values as this patch is altering indirect displacements */
	  
	  value += (p->word & mask);

	  if (value & ~mask)
	    {
	      asm_Module *	m;

	      
	      m = VMAddr( asm_Module, curmod );
	      
	      warn( "calculated offset (%08x) too big to fit in 8 bits, op code in module %s!",
		   value, m->file_name );
	      warn( "This is probably because the source file was compiled with the -Zpl1 option" );	      
	    }
	  
	  return (p->word & (~mask)) | (value & mask);
	}      
      
#endif /* __C40 */
      
#ifdef __ARM
    case PATCHARMDT:			/* ARM data transfer instr. patch */
      
      value += (p->word & 0xfff); 	/* patch value includes existing immediate offset */

      data = p->word & 0xfffff000U;

      if (p->type == OBJLABELREF)
	value -= 8;			/* adjust pc rel labelref for ARM pipelining */

      if (value < -2048 || value > 4096)
	error( "ARM data transfer patch value (%#x) too large (@%#x)", value, codepos );

      /* set direction bit (23) in ARM instruction - 1 = up 0 = down */

      if (value < 0)
	{
	  value = -value;		/* offsets are unsigned */
	  data &= ~(1 << 23);		/* down = backwards jump = bit 23=0 */
	}
      else
	{
	  data |= 1 << 23;
	}

      return (data | value);

    case PATCHARMDP:			/* ARM data processing instr. patch */
	{
	  word	ror = 0;
	  word	savval;


	  value += (p->word & 0xff); 	/* add existing contents to patch value */

	  if (p->type == OBJLABELREF)
	    value -= 8;			/* adjust pc rel labelref for ARM pipelining */

	  savval = value;
	
	  if (value < 0)
	    error( "ARM data processing patch value should not be negative (@%#x)", codepos );

	  if (value > 255)
	    {
	      ror = 0x10;

	      while (!(value & 3)) 	/* word aligned immediate data? */
		{
		  /* see ARM assembly lang prg. pg 40 */
		  /* ARM data man pg 21/25 */

		  value >>= 2;		/* shift immediate value left by  two */

		  if (--ror <= 4) 	/* and alter rotate by factor of two */
		    break;  		/* ARM immediate rotates are multiplied by 2 */
		}

	      if (ror == 0x10)
		ror = 0; 		/* if no rotates used */
	    }
	  
	  if (value > 255)
	    {
#if 1
	      error( "Patch value of %#x is too large for ARM DP patch.(origval = %#x) data=%#x (@%#x)",
		    value, savval, p->word, codepos );
#else
	      warn( "Patch value of %#x is too large for ARM DP patch.(origval = %#x) data=%#x (@%#x)",
		   value, savval, p->word, codepos );
#endif
	    }
	  
	  value |= ror << 8; /* 4 bits of ror preceed 8 bit immediate */

	  return (value | (p->word & (unsigned long) 0xfffff000U));
	}

    case PATCHARMDPLSB:			/* ARM data processing instr. patch least sig byte */
	{
	  patchNeg = FALSE; 		/* default to use an add instr. for labelrefs */

	  if (p->type == OBJLABELREF)
	    {
	      value -= 8;		/* adjust pc rel labelref for ARM pipelining */

	      if (value < 0)
		{			/* backwards label ref */
		  patchNeg = TRUE; 	/* use a sub instr. */
		  value    = -value;
		}
	    }
	  else if (value < 0)
	    {
	      error( "Negative patch value passed to armdplsb" );
	    }
	  
	  if (p->type != OBJLABELREF)
	    {
	      word	imm;


	      /* get instr. existing immediate const and add to patch value */

	      imm = (p->word & 0xfff); 	/* bodged 12 bit immediate */

	      value += imm;
	    }
	
	  restofdppatch = value;

	  value &= 0xff; 		/* only ls byte for this patch  - not optimal as rot cannot be used */

	  restofdppatch -= value; 	/* remember rest of patch */

	  if (patchNeg)
	    {
	      int	instr = (int)(p->word & 0xfffff000U);
		

	      if ((instr & 0x01e00000U) == 0x00800000U)
		{
		  instr &= ~0x01e00000U;	/* convert to a sub instruct */
		  instr |= 0x00400000U;
		}
	      else
		{
		  error( "armdplsb: Trying to change add to sub when instr not an add" );
		}
	      
	      return (value | instr);
	    }
	  else
	    {
	      return (value | (p->word & 0xfffff000U));
	    }
	}

    case PATCHARMDPMID:			/* ARM data processing instr. patch second least sig byte */
	{
	  /* dummy to clean up any unrequired patch info */
	  
	  value = restofdppatch;

	  value         &=  0xFF00;	/* use second ls byte for this patch */
	  restofdppatch -=  value;	/* work out remainder still to patch */
	  value         >>= 8;		/* move into instructions immediate field */
	  value         |=  0x0C00;	/* merge in immediate data's rotate field */
	  				/* 0x0C00 = left shift by eight bits */
	  if (patchNeg)
	    {
	      int	instr = (int)(p->word & 0xfffff000U);

	      
	      if ((instr & 0x01e00000U) == 0x00800000U)
		{
		  instr &= ~0x01e00000U;	/* convert to a sub instruct */
		  instr |= 0x00400000U;
		}
	      else
		{
		  error( "armdpmid: Trying to change add to sub when instr not an add" );
		}
	      
	      return (value | instr);
	    }
	  else
	    {
	      return (value | (p->word & 0xfffff000U));
	    }
	}

    case PATCHARMDPREST:		/* ARM data processing instr. patch - rest of last lsb patch */
	{
	  word	ror = 0;		/* dummy to clean up any unrequired patch info */

	  
	  value = restofdppatch; 	/* use residue from last armdplsb/mid patch */

	  if (value > 255)
	    {
	      ror = 0x10;

	      while (!(value & 3)) 	/* word aligned immediate data? */
		{
		  /* see ARM assembly lang prg. pg 40 */
		  /* ARM data man pg 21/25 */

		  value >>= 2;		/* shift immediate value left by  two */

		  if (--ror <= 4) 	/* and alter rotate by factor of two */
		    break;  		/* ARM immediate rotates are multiplied by 2 */
		}

	      if (ror == 0x10)
		ror = 0; 		/* if no rotates used */
	    }

	  if (value > 255 )
	    error( "Patch value of %#x is too large for ARM DP residue patch (@%#x)", value, codepos );
	  
	  value |= ror << 8; 		/* 4 bits of ror preceed 8 bit immediate */

	  if (patchNeg)
	    {
	      int	instr = (int)(p->word & 0xfffff000U);


	      if ((instr & 0x01e00000U) == 0x00800000U)
		{
		  instr &= ~0x01e00000U;	/* convert to a sub instruct */
		  instr |= 0x00400000U;
		}
	      else
		{
		  error( "armdprest: Trying to change add to sub when instr not an add" );
		}
	      
	      return (value | instr);
	    }
	  else
	    {
	      return (value | (p->word & 0xfffff000U));
	    }
	}

    case PATCHARMJP:			/* ARM branch instr. patch */
      if (p->type != OBJLABELREF)
	error( "ARM jp patch should only be used with a label ref (@%#x)", codepos );
      
      return (p->word & 0xff000000U) | (((value - 8) >> 2) & 0x00ffffffU);
      
    case PATCHARMAOFLSB:
      {
	Symbol *	s;


	s = VMAddr( Symbol, p->value.v );
      
	trace( "ARM patch: data/code symb of symbol %s", s->name );
      
	if (s->type != S_DATADONE && s->type != S_FUNCDONE)
	  {
	    asm_Module *	m;

	    m = VMAddr( asm_Module, curmod );
	    
	    warn( "Symbol \"%s\" used in module '%s', but not defined", s->name + 1, m->file_name );

	    s->type   = S_DATADONE;
	    s->module = curmod;
	  
	    return 0;
	  }

	/* symbol's offest into data or code table is s->value.w */

	value = s->value.w;
	
	restofdppatch = value;

	value &= 0xff;	/* only ls byte for this patch  - not optimal as rot cannot be used */

	restofdppatch -= value; 	/* remember rest of patch */

	if (s->type == S_DATADONE)
	  {
	    AOFSymbolIsData = TRUE;
	    
	    return 0xe2800000UL | ((p->word & 0xF) << 12) | ((p->word & 0xF) << 16) | value;
	  }
	else
	  {
	    AOFSymbolIsData = FALSE;
	    
	    return 0xe5900000UL | ((p->word & 0xF) << 12) | ((p->word & 0xF) << 16) | value;
	  }
      }
      
    case PATCHARMAOFMID:
      value = restofdppatch;

      value         &=  0xFF00;	/* use second ls byte for this patch */
      restofdppatch -=  value;	/* work out remainder still to patch */
      value         >>= 8;	/* move into instructions immediate field */
      value         |=  0x0C00;	/* merge in immediate data's rotate field */
	  			/* 0x0C00 = left shift by eight bits */

      if (AOFSymbolIsData)
	return 0xe2800000UL | ((p->word & 0xF) << 12) | ((p->word & 0xF) << 16) | value;
      else
	return 0xe1a00000UL;

    case PATCHARMAOFREST:
	{
	  word	ror = 0;		/* dummy to clean up any unrequired patch info */

	  
	  value = restofdppatch; 	/* use residue from last armdplsb/mid patch */

	  if (value > 255)
	    {
	      ror = 0x10;

	      while (!(value & 3)) 	/* word aligned immediate data? */
		{
		  /* see ARM assembly lang prg. pg 40 */
		  /* ARM data man pg 21/25 */

		  value >>= 2;		/* shift immediate value left by  two */

		  if (--ror <= 4) 	/* and alter rotate by factor of two */
		    break;  		/* ARM immediate rotates are multiplied by 2 */
		}

	      if (ror == 0x10)
		ror = 0; 		/* if no rotates used */
	    }

	  if (value > 255 )
	    error( "Patch value of %#x is too large for ARM DP residue patch (@%#x)", value, codepos );
	  
	  value |= ror << 8; 		/* 4 bits of ror preceed 8 bit immediate */

	  if (AOFSymbolIsData)
	    return 0xe2800000UL | ((p->word & 0xF) << 12) | ((p->word & 0xF) << 16) | value;
	  else
	    return 0xe1a00000UL;
	}
#endif /* __ARM */

    default:
      error( "Unknown machine dependent patch type in mcpatch: %#x (@%#x)", type, codepos );
   }
 
  return 0;

} /* mcpatch */

/*}}}*/

/*}}}*/
