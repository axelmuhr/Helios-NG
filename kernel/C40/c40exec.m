-- File:	c40exec.m
-- Subsystem:	'C40 Helios executive
-- Author:	P.A.Beskeen
-- Date:	Nov '91
--
-- Description: 'C40 specific Helios executive manifests
--
--
-- RcsId: $Id: c40exec.m,v 1.14 1993/02/05 11:12:56 paul Exp $
--
-- (C) Copyright 1991 Perihelion Software Ltd.
-- 
---
-- WARNING: These definition must be kept upto date with "c40exec.h"
--

_report ['include 'c40exec.m]
_def 'c40exec.m_flag 1

include structs.m
include sem.m
include c40.m
include c40mmap.m
include cpustate.m
include tim40.m


_def 'LINKDMA 1		-- default to DMA rather than interrupt driven links

_if _not _defp 'LINKDMA [
	_def 'LINKINTR 1
]

-- LinkReq struct hold status of link read/write over interrupts
struct LinkReq [
	-- @@@ maybe add mask for this links IOCRDY/IOCFULL intr. here?
	word	Count		-- number of words remaining to read/write
	word	Buf		-- current buffer pointer (WPTR)
]


-- Place address of execroot into register.
-- Address is defined by bootstrap that loads the address of the execroot
-- into the tvtp expansion regsister.
_defq 'GetExecRoot['AddrReg] [
	ldep	tvtp, AddrReg
]

-- Place word address of nucleus root struct into register.
-- @@@ be aware of GetRoot macro in root.m that will need changing if
-- ExecRoot.Nucleus changes position
_defq 'GetNucRoot['AddrReg] [
	ldep	tvtp, AddrReg
	-- C40WordAddress *+AddrReg(ExecRoot.KernelRoot), AddrReg
	lsh	-2, *+AddrReg(ExecRoot.KernelRoot), AddrReg
	addi	R_BASE, AddrReg
]


-- Misc exec definitions

_def 'ONEMILLISEC	1000		-- one millisecond in microseconds

_def 'PRIORITYLEVELS	8		-- number of priority levels
_def 'HIGHPRI		0		-- defines highest priority
_def 'REALIDLEPRI	PRIORITYLEVELS	-- defines scheduler idle priority

-- DEFAULTTICKSPERSLICE defines the default number of clock ticks for each
-- thread before it is sliced - currently the clock ticks at one millisecond
-- intervals.
_def 'DEFAULTTICKSPERSLICE	10		-- 1/100 of a second slice

-- DISPATCHSTACKSIZE is the maximum size of stack required for a call to
-- Dispatch(). It is use to allow a thread to stay in the dispatchers idle
-- loop until a new thread is sheduled, but still allow interrupts to occur
_def 'DISPATCHSTACKSIZE	40		-- words of stack space



-- endof c40exec.m
