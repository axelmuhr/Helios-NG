_def	'C40SPBUG 1	-- @@@ define while we build for TMX mask
_def	'C40STIKBUG 1	-- @@@ define while we build for TMX mask

-- File:	c40.m
-- Subsystem:	'C40 Helios AMPP macros
-- Author:	Nick Clifton, Paul Beskeen
-- Date:	Dec '91
--
-- Description: `C40 general and register manifests
--
--
-- RcsId: $Id: c40.m,v 1.16 1993/08/06 14:59:15 nickc Exp $
--
-- (C) Copyright 1992 Perihelion Software Ltd.


_report ['include c40.m]
_def 'c40.m_flag 1


-------------------------------------------------------------------------------
-- Helios C Procedure Call Standard (PCS) register aliases
--
-- The following register aliases must agree with those defined in
-- cc350/c40/target.h and <cpustate.h>

_def 'R_A1		R0
_def 'R_A2		R1  	-- second argument register
_def 'R_A3		R2	-- third  argument register
_def 'R_A4		R3	-- fourth argument register

_def 'R_V1		DP	-- variable registers
_def 'R_V2		BK
_def 'R_V3		R8
_def 'R_V4		R9

_def 'R_T1		R10	-- temporary regs

_def 'R_TMP1		RS	-- back-end temporary regs
_def 'R_TMP2		RE
_def 'R_TMP3		RC

_def 'R_ATMP		AR5	-- temporary backend address reg

_def 'R_LR		R11	-- link register

_def 'R_ADDR1		AR0	-- address registers
_def 'R_ADDR2		AR1
_def 'R_ADDR3		AR2
_def 'R_ADDR4		AR3

_test _defp 'PCS_3
[ -- new PCS
_def 'R_A1result	R0
_def 'R_INTresult	R0
_def 'R_FLTresult	R4
_def 'R_FA1		R4	-- first  fp arg reg
_def 'R_FA2		R5	-- second fp arg reg
_def 'R_FV1		R6	-- first  fp variable reg
_def 'R_FV2		R7	-- seconf fp variable reg
]
[ -- old PCS
_def 'R_A1result	R0
_def 'R_FA1		R0
_def 'R_FA2		R2	-- second fp arg reg
_def 'R_FV1		R4	-- fp variable regs
_def 'R_FV2		R5
_def 'R_FT1		R6	-- floating point temporary
_def 'R_FT2		R7	-- floating point temporary
] -- old PCS

_def 'R_MT		AR4	-- module table pointer
_def 'R_USP		AR6	-- user stack pointer
_def 'R_FP		AR7	-- frame pointer
_def 'R_BASE		IR0	-- byte address base
_def 'R_USE		IR1	-- user stack stack end pointer
_def 'R_SE		IR1	-- synonym R_USE
_def 'R_SSP		SP	-- system stack pointer
_def 'R_ST		ST	-- status register


-- 'C40 Procedure Call Standard (PCS) manifests

-- The number of arguments passed in registers as defined by the 'C40 PCS
_test _defp 'PCS_3
[
_def 'PCS_INTARGREGS	4
_def 'PCS_FLTARGREGS	4
]
[
_def 'PCS_ARGREGS	4
]

-- The number of bytes held as a stack overflow area. The true end of the
-- stack is pointed to by the user stack end register + PCSSTACKGUARD
-- NB/ This value MUST correspond with its namesake in include/c40.h
_def 'PCS_STACKGUARD	384   -- 96 * 4

-- Notes what type of user stack is used by the PCS.
_def 'PCS_FULLDECENDING	1


-------------------------------------------------------------------------------
-- Definitions for status register bits
--
--	To set bits x and y use:
--		OR	ST_X | ST_Y, ST
--
--	To reset bits x and y use:
--		ANDN	ST_X | ST_Y, ST

_def 'ST_C		1
_def 'ST_V		[(1 << 1)]
_def 'ST_Z		[(1 << 2)]
_def 'ST_N		[(1 << 3)]
_def 'ST_UF		[(1 << 4)]
_def 'ST_LV		[(1 << 5)]
_def 'ST_LUF		[(1 << 6)]
_def 'ST_OVM		[(1 << 7)]
_def 'ST_RM		[(1 << 8)]
_def 'ST_PCF		[(1 << 9)]
_def 'ST_CF		[(1 << 10)]
_def 'ST_CE		[(1 << 11)]
_def 'ST_CC		[(1 << 12)]
_def 'ST_GIE		[(1 << 13)]
_def 'ST_PGIE		[(1 << 14)]
_def 'ST_SET_COND	[(1 << 15)]
_def 'ST_ANALYSIS	[(1 << 16)]


-- Simple ST reg macros

_def 'AllIntsOff
[
	andn	ST_GIE, ST
]

_def 'AllIntsOn
[
	or	ST_GIE, ST
]


-------------------------------------------------------------------------------
-- Definitions for the Internal Interrupt Enable (IIE) register

-- Timer interrupt enables
--
-- Set = enable interrupt.
_def 'IIE_ETINT0		1
_def 'IIE_ETINT1		[(1 << 31)]

-- Bit seperation between ports
_def 'IIE_NEXTPORT	4

-- Input buffer full interrupt enable
--
-- Set = enable interrupt.
_def 'IIE_EICFULL0	[(1 << 1)]
_def 'IIE_EICFULL1	[(1 << 5)]
_def 'IIE_EICFULL2	[(1 << 9)]
_def 'IIE_EICFULL3	[(1 << 13)]
_def 'IIE_EICFULL4	[(1 << 17)]
_def 'IIE_EICFULL5	[(1 << 21)]

-- Input buffer ready interrupt enable
--
-- Set = enable interrupt.
_def 'IIE_EICRDY0	[(1 << 2)]
_def 'IIE_EICRDY1	[(1 << 6)]
_def 'IIE_EICRDY2	[(1 << 10)]
_def 'IIE_EICRDY3	[(1 << 14)]
_def 'IIE_EICRDY4	[(1 << 18)]
_def 'IIE_EICRDY5	[(1 << 22)]

-- Output buffer ready interrupt enable
--
-- Set = enable interrupt.
_def 'IIE_EOCRDY0	[(1 << 3)]
_def 'IIE_EOCRDY1	[(1 << 7)]
_def 'IIE_EOCRDY2	[(1 << 11)]
_def 'IIE_EOCRDY3	[(1 << 15)]
_def 'IIE_EOCRDY4	[(1 << 19)]
_def 'IIE_EOCRDY5	[(1 << 23)]

-- Output buffer empty interrupt enable
--
-- Set = enable interrupt.
_def 'IIE_EOCEMPTY0	[(1 << 4)]
_def 'IIE_EOCEMPTY1	[(1 << 8)]
_def 'IIE_EOCEMPTY2	[(1 << 12)]
_def 'IIE_EOCEMPTY3	[(1 << 16)]
_def 'IIE_EOCEMPTY4	[(1 << 20)]
_def 'IIE_EOCEMPTY5	[(1 << 24)]

-- DMA coprocessor channel interrupt enable
--
-- Set = enable interrupt.
_def 'IIE_EDMAINT0	[(1 << 25)]
_def 'IIE_EDMAINT1	[(1 << 26)]
_def 'IIE_EDMAINT2	[(1 << 27)]
_def 'IIE_EDMAINT3	[(1 << 28)]
_def 'IIE_EDMAINT4	[(1 << 29)]
_def 'IIE_EDMAINT5	[(1 << 30)]


-------------------------------------------------------------------------------
-- Definitions for the Internal / External Interrupt Flag (IIF) register

-- Define external pins (IIOF0-3) as interrupts or I/O pins
--
-- Set = interrupt, reset = I/O pin.
_def 'IIF_FUNC0		1
_def 'IIF_FUNC1		[(1 << 4)]
_def 'IIF_FUNC2		[(1 << 8)]
_def 'IIF_FUNC3		[(1 << 12)]

-- Define sub type of external pins (IIOF0-3)
--
-- If interrupt pin: Set = level triggered, Reset = edge triggered.
-- If I/O pin: Set = output, Reset = input.
_def 'IIF_TYPE0		[(1 << 1)]
_def 'IIF_TYPE1		[(1 << 5)]
_def 'IIF_TYPE2		[(1 << 9)]
_def 'IIF_TYPE3		[(1 << 13)]

-- Define flags for external pins (IIOF0-3)
--
-- If interrupt pin: set = interrupt asserted, reset = interrupt not asserted.
-- Writing zero to bit clears interrupt.
-- If input pin:  Reading returns the value of the pin. The pin is read only.
-- If output pin: The bit is the value on the pin and is read / write.
_def 'IIF_FLAG0		[(1 << 2)]
_def 'IIF_FLAG1		[(1 << 6)]
_def 'IIF_FLAG2		[(1 << 10)]
_def 'IIF_FLAG3		[(1 << 14)]

-- Interrupt enable for external pins (IIOF0-3)
--
-- If interrupt pin: Set = interrupt enabled, Reset = interrupt disabled.
-- No effect if not set as interrupt pin.
_def 'IIF_EIIOF0		[(1 << 3)]
_def 'IIF_EIIOF1		[(1 << 7)]
_def 'IIF_EIIOF2		[(1 << 11)]
_def 'IIF_EIIOF3		[(1 << 15)]

-- NMI flag
--
-- Set if NMI asserted, reset if not asserted.
_def 'IIF_NMI		[(1 << 16)]

-- Timer interrupt flags
--
-- Set if timer interrupt asserted, reset if not asserted.
-- Writing zero to bit clears interrupt.
_def 'IIF_TINT0		[(1 << 24)]
_def 'IIF_TINT1		[(1 << 31)]

-- DMA coprocessor channel interrupt flags
--
-- Set = channel interrupt asserted, reset = not asserted.
-- Writing 0 to bit clears the interrupt.
_def 'IIF_EDMAINT0	[(1 << 25)]
_def 'IIF_EDMAINT1	[(1 << 26)]
_def 'IIF_EDMAINT2	[(1 << 27)]
_def 'IIF_EDMAINT3	[(1 << 28)]
_def 'IIF_EDMAINT4	[(1 << 29)]
_def 'IIF_EDMAINT5	[(1 << 30)]


-------------------------------------------------------------------------------
-- Byte / Word pointer conversions

-- Convert a Helios C byte address pointer to a 'C40 CPU word pointer
-- All C (byte) addresses can be converted to word addresses, but may loose
-- some byte selection information in the conversion. This loss is not checked
-- for.

_defq 'C40WordAddress['CptrReg] [
	lsh	-2, CptrReg
	addi	R_BASE, CptrReg
]

-- Convert a 'C40 CPU word pointer to a Helios C byte address pointer.
-- Points to first byte of word. Note that this function does NOT check if
-- the conversion is actually possible. i.e. that the current address base
-- (IR0) is in the same address range quadrant as the word address.

_defq 'C40CAddress['WptrReg] [
	subi	R_BASE, WptrReg
	lsh	2, WptrReg
]


-------------------------------------------------------------------------------
-- Macros for Loading 32bit constants and
-- absolute addresses of labels into registers.

-- Load the absolute address of label into an address register.
-- Label can be up to 2GW away.
-- Address register must not be R_ATMP, plus R_ATMP gets corrupted.

_def 'ldabs32['label 'Areg] [
	ldi	R_LR, R_ATMP			-- save link reg
	laj	_absaddr_'$label + 1		-- jump past data word
		nop				-- get around LAJ bug
		ldi	R_LR, Areg
		addi	*Areg, Areg		-- absaddr + *absaddr
_absaddr_'$label:
	int	shift(-2, labelref(label))
	ldi	R_ATMP, R_LR			-- restore link reg
]

--
-- This macro is just like 'ldabs32 except that
-- it does not corrupt R_ATMP, but it does corrupt R_LR
--
_def 'ldabs32_unsafe['label 'Areg]
[
	laj	_absaddr_'$label + 1		-- jump past data word
		nop				-- get around LAJ bug
		ldi	R_LR, Areg
		addi	*Areg, Areg		-- absaddr + *absaddr
_absaddr_'$label:
	int	shift(-2, labelref(label))
]


-- Load the absolute address of label into a register.
-- Label can be +/- 128kb away.
-- Corrupts R_ATMP.

_def 'ldabs16['label 'reg] [
	ldi	R_LR, R_ATMP			-- save link reg
	laj	4
		nop				-- get around LAJ bug
		patchinstr(PATCHC40MASK16ADD,
			shift(-2, labelref(label)),
			addi	-2, R_LR)	-- add . + label
		ldi	R_LR, reg
	ldi	R_ATMP, R_LR			-- restore link reg
]

-- Load the absolute address of label into a register.
-- Label can be +/- 128kb away.
-- Corrupts R_LR

_def 'ldabs['label 'reg] [
	nop
	laj	4
		nop				-- get around LAJ bug
		patchinstr(PATCHC40MASK16ADD,
			shift(-2, labelref(label)),
			addi	-2, R_LR)	-- add . + label
		ldi	R_LR, reg
]


-- Load a 32bit constant into a register

_def 'ldi32['const 'reg] [
	ldhi	(((const) >> 16) & 0xffff), reg
	or	((const) & 0xffff), reg
]


-------------------------------------------------------------------------------
-- Misc Definitions


-- Define 'C40 interrupt manifests
-- IIOF0-3 and TINIT1 can be used
_def InterruptVectors	6


-- define 'vector' number passed via SetEvent
_def INTR_NMI		0
_def INTR_IIOF0		1
_def INTR_IIOF1		2
_def INTR_IIOF2		3
_def INTR_IIOF3		4
_def INTR_TINT1		5


_if _not _defp 'NULL [
	_def 'NULL	0
	_def 'FALSE	0
	_def 'TRUE	1
]


-- end of c40.m
