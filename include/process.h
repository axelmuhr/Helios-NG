/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- process.h								--
--                                                                      --
--	C process mamagement						--
--                                                                      --
--	Author:  NHG 12/10/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: process.h,v 1.5 1993/07/27 13:59:18 paul Exp $ */

#ifndef __process_h
#define __process_h

#ifndef __helios_h
#include <helios.h>
#endif


/* Standard logical priority levels */
#define HighestPri	-32768
#define DevicePri	-24576
#define HighServerPri	-16384
#define ServerPri	-8192
#define StandardPri	 0
#define	BackgroundPri	 8192
#define	LowBackgroundPri 16384
#define IdlePri		 32767

/* Number of bits used to form physical priority */
#if defined(__ARM) || defined(__C40)
# define PriorityLevelBits	3
#else
# define PriorityLevelBits	1	/* transputer */
#endif

extern void *NewProcess(word, VoidFnPtr , word );
extern void ExecProcess(void *args, word logpri);
extern void RunProcess(void *args);
extern void ZapProcess(void *args);
extern word Fork(word stacksize, VoidFnPtr fn, word argsize, ... );
extern word LogToPhysPri(word logpri);
extern word PhysToLogPri(word physpri);
extern word GetPhysPriRange(void);
extern word GetPriority(void);
extern void SetPriority(word logpri);

#if defined(__C40) || defined(__ARM)
/* run function to its completion or blocking, it will not be timesliced */
extern word System(WordFnPtr fn, ...);

/* Enable, disable or report status of timeslicing globally on this processor */
/* SliceState() always returns the latest status */
extern int SliceState(int state);

/* valid SliceState() arguments */
#define SLICE_DISABLE	0
#define SLICE_ENABLE	1
#define SLICE_REPORT	2	/* returns True/False indication */

/* Set the period of time each thread receives to run in a timeslice */
/* The C40 implementation is only accurate to the nearest millisecond */
/* If 'usecs' are zero, then the quantum is not set, and the current */
/* quantum is returned */
extern int SliceQuantum(int usecs);
#endif

#endif

/* -- End of process.h */
