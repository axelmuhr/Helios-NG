/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- kstart.c								--
--                                                                      --
--	Kernel entry point, called as soon as possible from assembler	--
--	entry point.							--
--                                                                      --
--	Author:  NHG 8/8/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* $Id: kstart.c,v 1.37 1993/10/04 12:11:18 paul Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd. */

#define __in_kstart 1	/* flag that we are in this module */

#include "kernel.h"
#include <task.h>

void debinit(void);
#ifdef CODES
	word *InitErrorCodes(void);
#endif

#include <config.h>
int GetROMConfig(Config *config); /* from romsupp.c or executive */


/*--------------------------------------------------------
-- Kstart						--
--							--
-- Startup procedure, call initialisation entry points	--
-- of all kernel subsystems.				--
--							--
--------------------------------------------------------*/
#ifdef __ARM

/*{{{  ARM KStart */

void ExecErrorHandler(word signal, char *errmsg, word *regs, bool unrecoverable);
extern void _FreeMemStop(void *);

/* Called from bootstrap with link number booted from and address of nucleus */
void Kstart(word bootlink, MPtr loadbase)
{
	RootStruct *root = GetRoot();
	Config *config = (Config *)(root + 1);
	word confsize;
	Task *task;
	SaveState *startupstate;

/* #define EARLYSYNCDEBUG*/
#ifdef EARLYSYNCDEBUG 	/* quick and dirty debug */
/* For conning IOServer into thinking helios has now fully booted, so we can */
/* send IOdebug()'s and KDebug()'s (KERNELDEBUG2 variety) straight away */
	char syncIt[12];

	syncIt[0] = 0xf0;
	syncIt[1] = 0;
	syncIt[2] = 0;
	syncIt[3] = 0;

	syncIt[4] = 0;
	syncIt[5] = 0;
	syncIt[6] = 0;
	syncIt[7] = 0;

	syncIt[8] = 0;
	syncIt[9] = 0;
	syncIt[10] = 0;
	syncIt[11] = 0;
#endif

/******************************** initialise Executive */

	ExecInit();		/* initialise executive & start system timer */

/******************************** Get configuration information */

	/* Only download config vector if we are not a ROM'ed system, and */
	/* the bootstrap has not done this for us already. */

	if (bootlink == -1) {
		/* If we are a ROM booted system, then the config structure  */
		/* will already have been initialised by the bootstrap.      */
		/* Its size has been placed in the ImageSize element of the  */
		/* structure (size needs to be passed as the number of links */
		/* defined may be non standard). This use of ImageSize is    */
		/* temporary, the kernel will set it to the nucleus's size.  */
		confsize = config->ImageSize;
	} else {
		/* bootlink != -1, so we must have been booted down a link.  */
		/* Get config vector from the same link we were booted down. */

		_LinkRx(4, bootlink, &confsize);	/* read size */
		_LinkRx(confsize, bootlink, config);	/* read config */

#if 0
		/* Max links should be defined by I/O Server */
		config->NLinks = COMMSLINKS;
#else
		/* @@@ *ERROR* four links should work, but don't *FIX*ME* */
		config->NLinks = 1;
#endif
	}

	config->LoadBase = loadbase;
	config->ImageSize = MWord_(loadbase,0);

	root->Flags = config->Flags;

	/* save config pos for GetConfig macro */
	root->Configuration = (word *)config;
	
#ifdef CODES
	root->ErrorCodes = InitErrorCodes();
#endif

/******************************** Initialise subsystems */

	MemInit(config, confsize); /* v1.2 additional parameter, config permanently stored */

	/* As the port garbage collector has some unidentified problems */
	/* with large task forces, default to having it turned off */
	root->GCControl = FALSE;

	PortInit( config );

	task = (Task *) Allocate( sizeof(Task), root->FreePool, &root->SysPool );

	/* ARM needs initial save area so that interrupts can occur. */
	/* Interrupt handlers always save the state to the CurrentSaveArea. */
	GetExecRoot()->CurrentSaveArea = startupstate =
	  (SaveState *) Allocate( sizeof(SaveState), root->FreePool, &root->SysPool );

	EventInit( config );

#ifdef LINKIO
	/* baud rate has now been set for VLSI PID board and comms can start */
	LinkInit( config, task );
#endif

#if defined(EARLYSYNCDEBUG)
# if defined(__ARM) && defined(KERNELDEBUG2)
	/* Now baud rate has been set we can start sending debugging */
	GetExecRoot()->KDebugEnable = TRUE;	  /* can now send KDebug()'s */
# endif
	/* Quick 'N Dirty Debug */
	/* Send sync early so we can KDebug() (KERNELDEBUG1/2/3) immediately */
	_LinkTx(12, bootlink, &syncIt[0]);
#endif

	MemInit2();

#ifdef KERNELDEBUG3
	/* initialise kernel debugging */
	InitKDebug();
#endif

	/* Initialise IOdebug() system */
	InitSemaphore(&root->IODebugLock,1);
	root->IODebugPort = NullPort;


/******************************** Set processor type */

	root->MachineType = GetCPUType();

# ifdef __SMT /* split module table */
/******************************** Initialise the code pointer index */

#  define INITCPI 32	/* number of slots in initial code pointer index */

	{
		MPtr mod;
		int slot = 1;
		
		/* allocate initial code pointer index */
		root->cpi = (word *)Allocate(INITCPI*sizeof(word),root->FreePool,&root->SysPool);
		root->cpislots = INITCPI;
#  ifdef __TRAN
		/* @@@ assumes Alloc zeros alloc'ed memory */
#  else
		ZeroBlock(root->cpi,INITCPI*sizeof(word));
#  endif			
		/* allocate code pointer table for each resident library */
		/* notes that table is not yet initialised by oring addr with 1 */
		while(1)
		{
			mod = (MPtr)MRTOA_(MInc_(loadbase,slot*sizeof(MPtr)));

			if( ModuleWord_(mod,Type) != T_Module) break;

#ifdef __C40
			root->cpi[slot] = C40WordAddress(Allocate(ModuleWord_(mod,MaxCodeP),root->FreePool,&root->SysPool)) | 1;
#else
			root->cpi[slot] = ((word) Allocate(ModuleWord_(mod,MaxCodeP),root->FreePool,&root->SysPool)) | 1;
#endif
			slot++;
		}
		InitSemaphore(&root->cpi_op,1); /* serialise cpi operations */
	}
# endif

/******************************** Initialise and run processor manager */

	if( config->FirstProg == 0 ) config->FirstProg = IVecProcMan;

	task->Program = (MPtr)MRTOA_(MInc_(loadbase,config->FirstProg*sizeof(MPtr)));

	InitPool( &task->MemPool );
	task->Port = NullPort;
	
#ifdef __ARM
	/* Register handler for executive critical errors */
	/* Errors are reported via KDebug interface */
	DefineExecErrorHandler(ExecErrorHandler);
#endif

	TaskInit( task );		/* run the processor manager */

#ifdef __ARM
	/* @@@ could dispose of allocated startup stack in same fashion. */
	_FreeMemStop(startupstate);
#else
	Stop();				/* Executive provided startup stack no longer required */
#endif
}

/*}}}*/

#elif defined(__C40)


/*{{{  C40 KStart */

#define STARTUPSTACKSIZE 4096	/* size of stack used for kernel initialisation */
void SetUserStackAndJump(word ar1, word arg2, void *, VoidFnPtr);
void KernelInit(word bootlink, MPtr loadbase);
void _FreeMemStop(void *);

/* Called from bootstrap with link number booted from and address of nucleus */
void Kstart(word bootlink, MPtr loadbase)
{
	RootStruct *root = GetRoot();
	word confsize = 0;
	Config *config = (Config *)(root + 1);

/******************************** initialise Executive */

	ExecInit();		/* initialise executive & start system timer */

/******************************** Get configuration information */

	/* Only download config vector if we are not a ROM'ed system, and */
	/* the bootstrap has not done this for us already. */

	if (bootlink == -1) {
		/* If we are a ROM booted system, then the config structure  */
		/* will already have been initialised by the bootstrap.      */
		/* Its size has been placed in the ImageSize element of the  */
		/* structure (size needs to be passed as the number of links */
		/* defined may be non standard). This use of ImageSize is    */
		/* temporary, the kernel will set it to the nucleus's size.  */
		confsize = config->ImageSize;
	      }
	else if (confsize == 0 && bootlink == 0)
	  {
	    /* The Shared Memory Link version of the Nucleus stores the config */
	    /* size in the StartUpStack field of the ExecRoot structure,       */
	    /* (since this is the only structure accessible to both the kernel */
	    /* and the bootstrap.  See NHG for futher details                  */

	    confsize = GetExecRoot()->StartupStack;
	
	} else {
		/* bootlink != -1, so we must have been booted down a link.  */
		/* Get config vector from the same link we were booted down. */

		/* read configuration vector size */
		_LinkRx(1, bootlink, C40WordAddress(&confsize));
		/* read word multiple adjusted sized configuration vector */
		_LinkRx(((confsize + 3 ) & ~3) >> 2, bootlink,
							C40WordAddress(config));
	}

	config->LoadBase = loadbase;
	config->ImageSize = MWord_(loadbase,0);

	root->Flags = config->Flags;

	/* save config pos for GetConfig macro */
	root->Configuration = (word *)config;

#ifdef CODES
	root->ErrorCodes = InitErrorCodes();
#endif

/******************************** Initialise Memory subsystem */

	MemInit(config, confsize);

	/* Stop using the on-chip RAM as stack space and swap to a stack */
	/* in C addressable memory. */

	GetExecRoot()->StartupStack = (word)Allocate(STARTUPSTACKSIZE,
						root->FreePool, &root->SysPool);

	SetUserStackAndJump(bootlink, loadbase,
		((char *)(GetExecRoot()->StartupStack)) + STARTUPSTACKSIZE-4,
		KernelInit );
}

/* Second stage kernel initialisation after it has setup its own stack */
void KernelInit(word bootlink, MPtr loadbase)
{
	RootStruct *root = GetRoot();
	Config *config = (Config *)(root + 1);
	Task *task;

/******************************** Initialise other subsystems */

	/* As the port garbage collector has some unidentified problems */
	/* with large task forces, default to having it turned off */
	root->GCControl = FALSE;

	PortInit( config );

	task = (Task *)Allocate( sizeof(Task), root->FreePool, &root->SysPool );

#ifdef LATENCYTEST
	NewWorker(DispatchLatTest);
#endif

	EventInit( config );
	
#ifdef LINKIO
	LinkInit( config, task );
#endif

	/* Watchdog timers are required to be reset at specific time */
	/* intervals. Helios startup time on a 40Mhz C40 is ~750 Ms. */
	/* Watchdog timers are usually a shorter period than this! */
	/* This code therefore allows an escape for the user to intercept */
	/* and handle the timer or reset it. It could be used for other */
	/* purposes, but as you cannot call kernel fns, etc, it seems of */
	/* limited use. */

	if (config->Spare[0]) {
		fncast f;

		f.w = C40WordAddress(((char *)&config->Spare[0]) + config->Spare[0]);

		f.vfn(config);
	}


	MemInit2();

#ifdef KERNELDEBUG3
	/* initialise kernel debugging */
	InitKDebug();
#endif
	/* Initialise IOdebug() system */
	InitSemaphore(&root->IODebugLock,1);
	root->IODebugPort = NullPort;

/******************************** Set processor type */

	root->MachineType = 0x320C40;


# ifdef __SMT /* split module table */
/******************************** Initialise the code pointer index */

#  define INITCPI 32	/* number of slots in initial code pointer index */

	{
		MPtr mod;
		word slot = 1;
		
		/* allocate initial code pointer index */
		root->cpi = (word *)Allocate(INITCPI*sizeof(word),root->FreePool,&root->SysPool);
		root->cpislots = INITCPI;

		ZeroBlock(root->cpi,INITCPI*sizeof(word));

		/* allocate code pointer table for each resident library */
		/* notes that table is not yet initialised by oring addr with 1 */
		while(1)
		{
			mod = (MPtr)MRTOA_(MInc_(loadbase,slot*sizeof(MPtr)));

			if( ModuleWord_(mod,Type) != T_Module) break;

			root->cpi[slot] = C40WordAddress(Allocate(ModuleWord_(mod,MaxCodeP),root->FreePool,&root->SysPool)) | 1;

			slot++;
		}
		InitSemaphore(&root->cpi_op,1); /* serialise cpi operations */
	}
# endif

/******************************** Initialise and run processor manager */

	if( config->FirstProg == 0 ) config->FirstProg = IVecProcMan;

	task->Program = (MPtr)MRTOA_(MInc_(loadbase,config->FirstProg*sizeof(MPtr)));

	InitPool( &task->MemPool );
	task->Port = NullPort;

	TaskInit( task );		/* run the processor manager */

	/* Executive provided startup stack no longer required */
	_FreeMemStop((char *)(GetExecRoot()->StartupStack));
}

/*}}}*/

#elif defined(__TRAN)

/*{{{  Transputer KStart */

void Kstart(Channel *bootlink, word *loadbase)
{
	RootStruct *root = GetRoot();
	Config *config = (Config *)(root + 1);
	word confsize;
	Task *task;


/******************************** Get configuration information */

	/* Only download config vector if we are not a ROM'ed system, and */
	/* the bootstrap has not done this for us already. */
	if ((confsize = GetROMConfig(config)) == 0 && bootlink != NULL)
	{
		/* config size is 0, so we must be a RAM loaded system */
		/* so get info from IO Server instead */
		in_(4,bootlink,&confsize);
		in_(confsize,bootlink,config);
	}

	config->LoadBase = loadbase;
	config->ImageSize = loadbase[0];

	root->Flags = config->Flags;

	/* save config pos for GetConfig macro */
	root->Configuration = (word *)config;
	
#ifdef CODES
	root->ErrorCodes = InitErrorCodes();
#endif

/******************************** Initialise subsystems */

	MemInit(config, confsize); /* v1.2 additional parameter, config permanently stored */

#if defined(TESTER)
	ExecInit();
#endif

	/* As the port garbage collector has some unidentified problems */
	/* with large task forces, default to having it turned off */
	root->GCControl = FALSE;

	PortInit( config );

	task = Allocate( sizeof(Task), root->FreePool, &root->SysPool );

#ifdef LINKIO
	LinkInit( config, task );
#endif

#ifdef TRANSPUTER
	EventInit( config );
#endif

	MemInit2();

#ifdef KERNELDEBUG3
	/* initialise kernel debugging */
	InitKDebug();
#endif

	/* Initialise IOdebug() system */
	InitSemaphore(&root->IODebugLock,1);
	root->IODebugPort = NullPort;

	Sleep( 1000 );		/* 1000 give system time to settle	*/

/******************************** Set processor type */

#if defined(TRANSPUTER)
	root->MachineType = rev_(lddevid_(-1,414,800));
	/* if result is -1 then processor has lddevid instruction */
	/* decode the result to give its type.			  */
	if( root->MachineType < 0 )
	{
		int id = lddevid_(0,0,0);
		if  ( id < 10 ) root->MachineType = 425;
		elif( id < 20 ) root->MachineType = 805;
		elif( id < 30 ) root->MachineType = 801;
		elif( id < 40 ) root->MachineType = 400 ; /* unused - default */
		elif( id < 50 ) root->MachineType = 222; /* unlikely */
		elif( id < 60 ) root->MachineType = 400;
		else root->MachineType = 400;
	}
#endif

/******************************** Initialise and run processor manager */

	if( config->FirstProg == 0 ) config->FirstProg = IVecProcMan;

	task->Program = (MPtr)RTOA(loadbase[config->FirstProg]);

	InitPool( &task->MemPool );
	task->Port = NullPort;
	
	TaskInit( task );		/* run the processor manager */

	Stop();
}

/*}}}*/

#else /* if (!ARM && !C40 && !TRAN) */

/*{{{  Everything else */

/* Called from bootstrap with link number booted from and address of nucleus */
void Kstart(word bootlink, word *loadbase)
{
	RootStruct *root = GetRoot();
	Config *config = (Config *)(root + 1);
	word confsize;
	Task *task;

#if 0 	/* quick and dirty debug */
/* For conning IOServer into thinking helios has now fully booted, */
/* so we can send IOdebug()'s and KDebug()'s straight away */
	char syncIt[12];

	syncIt[0] = 0xf0;
	syncIt[1] = 0;
	syncIt[2] = 0;
	syncIt[3] = 0;

	syncIt[4] = 0;
	syncIt[5] = 0;
	syncIt[6] = 0;
	syncIt[7] = 0;

	syncIt[8] = 0;
	syncIt[9] = 0;
	syncIt[10] = 0;
	syncIt[11] = 0;
#endif

/******************************** initialise Executive */

	ExecInit();		/* initialise executive & start system timer */

/******************************** Get configuration information */

	if (bootlink == -1) {
		/* If we are a ROM booted system, then the config structure  */
		/* will already have been initialised by the bootstrap.      */
		/* Its size has been placed in the ImageSize element of the  */
		/* structure (size needs to be passed as the number of links */
		/* defined may be non standard). This use of ImageSize is    */
		/* temporary, the kernel will set it to the nucleus's size.  */
		confsize = config->ImageSize;
	} else {
		/* bootlink != -1, so we must have been booted down a link.  */
		/* Get config vector from the same link we were booted down. */

		_LinkRx(4, bootlink, &confsize);	/* read size */
		_LinkRx(confsize, bootlink, config);	/* read config */

#ifdef __ARM
		/* set parent and (for now) debug bits on boot link */
		/* only one permanent link on arm */
		config->NLinks = 1; /* as IO Server config has not changed */
#endif
	}

#if 0
	/* quick 'n dirty debug */
	/* send sync early so we can IOdebug() straight away */
	_LinkTx(12, bootlink, &syncIt[0]);
#endif

	config->LoadBase = loadbase;
	config->ImageSize = MWord_(loadbase, 0)

	root->Flags = config->Flags;

	/* save config pos for GetConfig macro */
	root->Configuration = (word *)config;
	
#ifdef CODES
	root->ErrorCodes = InitErrorCodes();
#endif

/******************************** Initialise subsystems */

	MemInit(config, confsize); /* v1.2 additional parameter, config permanently stored */

	/* As the port garbage collector has some unidentified problems */
	/* with large task forces, default to having it turned off */
	root->GCControl = FALSE;

	PortInit( config );

	task = Allocate( sizeof(Task), root->FreePool, &root->SysPool );

#ifdef LINKIO
	LinkInit( config, task );
#endif

	EventInit( config );

	MemInit2();

#ifdef KERNELDEBUG3
	/* initialise kernel debugging */
	InitKDebug();
#endif
	/* Initialise IOdebug() system */
	InitSemaphore(&root->IODebugLock,1);
	root->IODebugPort = NullPort;


/******************************** Set processor type */

#if defined(__TRAN) && defined(TRANSPUTER)
	root->MachineType = rev_(lddevid_(-1,414,800));
	/* if result is -1 then processor has lddevid instruction */
	/* decode the result to give its type.			  */
	if( root->MachineType < 0 )
	{
		int id = lddevid_(0,0,0);
		if  ( id < 10 ) root->MachineType = 425;
		elif( id < 20 ) root->MachineType = 805;
		elif( id < 30 ) root->MachineType = 801;
		elif( id < 40 ) root->MachineType = 400 ; /* unused - default */
		elif( id < 50 ) root->MachineType = 222; /* unlikely */
		elif( id < 60 ) root->MachineType = 400;
		else root->MachineType = 400;
	}
#elif defined(__ARM)
	/* @@@ add code to distinguish between ARM2/3/6 */
	root->MachineType = 0xA2; /* arm2 */
#elif defined(__C40)
	root->MachineType = 0x320C40;
#elif defined(__i860)
	root->MachineType = 860;
#else
# error Processor unknown - MachineType not defined
#endif


#ifdef __SMT /* split module table */
/******************************** Initialise the code pointer index */

#define INITCPI 32	/* number of slots in initial code pointer index */

	{
		Module *mod;
		int slot = 1;
		
		/* allocate initial code pointer index */
		root->cpi = Allocate(INITCPI*sizeof(word),root->FreePool,&root->SysPool);
		root->cpislots = INITCPI;
#ifdef __TRAN
		/* @@@ assumes Alloc zeros alloc'ed memory */
#else
		ZeroBlock(root->cpi,INITCPI*sizeof(word));
#endif			
		/* allocate code pointer table for each resident library */
		/* notes that table is not yet initialised by oring addr with 1 */
		while((mod = (Module *)RTOA(loadbase[slot]))->Type == T_Module)
		{
#ifdef __C40
			root->cpi[slot] = C40WordAddress(Allocate(mod->MaxCodeP,root->FreePool,&root->SysPool)) | 1;
#else
			root->cpi[slot] = (word)Allocate(mod->MaxCodeP,root->FreePool,&root->SysPool) | 1;
#endif
			slot++;
		}
		InitSemaphore(&root->cpi_op,1); /* serialise cpi operations */
	}
#endif

/******************************** Initialise and run processor manager */

	if( config->FirstProg == 0 ) config->FirstProg = IVecProcMan;

	task->Program = (Program *)RTOA(loadbase[config->FirstProg]);

	InitPool( &task->MemPool );
	task->Port = NullPort;
	
#ifdef __ARM
	/* Handler will be soon be taken over by procman, as we can send */
	/* IOdebug()s from there. */

	/* register handler for executive critical errors */
	DefineExecErrorHandler(ExecErrorHandler);
#endif

	TaskInit( task );		/* run the processor manager */

	Stop();				/* Executive provided startup stack no longer required */
}

/*}}}*/

#endif


/*--------------------------------------------------------
-- MachineType						--
--							--
-- Return this processor's type, presently either T414,	--
-- T800, ARM2, i860 or 'C40 (414,800,0xA2,860 or 0xC40) --
-- Problem: how will we detect a 425?			--
--							--
--------------------------------------------------------*/

word MachineType(void)
{
	RootStruct *root = GetRoot();
	return root->MachineType;
}

/*--------------------------------------------------------
-- Delay						--
--							--
-- Delay the given number of microseconds		--
--							--
--------------------------------------------------------*/

#ifndef __TRAN
void Sleep(word time);
#endif

void Delay(word time)
{
#ifdef __TRAN
# ifdef TRANSPUTER
	if( ldpri_() == 1 ) time = time/64;
	Sleep(time);
# else
	Sleep(time);
# endif
#else
	System((WordFnPtr)Sleep,time);
#endif
}

#ifdef __TRAN
word (Timer)(void)
{
	return Timer();
}
#endif


/*--------------------------------------------------------
-- Debugging						--
--							--
--							--
--------------------------------------------------------*/

void debug( word *tv, word value );

void debinit(void)
{
	GetRoot()->TraceVec[0] = 4;
}

void _Mark(void)
{
	word *tv = GetRoot()->TraceVec;
	
	if( (*tv & TVEnable) == 0 )
	{
		debug(tv,0x22222222);
		debug(tv,Timer());
#ifdef __TRAN
		debug(tv,ldpri_()|ldlp_(7));
		debug(tv,ldl_(4));
#else
		debug(tv, _spreg());	/* returns callers sp */
		debug(tv, _linkreg());	/* returns pointer to caller */
		debug(tv, (word)_GetModTab());/* returns callers module table */
#endif
	}
}

#ifdef __TRAN
void _Trace(...)
#else
void _Trace(word a, word b, word c)
#endif
{
	word *tv = GetRoot()->TraceVec;

	if( (*tv & TVEnable) == 0 )
	{
		debug(tv,0x11111111);
		debug(tv,Timer());
#ifdef __TRAN
		debug(tv,ldpri_()|ldlp_(7));
		debug(tv,ldl_(4));
		debug(tv,ldl_(6));
		debug(tv,ldl_(7));
		debug(tv,ldl_(8));				
#else
		debug(tv, _spreg());	/* returns callers sp */
		debug(tv, _linkreg());	/* returns pointer to caller */
		debug(tv, (word)_GetModTab());/* returns callers module table */
		debug(tv,a);
		debug(tv,b);
		debug(tv,c);
#endif
	}
#if defined(SYSDEB) && defined(__C40)
	JTAGHalt();
#endif
}

void _Halt()
{
	_Mark();
	Stop();
}

void debug(word *tv,word value)
{
	word offset = (tv[0] & TVMask)/4;
	
	tv[offset] = value;
	
	/* if that was the last word in the vector, inc by 8 so wrap-round */
	/* skips over control word					   */
	if( (offset & (TVMask/4)) == (TVMask/4) ) tv[0] += 8;
	else tv[0] += 4;
}


#ifdef __ARM
#include <signal.h>

extern Task *_Task_;	/* entry in kernel modtab holding task pointer */
void my_backtrace(int *fp,int *sp,int *pc);

char *EmbeddedName(word *fn);

/* Executive Critical Error handler
 *
 * Registered by DefineExecErrorHandler and Entered via the
 * ExecRoot.CriticalErrorVector. The handler is entered in SVC mode with
 * IRQ interrupts disabled. It is assumed that the timeslice clock interrupt
 * is via an IRQ.
 *
 * Note that the stack has been swapped to the user stack. This may cause
 * problems if A) the user stack register does not point at a valid free
 * stack area, or B) The user stack itself 'invaded' protected memory
 * and caused the failure., the user stack being only used if the
 * error is recoverable?
 *
 */

#ifdef KERNELDEBUG3
/* use blocking debug */
void sKDebug(char *, ...);
# define KDebug sKDebug
#endif

void ExecErrorHandler(word signal, char *errmsg, word *regs, bool unrecoverable)
{
	ExecRoot *xroot = GetExecRoot();
	unsigned long pc = (unsigned long)regs[15];

#ifdef KERNELDEBUG3
	IntsOff();
#endif

	if (unrecoverable)
		KDebug("Unrecoverable Exec Error!\n");
	else
		KDebug("Recoverable Exec Error!\n");
	
	KDebug("[\n");
	KDebug("Signal #            : %d\n", signal);
	KDebug("Error               : %s\n", errmsg);

	KDebug("\n");

	KDebug("Task name           : ");

	if (regs[9] > 0x2e00
	 && regs[9] < (word)GetRoot()->TraceVec
	 && *((word *)regs[9]) == regs[9]) {
		/* ModTab is validated by checking its within the bounds of */
		/* acceptable memory and then checking that its first entry */
		/* points to itself. */

		/* Now check that task pointer and task name pointer are OK. */
		if ((word)_Task_ > 0x2e00
		 && (word)_Task_ <(word)GetRoot()->TraceVec
		 && (word)_Task_->TaskEntry > 0x2e00
		 && (word)_Task_->TaskEntry < (word)GetRoot()->TraceVec
		) {
			/* index into procman private struct to get task name */
			char *name = (char *)((word *)(_Task_->TaskEntry)+2);

			if (name > (char *)0x2e00
			 && name < (char *)GetRoot()->TraceVec
			) {
				char *chkname = name;

				while (*chkname >= 32 && *chkname < 127 )
					chkname++;
				if (*chkname != 0)
					KDebug("<unknown> Due to non ASCII char in name.\n");
				else
					KDebug("%s\n", name);
			} else {
				KDebug("<unknown> Due to invalid task name ptr %x.\n", name);
			}
		} else {
			KDebug("<unknown> Due to invalid task ptr (%x).\n", _Task_);
		}
	} else {
		KDebug("<unknown> Due to invalid module table pointer (r9) (%x).\n", regs[9]);
	}

	KDebug("\n");

	if ((word)xroot->CurrentSaveArea < 0x2e00 || (word)xroot->CurrentSaveArea > (word)GetRoot()->TraceVec) {
		KDebug("Warning             : Invalid CPU save state pointer.\n");
	} else {
		KDebug("Thread's initial fn : ");

		if (xroot->CurrentSaveArea->InitialFn == NULL) {
			KDebug(" <function name> is invalid");
		} else {
			char *name, *chkname;
			fncast fc;

			fc.vfn = xroot->CurrentSaveArea->InitialFn;

			name = chkname = EmbeddedName(fc.wp);

			while (*chkname >= 32 && *chkname < 127 )
				chkname++;
			if (*chkname != 0)
				KDebug("<function name> unknown");
			else
				KDebug(" %s", name);
		}
		KDebug(" (@ %x) priority: %d\n",
			xroot->CurrentSaveArea->InitialFn,
			xroot->CurrentSaveArea->priority);

		KDebug("Thread's start time : %x (S), execution time : %x (mS)\n",
			xroot->CurrentSaveArea->InitialTime,
			xroot->CurrentSaveArea->CPUTimeTotal);

		KDebug("Thread's SaveState  : %x.\n", xroot->CurrentSaveArea);
	}

	KDebug("\n");
	KDebug("Register state      :\n");

	KDebug(" r0/a1:  %x\t   r1/a2:    %x\t   r2/a3:       %x\n",
		regs[0], regs[1], regs[2]);
	KDebug(" r3/a4:  %x\t   r4/v1:    %x\t   r5/v2:       %x\n",
		regs[3], regs[4], regs[5]);
	KDebug(" r6/v3:  %x\t   r7/v4:    %x\t   r8/v5:       %x\n",
		regs[6], regs[7], regs[8]);
	KDebug(" r9/mt:  %x\t  r10/use:   %x\t  r11/fp:       %x\n",
		regs[9], regs[10], regs[11]);
	KDebug("r12/tmp: %x\t  r13/sp:    %x\t  r14/lr:       %x\n",
		regs[12], regs[13], regs[14]);

	{
		pc = (unsigned long)regs[15];

		KDebug("\n");
		KDebug("r15/pc:  %x", pc);

		KDebug(   "\t  flags: %c%c%c%c%c%c",
			(pc & (unsigned long)NFlag) ? 'N' : 'n',
			(pc & ZFlag) ? 'Z' : 'z',
			(pc & CFlag) ? 'C' : 'c',
			(pc & VFlag) ? 'V' : 'v',
			(pc & IRQDisable) ? 'I' : 'i',
			(pc & FIQDisable) ? 'F' : 'f' );

		KDebug(" mode: %s\n",
			(pc & ModeMask == SVCMode) ? "SVC" :
			(pc & ModeMask == IRQMode) ? "IRQ" :
			(pc & ModeMask == FIQMode) ? "FIQ" : "User");
	}

	KDebug("\n");

	/* Remove PSR flags from PC */
	pc = regs[15] & ~0xfc000003;

	if ((signal == SIGSEGV || signal == SIGILL) && pc < (word)GetRoot()->TraceVec)
		KDebug("Faulting instruction: %x\n", *((word *)pc) );

	if (regs[13] < 0x2e00 || regs[13] > (word)GetRoot()->TraceVec)
		KDebug("Warning             : Unlikely stack pointer (r13).\n");

	if (regs[13] < regs[10])
		KDebug("Warning             : Stack pointer (r13) is lower than Stack base (r10).\n");

	if (pc < 0x2e00 || pc > (word)GetRoot()->TraceVec)
		KDebug("Warning             : Unlikely PC (r15).\n");

	KDebug("\n");
	my_backtrace((int *)regs[11],(int *)regs[13],(int *)regs[15]); /*fp,sp,pc*/


	if (unrecoverable) {
		/* @@@ maybe we should call KillTask directly - but this would */
		/* leave a procman thread around, + loader would still have */
		/* for prog. - prehaps we should do both? */

		/* If we have an invalid module table then there is no chance */
		/* that we know the task associated with this error. So just */
		/* stop thread. */

		/* ModTab validated by checking its within the bounds of */
		/* acceptable memory and then checking that its first entry */
		/* points to itself. */
		if (regs[9] > 0x2e00
		 && regs[9] < (word)GetRoot()->TraceVec
		 && *((word *)regs[9]) == regs[9]) {
			MCB m;
			word e = SIGABRT;

			KDebug("\nAction              : Aborting this task\n]\n");

			/* send exception to task IOC to kill us off */
			m.MsgHdr.DataSize = 0;
			m.MsgHdr.ContSize = 1;
			m.MsgHdr.Flags	= MsgHdr_Flags_exception;
			m.MsgHdr.Dest	= _Task_->IOCPort;
			m.MsgHdr.Reply	= NullPort;
			m.MsgHdr.FnRc	= EC_Error|SS_Executive|EG_Exception|EE_Kill;
			m.Timeout	= -1;
			m.Control	= &e;

			PutMsg(&m);
			/*SendException(_Task_->IOCPort,EC_Error|SS_Executive|EG_Exception|EE_Kill);*/
		} else {
			KDebug("\n");
			KDebug("Action              : Halting this thread\n]\n");
		}
		Stop();
	} else {	/* raise a signal */
#ifdef SYSDEB
		KDebug("\n");
		KDebug("Action              : Passing through to signal handler\n]\n");
#endif
		EnterUserMode();
		CallException(_Task_, signal);
	}
}


# ifdef __ARM
/* When passed a function pointer, it returns the ascii text of the function */
/* name that is embedded in the code image, or NULL if not found. */

char *EmbeddedName(word *fn) {
	word w, i;

	fn = (word *)((word)fn & ~0xfc000003);	/* remove any flags and word align */

	/* Search up to 10 words before the STM looking for */
	/* the marker that shows where the function name is. */

	for (i = 0; (i < 10); i++) {
		w = *--fn;
		if ((w & 0xffff0000) == 0xff000000) {
			return (char *)fn - (w & 0xffff);
		}
	}

	return NULL;
}

/*---------------------------------------------------------------------------*/
/* _backtrace: non-terminating backtrace information
 * called by the following assembler fragment
 * |my_postmortem|
 *       MOV     a3,lk
 *       MOV     a2,sp
 *       MOV     a1,fp
 *       B       |my_backtrace|
 *
 * At the moment this code only dumps the function names and does NOT
 * Output the entry state of the functions (though this information can
 * be derived from the registers referenced in the entry STM instruction)
 */

void my_backtrace(int *fp, int *sp, int *pc)
{
	int c = 0;
	/* step down through the frame-pointer list printing function names */
	KDebug("Stack backtrace     : ");

	while (fp != 0) {	/* frame-pointer reference */
		int  *z;
		char *name;

		if (c++ > 100) {
			KDebug("<loop detected in backtrace>]\n");
			return;
		}
		z = (int *) ((fp[0] & 0x03fffffc) - 12);
		if (z < (int *)0x2e00 || z > (int *)GetRoot()->TraceVec) {
			KDebug("<corrupt frame pointer %x>\n", z);
			return;
		}

		/* sp = (int *)fp[-2]; */

		/* Returns name embedded in binary code, or NULL if not found */
		name = EmbeddedName((word *)z);

		if (name == NULL) {
			KDebug("<Cannot find fn name - backtrace end>\n");
			return;
		}
		KDebug("%s, ", name);
		fp = (int *)fp[-3];
	}

	KDebug("<top of frame>\n");

	return;
}

#ifdef KDebug
# undef KDebug
#endif

void backtrace(void)
{
	int	c = 0;
	int	*fp = (int *)_fpreg();

	/* step down through the frame-pointer list printing function names */
	KDebug("Stack backtrace     : ");

	while (fp != 0) {	/* frame-pointer reference */
		int  *z;
		char *name;

		if (c++ > 100) {
			KDebug("<loop detected in backtrace>\n");
			return;
		}

		z = (int *) ((fp[0] & 0x03fffffc) - 12);

		if (z < (int *)0x2e00 || z > (int *)GetRoot()->TraceVec) {
			KDebug("<corrupt frame pointer %x>\n", z);
			return;
		}

		/* sp = (int *)fp[-2];*/

		/* Returns name embedded in binary code, or NULL if not found */
		name = EmbeddedName((word *)z);

		if (name == NULL) {
			KDebug("<Cannot find fn name - backtrace end>\n");
			return;
		}

		KDebug("%s, ", name);
		fp = (int *)fp[-3];
	}

	KDebug("<top of frame>\n");

	return;
}
# endif
#endif


/* -- End of kstart.c */

