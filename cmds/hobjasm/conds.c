/*-> conds/c
 * Title:               The IF, WHILE and structure stack handler
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "conds.h"
#include "errors.h"
#include "globvars.h"
#include "mactypes.h"
#include "store.h"
#include <stdio.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*/

#define StructureStackSize 256

#ifdef __NCCBUG
StructureStackElement *structureStack = NULL ;
#else
StructureStackElement structureStack[StructureStackSize];
#endif

static CARDINAL stackPointer ;
static CARDINAL errorPointer ;
static BOOLEAN  initialised = FALSE ;

CARDINAL includedIFs ;
CARDINAL rejectedIFs ;
CARDINAL includedWHILEs ;
CARDINAL rejectedWHILEs ;

/*---------------------------------------------------------------------------*/

BOOLEAN Stack(StructureStackElement s)
{
 if (stackPointer >= StructureStackSize)
  {
   WarningReport("Structure stack overflow") ;
   exception = StackOverflow ;
   return FALSE ;
  }
 structureStack[stackPointer++] = s ;
 return TRUE ;
} /* End Stack */

/*---------------------------------------------------------------------------*/

BOOLEAN Unstack(StructureStackElement *s)
{
 if (stackPointer == 0)
  {
   WarningReport("Structure stack underflow") ;
   exception = StackUnderflow ;
   return FALSE ;
  }
 *s = structureStack[--stackPointer] ;
 return TRUE ;
} /* End Unstack */

/*---------------------------------------------------------------------------*/

void InitStructureStack(void)
{
 CARDINAL i ;

#ifdef __NCCBUG
 if (structureStack == NULL)
  structureStack = (StructureStackElement *)mymalloc(StructureStackSize * sizeof(StructureStackElement)) ;
#endif

 includedIFs = 0 ;
 rejectedIFs = 0 ;
 includedWHILEs = 0 ;
 rejectedWHILEs = 0 ;
 for (i = 0; i <= StructureStackSize-1; i++)
  {
   if (initialised && (structureStack[i].type == GetSSET) && (structureStack[i].u.file.fileName.key != NULL))
    {
     free(structureStack[i].u.file.fileName.key) ;
     structureStack[i].u.file.fileName.key = NULL ;
    }
   structureStack[i].type = ConditionalSSET ; /* So no associated store */
  }
 initialised = TRUE ;
 stackPointer = 0 ;
} /* End InitStructureStack */

/*---------------------------------------------------------------------------*/
/* Initialise for access to the stack for error reporting */

void InitErrorAccess(void)
{
 errorPointer = stackPointer ;
} /* End InitErrorAccess */

/*---------------------------------------------------------------------------*/
/* Get next macro element off stack for error reporting */

BOOLEAN NextMacroElement(StructureStackElement *s)
{
 while (errorPointer != 0)
  {
   errorPointer-- ;
   if (structureStack[errorPointer].type == MacroSSET)
    {
     *s = structureStack[errorPointer] ;
     return TRUE ;
    }
  }
 return FALSE ;
} /* End NextMacroElement */

/*---------------------------------------------------------------------------*/

void UnwindToGet(void)
{
 while (stackPointer != 0)
  {
   switch (structureStack[stackPointer-1].type)
    {
     case GetSSET   : return ;

     case MacroSSET : macroLevel-- ;
                      ExitMacro() ;
                      break ;
    }
   stackPointer-- ;
  }
} /* End UnwindToGet */

/*---------------------------------------------------------------------------*/
/* EOF conds/c */
