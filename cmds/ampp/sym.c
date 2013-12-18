/****************************************************************/
/* File: sym.c                                                  */
/*                                                              */
/* Routines to manipulate the symbol table                      */
/*                                                              */
/* Author: NHG 17-Feb-87                                        */
/****************************************************************/
#ifdef __TRAN
static char RcsId[] = "$Id: sym.c,v 1.3 1994/03/08 13:05:39 nickc Exp $ Copyright (C) Perihelion Software Ltd.";
#endif
  
#include <ctype.h>
#include <string.h>
#include "ampp.h"

#define trace if(traceflags&db_sym)_trace

PRIVATE struct List Symtab[HASHSIZE];
PRIVATE void cps( BYTE * src, BYTE * dst );
PUBLIC WORD eqs(  BYTE * s,   BYTE * t );
INT hash( char * s );

/********************************************************/
/* initsym                                              */
/*                                                      */
/* Initialize the symbol table.                         */
/*                                                      */
/********************************************************/

PUBLIC void initsym()
{
   int i;
   for (i=0; i<HASHSIZE ; i++) InitList(&(Symtab[i]));
}

/********************************************************/
/* lookup                                               */
/*                                                      */
/* Lookup a symbol in the table.                        */
/* Returns its defintion if present, otherwise NULL.    */
/*                                                      */
/********************************************************/

PUBLIC struct Symbol *lookup(BYTE *name)
{
        struct Symbol *sym = (struct Symbol *)(Symtab[hash(name)].Head);

	trace("Lookup %s %d",name,hash(name));
	trace("Queue = %x",&Symtab[hash(name)]);
        while ( sym->node.Next != NULL )
        {
		trace("Sym = %x %x %x %s",sym,sym->node.Next,sym->node.Prev,sym->name);
                if( strncmp(name,sym->name,31)==0 )
                {
                        trace("Symbol '%s' found in table",name);
                        return sym;
                }
                sym = (struct Symbol *)sym->node.Next;
        }
        return NULL;
}

/********************************************************/
/* insert                                               */
/*                                                      */
/* insert a symbol in the table.                        */
/*                                                      */
/********************************************************/
 
PUBLIC struct Symbol *insert(BYTE *name)
{
        struct Symbol *sym = lookup(name);

        if( sym != NULL ) return sym;

        sym = New(struct Symbol);

        cps(name,sym->name);
        sym->definition = NULL;

        trace("Adding '%s' to table at offset %d sym = %8x",name,hash(name),sym);
        AddTail(&(Symtab[hash(name)]),(Node *)sym);
	trace("Sym = %x %x %x %s",sym,sym->node.Next,sym->node.Prev,sym->name);

        return sym;
}


/********************************************************/
/* eqs                                                  */
/* cps                                                  */
/*                                                      */
/* String manipulation routines, only the first 31 chars*/
/* of any string are compared or copied.                */
/*                                                      */
/********************************************************/

PUBLIC INT eqs(
	       BYTE *s,
	       BYTE *t )
{
	return strncmp(s,t,31) == 0;
}

PRIVATE void cps(
		 BYTE *src,
		 BYTE *dst )
{
        int i;
        for( i = 0 ; src[i] && i <= 31 ; i++ )
                dst[i] = src[i];
        dst[i] = '\0';
}

INT hash(char *s)
{
        char *p;
        unsigned long h = 0, g;
        for( p = s ; *p != 0 ; p++ )
        {
                h = (h << 4) + *p;
                if (( g = h & 0xf0000000L ) != 0)
                {
                        h = h ^ (g >> 24);
                        h = h ^ g;
                }
        }
        return (INT)(h % HASHSIZE);
}

