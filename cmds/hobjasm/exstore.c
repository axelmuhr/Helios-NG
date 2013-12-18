/* -> exstore/c
 * Title:               Expression storage manager
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "exstore.h"
#include "store.h"
#include <stdio.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*/

char       *curStore ;  /* The remaining available area */
CARDINAL    curLength ; /* How much we've got left */

#ifdef __NCCBUG
EStackEntry *eStack = NULL ;
#else
EStackEntry eStack [EStackLimit] ;
#endif

/*---------------------------------------------------------------------------*/

void InitExprStore(void *p, CARDINAL size)
{
#ifdef DEBUG
 printf("DEBUG: InitExprStore entered\n") ;
#endif

 curStore = p ;
 curLength = size ;

#ifdef __NCCBUG  
 if (eStack == NULL)
  {
#ifdef DEBUG
   printf("DEBUG: Allocating \"eStack\" (&%08X bytes)\n",(int)(EStackLimit * sizeof(EStackEntry))) ;
#endif
   eStack = (EStackEntry *)mymalloc(EStackLimit * sizeof(EStackEntry)) ;
#ifdef DEBUG
   printf("DEBUG: \"eStack\" at &%08X\n",(int)eStack) ;
#endif
  }
#endif 
} /* End InitExprStore */

/*---------------------------------------------------------------------------*/

void ALLOCATE(char **p, CARDINAL size)
{
/*
EXCEPTION AllocateFailed;
*/
  *p = curStore;
  if (size > curLength) {
    printf("Expression storage allocator failed\n");
    exit(1);
/*
  RAISE(AllocateFailed);
*/
  }; /* if */
  curStore += size;
  curLength -= size;
} /* End ALLOCATE */

/*---------------------------------------------------------------------------*/
/* EOF exstore/c */
