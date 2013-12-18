/****************************************************************/
/* Helios Linker					     	*/
/*								*/
/* File: sym.c                                                  */
/*                                                              */
/* Routines to manipulate the symbol table                      */
/*                                                              */
/* Author: NHG 17-Feb-87                                        */
/****************************************************************/
/* RcsId: $Id: sym.c,v 1.7 1993/01/15 14:21:54 nick Exp $ */

#include "link.h"
#include <ctype.h>

#define trace if (traceflags & db_sym) _trace

PRIVATE VMRef	 symheap;
PRIVATE VMRef	 globheap;
PRIVATE STEntry	 Symtab[ GLOBAL_HASHSIZE ];
PUBLIC WORD	 symsize;

#ifdef __STDC__
WORD hash(char *);
#else
WORD hash();
#endif

/********************************************************/
/* initsym                                              */
/*                                                      */
/* Initialize the symbol table.                         */
/*                                                      */
/********************************************************/

PUBLIC void
initsym( void )
{
  int i;


  for (i = 0; i < GLOBAL_HASHSIZE ; i++)
    {
      Symtab[ i ].head    = NullVMRef;
      Symtab[ i ].entries = 0;
    }
  
  symheap  = VMPage();
  globheap = VMPage();
  symsize  = 0;

  return;
  
} /* initsym */


/********************************************************/
/* lookup                                               */
/*                                                      */
/* Lookup a symbol in the hash tables.                  */
/* it checks the local, module symbol table first,      */
/* followed by the global one.                          */
/* Returns the symbol if present, otherwise NULL.       */
/*                                                      */
/********************************************************/

PUBLIC VMRef
lookup( BYTE * name )
{
  UWORD		hs = hash(name);
  UWORD		h  = (hs % LOCAL_HASHSIZE);  
  VMRef		sym;
  int		loops = 0;
  int		entries = 0;
  asm_Module *	m;
	
  if (!VMSame( curmod, module0 ))
    {
      m       = VMAddr( asm_Module, curmod );
      sym     = m->symtab[ h ].head;
      entries = m->symtab[ h ].entries;
		
      while ( !NullRef( sym ) )
	{
	  Symbol *	s;

	  
	  s = VMAddr( Symbol, sym );
	  
	  trace( "mod lookup #%d sym %x %s == %s", loops, sym, name, s->name );
	  
	  if ( !VMSame( curmod, s->module ) || s->global ) 
	    warn( "global or external symbol found in local symbol table: %s", s->name );
	  
	  if (eqs( name, s->name ))
	    {
	      trace("mod lookup sym %s found",name);
	      
	      if (loops > entries)
		report("Warning: found %s: but loops %d > entries %d\n", name, loops, entries );
	      
	      return sym;
	    }
	  
	  sym = s->next;
	  
#ifdef dubious
	  if ( loops++ == entries + 10 )
	    {
	      report( "possible loop in symtab loop = %d entries = %d", loops, entries );
	      
	      traceflags |= db_sym;	      
	    }
	  else if ( loops == entries*2 ) error("quitting");
#else	  
	  loops++;
#endif	  
	}
      
      if (loops > entries)
	report( "Warning: didn't find %s and loops %d > entries %d", name, loops, entries );
    }
  
  h = (hs % GLOBAL_HASHSIZE);
  
  sym     = Symtab[ h ].head;
  loops   = 0;
  entries = Symtab[ h ].entries;
  
  while ( !NullRef(sym) )
    {
      Symbol *	s;


      s = VMAddr( Symbol, sym );
      
      trace( "glob lookup sym %x %s == %s", sym, name, s->name );
      
      if (!s->global && !VMSame( curmod, module0 ))
	warn( "local symbol found in global symbol table: %s", s->name );
      
      if ( eqs( name, s->name ) )
	{
	  trace( "glob lookup sym %s found", name );
	  
	  return sym;
	}
      
      sym = s->next;
      
      if (loops++ == entries + 10)
	{
	  report( "possible loop in global symtab loops = %d entries = %d",
		 loops, entries );
	  
	  traceflags |= db_sym;
	}
      else if (loops == 2 * entries)
	error( "quitting" );
    }
  
  trace( "lookup failed for %s", name );
  
  return NullVMRef;

} /* lookup */


/********************************************************/
/* insert                                               */
/*                                                      */
/* insert a symbol in the table.                        */
/*                                                      */
/********************************************************/

PUBLIC VMRef
insert(
       BYTE *	name,
       WORD	local )
{
  UWORD 	h = hash(name);
  VMRef	    	sym;
  Symbol *	s;
  asm_Module *	m;
  int		size = sizeof(Symbol) + strlen(name);

  
  if (local)
    {
      sym = VMalloc( size, symheap );
      
      if ( NullRef(sym) ) 
	{
	  symheap = VMPage();
	  sym     = VMalloc( size, symheap );
	}
    }
  else
    {
      sym = VMalloc( size, globheap );
      
      if ( NullRef(sym) ) 
	{
	  globheap = VMPage();
	  sym      = VMalloc( size, globheap );
	}
    }
  
  symsize += size;
  
  trace("insert %s %d sym = %x",name,local,sym);
  
  s = VMAddr( Symbol, sym );
  
  VMDirty( sym );
  VMlock(  sym );
  
  strcpy( s->name, name );
  
  s->type       = 0;
  s->value.w    = 0;
  s->referenced = FALSE;
#ifdef NEW_REF
  s->fwd_ref    = NullVMRef;
#endif
  
  if ( local && !VMSame( curmod, module0 ) )
    {
      VMRef headref;

      
      m = VMAddr( asm_Module, curmod );
      
      h %= LOCAL_HASHSIZE;
      
      headref = m->symtab[ h ].head;
      
      s->next   = headref;
      s->prev   = NullVMRef;
      s->module = curmod;
      s->global = FALSE;
      
      if ( !NullRef(headref) )
	{
	  Symbol *	head = VMAddr( Symbol, headref );
	  
	  head->prev = sym;
	  
	  VMDirty(headref);
	}
      
      m = VMAddr( asm_Module, curmod );	/* just in case */
      
      m->symtab[ h ].head = sym;
      m->symtab[ h ].entries++;
      
      VMDirty(curmod);
    }
  else
    {
      VMRef headref;
      
      
      h %= GLOBAL_HASHSIZE;
      
      headref  = Symtab[ h ].head;
      
      s->next   = headref;
      s->prev   = NullVMRef;
      s->module = NullVMRef;
      s->global = TRUE;        	
      
      if ( !NullRef(headref) )
	{
	  Symbol *	head = VMAddr( Symbol, headref );

	  
	  head->prev = sym;
	  
	  VMDirty(headref);
	}
      
      Symtab[ h ].head = sym;
      Symtab[ h ].entries++;
    }
  
  VMunlock( sym );
  
  return sym;

} /* insert */


/********************************************************/
/* movesym                                              */
/*                                                      */
/* Remove symbol from current symbol table and place    */
/* in global table.                                     */
/*                                                      */
/********************************************************/

PUBLIC void
movesym( VMRef sym )
{
  UWORD		hs;
  UWORD 	h;
  Symbol *	s;
  asm_Module *	m;

  
  s = VMAddr( Symbol, sym );
  
  if ( s->global || VMSame( s->module, module0 ))		/* already moved */
    {
      s->global = TRUE;
      
      VMDirty( sym );
      
      return;
    }
  
  hs = hash( s->name );  
  h  = (hs % LOCAL_HASHSIZE);
  
  /* else move symbol from local to global table */
  
  VMlock( sym );
  
  trace( "movesym ref %x name %s links %x %x", sym, s->name, s->next, s->prev );
  
  if ( NullRef( s->prev ) )
    {
      /* at head of list, step list head pointer over us */
      
      m = VMAddr( asm_Module, s->module );
      
      m->symtab[ h ].head = s->next;
      m->symtab[ h ].entries--;
    }
  else 
    {
      Symbol *	prev = VMAddr( Symbol, s->prev );


      /* else step prev's next pointer over us */
      
      prev->next = s->next;
      
      VMDirty( s->prev );
      
      m = VMAddr( asm_Module, s->module );
      
      m->symtab[ h ].entries--;
    }
      
  VMDirty( s->module );

  if ( !NullRef(s->next) )
    {
      Symbol *	next = VMAddr( Symbol, s->next );

      
      /* if not at tail of list, patch next's prev pointer */
      
      next->prev = s->prev;
      
      VMDirty( s->next );
    }	
  
  h = (hs % GLOBAL_HASHSIZE);
  
  /* finally set up our pointers */
  
  s->next   = Symtab[ h ].head;
  s->prev   = NullVMRef;
  s->global = TRUE;
  
  VMDirty( sym );
  
  if ( !NullRef( Symtab[ h ].head ) )
    {
      Symbol *	head = VMAddr( Symbol, Symtab[ h ].head );
      

      /* if list is not empty, patch prev of old head */
      
      head->prev = sym;
      
      VMDirty( Symtab[ h ].head );
    }
  
  Symtab[ h ].head = sym;
  Symtab[ h ].entries++;
  
  VMunlock( sym );

  return;

} /* movesym */


PUBLIC void
cps(
    BYTE *	src,
    BYTE *	dst )
{
  strcpy( dst, src );

} /* cps */
