/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S			                --
--                     -----------                                      --
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- locklib.h								--
--                                                                      --
--	Header file for the lock library.				--
--                                                                      --
--	Author:  BLV 21.2.90						--
--                                                                      --
------------------------------------------------------------------------*/

#ifndef __locklib_h
#define __locklib_h

#ifndef __syslib_h
#include <helios.h>
#include <syslib.h>
#include <gsp.h>
#endif

typedef Object		*Lock;
#define NullLock	((Lock) NULL)

extern Lock	GetLock(char *name);
extern void	FreeLock(Lock lock);

#endif
