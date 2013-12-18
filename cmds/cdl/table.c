/**
*
* Title:    Symbol Table Support
*
* Author:   Andy England
*
* Date:     May 1988
*
**/
static char *rcsid = "$Header: /hsrc/cmds/cdl/RCS/table.c,v 1.1 1990/08/28 10:43:18 james Exp $";

#include "table.h"

TABLE *NewTable()
{
  TABLE *Table = (TABLE *)malloc(sizeof(TABLE));

  unless (Table == NULL)
  {
    int Hash;

    for (Hash = 0; Hash < HASH_SIZE; Hash++) InitList(&Table->List[Hash]);
  }
  return Table;
}

void FreeTable(Table, Function)
TABLE *Table;
#ifdef helios
void (*Function)();
#else
int (*Function)();
#endif
{
  int Hash;

  for (Hash = 0; Hash < HASH_SIZE; Hash++)
    FreeList(&Table->List[Hash], Function);
  free(Table);
}

void AddSymbol(Table, Symbol)
TABLE *Table;
SYMBOL *Symbol;
{
  int Hash = HashFunction(Symbol->Name);

  AddTail(&Table->List[Hash], (NODE *)Symbol);
}

SYMBOL *FindSymbol(Table, Name)
TABLE *Table;
char *Name;
{
  SYMBOL *Symbol;
  int Hash = HashFunction(Name);

  for (Symbol = (SYMBOL *)Table->List[Hash].Head; Symbol->Next; Symbol = Symbol->Next)
  {
    if (strequ(Name, Symbol->Name)) return Symbol;
  }
  return NULL;
}

void RemSymbol(Table, Symbol)
TABLE *Table;
SYMBOL *Symbol;
{
#ifdef NEWCODE
  RemNode(Symbol);
#endif
}

int CountTable(Table, Function)
TABLE *Table;
int (*Function)();
{
  int Hash;
  int Count = 0;

  for (Hash = 0; Hash < HASH_SIZE; Hash++)
    Count += CountList(&Table->List[Hash], Function);
  return Count;
}

#ifdef helios
void WalkTable(TABLE *Table, void (*Function)())
#else
void WalkTable(Table, Function)
TABLE *Table;
int (*Function)();
#endif
{
  int Hash;

  for (Hash = 0; Hash < HASH_SIZE; Hash++)
    WalkList(&Table->List[Hash], Function);
}

int HashFunction(Name)
char *Name;
{
  return Name[0] % HASH_SIZE;
}

