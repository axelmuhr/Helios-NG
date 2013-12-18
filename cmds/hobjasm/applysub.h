/* -> applysub/h
 * Title:               Subroutines for expression evaluation
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#ifndef applysub_h
#define applysub_h

#include "extypes.h"

BOOLEAN Compare(CARDINAL pointer,CARDINAL pointer2,Operator op) ;

/* Coerce a one byte string on the stack to a one byte constant if necessary */
CARDINAL Coerce(CARDINAL pointer) ;

void CcSub(CARDINAL pointer,CARDINAL pointer2) ;

void LenSub(CARDINAL pointer,CARDINAL pointer2) ;

void ChrSub(CARDINAL pointer,CARDINAL pointer2) ;

void StrSub(CARDINAL pointer,CARDINAL pointer2) ;

#endif

/*---------------------------------------------------------------------------*/
/* EOF applysub/h */
