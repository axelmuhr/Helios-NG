/****************************************************************/
/* File: mem.c                                                  */
/*                                                              */
/*                                                              */
/* Author: NHG 19-Feb-87                                        */
/****************************************************************/
/* $Id: mem.c,v 1.5 1992/09/25 10:41:21 paul Exp $ */

#include "asm.h"

#define trace if(traceflags&db_mem)_trace

PUBLIC  WORD  	heapsize;		/* amount of heap memory used   */

PRIVATE VMRef 	vmheap;			/* virtual heap			*/

PUBLIC VMRef 	codeseg;		/* start of current segment	*/

PUBLIC WORD 	codesize;		/* amount of code memory used   */

PUBLIC WORD 	exprsize;		/* amount of memory used by exprs */

PUBLIC WORD 	codeflags = 0;		/* flags for code.flags field	*/

#define HEAPDELTA	20000

#ifdef MWC
UBYTE *lmalloc();
#define MALLOC(n) (UBYTE *)lmalloc(n)
#else
#ifdef IBMPC
UBYTE *getml();
#define MALLOC(n) (UBYTE *)getml(n)
#else
#ifdef __STDC__
void *malloc(int n);
#else
byte *malloc();
#endif
#define MALLOC(n) (UBYTE *)malloc(n)
#endif
#endif

#ifndef wordlen
#define wordlen( s ) (((s) + 3) & ~3)
#endif

/********************************************************/
/* initmem                                              */
/*                                                      */
/* initialise memory system                             */
/********************************************************/

PUBLIC void
initmem()
{
  VMInit( NULL, vmpagesize );
	
  codeseg  = VMPage();
  vmheap   = VMPage();
  codesize = 0;
  exprsize = 0;

  return;
}

/********************************************************/
/* alloc                                                */
/*                                                      */
/* allocate n bytes from the heap                       */
/********************************************************/

PUBLIC UBYTE *
alloc( n )
  INT 		n;
{
  UBYTE *	v;


  v = MALLOC( n );

  if ( v == NULL )
    error( "Cannot get local heap space" );

  heapsize += n;
	
  return v;
}

/********************************************************/
/* unary                                                */
/* binary                                               */
/*                                                      */
/* routines for building expression tree nodes          */
/*                                                      */
/********************************************************/

PUBLIC VMRef
unary( ntype, etype, expr )
  WORD ntype, etype;
  WORD expr;
{
  VMRef 	v    = VMNew( sizeof (Unode) );
  Unode *	node = VMAddr( Unode, v );

  
  node->ntype  = ntype;
  node->etype  = etype;
  node->expr.w = expr;
  
  exprsize += sizeof (Unode);
  
  trace( "unary %x %x : %x %x", v, node, etype, expr );

  return v;
}

PUBLIC VMRef
binary( ntype, ltype, rtype, lexp, rexp )
  WORD ntype,ltype,rtype;
  WORD lexp,rexp;
{
  VMRef 	v    = VMNew( sizeof (Bnode) );
  Bnode *	node = VMAddr( Bnode, v );

  node->ntype  = ntype;
  node->ltype  = ltype;
  node->rtype  = rtype;
  node->lexp.w = lexp;
  node->rexp.w = rexp;
  
  trace( "binary %x %x : l %x %x r %x %x", v, node, ltype, lexp, rtype, rexp );
  
  exprsize += sizeof (Bnode);
  
  return v;
}

/********************************************************/
/* newcode                                              */
/*                                                      */
/* allocates and initialise a new code structure        */
/*                                                      */
/********************************************************/

extern WORD lineno;

PUBLIC VMRef
newcode( type, size, vtype, loc, value )
  WORD 		type;
  WORD 		size;
  WORD		vtype;
  WORD 		loc;
  WORD 		value;
{
  Code *	p;
  VMRef 	v;
	

  codeptr();	/* ensure there is space */
	
  v = VMalloc( sizeof (Code), codeseg );
	
  p = VMAddr( Code, v );
		
  trace( "newcode: %8lx : %2lx %3ld %3ld %8lx", p, type, size, vtype, value );

  p->type    = type;
  p->size    = size;
  p->vtype   = vtype;
  p->flags   = codeflags; codeflags = 0;
  p->loc     = loc;
  p->value.w = value;

#ifdef LINENO
  p->line    = lineno;
#endif

  codesize += sizeof (Code);
	
  VMDirty( v );

  return v;
}

PUBLIC VMRef
codeptr()
{	
  /* if this would be the last code entry in the segment, get a	*/
  /* new block and add a s_newseg entry.				*/

  if ( VMleft(codeseg) < sizeof (Code) * 2 )
    {
      VMRef 	newseg  = VMPage();
      VMRef 	v       = VMalloc( sizeof (Code), codeseg );
      Code *	codetop = VMAddr( Code, v );

      
      if ( NullRef( newseg ) )
	error( "Cannot get code segement" );

      codetop->type  = s_newseg;
      codetop->size  = 0;
      codetop->vtype = 0;
#ifdef LINENO
      codetop->line  = lineno;
#endif
      codetop->value.v = newseg;
      codetop->loc     = -1;
      
      codesize += sizeof (Code);
      
      trace( "code extension: %lx", codeseg );
      
      VMDirty( v ) ;
      
      codeseg = newseg;
    }

  return VMnext( codeseg );
}

extern VMRef
VMNew( size )
  int size;
{
  VMRef v;


  size = wordlen( size );

  v = VMalloc( size, vmheap );
	
  if ( NullRef( v ) ) 
    {
      vmheap = VMPage();
      
      v = VMalloc( size, vmheap );
    }

  VMDirty( v );
  
  return v;
}

