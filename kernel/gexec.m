-- File:	gexec.m
-- Subsystem:	Generic Helios executive
-- Author:	P.A.Beskeen
-- Date:	Sept '92
--
-- Description: Generic Helios executive assembler manifests
--
-- WARNING: These definition must be kept upto date with "gexec.h"
--
--
-- RcsId: $Id: gexec.m,v 1.9 1993/10/04 12:11:18 paul Exp $
--
-- (C) Copyright 1992 Perihelion Software Ltd.
-- 

_report ['include 'gexec.m]
_def 'gexec.m_flag 1


include structs.m
include sem.m

_if _defp 'helios.C40 [
	include	c40.m
	include c40exec.m
]

_if _defp 'helios.arm [
	include arm.m
	include armexec.m
]


------------------------------------------------------------------------------
-- Miscellanous executive definitions:

-- Defines highest priority.
_def 'HIGHPRI		0


-- One millisecond in microseconds.
_def 'ONEMILLISEC	1000


-- Size in bytes of KDebug buffer.
_def KDSIZE		64


------------------------------------------------------------------------------
-- Executive structures:

-- The ThreadQ structure holds the runnable threads for each priority's run Q

struct ThreadQ [
	word head	-- SaveState *
	word tail	-- SaveState *
] -- ThreadQ;


-- LinkReq struct hold status of link read/write over interrupts

struct LinkReq [
	word	Count	--  number of bytes to read (or words on C40)
	word	Buf	-- address of buffer (wordptr on C40)
] -- LinkReq;


-- ExecRoot holds info on the current executing thread as well as all of
-- executive's static data.

struct ExecRoot [
	word	CurrentSaveArea		-- Points to current threads
					-- Save area (BPTR) - must be first
	word	CurrentPri		-- Priority of current thread
					-- The run Q's
	word	HighestAvailPri		-- Highest runnable thread

	word	KernelRoot		-- kernel RootStruct address (BPTR)
					-- *DO NOT* change pos without changing
					-- use by GetRoot macro in root.m!
	word	Nucleus			-- load address of nucleus (WPTR)

	word	IdleTime		-- Amount of time spent in idle
	word	Timer			-- Current Microsecond clock value
	word	SliceTime		-- Clock ticks until next time slice
	word	TicksPerSlice		-- Quantum given to each slice
	word	SliceEnabled		-- TRUE if we can time slice
	word	TimerQ			-- Timer queue for Sleep() (BPTR)

	--Run Q's for different priorities
	word	Queue0.head
	word	Queue0.tail
	word	Queue1.head
	word	Queue1.tail
	word	Queue2.head
	word	Queue2.tail
	word	Queue3.head
	word	Queue3.tail
	word	Queue4.head
	word	Queue4.tail
	word	Queue5.head
	word	Queue5.tail
	word	Queue6.head
	word	Queue6.tail
	word	Queue7.head
	word	Queue7.tail

	word	KnownThreads		-- List of all known current threads
					-- chained off SaveState->nextknown

	_test _defp 'helios.C40 [
		word	CAddressBase	-- Standard system C Address Base 
		word	HWConfig	-- default config sent to bootstrap
	][
		word	CriticalErrorVector
					-- Call this function (registered via
					-- DefineExecErrorHandler()) whenever
					-- a processor exception occurs
	]
					-- Holds small stack for Dispatch()
	vec	[_mul DISPATCHSTACKSIZE 4] DispatchStack

	-- These structures hold count and pointers for that links current
	-- transaction - set up by LinkRx/Tx and used by the INTERRUPT
	-- DRIVEN versions of __LinkTx/Rx
	_if _and _defp 'LINKINTR _defp 'helios.C40 [
		struct LinkReq		LinkInStat0
		struct LinkReq		LinkInStat1
		struct LinkReq		LinkInStat2
		struct LinkReq		LinkInStat3
		struct LinkReq		LinkInStat4
		struct LinkReq		LinkInStat5

		struct LinkReq		LinkOutStat0
		struct LinkReq		LinkOutStat1
		struct LinkReq		LinkOutStat2
		struct LinkReq		LinkOutStat3
		struct LinkReq		LinkOutStat4
		struct LinkReq		LinkOutStat5
	]

	_if _and _defp 'LINKINTR _defp 'helios.arm [
		_if _eq COMMSLINKS 4 [
			struct LinkReq		LinkInStat0
			struct LinkReq		LinkInStat1
			struct LinkReq		LinkInStat2
			struct LinkReq		LinkInStat3

			struct LinkReq		LinkOutStat0
			struct LinkReq		LinkOutStat1
			struct LinkReq		LinkOutStat2
			struct LinkReq		LinkOutStat3
		]
		_if _eq COMMSLINKS 1 [
			struct LinkReq		LinkInStat0
			struct LinkReq		LinkOutStat0
		]
	]

	-- C40 specific items:
	_if _defp 'helios.C40 [
		struct	IDROM ID_ROM		-- IDROM contents

		word	StartupStack		-- initial startup stack
		word	Latency			-- last timer latency seen
		word	MaxLatency		-- max timer latency seen
		word	MaxLatencyPC		-- PC of high latency thread
	]

	_if _defp 'LATENCYTEST [
		word	DispatchLatMs;		-- execroot timer at dispatch
		struct Sem DispatchLatCheck -- kick to high pri thread.
	]

	_if _defp 'helios.arm [
		-- Platform specific static data */

		_test _defp '__VY86PID [
			word	Serial1_IER_softcopy
		][
			word	spare0
		]
		word		spare1
		word		spare2
		word		spare3
		word		spare4
		word		spare5
		_if _not _defp 'LATENCYTEST [
			-- space for new dispatch latency test vars
			word		spare6
			word		spare7
			word		spare8
			word		spare9
		]
	]

	-- KDebug specific items:
	vec KDSIZE KDBuffer		-- static buffer for KDebug()'s
	word	KDBufPos		-- Position in buffer
	word	KDebugEnable		-- TRUE = can output KDebugs
	-- KERNELDEBUG3 support (message passing version)
	word	KD3StartB		-- start of msg dump buffer
	word	KD3EndB			-- end of msg dump buffer
	word	KD3StartM		-- pointer to start of msgs
	word	KD3EndM			-- pointer to end of msgs
	struct Sem KD3Sem		-- Kick sem to initiate msg send
	_if _defp 'helios.arm [
		vec	[_mul STARTUPSTACKSIZE 4] StartupStack
	]
] -- ExecRoot
 


-- End of gexec.m

