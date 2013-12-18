/*
 * stubs.c : 	stub generation file for the Helios Linker
 *
 *   Copyright (c) 1992 - 1994 Perihelion Software Ltd.
 *     All rights reserved.
 *
 * Author :	N Clifton
 * Version :	$Revision: 1.19 $
 * Date :	$Date: 1994/01/11 16:08:32 $
 * Id :		$Id: stubs.c,v 1.19 1994/01/11 16:08:32 nickc Exp $
 */

/*{{{  Headers */

#include "link.h" /* needed for definition of NEW_STUBS amoungst other things */

#include <unistd.h>	/* for SEEK_SET */
#include <module.h>	/* for Module structure */

/*}}}*/
#ifdef NEW_STUBS
/*{{{  Macros */

#define trace 	if (traceflags & db_stubs) _trace

/*}}}*/
/*{{{  Types */

typedef enum
  {
    CALLING_STUB,			/* ordinary calling stub */
    LOCAL_ADDR_STUB,			/* address returning stub for symbols in ordinary modules */
    SHARED_ADDR_STUB			/* address returning stub for symbols in shared library modules */
  }
StubType;

typedef struct
  {
    VMRef	next;			/* link into chain of stubs                  */
    VMRef	name_sym;		/* the symbol naming the stub		     */
    VMRef	symbol;			/* the symbol represented by this stub       */
    WORD	id;			/* id of the module containing the symbol    */
    WORD	offset;			/* byte offset of symbol within the module   */
    WORD	position;		/* byte position of the stub within the file */
    StubType	type;			/* the nature of the stub                    */
  }
Stub;

typedef struct
  {
    VMRef	vNext;			/* Link into the chain of branch stubs		*/
    VMRef	vSymbol;		/* The symbol to which this stub branches	*/
    WORD	iOffset;		/* This is the stub's offset into the codetable */
  }
BranchStub;

/*}}}*/
/*{{{  Variables */

extern  word	lastm;		/* Highest assigned module number, defined in module.c */
extern word	nchars;		/* Number of characters written, defined in genimage.c */

PRIVATE word	totalstubsize = 0;		/* Number of bytes of stubs produced */
PRIVATE word	iBranchCount  = 0;		/* Number of byes of branch stubs made */
PRIVATE int	inited        = 0;		/* True if structures are initialised */
PRIVATE VMRef	stub_list;			/* Linked list of stubs */
PRIVATE VMRef	stub_heap;			/* Heap to allocate new stub structures */
PRIVATE VMRef	vStubModule;			/* Fake module used by stub code */
PRIVATE VMRef	vBranchStubList;		/* Linked list of branch stubs */
PRIVATE word	iModuleNchars;			/* Start "address" of fake module */
PRIVATE word	iBranchNchars;			/* Start "address" of branch stubs */
PRIVATE word	iCallingStubStart;		/* where the next calling stub is located */

/*}}}*/
/*{{{  Functions */

#ifdef __C40
/*{{{  C40 Functions */

/*{{{  Macros */

#define fits_in_8_bits( val )		(((val) & 0xffffff80U) == 0 || ((val) & 0xffffff80U) == 0xffffff80U)
#define fits_in_16_bits( val )		(((val) & 0xffff8000U) == 0 || ((val) & 0xffff8000U) == 0xffff8000U)
#define fits_in_24_bits( val )		(((val) & 0xff800000U) == 0 || ((val) & 0xff800000U) == 0xff800000U)
#define LINKER_MODULE_NAME   "Fake Linker Module";

/*}}}*/
/*{{{  stub_size */

/*
 * returns the size (in bytes) that the calling stub will be when it is created
 */

PRIVATE WORD
stub_size( Stub * stub )
{
  WORD	size;


  if (stub->type == LOCAL_ADDR_STUB)
    {
      size = 6;
    }
  else
    {
      size = 3;
  
      if (!fits_in_8_bits( stub->id ))
	size += 2;

      if (!fits_in_8_bits( stub->offset >> 2 ))
	++size;

      if (size == 3 && stub->type == SHARED_ADDR_STUB)
	{
	  size = 4;
	}
    }
  
  return size * sizeof (WORD);
  
} /* stub_size */

/*}}}*/
/*{{{  branch_size */

/*
 * returns the size (in bytes) that the branch stub will be when it is created
 */

PRIVATE WORD
branch_size( BranchStub * stub )
{
  return sizeof (WORD);
  
} /* branch_size */

/*}}}*/
/*{{{  build_stub */

/*
 * create a calling stub
 */

PRIVATE void
build_stub( VMRef s )
{
  Stub *	stub;
  WORD		id;
  WORD		offset;
  char *	name;
  

  /* get hold of the stub */
  
  stub = VMAddr( Stub, s );
  name = VMAddr( Symbol, stub->symbol )->name;
  stub = VMAddr( Stub, s );
  
  trace( "building %s stub for %s at %d",
	stub->type == CALLING_STUB    ? "calling"              :
	stub->type == LOCAL_ADDR_STUB ? "local address "       :
	                                "module table address" ,
	name,
	nchars );

  if (stub->position != nchars)
    {
      warn( "Stub out of position.  Should be at %d (%d), is at %d (%d)",
	   stub->position, stub->position >> 2, nchars, nchars >> 2 );

      return;
    }

  id     = stub->id;
  offset = stub->offset >> 2;
  
  /* now create the calling stub */

  if (stub->type == CALLING_STUB)
    {
      /* create an ordinary stub */
      
      if (fits_in_8_bits( id ))
	{
	  if (fits_in_8_bits( offset ))
	    {
	      outword( 0x1Ecd0400UL + (id     & 0xff) );	/* LDA *+AR4(    id  ), AR5 */
	      outword( 0x1Ecd0500UL + (offset & 0xff) );	/* LDA *+AR5( offset ), AR5 */
	      outword( 0x6800000dUL );				/* Bu    AR5                */
	    }
	  else
	    {
	      /* symbol's offset within the code table is >= 256 */
	      
	      if (!fits_in_16_bits( offset ))
		{
		  warn( "Stub Creation: offset out of range for %s", name );
		  
		  return;	      
		}
	      
	      outword( 0x1Ecd0400UL + (id     & 0xff) );	/* LDA  *+AR4( id ), AR5 */
	      outword( 0x026d0000UL + (offset & 0xffff) );	/* ADDI   offset,    AR5 */
	      outword( 0x1Ecdc500UL );				/* LDA   *AR5,       AR5 */	  
	      outword( 0x6800000dUL );				/* Bu     AR5            */
	    }
	}
      else
	{
	  if (!fits_in_16_bits( id ))
	    {
	      warn( "Stub Creation: module id out of range for %s", name );
	      
	      return;	      
	    }
	  
	  /* symbol's module number is >= 256 */
	  
	  if (fits_in_8_bits( offset ))
	    {
	      outword( 0x1e8d000cUL );				/* LDA	 AR4,           AR5 */
	      outword( 0x026d0000UL + (id & 0xffff) );		/* ADDI  id,            AR5 */
	      outword( 0x1Ecdc500UL );				/* LDA  *AR5,           AR5 */	  
	      outword( 0x1Ecd0500UL + (offset & 0xff) );	/* LDA *+AR5( offset ), AR5 */
	      outword( 0x6800000dUL );				/* Bu    AR5                */	  
	    }
	  else
	    {
	      /* symbol's offset within the code table is >= 256 */
	      
	      if (!fits_in_16_bits( offset ))
		{
		  warn( "Stub Creation: offset out of range for %s", name );
		}
	      
	      outword( 0x1e8d000cUL );				/* LDA	 AR4,    AR5 */
	      outword( 0x026d0000UL + (id & 0xffff) );		/* ADDI  id,     AR5 */
	      outword( 0x1Ecdc500UL );				/* LDA  *AR5,    AR5 */
	      outword( 0x026d0000UL + (offset & 0xffff) );	/* ADDI  offset, AR5 */
	      outword( 0x1Ecdc500UL );				/* LDA  *AR5,    AR5 */	  
	      outword( 0x6800000dUL );				/* Bu    AR5         */
	    }
	}
    }
  else
    {
      /*
       * Create an address stub.
       *
       * An address stub is a stub which returns the
       * address of the function it represents.  These
       * are needed because when C code performs the
       * comparision of two function addresses it MUST
       * use the actual addresses, and not the addresses
       * of any calling stubs.  In particular the signal
       * handling routines in the POSIX library check
       * to see if the signal handler is being reset to
       * SIG_DFL.  Unfortunatly since the code calling
       * signal() is in an ordinary module, the pointer
       * it would normally pass in is the address of the
       * calling stub for SIG_DFL.  In the POSIX library,
       * however, the address of the real SIG_DFL is
       * available, and it is this that is used for
       * performing the comparison.  Hence the need for
       * this sort of stub.  What it does is return the
       * address of the real function.  For a function
       * in an ordinary module this is simply a case of
       * resolving the symbol.  For functions in resident
       * library's the address must be obtained via the
       * module table.
       */

      if (stub->type == LOCAL_ADDR_STUB)
	{
	  VMRef		mod;
	  VMRef		syms_mod;	  
	  Symbol *	sym;
	  char *	name;
	  word		size;
	  word		loc;
	  uword		pos;
	  

	  pos = stub->position;
	  
	  /* locate the symbol's module */

	  mod = module0;

	  sym = VMAddr( Symbol, stub->symbol );

	  syms_mod = sym->module;
	  name     = sym->name;
	  loc      = VMAddr( Code, sym->value.v )->loc;
	  size     = 0;

	  while (!NullRef( mod ) && mod != syms_mod)
	    {
	      asm_Module *	m;

	      
	      m = VMAddr( asm_Module, mod );
	      
	      size += m->length;
	      
	      mod = m->next;
	    }

	  if (NullRef( mod ))
	    {
	      warn( "Could not find local module containing symbol %s", name );

	      return;
	    }

	  trace( "offset of local symbol's module is %d", size );
	  trace( "symbol's offset = %d", loc );

	  pos -= loc + size;

	  pos /= sizeof (word);
	  
	  trace( "distance between stub and symbol = %d", pos );

	  pos += 5;	   /* allow for distance between start of stub and location placed in R11 */
	  
	  outword( 0x0819001fUL );			/* LDI   R11, RS  */
	  outword( 0x63000001UL );			/* LAJ  +1        */
	  outword( 0x1fed0000UL | pos >> 16    );	/* LDHI  0,   AR5 */
	  outword( 0x106d0000UL | pos & 0xffff );	/* OR    0,   AR5 */
	  outword( 0x198d001fUL );			/* SUBRI R11, AR5 */
	  outword( 0x68000019UL );			/* Bu    RS       */
	}
      else
	{
	  /* have to use the module table */
	  
	  if (fits_in_8_bits( id ))
	    {
	      if (fits_in_8_bits( offset ))
		{
		  outword( 0x6820001fUL );			/* BuD   R11                */
		  outword( 0x1Ecd0400UL + (id     & 0xff) );	/* LDA *+AR4(    id  ), AR5 */
		  outword( 0x1Ecd0500UL + (offset & 0xff) );	/* LDA *+AR5( offset ), AR5 */
		  outword( 0x0c800000UL );			/* NOP                      */
		}
	      else
		{
		  /* symbol's offset within the code table is >= 256 */
	      
		  if (!fits_in_16_bits( offset ))
		    {
		      warn( "Stub Creation: offset out of range for %s", name );
		      
		      return;
		    }
		  
		  outword( 0x6820001fUL );			/* BuD   R11                */
		  outword( 0x1Ecd0400UL + (id     & 0xff) );	/* LDA  *+AR4( id ), AR5 */
		  outword( 0x026d0000UL + (offset & 0xffff) );	/* ADDI   offset,    AR5 */
		  outword( 0x1Ecdc500UL );			/* LDA   *AR5,       AR5 */	  
		}
	    }
	  else
	    {
	      if (!fits_in_16_bits( id ))
		{
		  warn( "Stub Creation: module id out of range for %s", name );
		  
		  return;	      
		}
	      
	      /* symbol's module number is >= 256 */
	  
	      if (fits_in_8_bits( offset ))
		{
		  outword( 0x1e8d000cUL );			/* LDA	 AR4,           AR5 */
		  outword( 0x6820001fUL );			/* BuD   R11                */
		  outword( 0x026d0000UL + (id & 0xffff) );	/* ADDI  id,            AR5 */
		  outword( 0x1Ecdc500UL );			/* LDA  *AR5,           AR5 */	  
		  outword( 0x1Ecd0500UL + (offset & 0xff) );	/* LDA *+AR5( offset ), AR5 */
		}
	      else
		{
		  /* symbol's offset within the code table is >= 256 */
	      
		  if (!fits_in_16_bits( offset ))
		    {
		      warn( "Stub Creation: offset out of range for %s", name );
		      
		      return;	      
		    }
	      
		  outword( 0x1e8d000cUL );			/* LDA	 AR4,    AR5 */
		  outword( 0x026d0000UL + (id & 0xffff) );	/* ADDI  id,     AR5 */
		  outword( 0x6820001fUL );			/* BuD   R11         */
		  outword( 0x1Ecdc500UL );			/* LDA  *AR5,    AR5 */
		  outword( 0x026d0000UL + (offset & 0xffff) );	/* ADDI  offset, AR5 */
		  outword( 0x1Ecdc500UL );			/* LDA  *AR5,    AR5 */	  
		}
	    }
	}
    }
  
  return;
  
} /* build_stub */

/*}}}*/
/*{{{  GenerateBranchStub */

PRIVATE void
GenerateBranchStub( VMRef vBranchStub )
{
  VMRef		vSymbol;
  BranchStub *	pBranchStub;
  Symbol *      pSymbol;
  word		iOffset;
  char		name[ 128 ];	/* XXX */
  

  pBranchStub = VMAddr( BranchStub, vBranchStub );
  
  if (NullRef( pBranchStub ))
    {
      trace( "stubs: unable to find next branch stub" );
      return;
    }

  strcpy( name, VMAddr( Symbol, pBranchStub->vSymbol )->name );

  name[ 0 ] = '.';

  vSymbol = lookup( name );

  if (NullRef( vSymbol ))
    trace( "Branch symbol no longer exists" );

  if (!VMAddr( asm_Module, VMAddr( Symbol, vSymbol )->module )->linked)
    trace( "stubs: module containing destination of branch is not linked!" );
  
  iOffset = FindSymbolOffset( vSymbol );
  
  pSymbol = VMAddr( Symbol, vSymbol );

  iOffset -= (nchars - iModuleNchars);

  iOffset >>= 2;	/* convert from bytes to words */
  
  iOffset -= 1;
  
  trace( "generate a branch stub for %s, at offset %d", pSymbol->name, iOffset );
	
  outword( 0x60000000 | (iOffset & 0x00FFFFFF) ); /* BR */
  
  return;
  
} /* GenerateBranchStub */

/*}}}*/
/*{{{  GenerateInitCode */

/*
 * Generate code to initialise the linker's
 * fake module's code table.
 */

PRIVATE void
GenerateInitCode( void )
{
  word	iModId;
  word	iOffset;
  word	iInitNchars = nchars;
  
  
  trace( "Generating module initialisation code" );
  
#ifdef __SMT
  iModId = VMAddr( asm_Module, vStubModule )->id * 2 + 1;
#else
  iModId = VMAddr( asm_Module, vStubModule )->id;
#endif
  
  /* generate initiale sequence */
  
  outword( 0x04e00002 ); 		/* CMPI  2,   R0  */
  outword( 0x6806001f );		/* Bne   R11      */
  outword( 0x1e8a001f );		/* LDA   R11, AR2 */
  outword( 0x63000001 );		/* LAJ   + 1      */
  outword( 0x1e8d000c );		/* LDA   AR4, AR5 */
  outword( 0x026d0000 + iModId );	/* ADDI  mod, AR5 */
  outword( 0x1ecdc500 );		/* LDA  *AR5, AR5 */
  
  /* calculate where the branch stubs start */
  
  iOffset   = nchars;			/* LAJ goes to this "address" */
  iOffset  -= iBranchNchars;		/* calculate distance to start of branches */
  iOffset >>= 2;			/* convert from bytes to words */

  /* generate the code to initialise the branch stub addresses */
  
  while (iBranchCount)
    {
      trace( "Offset is %d", iOffset );
      
      if (fits_in_8_bits( iOffset ))
	{
	  outword( 0x37191f00 | (iOffset & 0xFF) );	/* SUBI3 off, R11, RS  */
	}
      else
	{
	  outword( 0x0819001f );			/* LDI  R11, RS */
	  outword( 0x18790000 | (iOffset & 0xFFFF) );	/* SUBI off, RS */
	}
      
      outword( 0x15592501 );			/* STI   RS, *AR5++(1) */
      
      iOffset      -= 1;
      iBranchCount -= 4;
    }
  
  /* finish the initialisation code */
  
  outword( 0x6800000a );	/* Bu	 AR2           */
  
  /* adjust code size to include init code */
  
  totalcodesize += (nchars - iInitNchars);
  
  return;
  
} /* GenerateInitCode */

/*}}}*/

/*}}}*/
#else /* ! __C40 */
#error Unknown processor type for stub creation
#endif /* __C40 */

/*{{{  Generic Functions */

/*{{{  InitStubs */

PRIVATE void
InitStubs( void )
{
  asm_Module *	pModule;
  word		i;
  

  /* adjust codesize to include the stub module */
  
  totalcodesize += sizeof (Module);

  /* initialise global variables */
  
  stub_list       = NullVMRef;
  vBranchStubList = NullVMRef;
  stub_heap       = VMPage();
  vStubModule     = VMalloc( sizeof (asm_Module), stub_heap );
  
  if (NullRef( vStubModule ))
    {
      trace( "URG - out of memory create StubModule\n" );
    }

  /* fill in the stub module structure */
  
  pModule = VMAddr( asm_Module, vStubModule );
  
  VMDirty( vStubModule );
  
  pModule->next      = NullVMRef;
  pModule->start     = NullVMRef;
  pModule->end	     = NullVMRef;
  pModule->refs      = NullVMRef;
  pModule->id        = ++lastm;
  pModule->linked    = TRUE;
  pModule->length    = 0;
  pModule->file_name = LINKER_MODULE_NAME;
  
   for ( i = 0; i < LOCAL_HASHSIZE ; i++ )
     {
       pModule->symtab[ i ].head    = NullVMRef,
       pModule->symtab[ i ].entries = 0;
     }

  trace( "Linker's fake module number is %d", lastm );
  
  /* note that we have performed the initialisation */
  
  inited = 1;

  trace( "Stubs initialised" );

  flush_output_buffer();

  iCallingStubStart = totalcodesize;
  
  return;
  
} /* InitStubs */

/*}}}*/
/*{{{  new_stub */

/*
 * Add a request for a stub calling 'symbol'.
 * Returns the byte offset from the start of the
 * file of where the calling stub will be located.
 */

WORD
new_stub(
	 VMRef	symbol,
	 bool	addr_stub )	/* true if this is an address stub */
{
  VMRef		name_sym;
  VMRef		module;  
  VMRef		s;
  Stub *	stub;
  Symbol *	sym;
  asm_Module *	mod;
  WORD		offset;
  WORD		id;
  WORD		pos;
  StubType	stub_type;
  char *	sname;
  word		iSize;
  

  if (!inited)
    InitStubs();
  
  sname = VMAddr( Symbol, symbol )->name;
  
  trace( "new %s stub request for %s",
	addr_stub ? "address" : "ordinary",
	sname );
  
  name_sym = symbol;
  
  /* search stub list for an already existing stub */
  
  s = stub_list;
  
  while (!NullRef( s ))
    {
      stub = VMAddr( Stub, s );
      
      if (VMSame( stub->name_sym, name_sym ))
	break;
      
      s = stub->next;
    }
  
  /* check the result of the search */
  
  if (!NullRef( s ))
    {
      stub = VMAddr( Stub, s );
      
      trace( "stub already exists at %d", stub->position );
      
      /* stub has already been allocated, just return its location */
      
      return stub->position;
    }
  
  /* no stub found - create a new one */
  
  if (!NullRef( stub_heap ))
    s = VMalloc( sizeof (Stub), stub_heap );
  
  if (NullRef( s ))
    {
      /* run out of space in current heap - get a new one */
	  
      stub_heap = VMPage();
      s         = VMalloc( sizeof (Stub), stub_heap );	  
    }
  
  if (addr_stub)
    {
      char	name[ 128 ];		/* XXX */
      
      
      /* default type */
      
      stub_type = LOCAL_ADDR_STUB;
      
      /* get hold of the symbol */
      
      sym = VMAddr( Symbol, symbol );
      
      /* duplicate name leaving off the "_addr" bit */
      
      strcpy( name, ((char *)&(sym->name[ 0 ])) + 5 );
      
      /* find the associated shared library symbol */
      
      symbol = lookup( name );
      
      if (symbol == NullVMRef)
	{
	  /* Could not find ".<symbol>" - hence it should be in a resident library */
	  
	  stub_type = SHARED_ADDR_STUB;
	}
      else
	{
	  /* check the type of the symbol */
      
	  sym = VMAddr( Symbol, symbol );

	  if (sym->type != S_CODESYMB)
	    {
	      /* ".<symbol> exists but it is not a code symbol - hence it should be ignored */
	      
	      stub_type = SHARED_ADDR_STUB;
	    }
	}

      if (stub_type == SHARED_ADDR_STUB)
	{
	  name[ 0 ] = '_';

	  trace( "looking up shared symbol %s", name );
	  
	  symbol = lookup( name );

	  if (symbol == NullVMRef)
	    {
	      warn( "Attempting to take address of unknown function %s", name + 1 );

	      return 0;
	    }

	  trace( "stub is in shared library" );
	}
      else
	{
	  trace( "stub is local" );
	}
      
      /* check the type of the symbol */
      
      sym = VMAddr( Symbol, symbol );
      
      if (sym->type == S_UNBOUND)
	{
	  warn( "Address of function \"%s\" is used, but it is not defined", name );
	  
	  return 0;
	}
	
      if (stub_type == LOCAL_ADDR_STUB && sym->type != S_CODESYMB)
	{
	  warn( "Address of function \"%s\" is used, but defined in '%s' as something else (%x)",
	       name + 1, sym->file_name, sym->type );
	  
	  return 0;
	}
      else if (stub_type == SHARED_ADDR_STUB && sym->type != S_FUNCDONE)
	{
	  warn( "Address of function \"%s\" is used, but defined in '%s' as something else (%x)",
	       name + 1, sym->file_name, sym->type );
	  
	  return 0;
	}
      
      sym->referenced = 1;
    }
  else
    {
      stub_type = CALLING_STUB;
      
      /* get hold of the symbol */
      
      sym = VMAddr( Symbol, symbol );

      if (sym->type != S_FUNCDONE)
	{
	  warn( "Function Stub \"%s\" is type %x", sym->name, sym->type );

	  return 0;
	}
    }

  sname = sym->name;
  
  trace( "creating a new %s stub for %s",
	addr_stub ? "address" : "ordinary",
	sname );
  
  /* get hold of the symbol's byte offset within it's module */
  
  offset = sym->value.w;

  trace( "%s's offset is %x", sym->name, offset >> 2 );

  module = sym->module;
  
  if (NullRef( module ))
    {
      trace( "symbol has no module!" );

      return 0;      
    }

  /* get the id of the module containing the symbol */
  
  mod  = VMAddr( asm_Module, module );

  id   = mod->id;

  if (id == -1)
    warn( "Module %s containing function %s is not linked!", mod->file_name, sname );

  if (smtopt)
    id = id * 2 + 1;
  
  trace( "symbol's module id is %d, slot number %d", mod->id, id );
  
  stub = VMAddr( Stub, s );

  VMDirty( s );
  VMlock(  s );
      
  /* fill in stub */

  flush_output_buffer();
  
  pos = iCallingStubStart;

  trace( "stub's position is %d", pos );
  
  stub->symbol   = symbol;
  stub->name_sym = name_sym;
  stub->position = pos;
  stub->id       = id;
  stub->offset   = offset;
  stub->type     = stub_type;
  
  /* add stub to chain */
      
  stub->next   = stub_list;
  stub_list    = s;
  
  /* adjust code size to account for (as yet uncreated) stub */

  iSize = stub_size( stub );
  
  totalcodesize     += iSize;
  totalstubsize     += iSize;
  iCallingStubStart += iSize;
  
  trace( "stub's size is %d", stub_size( stub ) );
  
  /* release lock on stub */
      
  VMunlock( s );

  /* return the location of the stub */
  
  return pos;
  
} /* new_stub */ 

/*}}}*/
/*{{{  RequestBranchStub */

/*
 * Add a request for a stub that branches to 'symbol'
 * and which has an address stored in a module's
 * codetable.  This will allow code that has been
 * moved in memory (because of AccelerateCode()),
 * to call functions outside of itself.  The symbol
 * does not yet have a module associated with it,
 * so fill this in.
 */

void
RequestBranchStub( VMRef vSymbol )
{
  VMRef		vBranchStub;
  Symbol *	pSymbol;
  BranchStub *	pBranchStub;
  Code *	pCode;
  word		iSize;
  
  
  if (!inited)
    InitStubs();

  trace( "Request for a branch stub for %s",
	 VMAddr( Symbol, vSymbol )->name );

  /* create new new Branch stub entry */
  
  if (!NullRef( stub_heap ))
    vBranchStub = VMalloc( sizeof (BranchStub), stub_heap );
  
  if (NullRef( vBranchStub ))
    {
      /* run out of space in current heap - get a new one */
	  
      stub_heap   = VMPage();
      vBranchStub = VMalloc( sizeof (BranchStub), stub_heap );	  
    }
  
  pBranchStub = VMAddr( BranchStub, vBranchStub );

  VMDirty( vBranchStub );
  VMlock(  vBranchStub );
      
  /* fill in branch stub */

  pBranchStub->vSymbol = vSymbol;
  pBranchStub->iOffset = iBranchCount;

  trace( "Branch stub's count is %d", iBranchCount );
  
  /* add stub to chain */
      
  pBranchStub->vNext = vBranchStubList;
  vBranchStubList    = vBranchStub;
  
  /* release lock on stub */
      
  VMunlock( vBranchStub );

  /* now fill in the symbol's information */
     
  pSymbol = VMAddr( Symbol, vSymbol );

  VMDirty( vSymbol );
  VMlock(  vSymbol );
  
  pSymbol->type      = S_FUNCDONE;
  pSymbol->module    = vStubModule;
  pSymbol->file_name = LINKER_MODULE_NAME;
  pSymbol->value.w   = iBranchCount;
  
  pCode = VMAddr( Code, pSymbol->value.v );

  if (NullRef( pCode ))
    {
      warn( "Stubs: cannot find Code structure for branch stub" );
    }
  else
    pCode->loc = iBranchCount;
 
  VMunlock( vSymbol );

  /* increment the branch count */

  pBranchStub = VMAddr( BranchStub, vBranchStub );

  iSize = branch_size( pBranchStub );
  
  iBranchCount += iSize;
  
  /* adjust code size to account for (as yet uncreated) stub */

  totalcodesize += iSize;

  return;
  
} /* RequestBranchStub */ 

/*}}}*/
/*{{{  build_stubs */

/*
 * Called at the end of the linking process.
 * This creates all the necessary calling stubs,
 * and appends them to the end of the code
 */

PUBLIC void
build_stubs( void )
{
#if (defined HOSTISBIGENDIAN && defined __BIGENDIAN) || (!defined HOSTISBIGENDIAN && !defined __BIGENDIAN)
  word *     pModuleName = (word *)"StubModule\0                     ";/* 32 bytes including \0 */
#else
  word *     pModuleName = (word *)"butSudoM \0el                    ";/* 32 bytes including \0 */
#endif
  VMRef		prev;  
  VMRef		s;
  asm_Module *	pModule;
  word	        iModuleSize;
  

  if (!inited)
    return;

  /*{{{  Build Module Header */

  trace( "building module header" );
  
  flush_output_buffer();
  lseek( outf, (off_t)-sizeof (WORD), SEEK_END );	/* over-write trailing zero word */
  nchars -= sizeof (WORD);				/* adjust output count */
  iModuleNchars = nchars;				/* remember start "address" of module */
  
  /* build module structure */

  pModule = VMAddr( asm_Module, vStubModule );
      
  outword( T_Module );
  flush_output_buffer();
  iModuleSize = nchars + 3 * sizeof (WORD);
  outword( 0 );	    		/* size in bytes of module */
  outword( pModuleName[ 0 ] );	/* Name */
  outword( pModuleName[ 1 ] );
  outword( pModuleName[ 2 ] );
  outword( pModuleName[ 3 ] );
  outword( pModuleName[ 4 ] );
  outword( pModuleName[ 5 ] );
  outword( pModuleName[ 6 ] );
  outword( pModuleName[ 7 ] );
  outword( pModule->id );	/* Id */
  outword( 0x1 );		/* Version */
#ifdef __SMT
  outword( 0 );			/* MaxData */
  if (iBranchCount > 0)
    outword( totalstubsize + iBranchCount + 2 * sizeof (WORD) );	/* Root of Init Chain */
  else
    outword( 0 );		/* No Init Chain */
  outword( iBranchCount );	/* MaxCodeP */
#else
  outword( iBranchCount );	/* MaxData */
  if (iBranchCount > 0)
    outword( totalstubsize + iBranchCount + 2 * sizeof (WORD) );	/* Root of Init Chain */
  else
    outword( 0 );		/* No Init Chain */
#endif

  /*}}}*/

  /*{{{  Build Calling Stubs */

  /* scan stub list reversing pointers ! */

  prev = NullVMRef;
  s    = stub_list;
  
  while (!NullRef( s ))
    {
      VMRef	next;      
      Stub *	stub;


      stub = VMAddr( Stub, s );

      next = stub->next;

      stub->next = prev;

      prev = s;

      s = next;      
    }

  trace( "building calling stubs" );
  
  s = stub_list = prev;
  
  /* now scan stub list building stubs */
  
  while (!NullRef( s ))
    {
      build_stub( s );
      
      s = VMAddr( Stub, s )->next;
    }

  trace( "built calling stubs" );

  /*}}}*/

  /*{{{  Build  Branch Stubs */

  /* get hold of the last module known to the linker */
  
  pModule = VMAddr( asm_Module, tailmodule );

  VMDirty( tailmodule );

  /* set the next pointer to point to the stub module */

  tailmodule = pModule->next = vStubModule;

  /* make the stub module be the last module */
  
  flush_output_buffer();
  curmod = tailmodule = vStubModule;
  codepos = 0;
  iBranchNchars = nchars;

  trace( "building branch stubs" );
  
  /* scan branch stub list reversing pointers ! */

  prev = NullVMRef;
  s    = vBranchStubList;
  
  while (!NullRef( s ))
    {
      VMRef		vNext;      
      BranchStub *	pBranchStub;


      pBranchStub = VMAddr( BranchStub, s );

      vNext = pBranchStub->vNext;

      pBranchStub->vNext = prev;

      prev = s;

      s = vNext;      
    }

  /* build the Branch stubs */

  for (s = vBranchStubList = prev;
       !NullRef( s );
       s = VMAddr( BranchStub, s )->vNext)
    {
      GenerateBranchStub( s );
    }

  if (iBranchCount)
    {
      /* Generate Module initialisation code */

      outword( 0 ); /* end of init chain */
      totalcodesize += sizeof (WORD);

      GenerateInitCode();
    }

  trace( "built branch stubs" );

  /*}}}*/

  /*{{{  Tidy Up Loose Ends */

  trace( "finishing up" );

  /* add the final zero word to finish off the module chain */
  
  outword( 0 );

  /* make sure that everything has been written out */
  
  flush_output_buffer();

  /* initialise the module structure's size field */

  if (lseek( outf, (off_t)(iModuleSize), SEEK_SET ) != iModuleSize)
    trace( "OH HELL" );

  outword( nchars - iModuleNchars - sizeof (WORD) );
  flush_output_buffer();

  if (gen_image_header)
    {
      /* confirm value of totalcodesize */
  
      if (lseek( outf, (off_t)0, SEEK_END ) != totalcodesize + 4 * sizeof (WORD))
	warn( "Stubs: end of file = %ld, should be = %ld",
	      lseek( outf, (off_t)0, SEEK_END ),
	      totalcodesize + 4 * sizeof (WORD) );
 
      /* finally update image with correct size */

      trace( "resetting code size to %d", totalcodesize + sizeof (WORD) );
  
      (void) lseek( outf, (off_t) 2 * sizeof (WORD), SEEK_SET );
      
      outword( totalcodesize + 1 * sizeof (WORD) );	/* remember trailing zero */
    }

  flush_output_buffer();

  trace( "stubs module built" );

  /*}}}*/

  return;
  
} /* build_stubs */

/*}}}*/

#ifdef SUN4
/*{{{  SUN4 functions */

/*
 * The gcc compiler appears to generate references to the following functions
 * without providing them in a standard library.  Since the code for the
 * linker does not use either of them, they are provided here as stubs
 */

int ___assert( void ) { return 0; }
int ___typeof( void ) { return 0; }  

/*}}}*/
#endif /* SUN4 */

/*}}}*/

/*}}}*/
#endif /* NEW_STUBS */

/* end of stubs.c */
