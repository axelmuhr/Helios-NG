-- File:	arm.m
-- Subsystem:	ARM Helios AMPP macros
-- Author:	Paul Beskeen
-- Date:	Sept. '92
--
-- Description: ARM general and register manifests
--
--
-- RcsId: $Id: arm.m,v 1.3 1993/08/05 17:06:06 paul Exp $
--
-- (C) Copyright 1992 Perihelion Software Ltd.
--     All RIghts Reserved.


_report ['include arm.m]
_def 'arm.m_flag 1


_if _and _defp '__ARCHIMEDES _not _defp '__IOC [
	-- ARM IOC chip used by Archimedes.
	_def '__IOC 1
]


-------------------------------------------------------------------------------
-- ARM Procedure Call Standard (PCS) manifests

-- The number of arguments passed in registers as defined by the 'C40 PCS
_def	'PCS_ARGREGS		4

-- The number of bytes held as a stack overflow area. The true end of the
-- stack is pointed to by the user stack end register + PCSSTACKGUARD
_def	'PCS_STACKGUARD		256

-- Notes what type of user stack is used by the PCS.
_def	'PCS_FULLDECENDING	1


-------------------------------------------------------------------------------
-- Interrupt vectors

-- To make platform portability easier, up to 64 possible indirect interrupt
-- vectors are defined. How these are mapped to actual devices is
-- platform dependent. @@@ This may be optimised in future.

-- Number of interrupt vectors
_def 'InterruptVectors	64


-- __ARCHIMEDES
-- The system interrupt handler on Archimedes based systems decodes IRQ
-- and FIQ interrupts into their IOC and expansion card sources.
-- 30 = (2 (Unknown IRQ/FIQ) + 24 (IOC) + 4 (XCB))
--
-- __IOC
-- IOC based systems decode IRQ and FIQ interrupts into their
-- IOC defined sources.
-- 26 = (2 (Unknown IRQ/FIQ) + 24 (IOC))
--
-- __VY86PID
-- VLSI PID board has eight IRQ sources and eight FIQ sources
-- 18 = (2 (Unknown IRQ/FIQ) + 8 (FIQ) + 8 (IRQ))


-- ARM interrupt vector numbers.
--
-- These define in a platform specific fashion which interrupt vectors
-- correspond to which interrupt sources.
--
-- On most systems the first two vectors are used to note unknown FIQ/IRQ
-- interrupt sources. On systems with no decoding, they simply correspond to
-- FIQ and IRQ.

_def	'INTR_FIQ	0
_def	'INTR_IRQ	1


_test _defp '__IOC [
-- Pseudo interrupt numbers for IOC based systems such as the Archimedes.
-- These numbers correspond to the bits held in the IOC request registers.
--
-- If special on Archimedes H/W:   Early Models		82C710/711 Models
--				   ------------		-----------------
-- IRQA request register
				-- printer busy		printer interrupt
_def 'INTR_IRQA_0	2	-- IOC: IL6
				-- serial port ring	low battery warning
_def 'INTR_IRQA_1	3	-- IOC: IL7
				-- printer ack		floppy disk index
_def 'INTR_IRQA_2	4	-- IOC: IF (high to low edge on IF)
				-- 		Vsync pulse
_def 'INTR_IRQA_3	5	-- IOC: IR (low to high edge on IR)
_def 'INTR_IRQA_4	6	-- IOC: power on reset
_def 'INTR_IRQA_5	7	-- IOC: Timer 0
_def 'INTR_IRQA_6	8	-- IOC: Timer 1
				--		FIQ downgrade
_def 'INTR_IRQA_7	9	-- IOC: force interrupt

-- IRQB request register
				--	expansion card FIQ downgrade
_def 'INTR_IRQB_0	10	-- IOC: IL0
				-- 	sound system buffer change
_def 'INTR_IRQB_1	11	-- IOC: IL1
				-- serial port ctrler	serial port 82C711
_def 'INTR_IRQB_2	12	-- IOC: IL2
				-- hard disk ctrler	IDE hard disk
_def 'INTR_IRQB_3	13	-- IOC: IL3
				-- floppy disk change	floppy 82C711 intr.
_def 'INTR_IRQB_4	14	-- IOC: IL4
				--	expansion card interrupt
_def 'INTR_IRQB_5	15	-- IOC: IL5
_def 'INTR_IRQB_6	16	-- IOC: KART Tx ready
_def 'INTR_IRQB_7	17	-- IOC: KART Rx ready


-- FIRQ request register

_def 'INTR_FIQ_0	18	-- IOC: FH0
_def 'INTR_FIQ_1	19	-- IOC: FH1
_def 'INTR_FIQ_2	20	-- IOC: FL
_def 'INTR_FIQ_3	21	-- IOC: C3
_def 'INTR_FIQ_4	22	-- IOC: C4
_def 'INTR_FIQ_5	23	-- IOC: C5
_def 'INTR_FIQ_6	24	-- IOC: IL0
_def 'INTR_FIQ_7	25	-- IOC: force interrupt

_if _defp '__ARCHIMEDES [
	-- Additional pseudo vectors for IOC based Archimedes.
	-- Decodes of the Acorn Expansion Card Bus (xcb) interrupts.

	_def 'INTR_XCB_0	26	-- IOC: IL4 - XCB slot 0
	_def 'INTR_XCB_1	27	-- IOC: IL4 - XCB slot 1
	_def 'INTR_XCB_2	28	-- IOC: IL4 - XCB slot 2
	_def 'INTR_XCB_3	29	-- IOC: IL4 - XCB slot 3
]

][ _if _defp '__VY86PID [
-- Pseudo interrupt vector number for the VLSI PID board.
_def 'INTR_IRQ_SERIAL	2	-- Serial port (intercepted by Kernel)
_def 'INTR_IRQ_TIMER	3	-- Timer (intercepted by Kernel)
_def 'INTR_IRQ_PARA	4	-- Parallel port
_def 'INTR_IRQ_3	5	-- Expansion slot IRQ 3
_def 'INTR_IRQ_4	6	-- Expansion slot IRQ 4
_def 'INTR_IRQ_5	7	-- Expansion slot IRQ 5
_def 'INTR_IRQ_6	8	-- Expansion slot IRQ 6
_def 'INTR_IRQ_PANIC	9	-- PANIC button
]
]


-------------------------------------------------------------------------------
-- ARM Processor Status Register bits

-- @@@ old definitions for ARM2/3 CPU
_def	'NFlag			[1 << 31]	-- miNus
_def	'ZFlag			[1 << 30]	-- Zero
_def	'CFlag			[1 << 29]	-- Carry
_def	'VFlag			[1 << 28]	-- oVerflow
_def	'FlagMask		[NFlag | ZFlag | CFlag | VFlag]

_def	'IRQDisable		[1 << 27]	-- processor IRQ disable
_def	'FIQDisable		[1 << 26]	-- processor FIQ disable
_def	'INTRMask		[IRQDisable | FIQDisable]

_def	'USRMode		[0b00]
_def	'FIQMode		[0b01]
_def	'IRQMode		[0b10]
_def	'SVCMode		[0b11]
_def	'ModeMask		[0b11]

_def	'PSRFlags		[FlagMask | INTRMask | ModeMask]

-- ARM6 psr bit definitions
_def	'A6_NFlag		[1 << 31]	-- Negative
_def	'A6_ZFlag		[1 << 30]	-- Zero
_def	'A6_CFlag		[1 << 29]	-- Carry
_def	'A6_VFlag		[1 << 28]	-- oVerflow
_def	'A6_FlagMask		[A6_NFlag | A6_ZFlag | A6_CFlag | A6_VFlag]

_def	'A6_IRQDisable		[1 << 7]	-- processor IRQ disable
_def	'A6_FIQDisable		[1 << 6]	-- processor FIQ disable
_def	'A6_IntrMask		[A6_IRQDisable | A6_FIQDisable]

_def	'A6_USR26Mode		[0b00000]
_def	'A6_FIQ26Mode		[0b00001]
_def	'A6_IRQ26Mode		[0b00010]
_def	'A6_SVC26Mode		[0b00011]

_def	'A6_USRMode		[0b10000]
_def	'A6_FIQMode		[0b10001]
_def	'A6_IRQMode		[0b10010]
_def	'A6_SVCMode		[0b10011]

_def	'A6_ABTMode		[0b10111]	-- new modes on ARM6
_def	'A6_UNDMode		[0b11011]

_def	'A6_ModeMask		[0b11111]

-- Macro for upwards compatible read/modify/write update of ARM6 psr.
_def	'A6_SETPSR['tmpreg 'psrmask 'psrbits] [
	mrs	tmpreg, cpsr
	bic	tmpreg, psrmask
	orr	tmpreg, psrbits
	msr	cpsr, tmpreg
]

-------------------------------------------------------------------------------
-- ARM Exception Vectors

_def	'VEC_Reset			0x00
_def	'VEC_UndefinedInstruction	0x04
_def	'VEC_SWI			0x08
_def	'VEC_PrefetchAbort		0x0c
_def	'VEC_DataAbort			0x10
_def	'VEC_AddressException		0x14	-- 26bit modes.
_def	'VEC_IRQ			0x18
_def	'VEC_FIQ			0x1C


-------------------------------------------------------------------------------
-- Misc Definitions

_if _not _defp 'NULL [
	_def 'NULL	0
	_def 'FALSE	0
	_def 'TRUE	1
]

_test _true [
	-- @@@ NOT PLATFORM PORTABLE - REMOVE
	_defq '_DBG['numbx] []
	_defq '_DBGB['numbx] []
	_defq '_DBG2['regx 'nammm] []
	_defq '_DBG2B['regx 'nammm] []
][

_defq '_DBG['numbx] [
	mov	tmp, 0x3300000
	add	tmp, 0x0042000
_xxx$numbx$:
	ldrb	a1, (tmp, 0xc)
	ands	a1, 1
	beq	_xxx$numbx
	mov	a1, 0x80 | ( numbx )
	strb	a1, (tmp, 4)
]

_defq '_DBGB['numbx] [
	mov	v2, 0x3300000
	add	v2, 0x0042000
_xxx$numbx$:
	ldrb	v1, (v2, 0xc)
	ands	v1, 1
	beq	_xxx$numbx
	mov	v1, 0x80 | ( numbx )
	strb	v1, (v2, 4)
]

_defq '_DBG2x['yyy 'xxx] [
_$xxx$yyy$:
	ldrb	a2 , (a1, 0xc)
	ands	a2 , 1
	beq	_$xxx$yyy
	mov	a3, a3 lsr 4
	and	a2, a3, 0xf
	orr	a2, 0x80
	strb	a2, (a1, 4)
]

_defq '_DBG2['regx 'nammm] [
	mov	a1, 0x3300000
	add	a1, 0x0042000

	mov	a3, regx

_xxx$regx1$nammm$:
	ldrb	a2 , (a1, 0xc)
	ands	a2 , 1
	beq	_xxx$regx1$nammm
	and	a2, regx, 0xf
	orr	a2, 0x80
	strb	a2, (a1, 4)

	_DBG2x 2 nammm
	_DBG2x 3 nammm
	_DBG2x 4 nammm
	_DBG2x 5 nammm
	_DBG2x 6 nammm
	_DBG2x 7 nammm
	_DBG2x 8 nammm

]

_defq '_DBG2Bx['yyy 'xxx] [
_$xxx$yyy$:
	ldrb	v2 , (v1, 0xc)
	ands	v2 , 1
	beq	_$xxx$yyy
	mov	v3, v3 lsr 4
	and	v2, v3, 0xf
	orr	v2, 0x80
	strb	v2, (v1, 4)
]

_defq '_DBG2B['regx 'nammm] [
	mov	v1, 0x3300000
	add	v1, 0x0042000

	mov	v3, regx

_xxx$regx1$nammm$:
	ldrb	v2 , (v1, 0xc)
	ands	v2 , 1
	beq	_xxx$regx1$nammm
	and	v2, regx, 0xf
	orr	v2, 0x80
	strb	v2, (v1, 4)

	_DBG2Bx 2 nammm
	_DBG2Bx 3 nammm
	_DBG2Bx 4 nammm
	_DBG2Bx 5 nammm
	_DBG2Bx 6 nammm
	_DBG2Bx 7 nammm
	_DBG2Bx 8 nammm

]
]

-- end of arm.m
