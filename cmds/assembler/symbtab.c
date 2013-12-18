/*
 * File:	symbtab.c
 * Subsystem:	Generic Assembler
 * Author:	P.A.Beskeen
 * Date:	Aug '91
 *
 * Description: symbol table insertion and search routines
 *
 * RcsId: $Id: symbtab.c,v 1.2 1993/07/12 16:18:08 nickc Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "gasm.h"

/* hash table (HASHSIZE must be a prime number) */
Symbol *SymTab[ HASHSIZE ];



/********************************************************/
/* InitSymbTab						*/
/*						    	*/
/* Initialize the symbol table.				*/
/*							*/
/********************************************************/

void InitSymbTab(void)
{
	int i;
	
	for (i = 0; i < HASHSIZE ; i++)
		SymTab[i] = NULL;
}


/****************************************************************/
/* NewSymb							*/
/*								*/
/* Create a new Symbol entry and insert it into the hash table.	*/
/*								*/
/* This hash table insertion routine is used for pseudo opcodes	*/
/* regs, and labels, not cpu specific assembler mneumonics.	*/
/*								*/
/* returns pointer to new entry					*/
/*								*/
/****************************************************************/

Symbol *NewSymb(char *name, int what, int value)
{
	int	h = Hash(name);
	Symbol *s = (Symbol *)malloc(sizeof(Symbol) + strlen(name) + 1);

	if(s == NULL)
		Fatal("out of memory while adding symbol table entry");

	/* initialise symbol entry */
	strcpy(s->name, name);
	s->what = what;
	s->referenced = FALSE;
	s->type.value = value;

	/* insert symbol into hash table chain for this hash value */
	s->next = SymTab[h];
	SymTab[h] = s;

	return s;
}


/****************************************************************/
/* NewSymbStruct						*/
/*								*/
/* Create a symbol entry that points to a structure, rather	*/
/* than holding a simple value. This is mainly used for CPU 	*/
/* specific mnemonics or registers.				*/
/*								*/
/****************************************************************/

void NewSymbStruct(char *name, int what, void *sstruct)
{
	int	h = Hash(name);
	Symbol *s = (Symbol *)malloc(sizeof(Symbol) + strlen(name) + 1);

	if(s == NULL)
		Fatal("out of memory while adding new symbol struct entry");

	/* initialise symbol entry */
	strcpy(s->name, name);
	s->what = what;
	s->referenced = FALSE;
	s->type.any = sstruct;

	/* insert symbol into hash table chain for this hash value */
	s->next = SymTab[h];
	SymTab[h] = s;
}


/********************************************************/
/* FindSymb						*/
/*							*/
/* Find the entry in the hash table corresponding to	*/
/* the named string. This search is case sensitive.	*/
/*							*/
/* Returns the symbol entry if present, otherwise NULL.	*/
/*							*/
/********************************************************/

Symbol *FindSymb(char *name)
{
	/* get chain entry in hash table for this symbols name */
	Symbol	*s = SymTab[ Hash(name) ];

	while ( s ) {
		if ( strcmp( name, s->name ) == 0) {
			/* return symbols entry */
			return s;
		}
		else
			/* check next entry in chain */
			s = s->next;
	}

	/* lookup failed */
	return NULL;
}


/********************************************************/
/* CaseInsensitiveFindSymb				*/
/*							*/
/* Find the entry in the hash table corresponding to	*/
/* the lowercased version of the named string.		*/
/*							*/
/* Returns the symbol entry if present, otherwise NULL.	*/
/*							*/
/********************************************************/

Symbol *CaseInsensitiveFindSymb(char *name)
{
	Symbol	*s = NULL;
	/* @@@ a static lc_name[MAXSIZE] would speed things up if optm. req. */
	char	*lc_name = (char *)malloc(strlen(name) + 1);
	char	*dst = lc_name;

	if (lc_name == NULL)
		Fatal("Out of memory whilst searching symbol table");

	/* make lowercased copy of name */
	while (*name != '\0')
		*dst++ = tolower(*name++);

	*dst = '\0';

	/* get chain entry in hash table for this symbols lowercased name */
	s = SymTab[ Hash(lc_name) ];

	while ( s ) {
		if ( strcmp( lc_name, s->name ) == 0) {
			/* found matching symbol so return symbols entry */
			free(lc_name);
			return s;
		}
		else
			/* check next entry in chain */
			s = s->next;
	}

	/* lookup failed */
	free(lc_name);
	return NULL;
}


/********************************************************/
/* ShowUnrefSymb					*/
/*							*/
/* print a warning for each unreferenced symbol defined	*/
/* in the program.					*/
/********************************************************/

void ShowUnrefSymb(void)
{
	int h;
		
	for( h = 0; h < HASHSIZE; h++ )	{
		Symbol *s = SymTab[h];

		while ( s ) {
			if( s->what == HT_LABEL && !s->referenced) {
				char err[255];

				strcpy(err, "label \"");
				strcat(err, s->name);
				strcat(err, "\" has not been referenced");
				Warn(err);
			}
			s = s->next;
		}
	}
}


/********************************************************/
/* PrintSymbTab						*/
/* DebugPrintSymbTab					*/
/*							*/
/* Print out entire symbol table			*/
/********************************************************/

void PrintSymbTab(void)
{
	int h;

	printf("Symbol Table Dump:\n");

	for( h = 0; h < HASHSIZE; h++ ) {
		Symbol *s = SymTab[h];

		while ( s ) {
			if (s->what == HT_LABEL)
				printf("%s offset %x %s\n",
					s->name,
					s->type.value,
					s->referenced ? "" : "(unreferenced)"
				);
			s = s->next;
		}
	}
}


#ifdef DEBUG
void DebugPrintSymbTab(void)
{
	int h;

	fprintf(stderr, "\nSymbol Table Dump:\n");

	for( h = 0; h < HASHSIZE; h++ ) {
		Symbol *s = SymTab[h];

		while ( s ) {
			fprintf(stderr,
				"%s : hash = %d, type = %s, value %d"
				" (%#x) %s\n",
				s->name, h,
				((s->what == HT_LABEL) ?
					"LABEL" :
					(s->what == HT_PSEUDO) ?
						"PSEUDO" :
						(s->what == HT_MNEMONIC) ?
							"MNEMONIC" :
							(s->what == HT_TOKENVAL) ?
								"REGISTER/PATCH/ETC" :
								"UNKNOWN!"),
				s->type.value,
				s->type.value,
				(s->referenced) ? "" : "(unref'ed)"
			);

			if (s->what == HT_MNEMONIC) {
				fprintf(stderr, "\tMNEMONIC: %s(%d), Di: %#8x, Tri: %#8x, //St: %#8x\n", \
					s->type.mnemonic->name, s->type.mnemonic->token, \
					s->type.mnemonic->diadic, s->type.mnemonic->triadic, \
					s->type.mnemonic->par_st
				);
			}
			s = s->next;
		}
	}
	fprintf(stderr, "\nEND of Symbol Table Dump\n\n");
}
#endif


/* end of symbtab.c */
