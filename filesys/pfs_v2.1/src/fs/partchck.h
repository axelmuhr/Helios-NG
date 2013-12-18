                                                                                /*
  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                    (c) 1991 by parsytec GmbH, Aachen                    |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |                          Parsytec File System                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  partchck.h							     |
   |                                                                         |
   |    Prototypes for partchck.c                                            |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    2 - O.Imbusch -  2 April 1991 - Double allocation extension          |
   |    1 - O.Imbusch - 25 March 1991 - Basic version                        |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#ifndef __PARTCHCK_H
#define __PARTCHCK_H

#include <device.h>
#include <helios.h>

extern bool IllegalPartitioning (DiscDevInfo *DDI);
extern bool IllegalAllocParts   (DiscDevInfo *DDI);

#endif

/*******************************************************************************
**
**  partchck.h
**
*******************************************************************************/