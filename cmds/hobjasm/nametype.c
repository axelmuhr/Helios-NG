/* -> nametype/c
 * Title:               Name handling
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */

#include "nametype.h"
#include "store.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
/*
 * abbreviate indicates whether abbreviations are allowed,
 * If so they are treated as case-insensitive.
 * length is the number of elements in the table.
 */
BOOLEAN NameLookup(Name *nameTable,Name name,BOOLEAN abbreviate,CARDINAL *index,CARDINAL length)
{
 CARDINAL i ;
 for (*index = 0; (*index < length); (*index)++)
  {
   if ((nameTable[*index].length == name.length) || ((nameTable[*index].length >= name.length) && abbreviate))
    {
     i = 0 ;
     if (abbreviate)
      {
       while ((i < name.length) && (toupper(nameTable[*index].key[i]) == toupper(name.key[i])))
        i++ ;
      }
     else
      {
       while ((i < nameTable[*index].length) && (nameTable[*index].key[i] == name.key[i]))
        i++;
      } ;
     if (i == name.length)
      return TRUE ;
    } ;
  } ;
 return FALSE ;
} /* End NameLookup */

/*---------------------------------------------------------------------------*/

void CopyName(CARDINAL number, char *value, Name *table)
/*Set up a command name table element*/
{
  table[number].length = strlen(value);
  table[number].key = mymalloc(table[number].length);
  memcpy(table[number].key, value, table[number].length);
} /* End CopyName */

/*> EOF namtype/c <*/
