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
   |  procname.h						             |
   |                                                                         |
   |    -Prototypes and definitions for procname.c                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    1 - O.Imbusch - 27 March 1991 - Basic version                        |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#ifndef __PROCNAME_H
#define __PROCNAME_H

#include <helios.h>

#define ThisFunc(FirstPar) (ProcName ((word **) (&FirstPar) [-3]))
#define CalledBy(FirstPar) (ProcName ((word **) (&FirstPar) [-2]))

extern char *ProcName (word *X);

#endif

/*******************************************************************************
**
**  procname.h
**
*******************************************************************************/
