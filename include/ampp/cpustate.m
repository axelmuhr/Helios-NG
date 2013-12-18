-- File:	cpustate.m
-- Subsystem:	Generic Helios executive
-- Author:	P.A.Beskeen
-- Date:	Nov '91
--
-- Description: SaveState and CPURegs structures
--
-- WARNING:	These definition must be kept up to date with <cpustate.h>.
--
--
-- RcsId: $Id: cpustate.m,v 1.3 1993/08/05 17:06:06 paul Exp $
--
-- (C) Copyright 1991-3 Perihelion Software Ltd.


_report ['include 'cpustate.m]
_def 'cpustate.m_flag 1


include structs.m

_if _defp 'helios.C40 [
	include c40.m
]


-- CPURegs structure, used to hold CPU context of suspended threads

_if _defp 'helios.arm [
	--		C REG NAME	ASM ALIASES	PCS USE:
	struct CPURegs [
		word	R_A1 	   --	a1/r0		argument variables
		word	R_A2 	   --	a2/r1
		word	R_A3 	   --	a3/r2
		word	R_A4 	   -- 	a4/r3
		word	R_V1 	   -- 	v1/r4		register variables
		word	R_V2 	   -- 	v2/r5
		word	R_V3 	   -- 	v3/r6
		word	R_V4 	   -- 	v4/r7
		word	R_V5 	   --	v5/r8
		word	R_MT 	   --	mt/dp/r9	module table pointer
		word	R_USE 	   --	use/sl/r10	stack limit
		word	R_FP 	   --	fp/r11		frame pointer
		word	R_TMP 	   --	tmp/ip/r12	temporary register
		word	R_SVC_SP   --	r13_svc		SVC stack pointer
		word	R_SVC_LR   --	r14_svc		SVC link register
		word	R_PC 	   --	pc/st/r15	program counter
		word	R_USER_SP  --	usp/sp/r13	User mode stack pointer
		word	R_USER_LR  --	lr/lk/r14	User mode link register
		word	R_CPSR     --	cpsr		ARM6 psr
	]
]


_if _defp 'helios.C40 [
			-- C PCS BINDING NAME:
					-- 'C40 REGISTER NAME:
						-- FUNCTION:
	struct CPURegs [
		word	PC		-- PC:	of sliced thread
	
		word	ST		-- st:	status reg
						-- C ADDRESS REGS
		word	ADDR1		-- ar0:
		word	ADDR2		-- ar1:
		word	ADDR3		-- ar2:
		word	ADDR4		-- ar3:
						-- MISC ADDRESS REGS
		word	MT		-- ar4:    module table pointer
		word	ATMP		-- ar5:    temporary address reg
		word	USP		-- ar6:    user stack pointer
		word	FP		-- ar7:    frame pointer

						-- REGISTER ARGUMENTS
		word	A1		-- r0 :    first arg and result reg
		word	R0f		-- r0 :    fp extension to 32bit reg

		word	A2		-- r1 :    32bits
		word	R1f		-- r1 :    fpext
		word	A3		-- r2 :    32bits
		word	R2f		-- r2 :    fpext
		word	A4		-- r3 :    32bits
		word	R3f		-- r3 :    fpext
	
						-- REGISTER VARIABLES
		word	V1		-- r4 :    32bits
		word	R4f		-- r4 :    fpext
		word	V2		-- r5 :    32bits
		word	R5f		-- r5 :    fpext
		word	V3		-- r6 :    32bits
		word	R6f		-- r6 :    fpext
		word	V4		-- r7 :    32bits
		word	R7f		-- r7 :    fpext

						-- TEMPORARY REGISTERS
		word	T1		-- r8 :    32bits
		word	R8f		-- r8 :    fpext
		word	T2		-- r9 :    32bits
		word	R9f		-- r9 :    fpext
		word	T3		-- r10:    32bits
		word	R10f		-- r10:    fpext

						-- MISC REGISTERS
		word	LR		-- r11:   link register
		word	R11f		-- r11:   fpext
		word	V6		-- dp :   variable reg (data page ptr)
		word	BASE		-- ir0:   byte address base
		word	USE		-- ir1:   user stack end pointer
		word	V5		-- bk :   variable reg

						-- TEMPORARY BACK-END REGS
		word	TMP1		-- rs :
		word	TMP2		-- re :
		word	TMP3		-- rc :
	]

	-- last register in CPU context
	_def 'CPURegs.LASTREG	CPURegs.TMP3
]


-- The save state structure used for holding the state of a suspended thread
struct SaveState [
	word	next			-- for queueing on run Q's
	word	nextknown		-- for exec housekeeping
	word	priority		-- thread priority
	word	endtime			-- Wakeup time if Sleep()ing
	word	status			-- Thread status
	word	stack_chunk		-- current stack chunk header
	word	TimedWaitUtil		-- true if OK, false if timedout
	word	CPUTimeTotal		-- milliseconds of CPU time used
	word	LastTimeStamp		-- Time stamp at last resume/slice (uS)
	word	InitialTime		-- Startup time of thread (Msecs)
	word	InitialFn		-- WPTR to thread's root fn
	struct	CPURegs CPUcontext	-- CPU state of this thread
]


-- SaveState status values

_def 'THREAD_STARTUP		0	-- thread is just starting, use RestoreSlicedState()
_def 'THREAD_SLICED		1	-- runnable, sliced or interrupted, RestoreSlicedState()
_def 'THREAD_RUNNABLE		2	-- runnable, resheduled, needs RestoreCPUState()
_def 'THREAD_RUNNING		3	-- current CPU thread 
_def 'THREAD_KILLED		4	-- thread has been Stop()'ed 
_def 'THREAD_BOGUS		5	-- illegal state of thread 
					-- THREAD_SLICED status in normal dispatch
_def 'THREAD_SAVED		6	-- only use for user SaveCPUState() 
_def 'THREAD_SLEEP		7	-- on timer Q
_def 'THREAD_TIMEDWAIT		8	-- on timer and semaphore Q's 
_def 'THREAD_SEMAPHORE		9	-- on semaphore Q 
_def 'THREAD_MSGREAD		10	-- blocked reading msg 
_def 'THREAD_MSGWRITE		11	-- blocked writing internal msg 

_def 'THREAD_MULTIWAIT		12	-- blocked during MultiWait

_def 'THREAD_LINKRX		13	-- blocked reading external msg 
_def 'THREAD_LINKTX		14	-- blocked writeing external msg 

_def 'THREAD_LINKWRITEQ		15	-- blocked on queue to write external msg 
_def 'THREAD_LINKWAIT		16	-- guardian waiting on dumb link 
_def 'THREAD_LINKEND		17	-- waiting for linktx/rx to complete 
					-- while in kernel: KillTask, 
					-- Configure, WaitLink or JumpLink 
_def 'THREAD_LINKXOFF		18	-- waiting for XON on link 

_def 'THREAD_LINKTHRU1		19	-- single buffering thru-routed msg 
_def 'THREAD_LINKTHRU2		20	-- double buffering thru-routed msg 

_def 'THREAD_DMAREQ		21	-- waiting for a DMA engine (not used)



-- end of cpustate.m

