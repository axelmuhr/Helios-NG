/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- task.c								--
--                                                                      --
--	Task creation, process manipulation and exception processing.	--
--                                                                      --
--	This should probably move into ProcMan and we should find a	--
--	different way of getting the ProcMan started.			--
--                                                                      --
--	Author:  NHG 8/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: task.c,v 1.23 1993/08/11 09:56:59 nickc Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.				*/


#define __in_task 1	/* flag that we are in this module */

#include "kernel.h"
#include <task.h>
#include <process.h>

static void TaskExit(void);

/* module table entry */
typedef struct slot {
	word d; /* data area pointer */
#ifdef __SMT
	word c; /* code table pointer */
#endif
} slot;

/*--------------------------------------------------------
-- TaskInit						--
--							--
-- Build and dispatch a new task.			--
--							--
--------------------------------------------------------*/

word TaskInit(Task *task)
{
#ifdef __SMT
	RootStruct *root = GetRoot();
	word *cpi = root->cpi;
	word cptsize = 0;
#endif
	word datasize = 0;
	word maxmod = 0;
	word blocksize;
	slot *modtab;
	byte *dptr;
	MPtr sysbase = (MPtr)GetSysBase();
	MPtr module;

	/* first pass over code is to calculate size of data segment required */
	
	module = task->Program;

	while( ModuleWord_(module,Type) != 0 )
	{
		MPtr mod1 = module;
		
		if( ModuleWord_(module,Type) == T_ResRef ) /* loader will have allocated cpt for this lib */
		{
			if( ResRefWord_(module,Module) != NULL )
				module = (MPtr)ResRefWord_(module,Module);
			else
			{
				MPtr mm = MInc_(sysbase,ModuleWord_(module,Id)*sizeof(MPtr));
				module = MRTOA_(mm);
			}
		}
#ifdef __SMT
		else
		{
			cptsize += ModuleWord_(module,MaxCodeP);
		}
#endif
#ifdef __TRAN
		/* Transputer Maxdata defined in terms of words */
		datasize += ModuleWord_(module,MaxData) * sizeof(word);
#else
		/* Maxdata defined in terms of bytes */
		datasize += ModuleWord_(module,MaxData);
#endif
		if( ModuleWord_(module,Id) > maxmod )
			maxmod = ModuleWord_(module,Id);

		module = ModuleNext_(mod1);
	}

	maxmod++;
	
	/* now datasize, cptsize & maxmod contain the values we need to	*/
	/* calculate the size of the data segment.			*/
	
	blocksize =	ProgramWord_(task->Program,Stacksize) +
			ProgramWord_(task->Program,Heapsize) +
			datasize +
#ifdef __SMT
			cptsize +
#endif
			maxmod * sizeof(slot);

	modtab = (slot *)AllocMem(blocksize,&task->MemPool);

#if 0 /*defined(__C40) && defined(SYSDEB)*/
	KDebug(("Taskinit: %s code %a, modtab %a,\n\t"
		"total static blocksize %x, stacksize %x, heapsize %x,\n\t"
		"static data size %x, cpt size %x, modtab size %x\n",
		task->Program->Module.Name, task->Program, modtab,
		blocksize, task->Program->Stacksize, task->Program->Heapsize,
		datasize, cptsize, maxmod * sizeof(slot)));
#endif

	if( modtab == NULL ) return EC_Error|SS_Kernel|EG_NoMemory|EO_Task;

#ifdef __SMT
	task->HeapBase = (byte *)((word)modtab + datasize + cptsize + maxmod * sizeof(slot));
#else
	task->HeapBase = (byte *)((word)modtab + datasize + maxmod * sizeof(word));
#endif
	task->ModTab = (word *)modtab;

#ifdef __TRAN
	modtab[0].d = 0;
	move_(blocksize-4,modtab+1,modtab);
#else
	/* @@@ doesn't allocmem zero memory anyway? - only in TRAN version */
	ZeroBlock(modtab,blocksize);
#endif

	modtab[0].d = (word)&modtab[0].d;
#if 0
	modtab[0].c = (word)&modtab[0].c; /* not really required */
#endif

	/* The task's own port was allocated by the processor manager,	*/
	/* change its ownership here, so it will be deleted with the task*/
	
	if( task->Port != NullPort ) GetPTE((task->Port),GetRoot())->Owner = (word)task;

	/* second pass over code to set up module table pointers and	*/
	/* calls init routines the first time.				*/
	dptr = (byte *)((word)modtab + maxmod * sizeof(slot));

	module = task->Program;

#ifdef __SMT
	Wait(&root->cpi_op);
#endif

	while( ModuleWord_(module,Type) != 0 )
	{
		MPtr mod1 = module;
#ifdef __SMT
		bool SharedCPT = FALSE;
		bool InitSharedCPT = FALSE; 
#endif

		if( ModuleWord_(module,Type) == T_ResRef )
		{
			if( ResRefWord_(module,Module) != NULL )
				module = (MPtr)ResRefWord_(module,Module);
			else
			{
				MPtr mm = MInc_(sysbase,ModuleWord_(module,Id)*sizeof(MPtr));
				module = MRTOA_(mm);
			}
#ifdef __SMT
			SharedCPT = TRUE;
#ifdef SYSDEB /* debugging */
			if (cpi[ModuleWord_(module,Id)] == NULL) {
# ifdef __ARM
				KDebug("TaskInit: SMT error: module addr %x, Null Shared code pointer index slot: %d\n", module, ModuleWord_(module,Id));
# else
#  ifdef  __C40
				KDebug("TaskInit: SMT error: module %a, Null Shared code pointer index slot: %d\n", module, ModuleWord_(module,Id));
#  endif
# endif
			}
#endif			
			/* use shared code pointer table */
			if (cpi[ModuleWord_(module,Id)] & 1) /* cpt not yet initialised */
			{
				InitSharedCPT = TRUE;
				cpi[ModuleWord_(module,Id)] &= ~1;
			}
			modtab[ModuleWord_(module,Id)].c = cpi[ModuleWord_(module,Id)];
#endif
		}
#ifdef __SMT
		else
		{
			/* use private code pointer table */
#ifdef __C40
			modtab[ModuleWord_(module,Id)].c = C40WordAddress(dptr);
#else
			modtab[ModuleWord_(module,Id)].c = (word)dptr;
#endif
			dptr += ModuleWord_(module,MaxCodeP);
		}
#endif

		modtab[ModuleWord_(module,Id)].d = (word)dptr;

#ifdef __TRAN
		/* transputer maxdata defined in terms of words */
		dptr += ModuleWord_(module,MaxData) * sizeof(word);
#else
		/* every one else in terms of bytes */
		dptr += ModuleWord_(module,MaxData);
#endif		


#ifdef __SMT
		if(InitSharedCPT || !SharedCPT)
#endif
		{
			MPtr curinit = ModuleInit_(module);

#ifdef __ARM
# if 0
			/* Enter User mode to call user task initialisation */
			/* code. */
			EnterUserMode();
# endif
#endif
			while( MWord_(curinit,0) != 0 )
			{
				fncast fn;
				
				curinit = MRTOA_(curinit);

				fn.w = (word)MInc_(curinit,sizeof(MPtr));
# ifdef __SMT
				/* initarg = 2, init cpt first */
				CallWithModTab(2, 0, fn.wfn, (word *)modtab);
# else
				CallWithModTab(0, 0, fn.wfn, (word *)modtab);
# endif
			}
#ifdef __ARM
# if 0
			KernelEnterSVCMode();
# endif
#endif
		}
		module = ModuleNext_(mod1);
	}

#ifdef __SMT
	Signal(&root->cpi_op);
#endif

	/* Now set up the pointer to the task's Task structure		*/
#ifdef __SMT
	/* first entry in data area is task pointer */
	*((word *)modtab[1].d) = (word)task;
#else
	/* @@@@ Note the magic number 48 !!!!				*/
	((word **)modtab)[1][48] = (word)task;
#endif
	
	/* pass 3 calls init routines the second time */
#ifdef __SMT
	/* @@@ code wasteful, could be combined into a loop with 3rd init */
	/* SMT requires three init passes: code(2), data(0), data(1) */
	module = task->Program;

	while( ModuleWord_(module,Type) != 0 )
	{
		MPtr mod1 = module;

		if( ModuleWord_(module,Type) == T_ResRef )
		{
			if( ResRefWord_(module,Module) != NULL )
				module = (MPtr)ResRefWord_(module,Module);
			else
			{
				MPtr mm = MInc_(sysbase,ModuleWord_(module,Id)*sizeof(MPtr));
				module = MRTOA_(mm);
			}
		}

		{
			MPtr curinit = ModuleInit_(module);

#ifdef __ARM
# if 0
			KernelEnterSVCMode();
# endif
#endif
			while( MWord_(curinit,0) != 0 )
			{
				fncast fn;
				
				curinit = MRTOA_(curinit);

				fn.w = (word)MInc_(curinit,sizeof(MPtr));
				
				CallWithModTab( 0, 0, fn.wfn, (word *)modtab);
			}
#ifdef __ARM
# if 0
			KernelEnterSVCMode();
# endif
#endif
		}

		module = ModuleNext_(mod1);
	}
#endif

	module = task->Program;

	while( ModuleWord_(module,Type) != 0 )
	{
		MPtr mod1 = module;
		
		if( ModuleWord_(module,Type) == T_ResRef )
		{
			if( ResRefWord_(module,Module) != NULL )
				module = (MPtr)ResRefWord_(module,Module);
			else
			{
				MPtr mm = MInc_(sysbase,ModuleWord_(module,Id)*sizeof(MPtr));
				module = MRTOA_(mm);
			}
			
		}

		{
			MPtr curinit = ModuleInit_(module);

#ifdef __ARM
# if 0
			/* Enter User mode to call user task initialisation */
			/* code. */
			EnterUserMode();
# endif
#endif
			while( MWord_(curinit,0) != 0 )
			{
				fncast fn;
				
				curinit = MRTOA_(curinit);

				fn.w = (word)MInc_(curinit,sizeof(MPtr));
				
				CallWithModTab( 1, 0, fn.wfn, (word *)modtab );
			}
#ifdef __ARM
# if 0
			KernelEnterSVCMode();
# endif
#endif
		}

		module = ModuleNext_(mod1);
	}

	/* the data has all been initialised, build entry frame & let it go */
	/* this code is neccesarily machine dependant			*/
	
	{
		word *s = (word *)((word)modtab+blocksize);
		word *w;
#ifdef never
		word descript[ 2 ];
		MPtr fn = ProgramMain_(task->Program);
		
		descript[ 0 ] = (word)modtab;
		descript[ 1 ] = (word)s - ProgramWord_(task->Program,Stacksize); /* stack base */

		fn = MRTOA_(fn);
		
		w = CreateProcess( s, MToFn_(fn), TaskExit, descript, 8 );
#else /* not never */
		
		fncast ansibodge;
		MPtr fn = ProgramMain_(task->Program);
#ifndef __TRAN
		word descript[ 2 ];
#endif
		
		ansibodge.w = (word)MRTOA_( fn );

#ifdef __C40
		/* initialise the stackchunk list */
		
		InitSemaphore( &task->StackLock, 1 );
		InitList( &task->StackChunks );		
#endif
		
#ifdef __TRAN
		
		w = CreateProcess( s, ansibodge.vfn, TaskExit, modtab, 8 );
#else

		descript[ 0 ] = (word)modtab;
		descript[ 1 ] = (word)s - ProgramWord_(task->Program,Stacksize); /* stack base */

		w = CreateProcess( s, ansibodge.vfn, TaskExit, descript, 8 );
#endif /* not __TRAN */
#endif /* not never */
		w[0] = (word)task;
		w[1] = 0;

		EnterProcess(w, LogToPhysPri(StandardPri));
	}

	return Err_Null;
}

/* If the main procedure of the root process returns, simply halt the	*/
/* process. This is all we can do at this level.			*/
static void TaskExit(void)
{
	Stop();
}


/*--------------------------------------------------------
-- LogToPhysPri()					--
--							--
-- Convert logical priority level to physical priority	--
--							--
--------------------------------------------------------*/

word LogToPhysPri(word logpri)
{
	word physpri;

	/* only allow 16 bit values */
	if (logpri > 32767 || logpri < -32768 )
		logpri = 32767;

	/* convert from logical to physical priority level */
	physpri = (logpri + 32768) >> (16 - PriorityLevelBits);

	if (physpri == 0)
		physpri = 1;	/* do not allow user programs to set HiPri */

	return physpri;
}


/*--------------------------------------------------------
-- PhysToLogPri()					--
--							--
-- Convert physical priority level to logical priority	--
--							--
--------------------------------------------------------*/

word PhysToLogPri(word physpri)
{
	/* convert from physical to logical priority level */
	return ((physpri << (16 - PriorityLevelBits)) - 32768);
}

/*--------------------------------------------------------
-- GetPriority()					--
--							--
-- Get processes logical priority level			--
--							--
--------------------------------------------------------*/
word GetPriority(void)
{
	return(PhysToLogPri(GetPhysPri()));
}


/*--------------------------------------------------------
-- SetPriority()					--
--							--
-- Set a processes logical priority level		--
--							--
--------------------------------------------------------*/
void SetPriority(word logpri)
{
	SetPhysPri(LogToPhysPri(logpri));
}

/*--------------------------------------------------------
-- InitProcess						--
-- StartProcess						--
-- StopProcess						--
--							--
-- Task sub-process thread support.			--
-- InitProcess creates a process ready for execution.	--
-- StartProcess gets it going.				--
-- StopProcess causes it to halt.			--
--							--
--------------------------------------------------------*/

word *InitProcess(word *stack, VoidFnPtr entry, VoidFnPtr exit, word *display, word nargs)
{
	return (word *)CreateProcess(stack,entry,exit,display,nargs);
}


void StartProcess(word *p, word physpri)
{
	EnterProcess(p,physpri);
}


void StopProcess(void)
{
	Stop();
}


/* turn off stack checking - i.e. if exception is stack error dont loop */
#if defined(__TRAN) || defined(__C40)
# pragma -s1
#elif defined (__ARM)
# pragma no_check_stack
#else
# error "Pragma required to turn off stack checking"
#endif


/*------------------------------------------------------------------------
-- CallException							--
--									--
-- Call the task exception routine with the task's module table.	--
--									--
-- For processors other than the transputer, this code is usually used	--
-- a means of delivering synchronous signals such as SIGSEGV, SIGSTAK	--
-- and SIGFPE. The handlers for these hardware traps call CallException	--
-- directly.								--
--									--
------------------------------------------------------------------------*/

void CallException(Task *task, word reason)
{
	if( task->ExceptCode == NULL ) return;
	
	CallWithModTab(	reason,
			(word)task->ExceptData,
			(WordFnPtr)task->ExceptCode,
			task->ModTab);
}


/* -- End of task.c */
