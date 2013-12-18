-- File:	ioc.m
-- Subsystem:	Helios/ARM implementation
-- Author:	P.A.Beskeen
-- Date:	Oct '92
--
-- Description: IOC (VL86C410) ARM I/O Controller manifests
--		Includes timers, serial keyboard, interrupt control,
--		periperal access and programmable I/O pins.
--
--
-- RcsId: $Id: ioc.m,v 1.2 1993/12/10 14:14:14 paul Exp $
--
-- (C) Copyright 1992 Perihelion Software Ltd.
--     All Rights Reserved.
-- 

_report ['include ioc.m]
_def 'ARM/ioc.m_flag 1


-- Std IOC address.

_if _not _defp 'IOC_base [
	_def	'IOC_base	0x03200000
]


-- IOC register block definitions.

struct intr_block [
	byte	status		-- RO
				byte pad0a	byte pad0b	byte pad0c
	byte	request		-- R/W (W = clear)
				byte pad1a	byte pad1b	byte pad1c
	byte	mask		-- R/W
				byte pad2a	byte pad2b	byte pad2c
				byte pad3a	byte pad3b	byte pad3c
								byte pad3d
]

-- Pseudonym for write operations to intr block registers
_defq 'intr_block.clear [intr_block.request]


struct timer_block [
	byte	count_lo	-- R/W (W = latch)
				byte pad0a	byte pad0b	byte pad0c
	byte	count_hi	-- R/W (W = latch)
				byte pad1a	byte pad1b	byte pad1c
	byte	go_cmd		-- WO
				byte pad2a	byte pad2b	byte pad2c
	byte	latch_cmd	-- WO
				byte pad3a	byte pad3b	byte pad3c
]

-- Pseudonyms for write operations to timer count registers
_defq 'timer_block.latch_lo [timer_block.count_lo]
_defq 'timer_block.latch_hi [timer_block.count_hi]


struct ioc_regs [
	byte	control		-- R/W
				byte pad0a	byte pad0b	byte pad0c
	byte	kart_data	-- R=Rx, w=Tx
				byte	pad1a	byte pad1b	byte pad1c

				byte pad2a  byte pad2b  byte pad2c  byte pad2d
				byte pad3a  byte pad3b  byte pad3c  byte pad3d

	struct	intr_block	irq_a
	struct	intr_block	irq_b
	struct	intr_block	firq

	struct	timer_block	timer_0
	struct	timer_block	timer_1
	struct	timer_block	timer_baud
	struct	timer_block	timer_kart
]


-- Internal IOC interrupt bits.

-- Std IRQ status/mask/request/clear register A bits.
-- Mostly latched events, cleared via IRQ clear reg.

_def 'IRQA_POR		[(1 << 4)]	-- Power On Reset
_def 'IRQA_TM0		[(1 << 5)]	-- Timer 0 expiry
_def 'IRQA_TM1		[(1 << 6)]	-- Timer 1 expiry
_def 'IRQA_FORCE	[(1 << 7)]	-- Force IRQ int bit (permanently 1)


-- Std IRQ status/mask/request register B bits.

_def 'IRQB_KSTx		[(1 << 6)]	-- KART transmit done
_def 'IRQB_KSRx		[(1 << 7)]	-- KART receiver data ready


-- Other interrupt bits in IOC registers are defined in hw_machine.h machine
-- specfic header files.

-- Delay required after KSRx set in IRQ B status before reading KART data reg,
-- with KART serial line on maximum speed.

_def 'IOC_KART_DELAY	16		-- in microseconds: 1/2 bit time


_test _false [
	-- Number of nanoseconds for each timer tick.
	-- Number arrived at impirically as documentation doesn't seem to be
	-- correct!

	_def 'IOC_Timer_Resolution	125	-- nS

][
	-- Number of nanoseconds for each timer tick (assumes 8Mhz IOC)
	-- As documented in the IOC manual!?

	_def 'IOC_Timer_Resolution	500	-- nS
]

-- One microseconds worth of timer ticks (1 tick = 125ns/500ns, 8/2 = 1uS)

_def 'IOC_Timer_1ms		[(1000 / IOC_Timer_Resolution)]

-- One milliseconds worth of timer ticks (1 tick = 125ns/500ns, 8/2 = 1uS)

_def 'IOC_Timer_1ms		[((1000 / IOC_Timer_Resolution) * 1000)]



-- end of ioc.m
