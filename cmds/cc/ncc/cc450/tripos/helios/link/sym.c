/****************************************************************/
/* File: sym.c                                                  */
/*                                                              */
/* Routines to manipulate the symbol table                      */
/*                                                              */
/* Author: NHG 17-Feb-87                                        */
/****************************************************************/
static char SccsId[] = "%W%	%G% Copyright (C) Perihelion Software Ltd.";

#include "link.h"
#include <ctype.h>

#define trace if(traceflags&db_sym) _trace

PRIVATE VMRef symheap;

PRIVATE STEntry Symtab[HASHSIZE];
PUBLIC WORD symsize;

WORD hash(ellipsis);

/********************************************************/
/* initsym                                              */
/*                                                      */
/* Initialize the symbol table.                         */
/*                                                      */
/********************************************************/

PUBLIC void initsym()
{
	int i;
	for (i=0; i<HASHSIZE ; i++)
	{
		Symtab[i].head = NullVMRef;
		Symtab[i].entries = 0;
	}
        
	symheap = VMPage();
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

PUBLIC VMRef lookup(name)
BYTE *name;
{
        WORD h = hash(name);
        VMRef sym;
	int loops = 0;
	int entries = 0;
	Module *m;
	
        if( !VMSame(curmod,module0) )
        {
        	m = VMAddr(Module,curmod);
        	sym = m->symtab[h].head;
		entries = m->symtab[h].entries;
		
                while ( !NullRef(sym) )
                {
                	Symbol *s = VMAddr(Symbol,sym);
			trace("mod lookup sym %x %s == %s",sym,name,s->name);
                        if( eqs(name,s->name) ) return sym;
                        sym = s->next;
         
                        if( loops++ == entries+10 )
                        {
                        	report("possible loop in symtab loop = %d entries = %d",loops,entries);
                        	traceflags |= db_sym;
                        }
                        else if ( loops == entries*2 ) error("quitting");
                }
        }

        sym = Symtab[h].head;
	loops = 0;
	entries = Symtab[h].entries;
	
        while ( !NullRef(sym) )
        {
               	Symbol *s = VMAddr(Symbol,sym);
		trace("glob lookup sym %x %s == %s",sym,name,s->name);
                if( eqs(name,s->name) ) return sym;
                sym = s->next;

                if( loops++ == entries+10 )
                {
                      	report("possible loop in global symtab loops = %d entries = %d",loops,entries);
                       	traceflags |= db_sym;
                }
                else if ( loops == 2*entries ) error("quitting");
        }
        
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
        WORD h = hash(name);
        VMRef sym;
	Symbol *s;
	Module *m;
	int size = sizeof(Symbol) + strlen(name);
	
	sym = VMalloc(size,symheap);
	
	if( NullRef(sym) ) 
	{
		symheap = VMPage();
		sym = VMalloc(size,symheap);
	}

	symsize += size;

	trace("insert %s %d sym = %x",name,local,sym);

	s = VMAddr(Symbol,sym);
	VMDirty(sym);
	
        cps(name,s->name);
        s->type = 0;
        s->value.w = NULL;
        s->module = curmod;
	s->global = !local;
	
        if( local && !VMSame(curmod,module0) )
        {
        	VMRef headref;
        	m = VMAddr(Module,curmod);
        	
        	headref = m->symtab[h].head;

        	s->next = headref;
        	s->prev = NullVMRef;

		if( !NullRef(headref) )
		{
	        	Symbol *head = VMAddr(Symbol,headref);
        		head->prev = sym;
        		VMDirty(headref);
        	}

		m = VMAddr(Module,curmod);	/* just in case */        	
		m->symtab[h].head = sym;
		m->symtab[h].entries++;
		VMDirty(curmod);
        }
        else
        {
        	VMRef headref = Symtab[h].head;

        	s->next = headref;
        	s->prev = NullVMRef;
        	
		if( !NullRef(headref) )
		{
	        	Symbol *head = VMAddr(Symbol,headref);
        		head->prev = sym;
        		VMDirty(headref);
        	}
        	
        	Symtab[h].head = sym;
        	Symtab[h].entries++;
        }

        return sym;
}

/********************************************************/
/* movesym                                              */
/*                                                      */
/* Remove symbol from current symbol table and place    */
/* in global table.                                     */
/*                                                      */
/********************************************************/

PUBLIC void movesym(sym)
VMRef sym;
{
	Symbol *s;
	int h;
	Module *m;
	s = VMAddr(Symbol,sym);
	
	if( s->global ) return;		/* already moved */
	
	h = hash(s->name);
	
	/* else move symbol from local to global table */
	
	VMlock(sym);
		
	trace("movesym ref %x name %s links %x %x",sym,s->name,s->next,s->prev);
	
	if( NullRef(s->prev) )
	{
		/* at head of list, step list head pointer over us */
		m = VMAddr(Module,s->module);
		m->symtab[h].head = s->next;
		m->symtab[h].entries--;
		VMDirty(s->module);
	}
	else 
	{
		/* else step prev's next pointer over us */
		Symbol *prev = VMAddr(Symbol,s->prev);
		prev->next = s->next;
		VMDirty(s->prev);
		m = VMAddr(Module,s->module);
		m->symtab[h].entries--;
		VMDirty(s->module);
	}

	if( !NullRef(s->next) )
	{
		/* if not at tail of list, patch next's prev pointer */
		Symbol *next = VMAddr(Symbol,s->next);
		next->prev = s->prev;
		VMDirty(s->next);
	}

	/* finally set up our pointers */
       	s->next = Symtab[h].head;
       	s->prev = NullVMRef;
	s->global = TRUE;
	VMDirty(sym);
	
	if( !NullRef(Symtab[h].head) )
	{
		/* if list is not empty, patch prev of old head */
	       	Symbol *head = VMAddr(Symbol,Symtab[h].head);
        	head->prev = sym;
		VMDirty(Symtab[h].head);
        }
        
       	Symtab[h].head = sym;
       	Symtab[h].entries++;
       	
       	VMunlock(sym);
       	
}

/********************************************************/
/* eqs                                                  */
/* cps                                                  */
/*                                                      */
/* String manipulation routines, only the first 31 chars*/
/* of any string are compared or copied.                */
/*                                                      */
/********************************************************/

#ifdef never
PUBLIC INT eqs(s,t)
BYTE *s, *t;
{
        return strncmp(s,t,31) == 0;
}
#endif

PUBLIC void cps(src,dst)
BYTE *src, *dst;
{
        int i;
        for( i = 0 ; src[i] && i <= 31 ; i++ )
                dst[i] = src[i];
        dst[i] = '\0';
}

