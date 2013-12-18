/****************************************************************/
/* File: sym.c                                                  */
/*                                                              */
/* Routines to manipulate the symbol table                      */
/*                                                              */
/* Author: NHG 17-Feb-87                                        */
/****************************************************************/
/* $Id: sym.c,v 1.9 1994/08/09 16:43:25 al Exp $ */

#include "asm.h"
#include <ctype.h>

#define static

#define trace if(traceflags&db_sym) _trace

PRIVATE VMRef symheap;
PRIVATE VMRef globheap;

PUBLIC STEntry Symtab[ GLOBAL_HASHSIZE ];
PUBLIC WORD symsize;

#ifdef __DOS386
UWORD hash(char *s);
#else /* !__DOS386 */
UWORD hash(ellipsis);
#endif /* __DOS386 */

/********************************************************/
/* initsym                                              */
/*                                                      */
/* Initialize the symbol table.                         */
/*                                                      */
/********************************************************/

PUBLIC void initsym()
{
	int i;
	
	for (i = 0; i < GLOBAL_HASHSIZE ; i++)
	{
		Symtab[i].head = NullVMRef;
		Symtab[i].entries = 0;
	}
        
	symheap = VMPage();
	globheap = VMPage();
	symsize = 0;
}

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
lookup( name )
  BYTE *	name;
{
  UWORD		hs = hash( name );
  UWORD 	h  = (hs % LOCAL_HASHSIZE);
  VMRef 	sym;
  int 		loops = 0;
  int 		entries = 0;
  asm_Module *	m;
  

/*  trace( "looking for %s", name ); */
  
  if ( !VMSame( curmod, module0 ) )
    {
      m       = VMAddr( asm_Module, curmod );
      
      sym     = m->symtab[ h ].head;
      entries = m->symtab[ h ].entries;
      
      while ( !NullRef( sym ) )
	{
	  Symbol *	s;
	  
	  if ( !VMcheck( sym ) ) 
	    error( "badvm l %x %x %s %d", sym, curmod, name, h );
	  
	  s = VMAddr( Symbol, sym );
	  
	  trace( "mod lookup sym %x %s == %s", sym, name, s->name );
	  
	  if ( !VMSame( curmod, s->module ) || s->global ) 
	    warn( "global or external symbol found in local symbol table: %s", s->name );
	  
	  if ( eqs( name, s->name ) )
	    {
	      return sym;
	    }
	  
	  sym = s->next;
	  
	  if ( loops++ == entries + 10 )
	    {
	      report( "possible loop in symtab loop = %d entries = %d", loops, entries );
	      
	      traceflags |= db_sym;
	    }
	  else if ( loops == entries * 2 )
	    error( "quitting" );
	}
    }
  
  h = (hs % GLOBAL_HASHSIZE);
  
  sym     = Symtab[ h ].head;
  loops   = 0;
  entries = Symtab[ h ].entries;

/*  trace( "checking global table, h = %lu", h ); */
  
  while ( !NullRef( sym ) )
    {
      Symbol *	s;
      
      
      if ( !VMcheck( sym ) ) 
	error( "badvm g %x %x %s %d", sym, curmod, name, h );
      
      s = VMAddr( Symbol, sym );
      
      trace( "glob lookup sym %x %s == %s", sym, name, s->name );
      
      
      if ( !s->global && !VMSame( curmod, module0 ) )
	warn( "local symbol found in global symbol table: %s", s->name );
      
      if ( eqs( name, s->name ) )
	{
	  return sym;
	}
      
      sym = s->next;
      
      if ( loops++ == entries + 10 )
	{
	  report( "possible loop in global symtab loops = %d entries = %d", loops, entries );
	  
	  traceflags |= db_sym;
	}
      else if ( loops == 2 * entries )
	error( "quitting" );
    }

/*  trace( "lookup: failed" ); */
  
  return NullVMRef;
}

/********************************************************/
/* insert                                               */
/*                                                      */
/* insert a symbol in the table.                        */
/*                                                      */
/********************************************************/

PUBLIC VMRef insert(name,local)
BYTE *name;
WORD local;
{
        UWORD h = hash(name);
        VMRef sym;
	Symbol *s;
	asm_Module *m;
	int size = sizeof(Symbol) + strlen(name);

	if( local )
	{
		sym = VMalloc(size,symheap);

		if( NullRef(sym) ) 
		{
 			symheap = VMPage();
			sym = VMalloc(size,symheap);
		}
	}
	else
	{
		sym = VMalloc(size,globheap);

		if( NullRef(sym) ) 
		{
 			globheap = VMPage();
			sym = VMalloc(size,globheap);
		}
	}

	symsize += size;

	trace("insert %s %d sym = %x",name,local,sym);

	s = VMAddr(Symbol,sym);
	VMDirty(sym);
	VMlock(sym);
	
        strcpy(s->name,name);
	
        s->type       = 0;
        s->def.w      = NULL;
	s->referenced = 0;

#ifdef NEW_REF
	s->fwd_ref = NullVMRef;
#endif
	
        if( local && !VMSame(curmod,module0) )
        {
        	VMRef headref;
        	m = VMAddr(asm_Module,curmod);
        	
		h %= LOCAL_HASHSIZE;
	  
        	headref = m->symtab[h].head;

        	s->next = headref;
        	s->prev = NullVMRef;
	        s->module = curmod;
		s->global = FALSE;

		if( !NullRef(headref) )
		{
	        	Symbol *head = VMAddr(Symbol,headref);
        		head->prev = sym;
        		VMDirty(headref);
        	}

		m = VMAddr(asm_Module,curmod);	/* just in case */        	
		m->symtab[h].head = sym;
		m->symtab[h].entries++;
		VMDirty(curmod);
        }
        else
        {
	  VMRef headref;
	  

		h %= GLOBAL_HASHSIZE;

		headref  = Symtab[h].head;
	  
        	s->next = headref;
        	s->prev = NullVMRef;
	        s->module = NullVMRef;
		s->global = TRUE;        	
		
		if( !NullRef(headref) )
		{
	        	Symbol *head = VMAddr(Symbol,headref);
        		head->prev = sym;
        		VMDirty(headref);
        	}
        	Symtab[h].head = sym;
        	Symtab[h].entries++;
        }

	VMunlock(sym);
        return sym;
}

/********************************************************/
/* movesym                                              */
/*                                                      */
/* Remove symbol from current symbol table and place    */
/* in global table.                                     */
/*                                                      */
/********************************************************/

PUBLIC void
movesym( sym )
  VMRef sym;
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
}

/********************************************************/
/* show_unref						*/
/*							*/
/* print a warning for each unreferenced symbol defined	*/
/* in the program.					*/
/********************************************************/

void show_unref()
{
	STEntry *tab = Symtab;
	int h;
	VMRef sym;
		
	for( h = 0; h < GLOBAL_HASHSIZE; h++ )
	{
        	sym = tab[h].head;

                while ( !NullRef(sym) )
                {
                	Symbol *s;
			asm_Module *m = NULL;
			char *type;

			if( !VMcheck(sym) ) error("badvm %x %d",sym,h);

			VMlock(sym);

			s = VMAddr(Symbol,sym);
#if 0
_trace("%x %x t %x g %d r %d m %x d %d %s",sym,s,s->type,s->global,s->referenced,
			s->module,s->def,s->name);
#endif
			if( !NullRef(s->module) ) m = VMAddr(asm_Module,s->module);			

			switch( s->type )
			{
			case s_datasymb:
			case s_datadone:
					type = "data"; break;
			case s_commsymb:
			case s_commdone:
					type = "common"; break;
			case s_codesymb:
					type = "code"; break;
			default:
					type = "unknown"; break;
			}

			if( s->referenced == 0 && m != NULL && 
			    m->linked && m->id == -1)
			{
				strcpy(infile,m->file);
				warn(" %s symbol %s unreferenced",type,s->name);
			}

			VMunlock(sym);
                        sym = s->next;
                }
        }
}

/********************************************************/
/* printtab						*/
/*							*/
/* print out symbol table				*/
/********************************************************/

void printtab(tab)
STEntry *tab;
{
	int h;
	VMRef sym;
		
	for( h = 0; h < LOCAL_HASHSIZE; h++ )
	{
        	sym = tab[h].head;

                while ( !NullRef(sym) )
                {
                	Symbol *s;
			asm_Module *m = NULL;
			char *type;

			if( !VMcheck(sym) ) error("badvm %x %d",sym,h);

			VMlock(sym);

			s = VMAddr(Symbol,sym);
#if 0
_trace("%x %x t %x g %d r %d m %x d %d %s",sym,s,s->type,s->global,s->referenced,
			s->module,s->def,s->name);
#endif
			if( !NullRef(s->module) ) m = VMAddr(asm_Module,s->module);
			
			switch( s->type )
			{
			case s_datasymb:
			case s_datadone:
					type = "data   "; break;
			case s_commsymb:
			case s_commdone:
					type = "common "; break;
			case s_codesymb:
					type = "code   "; break;
			default:
					type = "unknown"; break;
			}
			
			if( m == NULL )
			     report("%30s %s %4d no module",s->name,type,
			     		s->referenced);
			else report("%30s %s %4d [%d,%d]",s->name,type,
					s->referenced,m->id,s->def);

			VMunlock(sym);
                        sym = s->next;
                }
        }
}

