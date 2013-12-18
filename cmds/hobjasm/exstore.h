/* -> exstore/h
 * Title:               Expression storage management
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */

#ifndef exprstore_h
#define exprstore_h

#include "extypes.h"

void InitExprStore(void *p, CARDINAL size);

void ALLOCATE(char **p, CARDINAL size);

#ifdef __NCCBUG
extern EStackEntry *eStack ;
#else
extern EStackEntry eStack [EStackLimit];
#endif

#endif

/* End exstore/h */
