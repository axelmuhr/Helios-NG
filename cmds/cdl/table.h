/**
*
* Title:    Symbol Table Support.
*
* Author:   Andy England
*
* Date:     May 1988
*
* $Header: /hsrc/cmds/cdl/RCS/table.h,v 1.1 1990/08/28 10:43:45 james Exp $
*
**/

#include "list.h"

#define HASH_SIZE 20

typedef struct Symbol
{
  struct Symbol *Next;
  struct Symbol *Prev;
  char *Name;
} SYMBOL;

typedef struct Table
{
  LIST List[HASH_SIZE];
} TABLE;

#ifdef helios
TABLE *NewTable(void);
void FreeTable(TABLE *, void (*)());
void AddSymbol(TABLE *, SYMBOL *);
void RemSymbol(TABLE *, SYMBOL *);
SYMBOL *FindSymbol(TABLE *, char *);
int CountTable(TABLE *, int (*)());
void WalkTable(TABLE *, void (*)());
int HashFunction(char *);
#else
TABLE *NewTable();
void FreeTable();
void AddSymbol();
void RemSymbol();
SYMBOL *FindSymbol();
int CountTable();
void WalkTable();
int HashFunction();
#endif

