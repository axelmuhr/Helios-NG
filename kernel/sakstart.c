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
/* $Id: sakstart.c,v 1.3 1990/11/21 19:12:31 nick Exp $ */

#define __in_kstart 1	/* flag that we are in this module */

#include "kernel.h"
#include <task.h>

void debinit(void);

/*--------------------------------------------------------
-- Kstart						--
--							--
-- Startup procedure, call initialisation entry points	--
-- of all kernel subsystems.				--
--							--
--------------------------------------------------------*/

void Kstart(Channel *bootlink, word *loadbase, word bootaddr)
{
	Config *config = GetConfig();
	RootStruct *root;
	Task *task;
	int i;
	
	config->MemSize = 0;
	
	for( i = 0; i < 4 ; i++ )
	{
		LinkConf *c = &(config->LinkConf[i]);
		c->Id = i;
		c->Mode = Link_Mode_Dumb;
		c->State = 0;
		c->Flags = 0;
	}
	
	config->LinkConf[(((word)bootlink&0x1f)-16)/4].Flags |= Link_Flags_parent|Link_Flags_debug;
	
	MemInit( config, sizeof(Config) );

	Sleep( 1000 );		/* give system time to settle	*/

	root = GetRoot();

#ifdef TRANSPUTER
	root->MachineType = rev_(lddevid_(425,414,800));
#endif	
	InitSemaphore(&root->IODebugLock,1);

	task = Allocate( sizeof(Task), root->FreePool, &root->SysPool );

	/* search for the first program to run */
	for( i = 2; loadbase[i] != 0; i++ )
	{
		Program *p = (Program *)RTOA(loadbase[i]);
		if( p->Module.Type == T_Program ) task->Program = p;
	}	
	
	InitPool( &task->MemPool );
	
	TaskInit( task );

	Stop();
}

word (Timer)(void)
{
	return Timer();
}

/*--------------------------------------------------------
-- MachineType						--
--							--
-- Return this processor's type, presently either 414	--
-- or 800.						--
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

void Delay(word time)
{
#ifdef TRANSPUTER
	if( ldpri_() == 1 ) time = time/64;
#endif
	Sleep(time);
}

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
		debug(tv,ldpri_()|ldlp_(7));
		debug(tv,ldl_(4));
	}
}

void _Trace(...)
{
	word *tv = GetRoot()->TraceVec;

	if( (*tv & TVEnable) == 0 )
	{
		debug(tv,0x11111111);
		debug(tv,Timer());
		debug(tv,ldpri_()|ldlp_(7));
		debug(tv,ldl_(4));
		debug(tv,ldl_(6));
		debug(tv,ldl_(7));
		debug(tv,ldl_(8));				
	}
}

void _Halt()
{
	_Mark();
	stopp_();
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

/* -- End of kstart.c */


