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
   |  misc.h							             |
   |                                                                         |
   |    Useful definitions                                                   |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    2 - O.Imbusch - 23 July  1991 - PrintFault                           |
   |    1 - O.Imbusch - 19 March 1991 - Basic version                        |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#ifndef __MISC_H
#define __MISC_H

#include <fault.h>
#include <helios.h>
#include <stdio.h>

#define BOOL   bool
#define IN
#define OUT
#define INOUT

#define K     (1024L)
#define M     (K * K)
#define G     (K * M)
#define T     (K * G)

#define IsIn(Test,Min,Max) ((Test >= Min) && (Test <= Max))
#define PolyNew(Type,Nr)   ((Type *) (Malloc (sizeof (Type) * Nr)))
#define RoundTo(N,MultOf)  ((((N - 1) / MultOf) + 1) * MultOf)
#define PrintFault(Code)   Fault (Code, FaultBuff, K), printf (stderr, "Fault (0x%08lx): %s", Code, FaultBuff);

extern char FaultBuff [K];

#endif

/*******************************************************************************
**
**  misc.h
**
*******************************************************************************/
