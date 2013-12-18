/**
*
* Title:  Helios Debugger - Symbol table support.
*
* Author: Andy England
*
* Date:   September 1988
*
*         (c) Copyright 1988 - 1992, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
**/

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/debugger/RCS/table.c,v 1.3 1992/10/27 14:04:16 nickc Exp $";
#endif

#include "tla.h"

/**
*
* initchain(chain);
*
* Initialise chain structure.
*
**/
PUBLIC void initchain(CHAIN *chain)
{
  chain->head = NULL;
}

/**
*
* addlink(chain, link);
*
* Add a link to a chain.
*
**/
PUBLIC void addlink(CHAIN *chain, LINK *link)
{
  link->next = chain->head;
  chain->head = link;
}

/**
*
* addtail(chain, link);
*
* Add a link to the end of a chain.
*
**/
PUBLIC void addtail(CHAIN *chain, LINK *link)
{
  LINK *prev = (LINK *)chain;

  until (prev->next == NULL) prev = prev->next;
  link->next = NULL;
  prev->next = link;
}

/**
*
* walkchain(chain, func, arg);
*
* Walk a chain.
*
**/
PUBLIC void walkchain(CHAIN *chain, void (*func)(), long arg)
{
  LINK *link, *next;

  for (link = chain->head; link != NULL; link = next)
  {
    next = link->next;
    (*func)(link, arg);
  }
}

/**
*
* link = searchchain(chain, func, arg);
*
* Search a chain.
*
**/
PUBLIC LINK *searchchain(CHAIN *chain, int (*func)(), long arg)
{
  LINK *link, *next;

  for (link = chain->head; link != NULL; link = next)
  {
    next = link->next;
    if ((*func)(link, arg)) return link;
  }
  return NULL;
}

/**
*
* inittable(table);
*
* Initialise table structure.
*
**/
PUBLIC void inittable(TABLE table) /* CR: */
{
  int hash;

  for (hash = 0; hash < HASH_MAX; hash++) initchain(&table[hash]);
}

/**
*
* table = newtable();
*
* Allocate and initialise a table structure.
*
**/
PUBLIC TABLE *newtable(void)
{
  TABLE *table; /* CR: */

  unless ((table = (TABLE *)newmem(sizeof(TABLE))) == NULL)
      inittable(*table); /* CR:  passing the contents */
  return table;
}

/**
*
* value = hashval(name);
*
* Calculate the has value of a name.
*
**/
PRIVATE int hashval(char *name)
{
  int c;
  unsigned int h = 0;
  unsigned int g;

  until ((c = *name++) == '\0')
  {
    h = (h << 4) + c;
    unless ((g = (h & 0xF0000000)) == 0)
    {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }
  return h % HASH_MAX;
}

/**
*
* symbol = addsymbol(table, name);
*
* Add a symbol into a symbol table.
*
**/
PRIVATE int cmpsymbol(SYMBOL *symbol, char *name)
{
  return strcmp(symbol->name, name) == 0;
}

PUBLIC SYMBOL *addsymbol(TABLE table, char *name)
{
  SYMBOL *symbol;
  int hash = hashval(name);

  if ((symbol = (SYMBOL *)searchchain(&table[hash], cmpsymbol, (long)name)) == NULL &&
      (symbol = (SYMBOL *)newmem(sizeof(SYMBOL) + strlen(name) - 2)) != NULL)
  {
    initchain(&symbol->entrylist);
    strcpy(symbol->name, name);
    addtail(&table[hash], &symbol->link);
  }
  return symbol;
}

/**
*
* symbol = findsymbol(table, name);
*
* Search a table for a particular symbol.
*
**/
PUBLIC SYMBOL *findsymbol(TABLE table, char *name)
{
  return (SYMBOL *)searchchain(&table[hashval(name)], cmpsymbol, (long)name);
}

/**
*
* walktable(table, func, arg);
*
* Walk a symbol table applying the same function to all symbols.
*
**/
PUBLIC void walktable(TABLE table, void (*func)(), long arg)
{
  int hash;

  for (hash = 0; hash < HASH_MAX; hash++) walkchain(&table[hash], func, arg);
}

/**
*
* symbol = searchtable(table, func, arg);
*
* Search a table for a particular symbol.
*
**/
PUBLIC SYMBOL *searchtable(TABLE table, int (*func)(), long arg)
{
  int hash;
  SYMBOL *symbol;

  for (hash = 0; hash < HASH_MAX; hash++)
    unless ((symbol = (SYMBOL *)searchchain(&table[hash], func, arg)) == NULL) return symbol;
  return NULL;
}
