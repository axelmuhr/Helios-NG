-- File:	hw_arch.h
-- Subsystem:	Helios-ARM implementation
-- Author:	P.A.Beskeen
-- Date:	Oct '92
--
-- Description: Description of the ARM based Acorn Achimedies series
--		This includes many different models and can be a little
--		complex.
--
--		The Archimedies range consists of:
--		@@@ update with full range:
--			A440/R140 (ARM2, ST506)
--			A550 (ARM3, SCSI)
--			A680
--			A3000 (ARM2, IDE)
--			A3010 (ARM250, IDE)
--			A3020 (ARM250, IDE)
--			A4000
--			A5000 (ARM3, IDE)
--
--
-- RcsId: $Id: hw_arch.m,v 1.1 1993/08/05 13:10:57 paul Exp $
--
-- (C) Copyright 1992 Perihelion Software Ltd.
--     All Rights Reserved.


_report ['include 'hw_arch.m]
_def 'ARM/hw_arch.m_flag 1


include ARM/ioc.m	-- Most hardware is hung off an ARM IOC.
include ARM/xcb.m	-- Acorn expansion card bus (podules).


-- IOC interfaced hardware.

-- Control port register bits.

_def 'IOC_CON_I2C_DATA     [(1 << 0)]	-- I2C Bus data  line - R/W
_def 'IOC_CON_I2C_CLOCK    [(1 << 1)]	-- I2C Bus clock line - R/W


-- The sound mute line is defined on all machines, but only performs
-- its expected function on the A500 and A440.  On A680 it must be 
-- left as a 1.

_def 'IOC_CON_SOUND_MUTE   [(1 << 5)]	-- Sound Mute (A500/A440) - R/W


-- The set of bits which must be 1 when writing to the control port,
-- on any machine.

_def 'IOC_CON_WRITE_SET    [(1 << 7 | 1 << 6 | 1 << 4 | 1 << 3 | 1 << 2)]


-- The following bits are defined for A4x0 machines.

_def 'IOC_CON4_DISC_READY  [(1 << 2)]	-- floppy drive ready - RO
_def 'IOC_CON4_AUX_C4	   [(1 << 4)]	-- aux I/O line (unused) R/W


-- The following 3 bits are present only on A500's.

_def 'IOC_CON5_RTC_MINUTES [(1 << 2)]	-- Real Time Clock minutes - RO
_def 'IOC_CON5_RTC_SECONDS [(1 << 3)]	-- Real Time Clock seconds - RO
_def 'IOC_CON5_DISC_CHANGE [(1 << 4)]	-- Floppy disc changed - RO

-- A680 only.
_def 'IOC_CON6_DISC_CHANGE [(1 << 2)]	-- floppy disc changed - RO


-- Interrupt control register bits.

-- IRQ A block - mostly latched events, cleared via IRQ clear reg.

_def 'IRQA_PBSY		[(1 << 0)]	-- Printer BuSY
_def 'IRQA_RII		[(1 << 1)]	-- Serial line RInging Indication
_def 'IRQA_PACK		[(1 << 2)]	-- Printer ACKnowledge event
_def 'IRQA_VFLY		[(1 << 3)]	-- Vertical FLYback event


-- IRQ B block.

_def 'IRQB_XFIQ		[(1 << 0)]	-- XCB FIQ(!) bit - mask OFF
_def 'IRQB_SND		[(1 << 1)]	-- Sound buffer switch event
_def 'IRQB_SLCI		[(1 << 2)]	-- Serial Line Controller Int
_def 'IRQB_WINC		[(1 << 3)]	-- Winchester Controller int
_def 'IRQB_WIND		[(1 << 4)]	-- Winchester Data int
_def 'IRQB_WINCD	[(1 << 3)]	-- Combined Controller/Data bit (Archi)
_def 'IRQB_FDDC		[(1 << 4)]	-- Floppy Disc Disc Changed (Archi)
_def 'IRQB_FDINT	[(1 << 4)]	-- Floppy disc intr (A680)
_def 'IRQB_XCB    	[(1 << 5)]	-- Expansion card common IRQ


-- FIQ block.

_def 'FIQ_FDDR		[(1 << 0)]	-- Floppy Disc Data Request
_def 'FIQ_FDIR		[(1 << 1)]	-- Floppy Disc Interrupt Request
_def 'FIQ_ECONET	[(1 << 2)]	-- ECOnet interrupt
-- 3..5 not used
_def 'FIQ_XCB		[(1 << 6)]	-- XCB card FIQ
_def 'FIQ_FORCE 	[(1 << 7)]	-- Force FIQ int (permanently 1)


-- External (to IOC) I/O hardware control structures.

_def 'EXT_A_L4		0x03350040
_def 'EXT_A_L5		0x03360000
_def 'EXT_B_LATCH	0x03350018
_def 'EXT_L6		0x03350040


-- External Latch A bits.

_def 'UNITBITS		0xF
_def 'INUSEBIT		[(1 << 6)]
_def 'MOTORBIT		[(1 << 5)]
_def 'SIDEBIT		[(1 << 4)]


-- External Latch B floppy-specific bits on A500, A4xx.

_def 'DENBITS		0x7
_def 'DOUBDEN		0x5

_def 'FDCRESET		0x08
_def 'EXTB_FDC_BITS	[(DENBITS | FDCRESET)]


-- Other External Latch B bits.

-- Printer Strobe Line: bit 0 on A680, bit 4 on the others.

_def 'EXTB_L45_PR_STROBE [(1 << 4)]
_def 'EXTB_L6_PR_STROBE	 [(1 << 0)]


-- Audio system control (A500 only).

_def 'EXTB_AIN_MUTE	[(1 << 5)]	-- audio input mute
_def 'EXTB_AOUT_MUTE	[(1 << 6)]	-- audio output mute


-- Extra head select line for ST506 interface (not A680).

_def 'EXTB_HS3		[(1 << 7)]	-- Head Select 3 for HDC


-- FDC for A680 bits.

_def 'F6_UNITBITS	[((1 << 1) | (1 << 0))]
_def 'F6_DCRST		[(1 << 2)]	-- Disc change reset
_def 'F6_DENSITY	[(1 << 3)]	-- Density
_def 'F6_SIDEBIT	[(1 << 4)]	-- Side
_def 'F6_MOTORON	[(1 << 5)]	-- Motor on
_def 'F6_INUSE		[(1 << 6)]	-- In use
_def 'F6_RESET		[(1 << 7)]	-- Reset FDC



-- end of hw_arch.m
