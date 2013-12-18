/* -> occur/c
 * Title:               Handle cross referencing
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "asm.h"
#include "conds.h"
#include "constant.h"
#include "formatio.h"
#include "getline.h"
#include "globvars.h"
#include "nametype.h"
#include "occur.h"
#include "store.h"
#include "tables.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/

#define TableSize 1024

/*---------------------------------------------------------------------------*/

void PrintSymbol(Name name)
{
 CARDINAL i ;
 for (i = 0; i < name.length; i++)
  WriteCh(name.key[i]) ;
}

/*---------------------------------------------------------------------------*/
/* Print the occurrence chain for a single occurrence */

static void PrintOccurrence(OccStartPtr occ)
{
 OccPtr  ptr = occ->occ ;
 BOOLEAN first = TRUE ;

 do
  {
   WriteChs("      ") ;
   WriteCh((first) ? 'A' : 'a') ;
   WriteChs("t line ") ;
   WriteCardinal(ptr->line) ;
   WriteChs(" in ") ;
   switch (ptr->context)
    {
     case File  :
                  WriteChs("file ") ;
                  PrintSymbol(ptr->u.file) ;
                  WriteChs("\\N") ;
                  return ;

     case Macro :
                  WriteChs("macro ") ;
                  PrintSymbol(ptr->u.macro) ;
                  WriteChs("\\N") ;
    }
   ptr = ptr->newContext ;
  } while (1) ;

 return ;
}

/*---------------------------------------------------------------------------*/
/* Print an occurrence chain with the given title. "number" returns the
 * number of occurrences printed.
 */

static void PrintChain(char *type, OccStartPtr occ, CARDINAL *number)
{
 *number = 0 ;
 WriteChs("   ") ;
 WriteChs(type) ;
 WriteChs("\\N") ;
 if (occ == NULL)
  {
   WriteChs("      None\\N") ;
   return ;
  }

 while (occ != NULL)
  {
   (*number)++ ;
   PrintOccurrence(occ) ;
   occ = occ->next ;
  }

 return ;
}

/*---------------------------------------------------------------------------*/
/* Print the cross reference result for one symbol */

void PrintResults(SymbolPointer ptr)
{
 CARDINAL defCount ;
 CARDINAL useCount ;

 PrintChain("Definitions",ptr->defPtr,&defCount) ;
 PrintChain("Usages",ptr->usePtr,&useCount) ;
 if (defCount == 0)
  {
   WriteChs("Warning: ") ;
   PrintSymbol(ptr->key) ;
   WriteChs(" undefined\\N") ;
  }
 else
  if (defCount > 1)
   {
    WriteChs("Warning: ") ;
    PrintSymbol(ptr->key) ;
    WriteChs(" multiply defined\\N") ;
   }

 if (useCount == 0)
  {
   WriteChs("Comment: ") ;
   PrintSymbol(ptr->key) ;
   WriteChs(" unused\\N") ;
  }
 else
  if (useCount == 1)
   {
    WriteChs("Comment: ") ;
    PrintSymbol(ptr->key) ;
    WriteChs(" used only once\\N") ;
   }

 return ;
}

/*---------------------------------------------------------------------------*/

static void AddContext(OccStartPtr ptr)
{
 CARDINAL              i ;
 OccPtr                occ ;
 StructureStackElement s ;

 ptr->next = NULL ;

 /* Now set up the context chain */
 i = lineNumber ;
 ptr->occ = mymalloc(sizeof(*ptr->occ)) ;
 occ = ptr->occ ;
 while (NextMacroElement(&s))
  {
   occ->context = Macro ;
   occ->line = i ;
   occ->u.macro = s.u.macro.name ;
   occ->newContext = mymalloc(sizeof(*occ->newContext)) ;
   i = s.u.macro.lineNumber ;
   occ = occ->newContext ;
  }

 occ->context = File ;
 occ->line = i ;
 i = 0 ;
 while ((currentFileName[i] > Space) && (currentFileName[i] < Del))
  i++ ;
 occ->u.file.length = i ;
 occ->u.file.key = mymalloc(i) ;
 memcpy(occ->u.file.key,currentFileName,i) ;
 occ->newContext = NULL ;
}

/*---------------------------------------------------------------------------*/
/* Add a usage of a symbol to the reference chains */

void AddUse(SymbolPointer ptr)
{
 OccStartPtr usePtr ;

 if (!xrefOn || (pass != 1))
  return ;

 if (ptr->usePtr == NULL)
  {
   ptr->usePtr = mymalloc(sizeof(ptr->usePtr)) ;
   usePtr = ptr->usePtr ;
  }
 else
  {
   usePtr = ptr->usePtr ;
   while (usePtr->next != NULL)
    usePtr = usePtr->next ;
   usePtr->next = mymalloc(sizeof(*usePtr->next)) ;
   usePtr = usePtr->next ;
  }
 AddContext(usePtr) ;
 return ;
}

/*---------------------------------------------------------------------------*/
/* Add a definition to the reference chains */

void AddDef(SymbolPointer ptr)
{
 OccStartPtr defPtr ;

 if (!xrefOn || (pass != 1))
  return ;
 if (ptr->defPtr == NULL)
  {
   ptr->defPtr = mymalloc(sizeof(ptr->defPtr)) ;
   defPtr = ptr->defPtr ;
  }
 else
  {
   defPtr = ptr->defPtr ;
   while (defPtr->next != NULL)
    defPtr = defPtr->next ;
   defPtr->next = mymalloc(sizeof(defPtr->next)) ;
   defPtr = defPtr->next ;
  }
 AddContext(defPtr) ;
 return ;
}

/*---------------------------------------------------------------------------*/
/* EOF occur/c */
