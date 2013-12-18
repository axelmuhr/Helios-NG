/* -> store/c
 * Title:               Safe storage management
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "store.h"
#include <stdlib.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*/

void *mymalloc(size_t size)
{
 void *p = malloc(size) ;

#ifdef MDEBUG
 printf("DEBUG: mymalloc &%08X bytes\n",(int)size) ;
#endif

 if ((p == NULL) && (size != 0))
  {
   printf("Heap memory exhausted: failed to alloc &%08X bytes\n",(int)size) ;
   exit(0) ;
  } ; /* if */
 return p ;
} /* End mymalloc */

/*---------------------------------------------------------------------------*/
/* EOF store/c */
