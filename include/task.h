 /*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- task.h								--
--                                                                      --
--	Kernel task support.						--
--                                                                      --
--	Author:  NHG 16/8/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.	*/
/* $Id: task.h,v 1.6 1993/08/18 16:19:50 nickc Exp $ */


#ifndef __task_h
#define __task_h

#ifndef __helios_h
#include <helios.h>
#endif

#include <queue.h>
#include <message.h>
#include <memory.h>
#include <module.h>
#ifdef __C40
#include <sem.h>
#endif


/* Task structure */

struct Task 
{
	Node		Node;		/* queueing node		*/
	MPtr		Program;	/* pointer to program structure	*/
	Pool		MemPool;	/* task private memory pool	*/
#ifdef __cplusplus
	uword		Port;		/* initial message port		*/
	uword		Parent;		/* parent's message port	*/
	uword		IOCPort;	/* tasks IOC port		*/
#else
	Port		Port;		/* initial message port		*/
	Port		Parent;		/* parent's message port	*/
	Port		IOCPort;	/* tasks IOC port		*/
#endif
	word		Flags;		/* Flags field			*/
	VoidFnPtr	ExceptCode;	/* exception routine		*/
	byte		*ExceptData;	/* data for same		*/
	byte		*HeapBase;	/* base of heap			*/
	word		*ModTab;	/* pointer to task module table */
#ifdef __in_procman
	struct TaskEntry *TaskEntry;	/* pointer to procman control struct */
#else
	word		TaskEntry;	/* to avoid making TaskEntry public */
#endif
#ifdef __C40
	Semaphore	StackLock;	/* Lock on the StackChunks list */
	List		StackChunks;	/* List of free stack chunks    */
#endif
};

#ifndef __cplusplus
typedef struct Task Task;
#endif

/* Bits in flag word */

#define Task_Flags_ioc		1	/* SysLib Debugging...	*/
#define Task_Flags_stream	2
#define Task_Flags_memory	4
#define Task_Flags_error	8
#define Task_Flags_process	16
#define Task_Flags_pipe		32
#define Task_Flags_info		64
#define Task_Flags_meminfo	128
#define Task_Flags_fork		256
#define Task_Flags_servlib	512

#define Task_Flags_fixmem	0x01000000	/* no heap expansion	*/

/* Kernel procedures */

extern word TaskInit(Task *);
extern void CallException(Task *,word);
extern word *InitProcess(word *stack, VoidFnPtr entry, VoidFnPtr exit, word *display, word nargs);
extern void StartProcess(word *p, word pri);
extern void StopProcess(void);
extern word MachineType(void);

/* External variables */
 
extern Task *MyTask;	/* set by C startup */

#endif

/* -- End of tasks.h */

