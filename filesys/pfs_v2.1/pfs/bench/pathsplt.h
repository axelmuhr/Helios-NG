                                                                                /*

  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                    (c) 1991 by parsytec GmbH, Aachen                    |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  pathsplt.h							     |
   |                                                                         |
   |    Definitions/declarartions for pathsplt.c                             |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    1 - O. Imbusch -  1 March 1991 - Basic version                       |
   |                                                                         |
  []-------------------------------------------------------------------------[]

                                                                                */

#ifndef __PATHSPLT_H
#define __PATHSPLT_H

#include <helios.h>

#define PathSplitOK  (-1)

extern INT PathSplit (STRING Path,
                      STRING Dir,
                      STRING Name);

#endif

/*******************************************************************************
**
**  pathasplt.h
**
*******************************************************************************/
