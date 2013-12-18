                                                                                /*
  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                    (c) 1991 by parsytec GmbH, Aachen                    |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |                               Utitlities                                |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  procname.c							     |
   |                                                                         |
   |    -Actual function's and calling function's name                       |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    1 - O.Imbusch - 13 March 1991 - Basic version (extracted from anonym.|
   |                                    code (NHG?))                         |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#include <helios.h>
#include <module.h>

#include "procname.h"

char *ProcName (word *X)
{
  X = (word *) (((word) (X)) & ~3);
  
  while ((*X & T_Mask) != T_Valid)
    --X;
  	
  switch (*X)
  {
    case T_Proc:
      return ((Proc *) (X))->Name;
      break;

    case T_Module:
    case T_Program:
      return ((Module *) (X))->Name;
      break;
      
    default:

/* NOTE: the use of Error would lead to a recursion. */    
   
      IOdebug ("Illegal type in Function ProcName (0x%x).", *X);
      IOdebug ("Called by %s", CalledBy (X));
      return (NULL);
      break;
  }
}

/*******************************************************************************
**
**  procname.c
**
*******************************************************************************/
