--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- kernel.d								--
--                                                                      --
-- Helios/ARM specific kernel library definitions.			--
--                                                                      --
-- PAB June 90								--
--									--
-- *WARNING* This file must reflect the contents of kmodule.a		--
--------------------------------------------------------------------------
-- RCSId: $Id: kernel.d,v 1.4 1991/12/11 16:02:27 paul Exp $ --


global Kernel.library

_defq funcdef['fname]
[
	_test _defp '__SMT [
		codetable [_$fname]
	][
		data [_$fname] 4
	]
	global [_$fname]
]

_defq datadef['dname 'dsize]
[
	data [_$dname] 'dsize
	global [_$dname]
]
		
funcdef InitList
funcdef PreInsert
funcdef PostInsert
funcdef Remove
funcdef AddHead
funcdef AddTail
funcdef RemHead
funcdef RemTail
funcdef NewPort
funcdef FreePort
funcdef PutMsg
funcdef GetMsg
funcdef PutReady
funcdef GetReady
funcdef AbortPort
funcdef MultiWait
funcdef SendException
funcdef InitSemaphore
funcdef Wait
funcdef Signal
funcdef TestSemaphore
funcdef InitPool
funcdef AllocMem
funcdef FreeMem
funcdef FreePool
funcdef AllocFast
funcdef TaskInit
funcdef KillTask
funcdef CallException
funcdef _BootLink
funcdef EnableLink
funcdef AllocLink
funcdef FreeLink
funcdef Reconfigure
funcdef Terminate
funcdef LinkData
funcdef Delay
funcdef _Mark
funcdef _Trace
funcdef _Halt
funcdef WalkList
funcdef SearchList
funcdef InPool
funcdef MachineType
funcdef LinkIn
funcdef LinkOut
funcdef SetEvent
funcdef RemEvent
datadef _Task_ 4
funcdef InitProcess
funcdef StartProcess
funcdef StopProcess
funcdef GetPortInfo
funcdef FreeMemStop
funcdef SignalStop
funcdef Configure
funcdef SoftReset
funcdef TestWait
-- New functions added in Helios 1.2 below here
funcdef LogToPhysPri
funcdef PhysToLogPri
funcdef GetPhysPriRange
funcdef GetPriority
funcdef SetPriority
funcdef GetROMConfig
funcdef GetROMItem
funcdef GetNucleusBase
funcdef GetRootBase
funcdef CallWithModTab
funcdef _GetModTab
funcdef Timer
funcdef XchMsg
-- Generic NON transputer functions 
funcdef SaveCPUState
funcdef RestoreCPUState
funcdef DefineExecErrorHandler
funcdef AvoidEvents
funcdef HardenedWait
funcdef HardenedSignal
funcdef System
-- 2 fns moved from utillib
funcdef _cputime
funcdef _ldtimer

-- Extended memory allocation support
funcdef StatMem
funcdef	LowAllocMem
funcdef	ReAllocMem
--funcdef TrimAllocMem
funcdef	TimedWait
funcdef	SliceState
funcdef	SliceQuantum

funcdef _linkreg
funcdef _fpreg
funcdef _spreg

_if _defp __ABC [
	-- Fast memory allocator support
	funcdef FastStoreSize
]

-- compiler support fns
funcdef __multiply
funcdef _memcpy
funcdef __stack_overflow_1

funcdef __divtest
funcdef __divide
funcdef __udivide
funcdef __remainder
funcdef __uremainder
funcdef __stack_overflow

_if _defp __ABC [
	-- Relocatable Memory manager
	funcdef	MIAlloc
	funcdef	MIFree
	funcdef	MICompact
	funcdef	MILock
	funcdef	MIUnLock
	funcdef	MITrim
	funcdef MIInit

	-- CARD support
	funcdef	BuildPool
	funcdef RRDPoolInit

	funcdef SetUserEvent
	funcdef RemUserEvent
	funcdef CauseUserEvent

	-- Support for link guardians outside the kernel (using normal device drivers)
	funcdef IntelligentServer
	funcdef LinkTx
	funcdef LinkRx
	funcdef AbortLinkTx
	funcdef AbortLinkRx
	funcdef SchedulerDispatch
	funcdef Resume
]

-- ^^^^^^^^^ add new functions here ^^^^^^^^^^
-- when adding functions also update kmodule.a


data .MaxData 0
_if _defp '__SMT [
	codetable .MaxCodeP
]

_undef 'funcdef
_undef 'datadef
