-- File:	armexec.m
-- Subsystem:	ARM Helios executive
-- Author:	P.A.Beskeen
-- Date:	Oct '92
--
-- Description: ARM specific Helios executive manifests
--
--
-- RcsId: $Id: armexec.m,v 1.1 1993/08/24 08:45:33 paul Exp $
--
-- (C) Copyright 1991 Perihelion Software Ltd.
-- 
---
-- WARNING: These definition must be kept upto date with "armexec.h"
--

_report ['include 'armexec.m]
_def 'armexec.m_flag 1

include structs.m
include sem.m
include arm.m


_if _and _defp '__ARCHIMEDES _not _defp '__IOC [
	-- ARM IOC chip used by Archimedes.
	_def '__IOC 1
]



------------------------------------------------------------------------------
-- Define executive/kernel options:

_def 'LINKIO 1				-- Processor has links.

_if _defp 'LINKIO [
	_def 'LINKINTR 1		-- Set interrupt driven links.

	_def 'COMMSLINKS 4		-- Max Four possible link devices.
]

_def 'PERFMON 1				-- Enable performance monitor.

_if _defp '__ABC [
	_def 'ONCHIPRAM 1		-- If processor has on-chip RAM.
]

_def 'PRIORITYLEVELS	8		-- Number of priority levels.



------------------------------------------------------------------------------
-- Define executive parameters:

-- DEFAULTTICKSPERSLICE defines the default number of clock ticks for each
-- thread before it is sliced - currently the clock ticks at one millisecond
-- intervals.

_def 'DEFAULTTICKSPERSLICE	10		-- 1/10 of a second slice


-- DISPATCHSTACKSIZE is the maximum size of stack required for a call to
-- Dispatch(). It is use to allow a thread to stay in the dispatchers idle
-- loop until a new thread is sheduled, but still allow interrupts to occur
-- Must include the size of a SaveState structure.

_def 'DISPATCHSTACKSIZE		48		-- words of stack space


-- STARTUPSTACKSIZE is the size of the stack required for the first
-- thread used to initialise the system. @@@ This could well be smaller.

_def 'STARTUPSTACKSIZE		384		-- words of stack space


------------------------------------------------------------------------------
-- Misc ARM exec definitions

-- Branch opcode, used in initialising the exception vectors.

_def 'OP_AlwaysBranch		[(0b11101010 << 24)]



------------------------------------------------------------------------------
-- Basic Executive memory map definitions

-- All these constants are loadable with a single mov instruction as
-- they are all eight bit constants that are shiftable to their 32bit true
-- positions. All stacks are of the full decending variety i.e. push uses
-- post decrement.

_if _false [
	-- Default Nucleus load address in RAM based system (passed to kstart by
	-- bootstrap).

	_def	'Nucleus_base		0x3000


	-- Define ExecRoot position in low RAM. *WARNING* If this manifest is changed,
	-- then update ExecRoot_base in armexec.h.

	_def	'ExecRoot_base		0x2800

	-- Note NO SVC stack - SWI uses user stack, undef intr. & aborts set own stack
	_def	'SWI_table_base		0x8800	-- base of 4k (1k entries) swi table

	_def	'IRQ_stack_top		0x87ff	-- 2k IRQ stack
	_def	'IRQ_stack_end		0x8000	-- 2k IRQ stack

	_def	'FIQ_code_base		0x1c	-- 2k FIQ handler code area
][
	-- Default Nucleus load address in RAM based system (passed to kstart by
	-- bootstrap). *WARNING* If this manifest is changed, then update
	-- I/O Server boot protocol for some systems (Arch/PID)
	-- (Current addr = 0xA000)

	_defq	'Nucleus_base		[(ExecRoot_base + 0x800)]

	-- Define ExecRoot position in low RAM. *WARNING* If this manifest is
	-- changed, then update ExecRoot_base in armexec.h.

	-- Static data area for executive. This is THE fixed constant
	-- in the system (Current addr 0x9800)
	_defq	'ExecRoot_base		[(SWI_table_base + 0x1000)]

	-- Note NO SVC stack - SWI uses user stack, undef intr. & aborts set own stack

	-- base of 4k (1k entries) swi table
	_defq	'SWI_table_base		[(EXECRAMBASE + 0x0800)]

	-- 2k IRQ stack
	_defq	'IRQ_stack_top		[(EXECRAMBASE + 0x7ff)]
	_defq	'IRQ_stack_end		[(EXECRAMBASE + 0)]

	-- Note no FIQ stack - can use 2k - 0x1c area for code or data

	_defq	'FIQ_code_base		[0x1c]	-- 2k FIQ handler code area

	-- Leaves first 32k free for bootstrap/monitor system
	-- *Warning if this constant is changed then ExecRoot_base in the C
	-- header file armexec.h must be changed accordingly.
	_defq	'EXECRAMBASE		[0x8000]
]


------------------------------------------------------------------------------
-- Common address loading macros

-- Place address of execroot into register.
_defq 'GetExecRoot['Reg] [
	mov	Reg, ExecRoot_base
]

-- Place word address of nucleus root struct into register.
_defq 'GetNucRoot['Reg] [
	GetExecRoot Reg
	ldr	Reg, (Reg, ExecRoot.KernelRoot)
]

-- Place address of SWI jump table into register.
_defq	'GetSWITable['Reg] [
	mov	Reg, SWI_table_base
]


------------------------------------------------------------------------------
-- Processor independent CPU interrupt mask setting

-- SVC mode disabling of interrupts via processors mask
_defq	'AllIntsOff [
	_test _defp __ARM6 [
		error "AllIntsOff not defined for ARM600"
	][
		teqp	pc, INTRMask | SVCMode
	]
]

-- SVC mode enabling of interrupts via processors mask
_defq	'AllIntsOn [
	_test _defp __ARM6 [
		error "AllIntsOn not defined for ARM6"
	][
		teqp	pc, 0 | SVCMode
	]
]

------------------------------------------------------------------------------
-- Processor independent macros for setting the processor modes.
-- These macro's are designed to be used from non-user mode's to swap between
-- each other. Care should be exersised in no accesses to banked registers
-- should occur in the next instruction after the macro.

-- Set User mode and *ENABLE* interrupts
_defq 'SetUserMode [
	_test _defp __ARM6 [
		error "SetUserMode not defined for ARM6"
	][
		teqp	pc, 0 | UserMode
	]
]

-- Set SVC mode and disable interrupts
_defq 'SetSVCMode [
	_test _defp __ARM6 [
		error "SetSVCMode not defined for ARM6"
	][
		teqp	pc, INTRMask | SVCMode
	]
]

-- Set FIQ mode and disable interrupts
_defq 'SetFIQMode [
	_test _defp __ARM6 [
		error "SetFIQMode not defined for ARM6"
	][
		teqp	pc, INTRMask | FIQMode
	]
]

-- Set IRQ mode and disable interrupts
_defq 'SetIRQMode [
	_test _defp __ARM6 [
		error "SetIRQMode not defined for ARM6"
	][
		teqp	pc, INTRMask | IRQMode
	]
]

_if _defp '__ARM6 [
	error "@@@ need macros to set undef and abort modes on ARM6"
]



-- End of armexec.m
