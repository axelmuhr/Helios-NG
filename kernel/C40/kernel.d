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
-- Helios/C40 specific kernel library definitions.			--
--                                                                      --
-- PAB Dec 91								--
--									--
-- *WARNING* This file must reflect the contents of kmodule.a		--
--------------------------------------------------------------------------
-- RCSId: $Id: kernel.d,v 1.26 1993/03/26 16:49:54 nickc Exp $ --


global Kernel.library

_defq funcdef['fname]
[
	codetable [_$fname]
	global [_$fname]
]

_defq datadef['dname 'dsize]
[
	data [_$dname], dsize
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
-- funcdef _BootLink
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
-- funcdef SoftReset
funcdef TestWait
-- New functions added in Helios 1.2 below here
funcdef LogToPhysPri
funcdef PhysToLogPri
funcdef GetPhysPriRange
funcdef GetPriority
funcdef SetPriority
-- funcdef GetROMConfig
-- funcdef GetROMItem
funcdef GetNucleusBase
funcdef GetRootBase
funcdef CallWithModTab
funcdef _GetModTab
funcdef Timer
funcdef XchMsg

-- Generic NON transputer functions 
funcdef SaveCPUState
funcdef RestoreCPUState
--funcdef DefineExecErrorHandler
funcdef AvoidEvents
funcdef HardenedWait
funcdef HardenedSignal
funcdef	System

-- 2 fns moved from utillib
funcdef _cputime
funcdef _ldtimer

-- Extended memory allocation support
funcdef StatMem
funcdef	LowAllocMem
--funcdef ReAllocMem
--funcdef TrimAllocMem

funcdef	TimedWait
funcdef SliceState	-- enable/disable/report if slicing enabled
funcdef	SliceQuantum	-- set size of timeslice

-- dbg
funcdef _linkreg
funcdef _fpreg
funcdef _spreg

-- ncc compiler support fns
funcdef __divtest
funcdef __divide
funcdef __udivide
funcdef __remainder
funcdef __uremainder
funcdef __stack_overflow

funcdef GetExecRoot
funcdef AllocSpecial	-- Generic Fast/Global memory allocator

-- C40 specific functions
funcdef	memcpy
funcdef memset

funcdef	'C40CAddress
funcdef	'C40WordAddress

-- debug functions
funcdef JTAGHalt

funcdef	_udiv10		-- fast unsigned divide by 10
funcdef	_sdiv10		-- fast signed   divide by 10
funcdef	_backtrace	-- debug function
funcdef _DataToFuncConvert	-- converts data ptrs to fn ptrs
funcdef _FuncToDataConvert	-- converts fn ptrs to data ptrs

funcdef GetIDROM	-- returns pointer to saved IDROM structure

funcdef _stack_size
funcdef Accelerate

funcdef MP_GetWord	-- byteptr <-> wordptr data access functions
funcdef MP_PutWord
funcdef MP_GetData
funcdef MP_PutData

funcdef MP_ReadLock	-- Shared memory support functions.
funcdef MP_ReadFPLock
funcdef MP_WriteUnlock
funcdef MP_WriteFPUnlock
funcdef MP_Signal
funcdef MP_BusyWait

funcdef MP_LinkIn
funcdef MP_LinkOut

funcdef GetHWConfig
funcdef ReleaseStack

-- ^^^^^^^^^ add new functions here ^^^^^^^^^^
-- when adding functions also update kmodule.a

data .MaxData, 0
codetable .MaxCodeP

_undef 'funcdef
_undef 'datadef
