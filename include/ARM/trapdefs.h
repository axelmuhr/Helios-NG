/*
 * File:	trapdefs.h
 * Subsystem:	Helios-ARM header files.
 * Author:	P.A.Beskeen (Kernel library make system)
 * Date:	As timestamped
 *
 * Description: *AUTOMATICALLY* produced trap number definitions.
 *
 * WARNING:	Never alter these definitions by hand. Always update
 *		the library definition file.
 *
 *		<ARM/trap.h> should always be included in preference
 *		to this file.
 *
 *
 * (C) Copyright 1992 Perihelion Software Ltd.
 *     All Rights Reserved.
 */


#ifndef __trapdefs_h
#define __trapdefs_h

#include <ARM/trap.h>

#define TRAP_USERSTACKSAVEAREA	0


		
#define TRAP_NewPort		(1 | TRAP_STDHELIOS)
#define TRAP_FreePort		(2 | TRAP_STDHELIOS)
#define TRAP_PutMsg		(3 | TRAP_STDHELIOS)
#define TRAP_GetMsg		(4 | TRAP_STDHELIOS)
#define TRAP_PutReady		(5 | TRAP_STDHELIOS)
#define TRAP_GetReady		(6 | TRAP_STDHELIOS)
#define TRAP_AbortPort		(7 | TRAP_STDHELIOS)
#define TRAP_MultiWait		(8 | TRAP_STDHELIOS)
#define TRAP_SendException		(9 | TRAP_STDHELIOS)
#define TRAP_InitSemaphore		(10 | TRAP_STDHELIOS)
#define TRAP_Wait		(11 | TRAP_STDHELIOS)
#define TRAP_Signal		(12 | TRAP_STDHELIOS)
#define TRAP_TestSemaphore		(13 | TRAP_STDHELIOS)
#define TRAP_InitPool		(14 | TRAP_STDHELIOS)
#define TRAP_AllocMem		(15 | TRAP_STDHELIOS)
#define TRAP_FreeMem		(16 | TRAP_STDHELIOS)
#define TRAP_FreePool		(17 | TRAP_STDHELIOS)
#define TRAP_TaskInit		(18 | TRAP_STDHELIOS)
#define TRAP_KillTask		(19 | TRAP_STDHELIOS)
#define TRAP_CallException		(20 | TRAP_STDHELIOS)
#define TRAP_EnableLink		(21 | TRAP_STDHELIOS)
#define TRAP_AllocLink		(22 | TRAP_STDHELIOS)
#define TRAP_FreeLink		(23 | TRAP_STDHELIOS)
#define TRAP_Reconfigure		(24 | TRAP_STDHELIOS)
#define TRAP_Terminate		(25 | TRAP_STDHELIOS)
#define TRAP_LinkData		(26 | TRAP_STDHELIOS)
#define TRAP_Delay		(27 | TRAP_STDHELIOS)
#define TRAP__Halt		(28 | TRAP_STDHELIOS)
#define TRAP_InPool		(29 | TRAP_STDHELIOS)
#define TRAP_LinkIn		(30 | TRAP_STDHELIOS)
#define TRAP_LinkOut		(31 | TRAP_STDHELIOS)
#define TRAP_SetEvent		(32 | TRAP_STDHELIOS)
#define TRAP_RemEvent		(33 | TRAP_STDHELIOS)
#define TRAP_InitProcess		(34 | TRAP_STDHELIOS)
#define TRAP_StartProcess		(35 | TRAP_STDHELIOS)
#define TRAP_StopProcess		(36 | TRAP_STDHELIOS)
#define TRAP_GetPortInfo		(37 | TRAP_STDHELIOS)
#define TRAP_FreeMemStop		(38 | TRAP_STDHELIOS)
#define TRAP_SignalStop		(39 | TRAP_STDHELIOS)
#define TRAP_Configure		(40 | TRAP_STDHELIOS)
#define TRAP_TestWait		(41 | TRAP_STDHELIOS)
#define TRAP_SetPriority		(42 | TRAP_STDHELIOS)
#define TRAP_XchMsg		(43 | TRAP_STDHELIOS)
#define TRAP_AvoidEvents		(44 | TRAP_STDHELIOS)
#define TRAP_HardenedWait		(45 | TRAP_STDHELIOS)
#define TRAP_HardenedSignal		(46 | TRAP_STDHELIOS)
#define TRAP_System		(47 | TRAP_STDHELIOS)
#define TRAP__cputime		(48 | TRAP_STDHELIOS)
#define TRAP__ldtimer		(49 | TRAP_STDHELIOS)
#define TRAP_StatMem		(50 | TRAP_STDHELIOS)
#define TRAP_LowAllocMem		(51 | TRAP_STDHELIOS)
#define TRAP_TimedWait		(52 | TRAP_STDHELIOS)
#define TRAP_SliceState		(53 | TRAP_STDHELIOS)
#define TRAP_SliceQuantum		(54 | TRAP_STDHELIOS)
#define TRAP_IntsOn		(55 | TRAP_STDHELIOS)
#define TRAP_IntsOff		(56 | TRAP_STDHELIOS)
#define TRAP_EnterSVCMode		(57 | TRAP_STDHELIOS)
#define TRAP_EnableIRQ		(58 | TRAP_STDHELIOS)
#define TRAP_DisableIRQ		(59 | TRAP_STDHELIOS)
#define TRAP_EnableFIQ		(60 | TRAP_STDHELIOS)
#define TRAP_DisableFIQ		(61 | TRAP_STDHELIOS)
#define TRAP_ClockIntsOn		(62 | TRAP_STDHELIOS)
#define TRAP_ClockIntsOff		(63 | TRAP_STDHELIOS)
#define TRAP_KDebug		(64 | TRAP_STDHELIOS)

#define TRAP_LASTTRAPNUMBER	64


#endif /*__trapdefs_h*/

/* end of trapdefs.h */
