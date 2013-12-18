-- File:	trapdefs.m
-- Subsystem:	Helios-ARM header files.
-- Author:	P.A.Beskeen (Kernel library make system)
-- Date:	As timestamped
--
-- Description: *AUTOMATICALLY* produced trap number definitions.
--
-- WARNING:	Never alter these definitions by hand. Always update
--		the library definition file.
--
--		trap.m should always be included in preference to this file.
--
--
-- (C) Copyright 1992 Perihelion Software Ltd.
--     All Rights Reserved.

_report ['include trapdefs.m]
_def 'trapdefs.m_flag 1

_defq 'TRAP_USERSTACKSAVEAREA	0


		
_defq 'TRAP_NewPort		[(1 | TRAP_STDHELIOS)]
_defq 'TRAP_FreePort		[(2 | TRAP_STDHELIOS)]
_defq 'TRAP_PutMsg		[(3 | TRAP_STDHELIOS)]
_defq 'TRAP_GetMsg		[(4 | TRAP_STDHELIOS)]
_defq 'TRAP_PutReady		[(5 | TRAP_STDHELIOS)]
_defq 'TRAP_GetReady		[(6 | TRAP_STDHELIOS)]
_defq 'TRAP_AbortPort		[(7 | TRAP_STDHELIOS)]
_defq 'TRAP_MultiWait		[(8 | TRAP_STDHELIOS)]
_defq 'TRAP_SendException		[(9 | TRAP_STDHELIOS)]
_defq 'TRAP_InitSemaphore		[(10 | TRAP_STDHELIOS)]
_defq 'TRAP_Wait		[(11 | TRAP_STDHELIOS)]
_defq 'TRAP_Signal		[(12 | TRAP_STDHELIOS)]
_defq 'TRAP_TestSemaphore		[(13 | TRAP_STDHELIOS)]
_defq 'TRAP_InitPool		[(14 | TRAP_STDHELIOS)]
_defq 'TRAP_AllocMem		[(15 | TRAP_STDHELIOS)]
_defq 'TRAP_FreeMem		[(16 | TRAP_STDHELIOS)]
_defq 'TRAP_FreePool		[(17 | TRAP_STDHELIOS)]
_defq 'TRAP_TaskInit		[(18 | TRAP_STDHELIOS)]
_defq 'TRAP_KillTask		[(19 | TRAP_STDHELIOS)]
_defq 'TRAP_CallException		[(20 | TRAP_STDHELIOS)]
_defq 'TRAP_EnableLink		[(21 | TRAP_STDHELIOS)]
_defq 'TRAP_AllocLink		[(22 | TRAP_STDHELIOS)]
_defq 'TRAP_FreeLink		[(23 | TRAP_STDHELIOS)]
_defq 'TRAP_Reconfigure		[(24 | TRAP_STDHELIOS)]
_defq 'TRAP_Terminate		[(25 | TRAP_STDHELIOS)]
_defq 'TRAP_LinkData		[(26 | TRAP_STDHELIOS)]
_defq 'TRAP_Delay		[(27 | TRAP_STDHELIOS)]
_defq 'TRAP__Halt		[(28 | TRAP_STDHELIOS)]
_defq 'TRAP_InPool		[(29 | TRAP_STDHELIOS)]
_defq 'TRAP_LinkIn		[(30 | TRAP_STDHELIOS)]
_defq 'TRAP_LinkOut		[(31 | TRAP_STDHELIOS)]
_defq 'TRAP_SetEvent		[(32 | TRAP_STDHELIOS)]
_defq 'TRAP_RemEvent		[(33 | TRAP_STDHELIOS)]
_defq 'TRAP_InitProcess		[(34 | TRAP_STDHELIOS)]
_defq 'TRAP_StartProcess		[(35 | TRAP_STDHELIOS)]
_defq 'TRAP_StopProcess		[(36 | TRAP_STDHELIOS)]
_defq 'TRAP_GetPortInfo		[(37 | TRAP_STDHELIOS)]
_defq 'TRAP_FreeMemStop		[(38 | TRAP_STDHELIOS)]
_defq 'TRAP_SignalStop		[(39 | TRAP_STDHELIOS)]
_defq 'TRAP_Configure		[(40 | TRAP_STDHELIOS)]
_defq 'TRAP_TestWait		[(41 | TRAP_STDHELIOS)]
_defq 'TRAP_SetPriority		[(42 | TRAP_STDHELIOS)]
_defq 'TRAP_XchMsg		[(43 | TRAP_STDHELIOS)]
_defq 'TRAP_AvoidEvents		[(44 | TRAP_STDHELIOS)]
_defq 'TRAP_HardenedWait		[(45 | TRAP_STDHELIOS)]
_defq 'TRAP_HardenedSignal		[(46 | TRAP_STDHELIOS)]
_defq 'TRAP_System		[(47 | TRAP_STDHELIOS)]
_defq 'TRAP__cputime		[(48 | TRAP_STDHELIOS)]
_defq 'TRAP__ldtimer		[(49 | TRAP_STDHELIOS)]
_defq 'TRAP_StatMem		[(50 | TRAP_STDHELIOS)]
_defq 'TRAP_LowAllocMem		[(51 | TRAP_STDHELIOS)]
_defq 'TRAP_TimedWait		[(52 | TRAP_STDHELIOS)]
_defq 'TRAP_SliceState		[(53 | TRAP_STDHELIOS)]
_defq 'TRAP_SliceQuantum		[(54 | TRAP_STDHELIOS)]
_defq 'TRAP_IntsOn		[(55 | TRAP_STDHELIOS)]
_defq 'TRAP_IntsOff		[(56 | TRAP_STDHELIOS)]
_defq 'TRAP_EnterSVCMode		[(57 | TRAP_STDHELIOS)]
_defq 'TRAP_EnableIRQ		[(58 | TRAP_STDHELIOS)]
_defq 'TRAP_DisableIRQ		[(59 | TRAP_STDHELIOS)]
_defq 'TRAP_EnableFIQ		[(60 | TRAP_STDHELIOS)]
_defq 'TRAP_DisableFIQ		[(61 | TRAP_STDHELIOS)]
_defq 'TRAP_ClockIntsOn		[(62 | TRAP_STDHELIOS)]
_defq 'TRAP_ClockIntsOff		[(63 | TRAP_STDHELIOS)]
_defq 'TRAP_KDebug		[(64 | TRAP_STDHELIOS)]

_defq 'TRAP_LASTTRAPNUMBER	64


-- end of trapdefs.m
